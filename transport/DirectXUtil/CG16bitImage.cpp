//	CG16bitImage.cpp
//
//	Implementation of raw 16-bit image object

#include "SDL.h"
#include "SDL_image.h"

#include "portage.h"

#include "CMath.h"
#include "CStringArray.h"
#include "CFileDirectory.h"

#include "CG16bitImage.h"
#include "CG16bitFont.h"
#include "CG16bitSprite.h"

//#define DEBUG_TIME

#define SDL_DoLockSurface(srf)															\
		if (SDL_MUSTLOCK(srf))																\
			if (SDL_LockSurface(srf) < 0)

#define SDL_DoUnlockSurface(srf)															\
	if (SDL_MUSTLOCK(srf))																	\
		{																							\
		SDL_UnlockSurface(srf);																\
		}

int g_iTemp;
#define IntMult(wColor, wScale)		(g_iTemp = (DWORD)(wColor) * (DWORD)(wScale) + 0x80, (DWORD)(((g_iTemp >> 8) + g_iTemp) >> 8))

bool InitBlendPixelTables (void);

AlphaArray5 g_Alpha5 [256];
AlphaArray6 g_Alpha6 [256];
bool g_bInit = InitBlendPixelTables();

/* Convenience function to save CG16bitImages to a test file. */
ALERROR SaveImg(CG16bitImage *img, const char *fn)
	{
	ALERROR err;
	SDL_Surface *dst = SDL_CreateRGBSurface(SDL_SWSURFACE, img->GetWidth(), img->GetHeight(), 16, 0, 0, 0, 0);
	img->BltToSurface(dst, 0, 0);
	err = SDL_SaveBMP(dst, fn) < 0 ? ERR_FAIL : NOERROR;
	SDL_FreeSurface(dst);
	return err;
	}
	      
/* Convenience function to save SDL_Surface objects to a test file.  */
ALERROR SaveSurface(SDL_Surface *img, const char *fn)
	{
	ALERROR err;
	err = SDL_SaveBMP(img, fn) < 0 ? ERR_FAIL : NOERROR;
	return err;
	}

static unsigned int g_iConvertCount = 0;

/* XXX Every place this is used is very expensive. */
SDL_Surface *ConvertTo16Bit(SDL_Surface *src, BOOL &bConvert)
	{
	if (src == NULL || src->format->BitsPerPixel == 16)
		{
		bConvert = FALSE;
		return src;
		}

	SDL_Surface *dst;
	SDL_Surface *tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, 0, 0, 16, 0xf800,
			0x07e0, 0x1f, 0);

	dst = SDL_ConvertSurface(src, tmp->format, SDL_SWSURFACE);
	SDL_FreeSurface(tmp);
	if (dst == NULL)
		{
		printf("Unable to convert bitmask surface to 16 bits.\n");
		}
	bConvert = TRUE;

	g_iConvertCount++;

	return dst;
	}


CG16bitImage::CG16bitImage (void) : CObject(NULL),
		m_cxWidth(0),
		m_cyHeight(0),
		m_bBound(false),
		m_pSurface(NULL),
		m_pRGB(NULL),
		m_pAlpha(NULL),
		m_wBackColor(0),
		m_bHasMask(false),
		m_pRedAlphaTable(NULL),
		m_pGreenAlphaTable(NULL),
		m_pBlueAlphaTable(NULL),
		m_pSprite(NULL)

//	CG16bitImage constructor

	{
	}

CG16bitImage::~CG16bitImage (void)

//	CG16bitImage destructor

	{
	DeleteData();
	}

void CG16bitImage::DrawText (int x, int y, const CG16bitFont &Font, WORD wColor, CString sText, DWORD dwFlags, int *retx)
	{
	Font.DrawText(*this, x, y, wColor, sText, dwFlags, retx);
	}


bool CG16bitImage::AdjustCoords (int *xSrc, int *ySrc, int cxSrc, int cySrc,
								 int *xDest, int *yDest,
								 int *cxWidth, int *cyHeight) const

//	AdjustCoords
//
//	Make sure that the coordinates are in range and adjust
//	them if they are not.

	{
	if (xSrc && *xSrc < 0)
		{
		*cxWidth += *xSrc;
		*xDest -= *xSrc;
		*xSrc = 0;
		}

	if (ySrc && *ySrc < 0)
		{
		*cyHeight += *ySrc;
		*yDest -= *ySrc;
		*ySrc = 0;
		}

	if (*xDest < m_rcClip.left)
		{
		*cxWidth += (*xDest - m_rcClip.left);
		if (xSrc) *xSrc -= (*xDest - m_rcClip.left);
		*xDest = m_rcClip.left;
		}

	if (*yDest < m_rcClip.top)
		{
		*cyHeight += (*yDest - m_rcClip.top);
		if (ySrc) *ySrc -= (*yDest - m_rcClip.top);
		*yDest = m_rcClip.top;
		}

	*cxWidth = std::min((long int)*cxWidth, m_rcClip.right - *xDest);
	if (xSrc)
		*cxWidth = std::min(*cxWidth, cxSrc - *xSrc);

	*cyHeight = std::min((long int)*cyHeight, m_rcClip.bottom - *yDest);
	if (ySrc)
		*cyHeight = std::min(*cyHeight, cySrc - *ySrc);

	return (*cxWidth > 0 && *cyHeight > 0);
	}

WORD CG16bitImage::BlendPixel (WORD pxDest, WORD pxSource, DWORD byOpacity)

//	BlendPixel
//
//	Blends the dest and the source according to byTrans

	{
	DWORD dTemp = pxDest;
	DWORD sTemp = pxSource;

	BYTE *pAlpha5 = g_Alpha5[byOpacity];
	BYTE *pAlpha6 = g_Alpha6[byOpacity];
	BYTE *pAlpha5Inv = g_Alpha5[255 - byOpacity];
	BYTE *pAlpha6Inv = g_Alpha6[255 - byOpacity];

	DWORD dwRedResult = pAlpha5Inv[(dTemp & 0xf800) >> 11] + pAlpha5[(sTemp & 0xf800) >> 11];
	DWORD dwGreenResult = pAlpha6Inv[(dTemp & 0x7e0) >> 5] + pAlpha6[(sTemp & 0x7e0) >> 5];
	DWORD dwBlueResult = pAlpha5Inv[(dTemp & 0x1f)] + pAlpha5[(sTemp & 0x1f)];

	return (WORD)((dwRedResult << 11) | (dwGreenResult << 5) | (dwBlueResult));

#ifdef OLD_BLEND
	DWORD sTemp = pxSource;
	DWORD dTemp = pxDest;

	DWORD sb = sTemp & 0x1f;
	DWORD db = dTemp & 0x1f;
	DWORD sg = (sTemp >> 5) & 0x3f;
	DWORD dg = (dTemp >> 5) & 0x3f;
	DWORD sr = (sTemp >> 11) & 0x1f;
	DWORD dr = (dTemp >> 11) & 0x1f;

	return (WORD)((byOpacity * (sb - db) >> 8) + db |
			((byTrans * (sg - dg) >> 8) + dg) << 5 |
			((byTrans * (sr - dr) >> 8) + dr) << 11);
#endif
	}

WORD CG16bitImage::BlendPixelGray (WORD pxDest, WORD pxSource, DWORD byOpacity)

//	BlendPixelGray
//
//	Blends the dest and the source according to byOpacity. Uses only 5-bits of data for the
//	green channel to preserve the relative weight of all channels. [Without this, sometimes
//	gray colors turn green or purple, because of uneven round-off.]

	{
	DWORD dTemp = pxDest;
	DWORD sTemp = pxSource;

	BYTE *pAlpha5 = g_Alpha5[byOpacity];
	BYTE *pAlpha5Inv = g_Alpha5[255 - byOpacity];

	DWORD dwRedResult = pAlpha5Inv[(dTemp & 0xf800) >> 11] + pAlpha5[(sTemp & 0xf800) >> 11];
	DWORD dwGreenResult = pAlpha5Inv[(dTemp & 0x7e0) >> 6] + pAlpha5[(sTemp & 0x7e0) >> 6];
	DWORD dwBlueResult = pAlpha5Inv[(dTemp & 0x1f)] + pAlpha5[(sTemp & 0x1f)];

	return (WORD)((dwRedResult << 11) | (dwGreenResult << 6) | (dwBlueResult));
	}

WORD CG16bitImage::BlendPixelPM (DWORD pxDest, DWORD pxSource, DWORD byOpacity)

//	BlendPixelPM
//
//	Blends the dest and the source according to byTrans assuming that the
//	source and destination use pre-multiplied values

	{
	DWORD dwInvTrans = (byOpacity ^ 0xff);

	DWORD dwRedGreenS = ((pxSource << 8) & 0x00f80000) | (pxSource & 0x000007e0);
	DWORD dwRedGreen = (((((pxDest << 8) & 0x00f80000) | (pxDest & 0x000007e0)) * dwInvTrans) >> 8) + dwRedGreenS;

	DWORD dwBlue = (((pxDest & 0x1f) * dwInvTrans) >> 8) + (pxSource & 0x1f);

	return (WORD)(((dwRedGreen & 0x00f80000) >> 8) | (dwRedGreen & 0x000007e0) | dwBlue);
	}

void CG16bitImage::Blt (int xSrc, int ySrc, int cxWidth, int cyHeight, const CG16bitImage &Source, int xDest, int yDest)

