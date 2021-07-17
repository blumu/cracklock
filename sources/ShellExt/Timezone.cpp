// Timezone.cpp:
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
#include "Timezone.h"
#include "..\common\tz.h"
#include <strsafe.h>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

LONG EnumerateSubKeys(HKEY hKey, vector<tstring>* o_subkeyArray)
{

    TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
    DWORD    cbName;                   // size of name string 
    TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
    DWORD    cchClassName = MAX_PATH;  // size of class string 
    DWORD    cSubKeys=0;               // number of subkeys 
    DWORD    cbMaxSubKey;              // longest subkey size 
    DWORD    cchMaxClass;              // longest class string 
    DWORD    cValues;              // number of values for key 
    DWORD    cchMaxValue;          // longest value name 
    DWORD    cbMaxValueData;       // longest value data 
    DWORD    cbSecurityDescriptor; // size of security descriptor 
    FILETIME ftLastWriteTime;      // last write time 
 
    DWORD i, retCode; 
 
    //TCHAR  achValue[MAX_VALUE_NAME]; 
    DWORD cchValue = MAX_VALUE_NAME; 
 
    // Get the class name and the value count. 
    retCode = RegQueryInfoKey(
        hKey,                    // key handle 
        achClass,                // buffer for class name 
        &cchClassName,           // size of class string 
        NULL,                    // reserved 
        &cSubKeys,               // number of subkeys 
        &cbMaxSubKey,            // longest subkey size 
        &cchMaxClass,            // longest class string 
        &cValues,                // number of values for this key 
        &cchMaxValue,            // longest value name 
        &cbMaxValueData,         // longest value data 
        &cbSecurityDescriptor,   // security descriptor 
        &ftLastWriteTime);       // last write time 
 
    // Enumerate the subkeys, until RegEnumKeyEx fails.
    
    if (cSubKeys) {

        for (i=0; i<cSubKeys; i++) 
        { 
            cbName = MAX_KEY_LENGTH;
            retCode = RegEnumKeyEx(hKey, i,
                     achKey, 
                     &cbName, 
                     NULL, 
                     NULL, 
                     NULL, 
                     &ftLastWriteTime); 
            if (retCode == ERROR_SUCCESS)  {
                o_subkeyArray->push_back(tstring(achKey));

            }
        }
    } 

    return 0;
}





LONG EnumerateKeyValueNames(HKEY hKey, vector<tstring>* o_keyValueNamesArray)
{
    // Enumerate the key values.
   // TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
   // DWORD    cbName;                   // size of name string 
    TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
    DWORD    cchClassName = MAX_PATH;  // size of class string 
    DWORD    cSubKeys=0;               // number of subkeys 
    DWORD    cbMaxSubKey;              // longest subkey size 
    DWORD    cchMaxClass;              // longest class string 
    DWORD    cValues;              // number of values for key 
    DWORD    cchMaxValue;          // longest value name 
    DWORD    cbMaxValueData;       // longest value data 
    DWORD    cbSecurityDescriptor; // size of security descriptor 
    FILETIME ftLastWriteTime;      // last write time 
 
    DWORD i, retCode; 
 
    TCHAR  achValue[MAX_VALUE_NAME]; 
    DWORD cchValue = MAX_VALUE_NAME; 
 
    // Get the class name and the value count. 
    retCode = RegQueryInfoKey(
        hKey,                    // key handle 
        achClass,                // buffer for class name 
        &cchClassName,           // size of class string 
        NULL,                    // reserved 
        &cSubKeys,               // number of subkeys 
        &cbMaxSubKey,            // longest subkey size 
        &cchMaxClass,            // longest class string 
        &cValues,                // number of values for this key 
        &cchMaxValue,            // longest value name 
        &cbMaxValueData,         // longest value data 
        &cbSecurityDescriptor,   // security descriptor 
        &ftLastWriteTime);       // last write time 
 

    if (cValues)  {

        for (i=0, retCode=ERROR_SUCCESS; i<cValues; i++)  { 
            cchValue = MAX_VALUE_NAME; 
            achValue[0] = '\0'; 
            retCode = RegEnumValue(hKey, i, 
                achValue, 
                &cchValue, 
                NULL, 
                NULL,
                NULL,
                NULL);
 
            if (retCode == ERROR_SUCCESS ) 
            { 
                o_keyValueNamesArray->push_back(tstring(achValue));
            } 
        }
    }

    return 0;
}



