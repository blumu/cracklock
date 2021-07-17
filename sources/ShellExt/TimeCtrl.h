////////////////
// TimeCtrl.h

#if !defined(TC_CLOCK_SRC) && !defined(TC_CALENDAR_SRC)
#include "CtrlClock.h"
#include "CtrlCalendar.h"
#endif

#define BACKGROUNDCLASS      _T("CBackground")

BOOL RegTimeCtrl (HINSTANCE);
void UnregTimeCtrl (HINSTANCE);
void DeleteGDIObjects(void);
void MAJTCColor (void);
LRESULT CALLBACK CBackgroundWndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

#define GET_CTRL_INFO(ptr, ptrtype)		ptr = (ptrtype) GetWindowLongPtr (hwnd, GWLP_USERDATA)
#define SET_CTRL_INFO(ptr)				SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)ptr)

#define PTR_EXTRA		4

#if defined(TC_CLOCK_SRC) || defined(TC_CALENDAR_SRC)
// Objet GDI globeaux
extern HBRUSH	g_hBRSC3DDK,
				g_hBRSC3DFC,
				g_hBRSC3DHL,
				g_hBRSCACTIVECAPTION,
				g_hBRSCINACTIVECAPTION,
				g_hBRSCWINDOW,
				g_hBRSCBKDESKTOP;

extern HPEN	g_hPENSC3DHL,
			g_hPENSC3DFC,
			g_hPENSC3DDK;
#endif
