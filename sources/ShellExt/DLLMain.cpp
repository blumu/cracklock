//  MODULE:    DllMain.cpp
//
//  PURPOSE:   fonctions exporté par la dll
//
//

#include "StdAfx.h"			// Header précompilé
#include "..\Resources\Resources.h"	// Resources localisés
#include "TimeCtrl.h"		// Contrôles temps/date
#include "ShellExt.h"		// Définition de la classe CShellExt
#include "DLLMain.h"		// Prototypes des fonctions de DLLMain


//
// Global variables
//
UINT		g_cRefThisDll = 0;				// Reference count of this DLL.
HINSTANCE	g_hmodThisDll = NULL;			// Handle to this DLL itself.
HINSTANCE	g_hResDll = NULL;				// handle de la DLL contenant les resources internationales
TCHAR		g_szWinInitFile[_MAX_PATH];		// Chemin d'accès au fichier WININIT.INI de windows
TCHAR		g_szCLPath[_MAX_PATH];			// Chemin d'accès au répertoire où est installé cracklock


DWORD		g_dwPlatformId;				// Version de Windows
PTSTR       g_S_TABGENERAL,					// titres des onglets
            g_S_TABOPTIONS,
            g_S_TABMOREOPTIONS,
            g_S_TABDEPEND,
            g_S_TABHELP,
            g_S_ADVANCED;								// légende du bouton avancé

HBRUSH		g_hBrSYSCOLOR_WINDOW;		// Pinceau pour le fond des ctrls EDIT
HBRUSH		g_hBrSYSCOLOR_3DFACE;		// Pinceau pour le fond des ctrls EDIT
BOOL		g_b24Hours;					// Mode 24 heure / 12 heures
BOOL		g_bLeadingHour;				// Leading zeros pour l'heure affichéz OUI/NON

// Initialise les couleurs utilisés par Cracklock à partir des couleurs systèmes
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

// Désinitialise les couleurs utilisés par Cracklock
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

            // Charge en mémoire la dll contenant les resources localisées
            if( !OnLanguageChanged() )
                return 0;

            // Obtient le répertoire où est installé Cracklock
            GetCLPath(g_szCLPath, _countof(g_szCLPath));

            // Infos sur la version de Windows
            versInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
            GetVersionEx(&versInfo);
            g_dwPlatformId = versInfo.dwPlatformId;

            // Crée le chemin d'accès au fichier WININIT.INI
            GetWindowsDirectory(g_szWinInitFile, _MAX_PATH);
            _tcscat_s(g_szWinInitFile, _T("\\WININIT.INI"));

            g_hBrSYSCOLOR_WINDOW =
                g_hBrSYSCOLOR_3DFACE = NULL;

            g_b24Hours =
                g_bLeadingHour = FALSE;

            // Crée les pinceaux
            InitColor();

            // Enregistre les contrôles d'affichage heure/date
            RegTimeCtrl(hInstance);

            break;

        case DLL_PROCESS_DETACH:
            // Désenregistre les contrôles temps
            UnregTimeCtrl(hInstance);

            // libère les handles des pinceaux
            UninitColor();

            // Libère la mémoire allouée par la dll des ressources
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
