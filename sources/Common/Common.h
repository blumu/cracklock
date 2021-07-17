////////////
// Common.h
// Fonctions communes a plusieurs projets de Cracklock
// 
// Ce header contient les prototypes de fonctions exportees par CL_SHELLEXT_DLL

#pragma once
#include <tchar.h>
#include <string>

#include "..\ShellExt\impexp.h" // utilise par le project ShellExt afin d'exporte des functions dans la dll CL_SHELLEXT_DLL
#include "INIReg.h"

using namespace std;

#ifndef tstring
#ifdef UNICODE
    typedef wstring tstring;
#else
    typedef string tstring;
#endif
#endif



//////////
// Constantes


// Sous-clés de Cracklock dans la base de registre
#define REGKEY_MB                   REGKEY_APPROOT // _T("Software\\MicroBest")
#define KEYAPPNAME                  _T("Cracklock")
#define REGKEY_MBCL                 REGKEY_MB _T("\\") KEYAPPNAME
#define REGKEY_MBCL_CURVERSION      REGKEY_MBCL _T("\\3.0")
#define REGKEY_MBCL_APPS            REGKEY_MBCL_CURVERSION _T("\\Apps")
#define REGKEY_MBCL_DLLS            REGKEY_MBCL_CURVERSION _T("\\Dlls")
#define REGKEY_MBCL_RENAME          REGKEY_MBCL_CURVERSION _T("\\Rename")
#define REGKEY_MBCL_SETTINGS        REGKEY_MBCL_CURVERSION _T("\\Settings")
#define REGKEY_EXPLORER_RUNONCE     _T("Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce")

// Clés et valeur dans la reg pour l'installation et la désinstallation d'un system-wide injector
#define REGKEY_RUN                  _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServices")
#define REGVALUE_RUN_INJECTOR       _T("Cracklock Injector")
#define REGKEY_WINNT_WIN            _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows")
#define REGVALUE_APPINIT_DLLS       _T("AppInit_DLLS")

// Clé contenant les variables d'environement
#define REGKEY_ENVIRONMENT          _T("Environment")

#define REGVALUE_ENVVAR_PATH        _T("Path")

#define REGVALUE_SETTINGS_ADVANCED              _T("AdvancedMode")
#define REGVALUE_SETTINGS_CLPATH                _T("CracklockPath")
#define REGVALUE_SETTINGS_LANGUAGE              _T("Language")
#define REGVALUE_SETTINGS_COPYINJECTDLL         _T("CopyInjectDll")
#define REGVALUE_SETTINGS_FORWARDBYDEFAULT      _T("ForwardToLocalFolderByDefault")
#define REGVALUE_SETTINGS_DEFAULTINJECTIONMODE  _T("DefaultInjectionMode")

#define REGVALUE_PROG_DATE                      _T("Date")
#define REGVALUE_PROG_FLAGS                     _T("Flags")
#define REGVALUE_PROG_TIMEZONE                  _T("Timezone")
#define REGVALUE_PROG_LOADERFILE                _T("LoaderFile")
#define REGVALUE_PROG_DESCRIPTION               _T("Name")
#define REGVALUE_PROG_CMDPARAMETERS             _T("Parameters")
#define REGVALUE_PROG_FORWARD                   _T("Forward")
#define REGVALUE_PROG_STANDALONE                _T("Standalone")
#define REGVALUE_PROG_LOCAL                     _T("Local")
#define REGVALUE_PROG_LOCALFILESUFFIX           _T(".cracklock")
#define REGVALUE_PROG_LOCALSETTINGSECTION       _T("Configuration")



/// Shell extension registry keys
#define  SHELLEX_GUID                             _T("{6EF84290-174B-11d1-B524-0080C8141490}")

#define REGKEY_SHELLEXTCLSID                      _T("CLSID\\") SHELLEX_GUID
#define REGKEY_SHELLEXTCLSID_INPROCSRV            _T("CLSID\\") SHELLEX_GUID _T("\\InProcServer32")
#define REGVALUE_THREADINGMODEL                   _T("ThreadingModel")
#define REGDATA_APARTMENT                         _T("Apartment")
#define REGKEY_PROPSHEETHANDLER                   _T("*\\shellex\\PropertySheetHandlers\\Cracklock")
#define REGKEY_CONTEXTMENUHANDLER                 _T("exefile\\shellex\\ContextMenuHandlers\\Cracklock")
#define REGKEY_SHELLEXTENSIONAPPROVED             _T("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved")





