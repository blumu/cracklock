#pragma once

class CMoreOptions : public CTabSheet
{
public:
	CMoreOptions();

private:
	// M�thode appel�e pour rafraichir le contenu des coontr�les
	// � partir des donn�es de l'objet CShellExt
	void LoadControlData();

	// M�thode appel�e pour mettre � jour les donn�es de l'objet CShellExt
	// � partir des contr�les de l'onglet
	void SaveControlData();
    
	INT_PTR SheetProc(UINT, WPARAM, LPARAM);
	BOOL OnInit( HWND );
	BOOL OnCommand (WORD, WORD, HWND);
    BOOL OnDestroy();

};

