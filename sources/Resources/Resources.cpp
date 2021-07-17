typedef int             BOOL;
typedef void			*HANDLE;
typedef unsigned long   DWORD;
typedef void			*LPVOID;

BOOL __stdcall DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return 1;
}
