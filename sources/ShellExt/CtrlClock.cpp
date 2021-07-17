//  MODULE:		CtrlClock.CPP
//
//  PURPOSE:	Implemente le contrôle CLOCK
//
//  by William BLUM, September 03 1997

#include "StdAfx.h"			// Header précompilé
#include <math.h>			// Fonctions mathématiques du C

#define TC_CLOCK_SRC
#include "TimeCtrl.h"		// Contrôles temps
#include "CtrlClock.h"		// Contrôle horloge

#define TWOPI       (2 * 3.14159)

LRESULT CALLBACK CClockWndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static POINT HandsPoints[3][5] = {	0, -7,     5, 0,   0, 30,    -5, 0,   0,  -7,
										0, -10,    3, 0,   0, 40,    -3, 0,   0, -10,
										0,   0,    0, 0,   0,  0,     0, 0,   0,  40 } ;
	PCCLOCKINFO  pCI ;
	switch (iMsg)
	{
		case WM_CREATE:
			// Alloue une structure pour stocker les infos sur ce contrôle
			pCI = (PCCLOCKINFO) malloc (sizeof(CCLOCKINFO));
			SET_CTRL_INFO (pCI);
			return 0 ;

		case WM_SIZE:
		{
			GET_CTRL_INFO(pCI, PCCLOCKINFO) ;
			int i,j;

			pCI->cxClient = LOWORD (lParam);
			pCI->cyClient = HIWORD (lParam);
			pCI->ptCenter.x = pCI->cxClient / 2;
			pCI->ptCenter.y = pCI->cyClient / 2;
			for(j=0 ; j<3; j++)
			{
				for(i=0 ; i<5; i++)  
				{
					pCI->ptHands[j][i].x = pCI->cxClient * HandsPoints[j][i].x/100;
					pCI->ptHands[j][i].y = pCI->cyClient * HandsPoints[j][i].y/100;
				}
			}
			return 0;
		}

		case WM_ENABLE:
			InvalidateRect(hwnd, NULL, TRUE);
			return 0;

		case WM_PAINT:
		{
			GET_CTRL_INFO(pCI, PCCLOCKINFO) ;
			HDC			hdc;
			PAINTSTRUCT	ps;
			hdc = BeginPaint (hwnd, &ps) ;
			DrawClock (pCI, hdc, hwnd, IsWindowEnabled(hwnd)) ;
			DrawHands (pCI, hdc, TRUE, IsWindowEnabled(hwnd)) ;
			EndPaint (hwnd, &ps) ;    
			return 0;
		}

		case CC_GETTIME:
		{
			GET_CTRL_INFO(pCI, PCCLOCKINFO) ;
			PSYSTEMTIME pST_get = (PSYSTEMTIME) lParam;

			pST_get->wHour = pCI->wHour;
			pST_get->wMinute = pCI->wMin;
			pST_get->wSecond = pCI->wSec;

			return 0;
		}

		case CC_SETTIME:
		{
			GET_CTRL_INFO(pCI, PCCLOCKINFO);
			PSYSTEMTIME pST_new = (PSYSTEMTIME) lParam;

			HDC hdc = GetDC (hwnd);
			DrawHands (pCI, hdc, FALSE, IsWindowEnabled(hwnd));
  
			if( wParam & ST_HOUR )
				pCI->wHour = pST_new->wHour;
			if( wParam & ST_MIN )
				pCI->wMin = pST_new->wMinute;
			if( wParam & ST_SEC )
				pCI->wSec = pST_new->wSecond;

			DrawHands (pCI, hdc, TRUE, IsWindowEnabled(hwnd));      
			ReleaseDC (hwnd, hdc);
			return 0;
		}

		case WM_DESTROY:
			GET_CTRL_INFO(pCI, PCCLOCKINFO) ;

			free (pCI);
			return 0;
	}
	return DefWindowProc (hwnd, iMsg, wParam, lParam);
}

