//  MODULE:    Dllfuncs.cpp
//
//  PURPOSE:   Fonctions exportées par CL_SHELLEXT_DLL
//

#include "StdAfx.h"			// Header précompilé
#include "..\Resources\Resources.h"	// Resources localisés
#include "resource.h"		// Resources
#include "DLLFuncs.h"


// Retourne la couleur a utiliser pour les hyperliens
COLORREF GetHotLinkColor()
{
    if( GetSysColorBrush(COLOR_HOTLIGHT) != NULL )
        return GetSysColor(COLOR_HOTLIGHT);
    else
        return RGB(0,0,255);
}

// Retourne TRUE si le curseur se trouve sur le control idCtrl de la fenetre hDlg
bool IsCursorOnControl(HWND hDlg, UINT idCtrl)
{
    POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(hDlg, &pt);

    // Curseur sur un hyperlien ?
	return RealChildWindowFromPoint(hDlg, pt) == GetDlgItem(hDlg,idCtrl);
}

// Lance l'aide HTML correspondant au contexte demandé
// dwID=-1 pour acceder a la page de garde de l'aide
void InvokeHelp(HWND hwnd, DWORD dwID)
{
    // Affiche l'aide correspondant à l'identificateur du contrôle

    if( PTSTR chmfile = LoadResString(g_hResDll, IDS_CHMFILE) ) {
        TCHAR szHelpLink[2*_MAX_PATH + 20];
        // shall we open a specific subfile of the CHM help?
        if(dwID != -1)  {
            //mk:@MSITStore:C:\documents\Cracklock\Help\cracklock-en.chm::file.html
            PTSTR pszFile = LoadResString(g_hResDll, dwID );
            StringCchPrintf (szHelpLink, _countof(szHelpLink), _T("mk:@MSITStore:%s\\%s\\%s::%s"),
                g_szCLPath, DIR_CL_HELP, chmfile, pszFile);
            FreeResString(pszFile);
        }
        else {
            //mk:@MSITStore:C:\documents\Cracklock\Help\cracklock-en.chm
            StringCchPrintf (szHelpLink, _countof(szHelpLink), _T("mk:@MSITStore:%s\\%s\\%s"),
                g_szCLPath, DIR_CL_HELP, chmfile);
        }

        ShellExecute(hwnd, NULL, _T("hh"), szHelpLink, NULL, SW_SHOWDEFAULT);

        FreeResString( chmfile );
    }
}

// Libère la mémoire allouée par la dll des ressources
void FreeResDll( void )
{
    // Libère la mémoire allouée pour les chaînes
    FreeResString(g_S_ADVANCED);
    FreeResString(g_S_TABHELP);
    FreeResString(g_S_TABDEPEND);
    FreeResString(g_S_TABOPTIONS);
    FreeResString(g_S_TABMOREOPTIONS);
    FreeResString(g_S_TABGENERAL);

    // libère la dll contenant les ressources
    FreeLibrary(g_hResDll);
}

// Fonction appellée par CLMANAGER lorsque le language a changé
BOOL OnLanguageChanged (void)
{
    const TCHAR* gszNOTRANSLATION = _T("NO TRANSLATION");

  // Libère la mémoire allouée par la dll des ressources si elle a déjà été chargée
    if( g_hResDll )  
        FreeResDll();

    // Charge en mémoire la dll contenant les resources localisées
    g_hResDll = LoadResDll(true);
    if( !g_hResDll )
        return FALSE;

    // Crée les chaîne internationales
#define traduction(v,id)			(v) = LoadResString(g_hResDll, (id)); \
												                if( !(v) ) v = _tcsdup(gszNOTRANSLATION)

    traduction(g_S_TABGENERAL, IDS_TABGENERAL);
    traduction(g_S_TABOPTIONS, IDS_TABOPTIONS);
    traduction(g_S_TABMOREOPTIONS, IDS_TABMOREOPTIONS);
    traduction(g_S_TABDEPEND, IDS_TABDEPEND);
    traduction(g_S_TABHELP, IDS_TABHELP);
    traduction(g_S_ADVANCED, IDS_ADVANCED);

    return TRUE;
#undef traduction
}


