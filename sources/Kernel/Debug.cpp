//==================================
// William BLUM 1998
// DEBUG.CPP
//==================================
#ifdef _DEBUG

#include <windows.h>
#include <vector>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"
#include "Hook.h"
#include "Debug.h"


using namespace std ;

HWND	g_hDlg = NULL;
BOOL	g_bDlgCreation = FALSE;

extern HMODULE	g_hModule;	// Module courrant

extern vector<HINSTANCE> lstModule;	// handles des modules chargé par le process courrant

void Debug_Clear( int IDLST )
{
	SendMessage(GetDlgItem(g_hDlg, IDLST), LB_RESETCONTENT, 0, 0);
}

void Debug_EnumLoadedModule (void)
{
	TCHAR szMod[_MAX_FNAME];
	vector<HINSTANCE>::iterator i, start, end;

	Debug_Clear(IDC_LST2);

	start = lstModule.begin();
	end = lstModule.end();

	for(i = start; i != end; i++)
	{
		if( GetModuleFileName(*i, szMod, _MAX_FNAME) )
			Debug_Print(IDC_LST2, _T("(%08X) %s"), *i, szMod);
	}
}

void Debug_Init()
{
	if( g_bDlgCreation )
		return;
	if( !g_hDlg ) {
		g_bDlgCreation = TRUE;
		g_hDlg = CreateDialog (g_hModule, MAKEINTRESOURCE(IDD_DEBUG), (HWND)NULL, (DLGPROC)DebugDlgProc);
		g_bDlgCreation = FALSE;
	}
}

void __cdecl Debug_PrintA(int IDLST, const char *format, ... )
{
    Debug_Init();

    va_list arglist;
    char szDebug[1024];

    va_start(arglist, format);
    StringCbVPrintfA(szDebug, _countof(szDebug), format, arglist);
    SendMessageA(GetDlgItem(g_hDlg, IDLST), LB_ADDSTRING, 0, (LPARAM)szDebug);
}

void __cdecl Debug_PrintW(int IDLST, const WCHAR *format, ... )
{
    Debug_Init();

    va_list arglist;
    WCHAR szDebug[1024];

    va_start(arglist, format);
    StringCbVPrintfW(szDebug, _countof(szDebug), format, arglist);
    SendMessageW(GetDlgItem(g_hDlg, IDLST), LB_ADDSTRING, 0, (LPARAM)szDebug);
}



BOOL CALLBACK DebugDlgProc(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:

			g_hDlg = hwndDlg;

			ShowWindow(g_hDlg, SW_SHOW);

			// Obtient les infos systèmes
			SYSTEM_INFO	si;
			GetSystemInfo(&si);

			Debug_Print(IDC_LST, _T("si.lpMinimumApplicationAddress = %08X"), si.lpMinimumApplicationAddress);
			Debug_Print(IDC_LST, _T("si.lpMaximumApplicationAddress = %08X"), si.lpMaximumApplicationAddress);

			Debug_EnumLoadedModule();
			break;

		case WM_SIZE:
			SetWindowPos(GetDlgItem(hwndDlg, IDC_LST),0,0,0,LOWORD(lParam), HIWORD(lParam)*2/3, SWP_NOMOVE | SWP_NOOWNERZORDER );
			SetWindowPos(GetDlgItem(hwndDlg, IDC_LST2),0,0,HIWORD(lParam)*2/3,LOWORD(lParam), HIWORD(lParam)/3, SWP_NOOWNERZORDER );
			break;

		case WM_COMMAND:
			if( LOWORD(wParam) == IDCANCEL )
			{
				DestroyWindow(hwndDlg);
				return 0;
			}
			return TRUE;
	}
	return 0;
}

#endif
