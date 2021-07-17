#pragma once

class CMoreOptions : public CTabSheet
{
public:
	CMoreOptions();

private:
	// Méthode appelée pour rafraichir le contenu des coontrôles
	// à partir des données de l'objet CShellExt
	void LoadControlData();

	// Méthode appelée pour mettre à jour les données de l'objet CShellExt
	// à partir des contrôles de l'onglet
	void SaveControlData();
    
	INT_PTR SheetProc(UINT, WPARAM, LPARAM);
	BOOL OnInit( HWND );
	BOOL OnCommand (WORD, WORD, HWND);
    BOOL OnDestroy();

};