// WinProc pour la fenetre de configuration general
INT_PTR CALLBACK SettingsDlgProc(HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    static TCHAR szCracklockPath[_MAX_PATH]; 
    switch (uMsg) {
        case WM_INITDIALOG:
            {
                // Centre la boite de dialogue
                RECT rcParent, rcWnd;
                GetWindowRect(GetParent(hwndDlg), &rcParent);
                GetWindowRect(hwndDlg, &rcWnd);
                SetWindowPos(hwndDlg,
                    HWND_BOTTOM,
                    rcParent.left+((rcParent.right-rcParent.left)-(rcWnd.right-rcWnd.left))/2,
                    rcParent.top+((rcParent.bottom-rcParent.top)-(rcWnd.bottom-rcWnd.top))/2,
                    0,
                    0,
                    SWP_NOSIZE);

                // Set the checkboxes values
                int SWIMode = GetSystemwideInjectionMode();
                CheckDlgButton(hwndDlg, IDC_CHKSWI_CULOGON, SWIMode&SYSTEMWIDE_INJECTOR_AT_CULOGON ? BST_CHECKED : BST_UNCHECKED );
                CheckDlgButton(hwndDlg, IDC_CHKSWI_LMLOGON, SWIMode&SYSTEMWIDE_INJECTOR_AT_LMLOGON ? BST_CHECKED : BST_UNCHECKED );
                CheckDlgButton(hwndDlg, IDC_CHKSWI_APPINIT , SWIMode&SYSTEMWIDE_APPINIT ? BST_CHECKED : BST_UNCHECKED );
                CheckDlgButton(hwndDlg, IDC_CHKSHELLEXT, IsShellExtInstalled() ? BST_CHECKED : BST_UNCHECKED );

                CheckRadioButton(hwndDlg, IDC_RADSTANDALONE, IDC_RADSHARED, IsCracklockInPATHEnvVar() ? IDC_RADSHARED : IDC_RADSTANDALONE);

                // Si la version de Windows est < Windows NT alors grise certaines options
                if(g_dwPlatformId != VER_PLATFORM_WIN32_NT) {
                    EnableWindow(GetDlgItem(hwndDlg, IDC_CHKSWI_APPINIT), FALSE);
                }
                
                // Pareil si on a pas les droits administrateur...
                if ( !IsUserAnAdmin() ) {
                    EnableWindow(GetDlgItem(hwndDlg, IDC_CHKSWI_APPINIT), FALSE);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_CHKSWI_LMLOGON), FALSE);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_CHKSHELLEXT), FALSE);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_GRPADMIN), FALSE);
                }

                bool bVal = false; // do not forward by default
                GetCracklockSettingValue(REGVALUE_SETTINGS_FORWARDBYDEFAULT, &bVal);
                CheckDlgButton(hwndDlg, IDC_CHKFORWARDSETTINGSTOLOCAL, bVal ? BST_CHECKED : BST_UNCHECKED );

                // Populate the preset combo
                PTSTR psz;

                psz = LoadResString(g_hResDll, IDS_CUSTOM);
                ComboBox_InsertString(GetDlgItem(hwndDlg, IDC_CMBPRESET), -1, psz);
                FreeResString(psz);
                psz = LoadResString(g_hResDll, IDS_DEFAULT);
                ComboBox_InsertString(GetDlgItem(hwndDlg, IDC_CMBPRESET), -1, psz);
                FreeResString(psz);
                psz = LoadResString(g_hResDll, IDS_FLASHDRIVE);
                ComboBox_InsertString(GetDlgItem(hwndDlg, IDC_CMBPRESET), -1, psz);
                FreeResString(psz);

                // select the "Custom..." preset
                ComboBox_SetCurSel(GetDlgItem(hwndDlg, IDC_CMBPRESET), 0);

                // Populate the injection mode combo
                psz = LoadResString(g_hResDll, IDS_RUNTIMEINJECTION);
                ComboBox_InsertString(GetDlgItem(hwndDlg, IDC_CMBINJECTIONMODE), -1, psz);
                FreeResString(psz);
                psz = LoadResString(g_hResDll, IDS_STATICINJECTION);
                ComboBox_InsertString(GetDlgItem(hwndDlg, IDC_CMBINJECTIONMODE), -1, psz);
                FreeResString(psz);

                // select the default injection mode
                DWORD dwVal = 1; // default injection mode set to static
                GetCracklockSettingValue(REGVALUE_SETTINGS_DEFAULTINJECTIONMODE, &dwVal);
                ComboBox_SetCurSel(GetDlgItem(hwndDlg, IDC_CMBINJECTIONMODE), dwVal);

                // select the storage location
                SettingsStorageLocation loc = GetSettingStorageLocation();
                CheckRadioButton(hwndDlg,IDC_RAD_WINREG, IDC_RAD_APPDATADIR, loc == WinReg ? IDC_RAD_WINREG : loc == AppDataINI ? IDC_RAD_APPDATADIR : IDC_RAD_CLDIR);
                
            }
            break;

        case WM_COMMAND:
            switch( wParam ) {
                case IDOK:
                    // IDC_CHKSWI_CULOGON
                    if( IsDlgButtonChecked(hwndDlg, IDC_CHKSWI_CULOGON) )
                        InstallSystemwideInjection(SYSTEMWIDE_INJECTOR_AT_CULOGON);
                    else
                        UninstallSystemwideInjection(SYSTEMWIDE_INJECTOR_AT_CULOGON);
                    
                    if( IsUserAnAdmin() ) {
                        // IDC_CHKSWI_LMLOGON
                        if( IsDlgButtonChecked(hwndDlg, IDC_CHKSWI_LMLOGON) )
                            InstallSystemwideInjection(SYSTEMWIDE_INJECTOR_AT_LMLOGON);
                        else
                            UninstallSystemwideInjection(SYSTEMWIDE_INJECTOR_AT_LMLOGON);

                        // IDC_CHKSWI_APPINIT
                        if( IsDlgButtonChecked(hwndDlg, IDC_CHKSWI_APPINIT) )
                            InstallSystemwideInjection(SYSTEMWIDE_APPINIT);
                        else
                            UninstallSystemwideInjection(SYSTEMWIDE_APPINIT);

                        // IDC_CHKSHELLEXT
                        if( IsDlgButtonChecked(hwndDlg, IDC_CHKSHELLEXT) ) {
                            InstallShellExt();
                        }
                        else {
                            UninstallShellExt();
                        }

                        // Kill the residant injector if necessary
                        if( !( IsDlgButtonChecked(hwndDlg, IDC_CHKSWI_CULOGON)
                               ||IsDlgButtonChecked(hwndDlg, IDC_CHKSWI_LMLOGON)) )
                            KillResidantInjector();

                    }    
                    
                    // If shared then add cracklock\bin to the path
                    InstallCracklockInPATHEnvVar(IsDlgButtonChecked(hwndDlg, IDC_RADSHARED) ? true : false);

                    // save the default injection mode
                    SetCracklockSettingValue(REGVALUE_SETTINGS_DEFAULTINJECTIONMODE, (DWORD)ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_CMBINJECTIONMODE)));

                    // save the forwardbydefault option
                    SetCracklockSettingValue(REGVALUE_SETTINGS_FORWARDBYDEFAULT, IsDlgButtonChecked(hwndDlg, IDC_CHKFORWARDSETTINGSTOLOCAL) ? true : false);


                    // change the storage location
                    if(!ChangeSettingsLocation(IsDlgButtonChecked(hwndDlg, IDC_RAD_WINREG) 
                                            ? WinReg 
                                            : IsDlgButtonChecked(hwndDlg, IDC_RAD_APPDATADIR) 
                                                ? AppDataINI 
                                                : BinINI // IsDlgButtonChecked(hwndDlg, IDC_RAD_CLDIR)  ? BinINI  : BinINI
                                                ) ) {
                        // Message : "Cannot move settings to the desired storage location. Please check the file permissions for the destination folder!"
                        PTSTR msg = LoadResString(g_hResDll, IDS_SETTINGSPERMISSIONERROR),
                              title = LoadResString(g_hResDll, IDS_MANAGERTITLE);                        
                        MessageBox(hwndDlg, msg, title, MB_OK | MB_ICONEXCLAMATION );
                        FreeResString(msg);
                        FreeResString(title);
                    }
                    else {    
                        EndDialog(hwndDlg, TRUE); 
                    }

                    return TRUE; 

                case IDCANCEL: 
                    EndDialog(hwndDlg, FALSE); 
                    return TRUE; 

                default:
                    break;
            }
            break;


        default:
            break;
    }
    return FALSE;
}

