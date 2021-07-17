//  MODULE:   TimeCtrl.CPP
//
//  PURPOSE:  Module commun pour TC_CALDR.cpp et TC_CLOCK.cpp
//
//  by William BLUM, September 03 1997

#include "StdAfx.h"			// Header précompilé
#include "TimeCtrl.h"		// Contrôles temps

// Objet GDI globeaux
HBRUSH	g_hBRSC3DDK = NULL,
		g_hBRSC3DFC = NULL,
		g_hBRSC3DHL = NULL,
		g_hBRSCACTIVECAPTION = NULL,
		g_hBRSCINACTIVECAPTION = NULL,
		g_hBRSCWINDOW = NULL,
		g_hBRSCBKDESKTOP = NULL;

HPEN	g_hPENSC3DHL = NULL,
		g_hPENSC3DFC = NULL,
		g_hPENSC3DDK = NULL;

/////////////////////////////
// Fonctions d'enregistrement des contrôles

BOOL RegTimeCtrl (HINSTANCE hmodDll)
{
	WNDCLASSEX	wc;

	wc.cbSize			= sizeof (wc) ;
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_OWNDC | CS_GLOBALCLASS;
	wc.cbClsExtra		= 0 ;
	wc.cbWndExtra		= PTR_EXTRA ;
	wc.hInstance		= hmodDll;
	wc.hIcon			= NULL ;
	wc.hCursor			= LoadCursor (NULL, IDC_ARROW) ;  
	wc.lpszMenuName		= NULL ;
	wc.hIconSm			= NULL ;

	wc.hbrBackground	= (HBRUSH) GetStockObject (WHITE_BRUSH) ;
	wc.lpszClassName	= CALENDARCLASS ;
	wc.lpfnWndProc	= CCalendarWndProc ;
	if (  RegisterClassEx(&wc) == NULL )
		return FALSE;

	wc.hbrBackground	= (HBRUSH) GetStockObject (LTGRAY_BRUSH) ;
	wc.lpszClassName	= CLOCKCLASS;
	wc.lpfnWndProc		= CClockWndProc ;
	if (  RegisterClassEx(&wc) == NULL )
	{      
		UnregisterClass(CALENDARCLASS, hmodDll);
		return FALSE;
	}

	wc.style			= CS_HREDRAW | CS_VREDRAW |CS_GLOBALCLASS;
	wc.hbrBackground	= (HBRUSH) GetStockObject (LTGRAY_BRUSH) ;
	wc.lpszClassName	= BACKGROUNDCLASS;
	wc.lpfnWndProc		= CBackgroundWndProc ;
	if (  RegisterClassEx(&wc) == NULL )
	{      
		UnregisterClass(CALENDARCLASS, hmodDll);
		UnregisterClass(CLOCKCLASS, hmodDll);
		return FALSE;
	}

	// Configuration de l'affichage des couleurs
	MAJTCColor();

	return TRUE;
}


/////////////////////////////
// Fonctions de désenregistrement des contrôles

void UnregTimeCtrl (HINSTANCE hmodDll)
{
	UnregisterClass((PTSTR) CALENDARCLASS, hmodDll);
	UnregisterClass((PTSTR) CLOCKCLASS, hmodDll);

	DeleteGDIObjects();
}


/////////////////////////////
// Libération des objets GDI

void DeleteGDIObjects(void)
{
	DeleteObject(g_hBRSC3DDK);
	DeleteObject(g_hBRSC3DFC);
	DeleteObject(g_hBRSC3DHL);
	DeleteObject(g_hBRSCACTIVECAPTION);
	DeleteObject(g_hBRSCINACTIVECAPTION);
	DeleteObject(g_hBRSCWINDOW);
	DeleteObject(g_hBRSCBKDESKTOP);

	DeleteObject(g_hPENSC3DHL);
	DeleteObject(g_hPENSC3DFC);
	DeleteObject(g_hPENSC3DDK);
}


/////////////////////////////
// Mise à jour des pinceaux GDI avec les couleurs systèmes

void MAJTCColor (void)
{
	LOGBRUSH lb;

	lb.lbStyle = BS_SOLID;
	lb.lbHatch = NULL;

	DeleteGDIObjects();

	lb.lbColor = GetSysColor(COLOR_3DDKSHADOW);
	g_hBRSC3DDK = CreateBrushIndirect(&lb);

	lb.lbColor = GetSysColor(COLOR_BTNFACE);
	g_hBRSC3DFC = CreateBrushIndirect(&lb);

	lb.lbColor = GetSysColor(COLOR_3DHILIGHT);
	g_hBRSC3DHL = CreateBrushIndirect(&lb);

	lb.lbColor = GetSysColor(COLOR_ACTIVECAPTION);
	g_hBRSCACTIVECAPTION = CreateBrushIndirect(&lb);

	lb.lbColor = GetSysColor(COLOR_INACTIVECAPTION);
	g_hBRSCINACTIVECAPTION = CreateBrushIndirect(&lb);

	lb.lbColor = GetSysColor(COLOR_BACKGROUND);
	g_hBRSCBKDESKTOP = CreateBrushIndirect (&lb);

	lb.lbColor = GetSysColor(COLOR_WINDOW);
	g_hBRSCWINDOW = CreateBrushIndirect (&lb);

	g_hPENSC3DHL = CreatePen(PS_SOLID, 1,  GetSysColor(COLOR_3DHILIGHT));
	g_hPENSC3DDK = CreatePen(PS_SOLID, 1,  GetSysColor(COLOR_3DDKSHADOW));
	g_hPENSC3DFC = CreatePen(PS_SOLID, 1,  GetSysColor(COLOR_BTNFACE));
}



LRESULT CALLBACK CBackgroundWndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND hwndHH = NULL;
	static HWND hwndMM = NULL;
	static HWND hwndSS = NULL;
	static HWND hwndAMPM = NULL;

	switch (iMsg)
	{
		case WM_CREATE:
			hwndHH = CreateWindow(WC_EDIT, _T("HH"),0,0,0,10,10,hwnd,NULL,NULL,NULL);
			hwndMM = CreateWindow(WC_EDIT, _T("MM"),0,30,0,10,10,hwnd,NULL,NULL,NULL);
			hwndSS = CreateWindow(WC_EDIT, _T("SS"),0,60,0,10,10,hwnd,NULL,NULL,NULL);
			hwndAMPM = CreateWindow(WC_LISTBOX, _T("AMPM"),0,60,0,10,10,hwnd,NULL,NULL,NULL);
			return 0 ;
//		case WM_SIZE:
	//		return 0;
		case WM_ENABLE:
			EnableWindow(hwndHH, TRUE);
			EnableWindow(hwndMM, TRUE);
			EnableWindow(hwndSS, TRUE);
			EnableWindow(hwndAMPM, TRUE);
			InvalidateRect(hwnd, NULL, TRUE);
			return 0;

		case WM_PAINT:
		{
			HDC			hdc;
			PAINTSTRUCT	ps;
			RECT	  rc;
			GetClientRect(hwnd, &rc);

			hdc = BeginPaint (hwnd, &ps) ;
					HBRUSH hbrold = SelectBrush(hdc, GetSysColorBrush(IsWindowEnabled(hwnd) ? COLOR_WINDOW : COLOR_BTNFACE ));
						Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom );

					SelectBrush(hdc, hbrold);
			EndPaint (hwnd, &ps) ;    
			return 0;
		}

//		case WM_DESTROY:
	//		return 0;
	}
	return DefWindowProc (hwnd, iMsg, wParam, lParam);
}
