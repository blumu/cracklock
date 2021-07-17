// DLGRetry.h
INT_PTR CALLBACK RetryDlgProc(HWND, UINT, WPARAM, LPARAM);


typedef struct {
    HFONT   hFont;
    CNodes  *files;
} DLGRETRYPARAM;