// MANAGER.CPP

#define INC_OLE2        // WIN32, get ole2 from windows.h

#include <windows.h>
#include <commctrl.h>		// Contrôles communs
#include <stddef.h>			// Définition standard du C
#include <initguid.h>		// Outils d'initialisation de GUID
#include <shlguid.h>		// Définitions des GUIDs standards du Shell
#include <shlobj.h>			// Définitions des objets standards du Shell
#include "..\Resources\resources.h"	// Resources localisés
#include "..\Common\Common.h"		// Outils communs
#include "..\Common\ListView.h"	    // Outils pour le contrôle ListView
#include "..\ShellExt\DLLMain.h"	// Prototypes des fonctions de DLLMain
#include "..\ShellExt\Shellext.h"	// Définition des classes CShellExtClassFactory et CShellExt
#include "..\ShellExt\DllFuncs.h"	// Fonctions exportés par CL_SHELLEXT_DLL
#include "..\ShellExt\PETools.h"
#include "Resource.h"				// Resources
#include "about.h"
#include <tchar.h>
#include <strsafe.h>

//======================== Structures =======================================
class LVNodeParam
{
public:
    LPTSTR	pszFilename;    // full path to the PE file
    LPTSTR	pszFileDir;     // full access path
    LPTSTR	pszDateTime;    // formated string representing the date and time selected for this app
    AppSettings settings;   // application settings
	int     tziIndex;       // index de la timezone selectionne dans le champs settings.m_tzi
	bool	bAddItem;       // true si il s'agit de l'item special "Add program"
    LVNodeParam()
    {
        pszFilename = NULL;
        pszFileDir = NULL;
        pszDateTime = NULL;
    }
    ~LVNodeParam()
    {
        if(pszFilename) free(pszFilename);
	    if(pszFileDir) free(pszFileDir);
        if(pszDateTime) free(pszDateTime);
    }
};


// Structure passée dans le champs lParam los de la création de la
// boîte de dialogue de configuration
enum operation
{
    None,
    Activate,
    Deactivate
};
struct CONFIGPARAM
{
    PTSTR  pszFilename;
    operation op;
    bool bOpAndClose;
};



typedef  HRESULT (STDAPICALLTYPE  *PDLLGETCLASSOBJECT ) (REFCLSID, REFIID, void **);

//======================== Global Variables =================================

HINSTANCE			g_hInstance,
					g_hResDll;
HWND				g_hMain,
					g_hwndLV;
HIMAGELIST			g_hImgLstSml,
					g_hImgLstBig;

BOOL				g_bEnd;	// Determine si après la fermeture
							// de la fenêtre, il faut quitter l'appli
							// ou bien recharger la fenêtre avec
							// un langage différent

UINT			    g_iNbLang;	// Nombre de langues ajoutés au menu

HMENU				g_hmnuLang;	// handle du menu popup "Langage>"

// icon index in the image lists
int                 g_iIcoAddProgram,
	                g_iIcoAbsent,
	                g_iIcoDefault;


// pointeur sur l'objet de création d'objets CShellExt dont la classe
// se trouve dans CL_SHELLEXT_DLL
CShellExtClassFactory *g_pcf;

CTimeZoneInfoManager g_tzimgr;

// retourne l'item sélectionné dans un contrôle listview
#define ListView_GetSelectedItem(hwnd)		ListView_GetNextItem((hwnd), -1, LVNI_SELECTED)

///////////////
// clés dans la registry

// clé du style du ctrl Listview
#define REGVALUE_SETTINGS_LISTVIEW		_T("ListView")
// clé de la position et de la taille de la fenêtre principale
#define REGVALUE_SETTINGS_MAINRECT		_T("MainRect")



// error code returned through the EndDialog function by ConfigDlgProc
#define CONFIGRET_ERROR         0
#define CONFIGRET_NOCHANGE      1
#define CONFIGRET_SUCCESS       2



//======================== Prototypes =============================================

INT_PTR CALLBACK ConfigDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MainWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

//======================== Code =============================================


// Crée la classe de la fenêtre principale et la fenêtre elle-même
BOOL InitApplication ( int nCmdShow )
{
	WNDCLASSEX	wcx;
	TCHAR *pszTitle;

	// Prepare la structure de création de classe
	ZeroMemory(&wcx, sizeof(wcx));
	wcx.cbSize = sizeof(wcx);					// size of structure
	wcx.style = CS_HREDRAW | CS_VREDRAW;		// redraw if size changes
	wcx.lpfnWndProc = (WNDPROC)MainWndProc;		// points to window procedure
	wcx.cbClsExtra = 0;							// no extra class memory
	wcx.cbWndExtra = 0;							// no extra window memory
	wcx.hInstance = g_hInstance;				// handle of instance
	wcx.hIcon = LoadIcon(g_hInstance,
					MAKEINTRESOURCE(IDI_APP));	// predefined app. icon
	wcx.hCursor = LoadCursor(NULL, IDC_ARROW);	// predefined arrow
	wcx.hbrBackground = NULL;					// white background brush
	wcx.lpszMenuName =  NULL;					// name of menu resource
	wcx.lpszClassName = _T("MainWinClass");			// name of window class
	wcx.hIconSm = LoadIcon(g_hInstance,
					MAKEINTRESOURCE(IDI_APP));	// small class icon

	// Enregistre la 'window class'
	if( !RegisterClassEx(&wcx) )
		return FALSE;

	// Crée la fenêtre principale
	pszTitle = LoadResString(g_hResDll, IDS_MANAGERTITLE);


	//////////
	// Lit la taille de la fenêtre dans la registry

	// Ouvre la clé de Cracklock
	RECT rcWin;
	if( GetCracklockSettingValue(REGVALUE_SETTINGS_MAINRECT, &rcWin) ) {
        if( rcWin.right <= rcWin.left )
            rcWin.left = rcWin.right = CW_USEDEFAULT;
        else
            rcWin.right -= rcWin.left;
		
        if( rcWin.bottom <= rcWin.top )
            rcWin.bottom = rcWin.top = CW_USEDEFAULT;
        else
            rcWin.bottom -= rcWin.top;
	}
	else
	{
		rcWin.left = CW_USEDEFAULT;
		rcWin.top = CW_USEDEFAULT;
		rcWin.right = CW_USEDEFAULT;
		rcWin.bottom = CW_USEDEFAULT;
	}
	
	if( g_hResDll && pszTitle && g_hInstance)
		CreateWindowEx(WS_EX_ACCEPTFILES,
        _T("MainWinClass"),						// name of window class
				pszTitle,				// title-bar string
				WS_OVERLAPPEDWINDOW | WS_VISIBLE,	// top-level window
				rcWin.left,				// default horizontal position
				rcWin.top,				// default vertical position
				rcWin.right,				// default width
				rcWin.bottom,				// default height
				NULL,						// no owner window
				LoadMenu(g_hResDll,
				MAKEINTRESOURCE(IDM_PROGRAM)),	// use class menu
				g_hInstance,				// handle of application instance
				NULL);						// no window-creation data

	FreeResString(pszTitle);

	if (!g_hMain)
		return FALSE;

	SetMenuDefaultItem(GetSubMenu(GetMenu(g_hMain),0), ID_PROGRAM_CONFIG, FALSE);

	return TRUE;
}



int CracklockHelp ( void )
{
	// Charge les ressources localisées
	g_hResDll = LoadResDll(true);
	if( g_hResDll )
	{
		InvokeHelp(NULL, -1);

		// libère les ressources localisées
		FreeLibrary(g_hResDll);

		return 0;
	}
	else
		return 1;
}

