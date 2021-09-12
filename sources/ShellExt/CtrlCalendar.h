/////////////
// CtrlCalendar.h

#define CALENDARCLASS     _T("CCalendar")
typedef struct
{
	int		cxClient, cyClient;       // Taille de la zone client du contr�le
	int		cxBox, cyBox;             // Taille d'une bo�te contenant un jour

	WORD	wDay;                   // Jour du mois affich�e
	WORD	wMonth;                 // Mois de l'ann�e affich�e  
	WORD	wYear;                  // Ann�e affich�e

	int   iFirstDay;              // Position du premier jour du mois
} CCALENDARINFO, *PCCALENDARINFO;

void GetWININI( void );
void DrawDaysTitle(PCCALENDARINFO, HDC, BOOL);
void DrawDaysBox(PCCALENDARINFO, HDC);
void DrawSelectedDay(PCCALENDARINFO, HDC, BOOL, BOOL);
void DrawCalendarFocus(PCCALENDARINFO, HDC);
LPRECT GetDayRect(PCCALENDARINFO, WORD, LPRECT);
LRESULT CALLBACK CCalendarWndProc (HWND, UINT, WPARAM, LPARAM);

#define CC_SETDATE			WM_USER+20
#define CC_GETDATE			WM_USER+21
#define CCN_DATECHANGE		WM_USER+22

#define SD_DAY				1
#define SD_MONTH			2
#define SD_YEAR				4
#define SD_ALL				(SD_DAY|SD_MONTH|SD_YEAR)
#define SD_NOTIFY			8
