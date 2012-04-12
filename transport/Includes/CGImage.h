#ifndef INCL_CGIMAGE
#define INCL_CGIMAGE

#include "SDL.h"

struct CG16bitFont;
class CGImage;

extern CGImage *g_screen;
typedef DWORD PIXEL;

struct SPoint
	{
	int x;
	int y;
	};


ALERROR SaveImg(CGImage *img, const char *fn);

class CGImage : public CObject
	{
	protected:
		SDL_Surface *m_Img;
		bool m_bEnableAlpha;

	public:
		static struct SDL_PixelFormat *DispPixFormat;

		enum MarkerTypes
			{
			markerPixel,
			markerSmallRound,
			markerSmallSquare,
			markerSmallCross,
			markerMediumCross,
			markerSmallFilledSquare,
			};

		struct SDrawLineCtx
			{
			CGImage *pDest;

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

		struct SAlphaGradientCircleLineCtx
			{
			CGImage *pDest;
			int xDest;
			int yDest;
			int iRadius;
			COLORREF wColor;

			BYTE dwRed;
			BYTE dwGreen;
			BYTE dwBlue;
			};

		struct SBltCircleLineCtx
			{
			//	Dest
			int xDest;
			int yDest;
			int iRadius;

			//	Source
			const CGImage *pSrc;
			int xSrc;
			int ySrc;
			int cxSrc;
			int cySrc;
			DWORD byOpacity;
			};


		/* Constructors and generics. */
		CGImage (void);
		CGImage (SDL_Surface *sfc) : CObject(NULL) { CreateFromSurface(sfc, NULL); }
		virtual ~CGImage ();

		const CGImage &operator=(const CGImage &rhs)
			{
			if (rhs.m_Img && rhs.m_Img != m_Img)
				{
				DiscardSurface();
				m_Img = SDL_ConvertSurface(rhs.m_Img, rhs.m_Img->format, 0);
				}
			return *this;
			}

		virtual bool AdjustCoords (int *xSrc, int *ySrc, int cxSrc, int cySrc, int *xDest, int *yDest, int *cxWidth, int *cyHeight) const;

		/* Creation functions. */
		virtual ALERROR Create(int cxWidth, int cyHeight, BYTE depth = 32);
		virtual ALERROR CreateFromFile (CString &sFilename);
		virtual ALERROR CreateFromSurface (struct SDL_Surface *, struct SDL_Surface *, bool _notused = false);

		virtual void CopyAlpha (int xSrc, int ySrc, int cxWidth, int cyHeight, const CGImage &Soruce, int xDest, int yDest);

		/* Destruction functions. */
		virtual void DiscardSurface (void);

		/* Utility functions to bind an SDL_Surface to this object. */
		virtual ALERROR BindSurface(struct SDL_Surface *);
		virtual ALERROR UnbindSurface();

		/* Different ways to push data around. */
		virtual void Blt (int x, int y, CGImage &Source, int xSrc, int ySrc,
				int x2Src, int y2Src, bool bUseAlpha = true);
		virtual void BltRotoZ (int x, int y, int x2, int y2, CGImage &Source,
				int xSrc, int ySrc, int x2Src, int y2Src, double degree, int smooth,
				bool bUseAlpha = true);
		virtual void RotoZXY(int x, int y, int x2, int y2, int xSrc, int ySrc,
				int x2Src, int y2Src, double degree, int *w, int *h);
		inline void BltToDC (int x, int y)
			{
			if (g_screen->m_Img != m_Img)
				g_screen->Blt(x, y, *this, 0, 0, GetWidth(), GetHeight());
			SDL_Flip(g_screen->m_Img);
			}
		virtual ALERROR BltToAlpha (struct SDL_Surface *src);
		virtual ALERROR BltToAlpha (CGImage *src) { return BltToAlpha(src->m_Img); }

		/* Accessors. */
		inline SDL_Surface *GetSurface() { return m_Img; }
		virtual void SetAlphaChannel (BYTE bAlpha);
		virtual void SetAlphaChannel (int x, int y, int x2, int y2, BYTE bAlpha);
		virtual void SetAlpha (BYTE bAlpha);
		virtual BYTE GetAlpha ();
		void SetAlphaBlit(bool bEnable) { m_bEnableAlpha = bEnable; }
		bool GetAlphaBlit(void) { return m_bEnableAlpha; }

		virtual void SetTransparentColor (COLORREF wColor);
		virtual COLORREF GetTransparentColor ();

		virtual void SetBackColor (COLORREF);
		virtual COLORREF GetBackColor (void) const;

		virtual void SetClipRect (const RECT &rcClip);
		virtual RECT GetClipRect (void) const;
		virtual void ResetClipRect (void);

		virtual bool Intersect(int x, int y, int x2, int y2, CGImage &Source, int xSrc, int ySrc);
		
		virtual int GetWidth (void) const { return m_Img->w; }
		virtual int GetHeight (void) const { return m_Img->h; }

		virtual bool IsEmpty (void) const { return !m_Img; }

		/* Static 32-bit color utility functions. */
		static inline COLORREF RGBColor (BYTE bRed, BYTE bGreen, BYTE bBlue)
		{ return 0xFF000000 | bRed << 16 | bGreen << 8 | bBlue;		}
		static inline COLORREF RGBAColor (BYTE bRed, BYTE bGreen, BYTE bBlue, BYTE bAlpha)
		{ return bAlpha << 24 | bRed << 16 | bGreen << 8 | bBlue; }
		static inline BYTE AlphaColor (COLORREF wColor) { return (wColor >> 24) & 0xff; }
		static inline BYTE RedColor (COLORREF wColor) { return (wColor >> 16) & 0xff; }
		static inline BYTE GreenColor (COLORREF wColor) { return (wColor >> 8) & 0xff; }
		static inline BYTE BlueColor (COLORREF wColor) { return wColor & 0xff; }
		static inline COLORREF SetAlphaColor (COLORREF wColor, BYTE bAlpha)
		{ return RGBAColor(RedColor(wColor), GreenColor(wColor), BlueColor(wColor), bAlpha); }
		/* Basic COLORREF manipulation functions. */
		static COLORREF BlendColor (COLORREF pxDest, COLORREF pxSource, BYTE bAlpha);
		static inline COLORREF FadeColor (COLORREF wStart, COLORREF wEnd, BYTE iFade)
			{
			/* iFade is the percentage value to fade. */
			assert(iFade <= 100);
			return RGBColor(RedColor(wStart) + ((RedColor(wEnd) - RedColor(wStart)) * iFade / 100),
				GreenColor(wStart) + ((GreenColor(wEnd) - GreenColor(wStart)) * iFade / 100),
				BlueColor(wStart) + ((BlueColor(wEnd) - BlueColor(wStart)) * iFade / 100));
			}

		static inline COLORREF DarkenColor (COLORREF pxSource, BYTE bAlpha)
		{ return BlendColor(RGBColor(0, 0, 0), pxSource, bAlpha); }
		static inline COLORREF LightenColor (COLORREF pxSource, BYTE bAlpha)
		{ return BlendColor(RGBColor(255, 255, 255), pxSource, bAlpha); }
		static inline bool IsGrayscaleColor (COLORREF wColor)
		{ return ((BlueColor(wColor) == GreenColor(wColor)) && (GreenColor(wColor) == RedColor(wColor))); }
		static inline COLORREF GrayscaleColor (BYTE bColor)
		{ return RGBColor(bColor, bColor, bColor); }

		/* Drawing functions. */
		virtual void DrawDot (int x, int y, COLORREF wColor, MarkerTypes iMarker);
		virtual void DrawLine (int x1, int y1, int x2, int y2, int iWidth, COLORREF wColor);
		virtual void DrawLineBiColor (int x1, int y1, int x2, int y2, int iWidth, COLORREF wColor1, COLORREF wColor2);
		virtual void DrawLineProcInit (int x1, int y1, int x2, int y2, SDrawLineCtx *pCtx);
		virtual void DrawLineProc (SDrawLineCtx *pCtx, DRAWLINEPROC pfProc);
		virtual void DrawLineBroken (int x, int y, int x2, int y2, int xyBreak, COLORREF wColor);
		virtual void DrawLineDotted (int x1, int y1, int x2, int y2, COLORREF wColor);

		virtual void DrawText (int x, int y, const CG16bitFont &Font, COLORREF wColor, CString sText, DWORD dwFlags = 0, int *retx = NULL);

		virtual void DrawRect (int x, int y, int x2, int y2, COLORREF wColor);
		virtual void DrawRectFilled (int x, int y, int x2, int y2, COLORREF wColor);
		virtual void DrawRectDotted (int x, int y, int x2, int y2, COLORREF wColor);
		virtual void DrawCircleFilled (int x, int y, int iRadius, COLORREF wColor);
		virtual void DrawCircleBlt (int x, int y, int iRadius, const CGImage &Src, int xSrc, int ySrc, int x2Src, int y2Src, BYTE bAlpha);
		virtual void DrawCircleGradient (int x, int y, int iRadius, COLORREF wColor);
		virtual void DrawRingGradient (int x, int y, int iRadius, int iRingThickness, COLORREF *wColorRamp, BYTE *bAlphaRamp);

		virtual void DrawMask (int x, int y, const CGImage &Src, int xSrc, int ySrc, int x2Src, int y2Src, COLORREF wColor);

		virtual void DrawGlowImage (int x, int y, CGImage &Src, int xSrc, int ySrc, int x2Src, int y2Src, int iGlowSize);

		COLORREF GetPixel(int x, int y) const;

		friend ALERROR SaveImg(CGImage *img, const char *fn);

		/* Placeholders. */
		inline void ConvertToSprite(void) { }
		inline ALERROR CreateBlank (int cxWidth, int cyHeight, bool _notused = false )
		{ return Create(cxWidth, cyHeight, 32); }
		inline ALERROR CreateBlankAlpha (int cxWidth, int cyHeight, bool _notused = false ) { return Create(cxWidth, cyHeight, 8); }
		inline ALERROR CreateFromBitmap (CString sFilename, bool _notused = false) { return CreateFromFile(sFilename); }
		inline void Destroy(void) { DiscardSurface(); }
		inline void SetBlending(COLORREF wAlpha) { SetAlpha(wAlpha); }

		inline void Blt (int xSrc, int ySrc, int cxWidth, int cyHeight, const CGImage &Source, int xDest, int yDest)
		{ Blt(xDest, yDest, *(CGImage *)&Source, xSrc, ySrc, xSrc + cxWidth, ySrc + cyHeight); }
		inline void BltLighten (int xSrc, int ySrc, int cxWidth, int cyHeight, int iAlpha, const CGImage &Source, int xDest, int yDest)
		{ Blt(xDest, yDest, *(CGImage *)&Source, xSrc, ySrc, xSrc + cxWidth, ySrc + cyHeight); }
		inline void BltInto (struct SDL_Surface *src, int x, int y)
		{ assert(0); /* Unk. */}
		inline void BltToSurface (struct SDL_Surface *dst, int x, int y)
		{ assert(0); /* Unk. */}
		inline void BltWithMask (int xSrc, int ySrc, int cxWidth, int cyHeight, const CGImage &Mask, const CGImage &Source, int xDest, int yDest)
		{ assert(0); /* Unk. */}
		inline void ColorTransBlt (int xSrc, int ySrc, int cxWidth, int cyHeight, int iAlpha, CGImage &Source, int xDest, int yDest)
		{ Blt(xDest, yDest, Source, xSrc, ySrc, xSrc + cxWidth, ySrc + cyHeight); }
		inline void DrawPixel(int x, int y, COLORREF wColor) { DrawDot(x, y, wColor, markerPixel); }
		inline void DrawPixelTrans(int x, int y, COLORREF wColor, BYTE bAlpha)
		{ DrawDot(x, y, SetAlphaColor(wColor, bAlpha), markerPixel); }
		inline void DrawPixelTrans(WORD *pPos, COLORREF wColor, BYTE bAlpha)
		{ assert(0); /* Unk., likely remove. */ }
		inline void DrawPlainLine (int x1, int y1, int x2, int y2, COLORREF wColor) { DrawLine(x1, y1, x2, y2, 1, wColor); }
		inline void DrawBiColorLine (int x1, int y1, int x2, int y2, int iWidth, COLORREF wColor1, COLORREF wColor2)
		{ DrawLineBiColor(x1, y1, x2, y2, iWidth, wColor1, wColor2); }
		inline bool HasMask() { return true; }
		inline void Fill(int x, int y, int cxWidth, int cyHeight, COLORREF wColor) { DrawRectFilled(x, y, x + cxWidth, y + cyHeight, wColor); }
		inline void FillRGB(int x, int y, int cxWidth, int cyHeight, COLORREF wColor) { DrawRectFilled(x, y, x + cxWidth, y + cyHeight, wColor); }
		inline void FillColumn (int x, int y, int cyHeight, COLORREF wColor) { DrawLine(x, y, x, y + cyHeight, 1, wColor); }
		inline void FillLine(int x, int y, int cxWidth, COLORREF wColor) { DrawLine(x, y, x + cxWidth, y, 1, wColor); }
		inline void FillLineGray(int x, int y, int cxWidth, COLORREF wColor, BYTE bAlpha)
		{ DrawLine(x, y, x + cxWidth, y, 1, SetAlphaColor(wColor, bAlpha)); }
		inline void FillLineTrans(int x, int y, int cxWidth, COLORREF wColor, BYTE bAlpha)
		{ DrawLine(x, y, x + cxWidth, y, 1, SetAlphaColor(wColor, bAlpha)); }
		inline void DrawAlpha (int xSrc, int ySrc, int cxWidth, int cyHeight, SDL_Surface *Source, COLORREF wColor, int xDest, int yDest, BYTE bAlpha = 0xff)
		{ /* XXX Remove all references. */ assert(0); }
		inline void FillMask (int xSrc, int ySrc, int cxWidth, int cyHeight, const CGImage &Source, COLORREF wColor, int xDest, int yDest, BYTE bAlpha = 0xff)
		{ DrawMask(xDest, yDest, Source, xSrc, ySrc, xSrc + cxWidth, ySrc + cyHeight, SetAlphaColor(wColor, bAlpha)); }
		inline void FillTrans (int x, int y, int cxWidth, int cyHeight, COLORREF wColor, DWORD bAlpha)
		{ DrawRectFilled(x, y, x + cxWidth, y + cyHeight, SetAlphaColor(wColor, bAlpha)); }
		inline void FillTransGray (int x, int y, int cxWidth, int cyHeight, COLORREF wColor, DWORD bAlpha)
		{ DrawRectFilled(x, y, x + cxWidth, y + cyHeight, SetAlphaColor(wColor, bAlpha)); }
		inline void FillTransRGB (int x, int y, int cxWidth, int cyHeight, COLORREF wColor, BYTE bAlpha)
		{ DrawRectFilled(x, y, x + cxWidth, y + cyHeight, SetAlphaColor(wColor, bAlpha)); }
		inline void GaussianScaledBlt (int xSrc, int ySrc, int cxWidth, int cyHeight, CGImage &Source, int xDest, int yDest, int cxDestWidth, int cyDestHeight)
		{ BltRotoZ(xDest, yDest, xDest + cxDestWidth, yDest + cyDestHeight, Source, xSrc, ySrc, xSrc + cxWidth, ySrc + cyHeight, 0, 1); }
		static inline COLORREF BlendPixel (COLORREF pxDest, COLORREF pxSource, BYTE bAlpha)
		{ return BlendColor(pxDest, pxSource, bAlpha); }
		static inline COLORREF BlendPixelGray (COLORREF pxDest, COLORREF pxSource, BYTE bAlpha)
		{ return BlendColor(pxDest, pxSource, bAlpha); }
		static inline COLORREF BlendPixelPM (COLORREF pxDest, COLORREF pxSource, BYTE bAlpha)
		{ return BlendColor(pxDest, pxSource, bAlpha); }
		static inline COLORREF DarkenPixel (COLORREF pxSource, BYTE bAlpha) { return DarkenColor(pxSource, bAlpha); }
		static inline COLORREF LightenPixel (COLORREF pxSource, BYTE bAlpha) { return LightenColor(pxSource, bAlpha); }
		static inline bool IsGrayscaleValue (COLORREF wColor) { return IsGrayscaleColor(wColor); }
		static inline COLORREF GrayscaleValue (COLORREF wValue) { return GrayscaleColor(wValue); }
		static inline COLORREF RGBValue (BYTE bRed, BYTE bGreen, BYTE bBlue) { return RGBColor(bRed, bGreen, bBlue); }
		static inline BYTE BlueValue (COLORREF wColor) { return BlueColor(wColor); }
		static inline BYTE GreenValue (COLORREF wColor) { return GreenColor(wColor); }
		static inline BYTE RedValue (COLORREF wColor) { return RedColor(wColor); }
		static inline COLORREF PixelFromRGB (COLORREF rgb) { return rgb; }
		static inline COLORREF RGBFromPixel (COLORREF wColor) { return wColor; }

	private:
		/* Pixel manipulation routines to convert RGB colors to display format pixels. */
		static inline PIXEL RGBPixel (BYTE bRed, BYTE bGreen, BYTE bBlue)
			{
			return ((bRed & CGImage::DispPixFormat->Rmask) << CGImage::DispPixFormat->Rshift) |
					((bGreen & CGImage::DispPixFormat->Gmask)  << CGImage::DispPixFormat->Gshift) |
					((bBlue & CGImage::DispPixFormat->Bmask)  << CGImage::DispPixFormat->Bshift);
			}
		static inline PIXEL RGBPixel (COLORREF cColor)
			{
			return ((RedColor(cColor) & CGImage::DispPixFormat->Rmask) << CGImage::DispPixFormat->Rshift) |
					((GreenColor(cColor) & CGImage::DispPixFormat->Gmask)  << CGImage::DispPixFormat->Gshift) |
					((BlueColor(cColor) & CGImage::DispPixFormat->Bmask)  << CGImage::DispPixFormat->Bshift);
			}
		static inline PIXEL RGBAPixel (BYTE bRed, BYTE bGreen, BYTE bBlue, BYTE bAlpha)
			{
			return ((bRed & CGImage::DispPixFormat->Rmask) << CGImage::DispPixFormat->Rshift) |
					((bGreen & CGImage::DispPixFormat->Gmask)  << CGImage::DispPixFormat->Gshift) |
					((bBlue & CGImage::DispPixFormat->Bmask)  << CGImage::DispPixFormat->Bshift) |
					((bAlpha & CGImage::DispPixFormat->Amask)  << CGImage::DispPixFormat->Ashift);
			}
		static inline PIXEL RGBAPixel (COLORREF cColor)
			{
			return ((RedColor(cColor) & CGImage::DispPixFormat->Rmask) << CGImage::DispPixFormat->Rshift) |
					((GreenColor(cColor) & CGImage::DispPixFormat->Gmask)  << CGImage::DispPixFormat->Gshift) |
					((BlueColor(cColor) & CGImage::DispPixFormat->Bmask)  << CGImage::DispPixFormat->Bshift);
					((AlphaColor(cColor) & CGImage::DispPixFormat->Amask)  << CGImage::DispPixFormat->Ashift);
			}
		static inline BYTE AlphaPixel (PIXEL wColor) { return (wColor >> CGImage::DispPixFormat->Ashift) & CGImage::DispPixFormat->Amask; }
		static inline BYTE RedPixel (PIXEL wColor) { return (wColor >> CGImage::DispPixFormat->Rshift) & CGImage::DispPixFormat->Rmask; }
		static inline BYTE GreenPixel (PIXEL wColor) { return (wColor >> CGImage::DispPixFormat->Gshift) & CGImage::DispPixFormat->Gmask; }
		static inline BYTE BluePixel (PIXEL wColor) { return (wColor >> CGImage::DispPixFormat->Bshift) & CGImage::DispPixFormat->Bmask; }

		void DrawCircleGradientLine(const SAlphaGradientCircleLineCtx &Ctx, int x, int y);
		void DrawCircleBltLine (const SBltCircleLineCtx &Ctx, int x, int y);
	};

/* More placeholders. */
typedef class CGImage CG16bitImage;
static inline void DrawAlphaGradientCircle (CGImage &Dest, int xDest, int yDest, int iRadius, COLORREF wColor)
{ Dest.DrawCircleGradient(xDest, yDest, iRadius, wColor); }

static inline void DrawBltCircle (CGImage &Dest, int xDest, int yDest, int iRadius, const CGImage &Src, int xSrc, int ySrc, int cxSrc, int cySrc, BYTE bAlpha)
{ Dest.DrawCircleBlt(xDest, yDest, iRadius, Src, xSrc, ySrc, xSrc + cxSrc, ySrc + cySrc, bAlpha); }

static inline void DrawBrokenLine (CGImage &Dest, int xSrc, int ySrc, int xDest, int yDest, int xyBreak, COLORREF wColor)
{ Dest.DrawLineBroken(xSrc, ySrc, xDest, yDest, xyBreak, wColor); }

static inline void DrawFilledCircle (CGImage &Dest, int xDest, int yDest, int iRadius, COLORREF wColor)
{ Dest.DrawCircleFilled(xDest, yDest, iRadius, wColor); }

static inline void DrawFilledCircleGray (CGImage &Dest, int xDest, int yDest, int iRadius, COLORREF wColor, BYTE bAlpha)
{ Dest.DrawCircleFilled(xDest, yDest, iRadius, CGImage::SetAlphaColor(wColor, bAlpha)); }

static inline void DrawFilledCircleTrans (CGImage &Dest, int xDest, int yDest, int iRadius, COLORREF wColor, BYTE bAlpha)
{ Dest.DrawCircleFilled(xDest, yDest, iRadius, CGImage::SetAlphaColor(wColor, bAlpha)); }

static inline void DrawGlowRing (CGImage &Dest, int xDest, int yDest, int iRadius, int iRingThickness, COLORREF *wColorRamp, BYTE *bAlphaRamp)
{ Dest.DrawRingGradient(xDest, yDest, iRadius, iRingThickness, wColorRamp, bAlphaRamp); }

static inline void DrawRectDotted (CGImage &Dest, int x, int y, int cxWidth, int cyHeight, COLORREF wColor)
{ Dest.DrawRectDotted(x, y, x + cxWidth, y + cyHeight, wColor); }

static inline void DrawDottedLine (CGImage &Dest, int x, int y, int x2, int y2, COLORREF wColor)
{ Dest.DrawLineDotted(x, y, x2, y2, wColor); }

#endif

