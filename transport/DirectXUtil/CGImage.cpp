//	CG16bitImage.cpp
//
//	Implementation of raw 16-bit image object

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_gfxPrimitives.h"
#include "SDL_rotozoom.h"

#include "png.h"

#include "portage.h"

#include "Kernel.h"
#include "CMath.h"
#include "CStringArray.h"
#include "CFileDirectory.h"

#include "CGImage.h"
#include "CG16bitFont.h"

#include "CGImageLine.hpp"

AlphaArray5 g_Alpha5 [256];
AlphaArray6 g_Alpha6 [256];

static int SDL_SetAlphaChannel(SDL_Surface *surface, Uint8 value);
static int SDL_SetAlphaChannel(SDL_Surface *surface, int x, int y, int x2, int y2, Uint8 value);

// Mysteriously absent from the libraries
static int SDL_SetAlphaChannel(SDL_Surface *surface, Uint8 value)
{
	return SDL_SetAlphaChannel(surface, 0, 0, surface->w, surface->h, value);
}

static int SDL_SetAlphaChannel(SDL_Surface *surface, int x, int y, int x2, int y2, Uint8 value)
{
	int row, col;
	int offset;
	Uint8 *buf;

	if ( (surface->format->Amask != 0xFF000000) &&
	     (surface->format->Amask != 0x000000FF) ) {
		SDL_SetError("Unsupported surface alpha mask format");
		return -1;
	}

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	if ( surface->format->Amask == 0xFF000000 ) {
			offset = 3;
	} else {
			offset = 0;
	}
#elif SDL_BYTEORDER == SDL_BIG_ENDIAN
	if ( surface->format->Amask == 0xFF000000 ) {
			offset = 0;
	} else {
			offset = 3;
	}
#else
#error Invalid SDL byte order specified.
#endif /* Byte ordering */

	/* Quickly set the alpha channel of an RGBA or ARGB surface */
	if ( SDL_MUSTLOCK(surface) ) {
		if ( SDL_LockSurface(surface) < 0 ) {
			return -1;
		}
	}
	row = y2;
	while (row-- > y) {
		col = x2;
		buf = (Uint8 *)surface->pixels + row * surface->pitch +
			x * surface->format->BytesPerPixel + offset;
		while(col-- > x) {
			*buf = value;
			buf += 4;
		}
	}
	if ( SDL_MUSTLOCK(surface) ) {
		SDL_UnlockSurface(surface);
	}
	return 0;
}

#define EPSILON(x) ((x) <= 0.001 && (x) >= -0.001)

#define SDL_DoLockSurface(srf)															\
		if (SDL_MUSTLOCK(srf))																\
			if (SDL_LockSurface(srf) < 0)

#define SDL_DoUnlockSurface(srf)															\
	if (SDL_MUSTLOCK(srf))																	\
		{																							\
		SDL_UnlockSurface(srf);																\
		}
 
struct SDL_PixelFormat *CGImage::DispPixFormat;
ALERROR SaveSurface(SDL_Surface *surface, const char *const fname);

static void SDL_GetPixelRGBA(SDL_Surface *surface, int x, int y, BYTE *r, BYTE *g, BYTE *b, BYTE *a);
static COLORREF SDL_GetPixel(SDL_Surface *surface, int x, int y);
static void SDL_SetPixelAlpha(SDL_Surface *surface, int x, int y, BYTE alpha);
static void SDL_SetPixelRGBA(SDL_Surface *surface, int x, int y, BYTE r, BYTE g, BYTE b, BYTE a = 0xff);
void SDL_SetPixel(SDL_Surface *surface, int x, int y, COLORREF c)
{ SDL_SetPixelRGBA(surface, x, y, CGImage::RedColor(c), CGImage::GreenColor(c), CGImage::BlueColor(c), CGImage::AlphaColor(c)); }

/* Convenience function to save CG16bitImages to a test file. */
ALERROR SaveImg(CGImage *img, const char *fn)
	{
	return SaveSurface(img->m_Img, fn);
	}
	      
/* Convenience function to save SDL_Surface objects to a test file.  */
ALERROR SaveSurfaceBMP(SDL_Surface *img, const char *fn)
	{
	ALERROR err;
	err = SDL_SaveBMP(img, fn) < 0 ? ERR_FAIL : NOERROR;
	return err;
	}