//	Blt
//
//	Blt the image to the destination

	{
	//	Make sure we're in bounds

	if (!AdjustCoords(&xSrc, &ySrc, Source.m_cxWidth, Source.m_cyHeight, 
			&xDest, &yDest,
			&cxWidth, &cyHeight))
		return;

	//	Do the blt

	WORD *pSrcRow = Source.GetPixel(Source.GetRowStart(ySrc), xSrc);
	WORD *pSrcRowEnd = Source.GetPixel(Source.GetRowStart(ySrc + cyHeight), xSrc);
	WORD *pDestRow = GetPixel(GetRowStart(yDest), xDest);

	while (pSrcRow < pSrcRowEnd)
		{
		WORD *pSrcPos = pSrcRow;
		WORD *pSrcPosEnd = pSrcRow + cxWidth;
		WORD *pDestPos = pDestRow;

		while (pSrcPos < pSrcPosEnd)
			*pDestPos++ = *pSrcPos++;

		pSrcRow = Source.NextRow(pSrcRow);
		pDestRow = NextRow(pDestRow);
		}
	}

void CG16bitImage::BltLighten (int xSrc, int ySrc, int cxWidth, int cyHeight, int iAlpha, const CG16bitImage &Source, int xDest, int yDest)

//	BltLighten
//
//	Blt the image to the destination

	{
	//	Make sure we're in bounds

	if (!AdjustCoords(&xSrc, &ySrc, Source.m_cxWidth, Source.m_cyHeight, 
			&xDest, &yDest,
			&cxWidth, &cyHeight))
		return;

	WORD *pSrcRow = Source.GetPixel(Source.GetRowStart(ySrc), xSrc);
	WORD *pSrcRowEnd = Source.GetPixel(Source.GetRowStart(ySrc + cyHeight), xSrc);
	WORD *pDestRow = GetPixel(GetRowStart(yDest), xDest);

	//	If we've got an alpha mask then blt using the transparency
	//	information.

	if (Source.m_pAlpha)
		{
		BYTE *pAlphaSrcRow = Source.GetAlphaValue(xSrc, ySrc);

		while (pSrcRow < pSrcRowEnd)
			{
			BYTE *pAlphaPos = pAlphaSrcRow;
			WORD *pSrcPos = pSrcRow;
			WORD *pSrcPosEnd = pSrcRow + cxWidth;
			WORD *pDestPos = pDestRow;

			while (pSrcPos < pSrcPosEnd)
				if (*pAlphaPos == 0)
					{
					pDestPos++;
					pSrcPos++;
					pAlphaPos++;
					}
				else
					{
					WORD wColor;

					if (*pAlphaPos == 255)
						wColor = *pSrcPos;
					else
						wColor = BlendPixelPM(*pDestPos, *pSrcPos, *pAlphaPos);

					int iTotalDest = GetRedValue(*pDestPos) + GetGreenValue(*pDestPos) + GetBlueValue(*pDestPos);
					int iTotalSrc = GetRedValue(wColor) + GetGreenValue(wColor) + GetBlueValue(wColor);

					if (iTotalSrc > iTotalDest)
						*pDestPos = wColor;

					pSrcPos++;
					pDestPos++;
					pAlphaPos++;
					}

			pSrcRow = Source.NextRow(pSrcRow);
			pDestRow = NextRow(pDestRow);
			pAlphaSrcRow = Source.NextAlphaRow(pAlphaSrcRow);
			}
		}

	//	If we've got constant transparency then use the alpha tables

	else if (Source.IsTransparent())
		{
		while (pSrcRow < pSrcRowEnd)
			{
			WORD *pSrcPos = pSrcRow;
			WORD *pSrcPosEnd = pSrcRow + cxWidth;
			WORD *pDestPos = pDestRow;

			while (pSrcPos < pSrcPosEnd)
				if (*pSrcPos == m_wBackColor)
					{
					pDestPos++;
					pSrcPos++;
					}
				else
					{
					//	Blend the pixel using the appropriate tables

					WORD rgbRed = Source.m_pRedAlphaTable[((*pSrcPos & 0xf800) >> 6) | ((*pDestPos & 0xf800) >> 11)];
					WORD rgbGreen = Source.m_pGreenAlphaTable[((*pSrcPos & 0x7e0) << 1) | ((*pDestPos & 0x7e0) >> 5)];
					WORD rgbBlue = Source.m_pBlueAlphaTable[(*pSrcPos & 0x1f) << 5 | (*pDestPos & 0x1f)];

					int iTotalDest = GetRedValue(*pDestPos) + GetGreenValue(*pDestPos) + GetBlueValue(*pDestPos);
					int iTotalSrc = rgbRed + rgbGreen + rgbBlue;

					if (iTotalSrc > iTotalDest)
						*pDestPos = rgbRed | rgbGreen | rgbBlue;

					pDestPos++;
					pSrcPos++;
					}

			pSrcRow = Source.NextRow(pSrcRow);
			pDestRow = NextRow(pDestRow);
			}
		}

	//	Otherwise just blt

	else
		{
		while (pSrcRow < pSrcRowEnd)
			{
			WORD *pSrcPos = pSrcRow;
			WORD *pSrcPosEnd = pSrcRow + cxWidth;
			WORD *pDestPos = pDestRow;

			while (pSrcPos < pSrcPosEnd)
				if (*pSrcPos == m_wBackColor)
					{
					pDestPos++;
					pSrcPos++;
					}
				else
					{
					int iTotalDest = GetRedValue(*pDestPos) + GetGreenValue(*pDestPos) + GetBlueValue(*pDestPos);
					int iTotalSrc = GetRedValue(*pSrcPos) + GetGreenValue(*pSrcPos) + GetBlueValue(*pSrcPos);

					if (iTotalSrc > iTotalDest)
						*pDestPos = *pSrcPos;

					pDestPos++;
					pSrcPos++;
					}

			pSrcRow = Source.NextRow(pSrcRow);
			pDestRow = NextRow(pDestRow);
			}
		}
	}

/*
 * Blt the source SDL_Surface into the CG16bitImage at the specified location.
 */
void CG16bitImage::BltInto (struct SDL_Surface *src, int x, int y)
	{
	BYTE *buf = ((BYTE *)m_pRGB) + (y * m_cxWidth + x) * 2; // *2 for 16-bit pixels
	int px_width = std::min(src->w, m_cxWidth - x);
	int px_height = std::min(src->h, m_cyHeight - y);
	BOOL own = false;

	if (src->format->BitsPerPixel != 16) {
		src = ConvertTo16Bit(src, own);
	}
	if (!src || px_width <= 0 || px_height <= 0)
		return;
		
	/* Lock the screen for direct access to the pixels */
	SDL_DoLockSurface(src) {
		return;
	}

	/*
	 * XXX This is probably totally wrong since I don't know if the source
	 * is the same bpp. Plus, there's negative offset alignment issues, since
	 * I'm basically doing manual clipping here.
	 */
	for (int i = 0; i < px_height; i++)
		{
		memcpy(buf + (i * m_iRGBRowSize * sizeof(DWORD)), ((BYTE *)src->pixels) + (i * src->pitch), px_width * 2); 
		}

	SDL_DoUnlockSurface(src);
	if (own)
		{
		SDL_FreeSurface(src);
		}
	}

extern struct SDL_Surface *g_screen;

void CG16bitImage::BltToDC (int x, int y)

//	BltToDC
//
//	Blt the surface to a DC
	{
	if (!m_bBound || g_screen != m_pSurface)
		{
		BltToSurface(g_screen, x, y);
		}

	/* Update just the part of the display that we've changed */
	SDL_UpdateRect(g_screen, x, y, m_cxWidth, m_cyHeight);
	}

void CG16bitImage::BltToSurface (SDL_Surface *dst, int x, int y)

//	BltToSurface
//
//	Blt the surface to a Surface

	{
	BYTE *target;
	BYTE *src;
	int len;
	int i;

	/* Lock the screen for direct access to the pixels */
	SDL_DoLockSurface(dst) {
		return;
	}

	ASSERT(dst->format->BitsPerPixel == 16);

	target = ((BYTE *)dst->pixels) + y * dst->pitch + x * dst->format->BytesPerPixel;
	src = (BYTE *)m_pRGB;
	len = m_cxWidth * dst->format->BytesPerPixel * m_cyHeight;
	for (i = 0; i < len; i++) {
		BYTE f;
		f = target[i];
		f = src[i];
	}

	/* XXX This is probably totally wrong since I don't know if the source is the same bpp. */
	memcpy(target, src, len);

	SDL_DoUnlockSurface(dst);
	}

void CG16bitImage::BltWithMask (int xSrc, int ySrc, int cxWidth, int cyHeight, const CG16bitImage &Mask, const CG16bitImage &Source, int xDest, int yDest)