int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow )
{
    MSG msg;
    g_hInstance = hInstance;

    CoInitialize(0);

    // Initilaise les contrôles commun
    INITCOMMONCONTROLSEX ic;
    ic.dwICC = ICC_TREEVIEW_CLASSES | ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES | ICC_LINK_CLASS;
    ic.dwSize = sizeof(ic);
    InitCommonControlsEx(&ic);

    // Charge la DLL contenant l'extension du shell
    PDLLGETCLASSOBJECT	shellex_DllGetClassObject =
        (PDLLGETCLASSOBJECT)
        GetProcAddress(
            LoadLibrary(CL_SHELLEXT_DLL),
            "DllGetClassObject");

    // appelle de la fonction DllGetClassObject de la librairie CL_SHELLEXT_DLL
    if( (shellex_DllGetClassObject)(CLSID_ShellExtension, IID_IClassFactory, (void **)&g_pcf) != NOERROR )
        return 0;

    if(strstr(lpszCmdLine, "-help") != 0)
        return CracklockHelp();

    if(strstr(lpszCmdLine, "-set-syswide") != 0)
        return InstallSystemwideInjection();

    // import settings from all possible location to current location
    if(strstr(lpszCmdLine, "-import") != 0)
        return ImportSettingsFromOtherLocations(GetSettingStorageLocation());

    // install cracklock with the settings stored in different location
    if(strstr(lpszCmdLine, "-set-storage-appdata") != 0)
        return CracklockSetStorageLocation(AppDataINI);
    if(strstr(lpszCmdLine, "-set-storage-bin") != 0)
        return CracklockSetStorageLocation(BinINI);
    if(strstr(lpszCmdLine, "-set-storage-winreg") != 0)
        return CracklockSetStorageLocation(WinReg);

    // add cracklock directory to the PATH envvar.
    if(strstr(lpszCmdLine, "-add-path") != 0) {
        InstallCracklockInPATHEnvVar(true);
        return 0;
    }
    // remove cracklock directory from the PATH envvar.
    if(strstr(lpszCmdLine, "-remove-path") != 0) {
        InstallCracklockInPATHEnvVar(false);
        return 0;
    }
    
    // uninstall cracklock
    if(strstr(lpszCmdLine, "-uninstall") != 0)
        return CracklockUninstall();


    do // début de la boucle de changement de langage
    {
        // Si le language n'est pas changé pendant la boucle
        // de traitement des messages (pendant l'utilisation du manager)
        // la boucle de changement de langage se terminera
        //
        // Si un changement de langage à lieu entre temps, dans ce
        // cas : la boucle de chngm de langage réiterera afin d'obtenir
        // une fenêtre dans le langage désiré
        g_bEnd = TRUE;

        // Charge les ressources localisées
        g_hResDll = LoadResDll(true);
        if( !g_hResDll )
            break;

        // Crée la classe de la fenêtre principale et la fenêtre elle-même
        if (!InitApplication(nCmdShow))
            break;

        while (GetMessage(&msg, (HWND) NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // libère les ressources localisées
        FreeLibrary(g_hResDll);

        // Supprime la class de la fenêtre principale
        UnregisterClass(L"MainWinClass", g_hInstance);

        // Previent CL_SHELLEXT_DLL du changement de langage
        if( !g_bEnd )
        {
            if( !OnLanguageChanged() ) {
                MessageBox(NULL, _T("Bad language DLL file!"), TITRE_MANAGER, MB_OK | MB_ICONERROR);
                g_bEnd = TRUE;
            }
        }

    }
    while( !g_bEnd ); // fin de la boucle de changement de langage

    // Libère les liens à la dll CL_SHELLEXT_DLL    g_pcf->Release();

    CoUninitialize();

    return 0;
}


// LVAppsCompare - Fonction de comparaison pour le tri du contrôle list view
//
// Retourne une valeur négative si le premier item doit précéder le second
//          une valeur positive si le premier item doir suivre le second
//          séro si les items sont équivalents
//
// lParam1 and lParam2 - les deux items à comparer
// lParam3 - paramètre de trie (colonne et sens)
//
int CALLBACK LVAppsCompare( LPARAM lParam1, LPARAM lParam2, LPARAM lParam3)
{
    LVNodeParam *pItem1 = (LVNodeParam *) lParam1,
                *pItem2 = (LVNodeParam *) lParam2;

    signed short sCol = HIWORD(lParam3),
                 sSens = LOWORD(lParam3);
    int iCmp, i;

    // Le premier item est celui qui permet d'ajouter d'autre
    // programmes : il doit toujours rester le premier même après un tri
    if( pItem1->bAddItem )
        return -1;

    if( pItem2->bAddItem )
        return 1;

    for(i=1, iCmp=0; i<4 && (iCmp == 0); i++)
    {
        switch( (sCol+i) % 5 )
        {
        case 1:
            iCmp = _tcsicmp(pItem1->settings.m_szProgDescr, pItem2->settings.m_szProgDescr);
            break;
        case 2:
            iCmp = (int)CompareFileTime(&pItem1->settings.m_ftDateTime, &pItem2->settings.m_ftDateTime);
            //iCmp = _tcsicmp(pItem1->pszDateTime, pItem2->pszDateTime);
            break;
        case 3:
            {
            long bias1 = g_tzimgr.m_arrRegTimeZoneInfo[pItem1->tziIndex]->m_regTZI.Bias,
                 bias2 = g_tzimgr.m_arrRegTimeZoneInfo[pItem2->tziIndex]->m_regTZI.Bias;

            iCmp = ( bias1 > bias2)
                || (( bias1 == bias2)&& (_tcscmp(g_tzimgr.m_arrRegTimeZoneInfo[pItem1->tziIndex]->m_szStd,
                                                 g_tzimgr.m_arrRegTimeZoneInfo[pItem2->tziIndex]->m_szStd) > 0)); 
            }
            break;
        case 4:
            iCmp = _tcsicmp(pItem1->pszFilename, pItem2->pszFilename);
            break;
        }
    }

    return iCmp * sSens;
}


// Initialise les colonnes pour le contrôle ListView
void MngrLVInitCol (HWND hwnd)
{
	LV_COLUMN	lvc;

	// Initialise la structure LV_COLUMN
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;

	// Ajoute les deux colonnes
	lvc.cx = 120;
	lvc.iSubItem = 0;
	lvc.pszText = LoadResString(g_hResDll, IDS_PROGRAMNAME);
	ListView_InsertColumn(hwnd, lvc.iSubItem, &lvc);
	FreeResString(lvc.pszText);

	lvc.cx = 100;
	lvc.iSubItem = 1;
	lvc.pszText = LoadResString(g_hResDll, IDS_COLUMNDATETIME);
	ListView_InsertColumn(hwnd, lvc.iSubItem, &lvc);
	FreeResString(lvc.pszText);

	lvc.cx = 200;
	lvc.iSubItem = 2;
	lvc.pszText = LoadResString(g_hResDll, IDS_COLUMNTIMEZONE);
	ListView_InsertColumn(hwnd, lvc.iSubItem, &lvc);
	FreeResString(lvc.pszText);

	lvc.cx = 350;
	lvc.iSubItem = 3;
	lvc.pszText = LoadResString(g_hResDll, IDS_PATH);
	ListView_InsertColumn(hwnd, lvc.iSubItem, &lvc);
	FreeResString(lvc.pszText);
}

// Ajoute un item au contrôle ListView
// retourne l'index de l'item cree
int CALLBACK MngrLVAdd (HWND hwnd, int iImage, int state, char stateImg, LPARAM lParam)
{
	LV_ITEM		lvi;

	// Prepare la structure pour la création d'un item
	lvi.mask = LVIF_TEXT;
	lvi.pszText = (PTSTR)LPSTR_TEXTCALLBACK;

	if( iImage != -1 )
	{
		lvi.mask |= LVIF_IMAGE;
		lvi.iImage = iImage;
	}

	if( lParam != -1 )
	{
		lvi.mask |= LVIF_PARAM;
		lvi.lParam = lParam;
	}

	if( state != -1 )
	{
		lvi.mask |= LVIF_STATE;
		lvi.stateMask = state;
		lvi.state = state;
	}
	else
		lvi.stateMask = 0;


	lvi.iItem = 1;
	lvi.iSubItem = 0;

	// Ajoute l'item au contrôle ListView
	lvi.iItem = ListView_InsertItem(hwnd, &lvi);
	ListView_SetItemText( hwnd, lvi.iItem, 1, LPSTR_TEXTCALLBACK );

	return lvi.iItem;
}

// Supprime tous les items de la liste
void DeleteListItems(HWND hwnd)
{
	// Prepare la structure pour la création d'un item
	LV_ITEM		lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iSubItem = 0;

	int iCnt = ListView_GetItemCount(hwnd);
	for(lvi.iItem=0; lvi.iItem<iCnt ;lvi.iItem++) {
		ListView_GetItem(hwnd, &lvi);

		// libère la mémoire allouée pour cet item
		if( lvi.lParam )
            delete(((LVNodeParam *)lvi.lParam));
	}
	ListView_DeleteAllItems(hwnd);
}

// Formate une structure date en une chaîne en respectant la configuration
// systeme pour l'affichage des dates
void FormatDateTime(PSYSTEMTIME pst, PTSTR *ppszOut, bool bDate, bool bTime)
{
	int cTime, cDate;
	PTSTR pszOut;

	if(bDate)
		cDate = GetDateFormat(
			LOCALE_USER_DEFAULT,	// locale for which date is to be formatted
			DATE_SHORTDATE,			// flags specifying function options
			pst,					// date to be formatted
			NULL,					// date format string
			NULL,					// buffer for storing formatted string
			0						// size of buffer
			);
	else
		cDate = 0;

	if( bTime ) 
		cTime = GetTimeFormat(
			LOCALE_USER_DEFAULT,	// locale for which date is to be formatted
			TIME_NOSECONDS,		// flags specifying function options
			pst,					// date to be formatted
			NULL,					// date format string
			NULL,					// buffer for storing formatted string
			0						// size of buffer
			);
	else
		cTime = 0;

	pszOut = *ppszOut = (TCHAR *)malloc(sizeof(TCHAR) * (cDate + cTime +2));
	pszOut[0] = _T('\0');

	if( bDate ) {
		GetDateFormat(
			LOCALE_USER_DEFAULT,	// locale for which date is to be formatted
			DATE_SHORTDATE,	// flags specifying function options
			pst,					// date to be formatted
			NULL,					// date format string
			pszOut,					// buffer for storing formatted string
			cDate					// size of buffer
			);

		pszOut += cDate-1;
		*pszOut++ = ' '; // insère un espace entre la date et l'heure
		*pszOut = '\0';
	}

	if(bTime)
		GetTimeFormat(
			LOCALE_USER_DEFAULT,	// locale for which date is to be formatted
			TIME_NOSECONDS,		// flags specifying function options
			pst,					// date to be formatted
			NULL,					// date format string
			pszOut,					// buffer for storing formatted string
			cTime					// size of buffer
			);
}

BOOL MyExtractIcon(LPCTSTR pszFile, HICON *phiconLarge, HICON *phiconSmall)
{
	SHFILEINFO sfi = {0};

	DWORD_PTR ret = SHGetFileInfo(pszFile, 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_LARGEICON);	
	*phiconLarge = ret!=0 ? sfi.hIcon : NULL;

	ret = SHGetFileInfo(pszFile, 0, &sfi, sizeof(sfi), SHGFI_ICON |  SHGFI_SMALLICON);
	*phiconSmall = ret!=0 ? sfi.hIcon : NULL;

	return (*phiconSmall) || (*phiconLarge);
}


// Create a listView node from an application setting section
LVNodeParam *AppSectionToLVParamNode(LPCTSTR subsecName, SettingSection &secApp)
{
    TCHAR filepath[_MAX_PATH];
    RegPathToFilePath(subsecName, _countof(filepath), filepath);

    // Alloue de la mémoire pour un nouvel item
    LVNodeParam *pfn = new LVNodeParam;
    if( pfn ) {
        pfn->bAddItem = false;
        // Chemin complet
        pfn->pszFilename = _tcsdup(filepath);    
        // Repertoire
        pfn->pszFileDir = _tcsdup(filepath);
        TruncateFilenameToFileDirectory(pfn->pszFileDir);

        // parametrage de l'application
        pfn->settings.LoadFromSection(filepath, &secApp);
        pfn->tziIndex = g_tzimgr.GetTimeZoneIndex(&pfn->settings.m_tzi);
        // formate la date et l'heure en une chaine
        FormatDateTime(&pfn->settings.m_stDateTime, &pfn->pszDateTime, pfn->settings.m_bDateMode, pfn->settings.m_bTimeMode);
    }
    return pfn;
}

// Add an application to the ListView
// retourne l'index de l'item cree
int AddAppToListView( LPCTSTR szSubsecName, SettingSection &secApp )
{
    // Cree l'item Listview correspondant
    LVNodeParam *pfn = AppSectionToLVParamNode( szSubsecName, secApp );
    if( pfn ) {
        // Extrait les icones des fichiers EXE
        HICON hIcoSml, hIcoBig;
        int	iIco;
        if( FileExists(pfn->pszFilename) ) {
            if( MyExtractIcon(pfn->pszFilename, &hIcoBig, &hIcoSml) ) {
                iIco = ImageList_AddIcon(g_hImgLstSml, hIcoSml);
                ImageList_AddIcon(g_hImgLstBig, hIcoBig);
                DestroyIcon(hIcoSml);
                DestroyIcon(hIcoBig);
            }
            else
                iIco  = g_iIcoDefault;
        }
        else
            iIco = g_iIcoAbsent;

        // Ajoute l'item a la listview
        return MngrLVAdd( g_hwndLV, iIco, -1, 0, (LPARAM)pfn);
    }
    return -1;
}

// Rafraîchie le contenu de la liste
void RefreshList(void)
{
    // enregistre l'etat de l'item courrament selectionne
    LVITEM selected_lvi;
    selected_lvi.iItem = ListView_GetNextItem(g_hwndLV, -1, LVNI_SELECTED);
    if( selected_lvi.iItem == -1 )
        selected_lvi.iItem = ListView_GetNextItem(g_hwndLV, -1, LVNI_FOCUSED);

    if( selected_lvi.iItem != -1 ) {
        selected_lvi.mask = LVIF_STATE;
        selected_lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
        ListView_GetItem(g_hwndLV, &selected_lvi);
    }


    // Efface les images de la liste
 	ImageList_RemoveAll(g_hImgLstSml);

	// Efface les images de la liste
	ImageList_RemoveAll(g_hImgLstBig);

	// Supprime tous les items de la liste
	DeleteListItems(g_hwndLV);

	// Alloue de la mémoire pour un nouvel item
    LVNodeParam *pfn = new LVNodeParam;

	PTSTR pResStr = LoadResString(g_hResDll, IDS_ADDPROGRAM);
        _tcscpy_s(pfn->settings.m_szProgDescr, pResStr);
	FreeResString( pResStr );
	pfn->tziIndex = 0;
	pfn->pszFilename = NULL;
	pfn->pszFileDir = NULL;
	pfn->pszDateTime = NULL;
	pfn->bAddItem = true;

	///////////
	// Ajoute l'item qui permet d'en ajouter d'autres
	HICON hIco;

	hIco = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_ADD), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0),
	g_iIcoAddProgram = ImageList_AddIcon(g_hImgLstSml, hIco);
	DestroyIcon(hIco);
	hIco = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_ABSENT), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0),
	g_iIcoAbsent = ImageList_AddIcon(g_hImgLstSml, hIco);
	DestroyIcon(hIco);
	hIco = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_DEFAULTAPP), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0),
	g_iIcoDefault = ImageList_AddIcon(g_hImgLstSml, hIco);
	DestroyIcon(hIco);

    hIco = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_ADD), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
	ImageList_AddIcon(g_hImgLstBig, hIco);
	DestroyIcon(hIco);
	hIco = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_ABSENT), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0),
	ImageList_AddIcon(g_hImgLstBig, hIco);
	DestroyIcon(hIco);
	hIco = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_DEFAULTAPP), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0),
	ImageList_AddIcon(g_hImgLstBig, hIco);
	DestroyIcon(hIco);

	MngrLVAdd(	g_hwndLV,
				g_iIcoAddProgram,
				LVIS_FOCUSED,	// par défaut a le focus
				0,				// pas d'images d'état
				(LPARAM)pfn);

	CLCommonSettingSection secCommon;

	if( !secCommon.Open( REGKEY_MBCL_APPS ) )
		return;

	TCHAR   szSubsecName[_MAX_PATH];
	for(secCommon.InitEnumSubsection(); 
		secCommon.EnumSubsection(szSubsecName, _countof(szSubsecName)); ) {

        // Charge la section correspondant a l'application
        if( SettingSection *secApp = secCommon.GetAppSubsectionForward(szSubsecName) ) {
            AddAppToListView( szSubsecName, *secApp );
            SettingSection::deleteObject(secApp);
        }
	}

    // si un item etait selectionne avant le refresh alors reselectionne le
    if( selected_lvi.iItem != -1 ) {
        // repositionne le focus sur l'item modifié
        ListView_SetItem(g_hwndLV, &selected_lvi);
    }
}

