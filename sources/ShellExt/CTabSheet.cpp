// CTabSheet.cpp

#include "stdafx.h"
#include "..\Resources\Resources.h"	// Resources localisés
#include "ShellExt.h"		// Défition de l'objet CShellExt

CTabSheet::CTabSheet ( void )
{
	m_pcs = NULL;
	m_hwnd = NULL;
	m_hTab = NULL;
	m_iTabIndex = -1;
}

CTabSheet::~CTabSheet ( void )
{
	DestroyWindow(m_hwnd);
}


// Function name	: TabInsert
// Description	    : Insert un onglet au control tab.
//                    La fenêtre de l'onglet est cree et associee a l'objet CTabSheet.
// Return type		: void
// Argument         : HWND hParent		handle de la fenetre contenant le control tab
// Argument         : HWND hTab				handle du control tab	
// Argument         : PCTSTR pszTitle titre de l'ongle tab	
// Argument         : int iItem				numero de l'onglet
// Argument         : BOOL bVisible		vrai si l'onglet doit etre visible
void CTabSheet::TabInsert(HWND hParent, HWND hTab, PTSTR pszTitle, int iItem, BOOL bVisible)
{
	// creation de la fenetre correspondant a cet onglet
	HWND hwnd = CreateDialogParam(g_hResDll, MAKEINTRESOURCE(m_wIDTab), hParent, TabSheetProc, (LPARAM)this);


	// ajoute l'onglet
	if( bVisible )
	{
		TCITEM tie;
		tie.mask = TCIF_TEXT | TCIF_PARAM;
		tie.iImage = -1;
 		tie.pszText = pszTitle;
		tie.lParam = (LPARAM)this;
		m_iTabIndex = TabCtrl_InsertItem(hTab, iItem, &tie);
		m_hTab = hTab;
	}
}

// Function name	: TabSelect
// Description	    : Sélectionne l'onglet de l'objet CTabSheet dans le contrôle tab
// Return type		: int 
int CTabSheet::TabSelect()
{
	if( m_hTab )
		return TabCtrl_SetCurSel(m_hTab, m_iTabIndex);
	
	return -1;
}

	
// Function name	: OnHelp
// Description	    : Affiche une aide 	selon le contrôle sur lequel se trouve le curseur
// Return type		: BOOL
BOOL CTabSheet::OnHelp (DWORD dwID)
{	

	int i, iCnt;
	DWORD dwHelp;
	static HELPMAPENTRY helpmap[] =
	{
		{ IDC_CHKACTIVATE,	IDS_HLPCONFIG },
		{ IDC_CHKSYS,		IDS_HLPCONFIG },
		
	};

	
	// fichier par défaut
	dwHelp = IDS_HLPCONFIG;

	// Cherche l'id du fichier d'aide correspondant au contrôle demandé
	iCnt = sizeof(helpmap) / sizeof(HELPMAPENTRY);
	for(i=0; i<iCnt; i++)
		if(helpmap[i].idCtrl == dwID )
		{
			dwHelp = helpmap[i].idCtrl;
			break;
		}

	// invoke l'aide
	InvokeHelp(m_hwnd, dwHelp);
	SetWindowLong(m_hwnd, DWLP_MSGRESULT, 0);
	return TRUE;
}

INT_PTR CALLBACK TabSheetProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	CTabSheet *pcts = (CTabSheet *)GetWindowLong(hDlg, DWLP_USER);

	if( uMessage == WM_INITDIALOG )
	{
		pcts = (CTabSheet *)lParam;
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pcts );
	
		// affecte la police unicode a tous les controles contenus dans cet onglet
		//EnumChildWindows(hDlg, EnumChildSetFont, (LPARAM)pcts->m_pcs->m_hfUnicode);
		
		// Initialise la page
		pcts->m_hwnd = hDlg;
		return pcts->OnInit(hDlg);
	}

	if( !pcts )
		return FALSE;

	switch (uMessage)
	{
	case WM_COMMAND:
		return pcts->OnCommand(
			(WORD)HIWORD(wParam),	// notification code
			(WORD)LOWORD(wParam),	// item, control, or accelerator identifier
			(HWND)lParam);			// handle of control

	case WM_NOTIFY:
		return pcts->OnNotify((int)wParam, (LPNMHDR)lParam);

	case WM_CHAR:
		if( (TCHAR)wParam == VK_F1 )
		{
			pcts->OnHelp(0);
			SetWindowLong(hDlg, DWLP_MSGRESULT, 0);
			return TRUE;
		}
		return pcts->SheetProc(uMessage, wParam, lParam);

	case WM_HELP:
		return pcts->OnHelp(((LPHELPINFO)lParam)->dwContextId);
	
	case WM_DESTROY:
		return pcts->OnDestroy();
	}

	return pcts->SheetProc(uMessage, wParam, lParam);
}
