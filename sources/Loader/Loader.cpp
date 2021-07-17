// LOADER.C


#include <windows.h>    // Définitions standards de Windows
#include <commctrl.h>    // Contrôles communs
#include <stddef.h>     // Définition standard du C
#include <tchar.h>
#include <Winternl.h>
#include "..\Resources\Resources.h" // Resources localisés
#include "..\Common\Common.h"    // Outils communs
#include "..\version.h"
#include <strsafe.h>
#include <shlwapi.h>

//======================== NTDLL Run-Time Dynamic Linking

typedef NTSTATUS (NTAPI *NtQueryInformationProcess_PROC)(
    IN  HANDLE ProcessHandle,
    IN  PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN  ULONG ProcessInformationLength,
    OUT PULONG ReturnLength
    );

NtQueryInformationProcess_PROC pNtQueryInformationProcess;
HINSTANCE hinstNtdll; 

bool LoadNtdll();
void UnloadNtdll();


//======================== Variables globales =================================

BOOL                        FFirstBreakpointHit = FALSE,
                            FSecondBreakpointHit = FALSE;

PROCESS_INFORMATION         ProcessInformation;
CREATE_PROCESS_DEBUG_INFO   ProcessDebugInfo;

CONTEXT                     OriginalThreadContext,
                            FakeLoadLibraryContext;
PVOID                       PFirstCodePage;

#define PAGE_SIZE   4096
BYTE OriginalCodePage[PAGE_SIZE];
BYTE NewCodePage[PAGE_SIZE];


//======================== Prototypes =============================================

DWORD HandleDebugEvent( DEBUG_EVENT * event );
void  HandleException(LPDEBUG_EVENT lpEvent, PDWORD continueStatus);
BOOL  InjectDll(void);
BOOL  ReplaceOriginalPagesAndContext(void);
PVOID FindFirstCodePage(HANDLE hProcess, PVOID PProcessBase);
BOOL CALLBACK SelectDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
bool HideDebugger(bool hide);


//======================== Implémentation =============================================

// Obtient le chemin d'accès complet au fichier CKKERN.DLL
BOOL GetKernelDllPath(PSTR buffer, UINT bufferByteSize)
{
    // Obtient le chemin d'accès complet de LOADER.EXE
    // (CLKERN.DLL est supposé se trouver dans le même répertoire).
    GetModuleFileNameA(0, buffer, bufferByteSize);

    PSTR pszFilename = strrchr(buffer, '\\');
    if ( !pszFilename )
        return FALSE;

    strcpy_s(pszFilename+1, bufferByteSize-(pszFilename+1-buffer), CL_KERNEL_DLL_A);
    return TRUE;
}


PTSTR sauteparametre (PTSTR pszCmdline)
{
    TCHAR delim;
    PTSTR pNextparam;

    // saute les blancs
    while( *pszCmdline == ' ' )
        pszCmdline++;

    // quel est le délimitateur ?
    if( *pszCmdline == '\"' )
    {
        delim = '\"'; // c'est un parametre entre guillemets
        pszCmdline++;
    }
    else
        delim = ' ';

    // extrait le chemin d'access entre guillemets
    if( pNextparam = _tcschr(pszCmdline, delim) )
    {
        // saute le delimitateur
        pNextparam++;

        // saute les blancs
        while( *pszCmdline == ' ' )
            pszCmdline++;

        return pNextparam;
    }
    else
        return pszCmdline;
}