// met à jour l'état des menus
void RefreshMenuState( HMENU hMenu )
{
	LVITEM lvi;
	int iState;

	lvi.iItem = ListView_GetSelectedItem(g_hwndLV);
	if( lvi.iItem != -1 )
	{
		lvi.mask = LVIF_PARAM;
		lvi.iSubItem = 0;
		ListView_GetItem(g_hwndLV, &lvi);
		if( ((LVNodeParam *)lvi.lParam)->bAddItem )
			iState = MF_GRAYED;
		else
			iState = MF_ENABLED;
	}
	else
		iState = MF_GRAYED;

	EnableMenuItem(hMenu, ID_PROGRAM_CONFIG, MF_BYCOMMAND | iState );
	EnableMenuItem(hMenu, ID_PROGRAM_RUN, MF_BYCOMMAND | iState);
	EnableMenuItem(hMenu, ID_PROGRAM_DELETE, MF_BYCOMMAND | iState);
}

void OnApplyClick(HWND hwnd, HWND hConfig)
{
	NMHDR nmh;
	nmh.code = PSN_APPLY;
	nmh.hwndFrom = 0;
	nmh.idFrom = 0;
	SendMessage(hConfig, WM_NOTIFY, 0, (LPARAM)&nmh);
	if( GetWindowLong(hConfig, DWLP_MSGRESULT) == PSNRET_NOERROR )
		EnableWindow(GetDlgItem(hwnd, ID_BTAPPLY), FALSE);
	else
		EnableWindow(GetDlgItem(hwnd, ID_BTAPPLY), TRUE);
}

