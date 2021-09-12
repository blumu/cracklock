#pragma once

class CGeneral : public CTabSheet
{
public:
	CGeneral();

private:
	// M�thode appel�e pour rafraichir le contenu des coontr�les
	// � partir des donn�es de l'objet CShellExt
	void LoadControlData();

	// M�thode appel�e pour mettre � jour les donn�es de l'objet CShellExt
	// � partir des contr�les de l'onglet
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
	HICON		m_hIcoHorlo;			        // Ic�ne de l'horloge

};

