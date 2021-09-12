#include "StdAfx.h"
#include <stddef.h>		// D�finitions standard de C
#include "petools.h"	// Outils d'exploration de fichier PE

// �crit des donn�es � une position precise dans un fichier d�j� ouvert
DWORD WriteFileAt( HANDLE hFile, PVOID pData, DWORD dwOffset, DWORD dwDataSize )
{
	DWORD dwBytesWritten;

	if(SetFilePointer(hFile, dwOffset, 0, FILE_BEGIN) == 0xFFFFFFFF)
		return 0;

	if(!WriteFile(hFile, (PVOID)pData, dwDataSize, &dwBytesWritten, NULL))
		return 0;

	return dwBytesWritten;
}

// Lit des donn�es � une position precise dans un fichier d�j� ouvert
DWORD ReadFileAt( HANDLE hFile, PVOID pData, DWORD dwOffset, DWORD dwDataSize )
{
	DWORD dwBytesRead;

	if(SetFilePointer(hFile, dwOffset, 0, FILE_BEGIN) == 0xFFFFFFFF)
		return 0;

	if(!ReadFile(hFile, (PVOID)pData, dwDataSize, &dwBytesRead, NULL))
		return 0;

	return dwBytesRead;
}

WORD PEMAGICOFFSET( HANDLE hFile )
{
	WORD  wPEMagicOffset = 0;

	// Lit l'adresse de la signature PE
	ReadFileAt( hFile, (PVOID)&wPEMagicOffset, offsetof(IMAGE_DOS_HEADER, e_lfanew), sizeof(WORD));

	return wPEMagicOffset;
}

// Retourne la signature du fichier
DWORD ImageType (HANDLE hFile, PWORD pwCharacteristics)
{
	IMAGE_DOS_HEADER idh;
	DWORD dwMagic;
	DWORD dwBytesRead;

	// Lit le header DOS
	if (!ReadFile(hFile, (PVOID)&idh, sizeof(IMAGE_DOS_HEADER), &dwBytesRead, NULL))
		return 0;

	// Image avec signature DOS ?
	if (idh.e_magic == IMAGE_DOS_SIGNATURE)
	{    
		// Lit la signature
		ReadFileAt( hFile, (PVOID)&dwMagic, idh.e_lfanew, sizeof(DWORD));

		// Image avec signature OS2 ?
		if( (WORD)dwMagic == IMAGE_OS2_SIGNATURE ||
			(WORD)dwMagic == IMAGE_OS2_SIGNATURE_LE)
			return dwMagic;
		else if( dwMagic == IMAGE_NT_SIGNATURE )
		{
			if( pwCharacteristics )
				// Lit les caract�ristiques du fichier PE
				ReadFileAt( hFile, (PVOID)pwCharacteristics, idh.e_lfanew + sizeof(idh.e_lfanew) + FIELDOFFSET(IMAGE_FILE_HEADER, Characteristics), sizeof(WORD));

			return IMAGE_NT_SIGNATURE;
		}
		else  
			return IMAGE_DOS_SIGNATURE;
	}
	else
		return 0;
}

// Retourne le nombre de sections dans le module
WORD NumOfSections (HANDLE hFile)
{
	WORD wNbSections;
	// Lit le nombre de sections dans le PE Header
	if( !ReadFileAt( hFile,
					(PVOID)&wNbSections,
					PEHDROFFSET(hFile) + offsetof(IMAGE_FILE_HEADER, NumberOfSections),
					sizeof(WORD)) )
		return FALSE;
	return wNbSections;
}

// Obtient le header d'une section identifi�e par son nom
BOOL GetSectionHdrByName (HANDLE hFile, PTSTR pszSection, PIMAGE_SECTION_HEADER psh)
{
	WORD		nSections = NumOfSections (hFile);
	DWORD		Section_Header_Offset = SECHDROFFSET(hFile);
	int			i;

	// localise la section � partir de son nom
	for (i=0; i<nSections; i++)
	{
		// Lit le section header qui suit
		if( !ReadFileAt( hFile,
						(PVOID)psh,
						Section_Header_Offset,
						sizeof(IMAGE_SECTION_HEADER)) )
			return FALSE;

		if (!_tcscmp ((PTSTR)psh->Name, pszSection))
			return TRUE;

		Section_Header_Offset += sizeof(IMAGE_SECTION_HEADER);
	}

	return FALSE;
}