void SetWindowsPos(HWND hwnd, HWND hConfig)
{
	RECT rc, rcBT, rcParent;
	LONG cxFrame, cyFrame;

	cxFrame = GetSystemMetrics(SM_CXDLGFRAME);
	cyFrame = GetSystemMetrics(SM_CYDLGFRAME);

	GetWindowRect(GetParent(hwnd), &rcParent);
	GetClientRect(hConfig, &rc);
	GetClientRect(GetDlgItem(hwnd, IDCANCEL), &rcBT);

	rcBT.left = rc.right - 2*cxFrame - rcBT.right;
	rcBT.top = rc.bottom + cyFrame/2;
	SetWindowPos(GetDlgItem(hwnd, ID_BTAPPLY), NULL, rcBT.left, rcBT.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	rcBT.left -= rcBT.right + 2*cxFrame;
	SetWindowPos(GetDlgItem(hwnd, IDCANCEL), NULL, rcBT.left, rcBT.top, rcBT.right, rcBT.bottom, SWP_NOZORDER );

	rcBT.left -= rcBT.right + 2*cxFrame;
	SetWindowPos(GetDlgItem(hwnd, IDOK), NULL, rcBT.left, rcBT.top, rcBT.right, rcBT.bottom, SWP_NOZORDER );


	rc.right += 2*cxFrame;
	rc.bottom = GetSystemMetrics(SM_CYCAPTION) + rcBT.top + rcBT.bottom + 3*cyFrame;
	rc.left = rcParent.left + ((rcParent.right - rcParent.left) - rc.right) / 2;
	rc.top = rcParent.top + ((rcParent.bottom - rcParent.top) - rc.bottom) / 2;

	SetWindowPos(hwnd, NULL, rc.left, rc.top, rc.right, rc.bottom, SWP_NOZORDER);
}


INT_PTR CALLBACK ConfigDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	static IShellConfigPage *piscp = NULL;
	static HWND hConfig = NULL;

	switch (uMessage)
	{
	case WM_INITDIALOG:
	{
		// Initialise la page
		if( g_pcf->CreateInstance(NULL, IID_IShellConfigPage, (void **)&piscp) != NOERROR
			|| piscp->CreateConfigurationWindow(((CONFIGPARAM *)lParam)->pszFilename, hDlg, (HWND *)&hConfig) != NOERROR
			) {
			// Message : "Fichier invalide"
			PTSTR pszErr = LoadResString(g_hResDll, IDS_ERR_NOTEXEFILE);
			MessageBox(hDlg, pszErr, NULL, MB_OK | MB_ICONERROR);
			FreeResString(pszErr);

			EndDialog(hDlg, CONFIGRET_ERROR);
		}
		else {
			switch( ((CONFIGPARAM *)lParam)->op ) {
			case Activate:
				piscp->SetActivate(true);
				break;
			case Deactivate:
				piscp->SetActivate(false);
				break;
			}

			if( ((CONFIGPARAM *)lParam)->bOpAndClose ) {
				OnApplyClick(hDlg, hConfig);
				EndDialog(hDlg, CONFIGRET_SUCCESS);
			}
			else
				SetWindowsPos(hDlg, hConfig);
		}
		return TRUE;
	}

	case PSM_CHANGED:
		EnableWindow(GetDlgItem(hDlg, ID_BTAPPLY), TRUE);
		return TRUE;

	case WM_COMMAND:
	{
		WORD wNotifyCode = HIWORD(wParam); // notification code
		WORD wID = LOWORD(wParam);         // item, control, or accelerator identifier
		HWND hwndCtl = (HWND) lParam;      // handle of control
		if(wNotifyCode == BN_CLICKED )
		{
			switch(wID)
			{
			case ID_BTAPPLY:
				OnApplyClick(hDlg, hConfig);
				break;

			case IDOK:
				OnApplyClick(hDlg, hConfig);
				EndDialog(hDlg, CONFIGRET_SUCCESS);
				break;

			case IDCANCEL:
				EndDialog(hDlg, CONFIGRET_NOCHANGE);
				break;

			default :
				return FALSE;
			}

			SetWindowLong(hDlg, DWLP_MSGRESULT, 0);
			return TRUE;
		}
		break;
	}

	case WM_DESTROY:
		if(piscp)
			piscp->Release();

		SetWindowLong(hDlg, DWLP_MSGRESULT, 0);
		return TRUE;
	}

	return FALSE;
}