//	BltWithMask
//
//	Blt the image to the destination

	{
	if (Mask.m_pAlpha == NULL)
		{
		Blt(xSrc, ySrc, cxWidth, cyHeight, Source, xDest, yDest);
		return;
		}

	//	Make sure we're in bounds

	if (!AdjustCoords(&xSrc, &ySrc, Source.m_cxWidth, Source.m_cyHeight, 
			&xDest, &yDest,
			&cxWidth, &cyHeight))
		return;

	if (!Mask.AdjustCoords(&xSrc, &ySrc, Source.m_cxWidth, Source.m_cyHeight,
			&xDest, &yDest,
			&cxWidth, &cyHeight))
		return;

	//	Do the blt

	WORD *pSrcRow = Source.GetPixel(Source.GetRowStart(ySrc), xSrc);
	WORD *pSrcRowEnd = Source.GetPixel(Source.GetRowStart(ySrc + cyHeight), xSrc);
	WORD *pDestRow = GetPixel(GetRowStart(yDest), xDest);
	BYTE *pAlphaRow = Mask.GetAlphaValue(xDest, yDest);

	while (pSrcRow < pSrcRowEnd)
		{
		WORD *pSrcPos = pSrcRow;
		WORD *pSrcPosEnd = pSrcRow + cxWidth;
		WORD *pDestPos = pDestRow;
		BYTE *pAlphaPos = pAlphaRow;

		while (pSrcPos < pSrcPosEnd)
			{
			if (*pAlphaPos == 255)
				{
				*pDestPos++ = *pSrcPos++;
				pAlphaPos++;
				}
			else if (*pAlphaPos == 0)
				{
				pDestPos++;
				pSrcPos++;
				pAlphaPos++;
				}
			else
				{
				DWORD dTemp = *pDestPos;
				DWORD sTemp = *pSrcPos;
				DWORD byOpacity = *pAlphaPos;

				BYTE *pAlpha5 = g_Alpha5[byOpacity];
				BYTE *pAlpha6 = g_Alpha6[byOpacity];
				BYTE *pAlpha5Inv = g_Alpha5[255 - byOpacity];
				BYTE *pAlpha6Inv = g_Alpha6[255 - byOpacity];

				DWORD dwRedResult = pAlpha5Inv[(dTemp & 0xf800) >> 11] + pAlpha5[(sTemp & 0xf800) >> 11];
				DWORD dwGreenResult = pAlpha6Inv[(dTemp & 0x7e0) >> 5] + pAlpha6[(sTemp & 0x7e0) >> 5];
				DWORD dwBlueResult = pAlpha5Inv[(dTemp & 0x1f)] + pAlpha5[(sTemp & 0x1f)];

				*pDestPos = (WORD)((dwRedResult << 11) | (dwGreenResult << 5) | (dwBlueResult));

				pDestPos++;
				pSrcPos++;
				pAlphaPos++;
				}
			}

		pSrcRow = Source.NextRow(pSrcRow);
		pDestRow = NextRow(pDestRow);
		pAlphaRow = Mask.NextAlphaRow(pAlphaRow);
		}
	}

void CG16bitImage::ColorTransBlt (int xSrc, int ySrc, int cxWidth, int cyHeight, int iAlpha, const CG16bitImage &Source, int xDest, int yDest)

//	Blt
//
//	Blt the image to the destination

	{
	//	Deal with sprite sources

	if (Source.m_pSprite)
		{
		Source.m_pSprite->ColorTransBlt(*this,
				xDest,
				yDest,
				xSrc,
				ySrc,
				cxWidth,
				cyHeight);
		return;
		}

	//	Make sure we're in bounds

	if (!AdjustCoords(&xSrc, &ySrc, Source.m_cxWidth, Source.m_cyHeight, 
			&xDest, &yDest,
			&cxWidth, &cyHeight))
		return;

	WORD *pSrcRow = Source.GetPixel(Source.GetRowStart(ySrc), xSrc);
	WORD *pSrcRowEnd = Source.GetPixel(Source.GetRowStart(ySrc + cyHeight), xSrc);
	WORD *pDestRow = GetPixel(GetRowStart(yDest), xDest);

	//	If we've got an alpha mask then blt using the transparency
	//	information.

	if (Source.m_pAlpha)
		{
		BYTE *pAlphaSrcRow = Source.GetAlphaValue(xSrc, ySrc);

		while (pSrcRow < pSrcRowEnd)
			{
			BYTE *pAlphaPos = pAlphaSrcRow;
			WORD *pSrcPos = pSrcRow;
			WORD *pSrcPosEnd = pSrcRow + cxWidth;
			WORD *pDestPos = pDestRow;

			while (pSrcPos < pSrcPosEnd)
				if (*pAlphaPos == 0)
					{
					pDestPos++;
					pSrcPos++;
					pAlphaPos++;
					}
				else if (*pAlphaPos == 255 || *pDestPos == 0)
					{
					*pDestPos++ = *pSrcPos++;
					pAlphaPos++;
					}
				else
					{
					DWORD pxSource = *pSrcPos;
					DWORD pxDest = *pDestPos;
					DWORD dwInvTrans = (((DWORD)(*pAlphaPos)) ^ 0xff);

					DWORD dwRedGreenS = ((pxSource << 8) & 0x00f80000) | (pxSource & 0x000007e0);
					DWORD dwRedGreen = (((((pxDest << 8) & 0x00f80000) | (pxDest & 0x000007e0)) * dwInvTrans) >> 8) + dwRedGreenS;
					DWORD dwBlue = (((pxDest & 0x1f) * dwInvTrans) >> 8) + (pxSource & 0x1f);

					*pDestPos++ = (WORD)(((dwRedGreen & 0x00f80000) >> 8) | (dwRedGreen & 0x000007e0) | dwBlue);

					pSrcPos++;
					pAlphaPos++;
					}

			pSrcRow = Source.NextRow(pSrcRow);
			pDestRow = NextRow(pDestRow);
			pAlphaSrcRow = Source.NextAlphaRow(pAlphaSrcRow);
			}
		}

	//	If we've got constant transparency then use the alpha tables

	else if (Source.IsTransparent())
		{
		while (pSrcRow < pSrcRowEnd)
			{
			WORD *pSrcPos = pSrcRow;
			WORD *pSrcPosEnd = pSrcRow + cxWidth;
			WORD *pDestPos = pDestRow;

			while (pSrcPos < pSrcPosEnd)
				if (*pSrcPos == m_wBackColor)
					{
					pDestPos++;
					pSrcPos++;
					}
				else
					{
					//	Blend the pixel using the appropriate tables

					WORD rgbRed = Source.m_pRedAlphaTable[((*pSrcPos & 0xf800) >> 6) | ((*pDestPos & 0xf800) >> 11)];
					WORD rgbGreen = Source.m_pGreenAlphaTable[((*pSrcPos & 0x7e0) << 1) | ((*pDestPos & 0x7e0) >> 5)];
					WORD rgbBlue = Source.m_pBlueAlphaTable[(*pSrcPos & 0x1f) << 5 | (*pDestPos & 0x1f)];

					*pDestPos++ = rgbRed | rgbGreen | rgbBlue;
					pSrcPos++;
					}

			pSrcRow = Source.NextRow(pSrcRow);
			pDestRow = NextRow(pDestRow);
			}
		}

	//	Otherwise just blt

	else
		{
		while (pSrcRow < pSrcRowEnd)
			{
			WORD *pSrcPos = pSrcRow;
			WORD *pSrcPosEnd = pSrcRow + cxWidth;
			WORD *pDestPos = pDestRow;

			while (pSrcPos < pSrcPosEnd)
				if (*pSrcPos == m_wBackColor)
					{
					pDestPos++;
					pSrcPos++;
					}
				else
					*pDestPos++ = *pSrcPos++;

			pSrcRow = Source.NextRow(pSrcRow);
			pDestRow = NextRow(pDestRow);
			}
		}
	}

void CG16bitImage::ConvertToSprite (void)

//	ConvertToSprite
//
//	Converts to a sprite

	{
	ALERROR error;
	CG16bitSprite *pSprite = new CG16bitSprite;

	error = pSprite->CreateFromImage(*this);
	if (error)
		{
		delete pSprite;
		return;
		}

	//	Done

	DeleteData();
	m_pSprite = pSprite;
	m_cxWidth = m_pSprite->GetWidth();
	m_cyHeight = m_pSprite->GetHeight();
	}

void CG16bitImage::CopyAlpha (int xSrc, int ySrc, int cxWidth, int cyHeight, const CG16bitImage &Source, int xDest, int yDest)

//	CopyAlpha
//
//	Copies the alpha mask from the source to this image

	{
	//	Make sure we're in bounds

	if (!AdjustCoords(&xSrc, &ySrc, Source.m_cxWidth, Source.m_cyHeight, 
			&xDest, &yDest,
			&cxWidth, &cyHeight))
		return;

	//	Copy the alpha mask if both images have it

	if (m_pAlpha && Source.m_pAlpha)
		{
		BYTE *pAlphaSrcRow = Source.GetAlphaValue(xSrc, ySrc);
		BYTE *pAlphaSrcRowEnd = Source.GetAlphaValue(xSrc, ySrc + cyHeight);
		BYTE *pDestRow = GetAlphaValue(xDest, yDest);

		while (pAlphaSrcRow < pAlphaSrcRowEnd)
			{
			BYTE *pAlphaPos = pAlphaSrcRow;
			BYTE *pAlphaPosEnd = pAlphaPos + cxWidth;
			BYTE *pDest = pDestRow;

			while (pAlphaPos < pAlphaPosEnd)
				*pDest++ = *pAlphaPos++;

			pAlphaSrcRow = Source.NextAlphaRow(pAlphaSrcRow);
			pDestRow = NextAlphaRow(pDestRow);
			}
		}

	//	Else, if only the destination has alpha, use the backcolor

	else if (m_pAlpha)
		{
		WORD *pSrcRow = Source.GetPixel(Source.GetRowStart(ySrc), xSrc);
		WORD *pSrcRowEnd = Source.GetPixel(Source.GetRowStart(ySrc + cyHeight), xSrc);
		BYTE *pDestRow = GetAlphaValue(xDest, yDest);

		while (pSrcRow < pSrcRowEnd)
			{
			WORD *pPos = pSrcRow;
			WORD *pPosEnd = pPos + cxWidth;
			BYTE *pDest = pDestRow;

			while (pPos < pPosEnd)
				{
				if (*pPos++ == m_wBackColor)
					*pDest++ = 0x00;
				else
					*pDest++ = 0xff;
				}

			pSrcRow = Source.NextRow(pSrcRow);
			pDestRow = NextAlphaRow(pDestRow);
			}
		}
	}