int APIENTRY _tWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpszCmdLine, int nCmdShow )
{
    STARTUPINFO si;
    DEBUG_EVENT event;
    PTSTR  pMsgBuf;
    PTSTR  pParse;
    PTSTR  pCmdline;
    
    PTSTR	pszCmdLn;
    DWORD  continueStatus;
    BOOL  bWaitResult;
    size_t   cntAlloc;
    PTSTR  pFin;
    WCHAR  delim;
    DWORD  dw;


    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &ProcessInformation, sizeof(ProcessInformation) );


    //////////// Parse la ligne de commande
    pCmdline = GetCommandLine();

    // saute le premier parametre (ie. le chemin d'access de loader.exe)
    pParse = sauteparametre(pCmdline);

    // saute les blancs
    while( *pParse == ' ' )
        pParse++;

    // quel est le délimitateur ?
    if( *pParse == '\"' )
    {
        delim = '\"';
        pParse++;
    }
    else
        delim = ' ';

    // extrait le nom du programme
    WCHAR  szExe[_MAX_PATH];
    cntAlloc = 0;
    if( pFin = _tcschr(pParse, delim) )
    {
        _tcsncpy_s(szExe, pParse, pFin-pParse);
        szExe[pFin-pParse] = '\0';
        cntAlloc = _tcslen(pFin+1);
    }
    else
        _tcscpy_s(szExe, pParse);

    cntAlloc += _tcslen(szExe) + 4; // 2 pour entourer de 2 guillemets, 1 espace et le 0 terminal

    // recrée la ligne de commande avec le nom du programme entoure
    // de guillemets
    pszCmdLn = (PTSTR)malloc(sizeof(TCHAR) * cntAlloc) ;
    _tcscpy_s(pszCmdLn, cntAlloc, _T("\""));
    _tcscat_s(pszCmdLn, cntAlloc, szExe);
    _tcscat_s(pszCmdLn, cntAlloc, _T("\""));
    if( pFin ) {
        if( delim == ' ' )
            _tcscat_s(pszCmdLn, cntAlloc, _T(" "));

        _tcscat_s(pszCmdLn, cntAlloc, pFin+1);
    }

    /////////////
    
    InitCommon(hInstance);
    if( szExe[0] != '\0') {
        // Convertit le nom de fichier relatif en chemin absolu
        TCHAR fullExePath[_MAX_PATH];
        PTSTR pFullpath;
        if( PathIsRelative(szExe) ) {
            pFullpath = fullExePath; 
            GetCurrentDirectory(_countof(fullExePath),fullExePath);
            PathAppend(fullExePath, szExe);
        }
        else
            pFullpath = szExe;


        AppSettings appset;
		bool bDebug = appset.Load( pFullpath ) && appset.szLoaderFile[0] == '\0';

        // Initialise la structure pour le lancement d'une application
        if( !CreateProcess(
                    0,        // lpszImageName
                    pszCmdLn,     // lpszCommandLine
                    0,        // lpsaProcess
                    0,        // lpsaThread
                    FALSE,      // fInheritHandles
                    bDebug ? DEBUG_ONLY_THIS_PROCESS : 0, // fdwCreate
                    0,        // lpvEnvironment
                    0,        // lpszCurDir
                    &si,       // lpsiStartupInfo
                    &ProcessInformation        // lppiProcInfo
                ) ) {
            HMODULE hResDll = LoadResDll(false);

            pMsgBuf = LoadResString(hResDll, IDS_ERR1);
            MessageBox(0, pMsgBuf, _T(APP_TITLE_LOADER), MB_OK | MB_ICONEXCLAMATION);
            FreeResString(pMsgBuf);

            FreeLibrary(hResDll);

            bDebug = FALSE;
        }

        // y-a-t il besoin de debugger l'application pour injecter la DLL?
        if( bDebug ) {
            LoadNtdll();
            while( 1 ) {
                bWaitResult = WaitForDebugEvent(&event, INFINITE);

                continueStatus = HandleDebugEvent( &event );

                if ( event.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT )
                    break;

                ContinueDebugEvent( event.dwProcessId,
                                    event.dwThreadId,
                                    continueStatus );
            }
            UnloadNtdll();
        }
    }

    // par defaut ou en cas d'erreur lance le cracklock manager
    else {
        TCHAR exepath[_MAX_PATH];
        if( GetCLPath(exepath, _countof(exepath)) ) {
            //_tcscpy(szExe, szPathCracklockBin);
            _tcscat_s(szExe, _T("\\") DIR_CL_BIN _T("\\") CL_MANAGER_EXE);
            // Exécute Cracklock Manager
            BOOL bret = CreateProcess( szExe,   // No module name (use command line).
                                       NULL, // Command line.
                                       NULL,             // Process handle not inheritable.
                                       NULL,             // Thread handle not inheritable.
                                       FALSE,            // Set handle inheritance to FALSE.
                                       0,                // No creation flags.
                                       NULL,             // Use parent's environment block.
                                       NULL,             // Use parent's starting directory.
                                       &si,              // Pointer to STARTUPINFO structure.
                                       &ProcessInformation );             // Pointer to PROCESS_INFORMATION structure.

            if(!bret)
                dw = GetLastError();
        }
    }
    free(pszCmdLn);

    return 0;
}


//PSTR SzDebugEventTypes[] = {
//"",
//"EXCEPTION",
//"CREATE_THREAD",
//"CREATE_PROCESS",
//"EXIT_THREAD",
//"EXIT_PROCESS",
//"LOAD_DLL",
//"UNLOAD_DLL",
//"OUTPUT_DEBUG_STRING",
//"RIP",
//};


