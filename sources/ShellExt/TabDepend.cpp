// TabDepend.cpp
//#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' " \
//                       "version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")



#include "StdAfx.h"			// Header précompilé
#include "..\Resources\Resources.h"	// Resources localisés
#include "resource.h"		// Resources
#include <commctrl.h>		// Contrôles communs
#include "ShellExt.h"		// Définition de la classe CShellExt

// Constantes: Numéro des images dans les listes d'images
//		- images de fichiers
#define	LV_App			0
#define	LV_Dll			1
#define	LV_WinSysDll	2
//		- images d'états
#define LV_State_None			0
#define	LV_State_Checked		1
#define	LV_State_GrayChecked	2
#define	LV_State_Unchecked		3


// Initialise les colonnes pour le contrôle ListView
void LVInitCol (HWND hwnd, int cx1, int cx2)
{ 
    LV_COLUMN	lvc;

    // Initialise la structure LV_COLUMN
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
    lvc.fmt = LVCFMT_LEFT; 

    // Ajoute les deux colonnes
    lvc.cx = cx1;
    lvc.iSubItem = 0;
    lvc.pszText = LoadResString(g_hResDll, IDS_COLUMNNAME);
    ListView_InsertColumn(hwnd, 0, &lvc);
    free(lvc.pszText);

    lvc.cx = cx2;
    lvc.iSubItem = 1;
    lvc.pszText = LoadResString(g_hResDll, IDS_COLUMNDIR);
    ListView_InsertColumn(hwnd, 1, &lvc);
    free(lvc.pszText);
} 

// LV_Compare - Fonction de comparaison pour le tri du contrôle list view
//
// Retourne une valeur négative si le premier item doit précéder le second
//          une valeur positive si le premier item doir suivre le second
//          séro si les items sont équivalents
//
// lParam1 and lParam2 - les deux items à comparer
// lParam3 - paramètre de trie (colonne et sens)
//
int CALLBACK LV_Compare( LPARAM lParam1, LPARAM lParam2, LPARAM lParam3)
{        
    CNode *pItem1 = (CNode *)lParam1,
          *pItem2 = (CNode *)lParam2;
    signed short sCol = HIWORD(lParam3),
	             sSens = LOWORD(lParam3);
    int iCmp, i;

    if( pItem1 == NULL )
        return -1;

    else if( pItem2 == NULL )
        return 1;

    for(i=1, iCmp=0; i<3 && (iCmp == 0); i++)
    {
        switch( (sCol+i) % 3 )
        {
        case 1:
            iCmp = _tcsicmp(pItem1->m_pszBaseName, pItem2->m_pszBaseName);
            break;
        case 2:
            iCmp = _tcsicmp(pItem1->m_pszFilePath, pItem2->m_pszFilePath);
            break;
        }
    }

    return iCmp * sSens;
}


CDepend::CDepend()
{
    m_hImgLst = m_hImgLstSt = NULL;
    m_wIDTab = IDD_TABDEPEND;
}	

// Redimensionne les controles
void CDepend::SetControlsSize()
{
    HWND hwndTitre = GetItem(IDC_INFO);	
    HDC hdcTitre = GetWindowDC(hwndTitre);
	
	
    RECT rcTitre, rcClient;
    GetWindowRect(hwndTitre, &rcTitre);
    GetWindowRect(m_hwnd, &rcClient);
    int margex = rcTitre.left-rcClient.left,
        margey = 5,
        width = rcClient.right-rcClient.left - 2*margex;

    // Calcul la taille du controle  necessaire pour afficher le titre
    int cLen = GetWindowTextLength(hwndTitre);
    PTSTR pszTitre = (PTSTR) malloc(sizeof(TCHAR) * (cLen+1));
    GetWindowText(hwndTitre, pszTitre, (cLen+1));
    //HFONT oldf = SelectFont(hdcTitre, m_pcs->m_hfUnicode);
        //DrawText(hdcTitre, pszTitre, cLen, &rcTitre, DT_WORDBREAK | DT_CALCRECT);
    //SelectFont(hdcTitre, oldf);
    free(pszTitre);

    // Redimensionne le controle pour le titre
    SetWindowPos(hwndTitre, NULL, 0, 0,
        width,
        rcTitre.bottom-rcTitre.top, SWP_NOMOVE | SWP_SHOWWINDOW );
	
    // Redimensionne la liste
    SetWindowPos(m_hwndLV, NULL, 
        margex,
        rcTitre.bottom-rcClient.top + margey, 
        width,
        rcClient.bottom-rcTitre.bottom - 2*margey, SWP_SHOWWINDOW );
}