ALERROR CG16bitImage::CreateBlank (int cxWidth, int cyHeight, bool bAlphaMask)

//	CreateBlank
//
//	Create a blank bitmap

	{
	ALERROR error;
	int iRGBRowSizeBytes;
	int iAlphaRowSize = 0;
	DWORD *pRGB = NULL;
	DWORD *pAlpha = NULL;

	//	Allocate the main buffer

	iRGBRowSizeBytes = AlignUp(cxWidth * sizeof(WORD), sizeof(DWORD));
	pRGB = (DWORD *)MemAlloc(cyHeight * iRGBRowSizeBytes);
	if (pRGB == NULL)
		{
		error = ERR_MEMORY;
		goto Fail;
		}

	memset(pRGB, 0, cyHeight * iRGBRowSizeBytes);

	//	Allocate alpha mask

	if (bAlphaMask)
		{
		iAlphaRowSize = AlignUp(cxWidth, sizeof(DWORD)) / sizeof(DWORD);
		pAlpha = (DWORD *)MemAlloc(cyHeight * iAlphaRowSize * sizeof(DWORD));
		memset(pAlpha, (char)0xFF, cyHeight * iAlphaRowSize * sizeof(DWORD));
		}

	//	Done

	DeleteData();
	m_cxWidth = cxWidth;
	m_cyHeight = cyHeight;
	ResetClipRect();
	m_iRGBRowSize = iRGBRowSizeBytes / sizeof(DWORD);
	m_iAlphaRowSize = iAlphaRowSize;
	m_pRGB = pRGB;
	m_pAlpha = pAlpha;
	m_bHasMask = bAlphaMask;

	return NOERROR;

Fail:

	if (pRGB)
		MemFree(pRGB);

	if (pAlpha)
		MemFree(pAlpha);

	return error;
	}

ALERROR CG16bitImage::CreateBlankAlpha (int cxWidth, int cyHeight, bool bInitialize)

//	CreateBlankAlpha
//
//	Creates a blank image with only a mask

	{
	//	Allocate alpha mask

	int iAlphaRowSize = AlignUp(cxWidth, sizeof(DWORD)) / sizeof(DWORD);
	DWORD *pAlpha = (DWORD *)MemAlloc(cyHeight * iAlphaRowSize * sizeof(DWORD));
	if (bInitialize)
		memset(pAlpha, (char)0xFF, cyHeight * iAlphaRowSize * sizeof(DWORD));

	//	Done

	DeleteData();
	m_cxWidth = cxWidth;
	m_cyHeight = cyHeight;
	ResetClipRect();
	m_iAlphaRowSize = iAlphaRowSize;
	m_pAlpha = pAlpha;
	m_bHasMask = true;
	m_pRGB = NULL;

	return NOERROR;
	}

ALERROR CG16bitImage::CreateFromBitmap (CString &sFilename, bool bPremultAlpha)

//	CreateFromBitmap
//
//	Create from a bitmap and mask. If hBitmask is NULL then we have no
//	mask

	{
	ALERROR error;
	SDL_Surface *img = NULL;

	img = IMG_Load(pathSanitize(sFilename).GetPointer());
	if (!img)
		{
		printf("Unable to load bitmap image '%s': %s", pathSanitize(sFilename).GetPointer(), IMG_GetError());
		return ERR_FAIL;
		}
	// Place the new image data into the CG16bitImage
	error = CreateFromSurface(img, NULL, bPremultAlpha);
	SDL_FreeSurface(img);
	return error;
	}

ALERROR CG16bitImage::CreateFromSurface (SDL_Surface *bmp, SDL_Surface *bmsk, bool bPremultAlpha)
	{
	ALERROR error;

	int iRGBRowSizeBytes;
	int iAlphaRowSize;
	DWORD *pRGB = NULL;
	DWORD *pAlpha = NULL;
	BOOL bBmpOwned = FALSE;
	BOOL bBmskOwned = FALSE;

	//	Figure out the width and height of the bitmap first

	int cxWidth;
	int cyHeight;
	void *pBase;
	int iStride;

	if ((bmp = ConvertTo16Bit(bmp, bBmpOwned)) != NULL)
		{
		cxWidth = bmp->w;
		cyHeight = bmp->h;
		iStride = bmp->w * sizeof(WORD);
		}

	//	Now load the alpha channel (if any). We do this first because we
	//	may need to pre-multiply the RGB values by the alpha later
	
	if ((bmsk = ConvertTo16Bit(bmsk, bBmskOwned)) != NULL)
		{
		int x, y;
		WORD *pSource;
		BYTE *pDest;
		void *pBase;
		int iAlphaStride;
		int cxAlphaWidth, cyAlphaHeight;

		cxAlphaWidth = bmsk->w;
		cyAlphaHeight = bmsk->h;
		iAlphaStride = bmsk->pitch;

		//	If we only have a mask then initialize the width and height appropriately

		if (bmp == NULL)
			{
			cxWidth = cxAlphaWidth;
			cyHeight = cyAlphaHeight;
			}

		//	Allocate a buffer to hold the alpha mask

		iAlphaRowSize = AlignUp(cxWidth, sizeof(DWORD)) / sizeof(DWORD);
		pAlpha = (DWORD *)MemAlloc(cyHeight * iAlphaRowSize * sizeof(DWORD));
		if (pAlpha == NULL)
			{
			error = ERR_MEMORY;
			goto Fail;
			}

		/* Lock the screen for direct access to the pixels */
		SDL_DoLockSurface(bmsk) {
			error = ERR_FAIL;
			goto Fail;
		}

		pBase = bmsk->pixels;

		//	Copy the bits
		for (y = 0; y < cyHeight; y++)
			{
			pSource = (WORD *)((char *)pBase + iAlphaStride * y);
			pDest = (BYTE *)(pAlpha + y * iAlphaRowSize);

			if (y < cyAlphaHeight)
				{
				for (x = 0; x < cxWidth; x++)
					{
					if (x < cxAlphaWidth)
						*pDest = (BYTE)((((*pSource) & 0x7e0) >> 5) * 255 / 63);
					else
						*pDest = 0;

					pDest++;
					pSource++;
					}
				}
			else
				{
				for (x = 0; x < cxWidth; x++)
					*pDest++ = 0;
				}
			}
		pBase = NULL;
		SDL_DoUnlockSurface(bmsk);

		}

	//	Now we get the bits of the bitmap. This section will initialize
	//	cxWidth, cyHeight, iRGBRowSize, and pRGB.
	//
	//	If this is a DIBSECTION of the correct format then we access
	//	the bits directly.

	if (bmp)
		{
		//	Allocate our own buffer

		iRGBRowSizeBytes = AlignUp(Absolute(iStride), sizeof(DWORD));
		pRGB = (DWORD *)MemAlloc(cyHeight * iRGBRowSizeBytes);
		if (pRGB == NULL)
			{
			error = ERR_MEMORY;
			goto Fail;
			}

		SDL_DoLockSurface(bmp) {
			error = ERR_FAIL;
			goto Fail;
		}
		pBase = bmp->pixels;

		if (bmp->format->BitsPerPixel == 16)
			{

			//	Copy bits
			if (!bPremultAlpha && pAlpha)
				{
				int x, y;
				BYTE *pAlphaRow = (BYTE *)pAlpha;
				WORD *pSourceRow = (WORD *)pBase;
				WORD *pDestRow = (WORD *)pRGB;
				for (y = 0; y < cyHeight; y++)
					{
					BYTE *pAlphaPos = pAlphaRow;
					WORD *pSourcePos = pSourceRow;
					WORD *pDestPos = pDestRow;
					for (x = 0; x < cxWidth; x++)
						{
						WORD wBlue = (WORD)IntMult(GetBlueValue(*pSourcePos), *pAlphaPos);
						WORD wGreen = (WORD)IntMult(GetGreenValue(*pSourcePos), *pAlphaPos);
						WORD wRed = (WORD)IntMult(GetRedValue(*pSourcePos), *pAlphaPos);

						*pDestPos = wBlue | (wGreen << 5) | (wRed << 11);

						pAlphaPos++;
						pSourcePos++;
						pDestPos++;
						}

					pAlphaRow += iAlphaRowSize * sizeof(DWORD);
					pSourceRow += iStride / sizeof(WORD);
					pDestRow += iRGBRowSizeBytes / sizeof(WORD);
					}
				}
			else
				{
				int i;
				char *pSource = (char *)pBase;
				char *pDest = (char *)pRGB;
				for (i = 0; i < cyHeight; i++)
					{
					memcpy(pDest, pSource, sizeof(WORD) * cxWidth);
					pSource += iStride;
					pDest += iStride;
					// pDest += iRGBRowSizeBytes;
					}
				}
			}
		else if (bmp->format->BitsPerPixel == 24)
			{
			//	Copy bits

			if (!bPremultAlpha && pAlpha)
				{
				int x, y;
				BYTE *pAlphaRow = (BYTE *)pAlpha;
				BYTE *pSourceRow = (BYTE *)pBase;
				WORD *pDestRow = (WORD *)pRGB;
				for (y = 0; y < cyHeight; y++)
					{
					BYTE *pAlphaPos = pAlphaRow;
					BYTE *pSourcePos = pSourceRow;
					WORD *pDestPos = pDestRow;
					for (x = 0; x < cxWidth; x++)
						{
						WORD wBlue = (WORD)IntMult((*pSourcePos++) >> 3, *pAlphaPos);
						WORD wGreen = (WORD)IntMult((*pSourcePos++) >> 2, *pAlphaPos);
						WORD wRed = (WORD)IntMult((*pSourcePos++) >> 3, *pAlphaPos);

						*pDestPos = wBlue | (wGreen << 5) | (wRed << 11);

						pAlphaPos++;
						pDestPos++;
						}

					pAlphaRow += iAlphaRowSize * sizeof(DWORD);
					pSourceRow += iStride;
					pDestRow += iRGBRowSizeBytes / sizeof(WORD);
					}
				}
			else
				{
				int x, y;
				BYTE *pSourcePos = (BYTE *)pBase;
				WORD *pDestPos = (WORD *)pRGB;
				for (y = 0; y < cyHeight; y++)
					{
					for (x = 0; x < cxWidth; x++)
						{
						DWORD dwBlue = ((DWORD)*pSourcePos++) >> 3;
						DWORD dwGreen = ((DWORD)*pSourcePos++) >> 2;
						DWORD dwRed = ((DWORD)*pSourcePos++) >> 3;

						*pDestPos = (WORD)(dwBlue | (dwGreen << 5) | (dwRed << 11));

						pDestPos++;
						}

//					pSourceRow += iStride;
//					pDestRow += iRGBRowSizeBytes / sizeof(WORD);
					}
				}
			}

		//	Otherwise we get the bits using GetDIBits

		else
			{
			ASSERT(FALSE);
			}

		pBase = NULL;
		SDL_DoUnlockSurface(bmp);
		}


	//	Free the old data and assign the new

	DeleteData();
	m_cxWidth = cxWidth;
	m_cyHeight = cyHeight;
	ResetClipRect();
	m_iRGBRowSize = iRGBRowSizeBytes / sizeof(DWORD);
	m_iAlphaRowSize = iAlphaRowSize;
	m_pRGB = pRGB;
	m_pAlpha = pAlpha;
	m_bHasMask = (pAlpha ? true : false);

	if (bBmskOwned)
		{
		SDL_FreeSurface(bmsk);
		}

	if (bBmpOwned)
		{
		SDL_FreeSurface(bmp);
		}

	return NOERROR;

Fail:

	if (pRGB)
		{
		MemFree(pRGB);
		pRGB = NULL;
		}

	if (pAlpha)
		{
		MemFree(pAlpha);
		pAlpha = NULL;
		}

	if (bBmskOwned)
		{
		SDL_FreeSurface(bmsk);
		}

	if (bBmpOwned)
		{
		SDL_FreeSurface(bmp);
		}

	return error;
	}

