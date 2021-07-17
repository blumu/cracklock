#pragma once

#include "Timezone.h"	// Fonctions pour la gestion des fuseaux horaires

class COptions : public CTabSheet
{
public:
	COptions();

private:
	CTimeZoneInfoManager m_TimeZoneInfoManager;

	// Méthode appelée pour rafraichir le contenu des coontrôles
	// à partir des données de l'objet CShellExt
	void LoadControlData();

	// Méthode appelée pour mettre à jour les données de l'objet CShellExt
	// à partir des contrôles de l'onglet
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

