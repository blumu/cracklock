//  MODULE:    DllMain.cpp
//
//  PURPOSE:   fonctions export� par la dll
//
//

#include "StdAfx.h"			// Header pr�compil�
#include "..\Resources\Resources.h"	// Resources localis�s
#include "TimeCtrl.h"		// Contr�les temps/date
#include "ShellExt.h"		// D�finition de la classe CShellExt
#include "DLLMain.h"		// Prototypes des fonctions de DLLMain


//
// Global variables
//
UINT		g_cRefThisDll = 0;				// Reference count of this DLL.
HINSTANCE	g_hmodThisDll = NULL;			// Handle to this DLL itself.
HINSTANCE	g_hResDll = NULL;				// handle de la DLL contenant les resources internationales
TCHAR		g_szWinInitFile[_MAX_PATH];		// Chemin d'acc�s au fichier WININIT.INI de windows
TCHAR		g_szCLPath[_MAX_PATH];			// Chemin d'acc�s au r�pertoire o� est install� cracklock


DWORD		g_dwPlatformId;				// Version de Windows
PTSTR       g_S_TABGENERAL,					// titres des onglets
            g_S_TABOPTIONS,
            g_S_TABMOREOPTIONS,
            g_S_TABDEPEND,
            g_S_TABHELP,
            g_S_ADVANCED;								// l�gende du bouton avanc�

HBRUSH		g_hBrSYSCOLOR_WINDOW;		// Pinceau pour le fond des ctrls EDIT
HBRUSH		g_hBrSYSCOLOR_3DFACE;		// Pinceau pour le fond des ctrls EDIT
BOOL		g_b24Hours;					// Mode 24 heure / 12 heures
BOOL		g_bLeadingHour;				// Leading zeros pour l'heure affich�z OUI/NON

// Initialise les couleurs utilis�s par Cracklock � partir des couleurs syst�mes
void InitColor()
{
    LOGBRUSH lb;
	
    lb.lbStyle = BS_SOLID;
    lb.lbHatch = NULL;

    lb.lbColor = GetSysColor(COLOR_WINDOW);
    if( g_hBrSYSCOLOR_WINDOW )
        DeleteObject(g_hBrSYSCOLOR_WINDOW);

    g_hBrSYSCOLOR_WINDOW = CreateBrushIndirect(&lb);

    lb.lbColor = GetSysColor(COLOR_3DFACE);
    if( g_hBrSYSCOLOR_3DFACE )
        DeleteObject(g_hBrSYSCOLOR_3DFACE);

    g_hBrSYSCOLOR_3DFACE = CreateBrushIndirect(&lb);
}

// D�sinitialise les couleurs utilis�s par Cracklock
void UninitColor()
{
    DeleteObject(g_hBrSYSCOLOR_WINDOW);
    DeleteObject(g_hBrSYSCOLOR_3DFACE);
}



extern "C" int APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, PVOID lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            OSVERSIONINFO versInfo;

            g_hmodThisDll = hInstance;

            // Initialise le moteur de settings (INI/Registry)
            InitCommon(hInstance);

            // Charge en m�moire la dll contenant les resources localis�es
            if( !OnLanguageChanged() )
                return 0;

            // Obtient le r�pertoire o� est install� Cracklock
            GetCLPath(g_szCLPath, _countof(g_szCLPath));

            // Infos sur la version de Windows
            versInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
            GetVersionEx(&versInfo);
            g_dwPlatformId = versInfo.dwPlatformId;

            // Cr�e le chemin d'acc�s au fichier WININIT.INI
            GetWindowsDirectory(g_szWinInitFile, _MAX_PATH);
            _tcscat_s(g_szWinInitFile, _T("\\WININIT.INI"));

            g_hBrSYSCOLOR_WINDOW =
                g_hBrSYSCOLOR_3DFACE = NULL;

            g_b24Hours =
                g_bLeadingHour = FALSE;

            // Cr�e les pinceaux
            InitColor();

            // Enregistre les contr�les d'affichage heure/date
            RegTimeCtrl(hInstance);

            break;

        case DLL_PROCESS_DETACH:
            // D�senregistre les contr�les temps
            UnregTimeCtrl(hInstance);

            // lib�re les handles des pinceaux
            UninitColor();

            // Lib�re la m�moire allou�e par la dll des ressources
            FreeResDll();

            break;
    }
    return 1;
}

// DllCanUnloadNow
STDAPI DllCanUnloadNow(void)
{
    return (g_cRefThisDll == 0 ? S_OK : S_FALSE);
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, PVOID *ppvOut)
{
    *ppvOut = NULL;

    if (IsEqualIID(rclsid, CLSID_ShellExtension))
    {
        CShellExtClassFactory *pcf = new CShellExtClassFactory;
        return pcf->QueryInterface(riid, ppvOut);
    }
    return CLASS_E_CLASSNOTAVAILABLE;
}
