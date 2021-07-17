// About.cpp

#include <windows.h>
#include <WinUser.h>
#include <WindowsX.h>

#include <CommCtrl.h>
#include "CSprite.h"				// Objet CSprite et CBitmap
#include "..\Resources\resources.h"	// Ressources localisés
#include "Resource.h"	// Ressources du Manager
#include "about.h"
#include <crtdbg.h>
#define IMP_SHELLEXT
#include "..\ShellExt\resource.h"
#include "..\ShellExt\DLLmain.h"
#include "..\ShellExt\DLLFuncs.h"
#include "..\Common\Common.h"		// Outils communs
#include "..\version.h"
#include "tchar.h"

// Position des images

// distance entre le bord gauche de l'horloge et le bord gauche de la tete
#define TETE_POSITION_X     45
// distance entre le bord haut de l'horloge et le bord haut de la tete
#define TETE_POSITION_Y     45

#define X_MARGIN         5


#define HORLO_PERIOD        500
#define HORLO_RAPIDPERIOD   50


// Repère des sprites du fichier TETES.BMP (IDB_TETES)
#define SPRITE_TETE_WIDTH 95
#define SPRITE_TETE_HEIGHT 95
#define SPRITE_TETE_XSPACE 1
#define SPRITE_TETE_YSPACE 1
#define SPRITE_LINE_LENGTH 6

// Scrolling
int scrollingHeight;

#define	SCROLL_DELTA	2
#define SCROLL_PERIOD	40

#define ScreenToClientRect(rect,parentRect)             rect.left-=parentRect.left;rect.right-=parentRect.left;rect.top-=parentRect.top;rect.bottom-=parentRect.top;
#define RectHeight(rc)                                  (rc.bottom-rc.top)
#define RectWidth(rc)                                   (rc.right-rc.left)


// définit dans manager.cpp
extern HINSTANCE	g_hInstance;

// VARIABLES GLOBALES

UINT_PTR idTimer = 0;		// id du timer actif
BOOL bShowingCredits;	// vrai lorsque le scrolling est actif
RECT rcArdoise;

// Objets pour afficher l'horloge et la tête
CBitmap		bmp_horlo;
CBitmap		bmp_tetes;
CSprite		spr_horlo;
CSprite		spr_tetes;
CPalette	palette;
int			iFrame;
int			iDelta;


// Objet pour le scrolling des crédits
CBitmap bmp_creditsgauche, bmp_creditsdroit;
HBITMAP	hbmpCreditsOld, hbmpCredits;
HDC		hdcCredits;
int		iScrollY;


// fonction appelé régulièrement pour faire défiler les crédits
void CALLBACK TimerScrollCredits (HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime )
{
	InvalidateRect(hwnd, &rcArdoise, FALSE);
	UpdateWindow(hwnd);

	iScrollY += SCROLL_DELTA;
	
	if( iScrollY >= scrollingHeight )
		iScrollY = RectHeight(rcArdoise);
}

