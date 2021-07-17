#pragma once

class CHelp : public CTabSheet
{
public:
	CHelp();

protected:
	BOOL OnInit( HWND );
	BOOL OnDestroy();
	BOOL OnCommand (WORD wNotifyCode, WORD wID, HWND hwndCtl);

	HICON m_hIcon[6];	// handles des icônes de la légende
};

