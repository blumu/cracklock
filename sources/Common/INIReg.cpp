// Author: William Blum
// Date: february 2008
// INIReg module
// C++ Abstraction layer for the Windows Registry.
// This module contains classes that let you store settings either in an INI file or in the registry.

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include "INIReg.h"


/////// Informations sur l'endroit ou sont stockees les settings

// la location des settings
SectionStorageLocation g_storageLocation;

// Path to the program directory where the INI config file will be stored 
// when the location BinINI is selected.
tstring BinPath;


void ReInitSettingEngine()
{
    g_storageLocation = SectionLocationOfSettingsLocation(GetSettingStorageLocation());
}

// Initialize the setting engine.
// Must be called before calling any other function in this module.
// [in] binpath: Path to the program directory where the INI config file will be stored 
// when the location BinINI is selected.
void InitSettingEngine(PCTSTR binpath)
{
    BinPath = binpath;
    ReInitSettingEngine();
}


// Converti une location de type SettingsStorageLocation en SectionStorageLocation
SectionStorageLocation SectionLocationOfSettingsLocation(SettingsStorageLocation loc)
{
    if ( loc == BinINI ) {
        return SectionStorageLocation((BinPath + _T("\\") + REGVALUE_SETTINGS_INIFILENAME).c_str());
    }
    else if ( loc == AppDataINI ) {
        TCHAR AppData[_MAX_PATH];
        HRESULT hres = SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA |CSIDL_FLAG_CREATE, 
                              NULL, 
                              SHGFP_TYPE_CURRENT, 
                              AppData);
        
        if(SUCCEEDED(hres)) 
            return SectionStorageLocation((tstring(AppData) + _T("\\") + REGVALUE_SETTINGS_INIFILENAME).c_str());
        else
            return SectionStorageLocation((BinPath + _T("\\") + REGVALUE_SETTINGS_INIFILENAME).c_str());

    }
    else // loc == WinReg
        return SectionStorageLocation();
}


// Retourne l'endroit ou sont stockes les parametres
SettingsStorageLocation GetSettingStorageLocation()
{
    HKEY hKey;

    // Test si le fichier de settings existe dans le repertoire de Cracklock
    // si oui alors choisi le comme lieu de stockage courrant.
    if( FileExists(SectionLocationOfSettingsLocation(BinINI).inifile) )
        return BinINI;
    // Test si le fichier de settings existe dans le repertoire Appdata de l'utilisateur courrant
    // si oui alors choisi le comme lieu de stockage courrant.
    else if( FileExists(SectionLocationOfSettingsLocation(AppDataINI).inifile) )
        return AppDataINI;
    // Test si la clef du programme existe dans la registry
    // si oui alors le lieu de stockage courrant est la base de registre
    else if( ERROR_SUCCESS == RegOpenKeyEx(HKEY_BASE, REGKEY_APPROOT, 0, KEY_ALL_ACCESS, &hKey) ) {
        RegCloseKey(hKey);
        return WinReg;
    }
    else
        // Par defaut, les settings sont enregistres dans le repertoire de Cracklock
        return BinINI;
}


// Supprime toutes les settings se trouvant dans la location loc
void DeleteSettings(SettingsStorageLocation loc)
{
	SectionStorageLocation secloc = SectionLocationOfSettingsLocation(loc);
    
    SettingSection::DeleteSectionTree(secloc, REGKEY_APPROOT);

	// supprime le fichier de config
    if(secloc.inifile)
        DeleteFile(secloc.inifile);
}




// Copie les settings vers un autre endroit en enumerant toutes les sous sections
bool CopySettings(SettingsStorageLocation source,
                           SettingsStorageLocation destination)
{
    if(source == destination)
        return true;

    SettingSection src_sec(SectionLocationOfSettingsLocation(source));
    if( !src_sec.Open(REGKEY_APPROOT) )
        return false;

    // create the destination section
    SettingSection dst_sec(SectionLocationOfSettingsLocation(destination));
    if( !dst_sec.Create(REGKEY_APPROOT) )
        return false;

    return src_sec.CopyTo(dst_sec, true);
}

// Deplace l'endroit ou sont stockes les parametres
bool ChangeSettingsLocation(SettingsStorageLocation newloc)
{
    SettingsStorageLocation current = GetSettingStorageLocation();

    if(current != newloc) {
        if(!CopySettings(current, newloc))
            return false;
    }

    if( newloc != WinReg ) DeleteSettings(WinReg);
    if( newloc != AppDataINI ) DeleteSettings(AppDataINI);
    if( newloc != BinINI ) DeleteSettings(BinINI);
    ReInitSettingEngine();
    return true;
}



