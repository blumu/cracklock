// CNodes.h
#pragma once

// Inclus le fichier de d�nition de l'objet node
#include "CNode.h"

// D�finition de base d'un objet nodes
class CNodes
{
public:
	CNodes ();
	~CNodes ();

	CNode *AddItem ( PCTSTR pszFilePath );
	void DelItem ( CNode *pDelItem );

	UINT GetItemCount ( void ) { return uCount; }
	
	CNode *Find ( PCTSTR pszSearch );
	CNode *Next ( CNode *pItem ) { return (pItem->next==&MN) ? NULL : (pItem->next); }
	CNode *Previous ( CNode *pItem ) { return (pItem->prev==&MN) ? NULL : (pItem->prev); }
	
private:
	CNode		MN;
	UINT		uCount;
};

