
void EnumLoadedModule ( HMODULE hMod );
void InstallHooks( HMODULE hMod );

#if defined(_X86_)
typedef DWORD           IATPTR;
#elif defined(_AMD64_)
typedef ULONGLONG       IATPTR;
#else
#error Unsupported architecture
#endif 