// Fonction appelée lors d'un changement de langage à partir du menu
void OnSetLanguage(UINT idMenu)
{
	MENUITEMINFO miti;

	// obtient les infos du menu correspondant au language choisi
	miti.cbSize = sizeof(MENUITEMINFO);
	miti.fMask = MIIM_DATA | MIIM_STATE;
	GetMenuItemInfo(g_hmnuLang, idMenu, FALSE, &miti);

	// Déjà sélectionné : sort de la fonction
	if( miti.fState & MFS_CHECKED )
		return;

	// Enregistre le nom de la DLL du langage sélectionné
	CLCommonSettingSection sec;
	if( sec.OpenCracklockSettingSection(true) ) {
		// enregistre le nom de la DLL pour la langue choisie par l'utilisateur
		sec.SetValue(REGVALUE_SETTINGS_LANGUAGE, (PCTSTR)miti.dwItemData);
	}

	// afin de réitèrer la boucle de changement de langage :
	g_bEnd = FALSE;

	// Détruie la fenêtre principale
	DestroyWindow(g_hMain);
}

// Obtient le nom du language contenu dans une ressource DLL de Cracklock
//
// PARAMÈTRES :
//
//	pszResPath	->		chemin du fichier des ressources
//
// RETOURNE : un pointeur sur une chaîne qui contient le
// nom de la langue (devraé est mibéré par 1 appel à FreeResString)
//
PTSTR GetResLanguageName( PCTSTR pszResDllPath )
{
	HINSTANCE hResInstance;
	TCHAR	*pszLang;

	if( pszResDllPath )
	{
		// charge le nom de la langue contenu dans cette dll
		hResInstance = LoadLibraryEx(pszResDllPath, NULL, LOAD_LIBRARY_AS_DATAFILE);

		pszLang = LoadResString(hResInstance, IDS_LANGUAGE);

		FreeLibrary(hResInstance);
	}
	else
		pszLang = LoadResString(g_hResDll, IDS_LANGUAGE);

	return pszLang;
}

// Ajoute un item pour une langue disponible
//
// hmnuLang :	handle du menu contenant la liste des langues
// pszResPath : chemin du fichier ressources correspondant à la langue désirée
void AddLanguageSubItem ( HMENU hmnuLang, PCTSTR pszResDllPath )
{	
    PTSTR pszLang = GetResLanguageName(pszResDllPath);
    if( !pszLang )
        return;

	// charge le nom de la dll courrament utilisée
	TCHAR	szCurResDllPath[_MAX_PATH];
	GetModuleFileName(g_hResDll, szCurResDllPath, _MAX_PATH);

	// ajoute le menu
	MENUITEMINFO miti;
    miti.cch = _tcslen(pszLang) + 1; // add 1 for terminal zero
    miti.cbSize = sizeof(MENUITEMINFO);
    miti.fMask = MIIM_ID | MIIM_DATA | MIIM_TYPE | MIIM_STATE ;
    miti.wID = ID_HELP_LANG_FIRST + GetMenuItemCount(hmnuLang);
    miti.dwItemData = (DWORD)_tcsdup(GetFileBaseNamePart(pszResDllPath));	// nom de la dll
    miti.dwTypeData = pszLang;												// language
    miti.fType = MFT_STRING | MFT_RADIOCHECK;
	miti.fState = MFS_ENABLED | (_tcsicmp(szCurResDllPath, pszResDllPath) ? 0 : MFS_CHECKED);
	InsertMenuItem(hmnuLang, GetMenuItemCount(hmnuLang), TRUE, &miti);
	FreeResString(pszLang);
}

// coche le menu correspondant à la vue demandée
void CheckViewMenu(UINT id)
{
	// coche le menu correspondant à la vue sélectionnée
	CheckMenuRadioItem(GetSubMenu(GetMenu(g_hMain), 1), ID_VIEW_LARGEICONS, ID_VIEW_DETAILSICONS, id, MF_BYCOMMAND);
}

BOOL OnInit(HWND hwnd)
{
    // init les variables globales
    g_hMain  = hwnd;

    ///////////
    // Obtient le chemin du répertoire où est installé Cracklock
    TCHAR szCLPath[_MAX_PATH];
    if( !GetCLPath(szCLPath, _countof(szCLPath)) )
        return FALSE;

    //////////
    // Ajoute un item pour chaque langue disponible

        // crée et ajoute le menu popup "Langage>"
        g_hmnuLang = CreatePopupMenu();

        PTSTR pMenuTitle = LoadResString(g_hResDll, IDS_LANGMENU);
        InsertMenu(GetSubMenu(GetMenu(g_hMain), 2), 0, MF_BYPOSITION | MF_POPUP, (UINT)g_hmnuLang, pMenuTitle);
        FreeResString(pMenuTitle);

        // chemin complet du rep contenant les dll res
        TCHAR szResPath[_MAX_PATH];
        StringCchPrintf (szResPath, _countof(szResPath), _T("%s\\%s\\*.*"), szCLPath, DIR_CL_LANG);

        WIN32_FIND_DATA wfd;
        HANDLE hsrh = FindFirstFile(szResPath, &wfd);
        if( hsrh != INVALID_HANDLE_VALUE )
        {
            do
            {
                if( _tcscmp(wfd.cFileName, _T(".")) &&
                    _tcscmp(wfd.cFileName, _T("..")) )
                {
                    // obtient le chemin complet de la res dll
                    StringCchPrintf(szResPath, _MAX_PATH, _T("%s\\%s\\%s"), szCLPath, DIR_CL_LANG, wfd.cFileName);
                    // ajoute le sous-menu
          AddLanguageSubItem(g_hmnuLang, szResPath);

                }
            }
            while( FindNextFile(hsrh, &wfd) );

            FindClose(hsrh);
        }
        else
        {
            MessageBox(NULL, _T("Can't find any resource file in the \"Cracklock\\Languages\\\" directory ! Please, try to reinstall Cracklock !"), TITRE_MANAGER, MB_OK | MB_ICONERROR);
            return FALSE;
        }

        // Nombre de langues ajoutés au menu
        g_iNbLang = GetMenuItemCount(g_hmnuLang);

    //
    /////////

    //////////
    // Création du contrôle ListView

        // Create the list view window.
        g_hwndLV = CreateWindowEx(WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR 
            | WS_EX_CLIENTEDGE , WC_LISTVIEW, _T(""),
                WS_CHILDWINDOW | WS_VISIBLE 
                |WS_CLIPCHILDREN
                |WS_HSCROLL
                |WS_TABSTOP
                |LVS_REPORT
                |LVS_SHAREIMAGELISTS
                |LVS_EDITLABELS
                |LVS_ALIGNLEFT
                | LVS_AUTOARRANGE | LVS_SINGLESEL | LVS_ALIGNTOP,
                0, 0, CW_USEDEFAULT, CW_USEDEFAULT,
                g_hMain, NULL, g_hInstance, NULL);

        MngrLVInitCol(g_hwndLV);

        // Crée les listes d'images pour le contrôle ListView
        g_hImgLstSml = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32|ILC_MASK, 10, 100);
        g_hImgLstBig = ImageList_Create(GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), ILC_COLOR32|ILC_MASK, 10, 100);

        // Associe les listes d'images au contrôle ListView
        ListView_SetImageList(g_hwndLV, g_hImgLstSml, LVSIL_SMALL);
        ListView_SetImageList(g_hwndLV, g_hImgLstBig, LVSIL_NORMAL);

    //
    //////////

    //////////
    // Applique les paramètres enregistrés dans la registry :
    
	// style de vue à appliquer au contrôle listview
	DWORD dwData = LVS_REPORT; // style by default
	GetCracklockSettingValue(REGVALUE_SETTINGS_LISTVIEW, &dwData);
	LV_SetView(g_hwndLV, dwData);
    switch(dwData) {
    case LVS_ICON:
        CheckViewMenu(ID_VIEW_LARGEICONS);
        break;
    case LVS_REPORT:
        CheckViewMenu(ID_VIEW_DETAILSICONS);
        break;
    case LVS_SMALLICON:
        CheckViewMenu(ID_VIEW_SMALLICONS);
        break;
    case LVS_LIST:
        CheckViewMenu(ID_VIEW_LISTICONS);
        break;
    }
    //
    /////////

    // Supprime les entrées incorrectes
    //CleanupSettingsEntries();

    // Rafraîchie le contenu de la liste
    RefreshList();

    return TRUE;
}

