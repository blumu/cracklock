//==================================
// William BLUM 1998
// KERNEL.H
//==================================

extern "C" BOOL WINAPI CracklockInit();
extern "C" BOOL WINAPI CracklockDeinit();
LRESULT CALLBACK HookFunction(int nCode, WPARAM wParam, LPARAM lParam);

HMODULE WINAPI GetModuleHandleACRK(LPCSTR lpModuleName);
HMODULE WINAPI GetModuleHandleWCRK(LPCWSTR lpModuleName);
DWORD WINAPI GetModuleFileNameACRK(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
DWORD WINAPI GetModuleFileNameWCRK(HMODULE hModule, LPWSTR lpFilename, DWORD nSize);

HMODULE WINAPI LoadLibraryACRK(LPCSTR lpLibFileName);
HMODULE WINAPI LoadLibraryWCRK(LPCWSTR lpLibFileName);
HMODULE WINAPI LoadLibraryExACRK(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
HMODULE WINAPI LoadLibraryExWCRK(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
BOOL WINAPI FreeLibraryCRK( HMODULE hLibModule );
void WINAPI FreeLibraryAndExitThreadCRK( HMODULE hLibModule, DWORD dwExitCode );
FARPROC WINAPI GetProcAddressCRK( HMODULE hModule, LPCSTR lpProcName );

DWORD WINAPI GetTimeZoneInformationCRK(LPTIME_ZONE_INFORMATION lpTimeZoneInformation);
VOID WINAPI GetSystemTimeCRK (LPSYSTEMTIME lpSystemTime);
VOID WINAPI GetLocalTimeCRK (LPSYSTEMTIME lpLocalTime);
VOID WINAPI GetSystemTimeAsFileTimeCRK (LPFILETIME lpSystemTimeAsFileTime);

struct HOOKENTRY
{
	PSTR	pFuncName;
	FARPROC	pHookFunc;
};
#define NB_HOOKENTRY(hooktbl)	(sizeof((hooktbl))/sizeof(HOOKENTRY))
