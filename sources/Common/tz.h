#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#define LL2FILETIME( ll, pft )\
    (pft)->dwLowDateTime = (UINT)(ll); \
    (pft)->dwHighDateTime = (UINT)((ll) >> 32);
#define FILETIME2LL( pft, ll) \
    ll = (((LONGLONG)((pft)->dwHighDateTime))<<32) + (pft)-> dwLowDateTime ;


static const int MonthLengths[2][12] =
{
	{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
	{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

static inline int IsLeapYear(int Year)
{
	return Year % 4 == 0 && (Year % 100 != 0 || Year % 400 == 0) ? 1 : 0;
}

/***********************************************************************
 *  TIME_DayLightCompareDate
 *
 *  Compares two dates without looking at the year
 *
 * RETURNS
 *
 *  -1 if date < compareDate
 *   0 if date == compareDate
 *   1 if date > compareDate
 *  -2 if an error occurs
 */
static int TIME_DayLightCompareDate(
    const SYSTEMTIME *date,        /* [in] The local time to compare. */
    const SYSTEMTIME *compareDate) /* [in] The daylight saving begin
                                       or end date */
{
    int limit_day, dayinsecs;

    if (date->wMonth < compareDate->wMonth)
        return -1; /* We are in a month before the date limit. */

    if (date->wMonth > compareDate->wMonth)
        return 1; /* We are in a month after the date limit. */

    if (compareDate->wDayOfWeek <= 6)
    {
        WORD First;
        /* compareDate->wDay is interpreted as number of the week in the month
         * 5 means: the last week in the month */
        int weekofmonth = compareDate->wDay;
          /* calculate the day of the first DayOfWeek in the month */
        First = ( 6 + compareDate->wDayOfWeek - date->wDayOfWeek + date->wDay 
               ) % 7 + 1;
        limit_day = First + 7 * (weekofmonth - 1);
        /* check needed for the 5th weekday of the month */
        if(limit_day > MonthLengths[date->wMonth==2 && IsLeapYear(date->wYear)]
                [date->wMonth - 1])
            limit_day -= 7;
    }
    else
    {
       limit_day = compareDate->wDay;
    }

    /* convert to seconds */
    limit_day = ((limit_day * 24  + compareDate->wHour) * 60 +
            compareDate->wMinute ) * 60;
    dayinsecs = ((date->wDay * 24  + date->wHour) * 60 +
            date->wMinute ) * 60 + date->wSecond;
    /* and compare */
    return dayinsecs < limit_day ? -1 :
           dayinsecs > limit_day ? 1 :
           0;   /* date is eqaul to the date limit. */
}

/***********************************************************************
 *  TIME_CompTimeZoneID
 *
 *  Computes the local time bias for a given time and time zone
 *
 *  Returns:
 *      TIME_ZONE_ID_INVALID    An error occurred
 *      TIME_ZONE_ID_UNKNOWN    There are no transition time known
 *      TIME_ZONE_ID_STANDARD   Current time is standard time
 *      TIME_ZONE_ID_DAYLIGHT   Current time is dayligh saving time
 */
static BOOL TIME_CompTimeZoneID (
    const TIME_ZONE_INFORMATION *pTZinfo, /* [in] The time zone data. */
    FILETIME      *lpFileTime,            /* [in] The system or local time. */
    BOOL           islocal                /* [in] it is local time */       
    )
{
    int ret;
    BOOL beforeStandardDate, afterDaylightDate;
    DWORD retval = TIME_ZONE_ID_INVALID;
    LONGLONG llTime = 0; /* initialized to prevent gcc complaining */
    SYSTEMTIME SysTime;
    FILETIME ftTemp;

    if (pTZinfo->DaylightDate.wMonth != 0)
    {
        if (pTZinfo->StandardDate.wMonth == 0 ||
            pTZinfo->StandardDate.wDay<1 ||
            pTZinfo->StandardDate.wDay>5 ||
            pTZinfo->DaylightDate.wDay<1 ||
            pTZinfo->DaylightDate.wDay>5)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return TIME_ZONE_ID_INVALID;
        }

        if (!islocal) {
            FILETIME2LL( lpFileTime, llTime );
            llTime -= ( pTZinfo->Bias + pTZinfo->DaylightBias )
                * (LONGLONG)600000000;
            LL2FILETIME( llTime, &ftTemp)
            lpFileTime = &ftTemp;
        }

        FileTimeToSystemTime(lpFileTime, &SysTime);
        
         /* check for daylight saving */
        ret = TIME_DayLightCompareDate( &SysTime, &pTZinfo->StandardDate);
        if (ret == -2)
          return TIME_ZONE_ID_INVALID;

        beforeStandardDate = ret < 0;

        if (!islocal) {
            llTime -= ( pTZinfo->StandardBias - pTZinfo->DaylightBias )
                * (LONGLONG)600000000;
            LL2FILETIME( llTime, &ftTemp)
            FileTimeToSystemTime(lpFileTime, &SysTime);
        }

        ret = TIME_DayLightCompareDate( &SysTime, &pTZinfo->DaylightDate);
        if (ret == -2)
          return TIME_ZONE_ID_INVALID;

        afterDaylightDate = ret >= 0;

        retval = TIME_ZONE_ID_STANDARD;
        if( pTZinfo->DaylightDate.wMonth <  pTZinfo->StandardDate.wMonth ) {
            /* Northern hemisphere */
            if( beforeStandardDate && afterDaylightDate )
                retval = TIME_ZONE_ID_DAYLIGHT;
        } else    /* Down south */
            if( beforeStandardDate || afterDaylightDate )
            retval = TIME_ZONE_ID_DAYLIGHT;
    } else 
        /* No transition date */
        retval = TIME_ZONE_ID_UNKNOWN;
        
    return retval;
}

/***********************************************************************
 *  TIME_TimeZoneID
 *
 *  Calculates whether daylight saving is on now.
 *
 *  Returns:
 *      TIME_ZONE_ID_INVALID    An error occurred
 *      TIME_ZONE_ID_UNKNOWN    There are no transition time known
 *      TIME_ZONE_ID_STANDARD   Current time is standard time
 *      TIME_ZONE_ID_DAYLIGHT   Current time is dayligh saving time
 */
static DWORD TIME_ZoneID(
        const TIME_ZONE_INFORMATION  *pTzi   /* Timezone info */
        )
{
    FILETIME ftTime;
    GetSystemTimeAsFileTime( &ftTime);
    return TIME_CompTimeZoneID( pTzi, &ftTime, FALSE);
}

/***********************************************************************
 *  TIME_GetTimezoneBias
 *
 *  Calculates the local time bias for a given time zone
 *
 * RETURNS
 *
 *  Returns TRUE when the time zone bias was calculated.
 */
static BOOL TIME_GetTimezoneBias(
    const TIME_ZONE_INFORMATION
                  *pTZinfo, /* [in] The time zone data. */
    FILETIME      *lpFileTime,         /* [in] The system or local time. */
    BOOL           islocal,            /* [in] it is local time */       
    LONG          *pBias               /* [out] The calculated bias in minutes */
    )
{
    LONG bias = pTZinfo->Bias;
    DWORD tzid = TIME_CompTimeZoneID( pTZinfo, lpFileTime, islocal);

    if( tzid == TIME_ZONE_ID_INVALID)
        return FALSE;
    if (tzid == TIME_ZONE_ID_DAYLIGHT)
        bias += pTZinfo->DaylightBias;
    else if (tzid == TIME_ZONE_ID_STANDARD)
        bias += pTZinfo->StandardBias;
    *pBias = bias;
    return TRUE;
}


/***********************************************************************
 *              TzSpecificLocalTimeToSystemTime  (KERNEL32.@)
 *
 *  Converts a local time to a time in utc.
 *
 * RETURNS
 *  Success: TRUE. lpUniversalTime contains the converted time
 *  Failure: FALSE.
 */
BOOL WINAPI MyTzSpecificLocalTimeToSystemTime(
    LPTIME_ZONE_INFORMATION lpTimeZoneInformation, /* [in] The desired time zone. */
    LPSYSTEMTIME            lpLocalTime,           /* [in] The local time. */
    LPSYSTEMTIME            lpUniversalTime)       /* [out] The calculated utc time. */
{
    FILETIME ft;
    LONG lBias;
    LONGLONG t;
    TIME_ZONE_INFORMATION tzinfo;

    if (lpTimeZoneInformation != NULL)
    {
        memcpy(&tzinfo, lpTimeZoneInformation, sizeof(TIME_ZONE_INFORMATION));
    }
    else
    {
        if (GetTimeZoneInformation(&tzinfo) == TIME_ZONE_ID_INVALID)
            return FALSE;
    }

    if (!SystemTimeToFileTime(lpLocalTime, &ft))
        return FALSE;
    FILETIME2LL( &ft, t)
    if (!TIME_GetTimezoneBias(&tzinfo, &ft, TRUE, &lBias))
        return FALSE;
    /* convert minutes to 100-nanoseconds-ticks */
    t += (LONGLONG)lBias * 600000000;
    LL2FILETIME( t, &ft)
    return FileTimeToSystemTime(&ft, lpUniversalTime);
}
