// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#pragma once

//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers


// Windows Header Files:
#define INC_OLE2        // WIN32, get ole2 from windows.h

#define ISOLATION_AWARE_ENABLED 1 // Necessary for the use of windows common control v6

#include <windows.h>
#include <winuser.h>
#include <windowsx.h>
#include <shlobj.h>

#include <algorithm>
#include <string>
#include <vector>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <errno.h>
#include <strsafe.h>

// Local Header Files

#define EXP_SHELLEXT
#include "DLLMain.h"			// Variables globales
#include "DLLFuncs.h"		    // Functions exportés par CL_SHELLEXT_DLL
#include "..\Common\Common.h"   // Outils communs
#include "..\Common\Listview.h"	    // Outils pour le contôle ListView
#include "..\version.h"			// Version information

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#define _CRT_SECURE_CPP_OVERLOAD_SECURE_NAMES 1

#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