// Retourne l'offset o� se trouve l'entr�e IMAGE_DIRECTORY sp�cifi�e
DWORD ImageDirectoryOffset (HANDLE hFile, DWORD dwIMAGE_DIRECTORY, PIMAGE_SECTION_HEADER psh)
{
	IMAGE_OPTIONAL_HEADER	oh;

	WORD		nSections = NumOfSections (hFile);
	DWORD		Section_Header_Offset = SECHDROFFSET(hFile);
	DWORD		RVAImageDir;
	int			i;

	// Lit le header optionnel
	if( !ReadFileAt(hFile,
					(PVOID)&oh,
					OPTHDROFFSET(hFile),
					sizeof(IMAGE_OPTIONAL_HEADER)) )
		return NULL;

	// dwIMAGE_DIRECTORY doit �tre compris entre 0 et (NumberOfRvaAndSizes-1)
	if (dwIMAGE_DIRECTORY >= oh.NumberOfRvaAndSizes)
		return NULL;

	// localise le r�pertoire demand� (RVA)
	RVAImageDir = oh.DataDirectory[dwIMAGE_DIRECTORY].VirtualAddress;

	// localise la section contenant le r�pertoire
	for (i=0; i<nSections; i++)
	{
		// Lit le section header qui suit
		if( !ReadFileAt( hFile,
						(PVOID)psh,
						Section_Header_Offset,
						sizeof(IMAGE_SECTION_HEADER)) )
			return NULL;

		if ( (psh->VirtualAddress <= (DWORD)RVAImageDir) &&
			((psh->VirtualAddress + psh->SizeOfRawData) > (DWORD)RVAImageDir) )
			// retourne l'adresse de la premi�re entr�e dans le r�pertoire
			return psh->PointerToRawData + (RVAImageDir - psh->VirtualAddress);

		Section_Header_Offset += sizeof(IMAGE_SECTION_HEADER);
	}

	return NULL;
}

///////////////////////
//
//	FONCTION
//		Copie dans un tableau les offsets de tous les noms des fichiers import�s
//		par un fichier EXE.
//
//	ENTR�E:
//		hFile			->	handle du fichier � explorer
//		pdwModules		->	pointeur sur un tableau qui va �tre rempli
//							avec les offsets des noms des modules import�s
//		dwMaxModules	->	Nombre maximum de modules � �crire dans le tableau
//
//	SORTIE:
//		int	-> nombre de modules import�s par le fichier EXE demand�
//
/*
int GetImportModuleNamesOffsets (HANDLE hFile, PDWORD pdwModules, DWORD dwMaxModules)
{
	IMAGE_SECTION_HEADER			ish;
	IMAGE_IMPORT_MODULE_DIRECTORY	iid;

	DWORD		ImportEntry_Offset = ImageDirectoryOffset(hFile, IMAGE_DIRECTORY_ENTRY_IMPORT, &ish);
	PDWORD		pCurOffset = pdwModules;
	DWORD		dwCnt = 0;

	// Extrait tous les offsets des noms des modules import�s
	do
	{
		// Lit l'entr�e suivante dans le r�pertoire d'import
		if( !ReadFileAt( hFile,
						(PVOID)&iid,
						ImportEntry_Offset,
						sizeof(IMAGE_IMPORT_MODULE_DIRECTORY)) )
			return NULL;

		// �crit l'offset du nom de module
		*pCurOffset = ish.PointerToRawData + (iid.dwRVAModuleName-ish.VirtualAddress);

		// Incr�mente � la prochaine entr�e du r�pertoire d'import
		ImportEntry_Offset += sizeof(IMAGE_IMPORT_MODULE_DIRECTORY);
		pCurOffset++;
		dwCnt++;
	}
	while (iid.dwRVAModuleName || (dwCnt >= dwMaxModules));

	return dwCnt;
}
*/