void OnDestroy()
{
	//////////
	// Libère la mémoire allouée par les membres DATAS des menus
	// des différents langages

	// pour chaque menu de langage
	MENUITEMINFO miti;
	miti.cbSize = sizeof(MENUITEMINFO);
	miti.fMask = MIIM_DATA;
	for( UINT i=0; i<g_iNbLang; i++)
	{
		GetMenuItemInfo(g_hmnuLang, i, TRUE, &miti);
		free((PVOID)miti.dwItemData);
	}

	//////////
	// Enregistre dans la registry le style de vue appliqué au
	// contrôle listview
	

	// Ouvre la clé de Cracklock
	CLCommonSettingSection secCL;
	
	secCL.OpenCracklockSettingSection(TRUE);

	secCL.SetValue(REGVALUE_SETTINGS_LISTVIEW, (DWORD)LV_GetView(g_hwndLV));

	RECT rcWin;
	GetWindowRect(g_hMain, &rcWin);
	secCL.SetValue(REGVALUE_SETTINGS_MAINRECT, &rcWin);


	// Supprime tous les items de la liste
	DeleteListItems(g_hMain);
}

// Cree une chaine de caracteres contenant la ligne de commande complete pour executer
// un programme avec le Loader de Cracklock.
//
// Retourne un pointeur sur une chaine contenant la ligne de commande. Il doit etre liberer 
// avec free(..) une fois que la chaine n'est plus utilisee.
//
LPTSTR CreateLoaderCommandLine(LPCTSTR pszAppExe, LPCTSTR pszAppParameters)
{
    size_t cnt = _countof(CL_LOADER_EXE) +
        _tcslen( pszAppExe ) +
        _tcslen( pszAppParameters ) +
        5; // les deux guillemets, les deux espaces et le 0 terminal
    PTSTR pszFile = (PTSTR)malloc( sizeof(TCHAR)* cnt );
                
    StringCchPrintf(pszFile, cnt, _T("%s \"%s\" %s"), CL_LOADER_EXE, pszAppExe, pszAppParameters);
    return pszFile;
}



void OnRun()
{
    LVITEM lvi;
    lvi.iItem = ListView_GetSelectedItem(g_hwndLV);
    if( lvi.iItem != -1 ) {
        lvi.mask = LVIF_PARAM;
        lvi.stateMask = 0;
        lvi.iSubItem = 0;
        ListView_GetItem(g_hwndLV, &lvi);

        // Ligne de commande complète avec le nom du fichier EXE entre guillemets
        LPTSTR pszFile = CreateLoaderCommandLine(((LVNodeParam *)lvi.lParam)->pszFilename,
            ((LVNodeParam *)lvi.lParam)->settings.m_szCmdParameters );

            // Exécute Cracklock Loader
            STARTUPINFO startupInfo;
            PROCESS_INFORMATION ProcessInformation;
            memset(&startupInfo, 0, sizeof(startupInfo));
            startupInfo.cb = sizeof(startupInfo);
            CreateProcess(	0,					// lpszImageName
                            pszFile,			// lpszCommandLine
                            0,					// lpsaProcess
                            0,					// lpsaThread
                            FALSE,				// fInheritHandles
                            0,					// fdwCreate
                            0,					// lpvEnvironment
                            0,					// lpszCurDir
                            &startupInfo,		// lpsiStartupInfo
                            &ProcessInformation	// lppiProcInfo
                            );
        free(pszFile);
    }
}

void OnDelete ()
{
    LVITEM lvi;
    lvi.iItem = ListView_GetSelectedItem(g_hwndLV);
    if( lvi.iItem != -1 ) {
        lvi.mask = LVIF_PARAM;
        lvi.stateMask = 0;
        lvi.iSubItem = 0;
        ListView_GetItem(g_hwndLV, &lvi);

        LVNodeParam *np = (LVNodeParam *)lvi.lParam;
        if( FileExists(np->pszFilename) ) {
            // Deassocie l'application de cracklock en simulant l'operation correspondante
            // dans la boite de dialog de configuration
            CONFIGPARAM	cp;
            cp.pszFilename = np->pszFilename;
            cp.bOpAndClose = true;
            cp.op = Deactivate;
            if( DialogBoxParam(g_hResDll, MAKEINTRESOURCE(IDD_CONFIG), g_hMain, ConfigDlgProc, (LPARAM)&cp) == CONFIGRET_SUCCESS ) {
                ListView_DeleteItem(g_hwndLV, lvi.iItem);
                delete np;
            }
        }
        else {
            // le fichier n'existe pas: supprime la section correspondante dans la base de setting generale
	        CLCommonSettingSection comsec;
            if( comsec.Open( REGKEY_MBCL_APPS ) ) {
                TCHAR szSubsecName[_MAX_PATH];
                if( comsec.SubsectionNameFromApp(np->pszFilename, szSubsecName, _countof(szSubsecName)) )
                    comsec. DeleteSubsection(szSubsecName, true);
            }
            // delete the local config file (if it exists)
            TCHAR inifile[_MAX_PATH];
            InifilenameFromApp(np->pszFilename, inifile, _countof(inifile));
            DeleteFile(inifile);
            ListView_DeleteItem(g_hwndLV, lvi.iItem);
            delete np;
        }
    }
}

// Rafraichit un item de la Listview a partir de la configuration de l'application stockee dans la registry/INI
bool RefreshLVItem(int iItem, PCTSTR pszFilename)
{
    // Met a jour l'item listview de l'application dont les parametres viennent d'etre modifiee
    CLCommonSettingSection secCommon;
    if( secCommon.Open(REGKEY_MBCL_APPS) ) {
        TCHAR szSubsecName[_MAX_PATH];
        if( secCommon.SubsectionNameFromApp(pszFilename, szSubsecName, _countof(szSubsecName)) ) {
            // Charge la section correspondant a l'application
            if( SettingSection *secApp = secCommon.GetAppSubsectionForward(szSubsecName) ) {
                // met a jours les infos de l'application
                LVITEM lvi;
                lvi.iItem = iItem;
                lvi.mask = LVIF_PARAM | LVIF_STATE;
                lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
                lvi.state = LVIS_SELECTED | LVIS_FOCUSED;
                lvi.iSubItem = 0;
                lvi.lParam = (LPARAM)AppSectionToLVParamNode( szSubsecName, *secApp );
                ListView_SetItem(g_hwndLV, &lvi);
                ListView_RedrawItems(g_hwndLV, lvi.iItem, lvi.iItem);
                SettingSection::deleteObject(secApp);
                return true;
            }
        }
    }
    return false;
}

