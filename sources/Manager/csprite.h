// CSprite.h

#define	MAX_FRAMES	12

// Objet CPalette: charge et applique une palette
class CPalette
{
public:
	CPalette();
	CPalette(HPALETTE hPal);
	~CPalette();

	CPalette operator=(HPALETTE hPal);
	operator HPALETTE() { return m_hPal; }
	
	BOOL loadFromRes(HINSTANCE hInstance, WORD wRes);
	CPalette *select(HDC hdc, BOOL bForceBackground);
	void destroy();
	int apply(HDC hdc, BOOL bMode);

#if _DEBUG
	BOOL CreateBluePalette();
#endif

public:

	BOOL	m_bCreated;	// palette créée ?
	
	HPALETTE m_hPal;	// handle GDI de la palette
};


// Objet CBitmap : charge et dessine des bitmaps
class CBitmap
{
public:
	CBitmap();
	~CBitmap();
	
	operator HBITMAP() { return m_hbmpImage; }

	
	void loadFromRes(HINSTANCE hInstance, WORD wRes, COLORREF clrTrans, CPalette *pPal);

	void create(HBITMAP hBmpSrc, COLORREF clrTrans, CPalette *pPal);
	void destroy();

	BOOL draw(HDC hdc, LONG xDst, LONG yDst, LONG cx, LONG cy, LONG xSrc, LONG ySrc);

public:
	// Handle Windows du bitmap
	HDC		m_hdcImage;
	HBITMAP	m_hbmpImage, m_hbmpImageOld;
	
	HDC		m_hdcMask;
	HBITMAP	m_hbmpMask, m_hbmpMaskOld;
	
	HDC		m_hdcCache;	
	HBITMAP	m_hbmpCache, m_hbmpCacheOld;

	BOOL	m_bCreated;		// bitmap créé ?

	// Taille du bitmap
	LONG	m_width, m_height;

	// nombre de plans et de bits par pixel
	int		m_iPlanes;
	int		m_iPixelBits;

};


// Objet CSprite : objets représenté à l'écran par un Bitmap et capable de se mouvoir
class CSprite
{
public:
    CSprite();
    ~CSprite();
    void create (CBitmap *bitmap);
    void destroy( void );

    void addframe (LONG xBmp, LONG yBmp, LONG cx, LONG cy);
    int getcurframe(void);
    void setcurframe(int iFrame);

    void move(LONG x, LONG y);
    void getRect(RECT *rc);
    void draw(HDC hdc);

private:

	int		m_nbframes;	// nombre total de frame
	int		m_curframe;	// frame courrante

	BOOL	m_bCreated;	// sprite créé ?
	
	LONG	m_x, m_y;	// position courrante du sprite à l'écran
	RECT	m_rcOld;	// ancienne région occupée par le sprite à l'écran
	

	CBitmap	*m_pBitmap;									// pointeur sur l'objet bitmap
	LONG	m_xBmp[MAX_FRAMES], m_yBmp[MAX_FRAMES];		// position des frames dans le bitmap
	LONG	m_width[MAX_FRAMES], m_height[MAX_FRAMES];	// dimensions des frames
};