/* Write an SDL_Surface with full 32-bit worth of data to a PNG file. */
ALERROR SaveSurface(SDL_Surface *surface, const char *const fname)
	{
	png_structp pPng;
	png_infop	pPngInfo;
	BYTE **png_rows;
	Uint8 r, g, b, a;
	int x, y;
	bool bAlpha = surface->format->Amask;
	char cWidth = bAlpha ? 4 : 3;
	char bpp = bAlpha ? 32 : 24;
	FILE *fOut = NULL;

	pPng = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (pPng == NULL)
		{
		kernelDebugLogMessage("Unable to create png write context.");
		return ERR_FAIL;
		}
	pPngInfo = png_create_info_struct(pPng);
	if (pPngInfo == NULL)
		{
		kernelDebugLogMessage("Unable to create png info struct.");
		png_destroy_write_struct(&pPng, (png_infopp) NULL);
		return ERR_FAIL;
		}

	if (setjmp(png_jmpbuf(pPng)))
		{
		kernelDebugLogMessage("Failed to write png.");
		png_destroy_write_struct(&pPng, &pPngInfo);
		if (fOut != NULL) fclose(fOut);
		return ERR_FAIL;
		}

	fOut = fopen(fname, "w");
	if (fOut == NULL)
		{
		kernelDebugLogMessage("Unable to open '%s' for writing.", fname);
		png_destroy_write_struct(&pPng, &pPngInfo);
		return ERR_FAIL;
		}

	png_init_io(pPng, fOut);

	png_set_IHDR(pPng, pPngInfo, surface->w, surface->h, bpp / cWidth,
			bAlpha ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	png_write_info(pPng, pPngInfo);

	/* Save the picture: */
	png_rows = new BYTE*[surface->h];

	for (y = 0; y < surface->h; y++)
		{
		png_rows[y] = new BYTE[cWidth * surface->w];

		for (x = 0; x < surface->w; x++)
			{
			SDL_GetPixelRGBA(surface, x, y, &r, &g, &b, &a);

			png_rows[y][x * cWidth + 0] = r;
			png_rows[y][x * cWidth + 1] = g;
			png_rows[y][x * cWidth + 2] = b;
			if (bAlpha)
				png_rows[y][x * cWidth + 3] = a;
			}
		}

	png_write_image(pPng, png_rows);

	for (y = 0; y < surface->h; y++)
		delete []png_rows[y];

	delete []png_rows;

	png_write_end(pPng, NULL);

	png_destroy_write_struct(&pPng, &pPngInfo);

	fclose(fOut);

	return NOERROR;
}

static void SDL_GetPixelRGBA(SDL_Surface *surface, int x, int y, BYTE *r, BYTE *g, BYTE *b, BYTE *a)
	{
	BYTE *p = ((BYTE *)surface->pixels) + y * surface->pitch + x * surface->format->BytesPerPixel;
	switch (surface->format->BytesPerPixel)
		{
		case 1:
			if (surface->format->palette && *p < surface->format->palette->ncolors)
				{
				SDL_Color *c = surface->format->palette->colors + *p;
				*r = c->r;
				*g = c->g;
				*b = c->b;
				*a = c->r;
				}
			else
				{
				*r = *g = *b = *a = *p;			/* If it's 8-bit, then we treat it as monochrome. */
				}
			return;
		case 2:
			SDL_GetRGBA(*(WORD *)p, surface->format, r, g, b, a);
			break;
		case 3:
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
				SDL_GetRGBA(p[0] << 16 | p[1] << 8 | p[2], surface->format, r, g, b, a);
			else
				SDL_GetRGBA(p[0] | p[1] << 8 | p[2] << 16, surface->format, r, g, b, a);
			break;
		case 4:
			SDL_GetRGBA(*(DWORD *)p, surface->format, r, g, b, a);
			break;
		default:
			return;
		}
		/* If it's a pseudo-grayscale image without an alpha channel, fake it. */
		if (*r == *g && *g == *b && surface->format->Amask == 0)
			*a = *r;
	}

static COLORREF SDL_GetPixel(SDL_Surface *surface, int x, int y)
	{
	BYTE r, g, b, a;
	SDL_GetPixelRGBA(surface, x, y, &r, &g, &b, &a);
	return CGImage::RGBAColor(r, g, b, a);
	}

static void SDL_SetPixelAlpha(SDL_Surface *surface, int x, int y, BYTE alpha)
	{
	BYTE *p = ((BYTE *)surface->pixels) + y * surface->pitch + x * surface->format->BytesPerPixel;

	if (surface->format->BytesPerPixel > 1 && !surface->format->Amask)
		{
		kernelDebugLogMessage("No alpha channel with %d bpp", surface->format->BitsPerPixel);
		exit(0);
		}

	switch (surface->format->BytesPerPixel)
		{
		case 1:
			*p = alpha;			/* If it's 8-bit, then we treat it as monochrome. */
			break;
		case 2:
		case 3:
			break;
		case 4:
			*(DWORD *)p = ((*(DWORD *)p) & ~surface->format->Amask) | ((alpha << surface->format->Ashift) & surface->format->Amask);
			break;
		default:
			break;
		}
	}

#define SDL_XY_TO_IDX(surface, x, y) ((surface)->w * (y) + (x))
#define SDL_COLORREF_TO_RGBA(cr) (((cr) >> 16) & 0xff), (((cr) >> 8) & 0xff), ((cr) & 0xff), (((cr) >> 24) & 0xff)  
#define SDL_COLORREF_TO_RGB(cr) (((cr) >> 16) & 0xff), (((cr) >> 8) & 0xff), ((cr) & 0xff) 

static void SDL_SetPixelRGBA(SDL_Surface *surface, int x, int y, BYTE r, BYTE g, BYTE b, BYTE a)
	{
	BYTE *p = ((BYTE *)surface->pixels) + y * surface->pitch + x * surface->format->BytesPerPixel;

	DWORD pixel = SDL_MapRGBA(surface->format, r, g, b, a);
	switch (surface->format->BytesPerPixel)
		{
		case 1:
			*p = r;
			break;
		case 2:
			*(WORD *)p = pixel;

		case 3:
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
				{
				p[0] = (pixel >> 16) & 0xff;
				p[1] = (pixel >> 8) & 0xff;
				p[2] = pixel & 0xff;
				}
			else
				{
				p[0] = pixel & 0xff;
				p[1] = (pixel >> 8) & 0xff;
				p[2] = (pixel >> 16) & 0xff;
				}
			break;
		case 4:
			*(DWORD *)p = pixel;
			break;
		default:
			break;
		}
	}

CGImage::CGImage (void) : CObject(NULL), m_Img(NULL), m_bEnableAlpha(true)
	{
	}

CGImage::~CGImage () 
	{
	DiscardSurface();
	}


bool CGImage::AdjustCoords (int *xSrc, int *ySrc, int cxSrc, int cySrc, int *xDest, int *yDest, int *cxWidth, int *cyHeight) const
	{
	/* XXX */ assert(false); return false;
	}

/* Creation functions. */
ALERROR CGImage::Create (int cxWidth, int cyHeight, BYTE depth)
	{
	DiscardSurface();
	if (depth == 0 || depth == 32) {
		m_Img = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, cxWidth, cyHeight,
				CGImage::DispPixFormat->BitsPerPixel, CGImage::DispPixFormat->Rmask,
				CGImage::DispPixFormat->Gmask, CGImage::DispPixFormat->Bmask,
				CGImage::DispPixFormat->Amask);
	} else {
		assert(depth == 8);
		m_Img = SDL_CreateRGBSurface(SDL_SWSURFACE, cxWidth, cyHeight, depth, 0xFF, 0, 0, 0);
	}

	if (m_Img == NULL)
		{
		kernelDebugLogMessage("Surface creation failed: %s", SDL_GetError());
		return ERR_MEMORY;
		}
	return NOERROR;
	}

