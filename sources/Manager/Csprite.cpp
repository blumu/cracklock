// CSprite.cpp

#include <windows.h>
#include <memory.h>
#include <crtdbg.h>

#include "CSprite.h"				// Objet CBitmap et CSprite

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : DibNumColors(VOID FAR * pv)                                *
 *                                                                          *
 *  PURPOSE    : Determines the number of colors in the DIB by looking at   *
 *               the BitCount filed in the info block.                      *
 *                                                                          *
 *  RETURNS    : The number of colors in the DIB.                           *
 *                                                                          *
 ****************************************************************************/
WORD DibNumColors (VOID FAR * pv)
{
    INT                 bits;
    LPBITMAPINFOHEADER  lpbi;
    LPBITMAPCOREHEADER  lpbc;

    lpbi = ((LPBITMAPINFOHEADER)pv);
    lpbc = ((LPBITMAPCOREHEADER)pv);

    /*  With the BITMAPINFO format headers, the size of the palette
     *  is in biClrUsed, whereas in the BITMAPCORE - style headers, it
     *  is dependent on the bits per pixel ( = 2 raised to the power of
     *  bits/pixel).
     */
    if (lpbi->biSize != sizeof(BITMAPCOREHEADER)){
        if (lpbi->biClrUsed != 0)
            return (WORD)lpbi->biClrUsed;
        bits = lpbi->biBitCount;
    }
    else
        bits = lpbc->bcBitCount;

    switch (bits){
        case 1:
                return 2;
        case 4:
                return 16;
        case 8:
                return 256;
        default:
                /* A 24 bitcount DIB has no color table */
                return 0;
    }
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   :  PaletteSize(VOID FAR * pv)                                *
 *                                                                          *
 *  PURPOSE    :  Calculates the palette size in bytes. If the info. block  *
 *                is of the BITMAPCOREHEADER type, the number of colors is  *
 *                multiplied by 3 to give the palette size, otherwise the   *
 *                number of colors is multiplied by 4.                                                          *
 *                                                                          *
 *  RETURNS    :  Palette size in number of bytes.                          *
 *                                                                          *
 ****************************************************************************/
WORD PaletteSize (VOID FAR * pv)
{
    LPBITMAPINFOHEADER lpbi;
    WORD               NumColors;

    lpbi      = (LPBITMAPINFOHEADER)pv;
    NumColors = DibNumColors(lpbi);

    if (lpbi->biSize == sizeof(BITMAPCOREHEADER))
        return (WORD)(NumColors * sizeof(RGBTRIPLE));
    else
        return (WORD)(NumColors * sizeof(RGBQUAD));
}

// constructeur l'objet bitmap
CBitmap::CBitmap()
{
	m_bCreated = FALSE;
}

// destructeur de l'objet bitmap
CBitmap::~CBitmap()
{
	destroy();
}

void CBitmap::destroy()
{
	if(!m_bCreated)
	  return;
	
	// selectionne l'ancien masque dans le DC
	SelectObject(m_hdcMask, m_hbmpMaskOld);
	// d�truit le DC et le masque
	DeleteDC(m_hdcMask);
	DeleteObject(m_hbmpMask);

	// IDEM pour l'image invers�e
	SelectObject(m_hdcImage, m_hbmpImageOld);
	DeleteDC(m_hdcImage);
	DeleteObject(m_hbmpImage);

	// IDEM pour le cache
	SelectObject(m_hdcCache, m_hbmpCacheOld);
	DeleteDC(m_hdcCache);
	DeleteObject(m_hbmpCache);

	m_bCreated = FALSE;
}

// cr�ation de l'objet bitmap � partir d'une ressource bitmap
void CBitmap::loadFromRes(HINSTANCE hInstance, WORD wRes, COLORREF clrTrans, CPalette *pPal)
{
	_ASSERT(hInstance);
	_ASSERT(pPal);

	HRSRC hRes;
	HGLOBAL hmemRes;
	DWORD cRes;
	PBITMAPINFO pBmpi;
	
    HDC         hdc;
	HBITMAP		hbmp;
    CPalette	*pPalOld;

    // d�truit l'instance pr�c�dente
	destroy();

	
	/// LECTURE DE LA RESSOURCE

	// trouve la ressource
	hRes = FindResource(hInstance, MAKEINTRESOURCE(wRes), RT_BITMAP);

    // charge la ressource et obtient sa taille
    hmemRes = LoadResource(hInstance, hRes);
	cRes = SizeofResource(hInstance, hRes);

	// lock le bloque de m�moire afin d'obtenir un pointeur
	pBmpi = (PBITMAPINFO) LockResource(hmemRes);
	if( ! pBmpi )
		goto exit_proc;
	
	//  CR�ATION DU BITMAP
	hdc = GetDC(NULL);

    pPalOld = pPal->select(hdc, FALSE);
    RealizePalette(hdc);     // GDI Bug...????

	
    hbmp = CreateDIBitmap(hdc,
                &pBmpi->bmiHeader,
                CBM_INIT,
                (LPSTR)&pBmpi->bmiHeader + pBmpi->bmiHeader.biSize + PaletteSize(&pBmpi->bmiHeader),
                pBmpi,
                DIB_RGB_COLORS );
	
	if( ! hbmp )
		goto exit_proc;

    if( pPalOld )
	{
		pPalOld->select(hdc,FALSE);
		RealizePalette(hdc);
	}

    ReleaseDC(NULL, hdc);

	
	create( hbmp, clrTrans, pPal);
	
exit_proc:
	
	// lib�ration de la ressource (pas besoin de unlocker)
	if( hmemRes	)
		FreeResource(hmemRes);

	if( hbmp )
		DeleteObject(hbmp);
}

// cr�ation de l'objet
void CBitmap::create(HBITMAP hBmpSrc, COLORREF clrTrans, CPalette *pPal)
{
	BITMAP bm;

	_ASSERT(hBmpSrc);
	_ASSERT(pPal);

    // d�truit l'instance pr�c�dente
	destroy();

	// taille du bitmap	
	GetObject(hBmpSrc, sizeof(BITMAP), (LPVOID)&bm);
	m_width = bm.bmWidth;
	m_height = bm.bmHeight;
	
	// cr�e un DC compatible avec l'�cran qui contiendra une copie du bitmap
	HDC hdcSrc = CreateCompatibleDC(NULL);	
	// selectionne le bitmap dans le DC
	HBITMAP hbmpSrcOld = (HBITMAP)SelectObject(hdcSrc, hBmpSrc);


	// nombre de plans et de bits par pixel
	m_iPlanes = GetDeviceCaps(hdcSrc, PLANES);
	m_iPixelBits = GetDeviceCaps(hdcSrc, BITSPIXEL);


		// COPIE DE L'IMAGE
		//
		// cr�e un DC qui contiendra l'image avec fond invers� (masque AND)
		m_hdcImage = CreateCompatibleDC(NULL);
		// cr�e un bitmap couleur
		m_hbmpImage = CreateBitmap(m_width, m_height, m_iPlanes, m_iPixelBits, NULL);
		// s�lectionne le bitmap dans le DC
		m_hbmpImageOld = (HBITMAP)SelectObject(m_hdcImage, m_hbmpImage);		
		// fait une copie de l'image 
		BitBlt(m_hdcImage, 0, 0, m_width, m_height, hdcSrc, 0, 0, SRCCOPY);


		// CR�ATION DU MASQUE XOR
		//
		// cr�e un DC pour le masque monochrome (masque XOR)
		m_hdcMask = CreateCompatibleDC(NULL);
		// cr�e le bitmap correspondant (monochrome par d�faut)
		m_hbmpMask = (HBITMAP)CreateCompatibleBitmap(m_hdcMask, m_width, m_height);
		// s�lectionne le dans le DC
		m_hbmpMaskOld = (HBITMAP)SelectObject(m_hdcMask, m_hbmpMask);
		// change la couleur du fond en la couleur de transparence
		SetBkColor(hdcSrc, clrTrans);		
		// copie le bitmap couleur dans le DC monochrome
		BitBlt(m_hdcMask, 0, 0, m_width, m_height, hdcSrc, 0, 0, SRCCOPY);


	// r�tablie le bitmap s�lectionn� � l'origine
	SelectObject(hdcSrc, hbmpSrcOld);

	// d�truit le DC temporaire
	DeleteDC(hdcSrc);


	// CR�ATION DU MASQUE 'AND'
	//
	SetBkColor(m_hdcImage, RGB(0,0,0));
	SetTextColor(m_hdcImage, RGB(255,255,255));
	// inverse le fond de l'image
	BitBlt(m_hdcImage, 0, 0, m_width, m_height, m_hdcMask, 0, 0, SRCAND);

	
	// cr�e un DC cache
	m_hdcCache = CreateCompatibleDC(NULL);
	m_hbmpCache = CreateBitmap(m_width, m_height, m_iPlanes, m_iPixelBits, NULL);
	m_hbmpCacheOld = (HBITMAP)SelectObject(m_hdcCache, m_hbmpCache);
	
	// objet cr��
	m_bCreated = TRUE;
}

// dessine une zone du bitmap sur un DC donn� et � une position donn�e
BOOL CBitmap::draw(HDC hdc, LONG xDst, LONG yDst, LONG cx, LONG cy, LONG xSrc, LONG ySrc)
{
	_ASSERT(m_bCreated);
	_ASSERT(hdc);

	COLORREF clrBack, clrFore;

	// enregistre les couleurs et change les en blanc et noir
	clrBack = SetBkColor(m_hdcCache, RGB(255,255,255));
	clrFore = SetTextColor(m_hdcCache, RGB(0,0,0));
	
		// travailler sur une copie de la zone de destination
		BitBlt(m_hdcCache, 0, 0, cx, cy, hdc, xDst, yDst, SRCCOPY);

		// masque la zone destination afin de cr�er un trou
		BitBlt(m_hdcCache, 0, 0, cx, cy, m_hdcMask, xSrc, ySrc, SRCAND);
	
		// copie l'image dans le trou cr�� par le masque
		BitBlt(m_hdcCache, 0, 0, cx, cy, m_hdcImage, xSrc, ySrc, SRCPAINT);
	
	// restaure les couleurs
	SetBkColor(m_hdcCache, clrBack);
	SetTextColor(m_hdcCache, clrFore);

	// copie le contenu du cache dans le DC de destination
	return BitBlt(hdc, xDst, yDst, cx, cy, m_hdcCache, 0, 0, SRCCOPY);
}


// constructeur l'objet sprite
CSprite::CSprite()
{
	m_bCreated = FALSE;
}

// destructeur de l'objet sprite
CSprite::~CSprite()
{
	destroy();
}

// d�truit l'instance de l'objet sprite
void CSprite::destroy(void)
{
	if(!m_bCreated)
		return;
	
	m_pBitmap = NULL;
	m_bCreated = FALSE;
}

// cr�e une instance de sprite
void CSprite::create (CBitmap *pBitmap)
{	
    _ASSERT(pBitmap);

    // d�truit l'instance pr�c�dente
	destroy();

	// bitmap contenant la repr�sentation du sprite
	m_pBitmap = pBitmap;
	
	// pas encore de frames
	m_nbframes = 0;
	m_curframe = -1;
	
	// position de d�part
	m_x = m_y = 0;	
	
	// sprite cr��
	m_bCreated = TRUE;
}


// ajoute une frame au sprite
void CSprite::addframe (LONG xBmp, LONG yBmp, LONG cx, LONG cy)
{
    _ASSERT(m_bCreated);
	_ASSERT(m_nbframes<MAX_FRAMES);

	// position de l'image du sprite dans le bitmap
	m_xBmp[m_nbframes] = xBmp;
	m_yBmp[m_nbframes] = yBmp;

	// taille du sprite
	m_width[m_nbframes] = cx;
	m_height[m_nbframes] = cy;

	// frame courrante
	m_curframe = m_nbframes++;
}


// dessine la frame du sprite
void CSprite::draw(HDC hdc)
{		
	_ASSERT(m_bCreated);
	_ASSERT(hdc);

	m_pBitmap->draw(hdc, m_x, m_y, m_width[m_curframe], m_height[m_curframe], m_xBmp[m_curframe], m_yBmp[m_curframe]);
}


// d�place le sprite
void CSprite::move(LONG x, LONG y)
{
	_ASSERT(m_bCreated);

	// repositionne le sprite
	m_x = x;
	m_y = y;
}

void CSprite::getRect(RECT *rc)
{
    rc->left = m_x;
    rc->top = m_y;
    rc->right = rc->left + m_width[m_curframe];
    rc->bottom = rc->top + m_height[m_curframe];
}

int CSprite::getcurframe(void)
{
	_ASSERT(m_bCreated);

	return m_curframe;
}

void CSprite::setcurframe(int iFrame)
{
	_ASSERT(m_bCreated);
	_ASSERT(iFrame<m_nbframes);

	m_curframe = iFrame;
}


// constructeurs de l'objet palette
CPalette::CPalette()
{
	m_bCreated = FALSE;
}

CPalette::CPalette(HPALETTE hPal)
{
	m_hPal = hPal;
	m_bCreated = TRUE;
}

CPalette CPalette::operator=(HPALETTE hPal)
{
	// d�truit l'instance pr�c�dente
	destroy();

	return CPalette(hPal);
}

// destructeur de l'objet palette
CPalette::~CPalette()
{
	destroy();
}

void CPalette::destroy()
{
	if(!m_bCreated)
	  return;

	// d�truit l'objet GDI
	if( m_hPal ) 
		DeleteObject(m_hPal);

	m_bCreated = FALSE;
}

// cr�ation de l'objet palette � partir d'une ressource bitmap
// (ne marche qu'avec des bitmaps de 256 couleurs)
BOOL CPalette::loadFromRes(HINSTANCE hInstance, WORD wRes)
{
    _ASSERT(hInstance);

	PBITMAPINFO pBmpi;
	HRSRC hRes;
	HGLOBAL hmemRes = NULL;
	DWORD cRes;
	
    // d�truit l'instance pr�c�dente
	destroy();

	/// LECTURE DE LA RESSOURCE
    
	// trouve la ressource
	hRes = FindResource(hInstance, MAKEINTRESOURCE(wRes), RT_BITMAP);

    // alloue un bloque de m�moire et obtient sa taille
    hmemRes = LoadResource(hInstance, hRes);
	cRes = SizeofResource(hInstance, hRes);

	// lock le bloque de m�moire afin d'obtenir un pointeur
	pBmpi = (PBITMAPINFO)LockResource(hmemRes);

	if( ! pBmpi )
		goto exit_proc;
	
	
	/// LECTURE DE LA PALETTE

    PLOGPALETTE pPal;
	WORD cBits, i, cColors;
	
	cColors = DibNumColors((LPVOID)&pBmpi->bmiHeader);		
	cBits = (WORD)pBmpi->bmiHeader.biBitCount;

	// pour des palettes de plus de 8 bits, utilise la palette par d�faut
	if( cBits > 8)
	{
 		m_hPal = (HPALETTE)GetStockObject(DEFAULT_PALETTE);
		goto exit_proc;
	}
	
    pPal = (PLOGPALETTE)malloc(sizeof(LOGPALETTE) +
            (cColors-1) * sizeof(PALETTEENTRY));
                  // cColors-1 car il y a d�j� une entr�e PALETTEENTRY
				  // dans la structure LOGPALETTE
    if( !pPal )
		goto exit_proc;


	/// CR�ATION DE LA PALETTE GDI

	// le champ RGBQUAD de la structure BITMAPINFO a un format diff�rent du champ PALETTEENTRY
	// de la structure LOGPALETTE, on ne peut donc pas effectuer un 'CopyMemory' ou 'memcpy'
	for(i=0; i < cColors; i++)
	{
		// copie et traduit les couleurs
		pPal->palPalEntry[i].peRed = pBmpi->bmiColors[i].rgbRed;
		pPal->palPalEntry[i].peGreen = pBmpi->bmiColors[i].rgbGreen;
		pPal->palPalEntry[i].peBlue = pBmpi->bmiColors[i].rgbBlue;
		pPal->palPalEntry[i].peFlags = 0;
	}
	
	pPal->palNumEntries = cColors;
	pPal->palVersion = 0x300;

	m_hPal = CreatePalette(pPal);


exit_proc:
	
	// lib�ration de la ressource (pas besoin de unlocker)
	if( hmemRes )
		FreeResource(hmemRes);

	if( pPal )
		free(pPal);
	
	m_bCreated = m_hPal ? TRUE : FALSE;
	
	return m_bCreated;
}

#if _DEBUG
BOOL CPalette::CreateBluePalette()
{
    // d�truit l'instance pr�c�dente
	destroy();

    PLOGPALETTE pPal;
    BYTE blue;

    pPal = (PLOGPALETTE)malloc(sizeof(LOGPALETTE) +
            255 * sizeof(PALETTEENTRY));

    if (!pPal)
        return FALSE;

    pPal->palVersion = 0x300;
    pPal->palNumEntries = 256;

    // d�grad� les plus clairs sont plac�s en premier pour une priorit� plus �lev�e
    for (blue = 255; blue > 0; blue--)
    {
        pPal->palPalEntry[255-blue].peRed = 
        pPal->palPalEntry[255-blue].peGreen = 
        pPal->palPalEntry[255-blue].peFlags = 0;
        pPal->palPalEntry[255-blue].peBlue = blue;
    }
    m_hPal = CreatePalette(pPal);
    free(pPal);
	
	m_bCreated = m_hPal ? TRUE : FALSE;
	
	return m_bCreated;
}
#endif

// s�lectionne la palette dans un DC
CPalette *CPalette::select(HDC hdc, BOOL bForceBackground)
{
	_ASSERT(hdc);

	HPALETTE hpalOld;
	
	hpalOld = SelectPalette(hdc, m_hPal, bForceBackground);
	
	return &(CPalette)hpalOld;
}