// Initialise le scrolling des crédits
void InitCreditsScrolling(HWND hDlg)
{
    // Charge le bitmap de fin de generique
    CBitmap fingen;
    fingen.loadFromRes(g_hInstance, IDB_FINGENERIQUE, RGB(255,0,255), &palette);

	// CRÉATION DE LA ZONE TAMPON CONTENANT LE DESSIN A FAIRE SCROLLER
	
    // crée un DC cache
    hdcCredits = CreateCompatibleDC(NULL);

    // couleur du texte
    SetTextColor(hdcCredits, RGB(255,0,0));
    SetBkMode(hdcCredits, TRANSPARENT);

    // crée la police de caractères pour afficher le texte
    HFONT hFntName = CreateFont(14,0, 0,0, FW_MEDIUM, FALSE, FALSE, FALSE, 0, 0, 0, 0, 0, _T("Helvetica"));

    // sélectionne la police de caractère dans le DC
    HFONT hOldFont = (HFONT)SelectObject(hdcCredits, hFntName);
    PTSTR pszCredits = LoadResString(g_hResDll, IDS_CREDITS);
    _ASSERT(pszCredits);
    
    RECT rcTexte;
    rcTexte.left = rcTexte.top = rcTexte.bottom = 0;
    rcTexte.right = RectWidth(rcArdoise);
    int creditHeight = DrawText(hdcCredits, pszCredits, _tcslen(pszCredits), &rcTexte, DT_CALCRECT|DT_CENTER);

    scrollingHeight = creditHeight + RectHeight(rcArdoise) * 2 + fingen.m_height;

	// crée un bitmap cache
	hbmpCredits = CreateBitmap( RectWidth(rcArdoise),
								scrollingHeight,
								GetDeviceCaps(hdcCredits, PLANES),
								GetDeviceCaps(hdcCredits, BITSPIXEL),
								NULL);

	// sélectionne le bitmap dans le cache
	hbmpCreditsOld = (HBITMAP)SelectObject(hdcCredits, hbmpCredits);



    // EFFACE LE FOND DU BITMAP CONTENANT LES CRÉDITS	
    RECT rcFill;
    HBRUSH hBr;
    rcFill.left = rcFill.top = 0;
    rcFill.right = RectWidth(rcArdoise);
    rcFill.bottom = scrollingHeight;
    hBr = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
    FillRect(hdcCredits, &rcFill, (HBRUSH)hBr);
    DeleteObject(hBr);


    // DESSINE L'HORLOGE ET LA TETE DANS LE BITMAP DU SCROLLING
    spr_horlo.draw(hdcCredits);
    spr_tetes.draw(hdcCredits);


    // ÉCRITURE DU TEXTE DANS LE BITMAP DU SCROLLING

    // charge la chaine de caractères contenant le message des crédits
    pszCredits = LoadResString(g_hResDll, IDS_CREDITS);
    _ASSERT(pszCredits);

    // écriture du texte
    rcTexte.left = 0;
    rcTexte.right = RectWidth(rcArdoise); 
    rcTexte.top = 2*RectHeight(rcArdoise);
    rcTexte.bottom = rcTexte.top + creditHeight;
    DrawText(hdcCredits, pszCredits, _tcslen(pszCredits), &rcTexte, DT_CENTER);


    // remet l'ancienne police
    SelectObject(hdcCredits, hOldFont);

    // libère la chaine de caractères contenant le message des crédits
    FreeResString(pszCredits);

    // Libère les polices de caractères créés
    DeleteObject(hFntName);


    // DESSINE L'HORLOGE ET LA TETE DANS LE BITMAP DU SCROLLING
    fingen.draw(hdcCredits, (RectWidth(rcArdoise) -fingen.m_width)/2, rcTexte.bottom, fingen.m_width, fingen.m_height, 0, 0);

    // CHARGE LE DÉCOR DU SCROLLING
    bmp_creditsgauche.loadFromRes(g_hInstance, IDB_CREDITSGAUCHE, RGB(255,0,255), &palette);
    bmp_creditsdroit.loadFromRes(g_hInstance, IDB_CREDITSDROIT, RGB(255,0,255), &palette);

    // dessine les decorations droite et gauche tout le long du scrolling
    for(int y = 2* RectHeight(rcArdoise);
        y+RectHeight(rcArdoise)<scrollingHeight; y+=bmp_creditsgauche.m_height) {
        bmp_creditsgauche.draw(hdcCredits, X_MARGIN, y, bmp_creditsgauche.m_width, bmp_creditsgauche.m_height, 0, 0);
        bmp_creditsdroit.draw(hdcCredits, RectWidth(rcArdoise)-bmp_creditsdroit.m_width-X_MARGIN, y, bmp_creditsdroit.m_width, bmp_creditsdroit.m_height, 0, 0);
    }

    // FAIT DÉMARRER LE SCROLLING	
    iScrollY = 0;			// position du scrll
    bShowingCredits = TRUE;	// scrolling actif
    idTimer = SetTimer(hDlg, 0, SCROLL_PERIOD, TimerScrollCredits);

}

// détruit les objets alloués pour le scrolling des crédits
void DestroyCreditsScrolling()
{
	SelectObject(hdcCredits, hbmpCreditsOld);
	DeleteObject(hbmpCredits);
	DeleteDC(hdcCredits);
}

