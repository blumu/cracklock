// CNode.cpp
// Classe CNode representant un fichier.

#include "StdAfx.h"

#include "CNode.h"		// D�fition de l'objet CNode
#include "PEtools.h"	// Outils divers

// Constructeur
CNode::CNode( void )
{
	m_bExist =
	m_bCracked =
	m_bCrackable = 
	m_bUsedByAnotherApp = FALSE;
}

// Test si le fichier est un fichier syst�me
bool CNode::IsWinSysFile ()
{
	static TCHAR *WINSYSFILES[] = { CL_SHELLEXT_DLL, CL_KERNEL_DLL, KERNEL32_DLL,
		_T("OLEAUT32.DLL"), _T("OLEPRO32.DLL"), _T("RPCRT4.DLL"), _T("NTDLL.DLL"), _T("ADVAPI32.DLL"), _T("OLE32.DLL"), _T("USER32.DLL"), _T("GDI32.DLL"),
		_T("SHELL32.DLL"), _T("SHLWAPI.DLL"), _T("RSAGNT32.DLL"), _T("COMDLG32.DLL"), _T("COMCTL32.DLL"), _T("WINMM.DLL") };


	// Trouve le d�but du nom de fichier dans le chemin complet
	PCTSTR pszFN = GetFileBaseNamePart(m_pszFilePath);

	// Compare avec les �l�ments de la liste des fichiers syst�mes
	for( int i = 0; i < sizeof(WINSYSFILES)/sizeof(PCTSTR); i++)
		if( _tcsicmp(WINSYSFILES[i], pszFN) == 0 )
			return true;
		
	return false;
}

// Crack ou d�crack un fichier.
// Retourne true si �a c'est bien termin�, sinon retourne false
// REMARQUE: si bRestart vaut true, alors le crackage se fait sur un
// fichier temporaire qui sera copier automatiquement apr�s un reboot.
bool CNode::RefreshCrack (bool bCrack, bool bAfterRestart)
{
	TCHAR	szFileTMP[_MAX_PATH], szFileDir[_MAX_PATH];
	PTSTR	pszFileDest;
	FILETIME ftCreation, ftLastAccess, ftLastWrite;

	// sauvegardes les dates du fichier avant modification
	GetFileTimeByName(m_pszFilePath, &ftCreation, &ftLastAccess, &ftLastWrite);

	if(	bAfterRestart &&
		GetDirectory(szFileDir, _MAX_PATH) &&
		GetTempFileName(szFileDir, _T("CKL"), 0, szFileTMP) )
	{
		// Effectue une copie temporaire du fichier		
		if( CopyFile(m_pszFilePath, szFileTMP, FALSE) )
		{
			// Attributs normaux pour le fichier temporaire
			SetFileAttributes(szFileTMP, FILE_ATTRIBUTE_NORMAL);
            CopyFileReboot(szFileTMP, m_pszFilePath, true);
			pszFileDest = szFileTMP;
		}
		else
			pszFileDest = m_pszFilePath;
	}
	else
		pszFileDest = m_pszFilePath;


	// Si le fichier est sur le point d'�tre crack� alors
	// effectue une sauvegarde du fichier
	if( bCrack )
		BackupFile();

	// Si le fichier est sur le point d'�tre d�crack� alors
	// restaure une �ventuelle sauvegarde du fichier
	if( !bCrack && RestoreFile(pszFileDest) )
	{
		m_bCracked = bCrack;
		return true;
	}
	
	// Modifie la table d'import du fichier PE
	if( PERemplaceImportName(pszFileDest, bCrack ? KERNEL32_DLL_A : CL_KERNEL_DLL_A, bCrack ? CL_KERNEL_DLL_A : KERNEL32_DLL_A) )
	{
		// r�cup�re les date du fichier avant les modifications
		SetFileTimeByName(pszFileDest, &ftCreation, &ftLastAccess, &ftLastWrite);

		m_bCracked = bCrack;
		return true;
	}

	return false;
}

// Obtient le nom du repertoire (sans le nom du fichier) et retourne le dans pszDir
bool CNode::GetDirectory (PTSTR pszDir, WORD wMaxSize)
{
    if( EINVAL == _tcsncpy_s(pszDir, wMaxSize, m_pszFilePath, m_pszBaseName-m_pszFilePath) )
        return false;

    // Cas particulier ci le fichier se trouve dans la racine
    if( pszDir[m_pszBaseName-m_pszFilePath-2] == ':' )
    {
        // rajoute l'anti-slash � la fin
        pszDir[m_pszBaseName-m_pszFilePath-1] = '\\';
        pszDir[m_pszBaseName-m_pszFilePath] = '\0';
    }

    return true;
}