ALERROR CGImage::CreateFromFile (CString &sFilename)
	{
	SDL_Surface *img;

	kernelDebugLogMessage("Loading '%s'", pathSanitize(sFilename).GetPointer());

	img = IMG_Load(pathSanitize(sFilename).GetPointer());
	if (img == NULL)
		{
		kernelDebugLogMessage("Unable to load '%s': %s", pathSanitize(sFilename).GetPointer(), IMG_GetError());
		return ERR_FAIL;
		}

	m_Img = SDL_DisplayFormatAlpha(img);
	if (m_Img == NULL)
		{
		kernelDebugLogMessage("Unable to blit '%s' into memory: %s", pathSanitize(sFilename).GetPointer(), SDL_GetError());
		SDL_FreeSurface(img);
		return ERR_FAIL;
		}
	return NOERROR;
	}

ALERROR CGImage::CreateFromSurface (struct SDL_Surface *sData, struct SDL_Surface *sAlpha, bool _notused)
	{
	DiscardSurface();

	if (sData == NULL && sAlpha != NULL)
		{
		Create(sAlpha->w, sAlpha->h, 8);
		}
	else
		{
		if (sAlpha == NULL)
			{
			m_Img = SDL_DisplayFormatAlpha(sData);
			SDL_SetAlpha(m_Img, 0, 0);
			}
		else
			{
			m_Img = SDL_DisplayFormatAlpha(sData);
			}

		if (m_Img == NULL)
			{
			kernelDebugLogMessage("Unable to create %ssurface: %s", sAlpha ? "alpha " : "", SDL_GetError());
			return ERR_FAIL;
			}
		}

	if (sAlpha)
		{
		if (sAlpha->w != m_Img->w || sAlpha->h != m_Img->h)
			{
			kernelDebugLogMessage("Alpha mask layer size (%u, %u) does not match image data size (%u, %u)", sAlpha->w, sAlpha->h, m_Img->w, m_Img->h);
			DiscardSurface();
			return ERR_FAIL;
			}

		if (BltToAlpha(sAlpha) != NOERROR)
			{
			kernelDebugLogMessage("Failed to blit alpha to surface");
			return ERR_FAIL;
			}
		}

#if 0 /* XXX Unused for image loading problems. */
		{
		static int img_id = 0;
		static char name[100];
		printf("\t%d.\t%s %s\n", img_id, sData ? "image" : "     ", sAlpha ? "alpha" : "");
		sprintf(name, "res/%d.png", img_id);
		img_id++;
		SaveImg(this, name);
		}
#endif
	return NOERROR;
	}

void CGImage::CopyAlpha (int xSrc, int ySrc, int cxWidth, int cyHeight, const CGImage &Soruce, int xDest, int yDest) 
	{
	/* XXX */ assert(false);
	}


/* Destruction functions. */
void CGImage::DiscardSurface (void) 
	{
	if (m_Img != NULL)
		{
		SDL_FreeSurface(m_Img);
		}
	m_Img = NULL;
	}


/* Utility functions to bind an SDL_Surface to this object. */
ALERROR CGImage::BindSurface(struct SDL_Surface *img) 
	{
	if (img != m_Img)
		DiscardSurface();
	m_Img = img;
	return NOERROR;
	}

ALERROR CGImage::UnbindSurface() 
	{
	m_Img = NULL;
	return NOERROR;
	}


/* Different ways to push data around. */
void CGImage::Blt (int x, int y, CGImage &Source, int xSrc, int ySrc, int x2Src, int y2Src, bool bUseAlpha)
	{
	SDL_Rect srcrect;
	SDL_Rect dstrect;
	DWORD flags = 0;
	DWORD alpha = 0;
	
	srcrect.x = xSrc;
	srcrect.y = ySrc;
	srcrect.w = x2Src - xSrc;
	srcrect.h = y2Src - ySrc;

	dstrect.x = x;
	dstrect.y = y;
	dstrect.w = 0;
	dstrect.h = 0;

	if (!bUseAlpha || !m_bEnableAlpha)
		{
		// Save the old alpha and flags
		flags = Source.m_Img->flags & (SDL_RLEACCEL | SDL_SRCALPHA);
		alpha = Source.m_Img->format->alpha;
		SDL_SetAlpha(Source.m_Img, flags & SDL_RLEACCEL, 0);
		}

	if (SDL_BlitSurface(Source.m_Img, &srcrect, m_Img, &dstrect) < 0)
		{
		kernelDebugLogMessage("Blit failure: %s", SDL_GetError());
		}

	if (!bUseAlpha || !m_bEnableAlpha)
		{
		// Reset the old alpha and flags
		SDL_SetAlpha(Source.m_Img, flags, alpha);
		}
	}

void CGImage::RotoZXY(int x, int y, int x2, int y2, int xSrc, int ySrc, int x2Src, int y2Src, double degree, int *w, int *h)
	{
	double zoomx, zoomy;

	zoomx = ((double)(x2 - x))/((double)(x2Src - xSrc));
	zoomy = ((double)(y2 - y))/((double)(y2Src - ySrc));

	rotozoomSurfaceSizeXY(x2Src - xSrc, y2Src - ySrc, degree, zoomx, zoomy, w, h);
	}

