// MODULE:		ShellExt.cpp
//
// Purpose:		Implements the class factory code
//				as well as CShellExt code

#include "StdAfx.h"			// Header précompilé
#include "..\Resources\Resources.h"	// Resources localisés

// Initialize GUIDs (should be done only and at-least once per DLL/EXE)
#pragma data_seg(".text")
#define INITGUID
#include <initguid.h>	// Outils d'initialisation de GUID
#include <objbase.h>
#include <shlguid.h>	// Définitions des GUIDs standards du Shell
#include "ShellExt.h"	// Définition des classes CShellExtClassFactory et CShellExt
#pragma data_seg()

#include "PETools.h"		// Outils d'exploration d'image PE
#include "DLGRetry.h"		// Boîte de dialogue Réessayer
#include "DLLfuncs.h"

CShellExtClassFactory::CShellExtClassFactory()
{
    m_cRef = 0L; 
    g_cRefThisDll++;	
}

CShellExtClassFactory::~CShellExtClassFactory()				
{
    g_cRefThisDll--;
}

STDMETHODIMP CShellExtClassFactory::QueryInterface(REFIID riid, PVOID FAR *ppv)
{
    *ppv = NULL;

    // Any interface on this object is the object pointer
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
    {
        *ppv = (LPCLASSFACTORY)this;
        AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CShellExtClassFactory::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CShellExtClassFactory::Release()
{
    if (--m_cRef)
        return m_cRef;

    delete this;
    return 0L;
}

STDMETHODIMP CShellExtClassFactory::CreateInstance(LPUNKNOWN pUnkOuter,
                                                   REFIID riid,
                                                   PVOID *ppvObj)
{
    *ppvObj = NULL;

    // Shell extensions typically don't support aggregation (inheritance)
    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    // Create the main shell extension object.  The shell will then call
    // QueryInterface with IID_IShellExtInit--this is how shell extensions are
    // initialized.

    //Create the CShellExt object
    CShellExt *pShellExt = new CShellExt();

    if (NULL == pShellExt)
        return E_OUTOFMEMORY;

    return pShellExt->QueryInterface(riid, ppvObj);
}


STDMETHODIMP CShellExtClassFactory::LockServer(BOOL fLock)
{
    return NOERROR;
}

// *********************** CShellExt *************************
CShellExt::CShellExt()
{
    // Initialise les variables membres
    m_cRef = 0L;
    m_pDataObj = NULL;
    g_cRefThisDll++;

    m_szPEFileName[0] = _T('\0');
    m_szPEFileDir[0] = _T('\0');
    m_pLoaderFile = NULL;

    memset(&m_stCreation, 0, sizeof(SYSTEMTIME));
    memset(&m_stModification, 0, sizeof(SYSTEMTIME));	

    m_bChanged = false;
    m_bDLL = false;

    m_bCracked = false;
    m_lpld = NULL;

    m_hPage = NULL;
    m_hTab = NULL;

    m_bShowAdvanced = false;
    m_bConfigurationMode = false;

    m_current = NULL;

    m_general.m_pcs = this;
    m_options.m_pcs = this;
    m_moreoptions.m_pcs = this;
    m_depend.m_pcs = this;
    m_help.m_pcs = this;

    m_bModifByProgram = false;

    m_pOrgWinProc = NULL;

    // charge les elements traduits
    OnLanguageChanged();

    // Chargement de la police unicode, ceci est necessaire car le control tab n'est pas capable de trouver les symboles unicodes Coreens quand les fonts de base (arial, ms sans serif,...) sont utilisees (alors que ca marche parfaitement pour tous les autres controles).    
    PTSTR pszFont;
    int fontsizeInPoint = DEFAULT_UNICODE_FONTSIZE;
    PTSTR pszResFont = LoadResString(g_hResDll, IDS_DLGFONT);
    if( pszResFont ) {
        pszFont = pszResFont;
        PTSTR comma = _tcschr(pszResFont, ',');
        if( comma ){
            *comma = '\0';
            fontsizeInPoint = _wtoi(comma+1);
        }
    }
    else
        pszFont = DEFAULT_UNICODE_FONTFACE;

    HDC hDC = GetDC(NULL);
    int nHeight = MulDiv(fontsizeInPoint, GetDeviceCaps(hDC, LOGPIXELSY), 72);
    int res = ReleaseDC(NULL, hDC);

    m_hfUnicode = CreateFont(  nHeight,  0,  0, 0,
            FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, FF_SWISS|DEFAULT_PITCH, pszFont);

    if( pszResFont )
        FreeResString(pszResFont);
}

CShellExt::~CShellExt()
{	
    // libere la police unicode
    DeleteObject(m_hfUnicode);

    // Libére de la mémoire la liste des fichiers dépendants
    delete m_lpld;

    if (m_pDataObj)
        m_pDataObj->Release();

    g_cRefThisDll--;
}

STDMETHODIMP CShellExt::QueryInterface(REFIID riid, PVOID FAR *ppv)
{
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IShellExtInit) || IsEqualIID(riid, IID_IUnknown))
        *ppv = (LPSHELLEXTINIT)this;
    else if (IsEqualIID(riid, IID_IContextMenu))
        *ppv = (LPCONTEXTMENU)this;
    else if (IsEqualIID(riid, IID_IShellPropSheetExt))
        *ppv = (LPSHELLPROPSHEETEXT)this;
    else if (IsEqualIID(riid, IID_IShellConfigPage))
        *ppv = (LPSHELLCONFIGPAGE)this;

    if (*ppv)
    {
        AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CShellExt::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CShellExt::Release()
{
    if (--m_cRef)
        return m_cRef;

    delete this;
    return 0L;
}

//
//	FUNCTION:	CShellExt::Initialize(LPCITEMIDLIST, LPDATAOBJECT, HKEY)
//
//	PURPOSE:	Called by the shell when initializing a context menu or property
//				sheet extension.
//
//	PARAMETERS:
//		pIDFolder - Specifies the parent folder
//		pDataObj  - Spefifies the set of items selected in that folder.
//		hRegKey   - Specifies the type of the focused item in the selection.
//
//	RETURN VALUE:
//
//		NOERROR in all cases.
//
//	COMMENTS:	Note that at the time this function is called, we don't know 
//				(or care) what type of shell extension is being initialized.  
//				It could be a context menu or a property sheet.
//

STDMETHODIMP CShellExt::Initialize (LPCITEMIDLIST pIDFolder,
                                    LPDATAOBJECT pDataObj,
                                    HKEY hRegKey)
{
    // Initialize can be called more than once
    if (m_pDataObj)
        m_pDataObj->Release();

    // duplicate the object pointer and registry handle
    if (pDataObj)
    {
        m_pDataObj = pDataObj;
        pDataObj->AddRef();
    }

    return NOERROR;
}

// Retourne true si le fichier PE a ete ouvert avec succes.
BOOL CShellExt::SetFile(PCTSTR pszFile // path du fichier PE a ouvrir
                        )
{
    WORD		wCharacteristics;
    PCTSTR		pszFilePart;
    FILETIME	ftCreation, ftModification;

    // Ouvre le fichier
    HANDLE	hFile = CreateFile(	pszFile,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL );

    if ( hFile == INVALID_HANDLE_VALUE )
        return FALSE;

    pszFilePart = GetFileBaseNamePart(pszFile);

    GetFileTime(hFile, &ftCreation, NULL, &ftModification);
    FileTimeToSystemTime(&ftCreation, &m_stCreation);
    FileTimeToSystemTime(&ftModification, &m_stModification);

    // vérifie que l'image est au format PE et qu'il ne s'agit pas d'un fichier backup
    if( (ImageType(hFile, &wCharacteristics) != IMAGE_NT_SIGNATURE) ||
        !_tcsncmp(pszFile + _tcslen(pszFile) - BACKUP_EXT_SIZE, BACKUP_EXT, BACKUP_EXT_SIZE) ||
        !_tcsicmp(pszFilePart, CL_SHELLEXT_DLL) ||
        !_tcsicmp(pszFilePart, CL_KERNEL_DLL) ||
        !_tcsicmp(pszFilePart, CL_LOADER_EXE) || 
        !_tcsicmp(pszFilePart, CL_MANAGER_EXE) || 
        !_tcsicmp(pszFilePart, CL_INJECTOR_EXE)
        )
    {
        // Ferme le fichier
        CloseHandle( hFile );
        return FALSE;
    }

    //////
    // Initialise les variables membres

    // le fichier PE est-il une DLL ?
    m_bDLL = (wCharacteristics & IMAGE_FILE_DLL) == IMAGE_FILE_DLL;

    // le fichier est-il cracké ?
    m_bCracked = IsFileCracked(hFile);

    // Ferme le fichier
    CloseHandle( hFile );

    // Chemin d'acces au fichier
    _tcscpy_s(m_szPEFileName, pszFile);

    // Obtient le repertoire du fichier sélectionné
    _tcscpy_s(m_szPEFileDir, m_szPEFileName);
    TruncateFilenameToFileDirectory(m_szPEFileDir);
    return TRUE;
}

BOOL CShellExt::GetFileName(PTSTR pszFileName, int cbMax)
{
    FORMATETC fmte =	{	CF_HDROP,
        (DVTARGETDEVICE FAR *)NULL,
        DVASPECT_CONTENT,
        -1,
        TYMED_HGLOBAL
    };
    STGMEDIUM	medium;
    HRESULT		hres = 0;

    if (m_pDataObj)  // Paranoid check, m_pDataObj should have something by now...
        hres = m_pDataObj->GetData(&fmte, &medium);

    // Find out how many files the user has selected...
    if( SUCCEEDED(hres) && (medium.hGlobal) && (DragQueryFile((HDROP)medium.hGlobal, (UINT)-1, 0, 0) == 1) )
    {
        //OK, the user has only selected a single file, so lets go ahead
        //and add the property sheet.

        //Get the name of the file the user has clicked on
        DragQueryFile((HDROP)medium.hGlobal, 0, pszFileName, cbMax);
        return TRUE;
    }
    else
        return FALSE;
}

//  FUNCTION: CShellExt::AddPages(LPFNADDPROPSHEETPAGE, LPARAM)
//
//  PURPOSE: Called by the shell just before the property sheet is displayed.
//
//  PARAMETERS:
//    lpfnAddPage -  Pointer to the Shell's AddPage function
//    lParam      -  Passed as second parameter to lpfnAddPage
//
//  RETURN VALUE:
//
//    NOERROR in all cases.  If for some reason our pages don't get added,
//    the Shell still needs to bring up the Properties... sheet.
STDMETHODIMP CShellExt::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
    TCHAR szFile[_MAX_PATH];
    PROPSHEETPAGE psp;
    HPROPSHEETPAGE hpage;

    if( GetFileName(szFile, _MAX_PATH) )
    {
        if( !SetFile(szFile) )
            return NOERROR;

        // Create a property sheet page object from a dialog box.
        //
        // We store a pointer to our class in the psp.lParam, so we
        // can access our class members from within the PageDlgProc
        psp.dwSize      = sizeof(psp);	// no extra data.
        psp.dwFlags     = PSP_USEREFPARENT | PSP_USETITLE; // | PSP_USECALLBACK;
        psp.hInstance   = g_hResDll;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_PROPEXT);
        psp.hIcon       = 0;
        psp.pszTitle = S_TABTITLE;
        psp.pfnDlgProc  = (DLGPROC)&PageDlgProc;
        psp.pcRefParent = &g_cRefThisDll;
        //psp.pfnCallback = PageCallback;
        psp.lParam      = (LPARAM)(CShellExt *)this;

        AddRef();
        hpage = CreatePropertySheetPage(&psp);

        if (hpage)
        {
            if (!lpfnAddPage(hpage, lParam)) 
            {
                DestroyPropertySheetPage(hpage);
                Release();
            }
        }
        else
            Release();
    }

    return NOERROR;
}