ALERROR CG16bitImage::BindSurface(SDL_Surface *pSurf)
	{
	ALERROR error;

	/* It's unknown what surfaces need locking; this isn't implemented yet. */
	ASSERT(SDL_MUSTLOCK(pSurf) == 0);

	/* Already bound to this surface. */
	if (pSurf && pSurf == m_pSurface)
		{
		return NOERROR;
		}

	if (m_bBound)
		{
		if ((error = UnbindSurface()) != NOERROR)
			{
			return error;
			}
		}

	ASSERT(m_pSurface == NULL);

	/* Make sure we're all cleaned up. */
	if (m_pRGB)
		{
		DeleteData();
		}

	m_cxWidth = pSurf->w;
	m_cyHeight = pSurf->h;
	m_iRGBRowSize = (pSurf->w  * sizeof(WORD)) / sizeof(DWORD);
	m_iAlphaRowSize = 0;
	m_bBound = TRUE;
	m_pSurface = pSurf;

	m_pRGB = (DWORD *)pSurf->pixels;
	m_pAlpha = NULL;
	m_wBackColor = 0;
	m_bHasMask = false;
	m_pRedAlphaTable = NULL;
	m_pGreenAlphaTable = NULL;
	m_pBlueAlphaTable = NULL;

	ResetClipRect();

	return NOERROR;
	}

ALERROR CG16bitImage::UnbindSurface()
	{
	if (!m_bBound)
		{
		return NOERROR;
		}
	m_pSurface = NULL;
	m_bBound = false;
	m_pRGB = NULL;
	m_cxWidth = 0;
	m_cyHeight = 0;
	m_iRGBRowSize = 0;

	return NOERROR;
	}

void CG16bitImage::DeleteData (void)

//	DeleteData
//
//	Cleanup the bitmap

	{
	if (m_bBound)
		{
		UnbindSurface();
		}
	if (m_pRGB)
		{
		MemFree(m_pRGB);
		m_pRGB = NULL;
		}

	if (m_pAlpha)
		{
		MemFree(m_pAlpha);
		m_pAlpha = NULL;
		m_bHasMask = false;
		}

	if (m_pRedAlphaTable)
		{
		MemFree(m_pRedAlphaTable);
		m_pRedAlphaTable = NULL;
		}

	if (m_pGreenAlphaTable)
		{
		MemFree(m_pGreenAlphaTable);
		m_pGreenAlphaTable = NULL;
		}

	if (m_pBlueAlphaTable)
		{
		MemFree(m_pBlueAlphaTable);
		m_pBlueAlphaTable = NULL;
		}

	if (m_pSprite)
		{
		delete m_pSprite;
		m_pSprite = NULL;
		}

	m_cxWidth = 0;
	m_cyHeight = 0;
	}

void CG16bitImage::DiscardSurface (void)

//	DiscardSurface
//
//	Discard surface

	{
	}

WORD CG16bitImage::FadeColor (WORD wStart, WORD wEnd, int iFade)

//	FadeColor
//
//	Fades the color from start to end by iFade (percent).
//	0 = wStart
//	100 = wEnd

	{
	int sTemp = (DWORD)wStart;
	int dTemp = (DWORD)wEnd;

	int sb = sTemp & 0x1f;
	int db = dTemp & 0x1f;
	int sg = (sTemp >> 5) & 0x3f;
	int dg = (dTemp >> 5) & 0x3f;
	int sr = (sTemp >> 11) & 0x1f;
	int dr = (dTemp >> 11) & 0x1f;

	return (WORD)(sb + ((db - sb) * iFade / 100))
			| (((WORD)(sg + ((dg - sg) * iFade / 100))) << 5)
			| (((WORD)(sr + ((dr - sr) * iFade / 100))) << 11);
	}

void CG16bitImage::Fill (int x, int y, int cxWidth, int cyHeight, WORD wColor)

//	Fill
//
//	Fills the image with the given color

	{
	//	Make sure we're in bounds

	if (!AdjustCoords(NULL, NULL, 0, 0, 
			&x, &y, 
			&cxWidth, &cyHeight))
		return;

	DWORD dwValue;
	DWORD *pStart;
	DWORD *pEnd;

	DWORD *pStartRow = GetRowStartDW(y);
	DWORD *pEndRow = GetRowStartDW(y + cyHeight);

	//	We try to do everything in DWORDs so first we have to figure out
	//	if the first column is odd or even

	bool bOddFirstColumn = false;
	bool bOddLastColumn = false;

	pStart = GetPixelDW(pStartRow, x, &bOddFirstColumn);
	if (bOddFirstColumn)
		pStart++;

	pEnd = GetPixelDW(pStartRow, x + cxWidth, &bOddLastColumn);

	dwValue = MAKELONG(wColor, wColor);

	//	Fill the part that we can

	DWORD *pPos = pStart;
	while (pPos < pEndRow)
		{
		while (pPos < pEnd)
			*pPos++ = dwValue;

		pStart += m_iRGBRowSize;
		pEnd += m_iRGBRowSize;
		pPos = pStart;
		}

	//	Now do the first column

	if (bOddFirstColumn)
		{
		pStart = GetPixelDW(pStartRow, x, &bOddFirstColumn);
		dwValue = ((DWORD)wColor) << 16;

		pPos = pStart;
		while (pPos < pEndRow)
			{
			SetHighPixel(pPos, dwValue);
			pPos += m_iRGBRowSize;
			}
		}

	//	Now do the last column

	if (bOddLastColumn)
		{
		pStart = GetPixelDW(pStartRow, x + cxWidth, &bOddLastColumn);
		dwValue = (DWORD)wColor;

		pPos = pStart;
		while (pPos < pEndRow)
			{
			SetLowPixel(pPos, dwValue);
			pPos += m_iRGBRowSize;
			}
		}
	}

void CG16bitImage::FillRGB (int x, int y, int cxWidth, int cyHeight, COLORREF rgbValue)

//	FillRGB
//
//	Fills the image with the given color

	{
	//	Make sure we're in bounds

	if (!AdjustCoords(NULL, NULL, 0, 0, 
			&x, &y, 
			&cxWidth, &cyHeight))
		return;

	DWORD dwValue;
	DWORD *pStart;
	DWORD *pEnd;

	DWORD *pStartRow = GetRowStartDW(y);
	DWORD *pEndRow = GetRowStartDW(y + cyHeight);

	//	We try to do everything in DWORDs so first we have to figure out
	//	if the first column is odd or even

	bool bOddFirstColumn = false;
	bool bOddLastColumn = false;

	pStart = GetPixelDW(pStartRow, x, &bOddFirstColumn);
	if (bOddFirstColumn)
		pStart++;

	pEnd = GetPixelDW(pStartRow, x + cxWidth, &bOddLastColumn);

	dwValue = DoublePixelFromRGB(rgbValue);

	//	Fill the part that we can

	DWORD *pPos = pStart;
	while (pPos < pEndRow)
		{
		while (pPos < pEnd)
			*pPos++ = dwValue;

		pStart += m_iRGBRowSize;
		pEnd += m_iRGBRowSize;
		pPos = pStart;
		}

	//	Now do the first column

	if (bOddFirstColumn)
		{
		pStart = GetPixelDW(pStartRow, x, &bOddFirstColumn);
		dwValue = HighPixelFromRGB(rgbValue);

		pPos = pStart;
		while (pPos < pEndRow)
			{
			SetHighPixel(pPos, dwValue);
			pPos += m_iRGBRowSize;
			}
		}

	//	Now do the last column

	if (bOddLastColumn)
		{
		pStart = GetPixelDW(pStartRow, x + cxWidth, &bOddLastColumn);
		dwValue = LowPixelFromRGB(rgbValue);

		pPos = pStart;
		while (pPos < pEndRow)
			{
			SetLowPixel(pPos, dwValue);
			pPos += m_iRGBRowSize;
			}
		}
	}

