// ListView.cpp

#include <windows.h>		// D�finitions standards de Windows
#include <commctrl.h>		// Contr�les communs
#include "ListView.h"

// Ajoute un item au contr�le ListView
int CALLBACK LV_Add (HWND hwnd, int iPos, LPCTSTR pszFN, LPCTSTR pszDIR, int iImage, char state, char stateImg, LPARAM lParam)
{
	LV_ITEM		lvi;

	// Prepare la structure pour la cr�ation d'un item
	if( pszFN )
	{
		lvi.mask = LVIF_TEXT;
		lvi.pszText = (PTSTR)pszFN;
	}
	else
		lvi.mask = 0;
	
	if( iImage != -1 )
	{
		lvi.mask |= LVIF_IMAGE;
		lvi.iImage = iImage;
	}

	if( lParam != -1 )
	{
		lvi.mask |= LVIF_PARAM;
		lvi.lParam = lParam;
	}

	if( state != -1 )
	{
		lvi.mask |= LVIF_STATE;
		lvi.stateMask = (UINT)-1;
		lvi.state = (UINT)state | INDEXTOSTATEIMAGEMASK((UINT)stateImg);
	}
	else
		lvi.stateMask = 0;

	
	lvi.iItem = iPos;
	lvi.iSubItem = 0;

	// Ajoute l'item au contr�le ListView
	lvi.iItem = ListView_InsertItem(hwnd, &lvi);
	ListView_SetItemText( hwnd, lvi.iItem, 1, (PTSTR)pszDIR );
			
	return lvi.iItem;
}

// G�re les �venements 'click sur une colonne' du contr�le ListView
void LV_ColumnClick( LPARAM lParam, PFNLVCOMPARE pfnCompare)
{          
#define pnm ((NM_LISTVIEW *) lParam) 
  
    static signed short sSortWay = 1,
	                    sLastSortColumn = -1;

    if ( sLastSortColumn == pnm->iSubItem )
        sSortWay = -sSortWay;
    else
        sSortWay = 1;
  
    sLastSortColumn = pnm->iSubItem;

    // Tri les items par colonne
    ListView_SortItems(pnm->hdr.hwndFrom, pfnCompare, (LPARAM) MAKELPARAM(sSortWay, sLastSortColumn) );
  
#undef pnm
}

// LV_SetView - sets a list view's window style to change the view. 
// hwndLV - handle to the list view control. 
// dwView - value specifying a view style. 	 
void LV_SetView(HWND hwndLV, DWORD dwView)
{
	// Get the current window style. 
	DWORD dwStyle = GetWindowLong(hwndLV, GWL_STYLE);  

	// Only set the window style if the view bits have changed. 
	if( (dwStyle & LVS_TYPEMASK) != dwView )
		SetWindowLong(hwndLV, GWL_STYLE, (dwStyle & ~LVS_TYPEMASK) | dwView);
}