STDMETHODIMP CShellExt::CreateConfigurationWindow(PTSTR pszFile, HWND hParent, HWND *phwnd)
{
    PROPSHEETPAGE psp;

    if( !SetFile(pszFile) )
        return E_FAIL;

    psp.lParam = (LPARAM)(CShellExt *)this;

    // affecte la police unicode a tous les controles contenus dans la fenetre parente. (cette fenetre correspond a la fentre de config du manager, elle ne contient pour l'instant que 3 boutons)
    //EnumChildWindows(hParent, EnumChildSetFont, (LPARAM)m_hfUnicode);


    m_bConfigurationMode = TRUE;
    *phwnd = CreateDialogParam(g_hResDll, MAKEINTRESOURCE(IDD_PROPEXT), hParent, PageDlgProc, (LPARAM)&psp);



    if( ! *phwnd )
        return E_FAIL;

    ShowWindow(*phwnd, SW_SHOW);
    AddRef();
    return NOERROR;
}

STDMETHODIMP CShellExt::SetActivate(bool bActive)
{
    m_appSettings.m_bActive = bActive;
    m_current->LoadControlData();
    OnModification();
    return NOERROR;
}

//  FUNCTION: CShellExt::ReplacePage(UINT, LPFNADDPROPSHEETPAGE, LPARAM)
//
//  PURPOSE: Called by the shell only for Control Panel property sheet 
//           extensions
//
//  PARAMETERS:
//    uPageID         -  ID of page to be replaced
//    lpfnReplaceWith -  Pointer to the Shell's Replace function
//    lParam          -  Passed as second parameter to lpfnReplaceWith
//
//  RETURN VALUE:
//t support this function.  It should never be
//    called.
STDMETHODIMP CShellExt::ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam)
{	return E_FAIL;	}


