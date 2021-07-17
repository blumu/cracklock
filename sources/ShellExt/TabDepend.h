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

    // Méthode appelée pour rafraichir le contenu des coontrôles
    // à partir des données de l'objet CShellExt
    void LoadControlData();

    // Méthode appelée pour mettre à jour les données de l'objet CShellExt
    // à partir des contrôles de l'onglet
    void SaveControlData();

    // Grise/degrise les controls de la page tab en fonctions des selections faite par l'utilisateur
    void RefreshControl();

private:
    BOOL OnInit( HWND );
    BOOL OnDestroy();
    BOOL OnNotify(int idCtrl, LPNMHDR pnmh);
    BOOL OnCommand(WORD, WORD, HWND );
    BOOL LVSelectItem( int iItem );

    HWND		m_hwndLV;		// handle contrôle ListView
    HIMAGELIST	m_hImgLst,		// handle de la liste d'images
                m_hImgLstSt;	// handle de la liste d'images d'état
    TCHAR		*m_S_ERRMSG;	// message d'erreur
    TCHAR		*m_S_LDRNAME;	// Nom de l'item loader
    TCHAR		*m_S_LDRDESC;	// Description de l'item loader
};