///Global functions///////////////////////////////////////////////
bool TimeZoneComparer(CRegTimeZoneInfo *pTZ1, CRegTimeZoneInfo *pTZ2)
{
    return ( pTZ1->m_regTZI.Bias > pTZ2->m_regTZI.Bias)
		|| (( pTZ1->m_regTZI.Bias == pTZ2->m_regTZI.Bias)&& (_tcscmp(pTZ1->m_szStd, pTZ2->m_szStd) > 0)); 
}



///////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTimeZoneInfoManager::CTimeZoneInfoManager()
{
    EnumerateTimeZones();
}



CTimeZoneInfoManager::~CTimeZoneInfoManager()
{
	//cleanup
	for (unsigned int cnt = 0; cnt < m_arrRegTimeZoneInfo.size(); cnt++)
	{
		CRegTimeZoneInfo* pobjRegTimeZoneInfo = m_arrRegTimeZoneInfo[cnt];
		delete pobjRegTimeZoneInfo;
		pobjRegTimeZoneInfo = NULL;
	}

}



/*
1. Open the registry key : "SOFTWARE\Microsoft\Windows NT\CurrentVersion\Time Zones
2. Read all subkeys
3. Fill all the TimeZone Structures.
*/
int CTimeZoneInfoManager::EnumerateTimeZones()
{

    //1.Open the registry key "SOFTWARE\Microsoft\Windows NT\CurrentVersion\Time Zones"
    TCHAR szTimeZoneKey[] = _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones");

    HKEY hTimeZones;
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, szTimeZoneKey, 0, KEY_READ, &hTimeZones);

    //2.Read all subkeys
    EnumerateSubKeys(hTimeZones, &m_arrTimeZones);

    //3. Fill all the TimeZone Structures.
    GetFullTimeZoneInfoFromRegistry();

    sort(m_arrRegTimeZoneInfo.begin(),m_arrRegTimeZoneInfo.end(), TimeZoneComparer);

    return 0;
}




int CTimeZoneInfoManager::GetFullTimeZoneInfoFromRegistry() 
{
    TCHAR szParentTimeZoneKey[] = _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones");

    //Prepare key
    tstring szChildTimeZoneKey;

    for (unsigned int cnt = 0; cnt < m_arrTimeZones.size(); cnt++)
    {
        //open child key
        HKEY hTimeZoneKey;
        RegOpenKeyEx(HKEY_LOCAL_MACHINE, (szParentTimeZoneKey + tstring(_T("\\")) + m_arrTimeZones[cnt]).c_str(), 0, KEY_READ, &hTimeZoneKey);

        //enumreate all value names
		vector<tstring> keyValueNamesArray;
        EnumerateKeyValueNames(hTimeZoneKey, &keyValueNamesArray);

        DWORD valueType, valueSize;
        BYTE value[512];
        
        CRegTimeZoneInfo* pobjRegTimeZoneInfo = new CRegTimeZoneInfo;
        ZeroMemory(pobjRegTimeZoneInfo, sizeof(CRegTimeZoneInfo));
        ZeroMemory((BYTE*)&pobjRegTimeZoneInfo->m_regTZI , sizeof(regTZI));

        for (unsigned int valname_cnt = 0; valname_cnt < keyValueNamesArray.size(); valname_cnt++)
        {
            if (! _tcsicmp(keyValueNamesArray[valname_cnt].c_str(),_T("TZI")))
            {
                valueSize = sizeof(regTZI);
            }
            else
            {
                valueSize = 512;
            }

             //now get the values and fill the structures
            RegQueryValueEx(hTimeZoneKey, keyValueNamesArray[valname_cnt].c_str(), NULL,
                                        &valueType, value, &valueSize);              

			if ( !_tcsicmp( keyValueNamesArray[valname_cnt].c_str(), _T("Display")))
				memcpy(pobjRegTimeZoneInfo->m_szDisplay, value, valueSize);

			if (!_tcsicmp( keyValueNamesArray[valname_cnt].c_str(), _T("Dlt")))
				memcpy(pobjRegTimeZoneInfo->m_szDlt, value, valueSize);

			if (!_tcsicmp( keyValueNamesArray[valname_cnt].c_str(), _T("Std")))
				memcpy(pobjRegTimeZoneInfo->m_szStd, value, valueSize);

			if (!_tcsicmp( keyValueNamesArray[valname_cnt].c_str(), _T("MapID")))
				memcpy(pobjRegTimeZoneInfo->m_szMapID, value, valueSize);

			if (!_tcsicmp( keyValueNamesArray[valname_cnt].c_str(), _T("Index")))
				memcpy((BYTE*)&pobjRegTimeZoneInfo->m_iIndex, value, valueSize);

			if (!_tcsicmp( keyValueNamesArray[valname_cnt].c_str(), _T("TZI")))
				memcpy((BYTE*)&pobjRegTimeZoneInfo->m_regTZI, value, valueSize);


            
        }
		if( _tcscmp(pobjRegTimeZoneInfo->m_szDisplay, _T("")) )
			m_arrRegTimeZoneInfo.push_back(pobjRegTimeZoneInfo);

    }

    return 0;
}

