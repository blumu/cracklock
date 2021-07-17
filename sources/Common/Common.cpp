//  MODULE:    Common.cpp
//
//  PURPOSE:   Outils commun a different module de cracklock dont le Kernel
//             Ces fonctions ne sont pas mises dans DLLFuncs.cpp afin a ce que CKERN.dll n'ait pas a importer la DLL CL_SHELLEXT_DLL (realtivement lourde). Ceci est important car CLKERN.dll est injectee dans chaque application prise en charge par Cracklock.
//
#include "stdafx.h"
#include "Common.h"   // Outils communs
#include <strsafe.h>


// l'option indiquant si le 'forward vers local' doit etre active par defaut
bool g_OptionForwardSettingsToLocalByDefault = false; // valeur par defaut


// Handle of the DLL/EXE module containing the code of the Common.cpp
// This is used by the GetCLPath function to retrieve the location of the Cracklock directory.
HMODULE g_hCurrentModule = GetModuleHandle(CL_KERNEL_DLL); // by default suppose that Common.cpp is included by the Kernel DLL



// Parcours la liste des sous-sections de REGKEY_MBCL_APPS (ou REGKEY_MBCL_DLLS selon que bDll=true ou false) 
// et supprime les fichiers INI locaux associes a chaque entree
void DeleteForwards(bool bDLL)
{
    // ouvre la clé comportant la liste des applications crackés
    SettingSection secApps;
    TCHAR inifile[_MAX_PATH], appfilename[_MAX_PATH],
          szKeyName[_MAX_PATH];
    DWORD dwForward;
    if( secApps.Open( bDLL ? REGKEY_MBCL_DLLS : REGKEY_MBCL_APPS ) ) {
        for(secApps.InitEnumSubsection(); secApps.EnumSubsection(szKeyName, _countof(szKeyName));) {
            SettingSection *secApp = secApps.GetSubsection(szKeyName);
            if( secApp && secApp->GetValue(REGVALUE_PROG_FORWARD, &dwForward) && dwForward ) {
                
                RegPathToFilePath(szKeyName, _countof(appfilename), appfilename );
                InifilenameFromApp(appfilename, inifile, _countof(inifile));
                DeleteFile(inifile);
            }
        }
    }
}

// Supprime tous les fichiers de configuration y compris les INI forwardes dans les repertoires locaux de
// chaque application crackee
void DeleteCLSettings(SettingsStorageLocation loc)
{
    // Parcours la liste des applications et des DLL pour supprimer les fichiers INI locaux
    DeleteForwards(true);  // supprime les forwards des DLL
    DeleteForwards(false); // supprime les forwards des EXE
    
    // supprime toutes les settings
    DeleteSettings(loc);
	/*
    SettingSection::DeleteSectionTree(secloc, REGKEY_MBCL_APPS);
	SettingSection::DeleteSectionTree(secloc, REGKEY_MBCL_DLLS);
	SettingSection::DeleteSection(secloc, REGKEY_MBCL_RENAME);
    SettingSection::DeleteSection(secloc, REGKEY_MBCL_SETTINGS);
    SettingSection::DeleteSection(secloc, REGKEY_MBCL_CURVERSION);
    SettingSection::DeleteSection(secloc, REGKEY_MBCL);
    SettingSection::DeleteSection(secloc, REGKEY_MB);
    */
}


// Initialise le module Commmon
void InitCommon(HMODULE hModule) // Handle du module contenant le code de Common.cpp
{
    g_hCurrentModule = hModule;

    // Initialise le gestionnaire de settings
    TCHAR CLPath[_MAX_PATH];
    GetCLPath(CLPath, _countof(CLPath));
    InitSettingEngine(CLPath);
    g_OptionForwardSettingsToLocalByDefault = false; // false by default
    GetCracklockSettingValue(REGVALUE_SETTINGS_FORWARDBYDEFAULT, &g_OptionForwardSettingsToLocalByDefault);
}




// Extrait le nom du fichier a partir d'un chemin complet.
//
// Retourne un pointeur sur le nom du fichier dans pszPath.
PCTSTR GetFileBaseNamePart( PCTSTR pszPath )
{
    size_t cbPath;

    for (cbPath = _tcslen(pszPath); cbPath>0; cbPath--)
    {
        if( (pszPath[cbPath-1]=='\\') ||
            (pszPath[cbPath-1]==':')  ||
            (pszPath[cbPath-1]=='/')  )
            break;
    }
    return &pszPath[cbPath];
}





