//  MODULE:    TabMoreOptions.cpp
//
//  PURPOSE:   Implemente l'onglet Options correspondant a la ressource IDD_OPTIONS


#include "StdAfx.h"						// Header précompilé
#include "..\Resources\Resources.h"		// Resources localisés
#include "ShellExt.h"					// Défition de l'objet CShellExt
#include "TimeCtrl.h"					// Contrôles temps/date
#include "DLLMain.h"					// Prototypes des fonctions de DLLMain

// Sauve le contenu d'un ctrl 'edit'
#define SaveEditLong(hctl, lval)    (SetWindowLong( hctl, GWLP_USERDATA, (LONG)lval ))

// Charge la sauvegarde d'un ctrl 'edit'
#define LoadEditLong(hctl)          (GetWindowLong( hctl, GWLP_USERDATA ))

// Sauve le contenu d'un ctrl 'list'
void SaveListContent(HWND hctl)
{
	SetWindowLong(hctl, GWLP_USERDATA,
		(LONG)SendMessage(hctl, LB_GETCURSEL, 0, 0)) ;
}

// Charge la sauvegarde d'un ctrl 'list'
void LoadListContent(HWND hctl)
{
	SendMessage (hctl, LB_SETCURSEL, 
		(LONG)GetWindowLong(hctl, GWLP_USERDATA), 0) ;
}

// Charge une resource en mémoire
//#define MapResource(hmod, name, type)	LockResource(LoadResource(hmod, FindResource(hmod, name, type)))
PVOID MapResource(HMODULE hmod, PTSTR name, PTSTR type)
{
	HRSRC	hRsrc;
	HGLOBAL	hGlob;
	PVOID	lpData;
	
	hRsrc = FindResource(hmod, name, type);
	hGlob = LoadResource(hmod, hRsrc);
	lpData = LockResource(hGlob);
	
	return lpData;
}

COptions::COptions()
{
	m_wIDTab = IDD_TABOPTIONS;
}

void setHeight(HWND hwnd, int height)
{
	RECT rc;
	GetWindowRect(hwnd,&rc);
	SetWindowPos(hwnd, NULL, 0, 0, rc.right-rc.left,
		height, SWP_NOMOVE|SWP_NOZORDER|SWP_SHOWWINDOW);
}

// Initialise la page de propriété
BOOL COptions::OnInit(HWND hDlg)
{
	// Configure l'affichage en fonctions des params systèmes
	MAJInternational();

	// Config le spin up/down des années
	SendMessage(GetItem(IDC_SPINYEAR), UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)MAX_YEAR, (short)MIN_YEAR));
	
	// Config le ctrl edit des années
	SendMessage(GetItem(IDC_EDYEAR), EM_SETLIMITTEXT, 4, 0);

	
	///// corrige la hauteur des boites EDIT pour l'heure/minute/seconde
	///// (l'editeur de resources ne permet pas de les dessinner aussi petits)
	///// ainsi que leur position
	RECT rc, rcParent;
	int height, width, widthsep;
	
	GetClientRect(GetItem(IDC_LSTAMPM),&rc);
	height = rc.bottom;

	GetClientRect(GetItem(IDC_SEPTIME0),&rc);
	widthsep = rc.right;

	GetWindowRect(m_hwnd, &rcParent);
	GetWindowRect(GetItem(IDC_EDTM0),&rc);
	width = rc.right-rc.left ;

	rc.left -= rcParent.left;
	rc.top -= rcParent.top;
	MoveWindow(GetItem(IDC_EDTM0), rc.left, rc.top, width, height, TRUE);
	rc.left += width;

	MoveWindow(GetItem(IDC_SEPTIME0), rc.left, rc.top, widthsep, height, TRUE);
	rc.left += widthsep;

	MoveWindow(GetItem(IDC_EDTM1), rc.left, rc.top, width, height, TRUE);
	rc.left += width;

	MoveWindow(GetItem(IDC_SEPTIME1), rc.left, rc.top, widthsep, height, TRUE);
	rc.left += widthsep;

	MoveWindow(GetItem(IDC_EDTM2), rc.left, rc.top, width, height, TRUE);	
	rc.left += width;

	if(!g_b24Hours)
	{
		RECT rcampm;
		GetClientRect(GetItem(IDC_LSTAMPM), &rcampm);

		MoveWindow(GetItem(IDC_LSTAMPM), rc.left, rc.top, width*2, height, TRUE);
		rc.left += rcampm.right;
	}

	RECT rcspin;
	GetClientRect(GetItem(IDC_SPINAMPM), &rcspin);
	MoveWindow(GetItem(IDC_SPINAMPM), rc.left+5, rc.top, rcspin.right, height, TRUE);


	//////////////////////

	SetWindowLong(m_hwnd, DWLP_MSGRESULT, 0);

	return TRUE;
}

