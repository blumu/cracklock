//==================================
// William BLUM 1998
// HOOK.CPP
//==================================
#include <windows.h>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"
#include "Kernel.h"
#include "Hook.h"
#include "Debug.h"

using namespace std ;

extern HOOKENTRY g_hooktable[];	// Table des interceptions des fonctions syst�mes
extern int g_nbHooks;			// Nombre de hooks dans la table de hooks
extern DWORD g_dwPlatformId;	// Mot de version de Windows
extern HMODULE g_hModule;		// handle de la dll
extern SYSTEM_INFO g_si;		// Infos syst�mes
extern vector<HINSTANCE> lstModule;	// handles des modules charg� par le process courrant

// Macro for adding pointers/DWORDs together without C arithmetic interfering
#define MakePtr( cast, ptr, addValue ) (cast)( (DWORD)(ptr)+(DWORD)(addValue))


// Function name    : IsAdressingSameFunction
// Description      : Retourne vrai si les deux pointeurs de fonction aboutissent � la
//                    m�me fonction (les stubs sont pris en comptes : pointeurs directes
//                    et indirectes)
//
// Return type      : BOOL
//                      * VRAI si les deux pointeurs aboutissent � la m�me fonction,
//                      * FAUX dans le cas contraire
//
// Argument         : IN FARPROC ImportFuncPtr  : pointeur indirectes ou directes sur une premi�re fonction
//                                                (par exemple obtenu de la table d'import).
// Argument         : IN FARPROC RealFuncPtr    : pointeur indirectes ou directes sur une deuxi�me fonction
//                                                (par exemple retourn� par GetProcAdress).
//
BOOL IsAdressingSameFunction( FARPROC ImportFuncPtr, FARPROC RealFuncPtr )
{
    if ( RealFuncPtr == ImportFuncPtr )
        return TRUE;

#if defined(_X86)
    // Sous Windows 95 il se peut que l'adresse pointe sur un stub
    if ( g_dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
    {
            ////// Note de Microsoft :
            //
            // If it's Chicago, and the app is being debugged (as this app is)
            // the loader doesn't fix up the calls to point directly at the
            // DLL's entry point.  Instead, the address in the .idata section
            // points to a PUSH xxxxxxxx / JMP yyyyyyyy stub.  The address in
            // xxxxxxxx points to another stub: PUSH aaaaaaaa / JMP bbbbbbbb.
            // The address in aaaaaaaa is the real address of the function in the
            // DLL.  This ugly code verifies we're looking at this stub setup,
            // and if so, grabs the real DLL entry point, and scans through
            // the InterceptedAPIArray list of addresses again.
            // ***WARNING*** ***WARNING*** ***WARNING*** ***WARNING***
            //////

        // Note de moi : Apparement sous windows 95 et 98
        // il n'y a plus qu'un seul stub contrairement � la beta de windows 95
        // qui comportait deux stubs (selon le papier de Microsoft)

        DWORD StubFuncPtr;
        if ( !IsBadReadPtr((PVOID)ImportFuncPtr, 9) && (*(PBYTE)ImportFuncPtr == 0x68)
             && (*((PBYTE)ImportFuncPtr+5) == 0xE9) )
        {
            StubFuncPtr = *((DWORD *)((PBYTE)ImportFuncPtr+1));

            if ( RealFuncPtr == StubFuncPtr )
                return TRUE;

            if ( !IsBadReadPtr((PVOID)RealFuncPtr, 9) && (*(PBYTE)RealFuncPtr == 0x68)
                && (*((PBYTE)RealFuncPtr+5) == 0xE9) )
            {
                if( *((DWORD *)((PBYTE)RealFuncPtr+1)) == StubFuncPtr)
                    return TRUE;
            }
        }
    }
#endif

    return FALSE;
}

// Fonction de handler pour l'exception
int ExeceptionHandler(DWORD dwCode, PIMAGE_THUNK_DATA pThunk)
{
    if(g_dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        MEMORY_BASIC_INFORMATION mbi;
        DWORD	dwOldProt;

        // Si l'exception n'est pas une "page fault" alors sort
        if (dwCode != EXCEPTION_ACCESS_VIOLATION)
            return EXCEPTION_EXECUTE_HANDLER;

        if( !VirtualQuery(&pThunk->u1.Function, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) )
            return EXCEPTION_EXECUTE_HANDLER;

        if( !VirtualProtect(mbi.BaseAddress, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &dwOldProt) )
            return EXCEPTION_EXECUTE_HANDLER;

        return EXCEPTION_CONTINUE_EXECUTION;
    }
    else
        return EXCEPTION_EXECUTE_HANDLER;
}

FARPROC WINAPI HookImportedFunction(
        HMODULE   hFromModule,        // Handle du module dont les appels � la fonction pszFunctionName du module pszFunctonModule seront intercept�s
        PSTR      pszFunctionModule,  // Nom du module contenant les fonctions � intercepter
        PSTR      pszFunctionName,    // Nom de la fonction � intercepter
        FARPROC   pfnNewProc          // Fonction interceptrice (remplace l'ancienne fonction)
        )
{
    if ( IsBadCodePtr(pfnNewProc) ) // Verifi la validit� de pfn
        return 0;

    // Commence par v�rifier que le nom du module et de la fonction � utiliser sont valides
    FARPROC pfnOriginalProc = GetProcAddress( GetModuleHandleA(pszFunctionModule), pszFunctionName );
    if ( !pfnOriginalProc )
        return 0;


    // Teste pour s'assurer qu'il s'agit d'un fichier image (header 'MZ')
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hFromModule;
    if ( IsBadReadPtr(pDosHeader, sizeof(IMAGE_DOS_HEADER)) )
        return 0;
    if ( pDosHeader->e_magic != IMAGE_DOS_SIGNATURE )
        return 0;

    // Le header MZ a un pointeur vers le header PE
    PIMAGE_NT_HEADERS pNTHeader = MakePtr(PIMAGE_NT_HEADERS, pDosHeader, pDosHeader->e_lfanew);

    // Tests suppl�mentaires pour s'assurer qu'on s'interesse � un image PE
    if ( IsBadReadPtr(pNTHeader, sizeof(IMAGE_NT_HEADERS)) )
        return 0;

    if ( pNTHeader->Signature != IMAGE_NT_SIGNATURE )
        return 0;

    // On poss�de un pointeur valide vers le header du module PE. Maintenant
    // on obtient un pointeur vers la section des imports (.idata)
    PIMAGE_IMPORT_DESCRIPTOR 
            pImportDesc = MakePtr(PIMAGE_IMPORT_DESCRIPTOR, pDosHeader,
                                    pNTHeader->OptionalHeader.
                                    DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].
                                    VirtualAddress);

    // V�rifie que la RVA de la section d'importation n'est pas nul (auquel cas il n'existe pas)
    if ( pImportDesc == (PIMAGE_IMPORT_DESCRIPTOR)pNTHeader )
        return FALSE;

    // It�re � travers le tableau des descripteurs de module import�s, en
    // cherchant le module dont le nom est pszFunctionModule
    // 
    // Bug fix� le 28feb2008: Attention, un module peut etre distribu� sur plusieur entr�es, il est donc imperatif
    // de parcourir toutes les entr�es de la IAT.
    while ( pImportDesc->Name )
    {
        PSTR pszModName = MakePtr(PSTR, pDosHeader, pImportDesc->Name);

        // est-ce que cette entr�e de l'IAT correspond le module recherch�?
        if( !IsBadReadPtr(pszModName, strlen(pszFunctionModule)+1) &&
            _stricmp(pszModName, pszFunctionModule) == 0 ) {

        // Obtient un pointeur sur la table des adresses d'import (IAT) du module trouv�
        PIMAGE_THUNK_DATA pThunk = MakePtr(PIMAGE_THUNK_DATA, pDosHeader, pImportDesc->FirstThunk);

        // Cherche dans la table des adresses d'import l'entr�e
        // qui correspond � l'adresse obtenue par l'appel � GetProcAddress ci-dessus.
        while ( pThunk->u1.Function ) {

            // On l'a trouv� !  On �crase alors l'adresse originale avec l'adresse
            // de la fonction interceptrice.  Retourne l'adresse originale
            // � l'appelant de sorte qu'il puisse etendre le chainage de hooks.
            if( IsAdressingSameFunction((FARPROC)pThunk->u1.Function, pfnOriginalProc ) ) {
                __try {

                    pThunk->u1.Function = (IATPTR)pfnNewProc;
                    return pfnOriginalProc;
                }

                // dans le cas ou le bloc try echoue, le handler ExeceptionHandler s'occupe de modifier les droits d'ecriture en memoire 
                // et de ressayer le bloc try. Sinon ExeceptionHandler echoue alors le bloc except est execute.
                __except(ExeceptionHandler(GetExceptionCode(), pThunk) ) {
    #if _DEBUG
                    OutputDebugString(_T("(Cracklock) cannot hook calls to "));
                    OutputDebugStringA(pszFunctionModule);
                    OutputDebugString(_T("."));
                    OutputDebugStringA(pszFunctionName);
                    OutputDebugString(_T("\n"));
    #endif
                    return 0;
                }
            }

            pThunk++;   // passe � l'adresse de fonction import�e suivante
        }

        }
        pImportDesc++;  // passe au descripteur de module import� suivant
    }

    return 0;   // fonction non trouv�e
}