// Function name: SubsectionNameFromApp
// Description: Obtient le nom de la sous-section qui correspond à un fichier donnée en énumérant
//			    toute les sections des Settings (ini/winreg)
//
// Retourne true si la clée a été trouvée
//
// Argument 
// [in] pszFileName: nom du fichier dont la clée est à chercher
// [in] maxsize: taille du buffer destination en characteres
// [out] pszKey: pointeur sur la chaine où sera retourné le nom de la clé
//
bool CLCommonSettingSection::SubsectionNameFromApp(PCTSTR pszFileName, PTSTR pszOutSecname, DWORD cchSize )
{
    TCHAR szCanonFileName[_MAX_PATH];
    TCHAR szEnumSubsec[_MAX_PATH],
          szEnumSubsecDecoded[_MAX_PATH],
          szEnumSubsecCanonical[_MAX_PATH];

    // obtient le nom long du fichier demandé
    //DWORD dw = PathCanonicalize(szCanonFileName, pszFileName);
    _tcscpy_s(szCanonFileName, pszFileName);

    // énumère toutes les sous clé de la clé passée en paramètre
	for(InitEnumSubsection();
		EnumSubsection(szEnumSubsec, _countof(szEnumSubsec)); )
    {
        // Decode le nom de section en un chemin d'accès au fichier correspondant
        RegPathToFilePath(szEnumSubsec, _countof(szEnumSubsecDecoded), szEnumSubsecDecoded);
        _tcscpy_s(szEnumSubsecCanonical, szEnumSubsecDecoded);
        //PathCanonicalize(szEnumSubsecCanonical, szEnumSubsecDecoded);

        // Est-ce que ce nom correspond au nom recherché ?
        if( !_tcsicmp(szCanonFileName, szEnumSubsecCanonical ) ) {
            // oui alors copie le nom de la section et retourne TRUE
            _tcscpy_s( pszOutSecname, cchSize, szEnumSubsec );
            return true;
        }
    }

    // clé introuvable
    return false;
}


// Ouvre la clé de registre ou sont enregistres les parametres de Cracklock
bool CLCommonSettingSection::OpenCracklockSettingSection(bool bCreateIfNecessary)
{
	Close();
	if( location.bRegistry  ) {
		DWORD dwDispo;
		if( bCreateIfNecessary )
			return RegCreateKeyEx( HKEY_BASE, REGKEY_MBCL_SETTINGS, 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &this->hKey, &dwDispo) ==ERROR_SUCCESS;
		else
			return RegOpenKeyEx( HKEY_BASE, REGKEY_MBCL_SETTINGS, 0, KEY_ALL_ACCESS, &this->hKey)==ERROR_SUCCESS;
	}
	else {
        this->setPath(_T(""), REGKEY_MBCL_SETTINGS);
		return true;
	}
}


// Retourne une sous-section correspondant a un programme donne. Si la sous-section indique un forward vers un fichier
// de setting local a l'application, alors la section retournee est celle contenue dans ce fichier.
// [in] appsubsectionname: le nom de la sous-section correspondant a un application.
// retourne un pointeur sur la section de l'application ou bien NULL si elle n'existe pas.
SettingSection *CLCommonSettingSection::GetAppSubsectionForward(PCTSTR appsubsectionname)
{
	SettingSection *newsec = this->GetSubsection(appsubsectionname);
	DWORD dwForward;
	// do we need to follow a forward?
	if( newsec && newsec->GetValue(REGVALUE_PROG_FORWARD, &dwForward) && dwForward ) {
		TCHAR inifile[_MAX_PATH], appfilename[_MAX_PATH];
		RegPathToFilePath(appsubsectionname, _countof(appfilename), appfilename );
		InifilenameFromApp(appfilename, inifile, _countof(inifile));
		newsec->location = SectionStorageLocation(inifile);
		if( !newsec->Open(REGVALUE_PROG_LOCALSETTINGSECTION) ) {
			delete newsec;
			newsec = NULL;
		}
	}

	return newsec;
}



