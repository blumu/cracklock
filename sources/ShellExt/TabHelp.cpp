// TabHelp.cpp

#include "StdAfx.h"			// Header précompilé
#include "resource.h"		// Resources
#include "..\Resources\Resources.h"	// Resources localisés
#include "ShellExt.h"		// Définition de la classe CShellExt

CHelp::CHelp()
{
	m_wIDTab = IDD_TABHELP;
}

// Initialise la page d'aide
BOOL CHelp::OnInit(HWND hDlg)
{

	static int list_icone[] = {	    IDI_LSTAPPEXE,		IDC_LSTAPPEXE,
									IDI_LSTCHK,			IDC_LSTCHK,
									IDI_LSTFILE,		IDC_LSTFILE,
									IDI_LSTGRAYCHK,		IDC_LSTGRAYCHK,
									IDI_LSTUNCHK,		IDC_LSTUNCHK,
									IDI_LSTWINSYSFILE,	IDC_LSTWINSYSFILE
								};
	
	_ASSERT(_countof(m_hIcon) >= _countof(list_icone)/2);
	
	int cx = GetSystemMetrics(SM_CXSMICON),
		cy = GetSystemMetrics(SM_CYSMICON);
 
	// Affecte une image a chaque contrôle
	for( int i=0; i<_countof(m_hIcon); i++) {		
		m_hIcon[i] = (HICON)LoadImage(g_hmodThisDll, MAKEINTRESOURCE(list_icone[2*i]), IMAGE_ICON, cx, cy, NULL);
		SendDlgItemMessage(m_hwnd, list_icone[2*i+1], STM_SETICON, (WPARAM)m_hIcon[i], (LPARAM)0);
	}

	SetWindowLong(m_hwnd, DWLP_MSGRESULT, 0);
	return TRUE;
}

// appelée lors du déchargement de la page
BOOL CHelp::OnDestroy()
{
	// détruit les icônes créés
	for( int i=0; i<_countof(m_hIcon); i++)
		DeleteObject(m_hIcon[i]);

	SetWindowLong(m_hwnd, DWLP_MSGRESULT, 0);
	return TRUE;
}


BOOL CHelp::OnCommand (WORD wNotifyCode, WORD wID, HWND hwndCtl)
{
	if( (wNotifyCode == BN_CLICKED) && 
		(wID == IDC_BTHELP)
		)
	{
		OnHelp(0);
		SetWindowLong(m_hwnd, DWLP_MSGRESULT, 0);
		return TRUE;
	}
	return FALSE;
}
