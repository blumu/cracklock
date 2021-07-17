//==================================
// William BLUM 1998
// KERNEL.CPP
//==================================
#include <windows.h>
#include <memory.h>
#include <algorithm>
#include <vector>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"		    // Resources
#include "..\Common\Common.h"	// Outils communs
#include "..\Common\tz.h"
#include "Kernel.h"
#include "Hook.h"			// Fonctions d'interception des appels aux APIs Win32
#include "Debug.h"			// Fonctions de débugage
 
using namespace std ;

// Table des interceptions des fonctions systèmes
HOOKENTRY g_hooktable[] = {	
	("GetLocalTime"),				(FARPROC)&GetLocalTimeCRK,
	("GetSystemTime"),			    (FARPROC)&GetSystemTimeCRK,
	("GetSystemTimeAsFileTime"),	(FARPROC)&GetSystemTimeAsFileTimeCRK,
	("LoadLibraryA"),				(FARPROC)&LoadLibraryACRK,
	("LoadLibraryExA"),			    (FARPROC)&LoadLibraryExACRK,
	("LoadLibraryW"),				(FARPROC)&LoadLibraryWCRK,
	("LoadLibraryExW"),			    (FARPROC)&LoadLibraryExWCRK,
	("FreeLibrary"),				(FARPROC)&FreeLibraryCRK,
	("FreeLibraryAndExitThread"),	(FARPROC)&FreeLibraryAndExitThreadCRK,
	("GetModuleHandleA"),			(FARPROC)&GetModuleHandleACRK,
	("GetModuleHandleW"),			(FARPROC)&GetModuleHandleWCRK,
	("GetModuleFileNameA"),		    (FARPROC)&GetModuleFileNameACRK,
	("GetModuleFileNameW"),		    (FARPROC)&GetModuleFileNameWCRK,
	("GetTimeZoneInformation"),	    (FARPROC)&GetTimeZoneInformationCRK,
	("GetProcAddress"),			    (FARPROC)&GetProcAddressCRK
};

// Nombre de hooks dans la table de hooks
int					g_nbHooks = NB_HOOKENTRY(g_hooktable);

HMODULE				g_hModule;			// Module de la dll
BOOL				g_bChicago = FALSE; // Vrai si c'est Win95
SYSTEM_INFO			g_si;				// Informations systèmes
DWORD				g_dwPlatformId;		// Mot de version de Windows

vector<HINSTANCE> lstModule;	// handles des modules chargé par le process courrant


// Mode de crackage
enum CrackMode
{
	None,
	Normal,
	Constant,
	Systemwide
} crkmode;

AppSettings g_appset; // settings de l'application lus depuis la base de registre

// Différence à maintenir entre la fausse et la vrai date
// pour le mode normal et le mode systemwide
FILETIME	g_ftDelta;

// Date/heure falsifiées retournées par les fonctions
// GetSystemTimeCrk et GetLocalTimeCrk
SYSTEMTIME  g_stFake;

// Handle du hook windows
HHOOK		g_hHook = NULL;

// Soustrait deux dates aux formats FILETIME
#define SubstractFileTime(res, op1, op2)	*(__int64 *)res = *(__int64 *)op1 - *(__int64 *)op2

LRESULT CALLBACK HookFunction( int nCode, WPARAM wParam, LPARAM lParam )
{
	return CallNextHookEx(g_hHook, nCode, wParam, lParam);	
}

extern "C" BOOL WINAPI CracklockInit(void)
{
	if( !g_hHook )
	{
		g_hHook = SetWindowsHookEx(WH_CBT, &HookFunction, g_hModule, 0);
	    return (g_hHook != 0);
	}

    return FALSE;
}

extern "C" BOOL WINAPI CracklockDeinit(void)
{
	if( g_hHook )
		return UnhookWindowsHookEx(g_hHook);

    return FALSE;
}