void RotatePoint (PPOINT ptDst, PPOINT pptCenter, PPOINT ptSrc, int iNum, int iAngle)
{
	for (int i = 0 ; i < iNum ; i++)
	{
		
		ptDst[i].x = (int) (ptSrc[i].x * cos (TWOPI * iAngle / 360) +
						ptSrc[i].y * sin (TWOPI * iAngle / 360)) + pptCenter->x;
		ptDst[i].y = pptCenter->y - (int) (ptSrc[i].y * cos (TWOPI * iAngle / 360) -
						ptSrc[i].x * sin (TWOPI * iAngle / 360)) ;
						
	}
}

void DrawClock (PCCLOCKINFO pCI, HDC hdc, HWND hwnd, BOOL bEnabled)
{
	int     iAngle ;
	POINT   pt[3] ;
	POINT   pt0;
	HPEN    hPenOld;
	HBRUSH  hBROld;


	hPenOld = (HPEN)SelectObject(hdc, g_hPENSC3DFC);
	hBROld = (HBRUSH)SelectObject(hdc, g_hBRSC3DFC);
	SendMessage(GetParent(hwnd), WM_CTLCOLORCLOCK, (WPARAM)hdc, (LPARAM)hwnd);
	Rectangle(hdc, 0, 0, pCI->cxClient, pCI->cyClient);
	SelectObject(hdc, hBROld);
	SelectObject(hdc, hPenOld);

	hBROld = (HBRUSH)SelectObject(hdc,  bEnabled ? g_hBRSCBKDESKTOP : g_hBRSCINACTIVECAPTION);

	for (iAngle = 0 ; iAngle < 360 ; iAngle += 6)
	{    
		pt0.x =   0 ;
		pt0.y = (long)(pCI->ptCenter.y*0.9);

		RotatePoint (pt, &pCI->ptCenter, &pt0, 1, iAngle) ;

		pt[2].x = pt[2].y = iAngle % 5 ? 2 : 5 ;
		pt[0].x -= pt[2].x / 2 ;
		pt[0].y -= pt[2].y / 2 ;

		pt[1].x  = pt[0].x + pt[2].x ;
		pt[1].y  = pt[0].y + pt[2].y ;

		Rectangle(hdc, pt[0].x, pt[0].y, pt[1].x, pt[1].y);
		hPenOld = (HPEN)SelectObject (hdc, g_hPENSC3DHL) ;   
		MoveToEx(hdc, pt[0].x, pt[1].y-1, NULL);
		LineTo(hdc, pt[0].x, pt[0].y);
		LineTo(hdc, pt[1].x-1, pt[0].y);


		SelectObject (hdc, g_hPENSC3DDK) ;
		LineTo(hdc, pt[1].x-1, pt[1].y-1);
		LineTo(hdc, pt[0].x, pt[1].y-1);

		SelectObject (hdc, hPenOld) ;
	}

	SelectObject(hdc, hBROld);
}

void DrawHands (PCCLOCKINFO pCI, HDC hdc, BOOL bDraw, BOOL bEnabled)
{
	int     iAngle[3] ;
	HBRUSH  hBrOld ;
	HPEN    hPenOld, hPenOld2 ;
	int     i ;
	POINT   ptTemp[5] ;

	iAngle[0] = (pCI->wHour * 30) % 360 + pCI->wMin / 2 ;
	iAngle[1] =  pCI->wMin  *  6 ;
	iAngle[2] =  pCI->wSec  *  6 ;

	hBrOld = (HBRUSH)SelectObject(hdc, bDraw ? (bEnabled ? g_hBRSCBKDESKTOP : g_hBRSCINACTIVECAPTION) : g_hBRSC3DFC );
	hPenOld = (HPEN)SelectObject(hdc, bDraw ? (bEnabled ? g_hPENSC3DDK : g_hPENSC3DFC) : g_hPENSC3DFC );

	for (i = 0; i < 3 ; i++)
	{
		RotatePoint (ptTemp, &pCI->ptCenter, (PPOINT)pCI->ptHands[i], 5, iAngle[i]) ;
		Polygon (hdc, ptTemp, 5) ;
		
		hPenOld2 = (HPEN)SelectObject(hdc,  bDraw ? (bEnabled ? g_hPENSC3DHL : g_hPENSC3DDK) : g_hPENSC3DFC );
		Polyline (hdc, &ptTemp[2], 3) ;
		SelectObject(hdc, hPenOld2);
	}

	SelectObject(hdc, hPenOld);
	SelectObject(hdc, hBrOld);
}