// Fonction appelée lorsque l'utilisateur à modifié quelque chose
// qui doit être confirmé par le bouton 'Apply'
void CShellExt::OnModification()
{
    // Active le bouton 'apply' si ce n'est pas le programme
    // lui-même qui modifie la date ou l'heure...
    if( ! m_bModifByProgram ) {
        m_bChanged = true;

        // Active le bouton 'apply'
        PropSheet_Changed( GetParent(m_hPage), m_hPage );
    }
}

// Fonction appelée lorsque une feuille a été sélectionée dans le
// contrôle tab
void CShellExt::OnSelChanged()
{
    TCITEM tci;

    // cache la feuille courrante	
    if( m_current )
    {
        m_current->hide();
        m_current->SaveControlData();
    }

    tci.mask = TCIF_PARAM;
    TabCtrl_GetItem(m_hTab, TabCtrl_GetCurSel(m_hTab), &tci);

    m_current = (CTabSheet *)tci.lParam;

    // affiche la nouvelle feuille sélectionnée
    if( m_current )
    {
        m_current->LoadControlData();
        m_current->show();
    }

}

// Fonction appelée lorsque l'on veut afficher ou effacer le contrôle tab
void CShellExt::ShowTab(BOOL bShowTab)
{
    RECT rc;

    ////////////////////
    // affiche/efface le contrôle tab
    ShowWindow(m_hTab, bShowTab ? SW_SHOW : SW_HIDE);

    ////////////////////
    // Redimensionne les pages
    GetWindowRect(m_hTab, &rc);
    ScreenToClient(m_hPage, (PPOINT)&rc);
    ScreenToClient(m_hPage, (PPOINT)&rc.right);

    // le contrôle tab est-il affiché ?
    if( bShowTab )
    {
        TabCtrl_AdjustRect(m_hTab, FALSE, &rc); 
    }

    m_general.move(&rc);
    m_depend.move(&rc);
    m_help.move(&rc);
    m_options.move(&rc);
    m_moreoptions.move(&rc);
}

