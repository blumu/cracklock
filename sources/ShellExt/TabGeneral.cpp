// TabGeneral.cpp

#include "StdAfx.h"			// Header précompilé
#include "..\Resources\Resources.h"	// Resources localisés
#include "Resource.h"		// Resources
#include "ShellExt.h"		// Définition de la classe CShellExt
#include "Dllmain.h"

CGeneral::CGeneral()
{
	m_wIDTab = IDD_TABGENERAL;
}

// Méthode appelée pour rafraichir le contenu des coontrôles
// à partir des données de l'objet CShellExt
void CGeneral::LoadControlData()
{		
	// Configure les cases à cocher
	CheckDlgButton(m_hwnd, IDC_CHKACTIVATE, m_pcs->m_appSettings.m_bActive ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(m_hwnd, IDC_CHKADVANCED, m_pcs->m_bShowAdvanced ? BST_CHECKED : BST_UNCHECKED);	
}

// Méthode appelée pour mettre à jour les données de l'objet CShellExt
// à partir des contrôles de l'onglet
void CGeneral::SaveControlData ()
{
	m_pcs->m_appSettings.m_bActive = IsDlgButtonChecked(m_hwnd, IDC_CHKACTIVATE) == BST_CHECKED;
	m_pcs->m_bShowAdvanced = IsDlgButtonChecked(m_hwnd, IDC_CHKADVANCED) == BST_CHECKED;
}

// appelée lors de l'initialisation de la page
BOOL CGeneral::OnInit (HWND hDlg)
{
	// Crée et affecte la police de caractère pour le titre
	m_hfntTitle = CreateFont(24,0, 0,0, FW_BLACK, FALSE, FALSE, FALSE, 0, 0, 0, 0, 0, _T("Arial"));
	SendMessage(GetItem(IDC_TITLE), WM_SETFONT, (WPARAM)m_hfntTitle, MAKELPARAM(TRUE, 0) );

	// rajoute la version au bout du titre
    m_hfntSubTitle = CreateFont(13,0, 0,0, FW_MEDIUM, FALSE, FALSE, FALSE, 0, 0, 0, 0, 0, _T("Arial"));
	SendMessage(GetItem(IDC_VERSIONINFO), WM_SETFONT, (WPARAM)m_hfntSubTitle, MAKELPARAM(TRUE, 0) );
#if defined(_X86_)
SetWindowText(GetItem(IDC_VERSIONINFO), INFO_VERSION_T _T(" (32-bit)"));
#elif defined(_AMD64_)
SetWindowText(GetItem(IDC_VERSIONINFO), INFO_VERSION_T _T(" (64-bit)"));
#else
#error Unsupported architecture
#endif  

	// Affecte l'icone de l'horloge au contrôle correspondant
	m_hIcoHorlo = (HICON)LoadImage(g_hmodThisDll, MAKEINTRESOURCE(IDI_DALIHORLO), IMAGE_ICON, 0, 0, NULL );
	SendDlgItemMessage(hDlg, IDC_ICO, STM_SETICON, (WPARAM)m_hIcoHorlo, (LPARAM)0);

	return TRUE;
}

// appelée lors du déchargement de la page
BOOL CGeneral::OnDestroy()
{
	// Libère le handle de police
	DeleteObject(m_hfntTitle);
    DeleteObject(m_hfntSubTitle);

	// détruit l'icône créé
	DeleteObject(m_hIcoHorlo);

	SetWindowLong(m_hwnd, DWLP_MSGRESULT, 0);
	return TRUE;
}

BOOL CGeneral::OnNotify(int idCtrl, LPNMHDR pnmh)
{ 
    switch(pnmh->code)
    {
     // Hyperlink
     case NM_CLICK:
         {
            PNMLINK pNmLink = (PNMLINK) pnmh;

            if( pNmLink->item.szUrl )                 
                ShellExecute(m_hwnd, NULL, pNmLink->item.szUrl , NULL, NULL, SW_SHOWDEFAULT);
         }
    default:
        return FALSE;
    }

    SetWindowLong(m_hwnd, DWLP_MSGRESULT, 0);
    return TRUE;
}


BOOL CGeneral::OnCommand (WORD wNotifyCode, WORD wID, HWND hwndCtl)
{
	switch(wNotifyCode) {		
	case BN_CLICKED:								
		return OnButtonClick(wID);
	}
	return FALSE;
}

BOOL CGeneral::OnButtonClick(WORD wID)
{
	switch(wID)
	{
	case IDC_CHKADVANCED:
		m_pcs->ShowTab( IsDlgButtonChecked(m_hwnd, IDC_CHKADVANCED) == BST_CHECKED );
		// le "break;" est omis vvolontairement
		// ....
	case IDC_CHKACTIVATE:
		m_pcs->OnModification();
		break;
	
	default:
		return FALSE;
	}
	SetWindowLong(m_hwnd, DWLP_MSGRESULT, 0);
	return TRUE;
}



/////////////// Version du hot-link implementant manuelement sans le control syslink
/*

/////////// Couleurs
#define COLOR_HOVERED_LINK          RGB(250,167,0)

HWND lasthovered = NULL;

BOOL CGeneral::OnCtlColorStatic(HWND hwnd, HDC hdc)
{
	if( hwnd == GetItem(IDC_MAIL_LINK) ||
		hwnd == GetItem(IDC_MB_LINK)
		)
	{
		SetBkColor(hdc, GetSysColor(COLOR_3DFACE));
		SetTextColor(hdc, (IsCursorOnControl(hDlg,IDC_MAIL_LINK)|| IsCursorOnControl(hDlg,IDC_MB_LINK)) ? 
                                COLOR_HOVERED_LINK : GetHotLinkColor());
		return (LRESULT)g_hBrSYSCOLOR_3DFACE;
	}
	return FALSE;
}




BOOL CGeneral::OnSetCursor()
{
	POINT pt;
	HWND  hwnd;
	
	GetCursorPos(&pt);
	ScreenToClient(m_hwnd, &pt);
	hwnd = ChildWindowFromPoint(m_hwnd, pt);
	
	// Curseur sur un hyperlien ?
	if(  hwnd == GetItem(IDC_MAIL_LINK) ||
		 hwnd == GetItem(IDC_MB_LINK)
		 )
	{
		SetCursor(g_hlinkCursor);
		SetWindowLong(m_hwnd, DWLP_MSGRESULT, 0);
		return TRUE;
	}
	return FALSE;
}


BOOL CGeneral::OnLButtonDown(LONG x, LONG y)
{
	POINT pt = { x, y };
	HWND  hwnd = ChildWindowFromPoint(m_hwnd, pt);
	
	// Clique sur un hyperlien ?		
	if( hwnd == GetItem(IDC_MAIL_LINK) )
		ShellExecute(m_hwnd, NULL, g_S_EMAILLINK, NULL, NULL, SW_SHOWDEFAULT);
	else if( hwnd == GetItem(IDC_MB_LINK) )
		ShellExecute(m_hwnd, NULL, g_S_MBLINK, NULL, NULL, SW_SHOWDEFAULT);
	else
		return FALSE;

	SetWindowLong(m_hwnd, DWLP_MSGRESULT, 0);
	return TRUE;
}




// Procédure de la fenêtre general
BOOL CGeneral::SheetProc(UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_CTLCOLORSTATIC:
		return OnCtlColorStatic((HWND)lParam, (HDC)wParam);

	case WM_SETCURSOR:
		return OnSetCursor();

	case WM_LBUTTONDOWN:
		return OnLButtonDown(LOWORD(lParam), HIWORD(lParam));


    case WM_MOUSELEAVE:
        if( lasthovered == GetItem(IDC_MAIL_LINK) ||  lasthovered == GetItem(IDC_MB_LINK) ) {
            lasthovered = NULL;
            InvalidateRect(GetItem(IDC_MAIL_LINK), NULL, FALSE);
            InvalidateRect(GetItem(IDC_MB_LINK), NULL, FALSE);
        }
        lasthovered = NULL;
        break;

    case WM_MOUSEMOVE:
        {
            POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    		HWND hovered = RealChildWindowFromPoint(hDlg, pt);
            if( hovered != lasthovered && 
                (    hovered == GetItem(IDC_MAIL_LINK) || lasthovered == GetItem(IDC_MAIL_LINK) 
                  || hovered == GetItem(IDC_MB_LINK) || lasthovered == GetItem(IDC_MB_LINK) 
                ) )
            {
                InvalidateRect(hTitle, NULL, FALSE);
            
                if( hovered == GetItem(IDC_MAIL_LINK) ||  hovered == GetItem(IDC_MB_LINK) ) {
                    // MouseLeave event
                    TRACKMOUSEEVENT tme;
                    tme.cbSize = sizeof(tme);
                    tme.dwFlags = TME_LEAVE;
                    tme.hwndTrack = hDlg;
                    TrackMouseEvent(&tme);
                }
            }

            lasthovered = hovered;
        }
	}
	return FALSE;
}
*/