void CTimeZoneInfoManager::GetTimeZoneInformationFromIndex(int index, TIME_ZONE_INFORMATION *ptzi)
{
	CRegTimeZoneInfo* pRegTimeZoneInfo = m_arrRegTimeZoneInfo[index];

    ZeroMemory(ptzi, sizeof(TIME_ZONE_INFORMATION));
    ptzi->Bias            = pRegTimeZoneInfo->m_regTZI.Bias;
    ptzi->DaylightBias    = pRegTimeZoneInfo->m_regTZI.DaylightBias;
    ptzi->DaylightDate    = pRegTimeZoneInfo->m_regTZI.DaylightDate;
    ptzi->StandardBias    = pRegTimeZoneInfo->m_regTZI.StandardBias;
    ptzi->StandardDate    = pRegTimeZoneInfo->m_regTZI.StandardDate;
#ifndef UNICODE
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pRegTimeZoneInfo->m_szStd , -1, tzi.StandardName, 32);
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pRegTimeZoneInfo->m_szDlt , -1, tzi.DaylightName , 32);
#else
	wcscpy_s(ptzi->StandardName, _countof(ptzi->StandardName), pRegTimeZoneInfo->m_szStd);
	wcscpy_s(ptzi->DaylightName, _countof(ptzi->DaylightName), pRegTimeZoneInfo->m_szDlt);
#endif
}

int CTimeZoneInfoManager::GetTimeZoneIndex(TIME_ZONE_INFORMATION *ptzi)
{
	TCHAR standardName[512];
	TCHAR dayLightName[512];

#ifndef UNICODE
	WideCharToMultiByte( CP_ACP, 0, tzi.StandardName, -1,standardName, 256, NULL, NULL );
	WideCharToMultiByte( CP_ACP, 0, tzi.DaylightName, -1,dayLightName, 256, NULL, NULL );
#else
	wcscpy_s(standardName, _countof(ptzi->StandardName), ptzi->StandardName);
	wcscpy_s(dayLightName, _countof(ptzi->DaylightName), ptzi->DaylightName);
#endif   

    for (unsigned int index = 0; index < m_arrRegTimeZoneInfo.size(); index++ )
    {
        CRegTimeZoneInfo* pRegTimeZoneInfo = m_arrRegTimeZoneInfo[index];
        if ( pRegTimeZoneInfo->m_regTZI.Bias == ptzi->Bias &&
             pRegTimeZoneInfo->m_regTZI.StandardBias == ptzi->StandardBias &&
             (!(_tcscmp(pRegTimeZoneInfo->m_szStd, standardName)))/* &&
             (!(_tcscmp(pRegTimeZoneInfo->m_szDlt, dayLightName)))*/
           )
        {
            return index;
        }
    }
    return 0;
}

int CTimeZoneInfoManager::GetCurrentTimeZoneIndex()
{
    TIME_ZONE_INFORMATION tzi;
    GetTimeZoneInformation(&tzi);
	return GetTimeZoneIndex(&tzi);
}

int CTimeZoneInfoManager::ConvertFromLocalToUTC(SYSTEMTIME* i_LocalTime, int i_TimeZoneIndex, SYSTEMTIME* o_UTCTime)
{
    //get timezone infor from index
    TIME_ZONE_INFORMATION tzi;
	GetTimeZoneInformationFromIndex(i_TimeZoneIndex, &tzi);
     //Change CurrentTimeZone
    MyTzSpecificLocalTimeToSystemTime(&tzi,i_LocalTime,o_UTCTime);   
    return 0;
}

int CTimeZoneInfoManager::ConvertFromUTCToLocal(SYSTEMTIME* i_UTCTime, int i_TimeZoneIndex, SYSTEMTIME* o_localTime)
{
    //get timezone infor from index
    TIME_ZONE_INFORMATION tzi;
	GetTimeZoneInformationFromIndex(i_TimeZoneIndex, &tzi);
   
    if (!SystemTimeToTzSpecificLocalTime(&tzi, i_UTCTime, o_localTime))
    {

    }
    return 0;

}


/******************************************** CRegTimeZoneInfo *********************************************/
