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

  // Valeur vrai si le fichier est actuellement cracké
  bool	m_bCracked;

  // Valeur vrai si le fichier peut être cracké
  bool	m_bCrackable;

  // Valeur vrai si au moins un autre programme nécessite que ce fichier soit cracké
  bool	m_bUsedByAnotherApp;

  // Nom de l'objet, ici il correspond au chemin d'accès au fichier
  // Par exemple un pointeur sur C:\MACHIN\TRUC.EXE
  PTSTR	m_pszFilePath;
  
  // Pointeur dans m_pszName sur la partie contenant juste le nom du fichier sans le chemin d'acces
  // e.g. si m_pszName pointe sur "C:\toto\truc.exe" alors m_pszBaseName pointe sur "truc.exe"
  PCTSTR m_pszBaseName;

  // Objet suivant du chaînage
  CNode	*prev;

  // Objet précèdant du chaînage
  CNode	*next;
};

////////////
// Macros 

// Test si un fichier exécutable peut être cracké en vérifiant que 
// l'entrée KERNEL32.DLL est bien présente dans la table d'imports
#define IsFileCrackable( hFile )    (LookImportTable(hFile, KERNEL32_DLL_A)!=NULL)

// Test si un fichier exécutable est cracké en vérifiant que 
// l'entrée CLKERN.DLL est bien présente dans la table d'imports
#define IsFileCracked( hFile )      (LookImportTable(hFile, CL_KERNEL_DLL_A)!=NULL)


