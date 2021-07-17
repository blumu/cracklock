#pragma once
 
// {6EF84290-174B-11d1-B524-0080C8141490}
DEFINE_GUID(CLSID_ShellExtension, 0x6ef84290, 0x174b, 0x11d1, 0xb5, 0x24, 0x0, 0x80, 0xc8, 0x14, 0x14, 0x90);

//STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, PVOID *ppvOut);


// this class factory object creates context menu handlers for Windows 95 shell
class CShellExtClassFactory : public IClassFactory
{
protected:
    ULONG	m_cRef;

public:
    CShellExtClassFactory();
    ~CShellExtClassFactory();

    //IUnknown members
    STDMETHODIMP            QueryInterface(REFIID, PVOID FAR *);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

    //IClassFactory members
    STDMETHODIMP            CreateInstance(LPUNKNOWN, REFIID, PVOID FAR *);
    STDMETHODIMP            LockServer(BOOL);
};

// {751E4340-44BA-11d2-B500-0080C8141490}
DEFINE_GUID(IID_IShellConfigPage, 0x751e4340, 0x44ba, 0x11d2, 0xb5, 0x0, 0x0, 0x80, 0xc8, 0x14, 0x14, 0x90);
interface IShellConfigPage : IUnknown
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IShellConfigPage methods ***
    STDMETHOD(CreateConfigurationWindow)(THIS_ PTSTR pszFile, HWND hParent, HWND *phwnd) PURE;
    STDMETHOD(SetActivate)(THIS_ bool bActive) PURE;
};
typedef IShellConfigPage *LPSHELLCONFIGPAGE;

#include "CNodes.h"         // Défition de l'objet CNodes
#include "CTabSheet.h"      // objet CTabSheet
#include "TabGeneral.h"     // onglet general
#include "TabOptions.h"     // onglet options
#include "TabMoreOptions.h" // onglet more options
#include "TabDepend.h"      // onglet dépendances
#include "TabHelp.h"        // onglet aide


class CShellExt : public IShellExtInit, 
                         IContextMenu,
                         IShellPropSheetExt,
                         IShellConfigPage
{
public:
    TCHAR       m_szPEFileName[_MAX_PATH];  // Fullpath to the filename the user clicked on
    TCHAR       m_szPEFileDir[_MAX_PATH];   // File directory (without filename)

    AppSettings m_appSettings;          // Configuration de l'application
                                        //  (chargee a partir de la base de registre).

    bool        m_bCracked;             // Le fichier EXE de l'application était-il cracké à son ouverture?
    SYSTEMTIME  m_stCreation,           // Date de création du fichier
                m_stModification;       // Date de derniere modification du fichier
    bool        m_bChanged;             // Des modifications ont-elles eu lieu apres le chargement de l'application ?
    bool        m_bDLL;                 // le fichier PE est-il une DLL ?

    CNodes      *m_lpld;                // Liste des dépendances du fichier
    CNode       *m_pLoaderFile;         // Pointeur sur le noeud correspondant au fichier chargeur
                                        // si = NULL alors on est en mode loader (pas de fichier sélectionné dans l'onglet depend)
    
    HWND        m_hPage;            // handle de la page "Cracklock"
    HWND        m_hTab;             // handle du controle tab 
    HFONT       m_hfUnicode;        // police Unicode

    BOOL        m_bShowAdvanced;        // Faut-il afficher les options avancées ?
    BOOL        m_bConfigurationMode;   // vaut TRUE en mode configuration :
                                        //  * force l'affichage des options avancés	
                                        //  * n'affiche pas l'onglet general

    CTabSheet    *m_current;         // page courrante
    CGeneral     m_general;          // pages affichés dans le contrôle tab
    COptions     m_options;
    CMoreOptions m_moreoptions;
    CDepend      m_depend;
    CHelp        m_help;

    bool        m_bModifByProgram;          // Modification par programme (true) / par l'utilisateur (false)
    WNDPROC     m_pOrgWinProc;              // Pointeur sur la procédure de fenêtre original

public:
    // Constructor/Destructor
    CShellExt();
    ~CShellExt();

    void OnModification();
    void ShowTab(BOOL);

    // Create a shortcut that starts the selected program with CL_LOADER_EXE
    bool CreateLoaderShortcut(PCTSTR szShortcutPath);
    bool CreateLoaderShortcut(int iFolder);
    PTSTR GetShortcutFilePath(PCTSTR shortcutDir);
    bool CreateLocalLoaderShortcut();

protected:
    BOOL SetFile(PCTSTR pszFile);
    friend BOOL CALLBACK EnumDep (PSTR pszName, DWORD dwOffset, LPARAM lParam, LPARAM lParam2);
    friend INT_PTR CALLBACK PageDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
    friend INT_PTR CALLBACK RetryDlgProc(HWND, UINT, WPARAM, LPARAM);


    void OnInit(HWND hDlg);
    BOOL OnApply();
    void OnDestroy();
    void OnSelChanged();

    void RegLoadSettings();
    void RegSaveSettings();

    BOOL GetFileName(PTSTR pszFileName, int cbMax);
    bool NeedToBeCrack(CNode *);

private:
    void LoadDefaultAppSetting();
    bool LoadAppSetting( SettingSection *sec = NULL );
    bool SaveAppSetting();
    bool DeleteAppSetting();


    /////////////
    // Implémentation des interfaces de base

protected:
    ULONG           m_cRef;
    LPDATAOBJECT    m_pDataObj;

public:
    //IUnknown methods
    STDMETHODIMP            QueryInterface(REFIID, PVOID FAR *);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

    //IShellExtInit methods
    STDMETHODIMP            Initialize(LPCITEMIDLIST pIDFolder, LPDATAOBJECT pDataObj, HKEY hKeyID);

    //IShellPropSheetExt methods
    STDMETHODIMP            AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
    STDMETHODIMP            ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam);

    //IContextMenu methods
    STDMETHODIMP            QueryContextMenu(HMENU hMenu,
                                             UINT indexMenu, 
                                             UINT idCmdFirst,
                                             UINT idCmdLast, 
                                             UINT uFlags);

    STDMETHODIMP            InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi);

    STDMETHODIMP            GetCommandString(UINT_PTR idCmd, 
                                             UINT uFlags, 
                                             UINT *reserved, 
                                             LPSTR pszName, 
                                             UINT cchMax);
    // IShellConfigPage methods
    STDMETHODIMP            CreateConfigurationWindow(PTSTR pszFile, HWND hParent, HWND *phwnd);
    STDMETHODIMP            SetActivate(bool bActive);

};