// Initialise les images du contrôle ListView
BOOL CDepend::OnInit( HWND hDlg )
{
    // Creation du contrôle ListView
    /*CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, _T(""), 
        LVS_SINGLESEL | LVS_AUTOARRANGE | LVS_ALIGNTOP |
        WS_CHILDWINDOW | WS_VISIBLE | LVS_REPORT |  
        WS_TABSTOP,
        CW_USEDEFAULT,CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT, 
        hDlg, NULL, GetModuleHandle(NULL), NULL); */
    m_hwndLV  = GetItem(IDC_LSTDEPEND);
    SetWindowLong(m_hwndLV, GWL_EXSTYLE, WS_EX_CLIENTEDGE);
    SetWindowLong(m_hwndLV, GWL_STYLE, LVS_SINGLESEL | LVS_AUTOARRANGE | LVS_ALIGNTOP |
        WS_CHILDWINDOW | WS_VISIBLE | LVS_REPORT |  WS_TABSTOP);
	

    // Crée la liste d'images pour le contrôle ListView
    m_hImgLst = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), TRUE, 3, 0);  

    // Ajoute les icones à la liste des images
    ImageList_AddIcon(m_hImgLst, LoadIcon(g_hmodThisDll, MAKEINTRESOURCE(IDI_LSTLOADER)));
    ImageList_AddIcon(m_hImgLst, LoadIcon(g_hmodThisDll, MAKEINTRESOURCE(IDI_LSTFILE)));
    ImageList_AddIcon(m_hImgLst, LoadIcon(g_hmodThisDll, MAKEINTRESOURCE(IDI_LSTWINSYSFILE)));

    // Lie le contrôle ListView à la liste d'icône
    ListView_SetImageList(m_hwndLV, m_hImgLst, LVSIL_SMALL);

    // Crée la liste d'images d'états pour le contrôle ListView
    m_hImgLstSt = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), TRUE, 3, 0);  

    // Ajoute les icones à la liste des images d'état (coché/grisé/décoché)
    ImageList_AddIcon(m_hImgLstSt, LoadIcon(g_hmodThisDll, MAKEINTRESOURCE(IDI_LSTCHK)));
    ImageList_AddIcon(m_hImgLstSt, LoadIcon(g_hmodThisDll, MAKEINTRESOURCE(IDI_LSTGRAYCHK)));
    ImageList_AddIcon(m_hImgLstSt, LoadIcon(g_hmodThisDll, MAKEINTRESOURCE(IDI_LSTUNCHK)));

    // Ajoute un item à la liste pour chaque fichier de la liste des dépandances
    int		iImage;
    char	cStateImage;
    int		bAppfile = TRUE;
    for(CNode	*pEnumNode = m_pcs->m_lpld->Find(NULL);
        pEnumNode != NULL;
        pEnumNode = m_pcs->m_lpld->Next(pEnumNode), bAppfile = FALSE)
    {
        if( pEnumNode->m_bExist ) {
            iImage = bAppfile ? LV_App : ( pEnumNode->IsWinSysFile() ? LV_WinSysDll : LV_Dll );

            if( !pEnumNode->m_bCrackable ) {
                cStateImage = LV_State_None;
            }
            else {
                if( pEnumNode == m_pcs->m_pLoaderFile )
                    cStateImage = LV_State_Checked;
                else if( pEnumNode->m_bUsedByAnotherApp )
                    cStateImage = LV_State_GrayChecked;
                else
                    cStateImage = LV_State_Unchecked;
            }
        }
        else {
            iImage = LV_WinSysDll;
            cStateImage = 0;
        }

        LV_Add(m_hwndLV, ListView_GetItemCount(m_hwndLV),
                        LPSTR_TEXTCALLBACK,
                        LPSTR_TEXTCALLBACK,
                        iImage,
                        pEnumNode->m_bExist ? 0 : LVIS_CUT,
                        cStateImage,
                        (LPARAM)pEnumNode);
    }
	
    // Charge les messages à partir des ressources
    m_S_ERRMSG = LoadResString(g_hResDll, IDS_ERRFILENOTFOUND);
    m_S_LDRNAME = LoadResString(g_hResDll, IDS_LDRNAME);
    m_S_LDRDESC = LoadResString(g_hResDll, IDS_LDRDESC);

    // Pointeur sur le noeud sélectionné
    SetWindowLongPtr (m_hwndLV, GWLP_USERDATA, (LONG_PTR)m_pcs->m_pLoaderFile);

    // Initialise le contrôle listview
    LVInitCol(m_hwndLV, 120, 500);

    return TRUE; 
}