void COptions::PopulateComboTimeZones(HWND hCombo)
{	
	ComboBox_ResetContent(hCombo);
	for (unsigned int cnt = 0; cnt < m_TimeZoneInfoManager.m_arrRegTimeZoneInfo.size(); cnt++) {
		CRegTimeZoneInfo* pRegTimeZoneInfo = m_TimeZoneInfoManager.m_arrRegTimeZoneInfo[cnt];
		ComboBox_AddString(hCombo,pRegTimeZoneInfo->m_szDisplay);
	}
	ComboBox_SetCurSel(hCombo, m_TimeZoneInfoManager.GetCurrentTimeZoneIndex());
}



// Configure l'affichage en fonction des paramètres internationaux
void COptions::MAJInternational()
{
	TCHAR LCData[200];
	LCID lcidUser = GetUserDefaultLCID();

	////////////////////////////
	/////// Partie heure
	GetLocaleInfo (lcidUser, LOCALE_ITIME, LCData, 200);

	g_b24Hours = !(LCData[0] == '0');
	if( g_b24Hours ) // AM/PM 12 heures
		ShowWindow(GetItem(IDC_LSTAMPM), SW_HIDE);
	else  // 24 heures
	{
		// AM string
		GetLocaleInfo (lcidUser, LOCALE_S1159, LCData, 200);
		SendMessage(GetItem(IDC_LSTAMPM), LB_ADDSTRING, 0, (LPARAM) &LCData);
		// PM string
		GetLocaleInfo (lcidUser, LOCALE_S2359, LCData, 200);
		SendMessage(GetItem(IDC_LSTAMPM), LB_ADDSTRING, 0, (LPARAM) &LCData);

		ShowWindow(GetItem(IDC_LSTAMPM), SW_SHOW);
	}

	// Séparateur de temps
	GetLocaleInfo (lcidUser, LOCALE_STIME, LCData, 200);
	SendMessage(GetItem(IDC_SEPTIME0), WM_SETTEXT, 0, (LPARAM) &LCData);
	SendMessage(GetItem(IDC_SEPTIME1), WM_SETTEXT, 0, (LPARAM) &LCData);

	// Leading zeros de l'heure ?
	GetLocaleInfo (lcidUser, LOCALE_ITLZERO, LCData, 1);
	g_bLeadingHour = (LCData[0] == '1');

	// Fixe le max. de car. pour les contrôles EDIT de l'heure
	SendMessage(GetItem(IDC_EDTM0), EM_SETLIMITTEXT, (WPARAM) 2, 0);
	SendMessage(GetItem(IDC_EDTM1), EM_SETLIMITTEXT, (WPARAM) 2, 0);
	SendMessage(GetItem(IDC_EDTM2), EM_SETLIMITTEXT, (WPARAM) 2, 0);


	////////////////////////////
	/////// Partie date
	UINT iSaveIndex = (UINT)SendMessage(GetItem(IDC_CMBMONTH), CB_GETCURSEL, 0, 0);
	
	// Ajoute les mois dans le combo  
	SendMessage(GetItem(IDC_CMBMONTH), CB_RESETCONTENT, 0, 0);
	for(UINT i=0; i<NB_MONTHSINYEAR; i++)
	{
		GetLocaleInfo (lcidUser, LOCALE_SMONTHNAME1+i, LCData, 200);
		LCData[0] = toupper(LCData[0]);
		SendMessage(GetItem(IDC_CMBMONTH), CB_ADDSTRING, 0, (LPARAM) &LCData);
	}
	SendMessage(GetItem(IDC_CMBMONTH), CB_SETCURSEL, iSaveIndex, NULL);

	
	////////////////////////////////
	///// Partie Timezome
	// Rempli le combo avec toutes les timezones possibles declarees dans la base de registre
	PopulateComboTimeZones(GetItem(IDC_CMBTIMEZONE));
}