BOOL APIENTRY DllMain (HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch( ul_reason_for_call )
	{
		case DLL_PROCESS_ATTACH:
		{
			g_hModule = hModule;

			// Infos sur la version de Windows
			OSVERSIONINFO versInfo;	
			versInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
			GetVersionEx(&versInfo);
			g_dwPlatformId = versInfo.dwPlatformId;

			// Obtient le nom de l'exécutable
			TCHAR AppFileName[_MAX_PATH];
			if ( !GetModuleFileName(NULL, AppFileName, _countof(AppFileName)) )
				return 0;

			// Charge la config à partir de la registry
            InitCommon(hModule);
            if( g_appset.Load( AppFileName ) ) {
                // &dwFlags
				// Initialisation des variables
				memset(&g_ftDelta, 0, sizeof(FILETIME));

				// Obtient les infos systèmes
				GetSystemInfo(&g_si);

				// Formate la date desirée sous la forme d'une structure SYSTEMTIME	
                FileTimeToSystemTime(&g_appset.m_ftDateTime, &g_stFake);

                if( !g_appset.m_bActive || !(g_appset.m_bTimeMode || g_appset.m_bDateMode || g_appset.m_bTimeZoneMode) )
					crkmode = None;
				else
				{
					// ************************
					// *   Mode Systemwide    *
					// ************************
					//
                    SYSTEMTIME	STTemp;
					if( g_appset.m_bOptSystem )
					{
						crkmode = Systemwide;
						// Interroge l'horloge de la machine (LOCAL)
						GetLocalTime(&STTemp);
						SystemTimeToFileTime(&STTemp, &g_ftDelta);

						// Calcule le décalage (= date désirée - date horloge)
						SubstractFileTime (&g_ftDelta, &g_appset.m_ftDateTime, &g_ftDelta);			

						// Met à jour l'horloge de la machine (LOCAL)
						FileTimeToSystemTime(&g_appset.m_ftDateTime, &STTemp);
						
					  // modifie la date du systeme (2 appels obligatoire, voir doc msdn)
						SetLocalTime(&STTemp); // daylight
						SetLocalTime(&STTemp); // time
					}

					// ************************
					// *   Mode Constant      *
					// ***********************
					//
					else if( g_appset.m_bOptConstant )
					{	
						crkmode = Constant;
					}

					// ************************
					// *     Mode Normal      *
					// ************************
					//	
					else
					{		
						crkmode = Normal;
						// Interroge l'horloge de la machine (Local)
						GetLocalTime(&STTemp);
						SystemTimeToFileTime(&STTemp, &g_ftDelta);
						// Calcule le décalage (= date horloge - date désirée)
						SubstractFileTime (&g_ftDelta, &g_ftDelta, &g_appset.m_ftDateTime);
					}

					// Remplace dans tous les modules chargés toutes les références au fonctions crackées 
					EnumLoadedModule((HMODULE)-1);
				}
			}
			else // rien trouvé dans la registry
			{	
				TCHAR szAppFileName [_MAX_PATH];
				TCHAR *pszAppName;

				GetModuleFileName(NULL, (PTSTR)&szAppFileName, _MAX_PATH);
				pszAppName = _tcsrchr(szAppFileName, _T('\\')) + 1;
				if( pszAppName && _tcsicmp(pszAppName, CL_INJECTOR_EXE) )
				{
					// aucun crackage
					crkmode = None;
				}
				else
				{
					if( g_dwPlatformId == VER_PLATFORM_WIN32_NT)
						FreeLibrary(hModule);
				}			

			/*
				// si l'application appellante est CL_INJECTOR_EXE ...
				HANDLE hSemaphore = CreateSemaphore(NULL, 0, 2, CL_INJECTOR_EXE);
				if( GetLastError() == ERROR_ALREADY_EXISTS )
				{
					CloseHandle(hSemaphore);

					// aucun crackage
					crkmode = None;
					g_appset.m_bTimeMode = FALSE;
					g_appset.m_bDateMode = FALSE;
				}
				else
				{
					// sinon libère la dll de la mémoire
					CloseHandle(hSemaphore);

					if( g_dwPlatformId == VER_PLATFORM_WIN32_NT)
						FreeLibrary(hModule);
					//else
						//return FALSE;

				}			
*/
			}

		}
		break;

		case DLL_PROCESS_DETACH:
		{
			SYSTEMTIME	STTemp;
			FILETIME	FTTemp;
			// ************************
			// *   Mode Systemwide    *
			// ************************
			//
			if( crkmode == Systemwide )
			{
				// Interroge l'horloge de la machine (UTC)
				GetLocalTime(&STTemp);
				SystemTimeToFileTime(&STTemp, &FTTemp);

				// Décale la date (= date courrante - décalage)
				SubstractFileTime (&FTTemp, &FTTemp, &g_ftDelta);

				// Met à jour l'horloge de la machine (UTC)
				FileTimeToSystemTime(&FTTemp, &STTemp);
				SetLocalTime(&STTemp);
			}
		}	
		break;

		case DLL_THREAD_ATTACH:		
		case DLL_THREAD_DETACH:
			break;
	}	
	return TRUE;
}

