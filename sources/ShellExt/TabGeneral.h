#pragma once

class CGeneral : public CTabSheet
{
public:
	CGeneral();

private:
	// Méthode appelée pour rafraichir le contenu des coontrôles
	// à partir des données de l'objet CShellExt
	void LoadControlData();

	// Méthode appelée pour mettre à jour les données de l'objet CShellExt
	// à partir des contrôles de l'onglet
	void SaveControlData();

	BOOL OnButtonClick( WORD );

    /*
	INT_PTR SheetProc(UINT, WPARAM, LPARAM);
    BOOL OnCtlColorStatic(HWND, HDC);
    BOOL OnSetCursor();
	BOOL OnLButtonDown(LONG, LONG);*/
	BOOL OnDestroy(); 

    BOOL OnCommand (WORD, WORD, HWND);
    BOOL OnNotify(int idCtrl, LPNMHDR pnmh);
	BOOL OnInit ( HWND );

	HFONT		m_hfntTitle, m_hfntSubTitle;	// Police du titre et sous-titre
	HICON		m_hIcoHorlo;			        // Icône de l'horloge

};