// Function name: FilePathToRegPath
// Description: Convertit un chemin d'accès à un fichier en un chemin d'accès dans l'encodage utilise pour les noms de section
//      * met en majuscules
//      * change les antislashs en slashs normaux
//
// Argument:
//   [in] pszSrc: nom du fichier à convertir
// 	 [in] cchDst: taille maximale du buffer destination en TCHARs
//   [out] pszDst : buffer ou est retourné le résultat
//
void FilePathToRegPath( PCTSTR pszSrc, SIZE_T cchDst, PTSTR pszDst)
{
    _ASSERT( pszSrc );
    _ASSERT( pszDst );

    _tcscpy_s(pszDst, cchDst, pszSrc);
    _tcsupr_s(pszDst, cchDst);
    while( pszDst = _tcschr(pszDst, '\\') )
        *pszDst = '/';
}

// Function name: RegPathToFilePath
// Description: Fonction inverse de FilePathToRegPath
// Arguments:
//   [in] pszSrc:: chemin d'access encode
//	 [in] cchDst: taille maximale du buffer destination en TCHARs
//   [out] pszDst: buffer destination contenant le chemin d'access decode
void RegPathToFilePath( PCTSTR pszSrc, SIZE_T cchDst, PTSTR pszDst )
{
    _ASSERT( pszSrc );
    _ASSERT( pszDst );

    if(pszSrc != pszDst)
        _tcscpy_s(pszDst, cchDst, pszSrc);

    while( pszDst = _tcschr(pszDst, '/')  )
        *pszDst = '\\';
}


SectionStorageLocation& SectionStorageLocation::operator= (const SectionStorageLocation &rhs)
{
	cleanup();
	this->inifile=rhs.inifile ?_tcsdup(rhs.inifile) : NULL;
	this->bRegistry=rhs.bRegistry;
	return *this;
}

bool operator== (const SectionStorageLocation &lhs, const SectionStorageLocation &rhs)
{
    return (lhs.bRegistry && rhs.bRegistry) || ((!lhs.bRegistry && !rhs.bRegistry && _tcsicmp(lhs.inifile,rhs.inifile)==0));
}

SectionStorageLocation::SectionStorageLocation(SectionStorageLocation &loc)
{
	this->inifile=loc.inifile ? _tcsdup(loc.inifile) : NULL;
	this->bRegistry=loc.bRegistry;
}
SectionStorageLocation::SectionStorageLocation(PCTSTR inipath)
{
	bRegistry=false;
	_ASSERT(inipath);
	inifile=_tcsdup(inipath);
}
SectionStorageLocation::~SectionStorageLocation()
{
	cleanup();
}

void SectionStorageLocation::cleanup()
{
    if(inifile){ free(inifile); inifile = NULL;}
}





const TCHAR gHexChar[] = _T("0123456789ABCDEF");

PTSTR Bin2Hex(void *data, size_t size)
{
    size_t cchHexbuff = _countof(INI_BOXEDDATATYPE_BINARY)-1  // tag
                        + size * 2  // data in hexadecimal
                        + 1; // terminal 0
    
    PTSTR hex = (PTSTR)malloc(sizeof(TCHAR) * cchHexbuff);	
	if (hex) {
        _tcscpy_s(hex, cchHexbuff, INI_BOXEDDATATYPE_BINARY);
		PTSTR hp = hex + _countof(INI_BOXEDDATATYPE_BINARY)-1;
        unsigned char *dp = (unsigned char *)data;
		
		while (size--) {
			unsigned char n1, n2;
			
			n1 = *dp >> 4;
			n2 = *dp & 0x0F;
			
			*hp++ = gHexChar[n1];
			*hp++ = gHexChar[n2];
			dp++;
		}
		*hp = 0;
	}	
	return hex;
}

class excBadHexString {};

