; #########################################################################

;     include files
;     ~~~~~~~~~~~~~
      include windows.inc
      include kernel32.inc
      include user32.inc

;     libraries
;     ~~~~~~~~~

      includelib kernel32.lib
      includelib user32.lib

; #########################################################################
        ;wsprintfA PROTO C :DWORD,:VARARG
        ;wsprintf equ <wsprintfA>



        ;=================
        ; Local prototypes
        ;=================
        WinMain PROTO

		; Prototypes des fonctions de CLKERN.DLL
		CracklockDeinit PROTO
		CracklockInit PROTO


      ;=============
      ; Local macros
      ;=============

      szText MACRO Name, Text:VARARG
        LOCAL lbl
          jmp lbl
            Name db Text,0
          lbl:
        ENDM

      m2m MACRO M1, M2
        push M2
        pop  M1
      ENDM

      return MACRO arg
        mov eax, arg
        ret
      ENDM


; ########################### Inserted modules ############################


; #########################################################################