// Vérifie le contenu d'un contrôle EDIT
void COptions::CheckEditContent( HWND hwndCtl, BOOL bAcceptEmpty )
{
	TCHAR szZONE[5];
  
	// Obtient le texte du ctrl EDIT
	SendMessage( hwndCtl, WM_GETTEXT, 5, (WPARAM)szZONE);
	// si c'est vide est que c'est accepté alors pas de vérification
	if( (szZONE[0]=='\0') && bAcceptEmpty )
		return;

	if (hwndCtl == GetItem(IDC_EDTM0) ||
		hwndCtl == GetItem(IDC_EDTM1) ||
		hwndCtl == GetItem(IDC_EDTM2))
	{
		TCHAR cTIME = _wtoi(szZONE);

		// Si le contenu de la zone n'est pas conforme...
		if( hwndCtl == GetItem(IDC_EDTM0) 			
			
			?	(cTIME > (g_b24Hours ? 23 : 12) ||
				cTIME < (g_b24Hours ? 0 : 1)   ||
				szZONE[0] == '\0')

			:	(cTIME > 59 ||
				cTIME < 0) ||
				szZONE[0] == '\0')
		{
  
			// Remplace par la dernière valeur conforme
			_itow_s( (int)LoadEditLong(hwndCtl), szZONE, 10);
			SendMessage( hwndCtl, WM_SETTEXT, 0, (LPARAM)szZONE);
			SendMessage( hwndCtl, EM_SETSEL, 0, (LPARAM)-1 );
		}
	    else
		{
			SYSTEMTIME ST_new;
			WORD wFlags;
      
			// Enregistre la valeur de la zone heure
			SaveEditLong(hwndCtl, cTIME);

			if (hwndCtl == GetItem(IDC_EDTM0))
			{
				wFlags = ST_HOUR;
				ST_new.wHour = cTIME;
			}
			if (hwndCtl == GetItem(IDC_EDTM1))
			{
				wFlags = ST_MIN;
				ST_new.wMinute = cTIME;
			}
			if (hwndCtl == GetItem(IDC_EDTM2))
			{
				wFlags = ST_SEC;
				ST_new.wSecond = cTIME;
			}

			// Met à jour l'horloge analogique
			SendMessage( GetItem(IDC_CLOCK), CC_SETTIME, wFlags, (LPARAM)&ST_new );
		}
	} 
	else if (hwndCtl == GetItem(IDC_EDYEAR))
	{
		SYSTEMTIME ST_new;

		ST_new.wYear = _wtoi(szZONE);

		// Si le contenu de la zone n'est pas conforme...
		if (ST_new.wYear<MIN_YEAR || ST_new.wYear>MAX_YEAR || szZONE[0] == '\0')
		{
			// Remplace par la dernière valeur conforme
			_itow_s(max(min((int)LoadEditLong(hwndCtl), MAX_YEAR),MIN_YEAR), szZONE, 10);
			SendMessage( hwndCtl, WM_SETTEXT, 0, (WPARAM)szZONE);
			SendMessage( hwndCtl, EM_SETSEL, 0, (LPARAM)-1 );
		}
		else
		{
			// Enregistre la valeur de la zone année
			SaveEditLong(hwndCtl, ST_new.wYear);
  
			// Met à jour le calendrier
			SendMessage(GetItem(IDC_CALENDAR), CC_SETDATE, SD_YEAR, (LPARAM)&ST_new);
		}
	}
}

// Configure le contrôle SPIN en fonction du contrôle qui a le focus
void COptions::ConfigSpin(HWND hwndCtl)
{
	short min_spin, max_spin;
	SendMessage(GetItem(IDC_SPINAMPM), UDM_SETBUDDY, (WPARAM) (HWND) hwndCtl,0);

	if (hwndCtl==GetItem(IDC_LSTAMPM))
	{
		min_spin = 0;
		max_spin = 1;
	}
	else if (hwndCtl==GetItem(IDC_EDTM0))
	{
		min_spin = (g_b24Hours ? 0 : 1);;
		max_spin = (g_b24Hours ? 23 : 12);
	}
	else
	{
		min_spin = 0;
		max_spin = 59;
	}
	SendMessage(GetItem(IDC_SPINAMPM), UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)max_spin, (short)min_spin));
}