//////////////////////////////////////////
//////////////////////////////////////////
///										//
///			Cracked Functions			//
///										//
//////////////////////////////////////////
//////////////////////////////////////////



///////////////////////////
//
// - 1 - Fonctions de détournement de la date et de l'heure
//
//	* DWORD WINAPI GetTimeZoneInformationCRK(LPTIME_ZONE_INFORMATION lpTimeZoneInformation)
//	* VOID WINAPI GetSystemTimeCRK (LPSYSTEMTIME lpSystemTime)
//	* VOID WINAPI GetLocalTimeCRK (LPSYSTEMTIME lpLocalTime)
//	* VOID WINAPI GetSystemTimeAsFileTimeCRK (LPFILETIME lpSystemTimeAsFileTime)
//

// Fonction qui convertit des dates relatives en date absolues
//
// Informations : 1 Les dates absolues comprennent le mois, le jour, l'année.
//				  2 Les dates relatives comprennet le mois, le numéro de la semaine (wDay),
//					le jour de la semaine (wDayOfWeek), mais pas l'année
//
void ConvertitDateRelativeEnAbsolue(LPSYSTEMTIME pstTime, WORD wYear)
{
	// Vérifie que c'est bien une date relative
	if( pstTime->wYear == 0 )
	{
		int	cDay[5], nbMonths, iWeek, iTestDay, iFirstDay;

		// nombre de jours dans le mois
		nbMonths = DaysInMonth(pstTime->wMonth, wYear);
		// numéro du jour commençant le mois (0 pour Dimanche, 1 pour Lundi, etc...)
		iFirstDay = CalcFirstDay(pstTime->wMonth, wYear, 6);

		// Trouve les 4 ou 5 numéros de jours dans le mois correspondant au jour de la semaine demandé (pstTime->wDayOfWeek)
		iWeek = -1;
		iTestDay = (pstTime->wDayOfWeek - iFirstDay + 1 + NB_DAYSINWEEK) % NB_DAYSINWEEK;
		while( (iWeek<5) && (iTestDay<=nbMonths) )
		{			
			cDay[++iWeek] = iTestDay;
			iTestDay += NB_DAYSINWEEK;
		}
		
		// Si on a trouvé le jours que dans 4 semaines, alors le jour de
		// la 5ème semaine (=dernière semaine) sera éqale au jour de la 4ème
		if( iWeek < 4 )
			cDay[4] = cDay[3];
		
		// pstTime->wDay contient le numéro de la semaine demandée
		// (sachant que 5 correspond à la dernière semaine)
		pstTime->wDay = cDay[pstTime->wDay-1];
		pstTime->wYear = wYear;		
	}
}