// Nom des DLL utilisées par CrackLock
#define CL_KERNEL_DLL_A         "CLKERN.DLL"
#define CL_KERNEL_DLL           _T(CL_KERNEL_DLL_A)
#define CL_SHELLEXT_DLL         _T("ShellExt.dll")

#define CL_MANAGER_EXE          _T("Manager.exe")
#define CL_INJECTOR_EXE         _T("Injector.exe")
#define CL_LOADER_EXE           _T("Loader.exe")

#define KERNEL32_DLL_A          "KERNEL32.DLL"
#define KERNEL32_DLL            _T(KERNEL32_DLL_A)

#define BACKUP_EXT              _T(".CLB")     // extension du backup (CrackLock Backup)
#define BACKUP_EXT_SIZE         4              // Taille de l'extension du backup

#define S_TABTITLE              _T("Cracklock")     // Titre de l'onglet

#define TITRE_APPLI             _T("Cracklock")         // Titre de l'application
#define SETUP_TITLE             _T("Cracklock setup")   // Titre a utiliser pendant l'installation de Cracklock
#define TITRE_MANAGER           _T("Cracklock Manager") // Titre du manager

// Sous-répertoires dans le rep de Cracklock
#define DIR_CL_BIN              _T("Bin")      // nom du sousrep Bin
#define DIR_CL_HELP             _T("Help")
#define DIR_CL_LANG             _T("Languages")

// Nom de la dll ressource utilisée par défaut
#define DEFAULT_RES_DLL         _T("CLRESUS.DLL")

// Font unicode contenant les symboles coreens et chinois
#define DEFAULT_UNICODE_FONTFACE    _T("Gulim")
#define DEFAULT_UNICODE_FONTSIZE    8

// Masque pour le mot de configuration dans la registry (valeur "Flags")
#define CLREG_CRACK                 0x0001
#define CLREG_CONSTANT              0x0002
#define CLREG_SYSTEM                0x0004
#define CLREG_VIRTUALMODE_TIMEZONE  0x0010
#define CLREG_VIRTUALMODE_DATE      0x0020
#define CLREG_VIRTUALMODE_TIME      0x0040


// calculs de dates
#define REF_YEAR        1980
#define REF_DAY         1  // Mardi

#define MIN_YEAR        1980
#define MAX_YEAR        2099
#define NB_DAYSINWEEK   7
#define NB_MONTHSINYEAR 12


DECLSPECIFIER void DeleteCLSettings(SettingsStorageLocation location);

/////// Settings functions specific to Cracklock
DECLSPECIFIER bool InifilenameFromApp(LPCTSTR pszAppFilename, LPTSTR pszAppIniFilename, int cchSize);
DECLSPECIFIER bool GetCracklockSettingValue( LPTSTR value, RECT *out);
DECLSPECIFIER bool GetCracklockSettingValue( LPTSTR value, DWORD *out);
DECLSPECIFIER bool GetCracklockSettingValue( LPTSTR value, bool *out);
DECLSPECIFIER bool SetCracklockSettingValue( LPTSTR value, const RECT *in);
DECLSPECIFIER bool SetCracklockSettingValue( LPTSTR value, DWORD in);
DECLSPECIFIER bool SetCracklockSettingValue( LPTSTR value, bool in);

// section of the common settings of Cracklock
class DECLSPECIFIER CLCommonSettingSection : public SettingSection {
public:

	bool OpenCracklockSettingSection(bool bCreateIfNecessary = false);
	bool SubsectionNameFromApp( PCTSTR pszFileName, PTSTR pszOutSecname, DWORD cchSize );
	SettingSection *GetAppSubsectionForward(PCTSTR appsubsectionname);

	static SettingSection *GetSectionOfApp( PCTSTR pszFileName, bool bDLL, bool bForceCreate = false);
};

// Objet contenant la configuration d'une application
class DECLSPECIFIER AppSettings {
public:
    bool        m_bActive;              // Cracklock est-il activé ?