// fonction appelé régulièrement pour faire pivoter la tête
void CALLBACK TimerTourneTete (HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime )
{
	iFrame += iDelta;	

	if( iFrame == 13 )
	{
		iDelta =-1;
		KillTimer(hwnd, idTimer);
		idTimer = SetTimer(hwnd, 0, HORLO_RAPIDPERIOD, TimerTourneTete);
	}
	else if (iFrame == -1)
	{
		iFrame = 0;
		iDelta = 1;
		KillTimer(hwnd, idTimer);
		idTimer = SetTimer(hwnd, 0, HORLO_PERIOD, TimerTourneTete );
	}
	else
	{
		spr_tetes.setcurframe(iFrame%12);
		InvalidateRect(hwnd, &rcArdoise, FALSE);
	}		
}

void InitHorloge(HWND hDlg)
{
    // charge la palette et le bmp de l'horloge
    palette.loadFromRes(g_hInstance, IDB_HORLOGE);
    bmp_horlo.loadFromRes(g_hInstance, IDB_HORLOGE, RGB(255,0,255), &palette);

    // Centre la fenetre par rapport a la fenetre du Manager
    RECT rcParent, rcWnd;
    GetWindowRect(GetParent(hDlg), &rcParent);
    GetWindowRect(hDlg, &rcWnd);
    AdjustWindowRectEx(&rcWnd,GetWindowLong(hDlg,GWL_STYLE), FALSE, GetWindowLong(hDlg,GWL_EXSTYLE));
    SetWindowPos(hDlg,
        HWND_BOTTOM,
        rcParent.left+(RectWidth(rcParent)-RectWidth(rcWnd))/2,
        rcParent.top+(RectHeight(rcParent)-RectHeight(rcWnd))/2,
        RectWidth(rcWnd),
        RectHeight(rcWnd),
        SWP_NOSIZE);

    WINDOWINFO wfi;
    GetWindowInfo(hDlg,&wfi);

    RECT rcLineTop, rcLineBot;
    GetWindowRect(GetDlgItem(hDlg, IDC_LINE_TOP), &rcLineTop);
    ScreenToClientRect(rcLineTop, wfi.rcClient);
    GetWindowRect(GetDlgItem(hDlg, IDC_LINE_BOTTOM), &rcLineBot);
    ScreenToClientRect(rcLineBot, wfi.rcClient);

    // Calcul la zone de l'ardoise
    rcArdoise.left = rcLineTop.left;
    rcArdoise.right = rcLineTop.right;
    rcArdoise.top = rcLineTop.bottom;
    rcArdoise.bottom = rcLineBot.top;


    //////////////////
    // Creation des sprites

    spr_horlo.create(&bmp_horlo);
    spr_horlo.addframe(0, 0, bmp_horlo.m_width, bmp_horlo.m_height);

    bmp_tetes.loadFromRes(g_hInstance, IDB_TETES, RGB(255,0,255), &palette);
    spr_tetes.create(&bmp_tetes);
	
    for(int i=0; i<12; i++) {
        spr_tetes.addframe(
            SPRITE_TETE_XSPACE + (SPRITE_TETE_WIDTH + SPRITE_TETE_XSPACE)  * (i%SPRITE_LINE_LENGTH),
            SPRITE_TETE_YSPACE + (SPRITE_TETE_HEIGHT + SPRITE_TETE_YSPACE) * (i/SPRITE_LINE_LENGTH),
            SPRITE_TETE_WIDTH,
            SPRITE_TETE_HEIGHT
            );
    }

    int horloge_x = (RectWidth(rcArdoise)-bmp_horlo.m_width)/2,
        horloge_y = (RectHeight(rcArdoise)-bmp_horlo.m_height)/2;
    spr_horlo.move(horloge_x, horloge_y);
    spr_horlo.setcurframe(0);
    spr_tetes.move(horloge_x + TETE_POSITION_X, horloge_y + TETE_POSITION_Y);
    spr_tetes.setcurframe(0);


    iFrame = 0;
    iDelta = 1;
    iScrollY = 0;    
    bShowingCredits = FALSE; // scrolling inactif

    // Lance le timer pour faire tourner la tete
    idTimer = SetTimer(hDlg, NULL, HORLO_PERIOD, TimerTourneTete);
}