void CGImage::BltRotoZ (int x, int y, int x2, int y2, CGImage &Source,
		int xSrc, int ySrc, int x2Src, int y2Src, double degree, int smooth, bool srcAlpha) 
	{
	CGImage img;
	SDL_Surface *surface;
	double zoomx, zoomy;

	zoomx = ((double)(x2 - x))/((double)(x2Src - xSrc));
	zoomy = ((double)(y2 - y))/((double)(y2Src - ySrc));

	/* XXX I don't think this does the rotation and zooming in quite the right order.  Perhaps they should be done in two discrete steps, though that will be very expensive. */
//	printf("Early zoom: %f x %f\n", zoomx, zoomy);
//	rotozoomSurfaceSizeXY(x2Src - xSrc, y2Src - ySrc, degree, zoomx, zoomy, &w, &h);

	/* Recalculate the zoom factors. */
//	zoomx = ((double)(w))/((double)(x2Src - xSrc));
//	zoomy = ((double)(h))/((double)(y2Src - ySrc));
//	printf("Late zoom [%d, %d]: %f x %f\n", w, h, zoomx, zoomy);

	img.Create(x2Src - xSrc, y2Src - ySrc);

	// Copy the full image, including alpha values, into the temporary.
	img.Blt(0, 0, Source, xSrc, ySrc, x2Src, y2Src, false);

	//printf("Roto: %f deg, %f x %f\n", degree, zoomx, zoomy);
	if (!EPSILON(zoomx) || !EPSILON(zoomy) || !EPSILON(degree))
		{
		if (EPSILON(degree))
			{
			surface = zoomSurface(img.m_Img, zoomx, zoomy, smooth);
			}
		else
			{
			surface = rotozoomSurfaceXY(img.m_Img, degree, zoomx, zoomy, smooth);
			}
		}
	else
		{
		// Nothing needed to be done.
		surface = img.m_Img;
		}

	img.BindSurface(surface);

	Blt(x, y, img, 0, 0, x2 - x, y2 - y, srcAlpha);
	}

ALERROR CGImage::BltToAlpha (struct SDL_Surface *sAlpha)
{
	int x, y;

	if (m_Img->format->Amask == 0 && m_Img->format->BitsPerPixel != 8)
		{
		kernelDebugLogMessage("No alpha mask available on target image (%d-bit)", m_Img->format->BitsPerPixel);
		DiscardSurface();
		return ERR_FAIL;
		}

	for (y = 0; y < sAlpha->h; y++)
		{
		for (x = 0; x < sAlpha->w; x++)
			{
			BYTE r, g, b, a;
			SDL_GetPixelRGBA(sAlpha, x, y, &r, &g, &b, &a);
			SDL_SetPixelAlpha(m_Img, x, y, a);
			}
		}
	return NOERROR;
}

void CGImage::SetAlpha (BYTE bAlpha) 
	{
	if (SDL_SetAlpha(m_Img, SDL_SRCALPHA, bAlpha) < 0)
		kernelDebugLogMessage("0x%08x: SetAlpha(0x%08x, %d) failure: %s", this, m_Img, bAlpha, SDL_GetError());
	}

void CGImage::SetAlphaChannel (int x, int y, int x2, int y2, BYTE bAlpha) 
	{
	SDL_SetAlphaChannel(m_Img, x, y, x2, y2, bAlpha);
	}

void CGImage::SetAlphaChannel (BYTE bAlpha) 
	{
	SDL_SetAlphaChannel(m_Img, bAlpha);
	}

BYTE CGImage::GetAlpha ()
	{ return m_Img->format->alpha; }

void CGImage::SetTransparentColor (COLORREF wColor) 
	{
	int x, y;
	BYTE r,g,b,a;
	BYTE cr = CGImage::RedColor(wColor),
		  cg = CGImage::GreenColor(wColor),
		  cb = CGImage::BlueColor(wColor);

	if (!(m_Img->flags & SDL_SRCALPHA))
		SDL_SetAlpha(m_Img, SDL_SRCALPHA, 0);

	for (y = 0; y < m_Img->h; y++)
		{
		for (x = 0; x < m_Img->w; x++)
			{
			SDL_GetPixelRGBA(m_Img, x, y, &r, &g, &b, &a);
			if (r == cr && g == cg && b == cb)
				{
				SDL_SetPixelAlpha(m_Img, x, y, 0);		// Clear the alpha.
				}
//			else
//				SDL_SetPixel(m_Img, x, y, pix | ~mask);	// Set the alpha.
			}
		}
	}

COLORREF CGImage::GetTransparentColor ()
	{ return m_Img->format->colorkey; }

void CGImage::SetBackColor (COLORREF wColor)
	{
	/* XXX Is this the fill color? */
	}

COLORREF CGImage::GetBackColor () const
	{
	return 0;
	}

void CGImage::SetClipRect (const RECT &rcClip)
	{
		SDL_Rect rect;

		rect.x = rcClip.left;
		rect.y = rcClip.top;
		rect.w = rcClip.right - rcClip.left;
		rect.h = rcClip.bottom - rcClip.top;

		SDL_SetClipRect(m_Img, &rect);
	}

RECT CGImage::GetClipRect (void) const
	{
		SDL_Rect rect;
		SDL_GetClipRect(m_Img, &rect);

		return RECT(rect.y, rect.y + rect.h, rect.x + rect.w, rect.x);
	}

void CGImage::ResetClipRect (void)
	{
	SDL_SetClipRect(m_Img, NULL);
	}
	
bool CGImage::Intersect(int x, int y, int x2, int y2, CGImage &Source, int xSrc, int ySrc)
	{
	BYTE r,g,b,a,rs,gs,bs,as;

	for (int yb = 0; yb < (y2 - y); yb++)
		{
		for (int xb = 0; xb < (x2 - x); xb++)
			{
			if ((x + xb) >= GetWidth() || (y + yb) >= GetHeight() ||
					(xSrc + xb) >= Source.GetWidth() || (ySrc + yb) >= Source.GetHeight())
				continue;
			SDL_GetPixelRGBA(m_Img, x + xb, y + yb, &r, &g, &b, &a);
  			SDL_GetPixelRGBA(Source.m_Img, xSrc + xb, ySrc + yb, &rs, &gs, &bs, &as);

			if (((m_Img->format->Amask && a) || ((m_Img->format->Amask == 0) && (r || g || b))) &&
					((Source.m_Img->format->Amask && as) || ((Source.m_Img->format->Amask == 0) && (rs || gs || bs))))
				{
				//printf("Intersection at [%d, %d]:[%x,%x,%x,%x] and [%d, %d]:[%x,%x,%x,%x]\n",
				//		x + xb, y + yb, r, g, b, a, xSrc + xb, ySrc + yb, rs, gs, bs, as);
				return true;
				}
			}
		}
	return false;
	}