// Description: Retourne la section correspondant à un programme donné à partir du chemin d'accès au fichier EXE
// Remark: Si bForceCreate vaut TRUE alors la section est créée si elle n'existe pas.
// Retourne la section correspondant au programme demandé ou NULL si elle n'existe pas et que bForceCreate=false.
//
// Argument:
//	[in] pszFileName: chemin d'accès complet au fichier exe
//  [in] bDLL: le fichier demandé est-il une DLL?
//  [in] bForceCreate: force la création de la clé si elle n'existe pas?
//
SettingSection *CLCommonSettingSection::GetSectionOfApp( PCTSTR pszFileName, bool bDLL, bool bForceCreate )
{
	// ouvre la clé comportant la liste des applications crackés
	CLCommonSettingSection secApps;
    bool ret = bForceCreate ? secApps.Create( bDLL ? REGKEY_MBCL_DLLS : REGKEY_MBCL_APPS )
                            : secApps.Open( bDLL ? REGKEY_MBCL_DLLS : REGKEY_MBCL_APPS );
	if( ret ) {
		// trouve la clée correspondant au fichier
		TCHAR szKeyName[_MAX_PATH];
		if( secApps.SubsectionNameFromApp(pszFileName, szKeyName, _countof(szKeyName)) ) {
			return secApps.GetAppSubsectionForward(szKeyName);
		}
		else if( bForceCreate ) {
			// convertit le chemin d'accès au fichier dans l'encodage utilise pour les noms de section
			FilePathToRegPath(pszFileName, _countof(szKeyName), szKeyName );

			// si l'option "forward par defaut vers le repertoire local" est activee alors cree le forward
			if(g_OptionForwardSettingsToLocalByDefault) {
				SettingSection *forw_sec = secApps.CreateSubsection(szKeyName);
				if( forw_sec ) {
					// forward la config vers un fichier INI local
					forw_sec->SetValue(REGVALUE_PROG_FORWARD, (DWORD)1);
					delete forw_sec;

					// Cree le fichier INI local
					TCHAR inifilename[_MAX_PATH];
					InifilenameFromApp(pszFileName,inifilename,_countof(inifilename));
                    SettingSection *appini = new SettingSection(SectionStorageLocation(inifilename));
                    if( appini && appini->Create(REGVALUE_PROG_LOCALSETTINGSECTION) )
						appini->SetValue(REGVALUE_PROG_LOCAL, (DWORD)1);
					
					return appini;
				}
				else
					return NULL;
			}
			else
			{
				// créé une clée pour le fichier
				return secApps.CreateSubsection(szKeyName);
			}
		}
	}
	return NULL;
}

// Calcul le chemin du fichier INI local d'une appplication a partir de son chemin d'acces.
// Le fichier INI local est obtenu en ajoutant le suffix .Cracklock au nom du fichier EXE.
bool InifilenameFromApp(LPCTSTR pszAppFilename, LPTSTR pszAppIniFilename, int cchSize)
{
	_tcscpy_s(pszAppIniFilename, cchSize, pszAppFilename);
	_tcscat_s(pszAppIniFilename, cchSize, REGVALUE_PROG_LOCALFILESUFFIX);
	return false;
}



// Read the description of an PE file from the version info using GetFileVersionInfo
bool GetAppDescription(PCTSTR file, PTSTR pszDescr, DWORD cchSize)
{
    DWORD dw;
    struct LANGANDCODEPAGE {
          WORD wLanguage;
          WORD wCodePage;
    } *lpTranslate;


    DWORD sizeBlock = GetFileVersionInfoSize( file, &dw);
    if( sizeBlock == 0 )
        return false;

    LPVOID pBlock = malloc( sizeBlock );
    
    bool ret = false;
    HRESULT hr;
    if( GetFileVersionInfo(file, NULL, sizeBlock, pBlock) )
    {
        UINT cbTranslate;
        TCHAR SubBlock[50];

        // Read the list of languages and code pages.
        VerQueryValue(pBlock, 
                      TEXT("\\VarFileInfo\\Translation"),
                      (LPVOID*)&lpTranslate,
                      &cbTranslate);

        // Read the file description for each language and code page.

        for( UINT i=0; i < (cbTranslate/sizeof(struct LANGANDCODEPAGE)); i++ )
        {
          hr = StringCchPrintf(SubBlock, _countof(SubBlock),
                    TEXT("\\StringFileInfo\\%04x%04x\\FileDescription"),
                    lpTranslate[i].wLanguage,
                    lpTranslate[i].wCodePage);
          if (FAILED(hr))
            continue;
	
          // Retrieve file description for language and code page "i". 
          UINT uBytes;
          PVOID lpBuffer;
          if( VerQueryValue(pBlock, 
                        SubBlock, 
                        &lpBuffer, 
                        &uBytes) ) {
            
            _tcscpy_s(pszDescr, cchSize, (LPCTSTR)lpBuffer);
            ret = true;
            break;
          }
        }
    }

    free( pBlock );

    return ret;
}




