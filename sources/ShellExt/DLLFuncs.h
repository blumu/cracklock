//  MODULE:    Dllfuncs.h
//  Ce module contient les definitions des fonctions exportees par la dll CL_SHELLEXT_DLL.
//  ShellExtension est importe par le Manager.
#pragma once

#include "impexp.h"
#include "..\Common\INIReg.h"

// Valeurs pour le parametre 'mode' de la fonction InstallSystemwideInjection et le type de retour de IsSystemwideInjectionSet
#define SYSTEMWIDE_NONE                     0x00
#define SYSTEMWIDE_APPINIT                  0x01
#define SYSTEMWIDE_INJECTOR_AT_CULOGON      0x02
#define SYSTEMWIDE_INJECTOR_AT_LMLOGON      0x04
#define SYSTEMWIDE_DEFAULT                    -1






//////////
// Prototypes

DECLSPECIFIER void CALLBACK RestoreLFN(HWND hwnd, HINSTANCE hinst, PSTR pszCmdLine, int nCmdShow);

DECLSPECIFIER void InvokeHelp(HWND hwnd, DWORD dwID);

DECLSPECIFIER BOOL OnLanguageChanged (void);

DECLSPECIFIER void CleanupSettingsEntries();

DECLSPECIFIER PTSTR GetFileExtPart( PTSTR, SIZE_T, PTSTR );
DECLSPECIFIER void TruncateFilenameToFileDirectory(LPTSTR fullpath);

DECLSPECIFIER void ErrorReport();

DECLSPECIFIER void GetFileTimeByName( PCTSTR lpcszFileName, LPFILETIME pftCreation, LPFILETIME pftLastAccess, LPFILETIME pftLastWrite );
DECLSPECIFIER void SetFileTimeByName( PCTSTR lpcszFileName, LPFILETIME pftCreation, LPFILETIME pftLastAccess, LPFILETIME pftLastWrite );

DECLSPECIFIER void ShowSettingsDlg(HWND parent);



DECLSPECIFIER int InstallSystemwideInjection( int mode = SYSTEMWIDE_DEFAULT );
DECLSPECIFIER int UninstallSystemwideInjection( int mode = SYSTEMWIDE_DEFAULT );
DECLSPECIFIER int GetSystemwideInjectionMode();

DECLSPECIFIER void RajouteAuTitre(HWND hwnd, PCTSTR pcsz);
DECLSPECIFIER BOOL CALLBACK EnumChildSetFont(HWND hwnd, LPARAM lParam);

DECLSPECIFIER void FreeResDll( void );

DECLSPECIFIER COLORREF GetHotLinkColor();

DECLSPECIFIER bool CreateShortcut(PCTSTR pszLinkPath,
                    PCTSTR pszExePath, PCTSTR pszArguments,                        
                    PCTSTR pszWorkingDirectory, PCTSTR pszDescription, 
                    PCTSTR pszIconPath, int iconIndex);

DECLSPECIFIER bool GetSpecialFolderLocation(LPTSTR path, int nFolder);
DECLSPECIFIER bool CreateShortcut(int nFolder, PCTSTR pszLinkFilename,
                    PCTSTR pszExePath, PCTSTR pszArguments, 
                    PCTSTR pszWorkingDirectory, PCTSTR pszDescription, 
                    PCTSTR pszIconPath, int iconIndex);

DECLSPECIFIER bool InstallShellExt();
DECLSPECIFIER bool UninstallShellExt();
DECLSPECIFIER bool IsShellExtInstalled();

DECLSPECIFIER void InstallCracklockInPATHEnvVar( bool bInstall );

DECLSPECIFIER bool CopyFileReboot(PCTSTR src, PCTSTR target, bool bAfterRestart);
DECLSPECIFIER bool DeleteFileReboot(PCTSTR filename, bool bProceedAfterReboot);
DECLSPECIFIER bool UpdateDll(PCTSTR srcDll, PCTSTR targetDir, bool bProceedAfterReboot);

DECLSPECIFIER void KillResidantInjector();

DECLSPECIFIER bool IsCursorOnControl(HWND hDlg, UINT idCtrl);

DECLSPECIFIER int CracklockUninstall( void );
DECLSPECIFIER int CracklockSetStorageLocation( SettingsStorageLocation newloc );
DECLSPECIFIER int ImportSettingsFromOtherLocations( SettingsStorageLocation dstloc );