// Obtient le nom du fichier de sauvegarde
bool CNode::GetBackupFileName ( PTSTR pszBKFN, DWORD chSize )
{
	if( EINVAL == _tcscpy_s(pszBKFN, chSize, m_pszFilePath) )
		return false;

	PTSTR  pszExt = GetFileExtPart(pszBKFN, NULL, NULL);

	// Remplace l'extension par l'extension de sauvegarde
	if( EINVAL == _tcscpy_s(pszExt-1, chSize-(pszExt-pszBKFN), BACKUP_EXT) )
		return false;

	return true;

}

// Effectue une sauvegarde du fichier
bool CNode::BackupFile ( void )
{
	
	bool bRet = false;
	TCHAR  szBKFile[_MAX_PATH];
	if( GetBackupFileName(szBKFile, _MAX_PATH) )
	{
    	FILETIME ftCreation, ftLastAccess, ftLastWrite;

		// sauvegarde les dates du fichier d'origine
		GetFileTimeByName(m_pszFilePath, &ftCreation, &ftLastAccess, &ftLastWrite);
	
		// copie le fichier sous l'extension de sauvegarde
        bRet = CopyFile( m_pszFilePath, szBKFile, TRUE ) ? true : false;

		// r�cup�re les dates du fichier d'origine pour le fichier backup
		SetFileTimeByName(szBKFile, &ftCreation, &ftLastAccess, &ftLastWrite);
	}

	return bRet;
}

// Restaure la sauvegarde du fichier si elle existe
bool CNode::RestoreFile ( PTSTR pszDest )
{
    TCHAR  szBKFile[_MAX_PATH];
    FILETIME ftCreation, ftLastAccess, ftLastWrite;

    bool bRet = false;
    if( GetBackupFileName(szBKFile, _MAX_PATH) )
    {
	    // sauvegarde les dates du fichier backup
	    GetFileTimeByName( szBKFile, &ftCreation, &ftLastAccess, &ftLastWrite);

	    // restaure le fichier de sauvegarde
        bRet = MoveFileEx( szBKFile, pszDest, MOVEFILE_REPLACE_EXISTING |  MOVEFILE_COPY_ALLOWED ) ? true : false;

	    // r�cup�re les dates du fichier bakcup
	    SetFileTimeByName( pszDest, &ftCreation, &ftLastAccess, &ftLastWrite);

        return bRet; // la restauration du backup a marche
	}

	return false;
}


// Description	    : �num�re la liste des applications � partir de la registry
//					  et v�rifie s'il existe encore au moins une r�f�rence
//					  au fichier demand� afin de d�terminer si il est encore
//					  utile de le crack�.
//
// Retourne true si le fichier est cracke pour une autre application.
// Argument         : PSTR pszExclude
//						nom d'une application � exclure lors de la recherche.
//						(Ceci permet de voir si une DLL a besoin d'�tre crack�e
//						pour un autre programme que le courrant).
//
//						Si pszExclude vaut NULL alors aucun programme n'est exclus
//						de la recherche.
//
bool CNode::IsUsedByAnotherApp( PTSTR pszExclude )
{
	TCHAR	szKeyName[_MAX_PATH];
	TCHAR	szExcludeKey[_MAX_PATH];
	TCHAR	szLoaderFile[_MAX_PATH];
	DWORD	dwFlags;

	// Fait deux fois la recherche, une fois pour les applications crack�es
	// et une autres pour les DLLs crack�es
	for(int j=0; j<=1; j++) {
		// j = 0 : Ouvre la cl� comportant la liste des APPLICATIONS crack�s
		// j = 1 : Ouvre la cl� comportant la liste des DLLS crack�s
		CLCommonSettingSection secApps;
		if( !secApps.Open( j ? REGKEY_MBCL_DLLS : REGKEY_MBCL_APPS) )
			continue;

		// Obtient la cl� qui correspond au fichier � exclure
		if( !secApps.SubsectionNameFromApp(pszExclude, szExcludeKey, _countof(szExcludeKey)) )
			szExcludeKey[0] = '\0';

		for(secApps.InitEnumSubsection();
			secApps.EnumSubsection(szKeyName, _countof(szKeyName)); )
		{				
			// Si le fichier est celui qui est exclus alors on ne le comptabilise pas !
			if( _tcsicmp(szExcludeKey, szKeyName) == 0 )
				continue;
			
			if( !FileExists(szKeyName) )
				continue;

			// Ouvre la cl� du fichier courrament explor�
			SettingSection *secApp = secApps.GetAppSubsectionForward(szKeyName);
			if( secApp ) {
				secApp->GetValue(REGVALUE_PROG_FLAGS, &dwFlags);

				// regarde s'il fait r�f�rence au fichier demand�
				if( secApp->GetValue(REGVALUE_PROG_LOADERFILE, szLoaderFile, _countof(szLoaderFile))
					  && !_tcsicmp(szLoaderFile, m_pszFilePath) )
					return true;
			}

		}	
	}
	return false;
}