void ShowSettingsDlg(HWND parent)
{
    DialogBox(g_hResDll,  MAKEINTRESOURCE(IDD_SETTINGS), parent, SettingsDlgProc);
}


// (Fonction exportée, appelée par rundll32.exe)
// Renome les fichiers inscrits dans la registry
void CALLBACK RestoreLFN(HWND hwnd, HINSTANCE hinst, PSTR pszCmdLine, int nCmdShow)
{
	TCHAR	szValueName[_MAX_PATH];
	TCHAR	szValueContent[_MAX_PATH];
	

	SettingSection sec;
	if(sec.Open(REGKEY_MBCL_RENAME)) {
	
		size_t	cchNameSize = _countof(szValueName), 
				ccbContentSize = sizeof(szValueContent);
        sec.InitEnumValues();
		while( sec.EnumValues(szValueName, &cchNameSize, (PBYTE)szValueContent, &ccbContentSize, NULL) )
		{
			// Renomme le fichier de szValueData vers szValueName
			// Exemple le clef:
			//		"C:\APP\TRUC MACHIN BIDULE.EXE" = C:\APP\TRUC~1.EXE
			// indique qu'il faut renomer le fichier C:\APP\TRUC~1.EXE vers "C:\APP\TRUC MACHIN BIDULE.EXE"
			MoveFile(szValueContent, szValueName);
            
            cchNameSize = _countof(szValueName);
		    ccbContentSize = sizeof(szValueContent);
		}
	}
	SettingSection::DeleteSection(REGKEY_MBCL_RENAME);
}




// Extrait l'extension d'un nom de fichier dans un chemin complet
//
// Retourne un pointeur sur cette extension ou bien sur la fin de la 
// chaîne si il n'y en a pas.
//
// entree:
//	pszPath: chemin d'access a un fichier
//	size: est la taille du buffer pszExtFileName
// sortie:
//	pszExtFileName: pointer sur un buffer de taille 'size' au moins dans lequel l'extension est copiee.
//  Si NULL alors l'extension n'est pas copie et la function retourne juste le pointeur sur l'extension
//  dans pszPath.
PTSTR GetFileExtPart( PTSTR pszPath, SIZE_T size, PTSTR pszExtFileName )
{
	size_t cbPath;

	for (cbPath = _tcslen(pszPath); cbPath>0; cbPath--)
	{
		if( pszPath[cbPath-1] == _T('.') )
		{
			if( pszExtFileName != NULL )
				_tcscpy_s(pszExtFileName, size, &pszPath[cbPath]);
			break;
		}
	}

	return &pszPath[cbPath];
}


// Obtient le repertoire ou se trouve un fichier a partir de son chemin d'access complet.
void TruncateFilenameToFileDirectory(LPTSTR fullpath)
{
    LPCTSTR namepart = GetFileBaseNamePart(fullpath);
    // cas particulier ci le fichier se trouve dans la racine
    if( fullpath[namepart-fullpath-2] == _T(':') )
        // rajoute l'anti-slash à la fin
        fullpath[namepart-fullpath-1] = L'\\';

    fullpath[namepart-fullpath] = '\0';
}


// Affiche un message rapportant la dernière erreur survenue
void ErrorReport()
{
	LPVOID lpMsgBuf;
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
					GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,
					0,
					NULL);	// Display the string.
	MessageBox( NULL, (PTSTR)lpMsgBuf, _T("GetLastError"), MB_OK|MB_ICONINFORMATION );
	// Free the buffer.
	LocalFree( lpMsgBuf );
}

// Supprime les entrées de Settings incorrectes
void CleanupSettingsEntries()
{			
	TCHAR szKeyName[_MAX_PATH];

	// Fait 2 recherches: une fois pour les applications crackées
	// et une autres pour les DLLs crackées.
	for(int j=0; j<=1; j++) {
		// j = 0 : Ouvre la clé comportant la liste des APPLICATIONS crackés
		// j = 1 : Ouvre la clé comportant la liste des DLLS crackés
		SettingSection sec;
		if( !sec.Open( j ? REGKEY_MBCL_APPS : REGKEY_MBCL_DLLS) )
			continue;

		for(sec.InitEnumSubsection();
			sec.EnumSubsection(szKeyName, _countof(szKeyName));) {				
			// Si l'entrée de la registry pointe sur un fichier inexistant...
			if( !FileExists(szKeyName) ) {
				// ...alors supprime la
				sec.DeleteSubsection(szKeyName);
			}
		}
	}
}

// Obitent la date et l'heure à laquelle un fichier a été créé, accédé dernierement, et modifié dernierement
void GetFileTimeByName( PCTSTR lpcszFileName, LPFILETIME pftCreation, LPFILETIME pftLastAccess, LPFILETIME pftLastWrite )
{
	HANDLE hFile = CreateFile(lpcszFileName, 0, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);

	GetFileTime(hFile, pftCreation, pftLastAccess, pftLastWrite);
	
	CloseHandle(hFile);
}

