// CNodes.cpp
// Liste chainee de fichiers CNode
#include "StdAfx.h"
#include "CNodes.h"		// Définition de la classe CNodes

CNode *CNodes::AddItem ( PCTSTR pszFilePath )
{
	CNode *pNewItem = new CNode;

	if( pNewItem != NULL )
	{
		// Insére le nouvel item entre le dernier et le premier élément
		pNewItem->prev = MN.prev;
		pNewItem->next = &MN;
		MN.prev->next = pNewItem;
		MN.prev = pNewItem;

		pNewItem->m_pszFilePath = _tcsdup(pszFilePath);
		
		// Pointeur sur partie comportant que le nom du fichier (sans chemin d'accès)
		pNewItem->m_pszBaseName = GetFileBaseNamePart(pNewItem->m_pszFilePath);

		uCount++;
	}
	return pNewItem;
}

void CNodes::DelItem ( CNode *pDelItem )
{
	if (pDelItem != NULL )
	{		
		pDelItem->next->prev = pDelItem->prev;
		pDelItem->prev->next = pDelItem->next;
		
		if( pDelItem->m_pszFilePath != NULL)
			free (pDelItem->m_pszFilePath);
		delete pDelItem;
		uCount--;
	}
}

CNode *CNodes::Find ( PCTSTR pszSearch )
{
	CNode *pItem;

	if ( pszSearch != NULL )
	{				
		for(pItem = MN.next; pItem != &MN; pItem = pItem->next)
			if( _tcsicmp(pItem->m_pszFilePath, pszSearch) == 0 )
				return pItem;

		return NULL;
	}
	else
		return (MN.next == &MN) ? NULL : MN.next;
}

CNodes::CNodes ()
{
	MN.next = MN.prev = &MN;
	MN.m_pszFilePath = NULL;
	uCount = 0;
}

CNodes::~CNodes (void)
{
	CNode *pItem;
	CNode *pItemNext;

	pItemNext = MN.next;
	while ( pItemNext != &MN)
	{
		pItem = pItemNext;
		pItemNext = pItemNext->next;
		
		if(pItem->m_pszFilePath != NULL)
			free (pItem->m_pszFilePath);	
		free (pItem);
	}
}