void* Hex2Bin(PCTSTR hex, size_t& size)
{
	PCTSTR hp;
	unsigned char *dp, *result;
	size_t sLen = _tcslen(hex);

	size = (sLen / 2 + sLen % 2);
	result = (unsigned char*)malloc(size);

	if (result) {
		hp = hex;
		dp = result;
		size_t i = size;

		while (i--) {
			TCHAR n1 = *hp++;
			TCHAR n2 = *hp++;
			
			if (!isxdigit(n1)) throw(excBadHexString());
			if (n2 && !isxdigit(n2)) throw(excBadHexString());
			
			if (n1 >= '0' && n1 <= '9')
				n1 -= '0';
			else
				n1 = toupper(n1) - 'A' + 10;
			
			if (n2) {
				if (n2 >= '0' && n2 <= '9')
					n2 -= '0';
				else
					n2 = toupper(n2) - 'A' + 10;
			}
			
			*dp++ = n1 << 4 | n2;
		}
	}
	
	return result;
}

// Type utilise pour encapsule des donnees dans une chaine destinee a etre enregistree dans un fichier INI
class INIBoxedData {
public:
    inline DWORD type()      { return m_type; }
    inline PTSTR getPTSZ() 
    { 
        _ASSERT( m_type == REG_SZ );
        return (PTSTR)m_pdataaftertag;
    }    
    
    inline DWORD getDWORD() { 
        _ASSERT( m_type == REG_DWORD );
        return _ttoi(m_pdataaftertag); // skip the tag "dword:"
    }

    size_t INIBoxedData::unbox(PBYTE pout, size_t cbMaxsize)
    {
        if( m_type == REG_DWORD ) {
            *(PDWORD)pout = getDWORD();
            return sizeof(DWORD);
        }
        else if( m_type == REG_SZ ) {
            if( m_cchSize == 0 )
                return 0;

            rsize_t max = cbMaxsize/sizeof(TCHAR);
            _tcscpy_s((PTSTR)pout, max, getPTSZ());
            return min(sizeof(TCHAR)*(_tcslen(getPTSZ())+1), max);
        }
        else if( m_type == REG_BINARY ) {
            // decode la chaine hexadecimale
            size_t stsize;
            void *pbin = Hex2Bin(m_pdataaftertag, stsize);
            if( pbin == NULL )
                return 0;

            // verifie qu'il y a assez de place dans le buffer fourni par l'appellant pour decoder la chaine hexadecimale
            if(stsize>cbMaxsize) {
                free(pbin);
                return -1;
            }

            memcpy(pout, pbin, stsize);
            free(pbin);
            return stsize;
        }
        else { // type inconnu: copie en brut
            size_t size = min(cbMaxsize, sizeof(TCHAR)*m_cchSize);
            memcpy(pout, m_pdataaftertag, size);
            return size;
        }
    }


private:
    DWORD m_type;
    PTSTR m_boxeddata; // raw boxed data
    PTSTR m_pdataaftertag; // point to the data following the tag
    size_t m_cchSize;      // size of the data after the tag in characters (including the 0)

public:

    INIBoxedData(PTSTR boxeddata, // boxed data
                 size_t cchSize    // size in TCHAR (including terminal 0)
                 ) {
        m_boxeddata = boxeddata;
        if(cchSize >= _countof(INI_BOXEDDATATYPE_DWORD) && 0 == _tcsncmp(boxeddata, INI_BOXEDDATATYPE_DWORD, _countof(INI_BOXEDDATATYPE_DWORD)-1)) {
            m_type = REG_DWORD;
            m_pdataaftertag = boxeddata+_countof(INI_BOXEDDATATYPE_DWORD)-1;
            m_cchSize = sizeof(DWORD)/sizeof(TCHAR);
        }
        else if(cchSize >= _countof(INI_BOXEDDATATYPE_SZ) && 0 == _tcsncmp(boxeddata, INI_BOXEDDATATYPE_SZ, _countof(INI_BOXEDDATATYPE_SZ)-1)) {
            m_type = REG_SZ;
            m_pdataaftertag = boxeddata+_countof(INI_BOXEDDATATYPE_SZ)-1;
            m_cchSize = cchSize - (_countof(INI_BOXEDDATATYPE_SZ)-1);
        }
        else {
            m_type = REG_BINARY;
            m_pdataaftertag = boxeddata+_countof(INI_BOXEDDATATYPE_BINARY)-1;
            m_cchSize = cchSize - (_countof(INI_BOXEDDATATYPE_BINARY)-1);;
        }

    }

};





// constructeur par defaut
SettingSection::SettingSection(){
    location = g_storageLocation; // par defaut, choisit la location des settings courrante
    pszEnumBuff = NULL;
	secname = NULL;
    pathToParent = NULL;
	hKey = NULL;
    enum_secnames = NULL;
    enum_nsecnames = 0;
}

