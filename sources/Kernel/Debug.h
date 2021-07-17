#ifdef _DEBUG
	BOOL CALLBACK DebugDlgProc(HWND, UINT, WPARAM, LPARAM);
	void __cdecl Debug_PrintA ( int IDLST, const char *format, ... );
	void __cdecl Debug_PrintW ( int IDLST, const WCHAR *format, ... );
#ifdef _UNICODE
#define  Debug_Print		Debug_PrintW
#else
#define  Debug_Print		Debug_PrintA
#endif
	void Debug_Clear( int IDLST );
	void Debug_EnumLoadedModule( void );

	#define dump_str(pstr)			( ((const void *)pstr == NULL) ? "0" : (PSTR)(pstr) )
#endif

