//  MODULE:    TabMoreOptions.cpp
//
//  PURPOSE:   Implemente l'onglet MoreOptions correspondant a la ressource IDD_MORETABOPTIONS
//
//

#include "StdAfx.h"						// Header pr�compil�
#include "..\Resources\Resources.h"		// Resources localis�s
#include "ShellExt.h"					// D�fition de l'objet CShellExt
#include "DLLMain.h"					// Prototypes des fonctions de DLLMain


CMoreOptions::CMoreOptions()
{
	m_wIDTab = IDD_TABMOREOPTIONS;
}

// Initialise la page de propri�t�
BOOL CMoreOptions::OnInit(HWND hDlg)
{
	SetWindowLong(m_hwnd, DWLP_MSGRESULT, 0);
	return TRUE;
}


// Fonction appel�e pour rafraichir le contenu des contr�les
// � partir des donn�es de l'objet CShellExt
void CMoreOptions::LoadControlData()
{			
    SetDlgItemText(m_hwnd, IDC_EDTCMDLINE, m_pcs->m_szPEFileName);
    SetDlgItemText(m_hwnd, IDC_EDTCMDPARAM, m_pcs->m_appSettings.m_szCmdParameters);
    CheckRadioButton(m_hwnd, IDC_RADDEFAULT, IDC_RADCOMPLETELYSTANDALONE,
            m_pcs->m_appSettings.m_bStandalone ?  IDC_RADCOMPLETELYSTANDALONE
                                 : m_pcs->m_appSettings.m_bLocalStorage ? IDC_RADLOCALSETTINGS : IDC_RADDEFAULT);
}

// M�thode appel�e pour mettre � jour les donn�es de l'objet CShellExt
// � partir des contr�les de l'onglet
void CMoreOptions::SaveControlData()
{
    GetDlgItemText(m_hwnd, IDC_EDTCMDPARAM, m_pcs->m_appSettings.m_szCmdParameters, _countof(m_pcs->m_appSettings.m_szCmdParameters));
    m_pcs->m_appSettings.m_bStandalone = IsDlgButtonChecked(m_hwnd, IDC_RADCOMPLETELYSTANDALONE) == BST_CHECKED;
    m_pcs->m_appSettings.m_bLocalStorage = m_pcs->m_appSettings.m_bStandalone || IsDlgButtonChecked(m_hwnd, IDC_RADLOCALSETTINGS) == BST_CHECKED;
}

BOOL CMoreOptions::OnCommand (WORD wNotifyCode, WORD wID, HWND hwndCtl)
{		
    // Active le bouton 'apply' si une modification a eu lieu
	switch(wNotifyCode) {
    case EN_UPDATE:
        if( wID == IDC_EDTCMDPARAM ) {
            LRESULT lResult = SendMessage(GetItem(IDC_EDTCMDPARAM), EM_GETMODIFY,0,0);  
            if( lResult ) 
		        m_pcs->OnModification();
        }
        break;

	case BN_CLICKED:					
		switch(wID) {					
		case IDC_RADDEFAULT:
        case IDC_RADLOCALSETTINGS:
        case IDC_RADCOMPLETELYSTANDALONE:
    		m_pcs->OnModification();
			break;
		default:
			break;
		}
		break;

	default:
		return FALSE;
	}		
	SetWindowLong(m_hwnd, DWLP_MSGRESULT, 0);
	return TRUE;
}

// Proc�dure de la fen�tre option
INT_PTR CMoreOptions::SheetProc(UINT uMessage, WPARAM wParam, LPARAM lParam)
{
/*
	switch (uMessage)
	{

	}
*/
	return FALSE;
}


// appel�e lors du d�chargement de la page
BOOL CMoreOptions::OnDestroy()
{
	SetWindowLong(m_hwnd, DWLP_MSGRESULT, 0);
	return TRUE;
}