SettingSection::SettingSection(SectionStorageLocation &loc)
{
	location = loc;
    pszEnumBuff = NULL;
	secname = NULL;
    pathToParent = NULL;
	hKey = NULL;
    enum_secnames = NULL;
    enum_nsecnames = 0;
}


SettingSection::~SettingSection() {
	Close();
}


void SettingSection::deleteObject(SettingSection *o)
{
    delete o;
} 


// Copie une sous-section vers une autre section.
// La section de destination peut se trouver dans une different "location storage"
// ce qui permet d'importer/exporter entre fichier INI et la base de registre.
bool SettingSection::CopyTo( SettingSection &dstsec,  // section de destination?
                             bool bRecursive)         // copie recursivement les sous-sections?
{
    bool ret = true;

    // copie les sous-sections
    if( bRecursive ) {
        TCHAR subsecname[_MAX_PATH];
        for(this->InitEnumSubsection(); this->EnumSubsection(subsecname, _countof(subsecname)); ) {
            // open the source subsection
            SettingSection *subsec_src = this->GetSubsection(subsecname);
            if( subsec_src == NULL ) {
                ret = false;
                continue;
            }

            // create the destination subsection
            SettingSection *subsec_dst = dstsec.CreateSubsection(subsecname);
            if( subsec_dst == NULL ) {
                ret = false;
                delete subsec_src;
                continue;
            }

            ret &= subsec_src->CopyTo(*subsec_dst, true);

            delete subsec_dst;
            delete subsec_src;
        }
    }
    
    // copie les valeurs
    TCHAR value[_MAX_PATH];
    size_t valsize = _countof(value);
    BYTE content[500];
    size_t size = sizeof(content);
    DWORD type;
    this->InitEnumValues();
    while( this->EnumValues(value, &valsize, content, &size, &type) ) {
        ret &= dstsec.SetValue(value, content, size, type);
        valsize = _countof(value);
        size = sizeof(content);
    }
    return ret;
}

// supprime une section entiere recursivement
bool SettingSection::DeleteSectionTree(SectionStorageLocation loc, PCTSTR secname)
{
    SettingSection sec(loc);
    if( sec.Open( secname ) ) {
	    TCHAR subsec[_MAX_PATH];
        if( loc.bRegistry ) {
            // we cannot not the 'EnumSubsection' enumerator here cause in the registry
            // the enumeration is affected by the removing of the keys
            while( ERROR_NO_MORE_ITEMS!=RegEnumKey(sec.hKey, 0, subsec, _countof(subsec)) )
                sec.DeleteSubsection(subsec, true);
        }
        else {
            for(sec.InitEnumSubsection(); sec.EnumSubsection(subsec, _countof(subsec)); )	    
                sec.DeleteSubsection(subsec, true);
        }

        sec.Close();
        return DeleteSection(loc, secname);
    }
    return false;
}


// delete section secname
// bRecursive=true if all the tree under the subsection needs to be deleted
bool SettingSection::DeleteSubsection(PCTSTR secname, bool bRecursive)
{    
    if( bRecursive ) {
	    SettingSection *sec = GetSubsection( secname );
	    if( sec ) {
		    TCHAR subsec[_MAX_PATH];
            if( location.bRegistry ) {
                while( ERROR_NO_MORE_ITEMS!=RegEnumKey(sec->hKey, 0, subsec, _countof(subsec)) )
                    sec->DeleteSubsection(subsec, true);
            }
            else {
		        for(sec->InitEnumSubsection();
			        sec->EnumSubsection(subsec, _countof(subsec)); )
			        sec->DeleteSubsection(subsec, true);
                delete sec;
            }
	    }
    }

    if( location.bRegistry )
		return ERROR_SUCCESS==RegDeleteKey(this->hKey, secname);
	else {
        tstring szFullSectionPath = this->path() + _T("\\") + secname;
        return WritePrivateProfileString(szFullSectionPath.c_str(), NULL, NULL, location.inifile)!=NULL;
    }
	
}



// Delete a section
bool SettingSection::DeleteSection(SectionStorageLocation location, PCTSTR section) {
	if( location.bRegistry )
		return ERROR_SUCCESS==RegDeleteKey(HKEY_BASE, section);
    else {
        tstring szFullSectionPath = section;
        return WritePrivateProfileString(szFullSectionPath.c_str(), NULL, NULL, location.inifile)!=NULL;
    }
}