// Function:		Renvoi la zone de temps dans laquelle se trouve l'heure crackée
//
// Le problem ici est de mimer le fonctionnment de GetTimeZoneInformation.
// Cette fonction doit retourner une valeur indiquant dans quelle time zone on se trouve. Comme la date courrante est changee par Cracklock, on doit faire ici le calcul nous meme pour savoir ce qu'il faut retourner.
//
// Algorithme:
//
//		1	On determine les dates des prochains changements d'heure
//			(il y a un chgm pour Standard et un autre pour Daylight Saving)
//
//		2	On determine le changement de date le plus proche (Standard ou Daylight)
//
//		3	Si le changement le plus proche est Standard alors, on se
//			trouve dans la zone Daylight sinon, on se trouve dans la zone
//			de temps Standard
//
DWORD WINAPI GetTimeZoneInformationCRK (LPTIME_ZONE_INFORMATION lpTimeZoneInformation)
{
	#if _DEBUG
		Debug_Print(IDC_LST, _T("GetTimeZoneInformationCRK(lpTimeZoneInformation:%08X)"), lpTimeZoneInformation);
	#endif
	
	SYSTEMTIME stCrk;
	FILETIME ftCrk, ftStd, ftDlt;
	DWORD	dwRet;

	//////////
	// Phase 1 Calcul des prochains changements d'heures
	//
	GetSystemTimeCRK(&stCrk);
	SystemTimeToFileTime(&stCrk, &ftCrk);
	
	// Si la timezone est virtualisee par Cracklock alors retourne celle qui a ete selectionnee par l'utilisateur
    if( g_appset.m_bTimeZoneMode )
        memcpy(lpTimeZoneInformation, &g_appset.m_tzi, sizeof(TIME_ZONE_INFORMATION));
	// sinon utilise la timezone du systeme
	else
		dwRet = GetTimeZoneInformation(lpTimeZoneInformation);

    if( g_appset.m_tzi.DaylightDate.wMonth == 0 || g_appset.m_tzi.StandardDate.wMonth == 0 ) 
        return TIME_ZONE_ID_UNKNOWN;


	ConvertitDateRelativeEnAbsolue(&lpTimeZoneInformation->StandardDate, stCrk.wYear);
	ConvertitDateRelativeEnAbsolue(&lpTimeZoneInformation->DaylightDate, stCrk.wYear);

	SystemTimeToFileTime(&lpTimeZoneInformation->StandardDate, &ftStd);
	SystemTimeToFileTime(&lpTimeZoneInformation->DaylightDate, &ftDlt);

	// Si le changement de cette année est déjà passé alors incréménte l'année
	if( CompareFileTime(&ftStd, &ftCrk) != 1 )
		lpTimeZoneInformation->StandardDate.wYear++;

	// idem
	if( CompareFileTime(&ftDlt, &ftCrk) != 1 )
		lpTimeZoneInformation->DaylightDate.wYear++;
	
	//////////
	// Phase 2 Quel sera le prochain changement d'heure ?
	// et Phase 3 Quel est la zone de temps actuelle ?
	//
	SystemTimeToFileTime(&lpTimeZoneInformation->StandardDate, &ftStd);
	SystemTimeToFileTime(&lpTimeZoneInformation->DaylightDate, &ftDlt);
	switch( CompareFileTime(&ftStd, &ftDlt) )
	{
		// Zone courrante = DayLight	
		case -1:
			dwRet = TIME_ZONE_ID_DAYLIGHT;
			break;
		// Zone courrante = Standard
		case 0:
		case 1:
			dwRet = TIME_ZONE_ID_STANDARD;
			break;
	}

	return dwRet;
}

// Charge les champs date et/ou heure avec leur valeur réelle actuelle
// si cette option a ete configuree dans cracklock manager
inline void FillWithRealDateOrTime(LPSYSTEMTIME pst, LPSYSTEMTIME pstReal)
{	
	if( !g_appset.m_bTimeMode )
	{
		pst->wHour = pstReal->wHour;
		pst->wMilliseconds = pstReal->wMilliseconds;
		pst->wMinute = pstReal->wMinute;
		pst->wSecond = pstReal->wSecond;
	}
	if( !g_appset.m_bDateMode )
	{
		pst->wDay = pstReal->wDay;
		pst->wDayOfWeek = pstReal->wDayOfWeek;
		pst->wMonth = pstReal->wMonth;
		pst->wYear = pstReal->wYear;
	}
}

VOID WINAPI GetSystemTimeCRK (LPSYSTEMTIME lpSystemTime)
{
	#ifdef _DEBUG
		Debug_Print(IDC_LST, _T("GetSystemTimeCRK(...)"));
	#endif

	// On obtient la date locale et on la convertit en date system en utilisant la timezone courrante (ceci est le meilleur moyen d'eviter
	// le casse tete des conversions manuelles des dates).
	SYSTEMTIME stloc;
	GetLocalTimeCRK(&stloc);
	BOOL bret = MyTzSpecificLocalTimeToSystemTime( g_appset.m_bTimeZoneMode ? &g_appset.m_tzi : NULL, &stloc, lpSystemTime);
	return;
}

VOID WINAPI GetSystemTimeAsFileTimeCRK (LPFILETIME lpSystemTimeAsFileTime)
{
	#ifdef _DEBUG
		Debug_Print(IDC_LST, _T("GetSystemTimeAsFileTimeCRK(...)"));
	#endif

	SYSTEMTIME st;

	GetSystemTimeCRK(&st);
	SystemTimeToFileTime(&st, lpSystemTimeAsFileTime);
}