// Function name: LoadDefault
// Description: Charge les parametres par defaut pour une application
// Retourne true si la chagement s'est bien deroule
bool AppSettings::LoadDefault(PCTSTR szFilePath // path sur le fichier PE
                              ) 
{
    // utilise la date courante par defaut
    GetLocalTime(&this->m_stDateTime);
    SystemTimeToFileTime(&this->m_stDateTime, &this->m_ftDateTime);

    // Options par defaut
    this->m_bActive = false;
    this->m_bOptSystem = false;    
    this->m_bOptConstant = false;
    this->m_bTimeMode = false;
    this->m_bDateMode = true;
    this->m_bTimeZoneMode = false;
    GetTimeZoneInformation(&this->m_tzi);

    // Description du programme
    if( !GetAppDescription(szFilePath, this->m_szProgDescr, _countof(this->m_szProgDescr)) )
        _tcscpy_s(this->m_szProgDescr, GetFileBaseNamePart(szFilePath));

    // Mode d'injection par defaut
    DWORD dwVal = 1; // default-injection set to "static" by default
    GetCracklockSettingValue(REGVALUE_SETTINGS_DEFAULTINJECTIONMODE, &dwVal);    
    if( dwVal == 0 ) // mode "run-time-injection par defaut"
        this->szLoaderFile[0] = _T('\0'); // n'utilise pas de fichier chargeur
    else { // mode "static par defaut"
        _tcscpy_s(szLoaderFile, szFilePath);
    }

    this->m_szCmdParameters[0] = _T('\0');

    // Mode standalone par defaut?
    this->m_bStandalone = !IsCracklockInPATHEnvVar();

    // Enregistrement local des settings par defaut?
    this->m_bLocalStorage = this->m_bStandalone;
    GetCracklockSettingValue(REGVALUE_SETTINGS_FORWARDBYDEFAULT, &this->m_bLocalStorage);
    
    return true;
}

// Function name: LoadFromSection
// Description: Charge à partir d'une section de la registry/INI les infos correspondants à une application cracké
// Retourne true si la chagement s'est bien deroule
bool AppSettings::LoadFromSection( PCTSTR szFilePath, // path sur le fichier PE correspondant a la section fournie dans le parametre sec
                                   const SettingSection *sec // Pointe sur la section a lire. 
                                    )
{
    // est-ce que la la section est valide?
    if( sec ) {
        // Nom du programme
		if( !sec->GetValue(REGVALUE_PROG_DESCRIPTION, m_szProgDescr, _countof(m_szProgDescr)) )
            _tcscpy_s(m_szProgDescr, GetFileBaseNamePart(szFilePath)); // la base du nom de fichier est prise comme description par defaut si aucune n'est specifiee

        ////////////////
        // CHARGE LA DATE ET L'HEURE
        if( !sec->GetValue(REGVALUE_PROG_DATE, &this->m_ftDateTime) )
            GetSystemTimeAsFileTime(&this->m_ftDateTime);

        // Formate la date sous forme d'une structure SYSTEMTIME
        FileTimeToSystemTime( &this->m_ftDateTime, &this->m_stDateTime);

        // Charge la Timezone
		if( !sec->GetValue(REGVALUE_PROG_TIMEZONE, &this->m_tzi) )
            GetTimeZoneInformation(&this->m_tzi);


        ////////////////
        // CHARGE LES OPTIONS
	    DWORD dwFlags;
        if( !sec->GetValue(REGVALUE_PROG_FLAGS, &dwFlags) )
    	    dwFlags = CLREG_CRACK;

        this->m_bActive = dwFlags & CLREG_CRACK ? true : false;
        this->m_bOptSystem = dwFlags & CLREG_SYSTEM ? true : false;
        this->m_bOptConstant = dwFlags & CLREG_CONSTANT ? true : false;
        // Porté du patch ? (date et/ou heure)
		this->m_bTimeMode = dwFlags & CLREG_VIRTUALMODE_TIME ? true : false;
        this->m_bDateMode = dwFlags & CLREG_VIRTUALMODE_DATE ? true : false;
        this->m_bTimeZoneMode = dwFlags & CLREG_VIRTUALMODE_TIMEZONE ? true : false;

        if( !sec->GetValue(REGVALUE_PROG_FORWARD, &this->m_bLocalStorage) )
            this->m_bLocalStorage = false;
        if( !sec->GetValue(REGVALUE_PROG_STANDALONE, &this->m_bStandalone) )
            this->m_bStandalone = false;

        ////////////////
        // CHARGE LE NOM DU FICHIER DE CHARGEMENT
		if( !sec->GetValue(REGVALUE_PROG_LOADERFILE, this->szLoaderFile, _countof(this->szLoaderFile)) )
            this->szLoaderFile[0] = _T('\0');

        ////////////////
        // CHARGE LES PARAMETRES D'EXECUTION DU FICHIER
        if( !sec->GetValue(REGVALUE_PROG_CMDPARAMETERS, this->m_szCmdParameters, _countof(this->m_szCmdParameters)) )
            this->m_szCmdParameters[0] = _T('\0');

        return true;
    }

    return false;

}