// appelée lors du déchargement
BOOL CDepend::OnDestroy()
{
    // Libère la mémoire alloué par les messages internationals
    FreeResString(m_S_ERRMSG);
    FreeResString(m_S_LDRNAME);
    FreeResString(m_S_LDRDESC);

    SetWindowLong(m_hwnd, DWLP_MSGRESULT, 0);
    return TRUE;
}

// Function name	: LVSelectItem
// Description	    : Change la sélection pour un autre item
// Return type		: BOOL 
// Argument         : int iItem
BOOL CDepend::LVSelectItem( int iItem )
{
    // Obtient les infos sur l'item que l'utilisateur veut cocher
    LV_ITEM	lvi;
    lvi.mask = LVIF_STATE | LVIF_PARAM;
    lvi.stateMask = LVIS_STATEIMAGEMASK;
    lvi.iItem = iItem;
    lvi.iSubItem = 0;
    lvi.lParam = NULL;
    ListView_GetItem(m_hwndLV, &lvi);

    CNode   // pointeur sur le fichier correspondant a l'item que l'utilisateur veut cocher
            *pNewSel = (CNode *)lvi.lParam,
            // pointeur sur le fichier actuellement coché
            *pSelectedNode = m_pcs->m_pLoaderFile;
    
    // si l'utilisateur veut cocher l'item qui est deja coche 
    // alors il n'y a rien a faire
    if( pSelectedNode == pNewSel ) 
        return TRUE;
    
    // si l'utilisateur tente de cocher un item qui n'est pas cochable
    // alors on ne fait rien
    if( lvi.state == LV_State_None )
        return TRUE;
    

    // Coche l'item qui a ete demande par l'utilisateur
    lvi.state = INDEXTOSTATEIMAGEMASK((UINT)LV_State_Checked);
    if ( !ListView_SetItem(m_hwndLV, &lvi) )
        return FALSE;

    lvi.mask = LVIF_STATE | LVIF_PARAM;
    lvi.stateMask = LVIS_STATEIMAGEMASK;
    lvi.iSubItem = 0;

    // Decoche l'item actuellement coché
    if( pSelectedNode )
        for(lvi.iItem=0; lvi.iItem<ListView_GetItemCount(m_hwndLV); lvi.iItem++) {
            ListView_GetItem(m_hwndLV, &lvi);
            if( (CNode *)lvi.lParam == pSelectedNode ) {
                lvi.state = INDEXTOSTATEIMAGEMASK((UINT)( pSelectedNode->m_bUsedByAnotherApp ? LV_State_GrayChecked : LV_State_Unchecked));
                ListView_SetItem(m_hwndLV, &lvi);
                break;
            }
        }

    // MAJ le pointeur sur le fichier cocheé
    m_pcs->m_pLoaderFile = pNewSel;
    m_pcs->OnModification();

    return TRUE;
}


