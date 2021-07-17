/////////////
// CtrlClock.h

#define CLOCKCLASS      _T("CClock")
typedef struct
{
  int		cxClient, cyClient;	// Taille de la zone client du contrôle
  POINT		ptCenter;			// Centre de la zone client du contrôle
  
  WORD		wHour;				// Heure affichée
  WORD		wMin;				// Minute affichée
  WORD		wSec;				// Seconde affichée
  
  POINT ptHands[3][5];			// Polygones des aiguilles
} CCLOCKINFO, *PCCLOCKINFO;

LRESULT CALLBACK CClockWndProc (HWND, UINT, WPARAM, LPARAM);
void SetIsotropic (HDC, int, int);
void RotatePoint (PPOINT, PPOINT, PPOINT, int, int);

#define CC_SETTIME		WM_USER+10
#define CC_GETTIME		WM_USER+11

#define WM_CTLCOLORCLOCK	WM_USER+12

// flags pour le message CC_SETTIME et la fonction DrawHands
#define ST_HOUR			1
#define ST_MIN			2
#define ST_SEC			4
#define ST_ALL			(ST_SEC|ST_MIN|ST_HOUR)

void DrawHands (PCCLOCKINFO pCI, HDC hdc, BOOL bDraw, BOOL bEnabled);
void DrawClock (PCCLOCKINFO pCI, HDC hdc, HWND hwnd, BOOL bEnabled);