// Change la date et l'heure à laquelle un fichier a été créé, accédé dernierement, et modifié dernierement
void SetFileTimeByName( LPCTSTR lpcszFileName, LPFILETIME pftCreation, LPFILETIME pftLastAccess, LPFILETIME pftLastWrite )
{
	HANDLE hFile = CreateFile(lpcszFileName, 0, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);

	SetFileTime(hFile, pftCreation, pftLastAccess, pftLastWrite);
	
	CloseHandle(hFile);
}






// Installe l'injection systematique a toutes les applications par le biais de la mode specifiee par 'mode'
// Si mode n'est pas specifie (ou si mode=-1) alors par defaut la methoe installee sera
//  - sous windows NT: AppInit_Dlls 
//  - sous Windows 95/ME: l'injecteur residant en memoire (CL_INJECTOR_EXE) execute au logon de l'utilisateur courant 
// retourne 0 si ok
int InstallSystemwideInjection( int mode )
{
    // si aucun mode n'est specifie alors
    if( mode == SYSTEMWIDE_DEFAULT ) {
        // Par defaut, sous NT utilise la technique AppInit
        if( g_dwPlatformId == VER_PLATFORM_WIN32_NT )
            mode = SYSTEMWIDE_APPINIT;
        // et sous 9x utilise la technique de l'injecteur lance a l'ouverture de session de l'utilisateur courrant
        else 
            mode = SYSTEMWIDE_INJECTOR_AT_CULOGON;
    }

    if( mode & SYSTEMWIDE_APPINIT ) {
        int ret = UpdateRegistrySeparatedStringValueNT(HKEY_LOCAL_MACHINE, REGKEY_WINNT_WIN, REGVALUE_APPINIT_DLLS, CL_KERNEL_DLL, EnvVarAddValue, _T(","));
        if( URSSV_SUCCESS != ret )
            return ret;
    }

    TCHAR szInjector[_MAX_PATH];
    _tcscpy_s(szInjector, g_szCLPath);
    _tcscat_s(szInjector, _T("\\Bin\\"));
    _tcscat_s(szInjector, CL_INJECTOR_EXE);

    if( mode & SYSTEMWIDE_INJECTOR_AT_CULOGON ) {
        HKEY hKey;
        if( RegCreateKeyEx( HKEY_CURRENT_USER ,
                            REGKEY_RUN, 0, _T(""), REG_OPTION_NON_VOLATILE, 
                            KEY_ALL_ACCESS, NULL, &hKey, NULL)
            != ERROR_SUCCESS )
            return 1;

        RegSetValueEx(hKey, REGVALUE_RUN_INJECTOR, 0, REG_SZ, (PBYTE)&szInjector,
            sizeof(TCHAR)*(_tcslen(szInjector)+1));

        RegCloseKey(hKey);
    }

    if( mode & SYSTEMWIDE_INJECTOR_AT_LMLOGON ) {
        HKEY hKey;
        if( RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                            REGKEY_RUN, 0, _T(""), REG_OPTION_NON_VOLATILE, 
                            KEY_ALL_ACCESS, NULL, &hKey, NULL)
            != ERROR_SUCCESS )
            return 1;

        RegSetValueEx(hKey, REGVALUE_RUN_INJECTOR, 0, REG_SZ, (PBYTE)&szInjector,
            sizeof(TCHAR)*(_tcslen(szInjector)+1));

        RegCloseKey(hKey);
    }

    return 0;
}


// Desinstalle l'injection systematique.
int UninstallSystemwideInjection( int mode )
{
     if( mode == SYSTEMWIDE_DEFAULT ) {
         mode = SYSTEMWIDE_INJECTOR_AT_CULOGON | SYSTEMWIDE_INJECTOR_AT_LMLOGON | SYSTEMWIDE_APPINIT;
     }

    // Si >=Windows NT 
    if( mode&SYSTEMWIDE_APPINIT && g_dwPlatformId == VER_PLATFORM_WIN32_NT )
    {
        // alors enleve CLKERN.DLL de la clef APPINIT_DLLS de la base de registre.
        UpdateRegistrySeparatedStringValueNT(HKEY_LOCAL_MACHINE, REGKEY_WINNT_WIN, REGVALUE_APPINIT_DLLS, CL_KERNEL_DLL, EnvVarDeleteValue, _T(","));
    }

    if( mode&SYSTEMWIDE_INJECTOR_AT_CULOGON ) {
        // supprime CL_INJECTOR_EXE de l'autorun
        HKEY hKey;
        if( RegOpenKeyEx(HKEY_CURRENT_USER, REGKEY_RUN, 0, KEY_ALL_ACCESS, &hKey)
            == ERROR_SUCCESS ) {
            RegDeleteValue(hKey, REGVALUE_RUN_INJECTOR);
            RegCloseKey(hKey);
        }
    }

    if( mode&SYSTEMWIDE_INJECTOR_AT_LMLOGON ) {
        // supprime CL_INJECTOR_EXE de l'autorun
        HKEY hKey;
        if( RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_RUN, 0, KEY_ALL_ACCESS, &hKey)
            == ERROR_SUCCESS ) {
            RegDeleteValue(hKey, REGVALUE_RUN_INJECTOR);
            RegCloseKey(hKey);
        }
    }

    return 0;
}