void CG16bitImage::FillColumn (int x, int y, int cyHeight, WORD wColor)

//	FillColumn
//
//	Fills a single column of pixels

	{
	int yEnd = y + cyHeight;

	//	Make sure this row is in range

	if (x < m_rcClip.left || x >= m_rcClip.right
			|| yEnd <= m_rcClip.top || y >= m_rcClip.bottom)
		return;

	int yStart = std::max<LONG>(y, m_rcClip.top);
	yEnd = std::min<LONG>(yEnd, m_rcClip.bottom);

	//	Get the pointers

	WORD *pPos = GetRowStart(yStart) + x;
	WORD *pPosEnd = GetRowStart(yEnd) + x;

	//	Do it

	while (pPos < pPosEnd)
		{
		*pPos = wColor;
		pPos = NextRow(pPos);
		}
	}

void CG16bitImage::FillLine (int x, int y, int cxWidth, WORD wColor)

//	FillLine
//
//	Fills a single raster line

	{
	int xEnd = x + cxWidth;

	//	Make sure this row is in range

	if (y < m_rcClip.top || y >= m_rcClip.bottom
			|| xEnd <= m_rcClip.left || x >= m_rcClip.right)
		return;

	int xStart = std::max<LONG>(x, m_rcClip.left);
	xEnd = std::min<LONG>(xEnd, m_rcClip.right);

	//	Get the pointers

	WORD *pRow = GetRowStart(y);
	WORD *pPos = pRow + xStart;
	WORD *pEnd = pRow + xEnd;

	//	Do the first pixel (if odd)

	if (xStart % 2)
		*pPos++ = wColor;

	if (pPos == pEnd)
		return;

	DWORD *pPosDW = (DWORD *)pPos;

	//	Do the last pixel (if odd)

	if (xEnd % 2)
		{
		pEnd--;
		*pEnd = wColor;
		}

	DWORD *pEndDW = (DWORD *)pEnd;

	//	Do the middle

	DWORD dwColor = MAKELONG(wColor, wColor);
	while (pPosDW < pEndDW)
		*pPosDW++ = dwColor;
	}

void CG16bitImage::FillLineGray (int x, int y, int cxWidth, WORD wColor, DWORD byOpacity)

//	FillLineGray
//
//	Fills a single raster line

	{
	int xEnd = x + cxWidth;

	//	Make sure this row is in range

	if (y < m_rcClip.top || y >= m_rcClip.bottom)
		return;

	int xStart = std::max<LONG>(x, m_rcClip.left);
	xEnd = std::min<LONG>(xEnd, m_rcClip.right);

	//	Get the pointers

	WORD *pRow = GetRowStart(y);
	WORD *pPos = pRow + xStart;
	WORD *pEnd = pRow + xEnd;

	//	Set up

	BYTE *pAlpha5 = g_Alpha5[byOpacity];
	BYTE *pAlpha5Inv = g_Alpha5[255 - byOpacity];
	DWORD sTemp = wColor;

	//	Blt

	while (pPos < pEnd)
		{
		DWORD dTemp = *pPos;

		DWORD dwRedResult = pAlpha5Inv[(dTemp & 0xf800) >> 11] + pAlpha5[(sTemp & 0xf800) >> 11];
		DWORD dwGreenResult = pAlpha5Inv[(dTemp & 0x7e0) >> 6] + pAlpha5[(sTemp & 0x7e0) >> 6];
		DWORD dwBlueResult = pAlpha5Inv[(dTemp & 0x1f)] + pAlpha5[(sTemp & 0x1f)];

		*pPos++ = (WORD)((dwRedResult << 11) | (dwGreenResult << 6) | (dwBlueResult));
		}
	}

void CG16bitImage::FillLineTrans (int x, int y, int cxWidth, WORD wColor, DWORD byOpacity)

//	FillLineTrans
//
//	Fills a single raster line

	{
	int xEnd = x + cxWidth;

	//	Make sure this row is in range

	if (y < m_rcClip.top || y >= m_rcClip.bottom)
		return;

	int xStart = std::max<LONG>(x, m_rcClip.left);
	xEnd = std::min<LONG>(xEnd, m_rcClip.right);

	//	Get the pointers

	WORD *pRow = GetRowStart(y);
	WORD *pPos = pRow + xStart;
	WORD *pEnd = pRow + xEnd;

	//	Set up

	BYTE *pAlpha5 = g_Alpha5[byOpacity];
	BYTE *pAlpha6 = g_Alpha6[byOpacity];
	BYTE *pAlpha5Inv = g_Alpha5[255 - byOpacity];
	BYTE *pAlpha6Inv = g_Alpha6[255 - byOpacity];
	DWORD sTemp = wColor;

	//	Blt

	while (pPos < pEnd)
		{
		DWORD dTemp = *pPos;

		DWORD dwRedResult = pAlpha5Inv[(dTemp & 0xf800) >> 11] + pAlpha5[(sTemp & 0xf800) >> 11];
		DWORD dwGreenResult = pAlpha6Inv[(dTemp & 0x7e0) >> 5] + pAlpha6[(sTemp & 0x7e0) >> 5];
		DWORD dwBlueResult = pAlpha5Inv[(dTemp & 0x1f)] + pAlpha5[(sTemp & 0x1f)];

		*pPos++ = (WORD)((dwRedResult << 11) | (dwGreenResult << 5) | (dwBlueResult));
		}
	}

void CG16bitImage::DrawAlpha(int xSrc, int ySrc, int cxWidth, int cyHeight, SDL_Surface *Source, WORD wColor, int xDest, int yDest, BYTE byOpacity)
	{
	if (!AdjustCoords(&xSrc, &ySrc, Source->w, Source->h,
			&xDest, &yDest,
			&cxWidth, &cyHeight))
		return;

	//	Compute opacity

	DWORD dwOpacity = byOpacity;

	WORD *pDestRow = GetPixel(GetRowStart(yDest), xDest);
	
	SDL_DoLockSurface(Source);
	if (Source->pixels)
		{
		BYTE *pAlphaSrcRow = ((BYTE *)Source->pixels) + ySrc * Source->pitch + xSrc;
		BYTE *pAlphaSrcRowEnd = ((BYTE *)Source->pixels) + (ySrc + cyHeight) * Source->pitch + xSrc;

		while (pAlphaSrcRow < pAlphaSrcRowEnd)
			{
			BYTE *pAlphaPos = pAlphaSrcRow;
			BYTE *pAlphaPosEnd = pAlphaPos + cxWidth;
			WORD *pDestPos = pDestRow;

			while (pAlphaPos < pAlphaPosEnd)
				{
				if (*pAlphaPos == 0x00)
					{
					}
				else if (*pAlphaPos == 0xff)
					{
					*pDestPos = wColor;
					}
				else
					{
					if (dwOpacity == 0xff)
						*pDestPos = BlendPixel(*pDestPos, wColor, *pAlphaPos);
					else
						*pDestPos = BlendPixel(*pDestPos, wColor, (*pAlphaPos * dwOpacity) >> 8);
					}
				pDestPos++;
				pAlphaPos++;
				}

			pDestRow = NextRow(pDestRow);
			pAlphaSrcRow = pAlphaSrcRow + Source->pitch;
			}
		}
	SDL_DoUnlockSurface(Source);
	}

void CG16bitImage::FillMask (int xSrc, int ySrc, int cxWidth, int cyHeight, const CG16bitImage &Source, WORD wColor, int xDest, int yDest, BYTE byOpacity)