// Adjuste les contrôles date et heure
void COptions::SetDateTime( const SYSTEMTIME *pST )
{
	TCHAR	szTMP[5];
	int		iTmpHour;

	// Active le drapeau de modification par programme
	m_pcs->m_bModifByProgram = true;

	////////////
	// Configuration des contrôles dates

	// Config le ctrl edit des années
	_itow_s(pST->wYear, szTMP, 10);
	SendMessage(GetItem(IDC_EDYEAR), WM_SETTEXT, 0, (LPARAM)szTMP);	
	SaveEditLong( GetItem(IDC_EDYEAR), pST->wYear );

	// Config le combo du mois
	SendMessage(GetItem(IDC_CMBMONTH), CB_SETCURSEL, pST->wMonth-1, NULL);

	////////////
	// Configuration des contrôles heures

	// Configure la liste AM/PM    
		// Si pST->wHour%24>11 alors mode PM (=1) sinon mode AM (=0)
	SendMessage( GetItem(IDC_LSTAMPM), LB_SETCURSEL, pST->wHour%24 > 11, 0 );
	SaveListContent( GetItem(IDC_LSTAMPM) );
	SendMessage(GetItem(IDC_LSTAMPM), LB_SETCURSEL, (WPARAM)-1, 0);

	// Configure les 3 zones heure, minute, seconde
	if( g_b24Hours )
		iTmpHour = pST->wHour % 24;
	else
		iTmpHour = (pST->wHour%12 == 0) ? 12 : pST->wHour%12;

	_itow_s(iTmpHour, szTMP, 10);
	SendMessage(GetItem(IDC_EDTM0), WM_SETTEXT, 0, (LPARAM)szTMP);
	FillTimeField(GetItem(IDC_EDTM0));
	SaveEditLong( GetItem(IDC_EDTM0), pST->wHour );
	
	_itow_s(pST->wMinute, szTMP, 10);
	SendMessage(GetItem(IDC_EDTM1), WM_SETTEXT, 0, (LPARAM)szTMP);
	FillTimeField(GetItem(IDC_EDTM1));
	SaveEditLong( GetItem(IDC_EDTM1), pST->wMinute );

	_itow_s(pST->wSecond, szTMP, 10);
	SendMessage(GetItem(IDC_EDTM2), WM_SETTEXT, 0, (LPARAM)szTMP);
	FillTimeField(GetItem(IDC_EDTM2));
	SaveEditLong( GetItem(IDC_EDTM2), pST->wSecond );

	///////////
	// Maj des ctrls agenda et heure

	// Met à jour le calendrier
	SendMessage( GetItem(IDC_CALENDAR), CC_SETDATE, SD_ALL, (LPARAM)pST);
	// Met à jour l'horloge analogique
	SendMessage( GetItem(IDC_CLOCK), CC_SETTIME, ST_ALL, (LPARAM)pST);

	// Désactive le drapeau de modification par programme
	m_pcs->m_bModifByProgram = false;
}

// Obtient la date et l'heure courrament sélectionnée dans les contrôles
void COptions::GetDateTime( PSYSTEMTIME pST )
{
	TCHAR szTxt[3];

	// obtient la date
	SendMessage( GetItem(IDC_CALENDAR), CC_GETDATE, NULL, (LPARAM)pST);
	
	// obtient l'heure, la minute et la seconde	
	SendMessage(GetItem(IDC_EDTM0), WM_GETTEXT, 3, (LPARAM)&szTxt);
	pST->wHour = _wtoi(szTxt);
	SendMessage(GetItem(IDC_EDTM1), WM_GETTEXT, 3,(LPARAM)&szTxt);
	pST->wMinute = _wtoi(szTxt);
	SendMessage(GetItem(IDC_EDTM2), WM_GETTEXT, 3, (LPARAM)&szTxt);
	pST->wSecond = _wtoi(szTxt);
	pST->wMilliseconds = 0;
	pST->wDayOfWeek = 0;

	// Si on est en mode 12 heures alors convertit en mode 24 heures
	if( !g_b24Hours )
	{		
		if( GetWindowLong(GetItem(IDC_LSTAMPM), GWLP_USERDATA)==1 )
			// MODE PM
			pST->wHour = (pST->wHour+12)%12 + 12;
		else
			// MODE AM
			pST->wHour %= 12;
	}	
	pST->wHour %= 24;
}