// Retourne:
//     - SYSTEMWIDE_NONE si aucun system-wide injection n'est actif
//     - Une combinaison des constantes suivantes si au moins une des methodes de  
//      system-wide injection est active:
//          - SYSTEMWIDE_APPINIT
//          - SYSTEMWIDE_INJECTOR_AT_CULOGON
//          - SYSTEMWIDE_INJECTOR_AT_LMLOGON
int GetSystemwideInjectionMode()
{
    int ret = SYSTEMWIDE_NONE;

    //// Test for system injection using the AppInitDlls regkey
    if( URSSV_PRESENT == UpdateRegistrySeparatedStringValueNT(HKEY_LOCAL_MACHINE, REGKEY_WINNT_WIN, 
            REGVALUE_APPINIT_DLLS, CL_KERNEL_DLL, EnvVarTestExistance, _T(",") ) )
        ret |= SYSTEMWIDE_APPINIT;

    //// Test for a system injection based on the CL_INJECTOR_EXE process executed at logon for every user
    HKEY hKey;
    //LONG size;
    if( RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_RUN, 0, KEY_ALL_ACCESS, &hKey)
        == ERROR_SUCCESS )
    {
        if( RegQueryValueEx (hKey, REGVALUE_RUN_INJECTOR, 0, NULL, NULL, NULL) == ERROR_SUCCESS )
            ret |= SYSTEMWIDE_INJECTOR_AT_LMLOGON;
        RegCloseKey(hKey);
    }

    //// Test for a system injection based on the CL_INJECTOR_EXE process executed at logon for the current user only
    if( RegOpenKeyEx(HKEY_CURRENT_USER , REGKEY_RUN, 0, KEY_ALL_ACCESS, &hKey)
        == ERROR_SUCCESS ) {
        if( RegQueryValueEx (hKey, REGVALUE_RUN_INJECTOR, 0, NULL, NULL, NULL) == ERROR_SUCCESS )
            ret |= SYSTEMWIDE_INJECTOR_AT_CULOGON;
        RegCloseKey(hKey);
    }

    return ret;
}



// rajoute du texte au bout de la légende d'un contrôle
void RajouteAuTitre(HWND hwnd, PCTSTR pcsz)
{
    size_t len = GetWindowTextLength(hwnd) + _tcslen(pcsz) + 1 + 1;
    TCHAR *stitre = (TCHAR *)malloc(sizeof(TCHAR)*len);
    GetWindowText(hwnd, stitre, len);
    _tcscat_s(stitre, len, _T(" "));
    _tcscat_s(stitre, len, pcsz);
    SetWindowText(hwnd, stitre);
    free(stitre);
}

// Fonction appellee lors de l'enumeration des fenetres filles d'une autre fenetre.
// (cf. EnumChildWindows)
//
// Elle affecte une fonte donne a la fenetre hwnd.
//
// lParam contient le handle HFONT de la police a affecter
//
BOOL CALLBACK EnumChildSetFont(HWND hwnd, LPARAM lParam)
{
    // seulement si la classe n'est pas une classe Listview
    TCHAR  classname[30];
    GetClassName(hwnd, classname, 30);
    if( (_tcscmp(classname, WC_LISTVIEW) != 0) )
        SendMessage(hwnd, WM_SETFONT, (WPARAM)lParam, MAKELPARAM(TRUE, 0));
    return TRUE;
}


//////// Fonctions pour creer un raccourci dans l'explorer de Windows
#include <objbase.h>
#include <shlguid.h>

bool GetSpecialFolderLocation(LPTSTR path, int nFolder)
{
    LPITEMIDLIST pidl;
    HRESULT hr = SHGetSpecialFolderLocation(NULL, nFolder, &pidl);

    if (SUCCEEDED(hr)) {
        bool b = SHGetPathFromIDList(pidl, path) ? true : false;
        CoTaskMemFree(pidl);
        return b;
    }
    return false;
}


// Create a shortcut.
bool CreateShortcut(PCTSTR pszLinkPath, // specifies the full path to the shortcut to be created
                    PCTSTR pszExePath, PCTSTR pszArguments,                        
                    PCTSTR pszWorkingDirectory, PCTSTR pszDescription, 
                    PCTSTR pszIconPath, int iconIndex)
{
    // Must have called CoInitalize before calling this function! 
    IShellLink* psl; 

    // Get a pointer to the IShellLink interface. 
    HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, 
                                    IID_IShellLink, (PVOID *) &psl); 

    if (SUCCEEDED(hres)) { 
        IPersistFile* ppf; 
    
        // Set the path to the shortcut target and add the 
        // description. 
        psl->SetPath( pszExePath ); 
        psl->SetArguments( pszArguments );
        psl->SetWorkingDirectory( pszWorkingDirectory);
        psl->SetDescription( pszDescription); 
        psl->SetIconLocation(pszIconPath, iconIndex);
    
        // Query IShellLink for the IPersistFile interface for saving the 
        // shortcut in persistent storage. 
        hres = psl->QueryInterface(IID_IPersistFile, (PVOID *) &ppf); 
    
        if (SUCCEEDED(hres)) 
        { 
            // Ensure that the string is ANSI. 
            //WORD wsz[MAX_PATH];     
            //MultiByteToWideChar(CP_ACP, 0, (LPCSTR) szPathLink, -1, wsz, MAX_PATH);     
            //hres = ppf->Save(wsz, TRUE); 

            // Save the link by calling IPersistFile::Save. 
            hres = ppf->Save(pszLinkPath, TRUE); 
            ppf->Release(); 
        } 
        psl->Release(); 
    } 
    return SUCCEEDED(hres);
}

// Create a shortcut. The location of the shortcut is specified by a SpecialFolder index 'nFolder'.
bool CreateShortcut(int nFolder, // index of the special folder where the shortcut must be created
                    PCTSTR pszLinkFilename, // shortcut file name
                    PCTSTR pszExePath, PCTSTR pszArguments, 
                    PCTSTR pszWorkingDirectory, PCTSTR pszDescription, 
                    PCTSTR pszIconPath, int iconIndex)
{
    TCHAR szPathLink[_MAX_PATH];
    if(!GetSpecialFolderLocation(szPathLink, nFolder))
        return false;
    _tcscat_s(szPathLink, _MAX_PATH, _T("\\"));
    _tcscat_s(szPathLink, _MAX_PATH, pszLinkFilename);

    return CreateShortcut(szPathLink,
                            pszExePath, pszArguments,
                            pszWorkingDirectory, pszDescription, 
                            pszIconPath, iconIndex);
}