// Méthode appelée pour rafraichir le contenu des contrôles
// à partir des données de l'objet CShellExt
void CDepend::LoadControlData ()
{
    // radio button Runtime/Static
    CheckRadioButton(m_hwnd, IDC_RADRUNTIMEINJEC, IDC_RADSTATICINJEC, m_pcs->m_pLoaderFile ? IDC_RADSTATICINJEC : IDC_RADRUNTIMEINJEC);

    // Change le texte informatif suivant que le system-wide injection soit actif ou non
    PTSTR pszRes = LoadResString(g_hResDll, 
                GetSystemwideInjectionMode() != SYSTEMWIDE_NONE
                ? IDS_SYSTEMWIDEACTIVATED : IDS_SYSTEMWIDEDEACTIVATED );
    BOOL bRet = SetDlgItemText(m_hwnd, IDC_RUNTIMEINFO, pszRes);
    FreeResString(pszRes);	
    
    //SetControlsSize();
    RefreshControl();
}

void CDepend::RefreshControl()
{
    // injection statique?
    BOOL bStatic = IsDlgButtonChecked(m_hwnd, IDC_RADSTATICINJEC);

    // Grise/degrise la liste de dependances
    EnableWindow(m_hwndLV, m_pcs->m_appSettings.m_bActive && bStatic);

    // Grise/degrise le texte d'infos et le lien de creation de raccourci
    EnableWindow(GetItem(IDC_LNKCREATESHORTCUT), m_pcs->m_appSettings.m_bActive && !bStatic );
    EnableWindow(GetItem(IDC_RUNTIMEINFO), m_pcs->m_appSettings.m_bActive && !bStatic );
    
    EnableWindow(GetItem(IDC_RADRUNTIMEINJEC), m_pcs->m_appSettings.m_bActive );
    EnableWindow(GetItem(IDC_RADSTATICINJEC), m_pcs->m_appSettings.m_bActive );
    
    EnableWindow(GetItem(IDC_GRPSTATIC), m_pcs->m_appSettings.m_bActive );
    EnableWindow(GetItem(IDC_GRPRUNTIME), m_pcs->m_appSettings.m_bActive );

    
    // Lie le contrôle ListView à une listes d'images d'états
    ListView_SetImageList(m_hwndLV, bStatic ? m_hImgLstSt : NULL, LVSIL_STATE);
}



// Méthode appelée pour mettre à jour les données de l'objet CShellExt
// à partir des contrôles de l'onglet
void CDepend::SaveControlData()
{
}

BOOL CDepend::OnNotify(int idCtrl, LPNMHDR pnmh)
{
    switch(pnmh->code)
    {
    // Hyperlink
    case NM_CLICK:
     {
        PNMLINK pNmLink = (PNMLINK) pnmh;

        if( pNmLink->item.szUrl && *pNmLink->item.szUrl )
            ShellExecute(m_hwnd, NULL, pNmLink->item.szUrl , NULL, NULL, SW_SHOWDEFAULT);
        else if( _tcscmp(_T("makeshortcut"), pNmLink->item.szID)  == 0)
            m_pcs->CreateLoaderShortcut(CSIDL_DESKTOP);
     }

    // Process LVN_GETDISPINFO to supply information about 
    // callback items.
    case LVN_GETDISPINFO:
    {
        // Provide the item or subitem's text, if requested. 
        NMLVDISPINFO *pnmv = (NMLVDISPINFO *)pnmh;
        if (pnmv->item.mask & LVIF_TEXT)
        {
            CNode *pItem = (CNode *)pnmv->item.lParam;
            switch( pnmv->item.iSubItem )
            {
            case 1: // chemin du fichier
                if( pItem )
                {
                    if( pItem->m_bExist )						
                        pItem->GetDirectory(pnmv->item.pszText, pnmv->item.cchTextMax);
                    else // "Fichier non trouvé !"													
                        _tcscpy_s(pnmv->item.pszText, pnmv->item.cchTextMax, m_S_ERRMSG);
                }
                else	// item pour le chargemement avec Manager
                    _tcscpy_s(pnmv->item.pszText, pnmv->item.cchTextMax, m_S_LDRDESC);
                break;
            case 0:	// Nom du fichier
                if( pItem )
                    _tcscpy_s(pnmv->item.pszText, pnmv->item.cchTextMax, pItem->m_pszBaseName );
                else	// item pour le chargemement avec Manager
                    _tcscpy_s(pnmv->item.pszText, pnmv->item.cchTextMax, m_S_LDRNAME);
                break;
            }
        }
        break;
    }

    case LVN_ITEMCHANGED :
        #define pnmv	((NMLISTVIEW *)pnmh)
		
        if((pnmv->uNewState & LVIS_SELECTED) == LVIS_SELECTED )
            LVSelectItem(pnmv->iItem);
        #undef pnmv
        break;

    case LVN_COLUMNCLICK:
        LV_ColumnClick( (LPARAM)pnmh, LV_Compare );
        break;

    default:
        return FALSE;
    }

    SetWindowLong(m_hwnd, DWLP_MSGRESULT, 0);
    return TRUE;
}


