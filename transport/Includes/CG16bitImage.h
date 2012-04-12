#ifndef INCL_CG16BITIMAGE
#define INCL_CG16BITIMAGE

#if USE_OLD_16BIT
class CG16bitFont;
class CG16bitSprite;

/* These should probably be relocated somewhere generic. */
extern AlphaArray5 g_Alpha5 [256];
extern AlphaArray6 g_Alpha6 [256];

struct SPoint
	{
	int x;
	int y;
	};

#define ALPHA_BITS				8
#define ALPHA_LEVELS			256

class CG16bitImage : public CObject
	{
	public:
		enum MarkerTypes
			{
			markerPixel,
			markerSmallRound,
			markerSmallSquare,
			markerSmallCross,
			markerMediumCross,
			markerSmallFilledSquare,
			};

		enum SurfaceTypes
			{
			r5g5b5,
			r5g6b5,
			r8g8b8,
			};

		struct SDrawLineCtx
			{
			CG16bitImage *pDest;

			int x1;					//	Start
			int y1;
			int x2;					//	End
			int y2;

			int x;					//	Current
			int y;

			int dx;					//	delta x
			int ax;					//	2 * abs(dx)
			int sx;					//	sign(dx)

			int dy;					//	delta y
			int ay;					//	2 * abs(dy)
			int sy;					//	sign(dy)

			int d;					//	leftover

			inline bool IsXDominant(void) { return (ax > ay); }
			};

		typedef void (*DRAWLINEPROC)(SDrawLineCtx *pCtx);

		CG16bitImage (void);
		virtual ~CG16bitImage (void);

		bool AdjustCoords (int *xSrc, int *ySrc, int cxSrc, int cySrc,
						   int *xDest, int *yDest,
						   int *cxWidth, int *cyHeight) const;
		void ConvertToSprite (void);
		ALERROR CreateBlank (int cxWidth, int cyHeight, bool bAlphaMask);
		ALERROR CreateBlankAlpha (int cxWidth, int cyHeight, bool bInitialize = true);
		ALERROR CreateFromBitmap (CString &sFilename, bool bPreMultAlpha = false);
		ALERROR CreateFromSurface (struct SDL_Surface *, struct SDL_Surface *, bool bPreMultAlpha = false);
		ALERROR BindSurface(struct SDL_Surface *);
		ALERROR UnbindSurface();
		inline void Destroy (void) { DeleteData(); }
		void DiscardSurface (void);
		void SetBlending (WORD wAlpha);
		void SetTransparentColor (WORD wColor);

		void Blt (int xSrc, int ySrc, int cxWidth, int cyHeight, const CG16bitImage &Source, int xDest, int yDest);
		void BltLighten (int xSrc, int ySrc, int cxWidth, int cyHeight, int iAlpha, const CG16bitImage &Source, int xDest, int yDest);
		void BltInto (struct SDL_Surface *src, int x, int y);
		void BltToDC (int x, int y);
		void BltToSurface (struct SDL_Surface *dst, int x, int y);
		void BltWithMask (int xSrc, int ySrc, int cxWidth, int cyHeight, const CG16bitImage &Mask, const CG16bitImage &Source, int xDest, int yDest);
		void ColorTransBlt (int xSrc, int ySrc, int cxWidth, int cyHeight, int iAlpha, const CG16bitImage &Source, int xDest, int yDest);
		void CopyAlpha (int xSrc, int ySrc, int cxWidth, int cyHeight, const CG16bitImage &Soruce, int xDest, int yDest);
		void DrawDot (int x, int y, WORD wColor, MarkerTypes iMarker);
		inline void DrawLine (int x1, int y1, int x2, int y2, int iWidth, WORD wColor) { BresenhamLineAA(x1, y1, x2, y2, iWidth, wColor); }
		void DrawLineProc (SDrawLineCtx *pCtx, DRAWLINEPROC pfProc);
		void DrawLineProcInit (int x1, int y1, int x2, int y2, SDrawLineCtx *pCtx);
		inline void DrawBiColorLine (int x1, int y1, int x2, int y2, int iWidth, WORD wColor1, WORD wColor2) { BresenhamLineAAFade(x1, y1, x2, y2, iWidth, wColor1, wColor2); }
		inline void DrawPixel (int x, int y, WORD wColor) { if (x >=0 && y >= 0 && x < m_cxWidth && y < m_cyHeight) *GetPixel(GetRowStart(y), x) = wColor; }
		inline void DrawPixelTrans (int x, int y, WORD wColor, BYTE byTrans)
				{
				if (x >=0 && y >= 0 && x < m_cxWidth && y < m_cyHeight) 
					{
					WORD *pPos = GetPixel(GetRowStart(y), x);
					*pPos = BlendPixel(*pPos, wColor, byTrans);
					}
				}
		static inline void DrawPixelTrans (WORD *pPos, WORD wColor, DWORD byTrans)
				{
				*pPos = BlendPixel(*pPos, wColor, byTrans);
				}
		inline void DrawPlainLine (int x1, int y1, int x2, int y2, WORD wColor) { BresenhamLine(x1, y1, x2, y2, wColor); }
		void DrawText (int x, int y, const CG16bitFont &Font, WORD wColor, CString sText, DWORD dwFlags = 0, int *retx = NULL);
		void Fill (int x, int y, int cxWidth, int cyHeight, WORD wColor);
		void FillRGB (int x, int y, int cxWidth, int cyHeight, COLORREF rgbValue);
		void FillColumn (int x, int y, int cyHeight, WORD wColor);
		void FillLine (int x, int y, int cxWidth, WORD wColor);
		void FillLineGray (int x, int y, int cxWidth, WORD wColor, DWORD byOpacity);
		void FillLineTrans (int x, int y, int cxWidth, WORD wColor, DWORD byOpacity);
		void DrawAlpha (int xSrc, int ySrc, int cxWidth, int cyHeight, SDL_Surface *Source, WORD wColor, int xDest, int yDest, BYTE byOpacity = 0xff);
		void FillMask (int xSrc, int ySrc, int cxWidth, int cyHeight, const CG16bitImage &Source, WORD wColor, int xDest, int yDest, BYTE byOpacity = 0xff);
		void FillTrans (int x, int y, int cxWidth, int cyHeight, WORD wColor, DWORD byOpacity);
		void FillTransGray (int x, int y, int cxWidth, int cyHeight, WORD wColor, DWORD byOpacity);
		void FillTransRGB (int x, int y, int cxWidth, int cyHeight, COLORREF rgbValue, int iAlpha);
		void GaussianScaledBlt (int xSrc, int ySrc, int cxWidth, int cyHeight, CG16bitImage &Source, int xDest, int yDest, int cxDestWidth, int cyDestHeight);
		inline BYTE *GetAlphaRow (int iRow) const { return (BYTE *)(m_pAlpha + (iRow * m_iAlphaRowSize)); }
		inline BYTE *GetAlphaValue (int x, int y) const { return ((BYTE *)(m_pAlpha + y * m_iAlphaRowSize)) + x; }
		inline WORD GetBackColor (void) const { return m_wBackColor; }
		inline const RECT &GetClipRect (void) const { return m_rcClip; }
		inline int GetHeight (void) const { return m_cyHeight; }
		inline WORD *GetPixel (WORD *pRowStart, int x) const { return pRowStart + x; }
		WORD GetPixelAlpha (int x, int y);
		inline WORD *GetRowStart (int y) const { return (WORD *)(m_pRGB + y * m_iRGBRowSize); }
		inline int GetWidth (void) const { return m_cxWidth; }
		inline bool HasAlpha (void) const { return (m_pAlpha != NULL); }
		inline bool HasMask (void) const { return m_bHasMask; }
		inline bool HasRGB (void) const { return (m_pRGB != NULL); }
		inline bool IsEmpty (void) const { return (m_cxWidth == 0 || m_cyHeight == 0); }
		inline bool IsSprite (void) const { return m_pSprite != NULL; }
		inline bool IsTransparent (void) const { return m_pRedAlphaTable != NULL; }
		void MaskedBlt (int xSrc, int ySrc, int cxWidth, int cyHeight, CG16bitImage &Source, int xDest, int yDest);
		inline BYTE *NextAlphaRow (BYTE *pAlpha) const { return (BYTE *)(((DWORD *)pAlpha) + m_iAlphaRowSize); }
		inline WORD *NextRow (WORD *pRow) const { return pRow + (m_iRGBRowSize * 2); }
		void ResetClipRect (void);
		static WORD BlendPixel (WORD pxDest, WORD pxSource, DWORD byOpacity);
		static WORD BlendPixelGray (WORD pxDest, WORD pxSource, DWORD byOpacity);
		static WORD BlendPixelPM (DWORD pxDest, DWORD pxSource, DWORD byOpacity);
		static WORD FadeColor (WORD wStart, WORD wEnd, int iFade);
		static inline WORD DarkenPixel (DWORD pxSource, DWORD byOpacity) { return BlendPixel(RGBValue(0,0,0), (WORD)pxSource, byOpacity); }
		static inline WORD LightenPixel (DWORD pxSource, DWORD byOpacity) { return BlendPixel(RGBValue(255,255,255), (WORD)pxSource, byOpacity); }
		static inline bool IsGrayscaleValue (WORD wColor) { return ((BlueValue(wColor) == GreenValue(wColor)) && (GreenValue(wColor) == RedValue(wColor))); }
		static inline WORD GrayscaleValue (WORD wValue) { return ((wValue << 8) & 0xf800) | ((wValue << 3) & 0x7c0) | (wValue >> 3); }
		static inline WORD RGBValue (WORD wRed, WORD wGreen, WORD wBlue) { return ((wRed << 8) & 0xf800) | ((wGreen << 3) & 0x7e0) | (wBlue >> 3); }
		static inline WORD BlueValue (WORD wColor) { return GetBlueValue(wColor) << 3; }
		static inline WORD GreenValue (WORD wColor) { return GetGreenValue(wColor) << 2; }
		static inline WORD RedValue (WORD wColor) { return GetRedValue(wColor) << 3; }
		static inline DWORD PixelFromRGB (COLORREF rgb) { return (GetBValue(rgb) >> 3) | ((GetGValue(rgb) >> 2) << 5) | ((GetRValue(rgb) >> 3) << 11); }
		static inline COLORREF RGBFromPixel (WORD wColor) { return RGB(RedValue(wColor), GreenValue(wColor), BlueValue(wColor)); }
		void SetClipRect (const RECT &rcClip);
		void TransBlt (int xSrc, int ySrc, int cxWidth, int cyHeight, int iAlpha, CG16bitImage &Source, int xDest, int yDest);

	private:
		struct RealPixel
			{
			float rRed;
			float rGreen;
			float rBlue;
			};

		void BresenhamLine (int x1, int y1, 
						    int x2, int y2,
						    WORD wColor);
		void BresenhamLineAA (int x1, int y1, 
							  int x2, int y2,
							  int iWidth,
							  WORD wColor);
		void BresenhamLineAAFade (int x1, int y1, 
								  int x2, int y2,
								  int iWidth,
								  WORD wColor1,
								  WORD wColor2);
		void WuLine (int x0, int y0, 
				     int x1, int y1,
				     int iWidth,
					 WORD wColor1);

		void DeleteData (void);
		inline DWORD DoublePixelFromRGB (COLORREF rgb) { return PixelFromRGB(rgb) | (PixelFromRGB(rgb) << 16); }
		inline DWORD *GetPixelDW (DWORD *pRowStart, int x, bool *retbOdd) const { *retbOdd = ((x % 2) == 1); return pRowStart + (x / 2); }
		RealPixel GetRealPixel (const RECT &rcRange, float rX, float rY, bool *retbBlack = NULL);
		static inline WORD GetBlueValue (WORD wPixel) { return (wPixel & 0x1f); }
		static inline WORD GetGreenValue (WORD wPixel) { return (wPixel & 0x7e0) >> 5; }
		static inline WORD GetRedValue (WORD wPixel) { return (wPixel & 0xf800) >> 11; }
		inline DWORD *GetRowStartDW (int y) const { return m_pRGB + y * m_iRGBRowSize; }
		inline DWORD HighPixelFromRGB (COLORREF rgb) const { return PixelFromRGB(rgb) << 16; }
		inline bool InClipX (int x) const { return (x >= m_rcClip.left && x < m_rcClip.right); }
		inline bool InClipY (int y) const { return (y >= m_rcClip.top && y < m_rcClip.bottom); }
		inline DWORD LowPixelFromRGB (COLORREF rgb) const { return PixelFromRGB(rgb); }
		inline void SetLowPixel (DWORD *pPos, DWORD dwValue) { *pPos = ((*pPos) & 0xFFFF0000) | dwValue; }
		inline void SetHighPixel (DWORD *pPos, DWORD dwValue) { *pPos = ((*pPos) & 0xFFFF) | dwValue; }
		void SetRealPixel (float rX, float rY, const RealPixel &Value, bool bNotBlack = false);

		int m_cxWidth;                          //      Width of the image in pixels
		int m_cyHeight;                         //      Height of the image in pixels
		int m_iRGBRowSize;			//	Number of DWORDs in an image row
		int m_iAlphaRowSize;		//	Number of DWORDs in an alpha mask row

		bool m_bBound;
		SDL_Surface *m_pSurface;
		//	The image is structured as an array of 16-bit pixels. Each row
		//	in the image is aligned on a DWORD boundary. The image starts with
		//	the upper left corner. Each pixel is a 5-6-5 RGB pixel.
		DWORD *m_pRGB;

		//	The alpha mask is an array of 8-bit mask values. Each row is
		//	aligned on a DWORD boundary. A value of 0 masks out the image
		//	at that location. A value from 1 to 255 blends the image with
		//	the background.
		DWORD *m_pAlpha;

		WORD m_wBackColor;			//	Back color is transparent
		bool m_bHasMask;			//	TRUE if image has m_pAlpha (or used to have m_pAlpha, but was optimized)

		//	We calculate these transparency tables when using blending
		WORD *m_pRedAlphaTable;
		WORD *m_pGreenAlphaTable;
		WORD *m_pBlueAlphaTable;

		//	Sprite (for optimized blting)
		CG16bitSprite *m_pSprite;

		//	Clipping support
		RECT m_rcClip;				//	Clip rect

	};

