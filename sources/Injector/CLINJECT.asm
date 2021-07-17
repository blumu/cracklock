; #########################################################################

      .386
      .model flat, stdcall  ; 32 bit memory model
      option casemap :none  ; case sensitive

      include CLINJECT.inc     ; local includes for this file

; #########################################################################

.data
    CL_INJECTOR_EXE db "CLINJECT.EXE",0

.code

start:

    invoke WinMain
    invoke ExitProcess, eax

WinMain proc

		LOCAL	osv  :OSVERSIONINFO
		LOCAL	hSemaphore  :HANDLE

		;mov osv.dwOSVersionInfoSize, sizeof OSVERSIONINFO
		;invoke GetVersionEx, ADDR osv

		;.IF osv.dwPlatformId == VER_PLATFORM_WIN32_NT
			;return 1
		;.ENDIF


		invoke CreateSemaphore, NULL, 0, 1, ADDR CL_INJECTOR_EXE
		mov hSemaphore, eax

		invoke GetLastError

		.IF eax == ERROR_ALREADY_EXISTS
			invoke CloseHandle, hSemaphore
			return 2
		.ENDIF


		invoke CracklockInit

		invoke GetCurrentProcess
		invoke SetPriorityClass, eax, IDLE_PRIORITY_CLASS

		invoke GetCurrentThread
		invoke SetThreadPriority, eax, THREAD_PRIORITY_IDLE

		invoke WaitForSingleObject, hSemaphore, INFINITE

		invoke CracklockDeinit;

		invoke CloseHandle, hSemaphore

		return 0


toto:
loop toto

WinMain endp

end start