/* Static 32-bit pixel utility functions. */
COLORREF CGImage::BlendColor (COLORREF pxDest, COLORREF pxSource, BYTE alpha)
	{
	if (alpha == 255)
		{
		return pxSource;
		}
	else
		{
		COLORREF Rshift, Gshift, Bshift, Ashift;
		COLORREF dc = pxDest;
		COLORREF color = pxSource;
		COLORREF R = 0, G = 0, B = 0, A = 0;
		COLORREF Rmask, Gmask, Bmask, Amask;

		Rshift = 16;  Rmask = 0x00FF0000;
		Gshift = 8;   Gmask = 0x0000FF00;
		Bshift = 0;   Bmask = 0x000000FF;
		Ashift = 24;  Amask = 0xFF000000;

		R = ((dc & Rmask) + (((((color & Rmask) - (dc & Rmask)) >> Rshift) * alpha >> 8) << Rshift)) & Rmask;
		G = ((dc & Gmask) + (((((color & Gmask) - (dc & Gmask)) >> Gshift) * alpha >> 8) << Gshift)) & Gmask;
		B = ((dc & Bmask) + (((((color & Bmask) - (dc & Bmask)) >> Bshift) * alpha >> 8) << Bshift)) & Bmask;
		if (Amask)
			A = ((dc & Amask) + (((((color & Amask) - (dc & Amask)) >> Ashift) * alpha >> 8) << Ashift)) & Amask;

		return R | G | B | A;
		}
	}


/* Drawing functions. */
void CGImage::DrawDot (int x, int y, COLORREF wColor, MarkerTypes iMarker) 
	{
	switch (iMarker)
		{
		case markerPixel:
			pixelRGBA(m_Img, x, y, SDL_COLORREF_TO_RGBA(wColor));
			break;
		case markerSmallRound:
			{
			static const BYTE wSmallRoundMask[3][3] = 
				{
					{ 100, 255, 100 },
					{ 255, 255, 255 },
					{ 100, 255, 100 }
				};

			for (int y1 = y - 1; y1 <= y + 1; y1++)
				{
				for (int x1 = x - 1; x1 <= x + 1; x1++)
					{
					pixelRGBA(m_Img, x1, y1, SDL_COLORREF_TO_RGB(wColor),
							wSmallRoundMask[y1 - (y - 1)][x1 - (x - 1)]);
					}
				}
			break;
			}
		case markerSmallSquare:
			{
			const int SMALL_SQUARE_SIZE = 2;
			rectangleRGBA(m_Img, x - SMALL_SQUARE_SIZE, y - SMALL_SQUARE_SIZE,
					x + SMALL_SQUARE_SIZE, y + SMALL_SQUARE_SIZE, SDL_COLORREF_TO_RGBA(wColor));
			break;
			}
		case markerSmallCross:
			{
			const int SMALL_CROSS_SIZE = 2;
			lineRGBA(m_Img, x - SMALL_CROSS_SIZE, y, x + SMALL_CROSS_SIZE, y, SDL_COLORREF_TO_RGBA(wColor));
			lineRGBA(m_Img, x, y - SMALL_CROSS_SIZE, x, y + SMALL_CROSS_SIZE, SDL_COLORREF_TO_RGBA(wColor));
			break;
			}
		case markerMediumCross:
			{
			const int MEDIUM_CROSS_SIZE = 4;
			lineRGBA(m_Img, x - MEDIUM_CROSS_SIZE, y, x + MEDIUM_CROSS_SIZE, y, SDL_COLORREF_TO_RGBA(wColor));
			lineRGBA(m_Img, x, y - MEDIUM_CROSS_SIZE, x, y + MEDIUM_CROSS_SIZE, SDL_COLORREF_TO_RGBA(wColor));
			break;
			}
		case markerSmallFilledSquare:
			{
			const int SMALL_SQUARE_SIZE = 2;
			boxRGBA(m_Img, x - SMALL_SQUARE_SIZE, y - SMALL_SQUARE_SIZE, x + SMALL_SQUARE_SIZE, y + SMALL_SQUARE_SIZE, SDL_COLORREF_TO_RGBA(wColor));
			break;
			}
		}
	}

void CGImage::DrawLine (int x1, int y1, int x2, int y2, int iWidth, COLORREF wColor) 
	{
	if (iWidth == 1)
		{
		lineRGBA(m_Img, x1, y1, x2, y2, SDL_COLORREF_TO_RGBA(wColor));
		} 
	else if (iWidth > 1)
		{
		/* When Cairo grows up and gets proper SDL support, it's lines will be much prettier. */
		y1 = y1 - iWidth / 2;
		y2 = y2 - iWidth / 2;
		for (int i = 0; i < iWidth; i++)
			lineRGBA(m_Img, x1, y1 + i, x2, y2 + i, SDL_COLORREF_TO_RGBA(wColor)); 
		}
	}

void CGImage::DrawLineBiColor (int x1, int y1, int x2, int y2, int iWidth, COLORREF wColor1, COLORREF wColor2) 
	{
	lineRGBABiColor(m_Img, x1, y1, x2, y2, SDL_COLORREF_TO_RGBA(wColor1), SDL_COLORREF_TO_RGBA(wColor2));
	/* XXX Need to implement gradient/pens support for things in gfxPrimitives. */
	//DrawLine(x1, y1, x2, y2, iWidth, wColor1);
	}

void CGImage::DrawLineProcInit (int x1, int y1, int x2, int y2, SDrawLineCtx *pCtx) 
	{
	/* XXX */
	}

void CGImage::DrawLineProc (SDrawLineCtx *pCtx, DRAWLINEPROC pfProc) 
	{
	/* XXX */
	}


void CGImage::DrawText (int x, int y, const CG16bitFont &Font, COLORREF wColor, CString sText, DWORD dwFlags, int *retx) 
	{
	Font.DrawText(*this, x, y, wColor, sText, dwFlags, retx);
	}


