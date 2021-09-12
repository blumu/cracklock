#pragma once

#include "Timezone.h"	// Fonctions pour la gestion des fuseaux horaires

class COptions : public CTabSheet
{
public:
	COptions();

private:
	CTimeZoneInfoManager m_TimeZoneInfoManager;

	// M�thode appel�e pour rafraichir le contenu des coontr�les
	// � partir des donn�es de l'objet CShellExt
	void LoadControlData();

	// M�thode appel�e pour mettre � jour les donn�es de l'objet CShellExt
	// � partir des contr�les de l'onglet
	void SaveControlData();

	INT_PTR SheetProc(UINT, WPARAM, LPARAM);
	BOOL OnInit( HWND );
	BOOL OnCommand (WORD, WORD, HWND);

	void MajEnableControl();
	void ConfigSpin( HWND );
	void MAJInternational();
	void CheckEditContent( HWND, BOOL );
	void FillTimeField( HWND );
	void PopulateComboTimeZones(HWND hCombo);

    void SetDateTime( const SYSTEMTIME *pst );
	void GetDateTime( PSYSTEMTIME pst );
};