void CShellExt::OnInit(HWND hDlg)
{
    RECT rc;

    // Charge le paramètrage à partir de la reg
    RegLoadSettings();

    // handle de la fenetre de l'onglet shellex-windows intitule "Cracklock"
    m_hPage = hDlg;


    // Creation du contrôle Tab	
    GetClientRect(m_hPage, &rc);	
    m_hTab  = CreateWindow(WC_TABCONTROL, _T(""),  			
        WS_CHILDWINDOW | WS_VISIBLE  | WS_TABSTOP | TCS_HOTTRACK,
        5, 5, rc.right-10, rc.bottom-10, 
        m_hPage, NULL, GetModuleHandle(NULL), NULL); 

    // affecte la police unicode au control tab
    SendMessage(m_hTab, WM_SETFONT, (WPARAM)m_hfUnicode, MAKELPARAM(TRUE,0));

    // Crée la liste des fichiers dépendants
    m_lpld = new CNodes;	


    char modulename[_MAX_PATH]; // c'est bien char et non pas TCHAR!!!

#ifdef _UNICODE
    // convertit le nom du fichier au format ANSI dans le format unicode
    size_t c;
    wcstombs_s(&c,modulename, _countof(modulename), GetFileBaseNamePart(m_szPEFileName), _countof(modulename) );
#else
    _tcscpy_s(modulename, GetFileBaseNamePart(m_szPEFileName));
#endif

    // Énumère les modules d'imports (afin de les ajouter à la liste)
    EnumDep( modulename, NULL, (LPARAM)this, NULL );

    // Charge les parametres pour ce fichier cracké
    if( !LoadAppSetting() )
    {
        // Les informations pour ce programme ne sont pas disponibles
        // ce programme n'a donc pas encore été cracké
        // Charge les infos par défaut
        LoadDefaultAppSetting();
    }

    // Ajoute les onglets au contrôle tab et crée
    // les fenêtres filles correspondant à chacun des onglets
#if _DEBUG 
    m_general.TabInsert(hDlg, m_hTab, g_S_TABGENERAL, 0, TRUE);
    m_bShowAdvanced = true;
#else
    m_general.TabInsert(hDlg, m_hTab, g_S_TABGENERAL, 0, !m_bConfigurationMode);
#endif
    m_options.TabInsert(hDlg, m_hTab, g_S_TABOPTIONS, 1, TRUE);	
    m_depend.TabInsert(hDlg, m_hTab, g_S_TABDEPEND, 2, TRUE);	
    m_moreoptions.TabInsert(hDlg, m_hTab, g_S_TABMOREOPTIONS, 3, TRUE);	
    m_help.TabInsert(hDlg, m_hTab, g_S_TABHELP, 4, TRUE);

    // Affiche ou efface le ctrl tab
    ShowTab((m_bShowAdvanced || m_bConfigurationMode)
        ////// Temporaire: la version 3.0 ne crack pas	////
        && !m_bDLL
        ////// encore les DLL indépendament				////
        );



    if( m_bDLL )
        m_depend.TabSelect();
    else if( m_bConfigurationMode )
        m_options.TabSelect();
    else
        m_general.TabSelect();

    // la séléction à changée
    OnSelChanged();
}

void CShellExt::OnDestroy()
{
    // Sauve le paramètrage dans la reg
    RegSaveSettings();

    Release();
}

// Charge le paramètrage à partir de la reg
void CShellExt::RegLoadSettings()
{
    m_bShowAdvanced = false; // do not show the advanced option by default
	GetCracklockSettingValue(REGVALUE_SETTINGS_ADVANCED, (DWORD *)&m_bShowAdvanced);
}

// Sauve le paramètrage dans la reg
void CShellExt::RegSaveSettings()
{
    // Si l'option d'affichage des options avancées a été modifiée par l'utilisateur alors enregistre la
    if( !m_bConfigurationMode )
		SetCracklockSettingValue(REGVALUE_SETTINGS_ADVANCED, (DWORD)m_bShowAdvanced);
}

// Function name	: LoadDefaultAppSetting
// Description	    : Charge les infos par défaut pour un programme pas encore cracké
// Parameter        : appset structure ou seront stockes les parametres lus
// Return type		: void
void CShellExt::LoadDefaultAppSetting()
{
    // charge les parametres par defaut
    m_appSettings.LoadDefault( m_szPEFileName );

    // si le fichier "app.exe.cracklock" existe alors charge les parametres a partir de ce fichier
    SettingSection *sec = NULL;
    TCHAR inipath[_MAX_PATH];
    InifilenameFromApp(m_szPEFileName, inipath, _countof(inipath));
    if( FileExists(inipath) ) {
        sec = new SettingSection(SectionStorageLocation(inipath));
        if( sec && sec->Open(REGVALUE_PROG_LOCALSETTINGSECTION) ) {
            LoadAppSetting(sec);
        }
        delete sec;
    }

    // fichier PE de chargement
    if( m_appSettings.szLoaderFile[0] ) // mode static
        // find the loader file in the list of dependencies
        m_pLoaderFile = m_lpld->Find(m_appSettings.szLoaderFile);
        // if the main exe PE file is not crackable then back up to a run-time injection 
        // by default.
        if( m_pLoaderFile && !m_pLoaderFile->m_bCrackable )
            m_pLoaderFile = NULL; 
        /*
        {
        // sélectionne le premier fichier crackable dans la liste des dépendances
        m_pLoaderFile = NULL;
        for(CNode *pEnumNode = m_lpld->Find(NULL);
            pEnumNode != NULL;
            pEnumNode = m_lpld->Next(pEnumNode)
            ) {
            if( pEnumNode->m_bCrackable ) {
                m_pLoaderFile = pEnumNode;
                break;			
            }
        }
        }*/
    else // mode run-time
        m_pLoaderFile = NULL; // n'utilise pas de fichier chargeur    

    
    // Corrige le flag d'activation si il s'avere que le fichier PE
    // n'est pas une DLL et qu'il est cracke.
    m_appSettings.m_bActive = m_bDLL ? false
                                     : m_appSettings.m_bActive || m_bCracked;
}


// Function name: LoadAppSetting
// Description: Charge à partir d'une section de la registry/INI les infos correspondants à une application cracké
// Parametres: sec pointes sur la section a lire. Si NULL a lors la section est lue a partir des parametres generaux
// de Cracklock (en prenant en compte les forward potentiels sur un fichier INI local)
//
// Retourne true si la chagement s'est bien deroule
bool CShellExt::LoadAppSetting( SettingSection *sec )
{
    bool secCreateHere = false; // record if the section was created by this function or if it was given in parameter

    // si aucune section n'est specifiee
    if( sec == NULL ) {
        // alors ouvre par defaut la section de l'application contenue dans les parametres principaux de Cracklock.
        sec = CLCommonSettingSection::GetSectionOfApp(m_szPEFileName, m_bDLL, false);
        secCreateHere = true;
    }

    if(!sec)
        return false;

    // read the section
    m_appSettings.LoadFromSection( m_szPEFileName, sec );

    if( secCreateHere )
        CLCommonSettingSection::deleteObject(sec);


    // find the loader file in the list of dependencies
	if( m_appSettings.szLoaderFile[0] )
        m_pLoaderFile = m_lpld->Find(m_appSettings.szLoaderFile);
    else
        m_pLoaderFile = NULL;


    // Corrige le flag d'activation si il s'avere que le fichier PE
    // n'est pas une DLL et qu'il est cracke.
    m_appSettings.m_bActive = m_bDLL ? false
                                     : m_appSettings.m_bActive || m_bCracked;

    return true;
}