// Rempli si nécessaire, un champs temps avec des '0'
void COptions::FillTimeField ( HWND hwndCtl )
{
	if ((hwndCtl == GetItem(IDC_EDTM0) && g_bLeadingHour)||
		hwndCtl == GetItem(IDC_EDTM1) ||
		hwndCtl == GetItem(IDC_EDTM2))
	{
		TCHAR szZONE[4];
		// Rajoute un zéro si nécessaire (ex: 05)
		SendMessage(hwndCtl, WM_GETTEXT, 4, (WPARAM)&szZONE);
		if (szZONE[1] == '\0')
		{
			szZONE[2] = '\0';
			szZONE[1] = szZONE[0];
			szZONE[0] = '0';
			SendMessage(hwndCtl, WM_SETTEXT, 0, (WPARAM)&szZONE);
		}
	}
}

// Met à jour les contrôles en les rendant Enabled ou Disabled suivant
// les paramètres sélectionnés
void COptions::MajEnableControl()
{
	// Porté du patch ? (date et/ou heure)
	bool bTime = BST_CHECKED ==IsDlgButtonChecked(m_hwnd ,IDC_CHKTIME),
		 bDate = BST_CHECKED ==IsDlgButtonChecked(m_hwnd, IDC_CHKDATE),
		 bTz = BST_CHECKED ==IsDlgButtonChecked(m_hwnd, IDC_CHKTZ);

	bTz &= m_pcs->m_appSettings.m_bActive;
	EnableWindow( GetItem(IDC_CMBTIMEZONE), bTz);

	bTime &= m_pcs->m_appSettings.m_bActive;
	EnableWindow( GetItem(IDC_CLOCK), bTime);
	EnableWindow( GetItem(IDC_EDTM0), bTime );
	EnableWindow( GetItem(IDC_EDTM1), bTime );
	EnableWindow( GetItem(IDC_EDTM2), bTime );
	EnableWindow( GetItem(IDC_LSTAMPM), bTime );
	EnableWindow( GetItem(IDC_SPINAMPM), bTime );
	EnableWindow( GetItem(IDC_SEPTIME0), bTime );
	EnableWindow( GetItem(IDC_SEPTIME1), bTime );

	bDate &= m_pcs->m_appSettings.m_bActive;
	EnableWindow( GetItem(IDC_CALENDAR), bDate);	
	EnableWindow( GetItem(IDC_CMBMONTH), bDate );
	EnableWindow( GetItem(IDC_EDYEAR), bDate );
	EnableWindow( GetItem(IDC_SPINYEAR), bDate );
	
	EnableWindow( GetItem(IDC_BTLOADPRESETDT), m_pcs->m_appSettings.m_bActive );
    EnableWindow( GetItem(IDC_INFO), m_pcs->m_appSettings.m_bActive );
	EnableWindow( GetItem(IDC_CHKCST), m_pcs->m_appSettings.m_bActive );
	EnableWindow( GetItem(IDC_CHKSYS), m_pcs->m_appSettings.m_bActive && !m_pcs->m_bDLL);
	EnableWindow( GetItem(IDC_CHKDATE), m_pcs->m_appSettings.m_bActive );
	EnableWindow( GetItem(IDC_CHKTIME), m_pcs->m_appSettings.m_bActive );
	EnableWindow( GetItem(IDC_CHKTZ), m_pcs->m_appSettings.m_bActive );
}