void CGImage::DrawRect (int x, int y, int x2, int y2, COLORREF wColor) 
	{
	rectangleRGBA(m_Img, x, y, x2, y2, SDL_COLORREF_TO_RGBA(wColor));
	}

void CGImage::DrawRectFilled (int x, int y, int x2, int y2, COLORREF wColor)
	{
	boxRGBA(m_Img, x, y, x2, y2, SDL_COLORREF_TO_RGBA(wColor));
	}

void CGImage::DrawCircleGradientLine(const SAlphaGradientCircleLineCtx &Ctx, int x, int y)
	{
	int xStart = Ctx.xDest - x;
	int xEnd = Ctx.xDest + x + 1;
	const RECT &rcClip = GetClipRect();

	if (xEnd <= rcClip.left || xStart >= rcClip.right)
		return;

	//	See which lines we need to paint

	int yLine = Ctx.yDest - y;
	bool bPaintTop = (yLine >= rcClip.top && yLine < rcClip.bottom);

	yLine = Ctx.yDest + y;
	bool bPaintBottom = ((y > 0) && (yLine >= rcClip.top && yLine < rcClip.bottom));

	//	Compute radius increment

	int iRadius = y;
	int d = -y;
	int deltaE = 3;
	int deltaSE = -2 * y + 1;

	//	Loop

	int xPos = 0;

	//	This will skip the center pixel in the circle (avoids a divide by
	//	zero in the inner loop).

	if (y == 0)
		{
		xPos = 1;
		d += deltaSE;
		deltaE += 2;
		iRadius++;
		}

	//	Blt the line 

	while (xPos <= x)
		{
		//	Figure out the radius of the pixel at this location

		if (d < 0)
			{
			d += deltaE;
			deltaE += 2;
			deltaSE += 2;
			}
		else
			{
			d += deltaSE;
			deltaE += 2;
			//  deltaSE += 0;
			iRadius++;
			}

		//	Compute the transparency based on the radius

		DWORD dwTrans = 255 - (255 * iRadius / Ctx.iRadius);
		if (dwTrans > 255)
			{
			xPos++;
			continue;
			}

		//	Paint

		if (Ctx.xDest - xPos < rcClip.right && Ctx.xDest - xPos >= rcClip.left)
			{
			if (bPaintTop)
				DrawDot(Ctx.xDest - xPos, Ctx.yDest - y, RGBAColor(Ctx.dwRed, Ctx.dwGreen, Ctx.dwBlue, dwTrans), markerPixel);
			if (bPaintBottom)
				DrawDot(Ctx.xDest - xPos, Ctx.yDest + y, RGBAColor(Ctx.dwRed, Ctx.dwGreen, Ctx.dwBlue, dwTrans), markerPixel);
			}

		if (xPos > 0 && Ctx.xDest + xPos < rcClip.right && Ctx.xDest + xPos >= rcClip.left)
			{
			if (bPaintTop)
				DrawDot(Ctx.xDest + xPos, Ctx.yDest - y, RGBAColor(Ctx.dwRed, Ctx.dwGreen, Ctx.dwBlue, dwTrans), markerPixel);
			if (bPaintBottom)
				DrawDot(Ctx.xDest + xPos, Ctx.yDest + y, RGBAColor(Ctx.dwRed, Ctx.dwGreen, Ctx.dwBlue, dwTrans), markerPixel);
			}

		xPos++;
		}
	}

void CGImage::DrawCircleGradient (int xDest, int yDest, int iRadius, COLORREF wColor) 
	{
	//	Deal with edge-conditions

	if (iRadius <= 0)
		{
		DrawDot(xDest, yDest, wColor, markerPixel);
		return;
		}

	//	Initialize some stuff

	int x = 0;
	int y = iRadius;
	int d = 1 - iRadius;
	int deltaE = 3;
	int deltaSE = -2 * iRadius + 5;

	//	Prepare struct

	SAlphaGradientCircleLineCtx Ctx;
	Ctx.xDest = xDest;
	Ctx.yDest = yDest;
	Ctx.iRadius = iRadius;
	Ctx.wColor = wColor;

	//	Pre-compute some color info

	Ctx.dwRed = CGImage::RedColor(wColor);
	Ctx.dwGreen = CGImage::GreenColor(wColor);
	Ctx.dwBlue = CGImage::BlueColor(wColor);

	// Draw center pixel
	
	DrawDot(xDest, yDest, wColor, markerPixel);

	//	Draw central line

	DrawCircleGradientLine(Ctx, iRadius, 0);

	//	Draw lines above and below the center

	int iLastDraw = -1;
	while (y > x)
		{
		if (d < 0)
			{
			d += deltaE;
			deltaE += 2;
			deltaSE += 2;
			}
		else
			{
			d += deltaSE;
			deltaE += 2;
			deltaSE += 4;

			//	Draw lines

			DrawCircleGradientLine(Ctx, x, y);
			iLastDraw = y;

			//	Next

			y--;
			}

		x++;

		//	Draw lines

		if (x != iLastDraw)
			DrawCircleGradientLine(Ctx, y, x);
		}
	}

void CGImage::DrawCircleBltLine (const SBltCircleLineCtx &Ctx, int x, int y)

