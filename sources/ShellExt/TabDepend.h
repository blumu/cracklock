// TabDepend.h
#pragma once

void LVInitCol (HWND hwnd, int cx1, int cx2);

class CDepend : public CTabSheet
{
public:
    CDepend();

	/*
    INT_PTR SheetProc(UINT uMessage, WPARAM wParam, LPARAM lParam);
    BOOL OnCtlColorStatic(HWND, HDC);
	BOOL OnSetCursor();
	BOOL OnLButtonDown(LONG, LONG);*/
    
    // Redimensionne les controles
    void SetControlsSize();

    // M�thode appel�e pour rafraichir le contenu des coontr�les
    // � partir des donn�es de l'objet CShellExt
    void LoadControlData();

    // M�thode appel�e pour mettre � jour les donn�es de l'objet CShellExt
    // � partir des contr�les de l'onglet
    void SaveControlData();

    // Grise/degrise les controls de la page tab en fonctions des selections faite par l'utilisateur
    void RefreshControl();

private:
    BOOL OnInit( HWND );
    BOOL OnDestroy();
    BOOL OnNotify(int idCtrl, LPNMHDR pnmh);
    BOOL OnCommand(WORD, WORD, HWND );
    BOOL LVSelectItem( int iItem );

    HWND		m_hwndLV;		// handle contr�le ListView
    HIMAGELIST	m_hImgLst,		// handle de la liste d'images
                m_hImgLstSt;	// handle de la liste d'images d'�tat
    TCHAR		*m_S_ERRMSG;	// message d'erreur
    TCHAR		*m_S_LDRNAME;	// Nom de l'item loader
    TCHAR		*m_S_LDRDESC;	// Description de l'item loader
};