// Fonction appelée pour rafraichir le contenu des coontrôles
// à partir des données de l'objet CShellExt
void COptions::LoadControlData()
{			
	// Configure les contrôles dates et temps
	SetDateTime(&m_pcs->m_appSettings.m_stDateTime);

	// Configue les cases à cocher
	CheckDlgButton(m_hwnd, IDC_CHKCST, m_pcs->m_appSettings.m_bOptConstant ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(m_hwnd, IDC_CHKSYS, m_pcs->m_appSettings.m_bOptSystem ? BST_CHECKED : BST_UNCHECKED);

	// Porté du patch ? (date et/ou heure)
	CheckDlgButton(m_hwnd, IDC_CHKDATE, m_pcs->m_appSettings.m_bDateMode ? BST_CHECKED : BST_UNCHECKED );
	CheckDlgButton(m_hwnd, IDC_CHKTIME, m_pcs->m_appSettings.m_bTimeMode ? BST_CHECKED : BST_UNCHECKED );
	CheckDlgButton(m_hwnd, IDC_CHKTZ, m_pcs->m_appSettings.m_bTimeZoneMode ? BST_CHECKED : BST_UNCHECKED );

	// Timezone
	ComboBox_SetCurSel(GetItem(IDC_CMBTIMEZONE), m_TimeZoneInfoManager.GetTimeZoneIndex(&m_pcs->m_appSettings.m_tzi));

	MajEnableControl();
}

// Méthode appelée pour mettre à jour les données de l'objet CShellExt
// à partir des contrôles de l'onglet
void COptions::SaveControlData()
{
    // date et heure
    GetDateTime(&m_pcs->m_appSettings.m_stDateTime);
    SystemTimeToFileTime(&m_pcs->m_appSettings.m_stDateTime, &m_pcs->m_appSettings.m_ftDateTime);

    // options
    m_pcs->m_appSettings.m_bOptConstant = IsDlgButtonChecked(m_hwnd, IDC_CHKCST) == BST_CHECKED;
    m_pcs->m_appSettings.m_bOptSystem = IsDlgButtonChecked(m_hwnd, IDC_CHKSYS) == BST_CHECKED;

    // Porté du patch ? (date et/ou heure)
    m_pcs->m_appSettings.m_bTimeMode = IsDlgButtonChecked(m_hwnd, IDC_CHKTIME) == BST_CHECKED;
    m_pcs->m_appSettings.m_bDateMode = IsDlgButtonChecked(m_hwnd, IDC_CHKDATE) == BST_CHECKED;
    m_pcs->m_appSettings.m_bTimeZoneMode = IsDlgButtonChecked(m_hwnd, IDC_CHKTZ) == BST_CHECKED;
    m_TimeZoneInfoManager.GetTimeZoneInformationFromIndex(ComboBox_GetCurSel(GetItem(IDC_CMBTIMEZONE)), &m_pcs->m_appSettings.m_tzi);
}

BOOL COptions::OnCommand (WORD wNotifyCode, WORD wID, HWND hwndCtl)
{		
	switch(wNotifyCode) {
	
	case LBN_KILLFOCUS:	// Un ctrl LIST perd le focus
		if(hwndCtl == GetItem(IDC_LSTAMPM)) // Contrôle AM-PM
		{
			SaveListContent( GetItem(IDC_LSTAMPM) );
		
			// enlève la sélection
			SendMessage(GetItem(IDC_LSTAMPM), LB_SETCURSEL, (WPARAM)-1, 0) ;
		}
		break;
			
	case LBN_SETFOCUS:	// Un ctrl LIST prends le focus		
		if(hwndCtl == GetItem(IDC_LSTAMPM)) // Contrôle AM-PM
		{					
			// Charge les données sauvegardées pour le contrôle AM-PM
			LoadListContent( GetItem(IDC_LSTAMPM) );
			ConfigSpin( hwndCtl);
		}
		break;

	case EN_SETFOCUS:   // Un ctrl EDIT prend le focus
		// Sélectionne le texte que contient le ctrl edit
		SendMessage (hwndCtl, EM_SETSEL, 0, -1);

		// Configure le spin controle pour agir sur ce control
		if (hwndCtl == GetItem(IDC_EDTM0) ||
			hwndCtl == GetItem(IDC_EDTM1) ||
			hwndCtl == GetItem(IDC_EDTM2))
		{
			ConfigSpin(hwndCtl);
		}
		break;

	case EN_KILLFOCUS: // Un ctrl EDIT perd le focus
		// Vérifie la conformité du contenu ctrl
		CheckEditContent( hwndCtl, FALSE );
		// Rempli avec des zéros si nécessaire
		FillTimeField(hwndCtl);
		break;

	case CBN_SELCHANGE:      
		if (GetItem(IDC_CMBMONTH) == hwndCtl) // Combo du mois
		{
			SYSTEMTIME ST_new;

			// Met à jour le calendrier
			ST_new.wMonth = (WORD)SendMessage(GetItem(IDC_CMBMONTH), CB_GETCURSEL, 0, 0) + 1;
			SendMessage(GetItem(IDC_CALENDAR), CC_SETDATE, SD_MONTH | SD_NOTIFY, (LPARAM)&ST_new);				
		}
		else if (wID==IDC_CMBTIMEZONE) 
			m_pcs->OnModification();
		break;
			
	case EN_UPDATE:
		// Vérifie la conformité du contenu ctrl
		CheckEditContent( hwndCtl, TRUE );
		// -> case CCN_DATECHANGE

	case CCN_DATECHANGE: // changement du contrôle calendrier
		m_pcs->OnModification();
		break;

	case BN_CLICKED:					
		// A-t-on cliqué sur le bouton Avancé (§) ?
		switch(wID)				
		{					
		case IDC_CHKCST:
		case IDC_CHKSYS:
			break;
		case IDC_CHKTIME:
		case IDC_CHKDATE:
		case IDC_CHKTZ:
			MajEnableControl();
			break;

		case IDC_BTLOADPRESETDT:
			{
				HMENU hPopup, hPopup2;
				RECT rc;
				GetWindowRect(hwndCtl, &rc);

				hPopup = LoadMenu(g_hResDll, MAKEINTRESOURCE(IDM_LOADPRESET));
				hPopup2 = GetSubMenu(hPopup, 0);
				int iRet = TrackPopupMenu(hPopup2, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON |  TPM_RIGHTBUTTON , rc.left, rc.bottom, 0, hwndCtl, NULL);
				switch (iRet) {
					case ID_FILEMODIFICATION:
						SetDateTime(&m_pcs->m_stModification);
						break;
					case ID_FILECREATIONDATE:
						SetDateTime(&m_pcs->m_stCreation);
						break;
					case ID_CURSYS:
						{
							SYSTEMTIME st;
							GetLocalTime(&st);
							SetDateTime(&st);
						}
						break;
					default:
						break;
				}
				DestroyMenu( hPopup2 );
				DestroyMenu( hPopup );
			}
			break;

		default:
			return FALSE;
		}

		// Active le bouton 'apply'
		m_pcs->OnModification();
		break;

	default:
		return FALSE;
	}		
	SetWindowLong(m_hwnd, DWLP_MSGRESULT, 0);
	return TRUE;
}

// Procédure de la fenêtre option
INT_PTR COptions::SheetProc(UINT uMessage, WPARAM wParam, LPARAM lParam)
{

	switch (uMessage)
	{
	case WM_SYSCOLORCHANGE:
		// Configuration de l'affichage des couleurs
		InitColor();
		MAJTCColor();
		return TRUE;
		
	case WM_SETTINGCHANGE:
		// Configuration de l'affichage international
		MAJInternational();
		SetWindowLong(m_hwnd, DWLP_MSGRESULT, 0);
		return TRUE;

	case WM_CTLCOLORLISTBOX:
		if( ((HWND)lParam == GetItem(IDC_LSTAMPM)) && (!IsWindowEnabled((HWND)lParam)) )
		{				
			SetBkColor((HDC) wParam, GetSysColor(COLOR_3DFACE));
			return (LRESULT)g_hBrSYSCOLOR_3DFACE;
		}
		break;

	case WM_CTLCOLORSTATIC:
		if( (HWND)lParam == GetItem(IDC_SEPTIME0) ||
			(HWND)lParam == GetItem(IDC_SEPTIME1) )
		{
			if ( IsWindowEnabled((HWND)lParam) )
			{
				SetBkColor((HDC) wParam, GetSysColor(COLOR_WINDOW));
				SetTextColor((HDC) wParam, GetSysColor(COLOR_WINDOWTEXT));
				return (LRESULT)g_hBrSYSCOLOR_WINDOW;
			}
		}
		break;

	case WM_VSCROLL:
		if ((HWND) lParam == GetItem(IDC_SPINAMPM))
		{
			SendMessage(m_hwnd,
						WM_COMMAND,
						MAKELONG(0, EN_UPDATE),
						SendMessage((HWND)lParam, UDM_GETBUDDY, 0, 0)
						);
			SetWindowLong(m_hwnd, DWLP_MSGRESULT, 0);
			return TRUE;
		}
		break;
	}

	return FALSE;
}