//	FillMask
//
//	Fills the source mask with the given color

	{
	//	Make sure we're in bounds

	if (!AdjustCoords(&xSrc, &ySrc, Source.m_cxWidth, Source.m_cyHeight, 
			&xDest, &yDest,
			&cxWidth, &cyHeight))
		return;

	//	Compute opacity

	DWORD dwOpacity = byOpacity;

	//	If we've got an alpha mask then blt using the transparency
	//	information.

	WORD *pDestRow = GetPixel(GetRowStart(yDest), xDest);
	if (Source.m_pAlpha)
		{
		BYTE *pAlphaSrcRow = Source.GetAlphaValue(xSrc, ySrc);
		BYTE *pAlphaSrcRowEnd = Source.GetAlphaValue(xSrc, ySrc + cyHeight);

		while (pAlphaSrcRow < pAlphaSrcRowEnd)
			{
			BYTE *pAlphaPos = pAlphaSrcRow;
			BYTE *pAlphaPosEnd = pAlphaPos + cxWidth;
			WORD *pDestPos = pDestRow;

			while (pAlphaPos < pAlphaPosEnd)
				{
				if (*pAlphaPos == 0x00)
					{
					}
				else if (*pAlphaPos == 0xff)
					{
					*pDestPos = wColor;
					}
				else
					{
					if (dwOpacity == 0xff)
						*pDestPos = BlendPixel(*pDestPos, wColor, *pAlphaPos);
					else
						*pDestPos = BlendPixel(*pDestPos, wColor, (*pAlphaPos * dwOpacity) >> 8);
					}
				pDestPos++;
				pAlphaPos++;
				}

			pDestRow = NextRow(pDestRow);
			pAlphaSrcRow = Source.NextAlphaRow(pAlphaSrcRow);
			}
		}
	else if (Source.m_pRGB)
		{
		WORD *pSrcRow = Source.GetPixel(Source.GetRowStart(ySrc), xSrc);
		WORD *pSrcRowEnd = Source.GetPixel(Source.GetRowStart(ySrc + cyHeight), xSrc);
		WORD wBackColor = Source.m_wBackColor;

		if (dwOpacity == 0xff)
			{
			while (pSrcRow < pSrcRowEnd)
				{
				WORD *pSrcPos = pSrcRow;
				WORD *pSrcPosEnd = pSrcRow + cxWidth;
				WORD *pDestPos = pDestRow;

				while (pSrcPos < pSrcPosEnd)
					if (*pSrcPos++ != wBackColor)
						*pDestPos++ = wColor;
					else
						pDestPos++;

				pSrcRow = Source.NextRow(pSrcRow);
				pDestRow = NextRow(pDestRow);
				}
			}
		else
			{
			while (pSrcRow < pSrcRowEnd)
				{
				WORD *pSrcPos = pSrcRow;
				WORD *pSrcPosEnd = pSrcRow + cxWidth;
				WORD *pDestPos = pDestRow;

				for (; pSrcPos < pSrcPosEnd; pSrcPos++, pDestPos++)
					{
					if (*pSrcPos != wBackColor)
						*pDestPos = BlendPixel(*pDestPos, wColor, dwOpacity);
					}

				pSrcRow = Source.NextRow(pSrcRow);
				pDestRow = NextRow(pDestRow);
				}
			}
		}
	}

void CG16bitImage::FillTrans (int x, int y, int cxWidth, int cyHeight, WORD wColor, DWORD byOpacity)

//	FillTrans
//
//	Fill with a color and transparency
	
	{
	//	Make sure we're in bounds

	if (!AdjustCoords(NULL, NULL, 0, 0, 
			&x, &y, 
			&cxWidth, &cyHeight))
		return;

	//	Fill

	WORD *pRow = GetPixel(GetRowStart(y), x);
	WORD *pRowEnd = GetPixel(GetRowStart(y + cyHeight), x);

	while (pRow < pRowEnd)
		{
		WORD *pPos = pRow;
		WORD *pPosEnd = pRow + cxWidth;

		for (; pPos < pPosEnd; pPos++)
			*pPos = BlendPixel(*pPos, wColor, byOpacity);

		pRow = NextRow(pRow);
		}
	}

void CG16bitImage::FillTransGray (int x, int y, int cxWidth, int cyHeight, WORD wColor, DWORD byOpacity)

//	FillTransGray
//
//	Fill with a color and transparency
	
	{
	//	Make sure we're in bounds

	if (!AdjustCoords(NULL, NULL, 0, 0, 
			&x, &y, 
			&cxWidth, &cyHeight))
		return;

	//	Fill

	WORD *pRow = GetPixel(GetRowStart(y), x);
	WORD *pRowEnd = GetPixel(GetRowStart(y + cyHeight), x);

	while (pRow < pRowEnd)
		{
		WORD *pPos = pRow;
		WORD *pPosEnd = pRow + cxWidth;

		for (; pPos < pPosEnd; pPos++)
			*pPos = BlendPixelGray(*pPos, wColor, byOpacity);

		pRow = NextRow(pRow);
		}
	}

void CG16bitImage::FillTransRGB (int x, int y, int cxWidth, int cyHeight, COLORREF rgbValue, int iAlpha)

//	FillTransRGB
//
//	Fill with a color and transparency
	
	{
	//	Make sure we're in bounds

	if (!AdjustCoords(NULL, NULL, 0, 0, 
			&x, &y, 
			&cxWidth, &cyHeight))
		return;

	//	Prepare

	BYTE byAlpha = (BYTE)iAlpha;
	WORD wColor = RGBValue(GetRValue(rgbValue) * byAlpha / 255,
			GetGValue(rgbValue) * byAlpha / 255,
			GetBValue(rgbValue) * byAlpha / 255);

	//	Fill

	WORD *pRow = GetPixel(GetRowStart(y), x);
	WORD *pRowEnd = GetPixel(GetRowStart(y + cyHeight), x);

	while (pRow < pRowEnd)
		{
		WORD *pPos = pRow;
		WORD *pPosEnd = pRow + cxWidth;

		for (; pPos < pPosEnd; pPos++)
			*pPos = BlendPixelPM(*pPos, wColor, byAlpha);

		pRow = NextRow(pRow);
		}
	}

WORD CG16bitImage::GetPixelAlpha (int x, int y)

//	GetPixelAlpha
//
//	Returns the alpha value at the given coordinates

	{
	if (x >=0 && y >= 0 && x < m_cxWidth && y < m_cyHeight)
		{
		if (m_pAlpha)
			return *GetAlphaValue(x, y);
		else
			return (*GetPixel(GetRowStart(y), x) == m_wBackColor ? 0x00 : 0xff);
		}
	else
		return 0x00;
	}

CG16bitImage::RealPixel CG16bitImage::GetRealPixel (const RECT &rcRange, float rX, float rY, bool *retbBlack)

//	GetRealPixel
//
//	Returns the pixel at the given floating point coordinates. This
//	routine checks the bounds and return 0 if we're out of range

	{
	int x = (int)rX;
	int y = (int)rY;

	if (x < rcRange.left || y < rcRange.top || x >= rcRange.right || y >= rcRange.bottom)
		{
		RealPixel Pixel = { 0.0, 0.0, 0.0 };
		if (retbBlack)
			*retbBlack = true;
		return Pixel;
		}
	else
		{
		RealPixel Pixel;
		WORD IntPixel = *GetPixel(GetRowStart(y), x);

		if (retbBlack)
			*retbBlack = (IntPixel == m_wBackColor);

		Pixel.rRed = (float)((IntPixel & 0xf800) >> 11) / 31.0f;
		Pixel.rGreen = (float)((IntPixel & 0x7e0) >> 5) / 63.0f;
		Pixel.rBlue = (float)(IntPixel & 0x1f) / 31.0f;

		return Pixel;
		}
	}

void CG16bitImage::GaussianScaledBlt (int xSrc, int ySrc, int cxWidth, int cyHeight, CG16bitImage &Source, int xDest, int yDest, int cxDestWidth, int cyDestHeight)

//	GaussianScaledBlt
//
//	Blt the image to half scale using a Gaussian filter

	{
	float rSampleRadius = 2.0;

	//	Do some basic bounds checking

	if (cxWidth <= 0 || cyHeight <= 0 || cxDestWidth <= 0 || cyDestHeight <= 0)
		return;

	//	Source range

	RECT rcRange;
	rcRange.left = xSrc;
	rcRange.top = ySrc;
	rcRange.right = xSrc + cxWidth;
	rcRange.bottom = ySrc + cyHeight;

	//	Figure out the start and end of the destination rect relative
	//	to xDest, yDest.

	int xStart = -std::min(xDest, 0);
	int yStart = -std::min(yDest, 0);
	int xEnd = std::min(cxDestWidth, m_cxWidth - xDest);
	int yEnd = std::min(cyDestHeight, m_cyHeight - yDest);

	//	Blow out of here if we've got nothing to blt

	if (xEnd - xStart <= 0 || yEnd - yStart <= 0)
		return;

	//	Figure out how big a destination pixel would be in the source

	float rPixelWidth = (float)cxWidth / (float)cxDestWidth;
	float rPixelHeight = (float)cyHeight / (float)cyDestHeight;

	//	Figure out the sample radius

	float rSampleX;
	if (rPixelWidth > 1.0)
		rSampleX = rPixelWidth / rSampleRadius;
	else
		rSampleX = 1.0f / (rSampleRadius * rPixelWidth);

	float rSampleY;
	if (rPixelHeight > 1.0)
		rSampleY = rPixelHeight / rSampleRadius;
	else
		rSampleY = 1.0f / (rSampleRadius * rPixelHeight);

	//	Set up the filter

	float rFilter[3][3];
	rFilter[0][0] = 0.027777778f;
	rFilter[0][1] = 0.111111111f;
	rFilter[0][2] = 0.027777778f;
	rFilter[1][0] = 0.111111111f;
	rFilter[1][1] = 0.444444444f;
	rFilter[1][2] = 0.111111111f;
	rFilter[2][0] = 0.027777778f;
	rFilter[2][1] = 0.111111111f;
	rFilter[2][2] = 0.027777778f;

	//	Loop over every destination pixel

	for (int y = yStart; y < yEnd; y++)
		for (int x = xStart; x < xEnd; x++)
			{
			RealPixel Value = { 0.0, 0.0, 0.0 };

			//	Map this pixel to the source

			float rX = xSrc + ((float)x + 0.5f) * rPixelWidth;
			float rY = ySrc + ((float)y + 0.5f) * rPixelHeight;

			//	Figure out the value for the destination pixel
			//	based on the source pixel (and surroundings)

			int iBlackCount = 0;
			for (int x1 = 0; x1 < 3; x1++)
				for (int y1 = 0; y1 < 3; y1++)
					{
					bool bBlack;
					RealPixel Test = Source.GetRealPixel(rcRange, rX + (x1 - 1) * rSampleX, rY + (y1 - 1) * rSampleY, &bBlack);
					if (bBlack)
						iBlackCount++;

					Value.rRed += Test.rRed * rFilter[x1][y1];
					Value.rGreen += Test.rGreen * rFilter[x1][y1];
					Value.rBlue += Test.rBlue * rFilter[x1][y1];
					}

			SetRealPixel((float)(xDest + x), (float)(yDest + y), Value, (iBlackCount <= 4));
			}
	}

