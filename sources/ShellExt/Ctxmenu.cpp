// MODULE:  Ctxmenu.cpp
//
// PURPOSE:  Implements the IContextMenu member functions necessary to support
//    the context menu portioins of this shell extension.  Context menu
//    shell extensions are called when the user right clicks on a file
//    (of the type registered for the shell extension)

#include "StdAfx.h"   // Header précompilé
#include "..\Resources\Resources.h" // Resources localisés
#include "ShellExt.h"  // Définition de la class CShellExt

//
//  FUNCTION: CShellExt::QueryContextMenu(HMENU, UINT, UINT, UINT, UINT)
//
//  PURPOSE: Called by the shell just before the context menu is displayed.
//           This is where you add your specific menu items.
//
//  PARAMETERS:
//    hMenu      - Handle to the context menu
//    indexMenu  - Index of where to begin inserting menu items
//    idCmdFirst - Lowest value for new menu ID's
//    idCmtLast  - Highest value for new menu ID's
//    uFlags     - Specifies the context of the menu event
//
//  RETURN VALUE:
//
//
//  COMMENTS:
//

STDMETHODIMP CShellExt::QueryContextMenu(HMENU hMenu,
        UINT indexMenu,
        UINT idCmdFirst,
        UINT idCmdLast,
        UINT uFlags)
{
    UINT idCmd = idCmdFirst;
    /*InsertMenu(hMenu,
               indexMenu++,
               MF_STRING|MF_BYPOSITION,
               idCmd++,
               _T("TOTO")); 
    return MAKE_SCODE(SEVERITY_SUCCESS, 0, (USHORT)(1));*/

	TCHAR  szFile[_MAX_PATH];
    if( !GetFileName(szFile, _MAX_PATH) )
        return NOERROR;

    if( !SetFile(szFile) || m_bDLL ) // Charge le fichier
        return NOERROR;

	bool bRunTimeInjection = false;
    SettingSection *sec = CLCommonSettingSection::GetSectionOfApp( szFile, false, false);
    if( sec ) {
        // verifie que l'application est en mode run-time
		TCHAR szLoaderFile[_MAX_PATH];
		bRunTimeInjection = !m_bCracked
                            && (!sec->GetValue(REGVALUE_PROG_LOADERFILE, szLoaderFile, _countof(szLoaderFile))
						        || szLoaderFile[0] == '\0'); // mode runtime?
		CLCommonSettingSection::deleteObject(sec);
    }

	if( !(uFlags & CMF_DEFAULTONLY) ) {
		TCHAR  *pszMenuText = LoadResString(g_hResDll, IDS_CTXMENU); // "start with Cracklock" menu
        InsertMenu(hMenu,
                   indexMenu++,
                   MF_STRING|MF_BYPOSITION,
                   idCmd++,
                   pszMenuText);
        FreeResString(pszMenuText);

        // Must return number of menu items that we have added.
        return MAKE_SCODE(SEVERITY_SUCCESS, 0, (USHORT)(idCmd-idCmdFirst));
    }

    return NOERROR;
}

//
//  FUNCTION: CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO)
//
//  PURPOSE: Called by the shell after the user has selected on of the
//           menu items that was added in QueryContextMenu().
//
//  PARAMETERS:
//    lpcmi - Pointer to an CMINVOKECOMMANDINFO structure
//
//  RETURN VALUE:
//
//
//  COMMENTS:
//

STDMETHODIMP CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
    HRESULT hr = E_INVALIDARG;

    //If HIWORD(lpcmi->lpVerb) then we have been called programmatically
    //and lpVerb is a command that should be invoked.  Otherwise, the shell
    //has called us, and LOWORD(lpcmi->lpVerb) is the menu ID the user has
    //selected.  Actually, it's (menu ID - idCmdFirst) from QueryContextMenu().
    if (!HIWORD(lpcmi->lpVerb))
    {
        UINT idCmd = LOWORD(lpcmi->lpVerb);

        if( idCmd == 0)
        {
            PROCESS_INFORMATION ProcessInformation;
            STARTUPINFO   startupInfo;
            TCHAR      szTemp[2*_MAX_PATH];

            // Ligne de commande complète avec le nom du fichier EXE entre guillemets
            StringCchPrintf (szTemp, _countof(szTemp), _T("%s\\Bin\\%s \"%s\""), g_szCLPath, CL_LOADER_EXE, m_szPEFileName);

            // Exécute Cracklock Loader
            memset(&startupInfo, 0, sizeof(startupInfo));
            startupInfo.cb = sizeof(startupInfo);
            startupInfo.wShowWindow = lpcmi->nShow;
            CreateProcess( 0,     // lpszImageName
                           szTemp,    // lpszCommandLine
                           0,     // lpsaProcess
                           0,     // lpsaThread
                           FALSE,    // fInheritHandles
                           0,     // fdwCreate
                           0,     // lpvEnvironment
                           0,     // lpszCurDir
                           &startupInfo,  // lpsiStartupInfo
                           &ProcessInformation // lppiProcInfo
                         );

            hr = NOERROR;
        }
    }
    return hr;
}

STDMETHODIMP CShellExt::GetCommandString(UINT_PTR idCommand,
        UINT uFlags,
        UINT *reserved,
        LPSTR pszName,
        UINT uMaxNameLen)
{
    TCHAR *pthelp = LoadResString(g_hResDll, IDS_CTXMENUHELP);

    HRESULT  hr = E_INVALIDARG;

    if(idCommand != 0)
    {
        return hr;
    }

    switch( uFlags )
    {
        /*
         case GCS_HELPTEXTA:
                hr = StringCchCopyNA(pszName, _tcslen(pszStr)/sizeof(pszStr(0)),
                                    "Display File Name", uMaxNameLen);
                break;
            case GCS_VERBA:
                hr = StringCchbCopyNA(pszName, _tcslen(pszStr)/sizeof(pszStr(0)),
                                    m_pszVerb, uMaxNameLen);
                break;
        */
        case GCS_HELPTEXTW:
        _tcscpy_s((PWSTR)pszName, uMaxNameLen, pthelp);
        break;

        case GCS_VERBW:
        _tcscpy_s((PWSTR)pszName, uMaxNameLen, _T("Cracklock"));
        break;
        case GCS_VERBA:
        strcpy_s((PSTR)pszName, uMaxNameLen, "Cracklock");
        break;


        default:
        hr = S_OK;
        break;
    }

    FreeResString(pthelp);
    return hr;
}
