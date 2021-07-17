// Injector.cpp
//

#include <windows.h>
#include "..\Kernel\Kernel.h"
#include "..\common.h"

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	OSVERSIONINFO versInfo;
	HANDLE hSemaphore;

	// Infos sur la version de Windows
	versInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&versInfo);

	if ( versInfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
		return 1;
				
	hSemaphore = CreateSemaphore(NULL, 0, 2, CL_INJECTOR_EXE);
	
	if( GetLastError() == ERROR_ALREADY_EXISTS )
	{
		CloseHandle(hSemaphore);
		return 2;
	}

	CracklockInit();

	SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
	
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
	
	WaitForSingleObject(hSemaphore, INFINITE);
	
	CracklockDeinit();

	CloseHandle(hSemaphore);

	return 0;
}