// Function name: Load
// Description: Charge la configuration d'une application crackée
// à partir d'un fichier App.exe.Cracklock si il existe ou sinon a partir 
// de la base de registre.
// Return: true si le chargement s'est bien passe
//
// Argument: 
//		[in] pszFileName: chemin d'accès au fichier EXE de l'application
//		[out] appsettings: structure ou sont retournes les parametres de l'application
//

bool AppSettings::Load(PCTSTR pszFileName)
{
    SettingSection *sec = NULL;

	// Si le fichier Appname.exe.cracklock existe alors lis les parametres a partir de ce fichier.
    TCHAR inipath[_MAX_PATH];
    InifilenameFromApp(pszFileName,inipath,_countof(inipath));
    if( FileExists(inipath) ) {
        sec = new SettingSection(SectionStorageLocation(inipath));
        if(sec )
            if( !sec->Open(REGVALUE_PROG_LOCALSETTINGSECTION)) {
                delete sec;
                sec = NULL;
            }
    }
    
    // Sinon: lis les parametres a partir des parametres principaux de Cracklock.
    // Ouvre la section correspondant à l'application demandée
    if( !sec )
	    sec = CLCommonSettingSection::GetSectionOfApp( pszFileName, false, false );

    if( !sec)
        return false;

    bool ret = this->LoadFromSection(pszFileName, sec);

    SettingSection ::deleteObject(sec);

    return ret;
}

bool GetCracklockSettingValue( LPTSTR value, RECT *out)
{
	CLCommonSettingSection sec;
	if( sec.OpenCracklockSettingSection() )
		return sec.GetValue(value, out);
	else
		return false;
}

bool GetCracklockSettingValue( LPTSTR value, DWORD *out)
{
	CLCommonSettingSection sec;
	if( sec.OpenCracklockSettingSection() )
		return sec.GetValue(value, out);
	else
		return false;
}

bool GetCracklockSettingValue( LPTSTR value, bool *out)
{
	CLCommonSettingSection sec;
	if( sec.OpenCracklockSettingSection() )
		return sec.GetValue(value, out);
	else
		return false;
}

// Save a Cracklock setting.
// [in] value: the name of the value
// [in] in: the content of to save
bool SetCracklockSettingValue( LPTSTR value, DWORD in)
{
  CLCommonSettingSection sec;
  if( sec.OpenCracklockSettingSection(true) )
	return sec.SetValue(value, in);
  else
	return false;
}
bool SetCracklockSettingValue(LPTSTR value, const RECT *in)
{
  CLCommonSettingSection sec;
  if( sec.OpenCracklockSettingSection(true) )
	return sec.SetValue(value, in);
  else
	return false;
}
bool SetCracklockSettingValue(LPTSTR value, bool in)
{
  CLCommonSettingSection sec;
  if( sec.OpenCracklockSettingSection(true) )
	return sec.SetValue(value, in);
  else
	return false;
}


#define IsLeapYear(wYear) ((wYear%4==0) - (wYear%100==0) + (wYear%1000==0))

// Calcule le nombre de jour dans un mois donné
int DaysInMonth (WORD wMonth, WORD wYear)
{
    static int NbMonthsDay[NB_MONTHSINYEAR] =
        {
            31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
        };

    //register 
        int ret = NbMonthsDay[wMonth-1];

    // Mois de février : année bisextile ?
    if( IsLeapYear(wYear) && (wMonth == 2) )
        ret++;

    return ret;
}

// Calcule la position du premier jour du mois dans le calendrier
//
// ENTRÉE :
//  * WORD wMonth   ==> Mois courrant
//  * WORD wYear   ==> Année courrante
//  * int cFirstDayOfWeek ==> Numéro du premier jour dans une semaine
//         (0 pour Lundi, 1 pour Mardi, etc... )
// SORTIE :
//  * int       ==> Numéro du premier jour du mois
//
int CalcFirstDay(WORD wMonth, WORD wYear, int iFirstDayOfWeek)
{
    WORD i;
    WORD nb_days = 0;

    // Calcule pour toutes les années pleines
    for (i=REF_YEAR; i<wYear; i++)
        nb_days += 365 + IsLeapYear(i);

    // Calcule pour tous les mois de la dernière année
    for (i=1; i<wMonth; i++)
        nb_days += DaysInMonth(i,wYear);


    return (REF_DAY + nb_days - iFirstDayOfWeek + NB_DAYSINWEEK) % NB_DAYSINWEEK;
    // le + NB_DAYSINWEEK c'est pour qu'il n'y est pas de nombres négatifs
}