// Dessine  dans la fenêtre l'horloge avec la tête
BOOL PaintAbout(HWND hDlg)
{
    if( GetUpdateRect(hDlg, NULL, FALSE) )
    {
        PAINTSTRUCT ps;
        HDC hdc, hdcCache;
        HBITMAP hbmpCache, hbmpCacheOld;
        HBRUSH hBr;
        RECT rcFill;
        CPalette *pPalOld;

        // commence la peinture
        hdc = BeginPaint(hDlg, &ps);

        // sélectionne la palette
        pPalOld = palette.select(hdc, TRUE);
        RealizePalette(hdc);

        // crée un DC cache
        hdcCache = CreateCompatibleDC(hdc);
        hbmpCache = CreateCompatibleBitmap(hdc,
                        RectWidth(rcArdoise),
                        RectHeight(rcArdoise));
        hbmpCacheOld = (HBITMAP)SelectObject(hdcCache, hbmpCache);

        // Efface le contenu du cache
        rcFill.left = rcFill.top = 0;
        rcFill.right = RectWidth(rcArdoise);
        rcFill.bottom = RectHeight(rcArdoise);
        hBr = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
        FillRect(hdcCache, &rcFill, (HBRUSH)hBr);
        DeleteObject(hBr);
        
        // Affichage des crédits ?
        if( bShowingCredits ) {
            BitBlt(hdcCache,
                0,
                0,
                RectWidth(rcArdoise),
                RectHeight(rcArdoise),
                hdcCredits,
                0,
                iScrollY,
                SRCCOPY);
        }
        // Affichage de l'horloge ?
        else {
            // Dessine les sprites dans le cache
            spr_horlo.draw(hdcCache);
            spr_tetes.draw(hdcCache);
        }

        // Copie le contenu du cache à l'écran
        BitBlt(hdc,
            rcArdoise.left,
            rcArdoise.top,
            RectWidth(rcArdoise),
            RectHeight(rcArdoise),
            hdcCache, 0, 0, SRCCOPY);

        // détruit le cache
        SelectObject(hdcCache, hbmpCacheOld);
        DeleteObject(hbmpCache);
        DeleteDC(hdcCache);

        // sélectionne la palette d'origine
        pPalOld->select(hdc, TRUE);
        RealizePalette(hdc);

        // termine la peinture
        EndPaint(hDlg, &ps);

        return TRUE;
    }

    return FALSE;
}



INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    static HFONT hfntTitle, hfntSubTitle;
    static HICON hIcoHorlo;
    static HWND hTitle;

    switch (uMessage)
    {
    case WM_INITDIALOG:
        {
            // Crée et affecte la police de caractère pour le titre
            hfntTitle = CreateFont(24,0, 0,0, FW_BLACK, FALSE, FALSE, FALSE, 0, 0, 0, 0, 0, _T("Arial"));
            SendMessage(GetDlgItem(hDlg, IDC_TITLE), WM_SETFONT, (WPARAM)hfntTitle, MAKELPARAM(TRUE, 0) );

            // rajoute la version au bout du titre
            hfntSubTitle = CreateFont(13,0, 0,0, FW_MEDIUM, FALSE, FALSE, FALSE, 0, 0, 0, 0, 0, _T("Arial"));
            SendMessage(GetDlgItem(hDlg,IDC_VERSIONINFO), WM_SETFONT, (WPARAM)hfntSubTitle, MAKELPARAM(TRUE, 0) );            
#if defined(_X86_)
SetWindowText(GetDlgItem(hDlg,IDC_VERSIONINFO), INFO_VERSION_T _T(" (32-bit)"));
#elif defined(_AMD64_)
SetWindowText(GetDlgItem(hDlg,IDC_VERSIONINFO), INFO_VERSION_T _T(" (64-bit)"));
#else
#error Unsupported architecture
#endif            

            // Infos de build
            SetWindowText(GetDlgItem(hDlg,IDC_BUILDINFO), (tstring(_T("Built on ")) + _T(__DATE__) + _T(" " ) + _T(__TIME__)).c_str());

            // Affecte l'icone de l'horloge au contrôle correspondant
            hIcoHorlo = (HICON)LoadImage(g_hmodThisDll, MAKEINTRESOURCE(IDI_DALIHORLO), IMAGE_ICON, 0, 0, NULL );
            SendDlgItemMessage(hDlg, IDC_ICO, STM_SETICON, (WPARAM)hIcoHorlo, (LPARAM)0);


            // Initialise l'horloge
            InitHorloge(hDlg);
        }
        return TRUE;


    case WM_PALETTECHANGED:
        // rien à faire si l'on répond à notre propre message
        if (wParam != (WPARAM)hDlg)
        {
            CPalette *pPalOld;

            HDC hdc = GetDC(hDlg);

            // charge les couleurs dans la palette système
            pPalOld = palette.select(hdc, TRUE);
            if( RealizePalette(hdc) )
            {
                // met à jour les couleurs des pixels du DC (au lieu de tout redessiner)
                UpdateColors(hdc);
                //InvalidateRect (hDlg, (LPRECT) (NULL), 1);
            }

            pPalOld->select(hdc, TRUE);
            RealizePalette(hdc);

            ReleaseDC(hDlg, hdc);
        }
        return TRUE;


    case WM_QUERYNEWPALETTE:
        {
            CPalette *pPalOld;
            int icnt;
            HDC hdc = GetDC(hDlg);

            // charge les couleurs dans la palette système
            pPalOld = palette.select(hdc, FALSE);
            icnt = RealizePalette(hdc);

            // Si le changment de palette a fonctionné alors redessine le contenu de la fenêtre
            if (icnt)
            {
                InvalidateRect(hDlg, NULL, TRUE);
                UpdateWindow(hDlg);
            }

            // charge les couleurs de la palette précédantes dans
            // les entrées restantes de la palette système
            pPalOld->select(hdc, TRUE);
            RealizePalette(hdc);

            ReleaseDC(hDlg, hdc);

            SetWindowLong(hDlg, DWLP_MSGRESULT, icnt);
        }
        return TRUE;
	


    case WM_PAINT:
        return PaintAbout(hDlg);

	case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
        if( bShowingCredits ) {
            bShowingCredits = FALSE;
            // Lance le timer pour faire tourner la tete
            idTimer = SetTimer(hDlg, NULL, HORLO_PERIOD, TimerTourneTete);
        }
        else
        {
            POINT ptClick;
            ptClick.x = LOWORD(lParam)-rcArdoise.left;  // horizontal position of cursor 
            ptClick.y = HIWORD(lParam)-rcArdoise.top;  // vertical position of cursor 

            RECT rcTete;
            spr_tetes.getRect(&rcTete);
            if( PtInRect(&rcTete, ptClick) )
            {
                // Tue le timer qui fait tourner la tête
                KillTimer(hDlg, idTimer);

                // Initialise le scrolling des crédits
                InitCreditsScrolling(hDlg);
            }
        }

        return TRUE;

    case WM_NOTIFY:
        {
            int idCtrl = (int)wParam;
            LPNMHDR pnmh = (LPNMHDR)lParam;
            switch(pnmh->code) {
             case NM_CLICK:
                 {
                    PNMLINK pNmLink = (PNMLINK) pnmh;

                    if( pNmLink->item.szUrl )                 
                        ShellExecute(hDlg, NULL, pNmLink->item.szUrl , NULL, NULL, SW_SHOWDEFAULT);
                 }
            default:
                return FALSE;
            }
        }
        break;

    case WM_COMMAND:
    {
        WORD wNotifyCode = HIWORD(wParam); // notification code
        WORD wID = LOWORD(wParam);         // item, control, or accelerator identifier
        HWND hwndCtl = (HWND) lParam;      // handle of control
        if( (wNotifyCode == BN_CLICKED) && (wID == IDCANCEL) )
        {
            EndDialog(hDlg, FALSE);
            SetWindowLong(hDlg, DWLP_MSGRESULT, 0);
            return TRUE;
        }
        break;
    }

    case WM_DESTROY:

        if( idTimer ) 
            KillTimer(hDlg, idTimer);

        DeleteObject(hfntSubTitle);
        DeleteObject(hfntTitle);
        DeleteObject(hIcoHorlo);

        spr_tetes.destroy();
        spr_horlo.destroy();
        bmp_tetes.destroy();
        bmp_horlo.destroy();
        palette.destroy();
        bmp_creditsgauche.destroy();
        bmp_creditsdroit.destroy();

        DestroyCreditsScrolling();
        return TRUE;

    }
    return FALSE;
}

