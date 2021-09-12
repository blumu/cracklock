#ifndef _INC_PETOOLS
#define _INC_PETOOLS


typedef struct tagImportDirectory
{
	DWORD		dwRVAFunctionNameList;
	DWORD		dwUseless1;
	DWORD		dwUseless2;
	DWORD		dwRVAModuleName;
	DWORD		dwRVAFunctionAddressList;
}IMAGE_IMPORT_MODULE_DIRECTORY, * PIMAGE_IMPORT_MODULE_DIRECTORY;

////////////////////////////////////////////////////////////
// Prototypes des functions et macros
////////////////////////////////////////////////////////////

////////////
// Offset des headers

// PE Magic offset
WORD PEMAGICOFFSET( HANDLE );

#define SIZE_OF_NT_SIGNATURE	(DWORD)sizeof(DWORD)

// PE Header offset (juste apr�s la signature PE)
#define PEHDROFFSET(hFile)		PEMAGICOFFSET(hFile) + SIZE_OF_NT_SIGNATURE

// PE Optional header (juste apr�s le PE header)
#define OPTHDROFFSET(hFile)		PEHDROFFSET(hFile) + (DWORD)sizeof(IMAGE_FILE_HEADER)
			 
// Sections headers offsets (juste apr�s le PE 'optional header')
#define SECHDROFFSET(hFile)		OPTHDROFFSET(hFile) + (DWORD)sizeof(IMAGE_OPTIONAL_HEADER)

// Calculate the byte offset of a field in a structure of type type.
#define FIELDOFFSET(type, field)    ((SHORT)&(((type *)0)->field))

// taille du buffer (en BYTES) utilise pour calculer la checksum
#define MAX_BUFFER_SIZE     1024

////////////
// Outils divers

// Obtient le header d'une section identifi�e par son nom
BOOL GetSectionHdrByName (HANDLE hFile, PSTR pszSection, PIMAGE_SECTION_HEADER psh);
// Obtient l'offset o� se trouve l'entr�e IMAGE_DIRECTORY sp�cifi�e
DWORD ImageDirectoryOffset (HANDLE, DWORD, PIMAGE_SECTION_HEADER);

// I/O fonctions
DWORD ReadFileAt( HANDLE hFile, PVOID pData, DWORD dwOffset, DWORD dwDataSize );
DWORD WriteFileAt( HANDLE hFile, PVOID pData, DWORD dwOffset, DWORD dwDataSize );

// Renvoi le type et les caract�ristiques de l'image
DWORD ImageType (HANDLE hFile, PWORD pwCharacteristics);

// Info contenu dans le 'optional header'
WORD NumOfSections( HANDLE );

// Obtient les offsets de tous les modules import�s
//int GetImportModuleNamesOffsets( HANDLE, PDWORD, DWORD );

// Enum�re les noms de tous les modules import�s
typedef BOOL (CALLBACK* PENUMIMPFNC)(PSTR, DWORD, LPARAM, LPARAM);
int EnumImportModuleNames (HANDLE, PENUMIMPFNC, LPARAM, LPARAM);
//int EnumImportFunctionByModule (HANDLE, PCTSTR, PENUMIMPFNC, LPARAM, LPARAM );

// Renvoi l'adresse dans le fichier o� figure l'importation recherch�e
DWORD LookImportTable( HANDLE, PCSTR );
DWORD IsFunctionUsed( HANDLE, PCSTR, PCSTR );

// Supprime une directory de la section IMAGE_OPTIONAL_HEADER
BOOL DeleteDirectory (HANDLE hFile, DWORD dwIMAGE_DIRECTORY);

// Calcul la checksum reelle du fichier
DWORD ComputePEFileRealChecksum( PCTSTR pszFileName );
DWORD PEComputeRealChecksum( HANDLE hFile );

// Remplace l'ancienne checksum (calculee par l'editeur de lien link.exe) dans le header du fichier PE par une nouvelle checksum calculee a partir du fichier
BOOL PEPatchLinkchecksum( HANDLE hFile );

// Change le nom d'une DLL importee par un fichier PE
BOOL PERemplaceImportName( PCTSTR pszFileName, PCSTR pszPattern, PCSTR pszReplace );

#endif