// Supprime s'il existe, le backslash à la fin d'un chemin d'accès
/*void RemoveEndSlash( PSTR pszPath )
{
    int nLen = strlen(pszPath);
    if( (pszPath[nLen-1] == '\\')
            || (pszPath[nLen-1] == '/') )
    {
        pszPath[nLen-1] = '\0';
    }
}*/

// Libère la mémoire allouée par un appelle à la fonction LoadResString
void FreeResString(PTSTR pszResStr)
{
    free(pszResStr);
}


// Charge une chaîne de caractère à partir de la table de chaîne des resources
PTSTR LoadResString(HMODULE hInst, UINT wID)
{
    UINT    block, num;
    int     len = 0;
    HRSRC   hRC = NULL;
    HGLOBAL hgl = NULL;
    PWSTR  str = NULL;
    register UINT i;
    PTSTR pResStr;

    if( wID == 0)
        return 0 ;

    block = (wID >> 4) + 1;// compute block number
    num = wID & 0xf;// compute offset into block

    hRC = FindResource(hInst, MAKEINTRESOURCE(block), RT_STRING);
    if (!hRC)
        return 0 ;

    hgl = LoadResource(hInst, hRC);
    if (!hgl)
        return 0 ;

    str = (LPWSTR)LockResource(hgl);
    if (!str)
        return 0 ;

    // Move up block to string we want
    for (i = 0; i < num; i++)
        str += *str + 1;

    // convert the string to current code page
    len = *str;

    pResStr = (PTSTR)malloc(sizeof(TCHAR) * (len+1) );

    if( pResStr )
    {
        // convert the string to current code page
#ifdef _UNICODE
        _tcsncpy_s(pResStr,len+1, str+1,len);
#else

        len = WideCharToMultiByte(CP_ACP,
                                  WC_COMPOSITECHECK,
                                  str + 1, *str,
                                  pResStr, len,
                                  NULL, NULL);
#endif

        pResStr[len] = '\0';
    }

    return pResStr;
}


// Obtient le chemin d'accès au répertoire d'un module donné
// [in] hModule: handle du module
// [out] pszPath: chemin d'acces du module
// [in] cchSize: taille du buffer pszPath
bool GetModulePath(HMODULE hModule, PTSTR pszPath, int cchSize)
{
    GetModuleFileName(hModule, pszPath, cchSize);

    TCHAR *pszFilename = _tcsrchr(pszPath, _T('\\'));
    if ( !pszFilename )
        return false;

    // removes the filename part
    *pszFilename = _T('\0');
    
    return true;
}



// Retourne le chemin d'access au repertoire Bin\ de Cracklock
// [out] pszPath: chemin d'access au repertoir Cracklock\Bin\ 
// [in] cchSize: taille du buffer pszPath
bool GetCLBinPath (PTSTR pszPath, int cchSize)
{
  return GetModulePath(g_hCurrentModule, pszPath, cchSize); // obtient le chemin a partir du handle de module d'une DLL se trouvant dans le repertoire bin
}


// Obtient le chemin d'accès au répertoire de Cracklock
// [out] pszPath chemin d'access au repertoir Cracklock
// [in] cchSize: taille du buffer pszPath
bool GetCLPath(PTSTR pszPath, int cchSize)
{
    GetCLBinPath(pszPath, cchSize);

    // removes the '\Bin' part
    TCHAR *pszBinPart = pszPath + _tcslen(pszPath) - _countof(DIR_CL_BIN) + 1;
    if( 0==_tcsnicmp(pszBinPart, DIR_CL_BIN,_countof(DIR_CL_BIN)-1) )
        *(pszBinPart-1) = '\0';

    return true;
    /*
	SettingSection sec;
	if( sec.OpenCracklockSettingSection() ) {
     if( sec.GetValue(KEY_SETTINGS_CLPATH, pszPath,  _countof(pszPath)) ) {
      RemoveEndSlash( pszPath );
      return true;
     }
     else {
      pszPath[0] = '\0';      
      return false;
     }
	}
   */
}

