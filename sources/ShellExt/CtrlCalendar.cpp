//	MODULE:		CtrlCalendar.cpp
//
//	PURPOSE:	Implemente le contrôle CALENDAR
//
//	by William BLUM, September 03 1997

#include "StdAfx.h"			// Header précompilé

#define TC_CALENDAR_SRC
#include "TimeCtrl.h"		// Contrôles temps
#include "CtrlCalendar.h"		// Contrôle calendrier

TCHAR	g_cDay[NB_DAYSINWEEK];	// Lettre correspondant au 7 jours de la semaine
int	 g_iFirstDayOfWeek;				// Numéro du premier jour dans la semaine

/////////////////////////////
// Contrôle CCalendar
LRESULT CALLBACK CCalendarWndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	PCCALENDARINFO  pCI;
	switch (iMsg)
	{
		case WM_CREATE:
			// Alloue une structure pour stocker les infos sur ce contrôle
			pCI = (PCCALENDARINFO) malloc (sizeof(CCALENDARINFO));
			SET_CTRL_INFO (pCI);

			pCI->wMonth = pCI->wDay = 1;
			pCI->wYear = MIN_YEAR;
			pCI->iFirstDay = CalcFirstDay(pCI->wMonth, pCI->wYear, g_iFirstDayOfWeek);
			GetWININI();
			return 0 ;

		case WM_SETTINGCHANGE:
			GetWININI();
			return 0;

		case WM_SIZE :
			GET_CTRL_INFO(pCI, PCCALENDARINFO) ;

			pCI->cxClient = LOWORD (lParam);
			pCI->cyClient = HIWORD (lParam);

			pCI->cxBox = pCI->cxClient / NB_DAYSINWEEK;
			pCI->cyBox = pCI->cyClient / NB_DAYSINWEEK;
			return 0 ;

		case WM_SETFOCUS:
		case WM_KILLFOCUS:
		{
			GET_CTRL_INFO(pCI, PCCALENDARINFO) ;
    
			HDC hdc = GetDC(hwnd);
			HFONT hFNTOld = (HFONT) SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
				DrawSelectedDay(pCI, hdc, IsWindowEnabled(hwnd), (iMsg == WM_SETFOCUS));
			SelectObject(hdc, hFNTOld);    
			ReleaseDC(hwnd, hdc);
			return 0;
		}

		case WM_GETDLGCODE:
			return DLGC_WANTARROWS;
		case WM_KEYDOWN:
		{
			GET_CTRL_INFO(pCI, PCCALENDARINFO) ;
			SYSTEMTIME ST_New;
			
			ST_New.wDay = 666;
			switch (wParam)
			{
				case VK_LEFT:
					ST_New.wDay = pCI->wDay - 1;      
					break; 
				case VK_RIGHT:
					ST_New.wDay = pCI->wDay + 1;
					break; 
				case VK_UP:
					ST_New.wDay = pCI->wDay - NB_DAYSINWEEK;
					break; 
				case VK_DOWN: 
					ST_New.wDay = pCI->wDay + NB_DAYSINWEEK;
					break;			
			}
			
			if( ST_New.wDay != 666 )
			{
				// Change le jour sélectionné pour ce ctrl
				SendMessage(hwnd, CC_SETDATE, SD_DAY | SD_NOTIFY, (LPARAM)&ST_New);
				return 0;
			}
			break;
		}

		case WM_LBUTTONDOWN:
		{
			GET_CTRL_INFO(pCI, PCCALENDARINFO) ;
			SYSTEMTIME ST_New;
    
			ST_New.wDay = (int)(HIWORD(lParam)/pCI->cyBox-1) * NB_DAYSINWEEK +
							(int)(LOWORD(lParam)/pCI->cxBox) - pCI->iFirstDay + 1;

			// Positionne le focus sur ce ctrl
			SetFocus(hwnd);

			// Change le jour sélectionné pour ce ctrl
			SendMessage(hwnd, CC_SETDATE, SD_DAY | SD_NOTIFY, (LPARAM)&ST_New);
			return 0;
		}
		
		case WM_ENABLE:
			InvalidateRect(hwnd, NULL, TRUE);
			return 0;
		
		case WM_PAINT:
			GET_CTRL_INFO(pCI, PCCALENDARINFO) ;

			PAINTSTRUCT	ps;
			HDC			hdc;
			HFONT		hFNTOld;

			hdc = BeginPaint (hwnd, &ps) ;
			hFNTOld = (HFONT) SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));

			// Dessine les titres des jours
			DrawDaysTitle(pCI, hdc, IsWindowEnabled(hwnd));
			// Dessine les jours
			DrawDaysBox(pCI, hdc);
			// Dessine le jour sélectionné
			DrawSelectedDay(pCI, hdc, IsWindowEnabled(hwnd), (GetFocus()==hwnd) );

			SelectObject(hdc, hFNTOld);    
			EndPaint (hwnd, &ps) ;
			return 0;

		case CC_GETDATE:
		{
			GET_CTRL_INFO(pCI, PCCALENDARINFO);
			PSYSTEMTIME pST_get = (PSYSTEMTIME) lParam;
			pST_get->wYear = pCI->wYear;
			pST_get->wMonth = pCI->wMonth;
			pST_get->wDay = pCI->wDay;
			return 0;
		}

		case CC_SETDATE:
		{
			GET_CTRL_INFO(pCI, PCCALENDARINFO) ;
			PSYSTEMTIME pST_new = (PSYSTEMTIME) lParam;

			HDC hdc;
			HFONT hFNTOld;
			RECT rcErase;

			hdc = GetDC(hwnd);
			hFNTOld = (HFONT)SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));

			if ( wParam == SD_DAY)
				// Efface la sélection sur le jour courrant
				GetDayRect(pCI, pCI->wDay, &rcErase);
			else
			{
				// Efface le calendrier entier
				rcErase.left = 0;
				rcErase.top = pCI->cyBox;
				rcErase.right = pCI->cxClient;
				rcErase.bottom = pCI->cyClient;
			}      

			// Test la validité de l'année
			if( wParam & SD_YEAR )
			{
				if( (pST_new->wYear < MIN_YEAR) || (pST_new->wYear > MAX_YEAR) )
					return FALSE;
				else
					pCI->wYear = pST_new->wYear;
			}

			// Test la validité du mois
			if( wParam & SD_MONTH )
			{
				if( (pST_new->wMonth < 1) || (pST_new->wMonth > NB_MONTHSINYEAR) )
					return FALSE;
				else
					pCI->wMonth = pST_new->wMonth;
			}

			// Test la validité du jour
			if( wParam & SD_DAY )
			{
				if( (pST_new->wDay < 1) || (pST_new->wDay > DaysInMonth(pCI->wMonth, pCI->wYear)) )
					return FALSE;
				else
					pCI->wDay = pST_new->wDay;
			}
			pCI->wDay = min(pCI->wDay, DaysInMonth(pCI->wMonth, pCI->wYear));

			// Efface la zone à redessiner
			FillRect(hdc, &rcErase, IsWindowEnabled(hwnd) ? g_hBRSCWINDOW : g_hBRSC3DFC);

			// Si le mois ou l'année change ...
			if ( wParam != SD_DAY)
				// ... calcule la position du premier jour
				pCI->iFirstDay = CalcFirstDay(pCI->wMonth, pCI->wYear, g_iFirstDayOfWeek);

			// Redessine les jours
			DrawDaysBox(pCI, hdc);

			// Dessine le jour sélectionné
			DrawSelectedDay(pCI, hdc, IsWindowEnabled(hwnd), (GetFocus()==hwnd));

			SelectObject(hdc, hFNTOld);    
			ReleaseDC(hwnd, hdc);
		
			// Notifie la fenêtre parent d'un changement de date si cela est demandé
			if( wParam & SD_NOTIFY )
				SendMessage(GetParent(hwnd), WM_COMMAND, (WPARAM)MAKEWPARAM(GetDlgCtrlID(hwnd), CCN_DATECHANGE), (LPARAM)hwnd);

			return 0;
		}

		case WM_DESTROY:
		{
			GET_CTRL_INFO(pCI, PCCALENDARINFO) ;

			free (pCI);
			return 0;
		}
	}
	return DefWindowProc (hwnd, iMsg, wParam, lParam);
}

