// TimeZone.h
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>
#include <algorithm>

using namespace std;
#ifdef UNICODE
    typedef wstring tstring;
#else
    typedef string tstring;
#endif



#define MAX_SIZE    255

struct regTZI
{
    long Bias;
    long StandardBias;
    long DaylightBias;
    SYSTEMTIME StandardDate; 
    SYSTEMTIME DaylightDate;
};


class CRegTimeZoneInfo
{
public:
    TCHAR tcName[MAX_SIZE];
    TCHAR m_szDisplay[MAX_SIZE];
    TCHAR m_szDlt[MAX_SIZE];
    TCHAR m_szStd[MAX_SIZE];
    TCHAR m_szMapID[MAX_SIZE];
    DWORD m_iIndex;
    DWORD ActiveTimeBias;
    regTZI m_regTZI;
};

class CTimeZoneInfoManager  
{
public:
    CTimeZoneInfoManager();
    virtual ~CTimeZoneInfoManager();

//operations
    int EnumerateTimeZones();
    int GetFullTimeZoneInfoFromRegistry();
    int ConvertFromLocalToUTC(SYSTEMTIME* i_LocalTime, int i_TimeZoneIndex, SYSTEMTIME* o_UTCTime);
    int ConvertFromUTCToLocal(SYSTEMTIME* i_UTCTime, int i_TimeZoneIndex, SYSTEMTIME* o_LocalTime);
	void GetTimeZoneInformationFromIndex(int index, TIME_ZONE_INFORMATION *ptzi);
	int GetTimeZoneIndex(TIME_ZONE_INFORMATION *ptzi);
	int GetCurrentTimeZoneIndex();

//attributes
public:
    std::vector <tstring>	          m_arrTimeZones;
    std::vector <CRegTimeZoneInfo*> m_arrRegTimeZoneInfo;

};