// Delete a section in the current storage place
bool SettingSection::DeleteSection(PCTSTR section)
{
    return SettingSection::DeleteSection(g_storageLocation, section);	
}


// Open a setting section
bool SettingSection::Open(PCTSTR section)
{
	Close();
	if( location.bRegistry ) {
		return ERROR_SUCCESS == RegOpenKeyEx(HKEY_BASE, section, 0, KEY_ALL_ACCESS, &this->hKey);
	}
	else {
        this->setPath(_T(""), section);
		return true;
	}
}

// Create a setting section
bool SettingSection::Create(PCTSTR section)
{
	Close();
	if( location.bRegistry ) {
        DWORD dwDispo;
		return ERROR_SUCCESS == RegCreateKeyEx(HKEY_BASE, section, 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &this->hKey, &dwDispo);
	}
	else {
        this->setPath(_T(""), section);
		return true;
	}
}


void SettingSection::Close()
{
    if(enum_secnames) { free(enum_secnames); enum_secnames=NULL; }
	if(hKey){ RegCloseKey(hKey); hKey = NULL;}
    if(pathToParent){ free(pathToParent); pathToParent = NULL;}
    if(secname){ free(secname); secname = NULL;}
    if(pszEnumBuff){ free(pszEnumBuff); pszEnumBuff = NULL; }
}


// Set the full path to the section
void SettingSection::setPath(PCTSTR _parent, PCTSTR _secname)
{
    if(pathToParent) free(pathToParent);
    pathToParent = _tcsdup(_parent);
    if(secname) free(secname);
    secname = _tcsdup(_secname);
}

// Open and return a subsection of the current section
SettingSection *SettingSection::GetSubsection(PCTSTR subsectionname)
{
	SettingSection *sec = new SettingSection(this->location);

	if( location.bRegistry  ) {
        // set the reg key
		if( RegOpenKeyEx( this->hKey, subsectionname, 0, KEY_ALL_ACCESS, &sec->hKey) !=ERROR_SUCCESS ) {
			delete sec;
			sec = NULL;
		}
	}
    else
        // set the path to the section
        sec->setPath(this->path().c_str(), subsectionname);

	return sec;
}

SettingSection *SettingSection::CreateSubsection(PCTSTR subsectionname)
{ 
	SettingSection *sec = new SettingSection(this->location);
	if( location.bRegistry  ) {
		DWORD dwDispo;				
		if( RegCreateKeyEx( this->hKey, subsectionname, 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &sec->hKey, &dwDispo)
			!=ERROR_SUCCESS ) {
			delete sec;
			sec = NULL;
		}
	}
	else {
        sec->setPath(this->path().c_str(), subsectionname);
	}
	return sec;
}





// Load data from the registry
bool SettingSection::GetValue(PCTSTR value, PBYTE pOut, DWORD cbMaxsize) const {
	if( location.bRegistry  ) {
		return RegQueryValueEx(this->hKey, value, NULL, NULL, pOut, &cbMaxsize)==ERROR_SUCCESS
				&& cbMaxsize>0;
	}
    else {
        // Lit le contenu de la valeur en augmentant le buffer a chaque fois jusqu'a ce que la totalite 
        // des donnees soit lue
        DWORD cchSizeBuff = 512;
        size_t cchSizeData;
        PTSTR pBuff = NULL;
        do {
    		cchSizeBuff *= 2;
            pBuff = (PTSTR) realloc(pBuff, cchSizeBuff * sizeof(TCHAR));
        }
        while( (cchSizeData = GetPrivateProfileString(path().c_str(), value, _T(""), pBuff, cchSizeBuff, location.inifile)) >= (cchSizeBuff-2) );

        if( cchSizeData <= 0 )
            return false;

        INIBoxedData box(pBuff, cchSizeData+1);
        box.unbox(pOut, cbMaxsize);      
        free(pBuff);
        return true;
    }
}

