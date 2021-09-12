// CNode.h
#pragma once

// Classe representant un fichier
class CNode
{
public:	
	CNode ( void );
	bool IsWinSysFile ( void );
	bool IsDll ( void ) ;
	bool IsUsedByAnotherApp( PTSTR pszExclude );
    bool RefreshCrack (bool bCrack, bool bAfterRestart = false);
    bool GetBackupFileName ( PTSTR pszBKFN, DWORD chSize );
    bool GetDirectory(PTSTR pszDir, WORD wMaxSize);
	bool BackupFile ( void );
	bool RestoreFile ( PTSTR pszDest );

public:
  
  // Valeur vrai si le fichier existe
  bool  m_bExist;

  // Valeur vrai si le fichier est actuellement crack�
  bool	m_bCracked;

  // Valeur vrai si le fichier peut �tre crack�
  bool	m_bCrackable;

  // Valeur vrai si au moins un autre programme n�cessite que ce fichier soit crack�
  bool	m_bUsedByAnotherApp;

  // Nom de l'objet, ici il correspond au chemin d'acc�s au fichier
  // Par exemple un pointeur sur C:\MACHIN\TRUC.EXE
  PTSTR	m_pszFilePath;
  
  // Pointeur dans m_pszName sur la partie contenant juste le nom du fichier sans le chemin d'acces
  // e.g. si m_pszName pointe sur "C:\toto\truc.exe" alors m_pszBaseName pointe sur "truc.exe"
  PCTSTR m_pszBaseName;

  // Objet suivant du cha�nage
  CNode	*prev;

  // Objet pr�c�dant du cha�nage
  CNode	*next;
};

////////////
// Macros 

// Test si un fichier ex�cutable peut �tre crack� en v�rifiant que 
// l'entr�e KERNEL32.DLL est bien pr�sente dans la table d'imports
#define IsFileCrackable( hFile )    (LookImportTable(hFile, KERNEL32_DLL_A)!=NULL)

// Test si un fichier ex�cutable est crack� en v�rifiant que 
// l'entr�e CLKERN.DLL est bien pr�sente dans la table d'imports
#define IsFileCracked( hFile )      (LookImportTable(hFile, CL_KERNEL_DLL_A)!=NULL)