void OnConfig()
{
    LVITEM lvi;
    lvi.iItem = ListView_GetSelectedItem(g_hwndLV);
    if( lvi.iItem != -1 ) {
        lvi.mask = LVIF_PARAM;
        lvi.stateMask = 0;
        lvi.iSubItem = 0;
        ListView_GetItem(g_hwndLV, &lvi);

        // Prépare les paramètres pour la dlgbox de configuration
        LVNodeParam *nodeparam = (LVNodeParam *)lvi.lParam;
        CONFIGPARAM	cp;
        cp.pszFilename = nodeparam->pszFilename;
        cp.bOpAndClose = false;
        cp.op = None;

        if( DialogBoxParam(g_hResDll, MAKEINTRESOURCE(IDD_CONFIG), g_hMain, ConfigDlgProc, (LPARAM)&cp) == CONFIGRET_SUCCESS )
            if( RefreshLVItem(lvi.iItem, nodeparam->pszFilename) )
                delete nodeparam; // si l'item a ete remplace alors libere l'ancien nodeparam qui etait associe a cet item
    }
}

// Ajoute un programme à la liste et affiche la boîte de configuration
void AddProgram( PTSTR pszFilename )
{
    
    int iPreviousItem = -1;
    // test si le fichier a ajouter est deja present dans la liste
    // si oui alors iPreviousItem contient l'index de l'item correspondant
    // sinon iPreviousItem =-1
    LVITEM lvi;
    lvi.iSubItem = 0;
    lvi.mask = LVIF_PARAM;
    LVNodeParam *lvnode;
    for(lvi.iItem=0;lvi.iItem<ListView_GetItemCount(g_hwndLV); lvi.iItem++) {        
        ListView_GetItem(g_hwndLV, &lvi);
        lvnode = (LVNodeParam *)lvi.lParam;
        if( lvnode && lvnode->pszFilename && _tcsicmp(pszFilename, lvnode->pszFilename) == 0 )
        {
            iPreviousItem = lvi.iItem;
            break;
        }
    }



	// Prépare les paramètres pour la dlgbox de configuration
	CONFIGPARAM	cp;
	cp.pszFilename = pszFilename;
	cp.bOpAndClose = false;
	cp.op = Activate;
	if( DialogBoxParam(g_hResDll, MAKEINTRESOURCE(IDD_CONFIG), g_hMain, ConfigDlgProc, (LPARAM)&cp) == CONFIGRET_SUCCESS )
	{
        if( iPreviousItem != -1 )
            RefreshLVItem(lvi.iItem, pszFilename); // met a jour l'item existant
        else {
		    // ajoute a la listview l'application qui vient d'etre associee a cracklock
            CLCommonSettingSection secCommon;
            if( secCommon.Open(REGKEY_MBCL_APPS) ) {
	            TCHAR szSubsecName[_MAX_PATH];
                if( secCommon.SubsectionNameFromApp(pszFilename, szSubsecName, _countof(szSubsecName)) ) {
                    // Charge la section correspondant a l'application
                    if( SettingSection *secApp = secCommon.GetAppSubsectionForward(szSubsecName) ) {
                        LVITEM lvi;
                        lvi.iItem = AddAppToListView( szSubsecName, *secApp );
                        if( lvi.iItem != -1 ) { 
                            // selectionne le nouvel item cree
                            lvi.iSubItem = 0;
                            lvi.mask = LVIF_STATE;
                            lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
                            lvi.state = LVIS_SELECTED | LVIS_FOCUSED;
                            ListView_SetItem(g_hwndLV, &lvi);
                        }
                        SettingSection::deleteObject(secApp);
                    }
                }
            }
        }
	}
}

// Affiche une boite de dialogue de selection de fichier afin que l'utilisateur
// specifie un programme a ajouter
void ChooseAppFileDlgbox(HWND hwnd)
{
	OPENFILENAME ofn;
	TCHAR *pszFilter, *pReplace;
	TCHAR szFileName[_MAX_PATH];

	// Crée le filtre (une liste de pairs separees par un caractere nul et terminee par 2 caracteres nuls)
	pszFilter = LoadResString(g_hResDll, IDS_PROGRAMFILTER);
	pReplace = pszFilter;
	while( pReplace = _tcschr(pReplace, L'|') )
		*pReplace++ = '\0';

	szFileName[0] = '\0';
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = pszFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrFileTitle = 0;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = 0;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST |  OFN_HIDEREADONLY;
	if( GetOpenFileName(&ofn) )
		AddProgram(szFileName);

	FreeResString(pszFilter);
}


// Resolution du nom de fichier pointe par un raccourci.
// entre:
//  pszShortcutFile    nom de fichier du raccourci
//  size			   taille du buffer szResolveFileName (en TCHAR)
// sortie:
//  szResolveFileName  nom de fichier apres resolution du lien
//
// Si pszShortcutFile n'est pas un raccourci alors cette fonction
// retourne dans szResolveFileName la chaine contenu dans pszShortcutFile.
void ResolveShortcut(PTSTR szResolveFileName, SIZE_T size, PCTSTR pszShortcutFile)
{
    IShellLink *			IShellLinkPtr;
    WCHAR					DoubleByteBuffer[MAX_PATH];
    CLSID					id;

    try {
        // obtient un pointeur vers l'interface IShellLink
        if(CoCreateInstance(CLSID_ShellLink, 0, CLSCTX_INPROC_SERVER,
                IID_IShellLink, (void **)&IShellLinkPtr))
            throw ;

        // Convertit en UNICODE
    #ifndef UNICODE
        // Ensure that the name is ANSI
        MultiByteToWideChar(CP_ACP, 0, pszShortcutFile, -1, &DoubleByteBuffer[0], MAX_PATH);
    #else
        _tcscpy_s(DoubleByteBuffer, pszShortcutFile);
    #endif

        // Est-ce un raccourci ?
        if(!GetClassFile(&DoubleByteBuffer[0], &id) && id == CLSID_ShellLink)
        {
            IPersistFile *  ppf;

            // obtient un pointeur vers l'interface IPersistFile de l'interface IShellLink
            // afin de pouvoir charger le raccourci
            if (!IShellLinkPtr->QueryInterface(IID_IPersistFile, (void **)&ppf))
            {
                // charge le raccourci dans le IShellLink
                if (!ppf->Load(&DoubleByteBuffer[0], STGM_READ))
                {
                    // obtient le nom de fichier vers lequel pointe ce raccourci
                    if (!IShellLinkPtr->GetPath( szResolveFileName, MAX_PATH, NULL, SLGP_UNCPRIORITY))
                    {
                    }
                }
                // libere l'interface IPersistFile
                ppf->Release();
            }
        }
        else
        {
            _tcscpy_s(szResolveFileName, size, pszShortcutFile);
        }

        // Free the IShellLink interface
        IShellLinkPtr->Release();

    }
    catch(...)
    {
        _tcscpy_s(szResolveFileName, size, pszShortcutFile);
    }
}