// Load a string from the registry
bool SettingSection::GetValue(PCTSTR value, PTSTR pszOut, DWORD cchSize) const {
	if( location.bRegistry  ) {
        DWORD dwSize = sizeof(TCHAR)*cchSize;
		return RegQueryValueEx(this->hKey, value, NULL, NULL, (PBYTE)pszOut, &dwSize)==ERROR_SUCCESS
				&& dwSize>0;
	}
    else {
        DWORD len = GetPrivateProfileString(this->path().c_str(), value, _T(""), pszOut, cchSize, this->location.inifile);
        if (len < _countof(INI_BOXEDDATATYPE_SZ) )
            return false;
        else  {
            // skip the tag "string:"
            memmove(pszOut, pszOut+_countof(INI_BOXEDDATATYPE_SZ)-1, sizeof(TCHAR)*(len+1-(_countof(INI_BOXEDDATATYPE_SZ)-1)));
            return true;
        }
        return true; 
    }
}


// Load a DWORD from the registry
bool SettingSection::GetValue(PCTSTR value, DWORD *pOut) const {
	if( location.bRegistry  ) {
		DWORD dwSize = sizeof(DWORD);
		return RegQueryValueEx(this->hKey, value, NULL, NULL, (PBYTE)pOut, &dwSize)==ERROR_SUCCESS;
	}
    else {
        // We do not use GetPrivateProfileInt otherwsie there would be no way to determine if the value was existing or not 
        // (since GetPrivateProfileInt returns a default value in that case)
        TCHAR buff[_countof(INI_BOXEDDATATYPE_DWORD)+11]; // 10 digits are enough for a 32 bits integer;
        DWORD len = GetPrivateProfileString(this->path().c_str(), value, _T(""), buff, _countof(buff), this->location.inifile);
        INIBoxedData box(buff, len+1);
        if( box.type() == REG_DWORD ) {
            *pOut = box.getDWORD();
            return true;
        }
        else
            return false;
    }
}

// Load a bool from the registry
bool SettingSection::GetValue(PCTSTR value, bool *pOut) const {
    DWORD dw;
    if( GetValue(value,&dw) ) {
        *pOut = dw!=0;
        return true;
    }
    else
        return false;
}

// Load a TIME_ZONE_INFORMATION from the registry
bool SettingSection::GetValue(PCTSTR value, TIME_ZONE_INFORMATION *pOut) const {
    return GetValue(value, (PBYTE)pOut, sizeof(TIME_ZONE_INFORMATION));
}

// Load a RECT from the registry
bool SettingSection::GetValue(PCTSTR value, RECT *pOut) const {
    return GetValue(value, (PBYTE)pOut, sizeof(RECT));
}

// Load a FILETIME from the registry
bool SettingSection::GetValue(PCTSTR value, FILETIME *pOut) const {
    return GetValue(value, (PBYTE)pOut, sizeof(FILETIME));
}


// Set a value given by raw data together with its type
bool SettingSection::SetValue(PCTSTR value, const PBYTE pIn, size_t cbSize, DWORD type)
{
	if( location.bRegistry  )
		return RegSetValueEx(this->hKey, value, NULL, type, (const BYTE *) pIn, cbSize)==ERROR_SUCCESS;
    else {
        if( type == REG_DWORD )
            return SetValue(value, (DWORD)*pIn); // DWORD data
        else if ( type == REG_SZ )
            return SetValue(value, (PTSTR)pIn); // string data
        else 
            return SetValue(value, pIn, cbSize); // raw data
    }

}

// set a string value
bool SettingSection::SetValue(PCTSTR value, PCTSTR pIn)
{
	if( location.bRegistry  )
		return RegSetValueEx(this->hKey, value, NULL, REG_SZ, (const BYTE *) pIn, (_tcslen(pIn)+1) * sizeof(TCHAR))==ERROR_SUCCESS;
	else
        return 0!=WritePrivateProfileString(this->path().c_str(), value, (tstring(INI_BOXEDDATATYPE_SZ) + pIn).c_str(), this->location.inifile);
}