//	DrawBltCircleLine
//
//	Draws a single horizontal line across the circle. For each point on the line
//	we compute the radius and angle so that we can map a bitmap around the circle
//	(effectively, a polar coordinates map).
//
//	The calculation of the radius uses a modified version of the algorithm
//	described in page 84 of Foley and van Dam. But because we are computing the
//	radius as we advance x, the function that we are computing is:
//
//	F(x,r) = x^2 - r^2 + Y^2
//
//	In which Y^2 is constant (the y coordinate of this line).
//
//	The solution leads to the two decision functions:
//
//	deltaE = 2xp + 3
//	deltaSE = 2xp - 2r + 1
//
//	The calculation of the angle relies on a fast arctangent approximation
//	on page 389 of Graphic Gems II.

	{
	int xStart = Ctx.xDest - x;
	int xEnd = Ctx.xDest + x + 1;
	const RECT &rcClip = GetClipRect();

	if (xEnd <= rcClip.left || xStart >= rcClip.right)
		return;

	//	See which lines we need to paint

	int yLine = Ctx.yDest - y;
	bool bPaintTop = (yLine >= rcClip.top && yLine < rcClip.bottom);

	yLine = Ctx.yDest + y;
	bool bPaintBottom = ((y > 0) && (yLine >= rcClip.top && yLine < rcClip.bottom));

	//	Compute radius increment

	int iRadius = y;
	int d = -y;
	int deltaE = 3;
	int deltaSE = -2 * y + 1;

	//	Compute angle increment

	const int iFixedPoint = 8192;
	int iAngle = 2 * iFixedPoint;
	int angle1 = (y == 0 ? (2 * iFixedPoint) : (iFixedPoint / y));
	int num1 = iFixedPoint * y;

	int cxSrcQuadrant = Ctx.cxSrc / 4;
	int cxSrcHalf = Ctx.cxSrc / 2;

	//	Loop

	int xPos = 0;

	//	This will skip the center pixel in the circle (avoids a divide by
	//	zero in the inner loop).

	if (y == 0)
		{
		xPos = 1;
		d += deltaSE;
		deltaE += 2;
		iRadius++;
		}

	//	Blt the line based on the type of source image
	while (xPos <= x)
		{
		//	Figure out the radius of the pixel at this location

		if (d < 0)
			{
			d += deltaE;
			deltaE += 2;
			deltaSE += 2;
			}
		else
			{
			d += deltaSE;
			deltaE += 2;
		//  deltaSE += 0;
			iRadius++;
			}

		//	If we're beyond the size of the source image then continue

		int yOffset = Ctx.iRadius - iRadius;
		if (yOffset >= Ctx.cySrc || yOffset < 0)
			{
			xPos++;
			continue;
			}

		yOffset += Ctx.ySrc;

		//	Figure out the angle of the pixel at this location

		if (xPos < y)
			iAngle -= angle1;
		else
			iAngle = num1 / xPos;

		int xOffset = iAngle * cxSrcQuadrant / (2 * iFixedPoint);

		//	Paint

		if (Ctx.xDest - xPos < rcClip.right && Ctx.xDest - xPos >= rcClip.left)
			{
			if (bPaintTop)
				DrawDot(Ctx.xDest - xPos, Ctx.yDest - y, Ctx.pSrc->GetPixel(cxSrcHalf - xOffset, yOffset), markerPixel);

			if (bPaintBottom)
				DrawDot(Ctx.xDest - xPos, Ctx.yDest + y, Ctx.pSrc->GetPixel(cxSrcHalf + xOffset, yOffset), markerPixel);
			}

		if (xPos > 0 && Ctx.xDest + xPos < rcClip.right && Ctx.xDest + xPos >= rcClip.left)
			{
			if (bPaintTop)
				DrawDot(Ctx.xDest + xPos, Ctx.yDest + y, Ctx.pSrc->GetPixel(cxSrcHalf + xOffset, yOffset), markerPixel);

			if (bPaintBottom)
				DrawDot(Ctx.xDest + xPos, Ctx.yDest - y, Ctx.pSrc->GetPixel(cxSrcHalf - xOffset - 1, yOffset), markerPixel);
			}

		xPos++;
		}
	}

void CGImage::DrawCircleBlt (int xDest, int yDest,
					int iRadius,
					const CG16bitImage &Src,
					int xSrc,
					int ySrc,
					int x2Src,
					int y2Src,
					BYTE byOpacity)

//	DrawBltCircle
//
//	Takes the source image and blts it as a circle

	{
	//	Deal with edge-conditions

	if (iRadius <= 0)
		return;

	//	Initialize some stuff

	int x = 0;
	int y = iRadius;
	int d = 1 - iRadius;
	int deltaE = 3;
	int deltaSE = -2 * iRadius + 5;

	//	Prepare struct

	SBltCircleLineCtx Ctx;
	Ctx.xDest = xDest;
	Ctx.yDest = yDest;
	Ctx.iRadius = iRadius;
	Ctx.pSrc = &Src;
	Ctx.xSrc = xSrc;
	Ctx.ySrc = ySrc;
	Ctx.cxSrc = x2Src - xSrc;
	Ctx.cySrc = y2Src - ySrc;
	Ctx.byOpacity = byOpacity;

	//	Draw central line

	DrawCircleBltLine(Ctx, iRadius, 0);

	//	Draw lines above and below the center

	int iLastDraw = -1;
	while (y > x)
		{
		if (d < 0)
			{
			d += deltaE;
			deltaE += 2;
			deltaSE += 2;
			}
		else
			{
			d += deltaSE;
			deltaE += 2;
			deltaSE += 4;

			//	Draw lines

			DrawCircleBltLine(Ctx, x, y);
			iLastDraw = y;

			//	Next

			y--;
			}

		x++;

		//	Draw lines

		if (x != iLastDraw)
			DrawCircleBltLine(Ctx, y, x);
		}
	}

void CGImage::DrawLineBroken (int x, int y, int x2, int y2, int xyBreak, COLORREF wColor) 
	{
	/* XXX */
	lineRGBA(m_Img, x, y, x2, y2, SDL_COLORREF_TO_RGBA(wColor));
	}

void CGImage::DrawCircleFilled (int x, int y, int iRadius, COLORREF wColor) 
	{
	filledCircleRGBA(m_Img, x, y, iRadius, SDL_COLORREF_TO_RGBA(wColor));
	}

void CGImage::DrawRingGradient (int x, int y, int iRadius, int iRingThickness, COLORREF *wColorRamp, BYTE *bAlphaRamp)
	{
	/* XXX */
	filledCircleRGBA(m_Img, x, y, iRadius, 0xff, 0, 0, 0xff);
	}

void CGImage::DrawRectDotted (int x, int y, int x2, int y2, COLORREF wColor) 
	{
	/* XXX */
	rectangleRGBA(m_Img, x, y, x2, y2, SDL_COLORREF_TO_RGBA(wColor));
	}