///////////////////////
//
//	FONCTION
//		�num�re la liste des importations d'un EXE en appelant une routine
//		utilisateur pour chaque entr�e dans la table d'import du fichier EXE
//
//	ENTR�E:
//		hFile			->	handle du fichier � explorer
//		pEnumFunction	->	pointeur sur la fonction � appeler pour
//							chacune des importations du EXE
//		lparam			->	1er param�tre � passer � la fonction d'enum�ration
//		lparam			->	2�me param�tre � passer � la fonction d'enum�ration
//
//	SORTIE:
//		int	-> nombre de modules import�s par le fichier EXE demand�
//
int EnumImportModuleNames (HANDLE hFile, PENUMIMPFNC pEnumFunction, LPARAM lParam, LPARAM lParam2)
{
  IMAGE_SECTION_HEADER          ish;
  IMAGE_IMPORT_MODULE_DIRECTORY iid;

  DWORD   ImportEntry_Offset = ImageDirectoryOffset(hFile, IMAGE_DIRECTORY_ENTRY_IMPORT, &ish);
  DWORD   dwCnt = 0;
  char    szImpMod[_MAX_FNAME]; // char et non pas TCHAR!!!

	// Si la table d'import n'existe pas alors sort de la fonction
	if( ImportEntry_Offset == NULL )
		return NULL;
  
	// Lit la premi�re entr�e dans le r�pertoire d'import
	if( !ReadFileAt(hFile,
        					(PVOID)&iid,
				        	ImportEntry_Offset,
						      sizeof(IMAGE_IMPORT_MODULE_DIRECTORY)) )
		return NULL;
	
	// Extrait tous les noms des modules import�s
	while (iid.dwRVAModuleName)
	{
		// Lit le nom du module import�
		if( !ReadFileAt( hFile,
						(PVOID)&szImpMod,
						ish.PointerToRawData + (iid.dwRVAModuleName-ish.VirtualAddress),
						_MAX_FNAME) )
			return NULL;

		// Appel de la fonction d'�numeration
		if( !pEnumFunction(szImpMod,
							ish.PointerToRawData + (iid.dwRVAModuleName-ish.VirtualAddress),
							lParam,
							lParam2) )
			break;

	// Incr�mente � la prochaine entr�e du r�pertoire d'import
		ImportEntry_Offset += sizeof(IMAGE_IMPORT_MODULE_DIRECTORY);
		dwCnt++;

    // Lit l'entr�e suivante dans le r�pertoire d'import
		if( !ReadFileAt(hFile,
        						(PVOID)&iid,
				        		ImportEntry_Offset,
						        sizeof(IMAGE_IMPORT_MODULE_DIRECTORY)) )
			return NULL;
  }

	return dwCnt;
}


BOOL CALLBACK EnumTestSubFonction (PSTR pszName, DWORD dwOffset, LPARAM lParam, LPARAM lParam2)
{
	if( _strcmpi((PSTR)lParam, pszName) == 0 )	
	{
		// module trouv�
		*((PDWORD)lParam2) = dwOffset;
		return FALSE; // arr�te l'�num�ration
	}

	return TRUE; // continue l'�numeration
}

// Renvoi l'adresse dans le fichier o� figure l'importation recherch�e
// NULL si elle ne se trouve pas dans la table d'import
DWORD LookImportTable( HANDLE hFile, PCSTR pszSrchMod )
{
	DWORD		dwRet = 0;
	
	EnumImportModuleNames (hFile, EnumTestSubFonction, (LPARAM)pszSrchMod, (LPARAM)&dwRet);
	
	return dwRet;
}