DWORD HandleDebugEvent( DEBUG_EVENT * event )
{
    DWORD continueStatus = DBG_EXCEPTION_NOT_HANDLED;
    // TCHAR buffer[1024];

    // StringCchPrintf (buffer, _countof(buffer), _T("Event: %s\r\n"),
    //          SzDebugEventTypes[event->dwDebugEventCode]);
    // OutputDebugString(buffer);


    if ( event->dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT )
    {
        ProcessDebugInfo = event->u.CreateProcessInfo;
    }
    else if ( event->dwDebugEventCode == EXCEPTION_DEBUG_EVENT )
    {
        HandleException(event, &continueStatus);
    }
    else if ( event->dwDebugEventCode == LOAD_DLL_DEBUG_EVENT )
    {
        event->u.LoadDll.lpBaseOfDll = event->u.LoadDll.lpBaseOfDll;
    }

    return continueStatus;
}

void HandleException(LPDEBUG_EVENT lpEvent, PDWORD continueStatus)
{
    // TCHAR buffer[128];
    // StringCchPrintf (buffer, _countof(buffer), _T("Exception code: %X  Addr: %08X\r\n"),
    //          lpEvent->u.Exception.ExceptionRecord.ExceptionCode,
    //          lpEvent->u.Exception.ExceptionRecord.ExceptionAddress);
    // OutputDebugString(buffer);


    if ( lpEvent->u.Exception.ExceptionRecord.ExceptionCode
            == EXCEPTION_BREAKPOINT )
    {
        if ( FFirstBreakpointHit == FALSE )
        {
            InjectDll();
            FFirstBreakpointHit = TRUE;
        }
        else if ( FSecondBreakpointHit == FALSE )
        {
            ReplaceOriginalPagesAndContext();
            FSecondBreakpointHit = TRUE;
			
			HideDebugger(true);
        }

        *continueStatus = DBG_CONTINUE;
    }
    else
    {
        *continueStatus = DBG_EXCEPTION_NOT_HANDLED;
    }
}
#if defined(_X86_)
/*
    X86 assembly code:

	81 EC 00 10 00 00			        sub esp,10    					; allocate 16 bytes on the stack
	68 -- -- -- -- --                   push address_of data_DllName	; push address of DLL name
	E8 -- -- -- -- --                   call KERNEL32.LoadLibrary
	CC    			                    int 3							; debugger breakpoint
                        data_DllName:   DB 'TOINJECT.DLL'

*/
#pragma pack ( 1 )
typedef struct
{
    WORD    instr_SUB;
    DWORD   operand_SUB_value;
    BYTE    instr_PUSH;
    DWORD   operand_PUSH_value;
    BYTE    instr_CALL;
    DWORD   operand_CALL_offset;
    BYTE    instr_INT_3;
    char    data_DllName[1]; // char et non pas TCHAR!!!
}
FAKE_LOADLIBRARY_CODE, * PFAKE_LOADLIBRARY_CODE;

void AssembleInjectionCode(PFAKE_LOADLIBRARY_CODE pNewCode, PVOID PFirstCodePage, PVOID pfnLoadLibrary)
{
    pNewCode->instr_SUB = 0xEC81;
    pNewCode->operand_SUB_value = 0x1000;

    pNewCode->instr_PUSH = 0x68;
    pNewCode->operand_PUSH_value = (DWORD)PFirstCodePage
                                   + offsetof(FAKE_LOADLIBRARY_CODE, data_DllName);
    pNewCode->instr_CALL = 0xE8;
    pNewCode->operand_CALL_offset = (DWORD)pfnLoadLibrary
                                    - (DWORD)PFirstCodePage - offsetof(FAKE_LOADLIBRARY_CODE,instr_CALL) - 5;
    pNewCode->instr_INT_3 = 0xCC;
}