// set a raw binary value
bool SettingSection::SetValue(PCTSTR value, const PBYTE pIn, size_t cbSize)
{
	if( location.bRegistry )
		return RegSetValueEx(this->hKey, value, NULL, REG_BINARY, (const BYTE *) pIn, cbSize)==ERROR_SUCCESS;
    else {
        PTSTR phex = Bin2Hex(pIn, cbSize);
        bool b = 0!= WritePrivateProfileString(this->path().c_str(), value, phex, this->location.inifile);
        free(phex);
        return b;
    }
}
// set a timezone value
bool SettingSection::SetValue(PCTSTR value, const TIME_ZONE_INFORMATION *pIn)
{
    return SetValue(value, (const PBYTE)pIn, sizeof(TIME_ZONE_INFORMATION));
}
bool SettingSection::SetValue(PCTSTR value, const RECT *pIn)
{
    return SetValue(value, (const PBYTE)pIn, sizeof(RECT));
}
// set a filetime value
bool SettingSection::SetValue(PCTSTR value, const FILETIME *pIn)
{
    return SetValue(value, (const PBYTE)pIn, sizeof(FILETIME));
}
// set a dword value
bool SettingSection::SetValue(PCTSTR value, DWORD in)
{
	if( location.bRegistry )
		return RegSetValueEx(this->hKey, value, NULL, REG_DWORD, (const BYTE *) &in, sizeof(DWORD))==ERROR_SUCCESS;
    else {        
        TCHAR buff[_countof(INI_BOXEDDATATYPE_DWORD) + MAX_INI_INT_DIGITS] // account for "DWORD:"
            = INI_BOXEDDATATYPE_DWORD;
        if( 0 != _itot_s(in,buff+_countof(INI_BOXEDDATATYPE_DWORD)-1, _countof(buff)-_countof(INI_BOXEDDATATYPE_DWORD)+1, 10) )
            return false;

        return 0!=WritePrivateProfileString(this->path().c_str(), value, buff, this->location.inifile);
    }
}
// set a boolean value
bool SettingSection::SetValue(PCTSTR value, bool in)
{
    return SetValue(value, (DWORD)(in ? 1 : 0));
}

int __cdecl PCTSTR_icompare(const void *l, const void *r )
{
    return _tcsicmp(*(PCTSTR *)l,*(PCTSTR *)r);
}

void SettingSection::InitEnumSubsection()
{
	if( !location.bRegistry  ) {
        // Lit la list des noms de sections en augmentant le buffer a chaque fois jusqu'a ce que la liste soit lue integralement
        DWORD cchSize = 512;
        do{	    
    		cchSize *= 2;
	    	pszEnumBuff = (PTSTR) realloc(pszEnumBuff, cchSize*sizeof(TCHAR));
	    }
        while (GetPrivateProfileSectionNames(pszEnumBuff, cchSize, location.inifile) >= (cchSize-2));

        // count the number of sections
        enum_nsecnames = 0;
        pszEnumPos = pszEnumBuff;
        while(*pszEnumPos!= _T('\0')) {
            pszEnumPos += _tcslen(pszEnumPos)+1;
            enum_nsecnames++;
        }

        // fill the section names table enum_secnames
        if(enum_secnames) free(enum_secnames);
        enum_secnames = new PCTSTR[enum_nsecnames];
        enum_nsecnames = 0;
        pszEnumPos = pszEnumBuff;
        while(*pszEnumPos!= _T('\0')) {
            enum_secnames[enum_nsecnames++] = pszEnumPos;
            pszEnumPos += _tcslen(pszEnumPos)+1;
        }

        // sort the table
        qsort((void *)enum_secnames, enum_nsecnames, sizeof(PCTSTR), PCTSTR_icompare);
    }
    enumindex = 0;
}


// Enumerate the subsections of a section setting.
// [out] section: the name of the iterated subsection
// [in] cbSize: maximal size in TCHAR characters of the destination buffer subsection
bool SettingSection::EnumSubsection(PTSTR subsection, DWORD cchSize )
{
	if( location.bRegistry  ) {
		return ERROR_NO_MORE_ITEMS!=RegEnumKey(this->hKey, enumindex++, subsection, sizeof(TCHAR)*cchSize);
	}
	else {
		_ASSERT(pszEnumBuff); // l'enumeration doit etre initialisee

        tstring p = path() + _T("\\");

        if( enumindex<enum_nsecnames ) {
            // find the next INI section whose path is a direct subsection of the current section
            while(enumindex<enum_nsecnames) {
                if( // check that it is a subsection of the current section
                    0==_tcsnicmp(enum_secnames[enumindex], p.c_str(), p.length()) 
                    ) {

                    // is it a direct subsection (ie only one level deeper in the section hierarchy)?
                    PCTSTR subsub = _tcschr(enum_secnames[enumindex]+p.length(), _T('\\'));
                    if( NULL == subsub ) {
                        // yes so return the enumerated subsection
                        _tcscpy_s(subsection, cchSize, enum_secnames[enumindex]+p.length());
                        enumindex++;
                        return true;
                    }

                    // if it is not a direct subsection then check that the shortest prefix of this subsection 
                    // that corresponds to a direct subsection has not already been added.
                    if(enumindex == 0 ||
                        0!=_tcsnicmp(enum_secnames[enumindex], enum_secnames[enumindex-1], subsub-enum_secnames[enumindex]) ) {

                        // if not then (this means that the INI file contain a section SEC\SUB1\SUB2 without having SEC\SUB1)
                        // then we add the direct subsection prefix (ie SEC\SUB1).
                        _tcsncpy_s(subsection, cchSize, enum_secnames[enumindex]+p.length(), subsub-(enum_secnames[enumindex]+p.length()));
                        enumindex++;
                        return true;
                    }
                }
                enumindex++;
            }
        }

        // end of enumeration: free the enumeration structures
        free(pszEnumBuff); pszEnumBuff = NULL;
        free(enum_secnames); enum_secnames = NULL;
        enum_nsecnames = 0;
	    return false;
	}
}

