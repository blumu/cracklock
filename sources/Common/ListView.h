// ListView.h
#pragma once

#include "..\Shellext\impexp.h"

int CALLBACK LV_Add (HWND hwnd, int iPos, LPCTSTR pszFN, LPCTSTR pszDIR, int iImage, char state, char stateImg, LPARAM lParam);
DECLSPECIFIER void LV_ColumnClick( LPARAM lParam, PFNLVCOMPARE pfnCompare);
DECLSPECIFIER void LV_SetView(HWND , DWORD );

// this macro returns a list view's style
#define LV_GetView(hwndLV)	GetWindowLong(hwndLV, GWL_STYLE) & LVS_TYPEMASK

// Définit dans un le module qui fait appelle à ListView.cpp
int CALLBACK LV_Compare( LPARAM, LPARAM, LPARAM );