// Obtient les infos stockées dans la registry pour l'affichage du calendrier 
void GetWININI( void )
{
	LCID	lcidUser = GetUserDefaultLCID();
	int		i;
	
	TCHAR weekday[256];

	GetLocaleInfo(lcidUser, LOCALE_IFIRSTDAYOFWEEK, weekday, sizeof(weekday));
	g_iFirstDayOfWeek = weekday[0] - '0';

	for(i=0; i<NB_DAYSINWEEK; i++)
	{
		if( GetLocaleInfo(lcidUser, LOCALE_SDAYNAME1+i, weekday, sizeof(weekday)) )
		  g_cDay[i] = _totupper(weekday[0]);
		else  {
			DWORD dw = GetLastError();
			g_cDay[i] = '*';
		}
	}
}

// Dessine les titres des jours
void DrawDaysTitle(PCCALENDARINFO pCI, HDC hdc, BOOL bEnabled)
{
	RECT	rc;
	int		i;

	// Dessine le cadre titres pour les jours
	rc.top = rc.left = 0;
	rc.right = pCI->cxClient;   
	rc.bottom = pCI->cyBox;
	FillRect(hdc, &rc, g_hBRSCINACTIVECAPTION);
	rc.top = pCI->cyBox;
	rc.bottom = pCI->cyClient;
	FillRect(hdc, &rc, bEnabled ? g_hBRSCWINDOW : g_hBRSC3DFC);

	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc,
		 GetSysColor(bEnabled ? COLOR_CAPTIONTEXT :COLOR_INACTIVECAPTIONTEXT) );
	
	rc.top = 0;
	rc.bottom = pCI->cyBox;
	for(i=0 ; i< NB_DAYSINWEEK ; i++)
	{      
		rc.left = i*pCI->cxBox;
		rc.right = rc.left + pCI->cxBox;

		DrawText(hdc, &g_cDay[ (i+g_iFirstDayOfWeek) % NB_DAYSINWEEK ], 1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}
}

