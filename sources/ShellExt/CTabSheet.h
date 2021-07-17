#pragma once

//////////
// Structures

struct HELPMAPENTRY
{
	DWORD	idCtrl;	// Identificateur du contrôle concerné
	DWORD	idStr;	// Identificateur de la chaîne de caractère contenant le lien internet
};

class CTabSheet
{
public:
	CTabSheet ( void );
	~CTabSheet ( void );

	operator HWND() { return m_hwnd; }

protected:
	//////////////
	// méthodes
	
	// Méthode pour afficher la feuille
	inline void show()
		{ ShowWindow(m_hwnd, SW_SHOW); }

	// Méthode pour cacher la feuille
	inline void hide()
		{ ShowWindow(m_hwnd, SW_HIDE); }

	// Méthode appelée pour déplacer et redimensionner l'onglet
	inline void move(PRECT prc)
		{ SetWindowPos(m_hwnd, NULL, prc->left, prc->top, prc->right-prc->left, prc->bottom-prc->top, SWP_NOZORDER ); }


	// Méthode appelée pour sélectionner l'onglet dans un contrôle tab
	int TabSelect();

	// Méthode appelée pour insérer l'onglet dans la liste des onglets
	// d'un contrôle tab et pour créer la fenêtre correspondante
	void TabInsert(HWND hParent, HWND hTab, PTSTR pszTitle, int iItem, BOOL bVisible);

	// Méthode appelée pour rafraichir le contenu des coontrôles
	// à partir des données de l'objet CShellExt
	virtual void LoadControlData() { }

	// Retourne le handle d'un contrôle à partir de son identificateur
	HWND GetItem(int id)	{ return GetDlgItem(m_hwnd, id); }
	

	/////////////
	// évenements

	virtual BOOL OnInit ( HWND ) { return FALSE; }
	virtual BOOL OnCommand (WORD, WORD, HWND) { return FALSE; }
	virtual BOOL OnDestroy () { return FALSE; }
	virtual BOOL OnNotify(int idCtrl, LPNMHDR pnmh) { return FALSE; }
	virtual BOOL OnHelp ( DWORD );

	// Méthode appelée pour mettre à jour les données de l'objet CShellExt
	// à partir des contrôles de l'onglet
	virtual void SaveControlData() {}


	////////////////
	// Procédure de fenêtre
	
	// procédure de l'onglet
	friend INT_PTR CALLBACK TabSheetProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);

	// procédure de l'onglet virtuel utilisée comme callback
	virtual INT_PTR SheetProc (UINT, WPARAM, LPARAM) { return FALSE; }

	// variables membres
	HWND		m_hwnd;		// Handle de la feuille
	HWND		m_hTab;		// Handle du contrôle tab dans lequel est crée l'onglet							
	WORD		m_wIDTab;	// ID de la resource "dialog" correspondant à cet onglet
	
	friend class CShellExt;
	CShellExt	*m_pcs;		// Pointeur sur l'objet CShellExt à qui appartient cet objet

	BOOL m_iTabIndex;		// index de l'onglet dans le contrôle tab
};