/////////////
// Installation et desinstallation de l'extension shell
// Add/Remove the following registry keys:
//        HKCR\REGKEY_SHELLEXTCLSID\ = "Cracklock"
//        HKCR\REGKEY_SHELLEXTCLSID_INPROCSRV\ = "CL_APP_PATH\Bin\ShellExtension.dll"
//        HKCR\REGKEY_SHELLEXTCLSID_INPROCSRV\REGVALUE_THREADINGMODEL = "Apartment"
//        HKCR\REGKEY_PROPSHEETHANDLER\ = SHELLEX_GUID
//        HKCR\REGKEY_CONTEXTMENUHANDLER\ = SHELLEX_GUID
//        HKLM\REGKEY_SHELLEXTENSIONAPPROVED\ = "Cracklock"
bool InstallShellExt()
{
    HKEY hKey;

    if( RegCreateKeyEx( HKEY_CLASSES_ROOT ,
                        REGKEY_SHELLEXTCLSID, 0, _T(""), REG_OPTION_NON_VOLATILE, 
                        KEY_ALL_ACCESS, NULL, &hKey, NULL)
        != ERROR_SUCCESS )
        return false;
    RegSetValueEx(hKey, NULL, 0, REG_SZ, (PBYTE)&TITRE_APPLI, sizeof(TCHAR)*(_tcslen(TITRE_APPLI)+1));
    RegCloseKey(hKey);


    if( RegCreateKeyEx( HKEY_CLASSES_ROOT ,
                        REGKEY_SHELLEXTCLSID_INPROCSRV, 0, _T(""), REG_OPTION_NON_VOLATILE, 
                        KEY_ALL_ACCESS, NULL, &hKey, NULL)
            != ERROR_SUCCESS )
        return false;
    
    TCHAR szShellExtDll[_MAX_PATH];
    _tcscpy_s(szShellExtDll, g_szCLPath);
    _tcscat_s(szShellExtDll, _T("\\Bin\\"));
    _tcscat_s(szShellExtDll, CL_SHELLEXT_DLL);
    RegSetValueEx(hKey, NULL, 0, REG_SZ, (PBYTE)&szShellExtDll, sizeof(TCHAR)*(_tcslen(szShellExtDll)+1));
    RegSetValueEx(hKey, REGVALUE_THREADINGMODEL, 0, REG_SZ, (PBYTE)&REGDATA_APARTMENT,
            sizeof(TCHAR)*(_tcslen(REGDATA_APARTMENT)+1));
    RegCloseKey(hKey);


    if( RegCreateKeyEx( HKEY_CLASSES_ROOT ,
                        REGKEY_PROPSHEETHANDLER, 0, _T(""), REG_OPTION_NON_VOLATILE, 
                        KEY_ALL_ACCESS, NULL, &hKey, NULL)
            != ERROR_SUCCESS )
        return false;
    RegSetValueEx(hKey, NULL, 0, REG_SZ, (PBYTE)&SHELLEX_GUID, sizeof(TCHAR)*(_tcslen(SHELLEX_GUID)+1));
    RegCloseKey(hKey);


    if( RegCreateKeyEx( HKEY_CLASSES_ROOT ,
                        REGKEY_CONTEXTMENUHANDLER, 0, _T(""), REG_OPTION_NON_VOLATILE, 
                        KEY_ALL_ACCESS, NULL, &hKey, NULL)
            != ERROR_SUCCESS )
        return false;
    RegSetValueEx(hKey, NULL, 0, REG_SZ, (PBYTE)&SHELLEX_GUID, sizeof(TCHAR)*(_tcslen(SHELLEX_GUID)+1));
    RegCloseKey(hKey);

    if( RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                        REGKEY_SHELLEXTENSIONAPPROVED, 0, _T(""),
                        REG_OPTION_NON_VOLATILE, 
                        KEY_ALL_ACCESS, NULL, &hKey, NULL)
        != ERROR_SUCCESS )
        return false;
    RegSetValueEx(hKey, SHELLEX_GUID, 0, REG_SZ, (PBYTE)&TITRE_APPLI, sizeof(TCHAR)*(_tcslen(TITRE_APPLI)+1));
    RegCloseKey(hKey);

    return true;
}

bool UninstallShellExt()
{
    HKEY hKey;

    if( RegCreateKeyEx( HKEY_CLASSES_ROOT ,
                        REGKEY_SHELLEXTCLSID_INPROCSRV, 0, _T(""), REG_OPTION_NON_VOLATILE, 
                        KEY_ALL_ACCESS, NULL, &hKey, NULL)
                        == ERROR_SUCCESS ) {
        RegDeleteValue(hKey, NULL);
        RegDeleteValue(hKey, REGVALUE_THREADINGMODEL);
        RegCloseKey(hKey);
        RegDeleteKey(HKEY_CLASSES_ROOT, REGKEY_SHELLEXTCLSID_INPROCSRV);
    }

    if( RegCreateKeyEx( HKEY_CLASSES_ROOT ,
                        REGKEY_SHELLEXTCLSID, 0, _T(""), REG_OPTION_NON_VOLATILE, 
                        KEY_ALL_ACCESS, NULL, &hKey, NULL)
                        == ERROR_SUCCESS ) {
        RegDeleteValue(hKey, NULL);
        RegCloseKey(hKey);
        RegDeleteKey(HKEY_CLASSES_ROOT, REGKEY_SHELLEXTCLSID);
    }

    if( RegCreateKeyEx( HKEY_CLASSES_ROOT ,
                        REGKEY_PROPSHEETHANDLER, 0, _T(""), REG_OPTION_NON_VOLATILE, 
                        KEY_ALL_ACCESS, NULL, &hKey, NULL)
                        == ERROR_SUCCESS ) {
        RegDeleteValue(hKey, NULL);
        RegCloseKey(hKey);
        RegDeleteKey(HKEY_CLASSES_ROOT, REGKEY_PROPSHEETHANDLER);
    }

    if( RegCreateKeyEx( HKEY_CLASSES_ROOT ,
                        REGKEY_CONTEXTMENUHANDLER, 0, _T(""), REG_OPTION_NON_VOLATILE, 
                        KEY_ALL_ACCESS, NULL, &hKey, NULL)
                        == ERROR_SUCCESS ) {
        RegDeleteValue(hKey, NULL);
        RegCloseKey(hKey);
        RegDeleteKey(HKEY_CLASSES_ROOT, REGKEY_CONTEXTMENUHANDLER);
    }

    if( RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                        REGKEY_SHELLEXTENSIONAPPROVED, 0, _T(""),
                        REG_OPTION_NON_VOLATILE, 
                        KEY_ALL_ACCESS, NULL, &hKey, NULL)
            == ERROR_SUCCESS ) {
        RegDeleteValue(hKey, NULL);
        RegCloseKey(hKey);
        RegDeleteKey(HKEY_CLASSES_ROOT, REGKEY_SHELLEXTENSIONAPPROVED);
    }

    return true;
}