VOID WINAPI GetLocalTimeCRK (LPSYSTEMTIME lpLocalTime)
{
	#ifdef _DEBUG
		Debug_Print(IDC_LST, _T("GetLocalTimeCRK(...)"));
	#endif
	
	SYSTEMTIME	stRealLoc;

	// Interroge l'horloge de la machine (LOC)
	GetLocalTime( &stRealLoc );

	// Mode normal -> effectue le décalage
	if( crkmode == Normal )
	{
        if( g_appset.m_bTimeMode || g_appset.m_bDateMode ) {
			// Décale le temps à retourner
			FILETIME	ftTmp;
			SystemTimeToFileTime( &stRealLoc, &ftTmp );
			SubstractFileTime(&ftTmp, &ftTmp, &g_ftDelta);
			FileTimeToSystemTime (&ftTmp, lpLocalTime);

			// charge la date et/ou l'heure réelle si celà a été specifié dans la configuration
			FillWithRealDateOrTime(lpLocalTime, &stRealLoc);
		}
        else {
    		memcpy(lpLocalTime, &stRealLoc, sizeof(SYSTEMTIME));
        }

		// Si la timezone est virtualisee et que l'heure n'est pas virtualisee alors
		// effectue le calcul de changement de fuseau d'horaire.
        if( g_appset.m_bTimeZoneMode && !g_appset.m_bTimeMode ) {
			SYSTEMTIME stRealUniv;
			MyTzSpecificLocalTimeToSystemTime(NULL,lpLocalTime,&stRealUniv);
			SystemTimeToTzSpecificLocalTime(&g_appset.m_tzi,&stRealUniv,lpLocalTime);
		}
	}		
	else if ( crkmode == Constant)
	{
		memcpy(lpLocalTime, &g_stFake, sizeof(SYSTEMTIME));

		// charge la date et/ou l'heure réelle quand celà a été configuré dans le manager
		FillWithRealDateOrTime(lpLocalTime, &stRealLoc);
	}	
	else if( crkmode==None || crkmode==Systemwide ) {
		memcpy(lpLocalTime, &stRealLoc, sizeof(SYSTEMTIME));
	}

}

//
///////////////////////////


///////////////////////////
//
// - 2 - Fonctions qui programment les interceptions
//		 aux appels de fonctions systèmes dans les DLLs chargés lors
//		 de l'exécution du programme
//
//	* HMODULE WINAPI LoadLibraryACRK(LPCSTR lpLibFileName)
//	* HMODULE WINAPI LoadLibraryWCRK(LPCWSTR lpLibFileName)
//	* HMODULE WINAPI LoadLibraryExWCRK(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
//	* HMODULE WINAPI LoadLibraryExACRK(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
//

HMODULE WINAPI LoadLibraryACRK(LPCSTR lpLibFileName)
{
	#if _DEBUG
		Debug_Print(IDC_LST, _T("LoadLibraryACRK(lpLibFileName: %hs)"), dump_str(lpLibFileName));
		Debug_EnumLoadedModule();
	#endif

	HMODULE hRet = LoadLibraryA(lpLibFileName);

	EnumLoadedModule(hRet);
	
	return hRet;
}

HMODULE WINAPI LoadLibraryWCRK(LPCWSTR lpLibFileName)
{
	#ifdef _DEBUG
		Debug_Print(IDC_LST, _T("LoadLibraryWCRK(lpLibFileName: %hl)"), dump_str(lpLibFileName));
		Debug_EnumLoadedModule();
	#endif

	HMODULE hRet = LoadLibraryW(lpLibFileName);

	EnumLoadedModule(hRet);
	return hRet;
}

HMODULE WINAPI LoadLibraryExACRK(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
	#ifdef _DEBUG
		Debug_Print(IDC_LST, _T("LoadLibraryExACRK(lpLibFileName: %hs, hFile: %X, dwFlags: %X)"), dump_str(lpLibFileName), hFile, dwFlags);
		Debug_EnumLoadedModule();
	#endif

	HMODULE hRet = LoadLibraryExA(lpLibFileName, hFile, dwFlags);

	EnumLoadedModule(hRet);
	return hRet;
}

HMODULE WINAPI LoadLibraryExWCRK(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
	#ifdef _DEBUG
		Debug_Print(IDC_LST, _T("LoadLibraryExWCRK(lpLibFileName: %hl, hFile: %X, dwFlags: %X)"), dump_str(lpLibFileName), hFile, dwFlags);
		Debug_EnumLoadedModule();
	#endif

	HMODULE hRet = LoadLibraryExW(lpLibFileName, hFile, dwFlags);

	EnumLoadedModule(hRet);
	return hRet;
}

BOOL WINAPI FreeLibraryCRK( HMODULE hLibModule )
{
	BOOL bRet = FreeLibrary(hLibModule);

	if( bRet )
	{
		vector<HINSTANCE>::iterator location;

		location = find(lstModule.begin(), lstModule.end(), (HINSTANCE)hLibModule);

		if( location != lstModule.end() )
			lstModule.erase(location);
	}

	#ifdef _DEBUG
		Debug_Print(IDC_LST, _T("FreeLibraryCRK(hLibModule: %X)"), hLibModule);
		Debug_EnumLoadedModule();
	#endif

	return bRet;
}