void SettingSection::InitEnumValues()
{
	if( location.bRegistry  )
    	valuesenumindex = 0;
    else {
        // Lit les sections en augmentant le buffer a chaque fois jusqu'a ce que le fichier soit lu integralement
        DWORD cchSize = 512;
        do {
    		cchSize  *= 2;
	    	pszEnumBuff = (PTSTR) realloc(pszEnumBuff, cchSize*sizeof(TCHAR));
        }
        while( GetPrivateProfileSection(path().c_str(), pszEnumBuff, cchSize, location.inifile) >= (cchSize-2) );
        pszEnumPos = pszEnumBuff;
    }
}


// Enumerate the value of a section setting.
// [out] name: the name of the iterated value
// [in/out] cbNameSize: maximal size in TCHAR characters of the buffer name. The length of name is returned in it.
// [out] content: the content of the iterated value
// [in/out] dwContentSize: maximal size in bytes of the buffer content. The number of byte read is returned in it.
// [out] pType: if not NULL, then the type of the data is stored in *pType. 
bool SettingSection::EnumValues(PTSTR name, size_t *pcchNameSize,
                                PBYTE content, size_t *pcbContentSize,
                                PDWORD pType)
{
    if( location.bRegistry ) {
        DWORD dwCbName = 0, dwCbContent = 0;
        LSTATUS ret = RegEnumValue(this->hKey, valuesenumindex++,
                                                 name, &dwCbName,
                                                 0, pType,
                                                 content, &dwCbContent);
        *pcchNameSize = dwCbName;
        *pcbContentSize = dwCbContent;
        return ERROR_NO_MORE_ITEMS!=ret;
	}
	else {
        _ASSERT(pszEnumPos);
        _ASSERT(pszEnumBuff);
        // l'enumeration doit etre initialisee
        // find the next INI section whose path extend the path of the current section
        if(*pszEnumPos!= _T('\0')) {
            PTSTR eq = _tcschr(pszEnumPos, _T('='));
            size_t len = _tcslen(pszEnumPos);
            // symbol '=' trouve?
            if( eq ) { 
                // extrait le nom de la valeur et son contenu
                size_t name_len = eq-pszEnumPos;
                _tcsncpy_s(name, *pcchNameSize, pszEnumPos, min(name_len,*pcchNameSize));
                *pcchNameSize = (DWORD)name_len;

                INIBoxedData box(eq+1, (DWORD)(len-(name_len+1)+1));
                if( pType )
                    *pType = box.type();
                *pcbContentSize = box.unbox(content, *pcbContentSize); // unbox the data

            }
            // pas de signe '='. Retourne le reste de la chaine comme valueur, sans contenu.
            else {
                _tcscpy_s(name, *pcchNameSize, pszEnumPos);
                *pcchNameSize = _tcslen(pszEnumPos);
                _tcscpy_s((PTSTR)content, *pcbContentSize, _T("")); // contenu vide
                *pcbContentSize = sizeof(TCHAR);
                if( pType )
                    *pType = REG_NONE;
            }
            pszEnumPos += len+1;
            return true;
        }
        else {
            free(pszEnumBuff); // end of enumeration
            pszEnumBuff = NULL;
		    return false;
        }
	}
}


// Test si un fichier existe
bool FileExists(PCTSTR szKeyName)
{
	HANDLE hFile = CreateFile( szKeyName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if( hFile == INVALID_HANDLE_VALUE )
		return false;
	else {
		CloseHandle( hFile );
		return true;
	}
}