void CG16bitImage::MaskedBlt (int xSrc, int ySrc, int cxWidth, int cyHeight, CG16bitImage &Source, int xDest, int yDest)

//	MaskedBlt
//
//	Blt using a mask. Any alpha value greater than 0 is part of the image

	{
	if (!AdjustCoords(&xSrc, &ySrc, Source.m_cxWidth, Source.m_cyHeight, 
			&xDest, &yDest,
			&cxWidth, &cyHeight))
		return;

	}

void CG16bitImage::ResetClipRect (void)

//	ResetClipRect
//
//	Clears the clip rect

	{
	m_rcClip.left = 0;
	m_rcClip.top = 0;
	m_rcClip.right = m_cxWidth;
	m_rcClip.bottom = m_cyHeight;
	}

void CG16bitImage::SetClipRect (const RECT &rcClip)

//	SetClipRect
//
//	Sets the clip rect

	{
	m_rcClip.left = std::min<LONG>(std::max<LONG>(0, rcClip.left), m_cxWidth);
	m_rcClip.top = std::min<LONG>(std::max<LONG>(0, rcClip.top), m_cyHeight);
	m_rcClip.right = std::min<LONG>(std::max<LONG>(m_rcClip.left, rcClip.right), m_cxWidth);
	m_rcClip.bottom = std::min<LONG>(std::max<LONG>(m_rcClip.top, rcClip.bottom), m_cyHeight);
	}

void CG16bitImage::SetRealPixel (float rX, float rY, const RealPixel &Value, bool bNotBlack)

//	SetRealPixel
//
//	Sets the pixel value

	{
	int x = (int)rX;
	int y = (int)rY;

	if (x < 0 || y < 0 || x >= m_cxWidth || y >= m_cyHeight)
		{
		}
	else
		{
		COLORREF rgbValue = RGB(
				(int)((Value.rRed * 255.0) + 0.5),
				(int)((Value.rGreen * 255.0) + 0.5),
				(int)((Value.rBlue * 255.0) + 0.5));

		WORD wPixel = (WORD)PixelFromRGB(rgbValue);
		if (bNotBlack && wPixel == 0)
			wPixel = 0x0001;

		*GetPixel(GetRowStart(y), x) = wPixel;
		}
	}

void CG16bitImage::SetBlending (WORD wAlpha)

//	SetBlending
//
//	Makes the current bitmap transparent when using ColorTransBlt
//	wAlpha is from 0 to 255, where 255 is opaque and 0 is invisible.

	{
	int iSrc, iDest;

	if (wAlpha == 255)
		{
		MemFree(m_pRedAlphaTable);
		m_pRedAlphaTable = NULL;
		MemFree(m_pGreenAlphaTable);
		m_pGreenAlphaTable = NULL;
		MemFree(m_pBlueAlphaTable);
		m_pBlueAlphaTable = NULL;
		}
	else
		{
		//	Allocate the tables

		if (m_pRedAlphaTable == NULL)
			{
			m_pRedAlphaTable = (WORD *)MemAlloc(2 * 32 * 32);
			m_pGreenAlphaTable = (WORD *)MemAlloc(2 * 64 * 64);
			m_pBlueAlphaTable = (WORD *)MemAlloc(2 * 32 * 32);
			}

		//	Initialize the 5 bit tables

		for (iSrc = 0; iSrc < 32; iSrc++)
			for (iDest = 0; iDest < 32; iDest++)
				{
				m_pRedAlphaTable[iSrc * 32 + iDest] =
						(128 + (255 - (BYTE)wAlpha) * iDest + (BYTE)wAlpha * iSrc) / 256;
				m_pBlueAlphaTable[iSrc * 32 + iDest] = m_pRedAlphaTable[iSrc * 32 + iDest];

				m_pRedAlphaTable[iSrc * 32 + iDest] = m_pRedAlphaTable[iSrc * 32 + iDest] << 11;
				}

		//	Initialize the 6 bit table

		for (iSrc = 0; iSrc < 64; iSrc++)
			for (iDest = 0; iDest < 64; iDest++)
				{
				m_pGreenAlphaTable[iSrc * 64 + iDest] =
						(128 + (255 - (BYTE)wAlpha) * iDest + (BYTE)wAlpha * iSrc) / 256;

				m_pGreenAlphaTable[iSrc * 64 + iDest] = m_pGreenAlphaTable[iSrc * 64 + iDest] << 5;
				}
		}
	}

void CG16bitImage::SetTransparentColor (WORD wColor)

//	SetTransparentColor
//
//	Sets the color to use for transparency. If we've got a mask, it
//	uses the mask to fill in the color on the image itself

	{
	if (m_pAlpha && m_pRGB)
		{
		for (int y = 0; y < m_cyHeight; y++)
			{
			BYTE *pSource = (BYTE *)(m_pAlpha + m_iAlphaRowSize * y);
			WORD *pDest = (WORD *)(m_pRGB + m_iRGBRowSize * y);

			for (int x = 0; x < m_cxWidth; x++)
				if (pSource[x] == 0)
					pDest[x] = wColor;
				else if (pDest[x] == wColor)
					pDest[x] = 0x0001;		//	Almost black
			}

		//	Free the mask since we don't need it anymore

		MemFree(m_pAlpha);
		m_pAlpha = NULL;

		//	The semantic of m_bHasMask is that callers should use
		//	ColorTransBlt instead of Blt.

		m_bHasMask = true;
		}

	m_wBackColor = wColor;
	}

void CG16bitImage::TransBlt (int xSrc, int ySrc, int cxWidth, int cyHeight, int iAlpha, CG16bitImage &Source, int xDest, int yDest)

//	TransBlt
//
//	Blt using alpha transparency

	{
	if (!AdjustCoords(&xSrc, &ySrc, Source.m_cxWidth, Source.m_cyHeight, 
			&xDest, &yDest,
			&cxWidth, &cyHeight))
		return;

	}

//	BlendPixel initialization

bool InitBlendPixelTables (void)
	{
	int i, j;

	for (i = 0; i < 32; i++)
		for (j = 0; j < 256; j++)
			g_Alpha5[j][i] = (BYTE)((DWORD)(((i << 3) * (j / 255.0f)) + 4.0f) >> 3);

	for (i = 0; i < 64; i++)
		for (j = 0; j < 256; j++)
			g_Alpha6[j][i] = (BYTE)((DWORD)(((i << 2) * (j / 255.0f)) + 2.0f) >> 2);

	return true;
	}


//	Testing and Timings --------------------------------------------------------

#ifdef DEBUG_TIME
class Test
	{
	public:
		Test (void)
			{
			int i;
			DWORD dwStart, dwTime;
			char szBuffer[1024];

			//	Wait a bit for system to quiesce

			::Sleep(1000);

			//	Create a blank 512x512 destination

			CG16bitImage Dest;
			Dest.CreateBlank(512, 512, false);
			WORD *pPos = Dest.GetRowStart(256);
			pPos += 256;

			//	Create an alpha source

			CG16bitImage AlphaSource;
			AlphaSource.CreateBlank(256, 256, true);
			for (i = 0; i < 256; i++)
				{
				WORD *pRow = AlphaSource.GetRowStart(i);
				WORD *pRowEnd = pRow + 256;
				WORD wColor = CG16bitImage::RGBValue(255, 0, 0);
				BYTE *pRowAlpha = AlphaSource.GetAlphaRow(i);
				BYTE byTrans = 0;

				while (pRow < pRowEnd)
					{
					*pRow++ = wColor;
					*pRowAlpha++ = byTrans++;
					}
				}
			
#if 0
			dwStart = ::GetTickCount();

			for (i = 0; i < 10000000; i++)
				{
				*pPos = CG16bitImage::BlendPixel(i % 64, i % 32, i % 256);
				}

			dwTime = ::GetTickCount() - dwStart;
			wsprintf(szBuffer, "10,000,000 BlendPixel: %dms\n", dwTime);
			::OutputDebugString(szBuffer);
#endif

#if 0
			dwStart = ::GetTickCount();

			for (i = 0; i < 10000000; i++)
				{
				*pPos = CG16bitImage::BlendPixelPM(i % 64, i % 32, i % 256);
				}

			dwTime = ::GetTickCount() - dwStart;
			wsprintf(szBuffer, "10,000,000 BlendPixelPM: %dms\n", dwTime);
			::OutputDebugString(szBuffer);
#endif

#if 0
			dwStart = ::GetTickCount();

			for (i = 0; i < 1000; i++)
				{
				DrawAlphaGradientCircle(Dest, 256, 256, 100, 255);
				}

			dwTime = ::GetTickCount() - dwStart;
			wsprintf(szBuffer, "1,000 DrawAlphaGradientCircle: %dms\n", dwTime);
			::OutputDebugString(szBuffer);
#endif

#if 1
			dwStart = ::GetTickCount();

			for (i = 0; i < 1000; i++)
				{
				Dest.ColorTransBlt(0, 0, 256, 256, 255, AlphaSource, 0, 0);
				}

			dwTime = ::GetTickCount() - dwStart;
			wsprintf(szBuffer, "1,000 ColorTransBlt (8-bit alpha): %dms\n", dwTime);
			::OutputDebugString(szBuffer);
#endif

			DebugBreak();
			}
	};

Test Testing;

#endif