// Charge en mémoire la dll contenant les resources localisées
//
// * bVerbose : affiche un message d'erreur si la fonction échoue
//
HINSTANCE LoadResDll ( bool bVerbose )
{
    // répertoire où est installé Cracklock
	TCHAR szCLPath[_MAX_PATH];
    if( !GetCLPath(szCLPath, _countof(szCLPath)) )
        return NULL;

    HINSTANCE hResDll = NULL;
	CLCommonSettingSection sec;
	if( sec.OpenCracklockSettingSection() ) {
        // Lit dans la reg le nom de la DLL pour la langue choisie par l'utilisateur
		TCHAR szRegDllName[_MAX_PATH];
        if( sec.GetValue(REGVALUE_SETTINGS_LANGUAGE, szRegDllName, _countof(szRegDllName)) ) {
			TCHAR szDllPath[_MAX_PATH];
            StringCchPrintf(szDllPath, _countof(szDllPath), _T("%s\\%s\\%s"), szCLPath, DIR_CL_LANG, szRegDllName);
            hResDll = LoadLibrary(szDllPath);
        }
    }

    // si échoué : Charge la dll par défaut à partir du rep de Cracklock
    if( !hResDll ) {
		TCHAR szDllPath[_MAX_PATH];
        StringCchPrintf(szDllPath, _countof(szDllPath), _T("%s\\%s\\%s"), szCLPath, DIR_CL_LANG, DEFAULT_RES_DLL);
        hResDll = LoadLibrary(szDllPath);
    }

    // si échoué : Charge la dll par défaut à partir du rep système
    if( !hResDll )
        hResDll = LoadLibrary(DEFAULT_RES_DLL);


    // si échoué et mode verbose: Message d'erreur
    if( !hResDll && bVerbose )
        MessageBox(NULL, _T("Cannot find or load the resource file CLRESUS.DLL!"), TITRE_APPLI, MB_OK | MB_ICONERROR);

    return hResDll;
}