void WINAPI FreeLibraryAndExitThreadCRK( HMODULE hLibModule, DWORD dwExitCode )
{
	vector<HINSTANCE>::iterator location;

	location = find(lstModule.begin(), lstModule.end(), (HINSTANCE)hLibModule);

	if( location != lstModule.end() )
		lstModule.erase(location);

	#ifdef _DEBUG
		Debug_Print(IDC_LST, _T("FreeLibraryAndExitThreadCRK(hLibModule: %X, dwExitCode: %X)"), hLibModule, dwExitCode);
		Debug_EnumLoadedModule();
	#endif

	FreeLibraryAndExitThread(hLibModule, dwExitCode);
}


///////////////////////////
//
// - 3 - Fonction qui intercepte les appels indirects aux fonctions
//       du systèmes
//
//	* FARPROC WINAPI GetProcAddressCRK( HMODULE hModule, LPCSTR lpProcName )
//

FARPROC WINAPI GetProcAddressCRK( HMODULE hModule, LPCSTR lpProcName )
{
	int i;
		
	// obtention de l'adresse d'une fonction par son nom ?
	if( HIWORD(lpProcName) != 0)
	{
		#ifdef _DEBUG
			Debug_PrintA(IDC_LST, "GetProcAddressCRK(hModule: %X, lpProcName: %s)", hModule, dump_str(lpProcName));
		#endif
		
		for(i=0; i<NB_HOOKENTRY(g_hooktable); i++)
		{
			if( _stricmp(g_hooktable[i].pFuncName, lpProcName) == 0 )
				return g_hooktable[i].pHookFunc;
		}
	}
	else // obtention de l'adresse d'une fonction par son numéro
	{
		#ifdef _DEBUG
			Debug_Print(IDC_LST, _T("GetProcAddressCRK(hModule: %X, lpProcName: %u)"), hModule, LOWORD(lpProcName));
		#endif
	}

	return GetProcAddress( hModule, lpProcName );
}

//
///////////////////////////

///////////////////////////
//
// - 5 - Fonctions qui permettent la non-detection de Cracklock 
//
//	* HMODULE WINAPI GetModuleHandleACRK(LPCSTR lpModuleName)
//	* HMODULE WINAPI GetModuleHandleWCRK(LPCWSTR lpModuleName)
//	* DWORD WINAPI GetModuleFileNameACRK(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
//	* DWORD WINAPI GetModuleFileNameWCRK(HMODULE hModule, LPWSTR lpFilename, DWORD nSize)
//

HMODULE WINAPI GetModuleHandleACRK(LPCSTR lpModuleName)
{
	#if _DEBUG
		Debug_PrintA(IDC_LST, "GetModuleHandleACRK(lpModuleName: %s)", dump_str(lpModuleName));
	#endif

	HMODULE hModule = GetModuleHandleA(lpModuleName);

	if( hModule == g_hModule)
		return NULL;
	else
		return hModule;
}

HMODULE WINAPI GetModuleHandleWCRK(LPCWSTR lpModuleName)
{
	#if _DEBUG
		Debug_PrintW(IDC_LST, L"GetModuleHandleWCRK(lpModuleName: %S)", dump_str(lpModuleName));
	#endif

	HMODULE hModule = GetModuleHandleW(lpModuleName);

	if( hModule == g_hModule)
		return NULL;
	else
		return hModule;
}

DWORD WINAPI GetModuleFileNameACRK(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
	#if _DEBUG
		Debug_PrintA(IDC_LST, "GetModuleFileNameACRK(hModule: %08X, lpFilename: %08X, nSize: %08X)", hModule, lpFilename, nSize);
	#endif

	if( hModule == g_hModule)
		return NULL;
	else
		return GetModuleFileNameA(hModule, lpFilename, nSize);
}

DWORD WINAPI GetModuleFileNameWCRK(HMODULE hModule, LPWSTR lpFilename, DWORD nSize)
{
	#if _DEBUG
		Debug_PrintW(IDC_LST, L"GetModuleFileNameWCRK(hModule: %08X, lpFilename: %08X, nSize: %08X)", hModule, lpFilename, nSize);
	#endif

	if( hModule == g_hModule)
		return NULL;
	else
		return GetModuleFileNameW(hModule, lpFilename, nSize);
}

//
///////////////////////////
