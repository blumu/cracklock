.586p
.model flat, stdcall  ; 32 bit memory model
option scoped         ; local labels are enabled, global labels inside
                      ; PROC should be defined with double colons (LABEL::)
option casemap :none  ; case sensitive

include windows.inc
include kernel32.inc
include user32.inc
include masm32.inc
include comdlg32.inc
include ntdll.inc


includelib kernel32.lib
includelib user32.lib
includelib masm32.lib
includelib comdlg32.lib

EXTERNDEF ProcessInformation :PROCESS_INFORMATION

    ; ---------------------
    ; literal string MACRO
    ; ---------------------
      literal MACRO quoted_text:VARARG
        LOCAL local_text
        .data
          local_text db quoted_text,0
        .code
        EXITM <local_text>
      ENDM

      CTEXT MACRO quoted_text:VARARG
        EXITM <offset literal(quoted_text)>
      ENDM      

.data
the_byte            db 1
null_byte           db 0


.data?

byte_location       dd ?
dw_buffer           dd ?

.code

InstallUninstall proc C action:dword
	mov     eax, action
	.IF !eax
		call get_byte_location
		mov byte_location,eax
		.if (eax)
			;1==Debugger 0==Clean ;)
			;invoke Writememory,addr null_byte,byte_location,1,MM_RESTORE
			invoke WriteProcessMemory,ProcessInformation.hProcess, byte_location, addr null_byte,1,0
			.if (eax!=1)
				;ooopps
				; invoke "Error WriteMemory failed"
			.elseif
			  ; IsDebugPresent hidden
			.endif
		.endif
	.ELSEIF eax == 1
		call get_byte_location
		mov byte_location,eax
		.if (eax)
			;resore original code
			;invoke Writememory,addr the_byte,byte_location,1,MM_RESTORE
			invoke WriteProcessMemory,ProcessInformation.hProcess,byte_location, addr the_byte,1,0
			
			.if (eax!=1)
				;ooopps
				; invoke "Error WriteMemory failed"
			.elseif
			  ;IsDebugPresent restored
			.endif
		.endif
	.ENDIF
	ret
InstallUninstall endp

; The following comment is an extract from from http://www.securityfocus.com/infocus/1893
;(1) kernel32!IsDebuggerPresent
;IsDebuggerPresent returns 1 if the process is being debugged, 0 otherwise. This API simply reads the PEB!BeingDebugged byte-flag (located at offset 2 in the PEB structure).
;Circumventing it is as easy as setting PEB!BeingDebugged to 0.
;Example:
;call IsDebuggerPresent
;test eax, eax
;jne @DebuggerDetected 
;
; IsDebuggerPresent dissasembled:
;ASSUME FS:NOTHING
;MOV EAX,DWORD PTR FS:[18h]
;MOV EAX,DWORD PTR DS:[EAX+30h]
;MOVZX EAX,BYTE PTR DS:[EAX+2h]


;(2) PEB!BeingDebugged method
;This field refers to the second byte in the Process Environment Block of the process. It is set by the system when the process is debugged.
;This byte can be reset to 0 without consequences for the course of execution of the program (it is an informative flag).
;;Example:
;mov eax, fs:[30h]
;mov eax, byte [eax+2]
;test eax, eax
;jne @DebuggerDetected

;CMP EAX,1
;JE @DebuggerDetected

get_byte_location proc
	LOCAL thread_context :CONTEXT
	LOCAL pbi :PROCESS_BASIC_INFORMATION
	LOCAL dwSize :DWORD

    push ebx
    ;;;;;invoke Getcputhreadid
    ;;;;;.if (eax)
        ;;;;;invoke FindThread,eax                  ;retreive thread info
        ;;;;;assume eax:ptr t_thread
        ;;;;;push [eax].reg.base[4*4]               ;base of FS
        ;pop ebx
        ;add ebx,30h
        ;;;;;invoke Readmemory,addr dw_buffer,ebx,4,MM_RESTORE
		;invoke ReadProcessMemory,ProcessInformation.hProcess,ebx,addr dw_buffer,4,0
        ;mov eax,dw_buffer
		;mov thread_context.ContextFlags, CONTEXT_CONTROL;
		invoke GetThreadContext, ProcessInformation.hThread,addr thread_context
		mov eax, thread_context.fs
		add eax, 30h
		[eax].reg.base[4*4]               ;base of FS
		invoke ReadProcessMemory,ProcessInformation.hProcess, 
				pbi.PebBaseAddress, &peb, sizeof(peb), &dwBytesRead

		invoke NtQueryInformationProcess
		; Attempt to get basic info on process
		invoke NtQueryInformationProcess,ProcessInformation.hProcess,
                                               ProcessBasicInformation,
                                               pbi,
                                               sizeof( PROCESS_BASIC_INFORMATION),
                                               0 ;addr dwSizeNeeded
		; Read Process Environment Block (PEB)
		invoke ReadProcessMemory,ProcessInformation.hProcess, pbi.PebBaseAddress, &peb, sizeof(peb), &dwBytesRead
		mov eax, pbi.PebBaseAddress
		

		; make eax points to the BeingDebugged field of the PEB structure
        add eax,2h
    ;.endif
    pop ebx
    ret
get_byte_location endp


END