#elif defined(_AMD64_)
/*
    AMD64 assembly code:

                     48 83 EC 20          sub         rsp,20h               ; allocate 8kb on the stack

    0000000000401000 54                   push        rsp                           ; save stack ptr
    
    0000000000401001 FF 34 24             push        qword ptr [rsp]   
    0000000000401004 40 80 E4 F0          and         spl,0F0h                      ; stack alignment
    0000000000401008 48 8D 0D F9 0F 00 00 lea         rcx,[402008h]  (data_DllName) ; Address of the name of the DLL to inject
    000000000040100F 48 83 EC 20          sub         rsp,20h                       ; Allocate parameter stack for next call to LoaLibrary,
    0000000000401013 E8 E8 1F 00 00       call        0000000000403000 (KERNEL32.LoadLibrary)
    0000000000401018 48 83 C4 28          add         rsp,28h                       ; free parameters stack for previous call.

    000000000040101C 5C                   pop         rsp                           ; restore stack ptr
    000000000040101D CC                   int         3                             ; call debugger through breakpoint

                        data_DllName:   DB 'TOINJECT.DLL'

*/
//#error DLL injection unsupported for AMD64!
#pragma pack ( 1 )
typedef struct
{
    WORD    instr_SUB;
    
    WORD    operand_SUB_value;
    BYTE    instr_PUSH;

    BYTE    instr_PUSH2;
    WORD    operand_PUSH2_value;

    WORD    instr_AND;
    WORD    operand_AND_value;
    
    WORD    instr_LEA;
    BYTE    instr_LEA_aux;
    DWORD   operand_LEA_value;
    
    WORD    instr_SUB2;
    WORD    operand_SUB2_value;
    
    BYTE    instr_CALL;
    DWORD   operand_CALL_offset;
    
    DWORD   instr_ADD;
    
    BYTE    instr_POP_RSP;
    
    BYTE    instr_INT_3;
    char    data_DllName[1]; // char et non pas TCHAR!!!
}
FAKE_LOADLIBRARY_CODE, * PFAKE_LOADLIBRARY_CODE;

void AssembleInjectionCode(PFAKE_LOADLIBRARY_CODE pNewCode, PVOID PFirstCodePage, PVOID pfnLoadLibrary)
{
    pNewCode->instr_SUB = 0x8348;
    pNewCode->operand_SUB_value = 0x20EC;

    pNewCode->instr_PUSH = 0x54;

    pNewCode->instr_PUSH2 = 0xFF;
    pNewCode->operand_PUSH2_value = 0x2434;

    pNewCode->instr_AND = 0x8040;
    pNewCode->operand_AND_value = 0XF0E4;

    pNewCode->instr_LEA = 0x8D48;
    pNewCode->instr_LEA_aux = 0x0D;
    pNewCode->operand_LEA_value = offsetof(FAKE_LOADLIBRARY_CODE, data_DllName)-(offsetof(FAKE_LOADLIBRARY_CODE, instr_LEA)+7);

    pNewCode->instr_SUB2 = 0x8348;
    pNewCode->operand_SUB2_value = 0x20EC;

    pNewCode->instr_CALL = 0xE8;
    pNewCode->operand_CALL_offset = (DWORD)pfnLoadLibrary - ((DWORD)PFirstCodePage + offsetof(FAKE_LOADLIBRARY_CODE,instr_CALL) + 5);
    pNewCode->instr_ADD = 0x28C48348;
    pNewCode->instr_POP_RSP = 0x5c;
    pNewCode->instr_INT_3 = 0xCC;
}
#endif

BOOL InjectDll(void)
{
    BOOL retCode;
    SIZE_T cBytesMoved;
    char szDllName[_MAX_PATH]; // Attention, c'est bien char et non pas TCHAR
    FARPROC pfnLoadLibrary;
    PFAKE_LOADLIBRARY_CODE pNewCode;

    ////////////////////////////////////////////////////
    // Étape 1 - Détermination des adresses importantes
    ////////////////////////////////////////////////////

    pfnLoadLibrary = GetProcAddress( GetModuleHandleA("KERNEL32.DLL"), "LoadLibraryA" );
    if ( !pfnLoadLibrary )
        return FALSE;

    PFirstCodePage = FindFirstCodePage(ProcessInformation.hProcess,
                                       ProcessDebugInfo.lpBaseOfImage);
    if ( !PFirstCodePage )
        return FALSE;

    if ( !GetKernelDllPath(szDllName, sizeof(szDllName)) )
        return FALSE;

    OriginalThreadContext.ContextFlags = CONTEXT_CONTROL;
    if ( !GetThreadContext(ProcessInformation.hThread,&OriginalThreadContext))
        return FALSE;

    /////////////////////////////////////////////////////////////
    // Étape 2 - Conserve une copie de la page de code d'origine
    /////////////////////////////////////////////////////////////

    retCode = ReadProcessMemory(ProcessInformation.hProcess, PFirstCodePage,
                                OriginalCodePage, sizeof(OriginalCodePage),
                                &cBytesMoved);
    if ( !retCode || (cBytesMoved != sizeof(OriginalCodePage)) )
        return FALSE;

    //////////////////////////////////////////////////////////////////////////////////////
    // Étape 3 - Écriture de la nouvelle page de code et changement du contexte de thread
    //////////////////////////////////////////////////////////////////////////////////////

    pNewCode = (PFAKE_LOADLIBRARY_CODE)NewCodePage;

    memcpy(pNewCode->data_DllName, szDllName, strlen(szDllName)+1); // Copie le nom de la DLL
    AssembleInjectionCode(pNewCode, PFirstCodePage, pfnLoadLibrary);

    // Écriture de la nouvel page de code
    retCode = WriteProcessMemory(ProcessInformation.hProcess, PFirstCodePage,
                                 &NewCodePage, sizeof(NewCodePage),
                                 &cBytesMoved);
    if ( !retCode || (cBytesMoved != sizeof(NewCodePage)) )
        return FALSE;

    FakeLoadLibraryContext = OriginalThreadContext;
#if defined(_X86_)
    FakeLoadLibraryContext.Eip = (DWORD)PFirstCodePage;
#elif defined (_AMD64_)
    FakeLoadLibraryContext.Rip = (DWORD64)PFirstCodePage;
#else
    #error DLL injection unsupported for this platform!
#endif

    if ( !SetThreadContext(ProcessInformation.hThread,
                           &FakeLoadLibraryContext) )
        return FALSE;

    return TRUE;
}