/*
///////////////////////
//
//	FONCTION
//		�num�re la liste des fonctions d'un module import�es par le EXE
//		en faisant appelle � une routine utilisateur pass�e en param�tre
//
//	ENTR�E:
//		hFile			->	handle du fichier � explorer
//		pEnumFunction	->	pointeur sur la fonction � appeler pour
//							chacune des fonctions du module sp�cifi�
//		lparam			->	param�tre � passer � la fonction d'enum�ration
//
//	SORTIE:
//		int	-> nombre de fonctions du module sp�cifi� import�es par le fichier EXE
int EnumImportFunctionByModule (HANDLE hFile, PCTSTR pszModule, PENUMIMPFNC pEnumFunction, LPARAM lParam, LPARAM lParam2)
{
	IMAGE_SECTION_HEADER			ish;
	IMAGE_IMPORT_MODULE_DIRECTORY	iid;
	IMPORTENUMPARAM					iep;

	DWORD		ImportEntry_Offset = ImageDirectoryOffset(hFile, IMAGE_DIRECTORY_ENTRY_IMPORT, &ish);
	DWORD		dwCurFntNamePtrOffset;
	DWORD		dwCnt = 0;
	char		szImpMod[_MAX_FNAME];
	char		szFnt[256];

	// locate section header for ".idata" section
	if (!GetSectionHdrByName (hFile, ".idata", &ish))
		return 0;

	// Extrait tous les noms des modules import�s
	iep.lParam = lParam;
	iep.lParam2 = lParam2;
	do
	{
		// Lit l'entr�e suivante dans le r�pertoire d'import
		if( !ReadFileAt( hFile,
						(PVOID)&iid,
						ImportEntry_Offset,
						sizeof(IMAGE_IMPORT_MODULE_DIRECTORY)) )
			return 0;

		// Lit le nom du module import�
		if( !ReadFileAt( hFile,
						(PVOID)&szImpMod,
						ish.PointerToRawData + (iid.dwRVAModuleName-ish.VirtualAddress),
						_MAX_FNAME) )
			return 0;

		if( _tcsicmp(pszModule, szImpMod) == 0)
		{

			if( iid.dwRVAFunctionNameList == 0)
				return 0;

			dwCurFntNamePtrOffset = ish.PointerToRawData + (iid.dwRVAFunctionNameList-ish.VirtualAddress);
			while(1)
			{

				// Lit l'adresse du nom de la fonction import�e
				if( !ReadFileAt( hFile,
							(PVOID)&iep.dwOffset,
							dwCurFntNamePtrOffset,
							sizeof(DWORD)) )
					break;

				if ( iep.dwOffset == 0 )
					break;

				if( !(iep.dwOffset & 0x80000000) )
				{
					iep.dwOffset += 4;

					// Lit le nom de la fonction import�e
					if( !ReadFileAt( hFile,
									(PVOID)&szFnt,
									iep.dwOffset,
									256) )
						break;

					if ( szImpMod == '\0' )
						break;

					// Appel de la fonction d'�numeration
					iep.pszName = szFnt;
					pEnumFunction( &iep );
				}
				
				dwCurFntNamePtrOffset += sizeof(DWORD);
				dwCnt++;
			}			

			return dwCnt;
		}

		// Incr�mente � la prochaine entr�e du r�pertoire d'import
		ImportEntry_Offset += sizeof(IMAGE_IMPORT_MODULE_DIRECTORY);
	}
	while (iid.dwRVAModuleName);

	return 0;
}

// Renvoi l'adresse dans le fichier o� figure le nom de la fonction import�e recherch�e
// NULL si elle ne s'y trouve pas
DWORD IsFunctionUsed( HANDLE hFile, PCSTR pszSrchMod, PCSTR pszSrchFnt )
{
	DWORD		dwRet = 0;
	
	EnumImportFunctionByModule (hFile, pszSrchMod, EnumTestSubFonction, (LPARAM)pszSrchFnt, (LPARAM)&dwRet);
	
	return dwRet;
}
*/





DWORD ComputePEFileRealChecksum( PCTSTR pszFileName )
{
    HANDLE	hFile;

    // Ouvre le fichier
    if( (hFile = CreateFile(	pszFileName,
        GENERIC_READ ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL )) == INVALID_HANDLE_VALUE)
        return FALSE;

        DWORD chk = PEComputeRealChecksum ( hFile );

    CloseHandle( hFile );

    return chk;
}

// Change le nom d'une DLL importee par un fichier PE
BOOL PERemplaceImportName( PCTSTR pszFileName, PCSTR pszPattern, PCSTR pszReplace )
{
    BOOL   ret = FALSE;

    // Ouvre le fichier
    HANDLE  hFile;
    if( (hFile = CreateFile(	pszFileName,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL )) == INVALID_HANDLE_VALUE)
        return FALSE;

    DWORD dwOffset = LookImportTable(hFile, pszPattern);
    if( dwOffset != 0 )
    {
        ret = WriteFileAt(hFile, (PVOID)pszReplace, dwOffset, (DWORD)strlen(pszReplace)+1) > 0;

        /////////////
        //// code added on 15 sep 2007
        //// Delete the "bound import table" directory if it exists
        DeleteDirectory(hFile, IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT);
        ////////////

        // Remplace l'ancienne checksum (calculee par l'editeur de lien link.exe) dans le header du fichier PE par une nouvelle checksum calculee a partir du fichier
        PEPatchLinkchecksum( hFile );
    }

    // Ferme le fichier et retourne � l'appelant
    CloseHandle( hFile );

    return ret;
}