// Function name: SaveAppSetting 
// Description: Enregistre dans la registry/INI les infos correspondants à un programme cracké 
// Retourne true si l'enregistrement s'est bien deroule
bool CShellExt::SaveAppSetting()
{
    // Ouvre la clé correspondant au fichier à cracker
	SettingSection *sec = CLCommonSettingSection::GetSectionOfApp(m_szPEFileName, m_bDLL, true);
    if ( sec ) {
        // si les parametres doivent etres enregistes en local alors...
        if( m_appSettings.m_bLocalStorage || m_appSettings.m_bStandalone ) {
            // indique dans les settings general que les informations pour cette application
            // se trouve dans un fichier de config .cracklock local a l'application
            sec->SetValue(REGVALUE_PROG_FORWARD, true );

            // reouvre la section a partir du fichier .cracklock local
            CLCommonSettingSection::deleteObject(sec);
            if( !(sec = CLCommonSettingSection::GetSectionOfApp(m_szPEFileName, m_bDLL, true)) )
                return false;            
        }
    
        // description du programme
		sec->SetValue(REGVALUE_PROG_DESCRIPTION, m_appSettings.m_szProgDescr);
        // la date        
		sec->SetValue(REGVALUE_PROG_DATE, &m_appSettings.m_ftDateTime);
        // la timezone
		sec->SetValue(REGVALUE_PROG_TIMEZONE, &m_appSettings.m_tzi);
        // mode independant?
        sec->SetValue(REGVALUE_PROG_STANDALONE, m_appSettings.m_bStandalone);
        // le nom du fichier chargeur
		sec->SetValue(REGVALUE_PROG_LOADERFILE, m_pLoaderFile ? (PCTSTR)m_pLoaderFile->m_pszFilePath : _T(""));
        // parametres d'execution
        sec->SetValue(REGVALUE_PROG_CMDPARAMETERS, m_appSettings.m_szCmdParameters);

        // CALCUL LE DWORD QUI ENCODE LE MODE DE CRACKAGE
        DWORD dwFlags = 0;
        // Crack activé ?
        if( m_appSettings.m_bActive )		dwFlags |= CLREG_CRACK;
        // mode système ?
        if( m_appSettings.m_bOptSystem )	dwFlags |= CLREG_SYSTEM;
        // mode constant ?
        if( m_appSettings.m_bOptConstant )	dwFlags |= CLREG_CONSTANT;
        // Porté du patch ? (date et/ou heure)
        if( m_appSettings.m_bDateMode )		dwFlags |= CLREG_VIRTUALMODE_DATE;
        if( m_appSettings.m_bTimeMode )		dwFlags |= CLREG_VIRTUALMODE_TIME;
        if( m_appSettings.m_bTimeZoneMode )	dwFlags |= CLREG_VIRTUALMODE_TIMEZONE;
        // le flag de configuration (contient le mode date/time/tz)
        sec->SetValue(REGVALUE_PROG_FLAGS, dwFlags);

        CLCommonSettingSection::deleteObject(sec);
        return true;
    }

    return false;
}

///////////////////////
//
//	FONCTION: Supprime l'entree correspondant au fichier PE sélectionné.
//	Retourne true si la suppression a reussie.
bool CShellExt::DeleteAppSetting()
{
    // Ouvre la section contenant la liste des fichiers crackés
	CLCommonSettingSection sec;
    if( sec.Open(m_bDLL ? REGKEY_MBCL_DLLS : REGKEY_MBCL_APPS) ) {
	    TCHAR szKeyName[_MAX_PATH];
		if( sec.SubsectionNameFromApp(m_szPEFileName, szKeyName, _countof(szKeyName)) )
            return sec.DeleteSubsection(szKeyName);
		else
			return false;
    }
	else 
		return false;

}

// Description	    : Retourne true si le fichier passé en paramètre
//					  doit être cracké.
// Argument         : CNode *pNode
bool CShellExt::NeedToBeCrack(CNode *pNode)
{
    // Si Cracklock est activé et que ce fichier est le fichier de
    // chargement OU si le fichier est utilisé par une autre application
    // alors ce fichier a besoin d'être cracké.
    return  ( m_appSettings.m_bActive && (pNode == m_pLoaderFile) )
            ||  pNode->m_bUsedByAnotherApp;
}