BOOL ReplaceOriginalPagesAndContext(void)
{
    BOOL retCode;
    SIZE_T cBytesMoved;

    retCode = WriteProcessMemory(ProcessInformation.hProcess, PFirstCodePage,
                                 OriginalCodePage, sizeof(OriginalCodePage),
                                 &cBytesMoved);
    if ( !retCode || (cBytesMoved != sizeof(OriginalCodePage)) )
        return FALSE;

    if ( !SetThreadContext(ProcessInformation.hThread,
                           &OriginalThreadContext) )
        return FALSE;

    return TRUE;
}

PVOID FindFirstCodePage(HANDLE hProcess, PVOID PProcessBase)
{
    // Lit l'offset du PE header du programme debugge
    DWORD peHdrOffset;
    SIZE_T cBytesMoved;
    if ( !ReadProcessMemory(ProcessInformation.hProcess,
                            (PBYTE)PProcessBase
                            + offsetof(IMAGE_DOS_HEADER, e_lfanew), // + 0x3C
                            &peHdrOffset,
                            sizeof(peHdrOffset),
                            &cBytesMoved) )
        return NULL;


    // Obtient l'adresse du point d'entre du programme
    DWORD baseOfCode; // L'adresse est 32bits y compris sur AMD64
    if ( !ReadProcessMemory(ProcessInformation.hProcess,
                            (PBYTE)PProcessBase
                            + peHdrOffset                            
                            + offsetof(IMAGE_NT_HEADERS, OptionalHeader) //+ 4 + IMAGE_SIZEOF_FILE_HEADER
                            + offsetof(IMAGE_OPTIONAL_HEADER, BaseOfCode),
                            &baseOfCode, sizeof(baseOfCode),
                            &cBytesMoved) )
        return NULL;

    return (PVOID) ((DWORD)PProcessBase + baseOfCode);

}




bool HideDebugger(bool hide)
{
    if( !pNtQueryInformationProcess )
        return FALSE;

	ULONG cbBytes;

	PROCESS_BASIC_INFORMATION pbi;
	NTSTATUS dwStatus = pNtQueryInformationProcess(ProcessInformation.hProcess,
                                               ProcessBasicInformation,
                                               &pbi,
                                               sizeof(PROCESS_BASIC_INFORMATION),
                                               &cbBytes);
	if(dwStatus >= 0 && pbi.PebBaseAddress) {
		/*if(ReadProcessMemory(ProcessInformation.hProcess,
			pbi->PebBaseAddress, &peb, sizeof(peb), &dwBytes)) {
	
		} */
		BYTE data = hide ? 0 : 1;
        SIZE_T ulBytes;
		WriteProcessMemory(ProcessInformation.hProcess, (PBYTE)pbi.PebBaseAddress+0x02, &data, 1, &ulBytes);

        return ulBytes==1;
	}
    return FALSE;
}



bool LoadNtdll()
{
    hinstNtdll = LoadLibrary(TEXT("ntdll.dll")); 
 
    if( hinstNtdll != NULL )  { 
        pNtQueryInformationProcess = (NtQueryInformationProcess_PROC) GetProcAddress(hinstNtdll, ("NtQueryInformationProcess")); 
 
        return TRUE;
    }
    else
        return FALSE;
}

// Unload NTDLL.DLL 
void UnloadNtdll()
{
    FreeLibrary(hinstNtdll); 
} 