// Dessine les jours
void DrawDaysBox(PCCALENDARINFO pCI, HDC hdc)
{
	TCHAR	szTMP[3];
	WORD	wNbDays;
	RECT	rcDay;
	int		i;

	// Calcule le nombre de jour dans un mois donné
	wNbDays = DaysInMonth(pCI->wMonth, pCI->wYear);

	// Dessine les jour du mois
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
	//SetTextColor(hdc, RGB(0,0,0));
	for(i=0; i<wNbDays; i++) {
		_itow_s(i+1, szTMP, 10);
		DrawText( hdc, szTMP,
					-1,
					GetDayRect(pCI, i+1, &rcDay),
					DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}
}
  
// Dessine le jour sélectionné
void DrawSelectedDay(PCCALENDARINFO pCI, HDC hdc, BOOL bEnabled, BOOL bFocus)
{  
	COLORREF	OldTxtColo;
	RECT		rcDay;
	TCHAR		szTMP[3];

	SetBkMode(hdc, TRANSPARENT);
	OldTxtColo = SetTextColor(hdc, GetSysColor(COLOR_CAPTIONTEXT));

	// Obtient la zone du jour sélectionné
	GetDayRect(pCI, pCI->wDay, &rcDay);

	// Dessine le fond de sélection
	FillRect(hdc, &rcDay, bEnabled ? g_hBRSCACTIVECAPTION : g_hBRSCINACTIVECAPTION);

	// Dessine le texte
	_itow_s(pCI->wDay, szTMP, 10);
	DrawText(hdc, szTMP, -1, &rcDay, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	SetTextColor(hdc, OldTxtColo);

	// Dessine le rectangle du focus
	if (bFocus)
		DrawFocusRect(hdc, &rcDay);
}

// Obtient la zone d'un certain jour
LPRECT GetDayRect(PCCALENDARINFO pCI, WORD wDay, LPRECT prc)
{
	wDay += pCI->iFirstDay - 1;
	prc->top = (wDay / NB_DAYSINWEEK+1)*pCI->cyBox;
	prc->left = (wDay % NB_DAYSINWEEK)*pCI->cxBox;
	prc->bottom = prc->top + pCI->cyBox;
	prc->right = prc->left + pCI->cxBox;
	return prc;
}
