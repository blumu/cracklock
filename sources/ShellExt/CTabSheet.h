#pragma once

//////////
// Structures

struct HELPMAPENTRY
{
	DWORD	idCtrl;	// Identificateur du contr�le concern�
	DWORD	idStr;	// Identificateur de la cha�ne de caract�re contenant le lien internet
};

class CTabSheet
{
public:
	CTabSheet ( void );
	~CTabSheet ( void );

	operator HWND() { return m_hwnd; }

protected:
	//////////////
	// m�thodes
	
	// M�thode pour afficher la feuille
	inline void show()
		{ ShowWindow(m_hwnd, SW_SHOW); }

	// M�thode pour cacher la feuille
	inline void hide()
		{ ShowWindow(m_hwnd, SW_HIDE); }

	// M�thode appel�e pour d�placer et redimensionner l'onglet
	inline void move(PRECT prc)
		{ SetWindowPos(m_hwnd, NULL, prc->left, prc->top, prc->right-prc->left, prc->bottom-prc->top, SWP_NOZORDER ); }


	// M�thode appel�e pour s�lectionner l'onglet dans un contr�le tab
	int TabSelect();

	// M�thode appel�e pour ins�rer l'onglet dans la liste des onglets
	// d'un contr�le tab et pour cr�er la fen�tre correspondante
	void TabInsert(HWND hParent, HWND hTab, PTSTR pszTitle, int iItem, BOOL bVisible);

	// M�thode appel�e pour rafraichir le contenu des coontr�les
	// � partir des donn�es de l'objet CShellExt
	virtual void LoadControlData() { }

	// Retourne le handle d'un contr�le � partir de son identificateur
	HWND GetItem(int id)	{ return GetDlgItem(m_hwnd, id); }
	

	/////////////
	// �venements

	virtual BOOL OnInit ( HWND ) { return FALSE; }
	virtual BOOL OnCommand (WORD, WORD, HWND) { return FALSE; }
	virtual BOOL OnDestroy () { return FALSE; }
	virtual BOOL OnNotify(int idCtrl, LPNMHDR pnmh) { return FALSE; }
	virtual BOOL OnHelp ( DWORD );

	// M�thode appel�e pour mettre � jour les donn�es de l'objet CShellExt
	// � partir des contr�les de l'onglet
	virtual void SaveControlData() {}


	////////////////
	// Proc�dure de fen�tre
	
	// proc�dure de l'onglet
	friend INT_PTR CALLBACK TabSheetProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);

	// proc�dure de l'onglet virtuel utilis�e comme callback
	virtual INT_PTR SheetProc (UINT, WPARAM, LPARAM) { return FALSE; }

	// variables membres
	HWND		m_hwnd;		// Handle de la feuille
	HWND		m_hTab;		// Handle du contr�le tab dans lequel est cr�e l'onglet							
	WORD		m_wIDTab;	// ID de la resource "dialog" correspondant � cet onglet
	
	friend class CShellExt;
	CShellExt	*m_pcs;		// Pointeur sur l'objet CShellExt � qui appartient cet objet

	BOOL m_iTabIndex;		// index de l'onglet dans le contr�le tab
};