///////
// Fonction appelée lorsque l'utilisateur à cliquer sur le bouton OK ou Apply.
//
// Applique les modifications efféctuées par l'utilisateur.
BOOL CShellExt::OnApply()
{
    if( !m_bChanged )
        return TRUE;

    m_current->SaveControlData();

    bool bProceedAfterReboot = false,
         bRebootNow = false;
    do {
        // Liste des fichiers qui n'ont pas pu etre modifies
        CNodes lockedfiles;

        // Met a jour chaque fichier de la liste des dépendances
        for(CNode *pEnumNode = m_lpld->Find(NULL);
            pEnumNode != NULL;
            pEnumNode = m_lpld->Next(pEnumNode)) {
            bool bNeedCrack = NeedToBeCrack(pEnumNode);
            if( pEnumNode->m_bCracked != bNeedCrack) {
                // Effectue les modifications necessaires sur le fichier
                if( !pEnumNode->RefreshCrack(bNeedCrack, bProceedAfterReboot) ) {
                    // les modifications ont echouees
                    lockedfiles.AddItem(pEnumNode->m_pszFilePath);
                }
            }
        }

        tstring srcdll = tstring(g_szCLPath) + _T("\\") + DIR_CL_BIN + _T("\\") + CL_KERNEL_DLL,
                targetdll = tstring(m_szPEFileDir) + CL_KERNEL_DLL,
                srcloader = tstring(g_szCLPath) + _T("\\") + DIR_CL_BIN + _T("\\") + CL_LOADER_EXE,
                targetloader = tstring(m_szPEFileDir) + CL_LOADER_EXE;
                
        // verifie que le programme a cracker ne se trouve pas dans le repertoire Bin de Cracklock
        if( _tcsicmp(targetdll.c_str(), srcdll.c_str()) ) {
            // injection statique?
            bool bStatic = m_pLoaderFile != NULL;
            bool bInstCLKern, bInstMCL, bInstShortcut;
            
            // Activating or deactiving the crack?
            if( m_appSettings.m_bActive ) {
                // Si en mode standalone alors
                if( m_appSettings.m_bStandalone ) {
                    bInstCLKern = true;
                    bInstMCL = !bStatic;
                    bInstShortcut = !bStatic;
                }
                // Si en mode non stand-alone
                else {
                    bInstCLKern = false;
                    bInstMCL = false;
                    bInstShortcut = false;
                }
            }
            // deactivating the crack on this application
            else {
                bInstCLKern = false;
                bInstMCL = false;
                bInstShortcut = false;
                // supprime le fichier app.exe.cracklock
                TCHAR settingfile[_MAX_PATH];
                InifilenameFromApp(m_szPEFileName, settingfile, _countof(settingfile));
                if( !DeleteFileReboot(settingfile, bProceedAfterReboot) 
                    && GetLastError() != ERROR_FILE_NOT_FOUND ) {
                    lockedfiles.AddItem(settingfile);
                }
            }

            if( bInstCLKern ) {
                // copie CLKERN.dll dans le repertoire de l'application
                if( !UpdateDll( srcdll.c_str(), targetdll.c_str(), bProceedAfterReboot ) ) {
                    lockedfiles.AddItem(targetdll.c_str());                
                }
            }
            else {
                // supprime le fichier CLKERN.DLL du repertoire de l'application si il existe
                if( !DeleteFileReboot(targetdll.c_str(), bProceedAfterReboot) 
                    && GetLastError() != ERROR_FILE_NOT_FOUND ) {
                    lockedfiles.AddItem(targetdll.c_str());
                }
            }

            if( bInstMCL ) {
                // copie le charger (CL_LOADER_EXE) dans le repertoire de l'application                    
                if(!UpdateDll(srcloader.c_str(), targetloader.c_str(), bProceedAfterReboot))
                    lockedfiles.AddItem(targetloader.c_str());
            }
            else {
                // supprime le fichier CL_LOADER_EXE du repertoire de l'application si il existe
                if( !DeleteFileReboot(targetloader.c_str(), bProceedAfterReboot) 
                    && GetLastError() != ERROR_FILE_NOT_FOUND ) {
                    lockedfiles.AddItem(targetloader.c_str());
                }
            }
            if( bInstShortcut ) {
                // cree un raccourci local
                CreateLocalLoaderShortcut();
            }
            else {
                // supprime le raccourci local
                if( PTSTR pszPathLink = GetShortcutFilePath(m_szPEFileDir) ) {
                    if( !DeleteFileReboot(pszPathLink, bProceedAfterReboot) 
                        && GetLastError() != ERROR_FILE_NOT_FOUND ) {
                        lockedfiles.AddItem(pszPathLink);
                    }
                    free(pszPathLink);
                }
            }

        }

        
        // Si tous les fichiers ont pu etre mis a jour alors sort de la boucle
        if( lockedfiles.GetItemCount() == 0 )
            break;

        else { // Si au moins un fichier n'a pas pu etre modifie...
            DLGRETRYPARAM param;
            param.hFont = this->m_hfUnicode;
            param.files = &lockedfiles;

            // ...alors affiche la boîte de dialogue DLGRetry
            switch(DialogBoxParam(g_hResDll, MAKEINTRESOURCE(IDD_RETRY), m_hPage,
                    (DLGPROC) RetryDlgProc, (LPARAM)&param) ) {
            case IDCANCEL:				// ANNULER
                return FALSE;
                
            case IDC_BTRESTARTNOW:		// RÉSSAYER APRÉS UN REBOOT IMMEDIAT
                bProceedAfterReboot = true;
                bRebootNow = true;
                break;
            case IDC_BTRESTARTLATER:	// RÉSSAYER APRÉS UN REBOOT PLUS TARD
                bProceedAfterReboot = true;
                bRebootNow = false;
                break;
            case IDC_BTRETRY:	// RÉESSAYER
                break;
            }
        }
    }
    while( 1 );        // Recommence tant qu'il y a une erreur
                       // et tant que l'utilisateur n'a pas annulé.


    // L'operation s'est terminee sans etre annulee:

    if( m_appSettings.m_bActive )
        // Cracklock a été activé: enregistre dans la registry les
        // infos concernant le programme cracké
        SaveAppSetting();
    else
        // Cracklock a été désactivé: supprime la clé correspondant
        // au programme
        DeleteAppSetting();


    m_bChanged = false;

    // Certains fichiers doivent etre remplaces au reboot de la machine
    if( bProceedAfterReboot ) {
        // Windows NT
        if ( g_dwPlatformId == VER_PLATFORM_WIN32_NT ) {
            HANDLE hToken;
            TOKEN_PRIVILEGES tkp;  // Obtient un token pour ce process

            // Obtient le LUID pour le privilege shutdown
            OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);

            LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);  
            tkp.PrivilegeCount = 1;  // un seul privilege requis
            tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;  

            // Obtient le privilege shutdown pour ce process.  
            AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);  

            // Shutdown le system et force toutes les applications a terminer.  
            if( bRebootNow )
                ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0);
        }
        // Windows 95
        else if ( g_dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ) {
            // Ajoute dans la registry une commande de restoration des NFL qui 
            // s'exécutera au prochain démarrage
            PTSTR   pTitre = LoadResString(g_hResDll, IDS_TITRESTORELFN),
                    pCmdLn = LoadResString(g_hResDll, IDS_CMDRESTORELFN);

            HKEY hKeyRunOnce;
            RegOpenKeyEx(HKEY_BASE, REGKEY_EXPLORER_RUNONCE, 0, KEY_ALL_ACCESS, &hKeyRunOnce);
            RegSetValueEx(hKeyRunOnce, pTitre, NULL, REG_SZ, (PBYTE)pCmdLn, 
                sizeof(TCHAR)*(_tcslen(pCmdLn)+1));
            RegCloseKey(hKeyRunOnce);

            FreeResString(pCmdLn);
            FreeResString(pTitre);

            if( bRebootNow )
                ExitWindowsEx(EWX_REBOOT, 0); // Reboot Windows
        }
    }

    return TRUE;
}