INT_PTR CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        return OnInit(hwnd) ? 0 : -1;

    case WM_SETFOCUS:
        SetFocus(g_hwndLV);
        break;

    case WM_SETTINGCHANGE:
        // Configuration de l'affichage international
        RefreshList();
        break;

    case WM_COMMAND:
        switch( wParam ) {
        case ID_PROGRAM_RUN:
            OnRun();
            break;

        case ID_PROGRAM_DELETE:
            OnDelete();
            break;

        case ID_PROGRAM_CONFIG:
            OnConfig();
            break;

        case ID_OPTION_SETTING:
            ShowSettingsDlg(hwnd);
            RefreshList();
            break;

        case ID_PROGRAM_CLOSE:
            DestroyWindow(hwnd);
            break;

        case ID_VIEW_LARGEICONS:
            LV_SetView(g_hwndLV, LVS_ICON);
            break;
        case ID_VIEW_SMALLICONS:
            LV_SetView(g_hwndLV, LVS_SMALLICON);
            break;
        case ID_VIEW_LISTICONS:
            LV_SetView(g_hwndLV, LVS_LIST);
            break;
        case ID_VIEW_DETAILSICONS:
            LV_SetView(g_hwndLV, LVS_REPORT );
            break;

        case ID_VIEW_REFRESH:
            RefreshList();
            break;

        case ID_HELP_DOCUMENTATION:
            InvokeHelp(hwnd, -1);
            break;

        case ID_HELP_WEBSITE:
            {
                PTSTR link = LoadResString(g_hResDll, IDS_MBLINK);
                ShellExecute(hwnd, NULL, link, NULL, NULL, SW_SHOWDEFAULT);
                FreeResString(link);
            }
            break;


        case ID_HELP_ABOUT:
            DialogBox(g_hResDll, MAKEINTRESOURCE(IDD_ABOUT), hwnd, AboutDlgProc);
            break;

        }

        // Test si c'est un menu de langage
        if( ((UINT)wParam >= ID_HELP_LANG_FIRST) && ((UINT)wParam < ID_HELP_LANG_FIRST + g_iNbLang) )
        {
            OnSetLanguage((UINT)wParam);
        }
        // Test si c'est un menu de vue
        else if((wParam >= ID_VIEW_LARGEICONS) && (wParam <= ID_VIEW_DETAILSICONS))
        {
            CheckViewMenu( (UINT)wParam );
        }

        return DefWindowProc (hwnd, uMsg, wParam, lParam) ;

    case WM_SIZE:
        SetWindowPos(g_hwndLV,0, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER | SWP_NOMOVE);
        break;

    case WM_DROPFILES:
    {
        HDROP hDrop = (HDROP)wParam;
        TCHAR szFileName[_MAX_PATH];
        TCHAR szResolveFileName[_MAX_PATH];

        DragQueryFile (hDrop, 0, szFileName, sizeof (szFileName));
            ResolveShortcut(szResolveFileName, _countof(szResolveFileName), szFileName);
            AddProgram(szResolveFileName);
        DragFinish (hDrop);

        break;
    }

    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code)
        {
            // Process LVN_GETDISPINFO to supply information about
            // callback items.
            case LVN_GETDISPINFO:
            {
                // Provide the item or subitem's text, if requested.
                NMLVDISPINFO *pnmv =  (NMLVDISPINFO *)lParam;
                if (pnmv->item.mask & LVIF_TEXT)
                {
                    LVNodeParam *pItem = (LVNodeParam *)pnmv->item.lParam;
                    if( pnmv->item.iItem == 0 ) {
                        if( pnmv->item.iSubItem  == 0)
                            _tcscpy_s(pnmv->item.pszText, pnmv->item.cchTextMax, pItem->settings.m_szProgDescr);
                    }
                    else {
                        switch( pnmv->item.iSubItem ) {
                        case 0:
                            _tcscpy_s(pnmv->item.pszText, pnmv->item.cchTextMax, pItem->settings.m_szProgDescr);
                            break;
                        case 1:
                            _tcscpy_s(pnmv->item.pszText, pnmv->item.cchTextMax, pItem->pszDateTime);
                            break;
                        case 2:
                            if(pItem->settings.m_bTimeZoneMode) {
                                _tcscpy_s(pnmv->item.pszText, pnmv->item.cchTextMax, g_tzimgr.m_arrRegTimeZoneInfo[pItem->tziIndex]->m_szDisplay);
                            }
                            else
                            {
                                PTSTR psz = LoadResString(g_hResDll, IDS_SYSTEMTIMEZONE);
                                _tcscpy_s(pnmv->item.pszText, pnmv->item.cchTextMax, psz);
                                FreeResString (psz);
                            }
                            break;
                        case 3:
                            _tcscpy_s(pnmv->item.pszText, pnmv->item.cchTextMax, pItem->pszFilename);
                            break;
                        }
                        break;
                    }
                }
            }
            return DefWindowProc (hwnd, uMsg, wParam, lParam);

            case LVN_ITEMCHANGED :
                RefreshMenuState( GetMenu(hwnd) );
                break;
            case LVN_BEGINLABELEDIT:
                return ((LVNodeParam *)((NMLVDISPINFO *)lParam)->item.lParam)->bAddItem;
            case LVN_ENDLABELEDIT:
                {
                    NMLVDISPINFO *pnmv =  (NMLVDISPINFO *)lParam;
                    LVNodeParam *pItem = (LVNodeParam *)pnmv->item.lParam;
                    if( !pItem->bAddItem && pnmv->item.pszText ) {
                        SettingSection *sec = CLCommonSettingSection::GetSectionOfApp( pItem->pszFilename, false, false);
                		if( sec ) {
                            sec->SetValue(REGVALUE_PROG_DESCRIPTION, pnmv->item.pszText);
                            _tcscpy_s(pItem->settings.m_szProgDescr, pnmv->item.pszText);
                        }
			            CLCommonSettingSection::deleteObject(sec);
                        return true;

                    }

                }                        
                break;
            case LVN_KEYDOWN:
                {
                    LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN) lParam;   
                    LVITEM lvi;

                    lvi.mask = LVIF_PARAM;
                    lvi.stateMask = 0;
                    lvi.iItem = ListView_GetSelectedItem(g_hwndLV);
                    lvi.iSubItem = 0;
                    if( ListView_GetItem(g_hwndLV, &lvi) ) { // item selected?
                        switch(((LPNMLVKEYDOWN)lParam)->wVKey) {
                        case VK_DELETE:
                            OnDelete();
                            break;
                        case VK_RETURN:
                            if( ((LVNodeParam *)lvi.lParam)->bAddItem )
                                ChooseAppFileDlgbox(hwnd);
                            else
                                OnConfig();
                            break;
                         case VK_F5:
                            RefreshList();
                            break;
                         case VK_F2:
                            ListView_EditLabel(g_hwndLV, ListView_GetNextItem(g_hwndLV, -1, LVNI_SELECTED));
                            break;
                        case VK_ESCAPE:
                            ListView_CancelEditLabel(g_hwndLV);
                            break;
                        default:
                            return DefWindowProc (hwnd, uMsg, wParam, lParam) ;
                        }
                    }
                    else {
                        switch(((LPNMLVKEYDOWN)lParam)->wVKey)
                        {
                         case VK_F5:
                            RefreshList();
                            break;
                        default:
                            return DefWindowProc (hwnd, uMsg, wParam, lParam) ;
                        }
                    }
                }
                break;

            case NM_RCLICK:
            {
                POINT pt;
                HMENU hPopup, hPopup2;
                GetCursorPos(&pt);

                hPopup = LoadMenu(g_hResDll, MAKEINTRESOURCE(IDM_POPUP));
                hPopup2 = GetSubMenu(hPopup, 0);
                SetMenuDefaultItem(hPopup2, ID_PROGRAM_CONFIG, FALSE);
                RefreshMenuState( hPopup2 );
                TrackPopupMenu(hPopup2, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON |  TPM_RIGHTBUTTON , pt.x, pt.y, 0, hwnd, NULL);
                DestroyMenu( hPopup2 );
                DestroyMenu( hPopup );
            }
            break;

            case NM_DBLCLK:
            {
                LVITEM lvi;

                lvi.mask = LVIF_PARAM;
                lvi.stateMask = 0;
                lvi.iItem = ListView_GetSelectedItem(g_hwndLV);
                lvi.iSubItem = 0;
                if( !ListView_GetItem(g_hwndLV, &lvi) )
                    break;

                if( ((LVNodeParam *)lvi.lParam)->bAddItem )
                    ChooseAppFileDlgbox(hwnd);
                else
                    OnConfig();
                break;

            }
            case LVN_COLUMNCLICK:
                LV_ColumnClick( lParam, LVAppsCompare );
                break;

            default:
                return DefWindowProc (hwnd, uMsg, wParam, lParam) ;
        }
        break;

    case WM_DESTROY:
        OnDestroy();

        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc (hwnd, uMsg, wParam, lParam) ;

    }
    return FALSE;
}