void CGImage::DrawLineDotted (int x, int y, int x2, int y2, COLORREF wColor) 
	{
	/* XXX */
	lineRGBA(m_Img, x, y, x2, y2, SDL_COLORREF_TO_RGBA(wColor));
	}

void CGImage::DrawMask (int x, int y, const CGImage &Src, int xSrc, int ySrc, int x2Src, int y2Src, COLORREF wColor)
	{
	int xs, ys;
	COLORREF ps;
	CGImage *pSrc = (CGImage *)&Src;

	ASSERT(Src.m_Img && Src.m_Img->format->BitsPerPixel == 8);

	if (AlphaColor(wColor) == 0)
		return;

	/* Iterate through the region. */
	for (ys = 0; ys < (y2Src - ySrc); ys++)
		{
		for (xs = 0; xs < (x2Src - xSrc); xs++)
			{
			ps = pSrc->GetPixel(xSrc + xs, ySrc + ys);

			/* Adjust the alpha against the Src value. */
			ps = RGBAColor(RedColor(wColor), GreenColor(wColor), BlueColor(wColor),
				AlphaColor(ps) * (AlphaColor(wColor) / 255.0));

			/* Blit the pixel to the surface with the new alpha. */
			DrawDot(x + xs, y + ys, ps, markerPixel);
			}
		}
	}

/* XXX Yanked from CObjectImageArray */
void CGImage::DrawGlowImage (int x, int y, CGImage &Src, int xSrc, int ySrc, int x2Src, int y2Src, int iGlowSize)

//	GenerateGlowImage
//
//	Generates a mask that looks like a glow. The mask is 0 for all image pixels
//	and for all pixels where there is no glow (thus we can optimize painting
//	of the glow by ignoring 0 values)

	{
	ASSERT(iGlowSize == 4);
	BYTE r,g,b,a;

	const int FILTER_SIZE = 4 + 1; /* iGlowSize + 1, locked to 4 atm. */

	//	We need to create the glow mask. The glow image is larger than the object image (by iGlowSize)
	int iFilterOffset[FILTER_SIZE] =
		{
		-iGlowSize,
		-(iGlowSize / 2),
		0,
		+(iGlowSize / 2),
		+iGlowSize
		};

	const int FIXED_POINT = 65536;

	static int iFilter[FILTER_SIZE][FILTER_SIZE] =
		{
			{	   57,   454,   907,   454,    57	},
			{	  454,  3628,  7257,  3628,   454	},
			{	  907,  7257, 14513,  7257,   907	},
			{	  454,  3628,  7257,  3628,   454	},
			{	   57,   454,   907,   454,    57	},
		};

	int cxSrcWidth = x2Src - xSrc;
	int cySrcHeight = y2Src - ySrc;
	int cxGlowWidth = cxSrcWidth + 2 * iGlowSize;
	int cyGlowHeight = cySrcHeight + 2 * iGlowSize;

	ASSERT((cxGlowWidth + x) <= m_Img->w);
	ASSERT((cyGlowHeight + y) <= m_Img->h);

	//	Loop over every pixel of the destination
	for (int yd = 0; yd < cyGlowHeight; yd++)
		{
		for (int xd = 0; xd < cxGlowWidth; xd++)
			{
			int xs, ys;

			xs = (xSrc - iGlowSize) + xd;
			ys = (ySrc - iGlowSize) + yd;

			if (xs >= xSrc && xs <= x2Src && ys >= ySrc && ys <= y2Src)
				{
				SDL_GetPixelRGBA(Src.m_Img, xs, ys, &r, &g, &b, &a);
				if (a && (r || g || b))
					// Pixel is in use, calculate a basic alpha.
					{
					if ((((DWORD)r + (DWORD)g + (DWORD)b) / 3) < 0x40)
						SDL_SetPixelAlpha(m_Img, xd, yd, 0x60);
					else
						SDL_SetPixelAlpha(m_Img, xd, yd, 0x00);

					continue;
					}
				}

			/*
			 * Walk the surrounding pixels based on the filter.  For each pixel that
			 * is set, add the filter value for that relative position to a total.
			 * Then, roll the total down by some fixed point math value, and max it
			 * at 0xf8.
			 */
			/* Generate a number 0-(iGlowSize+1) indicating the closest pixel to (-iGlowSize) to start with. */
			int xStart = ((xs - iGlowSize) < xSrc ? ((xSrc - (xs - iGlowSize) + (iGlowSize / 2 - 1)) / (iGlowSize / 2)) : 0);
			int xEnd = ((xs + iGlowSize) >= x2Src ? (FILTER_SIZE - (((iGlowSize / 2 + 1) + xs + iGlowSize - x2Src) / (iGlowSize / 2))) : FILTER_SIZE);
			int yStart = ((ys - iGlowSize) < ySrc ? ((ySrc - (ys - iGlowSize) + (iGlowSize / 2 - 1)) / (iGlowSize / 2)) : 0);
			int yEnd = ((ys + iGlowSize) >= y2Src ? (FILTER_SIZE - (((iGlowSize / 2 + 1) + ys + iGlowSize - y2Src) / (iGlowSize / 2))) : FILTER_SIZE);

			int iTotal = 0;

			for (int i = yStart; i < yEnd; i++)
				{
				for (int j = xStart; j < xEnd; j++)
					{
					int xg = xs + iFilterOffset[j];
					int yg = ys + iFilterOffset[i];

					SDL_GetPixelRGBA(Src.m_Img, xg, yg, &r, &g, &b, &a);
					if (a && (r || g || b))
						iTotal += iFilter[i][j];
					}
				}
					
			int iValue = (512 * iTotal / FIXED_POINT);
			SDL_SetPixelAlpha(m_Img, xd, yd, iValue > 0xf8 ? 0xf8 : iValue);
			}
		}
	}

COLORREF CGImage::GetPixel(int x, int y) const
	{
	return SDL_GetPixel(m_Img, x, y);
	}