INT_PTR CALLBACK PageDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    CShellExt *pcs = (CShellExt *)GetWindowLong(hDlg, DWLP_USER);

    switch (uMessage)
    {
        // When the shell creates a dialog box for a property sheet page,
        // it passes the pointer to the PROPSHEETPAGE data structure as
        // lParam. The dialog procedures of extensions typically store it
        // in the DWL_USER of the dialog box window.

        // --- NB --- Au lieu de stocker dans la zone DWL_USER
        // un pointeur sur cette structure inutile (PROPSHEETPAGE)
        // je stocke un pointeur sur la classe CSHELLEXT obtenu
        // dans le champs lParam de la structure PROPSHEETPAGE
    case WM_INITDIALOG:
        pcs = (CShellExt *)((LPPROPSHEETPAGE)lParam)->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pcs );


        pcs->OnInit(hDlg);
        return TRUE;

    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code)
        {
        case PSN_SETACTIVE:
            SetWindowLong(hDlg, DWLP_MSGRESULT, 0);
            return TRUE;

        case PSN_APPLY: // User has clicked the OK or Apply button
            SetWindowLong(hDlg, DWLP_MSGRESULT,
                ( pcs->OnApply() )
                ? PSNRET_NOERROR
                : PSNRET_INVALID_NOCHANGEPAGE );
            return TRUE;

        case TCN_SELCHANGE: 
            pcs->OnSelChanged();
            return TRUE;
        }
        break;

    case WM_CHAR:
        if( (TCHAR)wParam == VK_F1 )
        {
            InvokeHelp(hDlg, -1);
            SetWindowLong(hDlg, DWLP_MSGRESULT, 0);
            return TRUE;
        }
        break;

    case WM_DESTROY:
        pcs->OnDestroy();
        SetWindowLong(hDlg, DWLP_MSGRESULT, 0);
        return TRUE;
    }

    return FALSE;
}

