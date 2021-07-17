#pragma once

// - EXP_SHELLEXT is defined if the functions need to be exported. It is defined for the ShellExt sub-project.
// - IMP_SHELLEXT is defined if the functions are imported. This is the case for the Manager.
// - If the implementation .cpp file corresponding to the interface .h file that includes the present header 
//   is part of the project files (as for the project Kernel) then neither is defined.
#ifdef EXP_SHELLEXT
#	define DECLSPECIFIER __declspec(dllexport)
#	define DATADECLSPECIFIER extern __declspec(dllexport)
#elif defined(IMP_SHELLEXT)
#    define DECLSPECIFIER __declspec(dllimport)
#    define DATADECLSPECIFIER __declspec(dllimport)
#else
#    define DECLSPECIFIER
#    define DATADECLSPECIFIER extern
#endif