void DrawAlphaGradientCircle (CG16bitImage &Dest, 
		int xDest, 
		int yDest, 
		int iRadius,
		WORD wColor);
void DrawBltCircle (CG16bitImage &Dest, 
		int xDest, 
		int yDest, 
		int iRadius,
		const CG16bitImage &Src,
		int xSrc,
		int ySrc,
		int cxSrc,
		int cySrc,
		DWORD byOpacity);
void DrawBrokenLine (CG16bitImage &Dest, int xSrc, int ySrc, int xDest, int yDest, int xyBreak, WORD wColor);
void DrawFilledCircle (CG16bitImage &Dest, int xDest, int yDest, int iRadius, WORD wColor);
void DrawFilledCircleGray (CG16bitImage &Dest, int xDest, int yDest, int iRadius, WORD wColor, DWORD byOpacity);
void DrawFilledCircleTrans (CG16bitImage &Dest, int xDest, int yDest, int iRadius, WORD wColor, DWORD byOpacity);
void DrawGlowRing (CG16bitImage &Dest,
				   int xDest,
				   int yDest,
				   int iRadius,
				   int iRingThickness,
				   WORD *wColorRamp,
				   DWORD *byOpacityRamp);
void DrawRectDotted (CG16bitImage &Dest, int x, int y, int cxWidth, int cyHeight, WORD wColor);
void DrawDottedLine (CG16bitImage &Dest, int x1, int y1, int x2, int y2, WORD wColor);

#else
#include "CGImage.h"
typedef class CGImage CG16bitImage;
#endif
#endif