    SYSTEMTIME  m_stDateTime;           // date et heure au format SYSTEMTIME
    FILETIME    m_ftDateTime;           // au format FILETIME
    TIME_ZONE_INFORMATION m_tzi;        // timezone

    bool        m_bDateMode,            // true si la date doit être affectée par le crack
                m_bTimeMode,            // true si l'heure doit être affectée par le crack
                m_bTimeZoneMode;        // // true si la timezone doit être affectée par le crack

    bool        m_bOptSystem,           // option crack système-wide
                m_bOptConstant;         // option freeze

    // More options:
    bool        m_bLocalStorage,        // store settings in local dir?
                m_bStandalone;          // make the crack independant of Cracklock

    TCHAR       m_szProgDescr[_MAX_PATH];        // Program description
    TCHAR       m_szCmdParameters[_MAX_PATH];   // parametres d'execution du programme
    TCHAR       szLoaderFile[_MAX_PATH];        // fichier loader pour le mode static. Vide ("") si le mode "run-time injection" est actif.

    AppSettings()
    {
        m_bActive = false;
    
        memset(&m_stDateTime, 0, sizeof(m_stDateTime));
        memset(&m_ftDateTime, 0, sizeof(m_ftDateTime));        
        m_bDateMode = false;
        m_bTimeMode = false;
        m_bTimeZoneMode = false;
        memset(&m_tzi , 0, sizeof(m_tzi));

        m_bOptSystem = false;
        m_bOptConstant = false;

        m_bLocalStorage = false;
        m_bStandalone = false;
    
        m_szProgDescr[0] = _T('\0');
        m_szCmdParameters[0] = _T('\0');
        szLoaderFile[0] = _T('\0');
    }

    bool LoadDefault(PCTSTR szFilePath);
    bool LoadFromSection(PCTSTR szFilePath, const SettingSection *sec );
    bool Load(PCTSTR pszFileName);
};




DECLSPECIFIER void InitCommon(HMODULE hModule);

///////////// Date functions
DECLSPECIFIER int DaysInMonth (WORD wMonth, WORD wYear);
DECLSPECIFIER int CalcFirstDay(WORD wMonth, WORD wYear, int iFirstDayOfWeek);


///////////// File, filename and path manipulation functions
DECLSPECIFIER void RemoveEndSlash( PTSTR pszPath );
DECLSPECIFIER bool GetModulePath (HMODULE hModule, PTSTR pszPath, int cchSize);
DECLSPECIFIER bool GetCLPath(PTSTR pszPath, int cchSize);
DECLSPECIFIER bool GetCLBinPath(PTSTR pszPath, int cchSize);
DECLSPECIFIER PCTSTR GetFileBaseNamePart( PCTSTR pszPath );


///////////// Resources
DECLSPECIFIER PTSTR LoadResString(HMODULE hInst, UINT wID);
DECLSPECIFIER void FreeResString(PTSTR pszResStr);
DECLSPECIFIER HINSTANCE LoadResDll( bool bVerbose );



// Parametre de la function UpdateRegistrySeparatedStringValueNT
enum UPDATEREG_MODE {
	EnvVarAddValue,
	EnvVarDeleteValue,
	EnvVarTestExistance
};


// Code de retour de la fonction UpdateRegistrySeparatedStringValueNT
#define URSSV_SUCCESS               0x00
#define URSSV_ALREADYEXISTS         0x01
#define URSSV_KEYNOTFOUND           0x02
#define URSSV_VALUENOTFOUND         0x03
#define URSSV_DATANOTFOUND          0x04
#define URSSV_UNEXPECTEVALUETYPE    0x05
#define URSSV_WRONGPARAMETER        0x06

#define URSSV_ABSENT                0x00
#define URSSV_PRESENT               0x01

DECLSPECIFIER int UpdateRegistrySeparatedStringValueNT(HKEY keyroot, LPCTSTR keypath, LPCTSTR value,
                                            LPCTSTR valuedata, UPDATEREG_MODE mode, 
                                            LPCTSTR separator);

DECLSPECIFIER bool IsCracklockInPATHEnvVar();

DECLSPECIFIER bool GetAppDescription(PCTSTR file, PTSTR pszDescr, DWORD cchSize);