void InstallCracklockInPATHEnvVar( bool bInstall )
{
    TCHAR szCLBinPath[_MAX_PATH];
    GetCLBinPath(szCLBinPath, _countof(szCLBinPath));

    if( URSSV_SUCCESS == UpdateRegistrySeparatedStringValueNT(HKEY_CURRENT_USER, 
            REGKEY_ENVIRONMENT, REGVALUE_ENVVAR_PATH, szCLBinPath, 
            bInstall ? EnvVarAddValue : EnvVarDeleteValue, _T(";") ) ) {
        // Brodcaste un message signalant un changement de variable d'environment
            DWORD_PTR dwReturnValue = 0;
            SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
                (LPARAM) REGKEY_ENVIRONMENT, SMTO_ABORTIFHUNG, 5000,
                &dwReturnValue);
    }
}


bool IsShellExtInstalled ()
{
    HKEY hKey;
    if( RegOpenKeyEx(HKEY_CLASSES_ROOT, REGKEY_SHELLEXTCLSID_INPROCSRV, 0, KEY_ALL_ACCESS, &hKey)
        != ERROR_SUCCESS )
        return false;
    
    DWORD dwType, dwSize;
    bool ret = RegQueryValueEx (hKey, NULL, 0, &dwType, NULL, &dwSize) == ERROR_SUCCESS 
            && dwType ==REG_SZ && dwSize > 0;

    RegCloseKey(hKey);
    return ret;
}



// copie le fichier src vers le fichier target en remplacant target si il est deja present.
// Si bAfterRestart=true alors la copie se fera au prochain reboot de la machine
bool CopyFileReboot(PCTSTR src, PCTSTR target, bool bAfterRestart)
{
    if(bAfterRestart) {
		if ( g_dwPlatformId == VER_PLATFORM_WIN32_NT )
            return MoveFileEx(src, target, MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT) ? true : false;
		else if ( g_dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ) {
            TCHAR	target_short[_MAX_PATH],
                    source_short[_MAX_PATH];

			// Obtient le nom court du fichier destination
			GetShortPathName(target, target_short, _MAX_PATH);
			// Obtient le nom court du fichier source
			GetShortPathName(src, source_short, _MAX_PATH);

			// Inscrit dans WININIT.INI une entré de la forme
			// C:\APP\DST~1.EXE = C:\WINDOWS\TEMP\SRC~1.TMP
			WritePrivateProfileString(_T("Rename"), target_short, source_short, g_szWinInitFile);

			// Inscrit dans la registry une entrée de la forme
			// "C:\APP\DST NOM DE FICHIER LONG.EXE" = C:\APP\DST~1.EXE
			SettingSection sec;
			if(sec.Open(REGKEY_MBCL_RENAME))
				sec.SetValue(target, target_short);

            return true;
		}
        return false;
    }
    else
        return CopyFile( src, target, FALSE) ? true : false;  
}

// Supprime un fichier. Si ce n'est pas possible et que bAfterRestart=true alors la suppression 
// est programme pour le prochain redemarrage de la machine
bool DeleteFileReboot(PCTSTR filename, bool bProceedAfterReboot)
{
    if(bProceedAfterReboot) {
		if ( g_dwPlatformId == VER_PLATFORM_WIN32_NT )
            return MoveFileEx(filename, NULL, MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT) ? true : false;
		else if ( g_dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ) {
			// Obtient le nom court du fichier source
            TCHAR source_short[_MAX_PATH];
			GetShortPathName(filename, source_short, _MAX_PATH);

			// Inscrit dans WININIT.INI une entrée de la forme
			// = C:\WINDOWS\TEMP\SRC~1.TMP
			return 0!=WritePrivateProfileString(_T("Rename"), _T(""), source_short, g_szWinInitFile);

        }
        return false;
    }
    else
        return DeleteFile( filename) ? true : false;
}



// Remplacce la DLL 'targetDll' par 'srcDLL'.
// Si la DLL 'targetDLL' existe deja alors est est remplacee par 'targetDLL' si et seulement si son numero de version est inferieur.
// Si la DLL 'targetDll' n'existe pas alors la DLL 'srcDLL' est copiee vers 'targetDLL'.
// Retourne TRUE si le fichier a pu etre copie ou mis a jour.

