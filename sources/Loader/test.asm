;------------------------------------------------------------------
;
;      Hello64World1 - copyright Jeremy Gordon 2005-6
;
;      SIMPLE "HELLO WORLD" WINDOWS CONSOLE PROGRAM - for GoAsm 64-bits
;
;      Assemble using GoAsm /x64 Hello64World1 (produces PE COFF file)
;      Then link as windows console program using GoLink as follows:-
;      GoLink /console hello64world1.obj kernel32.dll
;      (add -debug coff if you want to watch the program in the debugger)
;
;      Note that the GetStdHandle and WriteFile calls are to kernel32.dll
;------------------------------------------------------------------
;
DATA SECTION
;
ALIGN 8           ;align qword following on 8-byte boundary
RCKEEP DQ 0       ;temporary place to keep things

DLLNAME DB 'GDI32.DLL'       ;temporary place to keep things
Message DB 'Hello 64 World (from GoAsm)'
;
CODE SECTION
;
START:
ARG ADDR DLLNAME       ;RCKEEP receives output from API
INVOKE LoadLibraryA     ;get, in rax, handle to active screen buffer
INT 3
ARG -11                 ;STD_OUTPUT_HANDLE
INVOKE GetStdHandle     ;get, in rax, handle to active screen buffer
;********************
ARG 0,ADDR RCKEEP       ;RCKEEP receives output from API
ARG 27                  ;length of string
ARG ADDR Message,RAX    ;rax=handle to active screen buffer
INVOKE WriteFile
XOR RAX,RAX             ;return zero
RET
