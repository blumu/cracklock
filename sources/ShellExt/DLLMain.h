#pragma once

#include "impexp.h"


//
// Global variables
//
DATADECLSPECIFIER UINT              g_cRefThisDll;					// Reference count of this DLL.
DATADECLSPECIFIER HINSTANCE         g_hmodThisDll;					// Handle to this DLL itself.
DATADECLSPECIFIER HINSTANCE         g_hResDll;						// handle de la DLL contenant les resources internationales
DATADECLSPECIFIER TCHAR             g_szWinInitFile[_MAX_PATH];		// Chemin d'accès au fichier WININIT.INI de windows
DATADECLSPECIFIER TCHAR             g_szCLPath[_MAX_PATH];			// Chemin d'accès au répertoire où est installé cracklock
DATADECLSPECIFIER PTSTR             g_S_TABGENERAL,					// titres des onglets		
                                    g_S_TABOPTIONS,
                                    g_S_TABMOREOPTIONS,
                                    g_S_TABDEPEND,
                                    g_S_TABHELP,
                                    g_S_ADVANCED;					// légende du bouton avancé

DATADECLSPECIFIER HBRUSH		g_hBrSYSCOLOR_WINDOW,		// Pinceau pour le fond des ctrls EDIT
								g_hBrSYSCOLOR_3DFACE;		// Pinceau pour le fond des ctrls EDIT

DATADECLSPECIFIER BOOL			g_b24Hours,					// Mode 24 heure / 12 heures
								g_bLeadingHour;				// Leading zeros pour l'heure affichéz OUI/NON

DATADECLSPECIFIER DWORD			g_dwPlatformId;				// Version de Windows



// Dllmain.h
void InitColor( void );
void UninitColor( void );