// Version based on VerInstallFile
// Pb: this version tries to open targetDLL with write access even if the file does not need to be updated!
bool UpdateDll_VIF(LPCTSTR srcDLL, LPCTSTR targetDLL)
{
    TCHAR srcDir[_MAX_PATH];
    _tcscpy_s(srcDir,srcDLL);
    TruncateFilenameToFileDirectory(srcDir);

    TCHAR srcBasename[_MAX_PATH];
    _tcscpy_s(srcBasename, GetFileBaseNamePart(srcDLL));

    TCHAR dstDir[_MAX_PATH];
    _tcscpy_s(dstDir,targetDLL);
    TruncateFilenameToFileDirectory(dstDir);
    
    TCHAR dstBasename[_MAX_PATH];
    _tcscpy_s(dstBasename, GetFileBaseNamePart(targetDLL));

    TCHAR tmpFile [_MAX_PATH];
    UINT tmpLen= _MAX_PATH;
    

    DWORD dwRet = VerInstallFile(VIFF_DONTDELETEOLD, srcBasename, dstBasename, srcDir,
                dstDir, _T(""), tmpFile, &tmpLen );
    
    // Copied successfuly?
    if ( dwRet == 0 )
        return TRUE;

    // destination file locked?
    else if( dwRet & (VIF_FILEINUSE | VIF_CANNOTDELETE) ) {
        // do nothing
    }
    
    // other problems (for instance the version differs) ?
    else if( dwRet & (VIF_CANNOTREADDST|VIF_MISMATCH ) ) {
        
        if ( dwRet&VIF_SRCOLD )
            return TRUE;

        // then just overwrite the destination dll (even if the source is older)
        else if( 0 == (dwRet = VerInstallFile(VIFF_FORCEINSTALL | VIFF_DONTDELETEOLD, srcBasename, dstBasename,
                srcDir, dstDir,_T("") , tmpFile, &tmpLen )) )
            return TRUE;
    }

    // delete any temporary file created
    if  ( dwRet & VIF_TEMPFILE ) {
        TCHAR delFile[_MAX_PATH];
        _tcscpy_s(delFile, dstDir);
        _tcscat_s(delFile, tmpFile );
        DeleteFile(delFile);
    }
    return FALSE;
}


// Version based on GetFileVersionInfo
// This version does not try to open open dstDLL with write access if the file does not need to be updated.
// Note:in this implementation we have to implement the version number test by ourself.
bool UpdateDll(PCTSTR srcDLL, PCTSTR dstDLL, bool bProceedAfterReboot)
{
    DWORD dw;
    bool bNeedCopy = false; // do we need to copy the file?

    DWORD sizeBlockDst = GetFileVersionInfoSize( dstDLL, &dw);
    DWORD sizeBlockSrc = GetFileVersionInfoSize( srcDLL, &dw);
    if( sizeBlockDst == 0 )
        bNeedCopy = true;
    else {
        LPVOID dstBlock = malloc( sizeBlockDst );
        LPVOID srcBlock = malloc( sizeBlockSrc );
        
        if(    GetFileVersionInfo(srcDLL, NULL, sizeBlockSrc, srcBlock)
            && GetFileVersionInfo(dstDLL, NULL, sizeBlockDst, dstBlock) )
        {
            VS_FIXEDFILEINFO *vsffi_src, *vsffi_dst;
            UINT size_src, size_dst;
            if( VerQueryValue(srcBlock, _T("\\"), (LPVOID *)&vsffi_src, &size_src)
                && VerQueryValue(dstBlock, _T("\\"), (LPVOID *)&vsffi_dst, &size_dst) ) {

                // Copy if and only the version of the destination file is older than the source
                bNeedCopy = vsffi_src->dwFileVersionMS > vsffi_dst->dwFileVersionMS 
                            || (vsffi_src->dwFileVersionMS == vsffi_dst->dwFileVersionMS
                                &&  vsffi_src->dwFileVersionLS > vsffi_dst->dwFileVersionLS);
            }
        }
        else
            bNeedCopy = true;

        free( srcBlock );
        free( dstBlock );
    }

    if( bNeedCopy ) {
		return CopyFileReboot(srcDLL, dstDLL, bProceedAfterReboot ) ? true : false;
    }
    else
        return true;
}


// Quitte CL_INJECTOR_EXE en signalant le semaphore
void KillResidantInjector()
{
    HANDLE hSemaphore = CreateSemaphore(NULL, 0, 1, CL_INJECTOR_EXE);
    ReleaseSemaphore(hSemaphore, 1, NULL);
    CloseHandle(hSemaphore);
}

// Copy the list of applications settings from location 'source' to location 'destination'
bool CopyAppsSettings(SettingsStorageLocation source, SettingsStorageLocation destination)
{
    if(source == destination)
        return true;

    SettingSection src_sec(SectionLocationOfSettingsLocation(source));
    if( !src_sec.Open(REGKEY_MBCL_APPS) )
        return false;

    // create the destination section
    SettingSection dst_sec(SectionLocationOfSettingsLocation(destination));
    if( !dst_sec.Create(REGKEY_MBCL_APPS) )
        return false;

    return src_sec.CopyTo(dst_sec, true);
}

// import settings from all possible locations into the current one
int ImportSettingsFromOtherLocations( SettingsStorageLocation dstloc )
{
    // Import application settings
    CopyAppsSettings(WinReg, dstloc);
    CopyAppsSettings(AppDataINI, dstloc);
    CopyAppsSettings(BinINI, dstloc);

    return 0;
}


// Installation de Cracklock
int CracklockSetStorageLocation( SettingsStorageLocation newloc )
{
    // Import application settings
    ImportSettingsFromOtherLocations(newloc);
    
    // Change the current location
    return ChangeSettingsLocation(newloc) ? 0 : 1;
}


// Desinstallation de Cracklock
int CracklockUninstall( void )
{
    // Quitte CL_INJECTOR_EXE en signalant le semaphore
    KillResidantInjector();
    
    // Supprime les entrees de la base de registre chargee d'activer le mode system-wide
    UninstallSystemwideInjection();

    // Desinstalle les extensions de l'explorer
    UninstallShellExt();

    // Supprime Cracklock de la variable d'environment PATH
    InstallCracklockInPATHEnvVar(false);

    // Message : "Faut-il virer toutes les configurations des applications?"
    g_hResDll = LoadResDll(true);
    PTSTR pszQuest = LoadResString(g_hResDll, IDS_DELREGSETTINGS);
    if( MessageBox(NULL, pszQuest, SETUP_TITLE, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES ) {        
		// si oui alors efface les settings dans tous les endroits possibles
		DeleteCLSettings(WinReg);
		DeleteCLSettings(AppDataINI);
		DeleteCLSettings(BinINI);
    }
    FreeResString(pszQuest);
    return 0;
}

