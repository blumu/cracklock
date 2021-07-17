// testunicode.cpp
//

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from 
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <commctrl.h>
#include <commdlg.h>

#include "resource.h"
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szchaine[MAX_LOADSTRING];


LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	INITCOMMONCONTROLSEX ic;
	ic.dwICC = ICC_TREEVIEW_CLASSES | ICC_TAB_CLASSES ;
	ic.dwSize = sizeof(ic);
	InitCommonControlsEx(&ic);
	DialogBox(hInstance, (LPCTSTR)IDD_ABOUTBOX, NULL, (DLGPROC)About);
	return TRUE;
}

LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		{		
		LoadString(hInst, IDS_KOREAN, szchaine, MAX_LOADSTRING);

		HWND hwndlv = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, _T
			(""), 
				WS_CHILD | WS_VISIBLE | LVS_AUTOARRANGE | LVS_REPORT | LVS_SINGLESEL | LVS_AUTOARRANGE | LVS_ALIGNTOP,
				0, 0, 100, 100, 
				hDlg, NULL, hInst, NULL); 


		// ajoute une colone aux deux listes
		LV_COLUMN	lvc;
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
		lvc.fmt = LVCFMT_LEFT; 
		lvc.cx = 120;
		lvc.iSubItem = 0;
		lvc.pszText = szchaine;
		ListView_InsertColumn(hwndlv, 0, &lvc);
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST1), 0, &lvc);

	  // ajoute un tab
		HWND hwndtab;
		//hwndtab = GetDlgItem(hDlg, IDC_TAB1);
		hwndtab = CreateWindowEx(WS_EX_CLIENTEDGE, WC_TABCONTROL, _T(""), 
				WS_CHILD | WS_VISIBLE ,
				200, 0, 200, 100, 
				hDlg, NULL, hInst, NULL); 
		

		TCITEM tie;
		BOOL bret = TabCtrl_SetUnicodeFormat(hwndtab,TRUE);
		tie.mask = TCIF_TEXT;
		tie.iImage = -1; 		
 		tie.pszText = szchaine;
		tie.lParam = (LPARAM)NULL;
		TabCtrl_InsertItem(GetDlgItem(hDlg, IDC_TAB1), 1, &tie);
		TabCtrl_InsertItem(hwndtab, 1, &tie);
 		tie.pszText = _T("wiwee");
		TabCtrl_InsertItem(hwndtab, 1, &tie);
		HFONT hf;
		hf = (HFONT)GetStockObject(DEVICE_DEFAULT_FONT);


 
/*	
		LOGFONT lf; 
	lf.lfHeight = -13;
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = 400;
	lf.lfItalic = 0;
	lf.lfUnderline = 0;
	lf.lfStrikeOut = 0;
	lf.lfCharSet = 0;
	lf.lfOutPrecision = 3;
	lf.lfClipPrecision = 2;
	lf.lfQuality = 1;
	lf.lfPitchAndFamily = 34;
	lf.lfFaceName = TEXT("@Arial Unicode MS");
 */
		LOGFONT lf; 
		CHOOSEFONT cf; 
    cf.lStructSize = sizeof(CHOOSEFONT); 
    cf.hwndOwner = (HWND)NULL; 
    cf.hDC = (HDC)NULL; 
    cf.lpLogFont = &lf; 
    cf.iPointSize = 0; 
    cf.Flags = CF_SCREENFONTS; 
    cf.rgbColors = RGB(0,0,0); 
    cf.lCustData = 0L; 
    cf.lpfnHook = (LPCFHOOKPROC)NULL; 
    cf.lpTemplateName = (LPTSTR)NULL; 
    cf.hInstance = (HINSTANCE) NULL; 
    cf.lpszStyle = (LPTSTR)NULL; 
    cf.nFontType = SCREEN_FONTTYPE; 
    cf.nSizeMin = 0; 
    cf.nSizeMax = 0; 
    ChooseFont(&cf); 
    hf = CreateFontIndirect(cf.lpLogFont); 
		
 
		//hf = CreateFont(  -13,  0,  0, 0, 400, 0,0,0,0,3,2,1,34,_T("Arial"));
		SendMessage(hwndtab, WM_SETFONT, (WPARAM)hf, TRUE);
		return TRUE;
		}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}