// Cette fonction fonctionne seulement sous Windows a partir de NT (WinNT/2000/XP/...)
// Elle met a jour une clef de base de registre contenant une liste de chaines separee par le  'separator'.
// Selon la valeur de mode, la fonction lit, ecrit ou bien supprime la chaine 'value' de la clef 
// keyroot\keypath\value
// Retourne 0 si la fonction s'est executee avec succes.
int UpdateRegistrySeparatedStringValueNT(HKEY keyroot, LPCTSTR keypath, LPCTSTR value,
                                          LPCTSTR valuedata, UPDATEREG_MODE mode, 
                                          LPCTSTR separator)
{
    HKEY hKey;
    DWORD dwDispo;

    switch( mode ) {
        case EnvVarTestExistance:
        {
            // Open the reg key
            if( RegOpenKeyEx(keyroot, keypath, 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS )
                return URSSV_ABSENT; // the value does not exist

            // Query the reg value data size
            DWORD dwSize, dwType;
            if( RegQueryValueEx(hKey, value, 0, &dwType, NULL, &dwSize) 
                != ERROR_SUCCESS ) {
                RegCloseKey(hKey);
                return URSSV_ABSENT; // the value does not exist
            }

            if( dwType != REG_SZ && dwType != REG_EXPAND_SZ ) {
                RegCloseKey(hKey);
                return URSSV_UNEXPECTEVALUETYPE; // wrong value type
            }

            PTSTR token, pszCurData = (PTSTR)malloc(dwSize);
            RegQueryValueEx(hKey, value, 0, NULL, (PBYTE)pszCurData, &dwSize);

            // tant qu'il y a des tokens dans pstr
            TCHAR *next_token;
            token = _tcstok_s( pszCurData, separator, &next_token );
            BOOL bFound = false;
            while( token != NULL && !bFound ) {
                bFound = _tcsicmp(valuedata, token) == 0;

                // token suivant
                token = _tcstok_s( NULL, separator, &next_token);
            }

            free(pszCurData);
            RegCloseKey(hKey);

            return bFound ? URSSV_PRESENT : URSSV_ABSENT;
        }

        case EnvVarAddValue:
        {
            if( RegCreateKeyEx(keyroot, keypath, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDispo)
                != ERROR_SUCCESS )
                return URSSV_KEYNOTFOUND;


            bool value_already_exist = false;

            // Query the value data size and type
            DWORD dwSize, dwType;
            PTSTR pszNewData = NULL;
            PTSTR pszCurData = NULL;
            if( ERROR_SUCCESS != RegQueryValueEx(hKey, value, 0, &dwType, NULL, &dwSize) 
                || dwSize <= 0)
            {
                pszNewData = _tcsdup(valuedata);
                dwType = REG_SZ;
            }
            else
            {
                pszCurData = (PTSTR)malloc(dwSize);
                RegQueryValueEx(hKey, value, 0, NULL, (PBYTE)pszCurData, &dwSize);

                SIZE_T cntNewValue = dwSize/sizeof(TCHAR) + _tcslen(valuedata) + _tcslen(separator);
                pszNewData = (PTSTR)malloc(cntNewValue*sizeof(TCHAR));

                // Ajoute 'valuedata' au debut de la liste
                _tcscpy_s(pszNewData, cntNewValue, valuedata);

                // ajoute les chaines deja existantes
                value_already_exist = false;
                TCHAR *next_token;
                PTSTR token = _tcstok_s( pszCurData, separator, &next_token );
                while( token != NULL ) {
                    if( _tcsicmp(valuedata, token) == 0 ) {
                        // n'ajoute pas 'valuedata' si cette chaine etait deja presente
                        value_already_exist = true;
                    }
                    else { 
                        _tcscat_s(pszNewData, cntNewValue, separator);
                        _tcscat_s(pszNewData, cntNewValue, token);
                    }

                    // passe au token suivant
                    token = _tcstok_s( NULL, separator, &next_token );
                }
            }

            if( !value_already_exist )
                RegSetValueEx(hKey, value, 0, dwType, (PBYTE)pszNewData,
                    sizeof(TCHAR)*(_tcslen(pszNewData)+1));

            free(pszCurData);
            free(pszNewData);
            RegCloseKey(hKey);

            return value_already_exist ? URSSV_ALREADYEXISTS : URSSV_SUCCESS;
        }

        case EnvVarDeleteValue:
        {
            // Open the reg key
            if( RegOpenKeyEx(keyroot, keypath, 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS )
                return URSSV_KEYNOTFOUND;

            // Query the reg value data size
            DWORD dwSize, dwType;
            if( RegQueryValueEx(hKey, value, 0, &dwType, NULL, &dwSize) 
                != ERROR_SUCCESS || dwSize <= 0) {
                RegCloseKey(hKey);
                return URSSV_KEYNOTFOUND;
            }

            if( dwType != REG_SZ && dwType != REG_EXPAND_SZ ) {
                RegCloseKey(hKey);
                return URSSV_UNEXPECTEVALUETYPE; // wrong value type
            }

            PTSTR token,
                    pszCurData = (PTSTR)malloc(dwSize),
                    pszNewData = (PTSTR)malloc(dwSize);
            SIZE_T cntNewValue = dwSize/sizeof(TCHAR);

            RegQueryValueEx(hKey, value, 0, NULL, (PBYTE)pszCurData, &dwSize);
            pszNewData[0] = '\0';

            // tant qu'il y a des tokens dans pstr
            TCHAR *next_token;
            token = _tcstok_s( pszCurData, separator, &next_token );
            BOOL first = true;
            BOOL found = false;
            while( token != NULL ) {
                if( _tcsicmp(valuedata, token) == 0 )
                    found = true;  // n'ajoute pas 'valuedata' a la nouvelle liste
                else {
                    if( !first ) _tcscat_s(pszNewData, cntNewValue, separator);
                    _tcscat_s(pszNewData, cntNewValue, token);
                    first = false;
                }

                // token suivant
                token = _tcstok_s( NULL, separator, &next_token);
            }

            if( found )
                RegSetValueEx(hKey, value, 0, dwType, (PBYTE)pszNewData,
                    sizeof(TCHAR)*(_tcslen(pszNewData)+1));

            free(pszCurData);
            free(pszNewData);
            RegCloseKey(hKey);
            return found ? URSSV_SUCCESS : URSSV_DATANOTFOUND;
        }

        default:
            return URSSV_WRONGPARAMETER;
    }
}


// retourne trus si le chemin d'access au repertoire cracklock\bin se trouve dans le PATH
bool IsCracklockInPATHEnvVar()
{
    TCHAR szCLBinPath[_MAX_PATH];
    GetCLBinPath(szCLBinPath, _countof(szCLBinPath));
    return URSSV_PRESENT == UpdateRegistrySeparatedStringValueNT(HKEY_CURRENT_USER, 
                                REGKEY_ENVIRONMENT, REGVALUE_ENVVAR_PATH, 
                                szCLBinPath, EnvVarTestExistance, _T(";") );
}