BOOL CDepend::OnCommand(WORD notifcode, WORD id, HWND hwndItem)
{
    switch( notifcode ) {
        case BN_CLICKED:
            switch( id ) {
                case IDC_RADRUNTIMEINJEC:
                    m_pcs->m_pLoaderFile = NULL;
                    m_pcs->OnModification();
                    RefreshControl();
                    break;

                case IDC_RADSTATICINJEC:
                    LVSelectItem(0);
                    m_pcs->OnModification();
                    RefreshControl();
                    break;
            }
            break;

        default:
            break;
    }
    return FALSE;
}


/*

DATADECLSPECIFIER HCURSOR		g_hlinkCursor;				// curseur pour le lien e-mail

// Crée les curseurs
g_hlinkCursor = LoadCursor(g_hmodThisDll, MAKEINTRESOURCE(IDC_LINK));

DestroyCursor( g_hlinkCursor );

BOOL CDepend::OnCtlColorStatic(HWND hwnd, HDC hdc)
{
    if( hwnd == GetItem(IDC_LNKCREATESHORTCUT) )
    {
        SetBkColor(hdc, GetSysColor(COLOR_3DFACE));
        SetTextColor(hdc, GetHotLinkColor());
        return (LRESULT)g_hBrSYSCOLOR_3DFACE;
    }
    return FALSE;
}

BOOL CDepend::OnSetCursor()
{
    POINT pt;
    HWND  hwnd;
	
    GetCursorPos(&pt);
    ScreenToClient(m_hwnd, &pt);
    hwnd = RealChildWindowFromPoint(m_hwnd, pt);
	
    // Curseur sur un hyperlien ?
    if(  hwnd == GetItem(IDC_LNKCREATESHORTCUT) )
    {
        SetCursor(g_hlinkCursor);
        SetWindowLong(m_hwnd, DWLP_MSGRESULT, 0);
        return TRUE;
    }
    return FALSE;
}

BOOL CDepend::OnLButtonDown(LONG x, LONG y)
{
    POINT pt = { x, y };
    HWND  hwnd = RealChildWindowFromPoint(m_hwnd, pt);
	
    // Clique sur un hyperlien ?		
    if( hwnd == GetItem(IDC_LNKCREATESHORTCUT) )
        CreateLoaderShortcut(CSIDL_DESKTOP);
    else
        return FALSE;

    SetWindowLong(m_hwnd, DWLP_MSGRESULT, 0);
    return TRUE;
}

// Procédure de la fenêtre general
BOOL CDepend::SheetProc(UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    switch (uMessage)
    {
    case WM_CTLCOLORSTATIC:
        return OnCtlColorStatic((HWND)lParam, (HDC)wParam);

    case WM_SETCURSOR:
        return OnSetCursor();

    case WM_LBUTTONDOWN:
        return OnLButtonDown(LOWORD(lParam), HIWORD(lParam));
    }
    return FALSE;
}

*/