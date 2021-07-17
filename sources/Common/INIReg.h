// INIReg module
// Abstraction layer for the Windows Registry that can store settings either in an INI file or in the registry.
#pragma once
#include <tchar.h>
#include <string>
#include <shlobj.h>

// This include is only used by Cracklock to export functions defined in this module
// Remove it if you want to use INReg in your own programs.
#include "..\ShellExt\impexp.h"

using namespace std;

#ifndef tstring
#ifdef UNICODE
    typedef wstring tstring;
#else
    typedef string tstring;
#endif
#endif

// maximum number of digits of a 32bit number
#define MAX_INI_INT_DIGITS              10

//#define HKEY_BASE                             HKEY_LOCAL_MACHINE
#define HKEY_BASE                               HKEY_CURRENT_USER
// Replace the value of the following macro by a registry key name corresponding to your application
#define REGKEY_APPROOT                          _T("Software\\MicroBest")
// Name of the INI file
#define REGVALUE_SETTINGS_INIFILENAME           _T("Cracklock.settings")

// Type des donnes INI mise en boite
#define INI_BOXEDDATATYPE_DWORD                 _T("dword:")
#define INI_BOXEDDATATYPE_SZ                    _T("string:")
#define INI_BOXEDDATATYPE_BINARY                _T("binary:")

DECLSPECIFIER void InitSettingEngine(PCTSTR binPath);

// There are three posssible setting storage locations
typedef enum { WinReg = 0, // in the Windows Registry
			   AppDataINI, // in an INI file in the users' AppData directory
			   BinINI      // in an INI file next to Cracklock binaries
     } SettingsStorageLocation;

DECLSPECIFIER SettingsStorageLocation GetSettingStorageLocation();

// Storage location for a setting section
class DECLSPECIFIER SectionStorageLocation
{
public:
	bool bRegistry; // true if stored in the windows registry
	PTSTR inifile; // if bRegistr=false then it contains the path to the INI file
	
	SectionStorageLocation()
	{
		bRegistry=true;
		inifile=NULL;
	}
	
	SectionStorageLocation& operator= (const SectionStorageLocation &rhs);
	SectionStorageLocation(SectionStorageLocation &loc);
	SectionStorageLocation(PCTSTR inipath);
	~SectionStorageLocation();
	void cleanup();

};

DECLSPECIFIER bool operator== (const SectionStorageLocation &lhs, const SectionStorageLocation &rhs);
DECLSPECIFIER SectionStorageLocation SectionLocationOfSettingsLocation(SettingsStorageLocation loc);

DECLSPECIFIER void DeleteSettings(SettingsStorageLocation location);
DECLSPECIFIER bool CopySettings(SettingsStorageLocation source, SettingsStorageLocation destination);
DECLSPECIFIER bool ChangeSettingsLocation(SettingsStorageLocation newloc);


// Class used to manipulate setting sections
class DECLSPECIFIER SettingSection {
public:
		// where is this section stored?
		SectionStorageLocation location; 

protected:
        PTSTR pathToParent; // path to the root section
        PTSTR secname;

		// variable used for subsection enumeration
		int enumindex;

        ///// members used by the Windows Registry settings engine
		    // regkey handle    
            HKEY hKey;
            // value enumeration
		    int valuesenumindex;


		///// members used by the INI settings engine
            // value enumeration
		    PTSTR pszEnumBuff;
            PTSTR pszEnumPos;

		    // sections enumeration
            PCTSTR *enum_secnames;
            int enum_nsecnames;
    
public:

        tstring path() const {
            return (this->pathToParent && this->secname
                        ? (this->pathToParent[0]
                            ? (tstring(this->pathToParent) + _T("\\") + tstring(this->secname))
                            : tstring(this->secname))
                        : tstring(_T("")));
        }

        // Attention: ne pas definir cette function inline sinon ca fait planter
        // les programme qui exporte la DLL ShellExt.
		static void deleteObject(SettingSection *o);

		bool Open(PCTSTR section);
		bool Create(PCTSTR section);        
		void Close();

        void setPath(PCTSTR _parent, PCTSTR _secname);

		SettingSection *GetSubsection(PCTSTR subsectionname);		
		SettingSection *CreateSubsection( PCTSTR subsectionname);		
		static bool DeleteSection(SectionStorageLocation location, PCTSTR section);
        static bool DeleteSection(PCTSTR section);

        bool CopyTo(SettingSection &dst, bool bRecursive);


		static bool DeleteSectionTree(SectionStorageLocation location, PCTSTR secname);
		bool DeleteSubsection(PCTSTR secname, bool bRecursive = false);


		bool GetValue(PCTSTR value, PBYTE pOut, DWORD cbMaxsize) const ;
        bool GetValue(PCTSTR value, PTSTR pszOut, DWORD cchSize) const ;
		bool GetValue(PCTSTR value, TIME_ZONE_INFORMATION *pOut) const ;
		bool GetValue(PCTSTR value, FILETIME *pOut) const ;
		bool GetValue(PCTSTR value, RECT *pOut) const ;
		bool GetValue(PCTSTR value, DWORD *pOut) const ;
		bool GetValue(PCTSTR value, bool *pOut) const ;

		bool SetValue(PCTSTR value, PCTSTR pIn);
		bool SetValue(PCTSTR value, const PBYTE pIn, size_t cbSize, DWORD type);
        bool SetValue(PCTSTR value, const PBYTE pIn, size_t cbSize);
		bool SetValue(PCTSTR value, const TIME_ZONE_INFORMATION *pIn);
		bool SetValue(PCTSTR value, const FILETIME *pIn);
		bool SetValue(PCTSTR value, const RECT *pIn);
		bool SetValue(PCTSTR value, DWORD in);
		bool SetValue(PCTSTR value, bool in);

		void InitEnumSubsection();
		bool EnumSubsection(PTSTR subsection, DWORD cchSize );

		void InitEnumValues();
		bool EnumValues(PTSTR name, size_t *pcchNameSize, PBYTE content, size_t *pcbContentSize, PDWORD pType );

        SettingSection();
		SettingSection(SectionStorageLocation &loc);
		~SettingSection();
};


DECLSPECIFIER bool FileExists(PCTSTR szKeyName);
DECLSPECIFIER void RegPathToFilePath( PCTSTR pszSrc, SIZE_T cchDst, PTSTR pszDst );
DECLSPECIFIER void FilePathToRegPath( PCTSTR pszSrc, SIZE_T cchDst, PTSTR pszDst );
