// DLGRetry.cpp

#include "StdAfx.h"

#include "../Resources/Resources.h"	// resources locazlis�es
#include "Resource.h"		// resources
#include <commctrl.h>		// Contr�les communs
#include "ShellExt.h"		// D�finition de la classe CShellExt
#include "DLGRetry.h"		// Bo�te de dialogue R�essayer


INT_PTR CALLBACK RetryDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    { 
    case WM_INITDIALOG:     
    {
        DLGRETRYPARAM *param = (DLGRETRYPARAM*)lParam;

        // affecte la police unicode m_pcs->m_hfUnicode a tous les controles contenus dans cette fentre
        //EnumChildWindows(hDlg, EnumChildSetFont, (LPARAM)param->hFont);

        // Creation du contr�le ListView
        RECT rcClient, rcTitre, rcLegend;
        GetWindowRect(hDlg, &rcClient);
        AdjustWindowRect(&rcClient, GetWindowStyle(hDlg), FALSE);

        GetWindowRect(GetDlgItem(hDlg,IDC_TITLE), &rcTitre);
        AdjustWindowRect(&rcClient, GetWindowStyle(GetDlgItem(hDlg,IDC_TITLE)), FALSE);
        GetWindowRect(GetDlgItem(hDlg,IDC_LEGEND), &rcLegend);
        int margex = 10;

        HWND hwndLv = CreateWindowEx(WS_EX_CLIENTEDGE,
                WC_LISTVIEW, _T(""), LVS_AUTOARRANGE | LVS_ALIGNTOP | WS_CHILDWINDOW | WS_VISIBLE 
                | LVS_REPORT | WS_BORDER | WS_TABSTOP,
                margex ,
                rcTitre.bottom-rcTitre.top, 
                rcTitre.right-rcTitre.left,
                rcLegend.top- rcTitre.bottom, 
                hDlg, NULL, GetModuleHandle(NULL), NULL); 

        // charge les colonnes du controle listview
        LVInitCol(hwndLv, 120, 230);

        // Cr�e la liste d'images pour le contr�le ListView
        HIMAGELIST hImgLst = ImageList_Create(16, 16, TRUE, 1, 0);

        // Ajout des icones � la liste d'images (d�coche/coche)        
        ImageList_AddIcon(hImgLst, LoadIcon(g_hmodThisDll, MAKEINTRESOURCE(IDI_LSTFILE)));

        // Lie le contr�le ListView � liste d'image
        ListView_SetImageList(hwndLv, hImgLst, LVSIL_SMALL);

        // Ajoute un item � la liste pour chaque fichier de la liste des d�pandances
        CNode	*pEnumNode;
        for(pEnumNode = param->files->Find(NULL);
            pEnumNode != NULL;
            pEnumNode = param->files->Next(pEnumNode)) {
            LV_Add(	hwndLv,
                    0,
                    LPSTR_TEXTCALLBACK,
                    LPSTR_TEXTCALLBACK,
                    0,   // Image
                    0,   // Pas gris�
                    0,   // Pas d'image d'�tat
                    (LPARAM)pEnumNode);
        }
        return TRUE;
    }

    case WM_COMMAND:
        if ( (wParam == IDCANCEL) || (wParam == IDC_BTRETRY) || (wParam == IDC_BTRESTARTNOW) || (wParam == IDC_BTRESTARTLATER) )
        {
            // Ferme la fen�tre et retourne le code du bouton s�lectionn�
            EndDialog(hDlg, wParam);
            SetWindowLong(hDlg, DWLP_MSGRESULT, 0);
            return TRUE;
        }
        break;

    case WM_NOTIFY:
        if( ((LPNMHDR)lParam)->code == LVN_COLUMNCLICK ) {
            LV_ColumnClick( lParam, LV_Compare );
            return TRUE;
        }
        else if( ((LPNMHDR)lParam)->code == LVN_GETDISPINFO ) {
            // Provide the item or subitem's text, if requested. 
            NMLVDISPINFO *pnmv = (NMLVDISPINFO *)lParam;
            if (pnmv->item.mask & LVIF_TEXT) {
                CNode *pItem = (CNode *)pnmv->item.lParam;
                if( pnmv->item.iSubItem )
                    pItem->GetDirectory(pnmv->item.pszText, pnmv->item.cchTextMax);
                else
                    _tcscpy_s(pnmv->item.pszText, pnmv->item.cchTextMax, pItem->m_pszBaseName);
            }
            return TRUE;
        }
        break;
    }

    return FALSE;       // did not process a message 
}