// Supprime une directory de la section IMAGE_OPTIONAL_HEADER
BOOL DeleteDirectory (HANDLE hFile, DWORD dwIMAGE_DIRECTORY)
{
    // Lit le header optionnel
    IMAGE_OPTIONAL_HEADER	oh;
    if( !ReadFileAt(hFile,
                    (PVOID)&oh,
                    OPTHDROFFSET(hFile),
                    sizeof(IMAGE_OPTIONAL_HEADER)) )
        return FALSE;

    // dwIMAGE_DIRECTORY doit �tre compris entre 0 et (NumberOfRvaAndSizes-1)
    if (dwIMAGE_DIRECTORY >= oh.NumberOfRvaAndSizes)
        return TRUE;

    oh.DataDirectory[dwIMAGE_DIRECTORY].VirtualAddress = 0;
    oh.DataDirectory[dwIMAGE_DIRECTORY].Size = 0;

    // Lit le header optionnel
    if( !WriteFileAt(hFile,
                    (PVOID)&oh,
                    OPTHDROFFSET(hFile),
                    sizeof(IMAGE_OPTIONAL_HEADER)) )
        return FALSE;

    return TRUE;
}

// Calcul la checksum reelle du fichier
DWORD PEComputeRealChecksum( HANDLE hFile )
{
    static IMAGE_OPTIONAL_HEADER	oh;
    DWORD chksumOffset = OPTHDROFFSET(hFile) + (DWORD)((PBYTE)&oh.CheckSum-(PBYTE)&oh);

    DWORD sum = 0;
    BYTE buffer[MAX_BUFFER_SIZE];
    DWORD dwBytesRead;
    DWORD dwCurrentViewOffset = 0;

    if(SetFilePointer(hFile, 0, 0, FILE_BEGIN) == 0xFFFFFFFF)
        return 0;

    BOOL bSuccess = ReadFile(hFile, (PBYTE)&buffer, sizeof(buffer), &dwBytesRead, NULL);
    while( bSuccess && dwBytesRead > 0 ) {
        // Check that the current chunk of data does not contain 
        // one of the 4 bytes of the link checksum.from the PE header
        // If it does then then overwrite the bytes with zeros.
        // (this is necessary to prevent the old checksum to take part
        //  in the new checksum computation)
        DWORD dwCheckInView = chksumOffset;
        for(int i=0; i<sizeof(DWORD);i++)
            if( dwCheckInView >= dwCurrentViewOffset
                && (dwCheckInView < dwCurrentViewOffset+dwBytesRead ))
                buffer[(dwCheckInView++)-dwCurrentViewOffset] = 0;

        dwCurrentViewOffset += dwBytesRead;

        PBYTE pCur = buffer;
        while( dwBytesRead > 1 ) {
            sum += *((PWORD)pCur);
            pCur += sizeof(WORD);
            //while(sum>>16)
            sum = (sum>>16) + (sum&0xFFFF);
            
            dwBytesRead -= sizeof(WORD);
        }
        if( dwBytesRead > 0 ) {
            sum += *((PBYTE)pCur);
            //while(sum>>16)
            sum = (sum>>16) + (sum&0xFFFF);
        }

        bSuccess = ReadFile(hFile, buffer, sizeof(buffer), &dwBytesRead, NULL);
    }

    return sum + GetFileSize(hFile, NULL);
}

// Remplace l'ancienne checksum (calculee par l'editeur de lien link.exe) dans le header du fichier PE par une nouvelle checksum calculee a partir du fichier
BOOL PEPatchLinkchecksum( HANDLE hFile )
{
    // calcul l'offset dans le fichier ou est stockee la checksum calculee par l'editeur de liens (link.exe)
    static IMAGE_OPTIONAL_HEADER	oh;
    DWORD chksumOffset = OPTHDROFFSET(hFile) + (DWORD)((PBYTE)&oh.CheckSum-(PBYTE)&oh);
    
    DWORD chksum;
    if( !ReadFileAt(hFile,
                &chksum,
                chksumOffset,
                sizeof(DWORD)) )
        return FALSE;

    // si il n'y a deja de checksum precalculee dans le header alors ne la recalcule pas
    if( !chksum )
        return TRUE;
    
    DWORD newChecksum = PEComputeRealChecksum(hFile);

    return WriteFileAt(hFile,
                &newChecksum,
                chksumOffset,
                sizeof(DWORD));
}