// Fonction d'énumération de la table d'import du fichier EXE
// (pour chaque entrée, ajoute un item à la liste des dépendances)
BOOL CALLBACK EnumDep(PSTR pszName, DWORD dwOffset, LPARAM lParam, LPARAM lParam2)
{
    CShellExt *pcs = (CShellExt *)lParam;
    HANDLE  hFile;
    TCHAR   szPATH[_MAX_PATH];
    PTSTR   lpFilePart;
    CNode   *pEnumNode;

    TCHAR   tszName[_MAX_PATH];

#ifdef _UNICODE
    // convertit le nom du fichier au format ANSI dans le format unicode
    size_t c;
    mbstowcs_s(&c, tszName, _countof(tszName), pszName, _countof(tszName));
#else
    _strcpy_s(tszName, pszName);
#endif


    // Regarde si l'item n'a pas déjà été ajouté
    for(pEnumNode = pcs->m_lpld->Find(NULL);
        pEnumNode != NULL;
        pEnumNode = pcs->m_lpld->Next(pEnumNode))
        if( _tcsicmp(pEnumNode->m_pszBaseName, tszName) == 0 )
            return TRUE;

    // Cherche l'emplacement du fichier
    bool bExist = SearchPath(pcs->m_szPEFileDir, tszName, NULL, _MAX_PATH-1, szPATH, &lpFilePart)
            || SearchPath(NULL, tszName, NULL, _MAX_PATH-1, szPATH, &lpFilePart);

    // Ouvre le fichier
    hFile = CreateFile( szPATH, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

    // N'ajoute pas CLKERN.DLL à la liste
    if( _tcsicmp(CL_KERNEL_DLL, tszName) != 0) {
        // Créé un item pour ce fichier    
        pEnumNode = pcs->m_lpld->AddItem( bExist ? szPATH : tszName);

        if( pEnumNode ) {            
            pEnumNode->m_bExist = bExist; // indique si le fichier existe ou non

            if( pEnumNode->IsWinSysFile() )
                pEnumNode->m_bCrackable = FALSE;
            else {
                if( IsFileCracked(hFile) ) {
                    pEnumNode->m_bCrackable = TRUE;
                    pEnumNode->m_bCracked = TRUE;
                    pEnumNode->m_bUsedByAnotherApp = pEnumNode->IsUsedByAnotherApp(pcs->m_szPEFileName);
                }
                else
                    pEnumNode->m_bCrackable = IsFileCrackable(hFile);
            }
        }
    }

    if (hFile != INVALID_HANDLE_VALUE) {
        // Énumère les modules d'imports
        EnumImportModuleNames(hFile, EnumDep, (LPARAM)pcs, NULL);

        // Ferme le fichier
        CloseHandle( hFile );
    }

    return TRUE;
}




// Create a shortcut that starts the selected program with CL_LOADER_EXE
bool CShellExt::CreateLoaderShortcut(PCTSTR szShortcutPath // location of the created shortcut
                                   )
{
    // Chemin complet vers le loader CL_LOADER_EXE
    size_t cnt = _tcslen(g_szCLPath) + _countof(CL_LOADER_EXE) 
              + _countof( DIR_CL_BIN ) + 3; // 3 pour 2 antislashs et le 0 terminal  
    PTSTR pszLoaderPath = (PTSTR)malloc( sizeof(TCHAR)* cnt );
    _tcscpy_s(pszLoaderPath, cnt, g_szCLPath);
    _tcscat_s(pszLoaderPath, cnt,  _T("\\"));
    _tcscat_s(pszLoaderPath, cnt, DIR_CL_BIN);
    _tcscat_s(pszLoaderPath, cnt, _T("\\"));
    _tcscat_s(pszLoaderPath, cnt, CL_LOADER_EXE);

    // Argument a passer au loader
    cnt = _tcslen( m_szPEFileName ) +
          _tcslen( m_appSettings.m_szCmdParameters ) +
          4; // les deux guillemets, l'espace et le 0 terminal
    PTSTR pszArg = (PTSTR)malloc( sizeof(TCHAR)* cnt );
    if( m_appSettings.m_szCmdParameters[0] != _T('\0') )
        StringCchPrintf (pszArg, cnt, _T("\"%s\" %s"), m_szPEFileName, m_appSettings.m_szCmdParameters);
    else
        StringCchPrintf (pszArg, cnt, _T("\"%s\""), m_szPEFileName);

    // Chemin d'acces comple au fichier raccourci
    PTSTR pszPathLink = GetShortcutFilePath(szShortcutPath);
    bool res = pszPathLink!=NULL;
    if(res) {
        res = CreateShortcut(pszPathLink,
                    pszLoaderPath, pszArg, m_szPEFileDir,
                    _T("Program loaded with Cracklock"), m_szPEFileName, 0);

        free(pszPathLink);
    }
    free(pszArg);
    free(pszLoaderPath);
    
    return res;
}

// Create a shortcut that starts the selected program with CL_LOADER_EXE
bool CShellExt::CreateLoaderShortcut(int iFolder // indicate the location of the shortcut by a special folder index (for ex. CSIDL_DESKTOP)
                                   )
{
    TCHAR szPathLink[_MAX_PATH];
    return GetSpecialFolderLocation(szPathLink, iFolder) && CreateLoaderShortcut(szPathLink);
}


// Obtient le chemin d'acces au raccourci se trouvant dans le repertoire shortcutDir
PTSTR CShellExt::GetShortcutFilePath(PCTSTR shortcutDir)
{
    // Chemin d'acces comple au fichier raccourci
    PTSTR suffix = LoadResString (g_hResDll, IDS_SHORTCUTSUFFIX);
    size_t cnt = _tcslen(shortcutDir) + 1 + _tcslen(m_appSettings.m_szProgDescr) + _tcslen( suffix ) + 5; // 4 pour ".LNK" + 1 pour le 0 terminal );
    PTSTR pszPathLink = (PTSTR)malloc(  sizeof(TCHAR)* cnt );
    _tcscpy_s(pszPathLink, cnt, shortcutDir);
    _tcscat_s(pszPathLink, cnt, _T("\\"));
    _tcscat_s(pszPathLink, cnt, m_appSettings.m_szProgDescr);
    _tcscat_s(pszPathLink, cnt, suffix);
    _tcscat_s(pszPathLink, cnt,  _T(".lnk"));
    FreeResString(suffix);
    return pszPathLink;
}

// Create a shortcut that starts the selected program with CL_LOADER_EXE
// in the local folder of the application (supposing that CL_LOADER_EXE has been copied to that folder).
bool CShellExt::CreateLocalLoaderShortcut()
{
    PCTSTR pszBasePEFilename = GetFileBaseNamePart(m_szPEFileName);

    // Chemin complet vers le loader CL_LOADER_EXE
    size_t cnt = _tcslen(m_szPEFileDir) + _countof(CL_LOADER_EXE) + 2;
    PTSTR pszLoaderPath = (PTSTR)malloc( sizeof(TCHAR)* cnt );
    _tcscpy_s(pszLoaderPath, cnt, m_szPEFileDir);
    _tcscat_s(pszLoaderPath, cnt,  _T("\\"));
    _tcscat_s(pszLoaderPath, cnt, CL_LOADER_EXE);

    // Argument a passer au loader
    cnt = _tcslen( pszBasePEFilename ) +
          _tcslen( m_appSettings.m_szCmdParameters ) +
          4; // les deux guillemets, l'espace et le 0 terminal
    PTSTR pszArg = (PTSTR)malloc( sizeof(TCHAR)* cnt );
    if( m_appSettings.m_szCmdParameters[0] != _T('\0') )
        StringCchPrintf (pszArg, cnt, _T("\"%s\" %s"), pszBasePEFilename, m_appSettings.m_szCmdParameters);
    else
        StringCchPrintf (pszArg, cnt, _T("\"%s\""), pszBasePEFilename);

    PTSTR pszPathLink = GetShortcutFilePath(m_szPEFileDir);
    bool res = pszPathLink!=NULL;
    if( res ) {    
        res = CreateShortcut(pszPathLink,
                        pszLoaderPath, pszArg,
                        _T(""), // working dir is the current directory
                        _T("Program loaded with Cracklock"),
                        pszBasePEFilename, 0);

        free(pszPathLink);
    }
    free(pszArg);
    free(pszLoaderPath);
    
    return res;
}