void InstallHooks( HMODULE hMod )
{
    int i;

    if( ((PVOID)hMod >= g_si.lpMinimumApplicationAddress) &&
        ((PVOID)hMod < g_si.lpMaximumApplicationAddress)
        ) {
        for(i=0; i<g_nbHooks; i++) {
            HookImportedFunction(hMod, ("KERNEL32.DLL"), g_hooktable[i].pFuncName, g_hooktable[i].pHookFunc);
        }
    }
}

void EnumLoadedModule ( HMODULE hMod )
{
    MEMORY_BASIC_INFORMATION	mbi;
    LPVOID	lp;
    TCHAR szMod[_MAX_FNAME];

    // v�rifie que le module n'est pas d�j� list�
    if(	 find(lstModule.begin(), lstModule.end(), (HINSTANCE)hMod) == lstModule.end() )
    {

        lp = g_si.lpMinimumApplicationAddress;
        while( VirtualQuery(lp, &mbi, sizeof(mbi)) == sizeof(mbi)
            && lp < g_si.lpMaximumApplicationAddress )
        {
            if (mbi.State == MEM_FREE)
                mbi.AllocationBase = mbi.BaseAddress;

            if(
                // si le module n'est pas d�j� list�
                (find(lstModule.begin(), lstModule.end(), (HINSTANCE)mbi.AllocationBase) == lstModule.end() )

                // (si on est sous Windows NT alors le flag MEM_IMAGE doit �tre activ�)
                && ( (g_dwPlatformId != VER_PLATFORM_WIN32_NT) || (mbi.Type == MEM_IMAGE) )

                // verifie la correspondance des adresses
                && (mbi.BaseAddress == mbi.AllocationBase)

                // verifie que le module charg� n'est pas CLKERN.DLL
                && (mbi.BaseAddress != (void *)g_hModule)

                // Verifie que le nom du module est disponible
                && GetModuleFileName((HINSTANCE) mbi.AllocationBase, szMod, _MAX_FNAME)
                )
            {
#if _DEBUG
                OutputDebugString(_T("(Cracklock) Injecting in module "));
                OutputDebugString(szMod);
                OutputDebugString(_T("\n"));
#endif
                lstModule.push_back((HINSTANCE)mbi.AllocationBase);
                InstallHooks((HINSTANCE)mbi.AllocationBase);
            }

            lp = (LPVOID)((DWORD)mbi.BaseAddress + (DWORD)mbi.RegionSize);
        }
    }
}

