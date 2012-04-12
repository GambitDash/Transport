//	CObjectImage.cpp
//
//	CObjectImage class

#include "PreComp.h"

#include "CFile.h"
#include "CFileDirectory.h"

#include "SDL_image.h"

#define UNID_ATTRIB							CONSTLIT("UNID")
#define BITMAP_ATTRIB						CONSTLIT("bitmap")
#define BITMASK_ATTRIB						CONSTLIT("bitmask")
#define BACK_COLOR_ATTRIB					CONSTLIT("backColor")
#define SPRITE_ATTRIB						CONSTLIT("sprite")
#define NO_PM_ATTRIB						CONSTLIT("noPM")

CObjectImage::CObjectImage (void) :
		m_dwUNID(0),
		m_pBitmap(NULL)

//	CObjectImage constructor

	{
	}

CObjectImage::CObjectImage (CG16bitImage *pBitmap) :
		m_dwUNID(0),
		m_bTransColor(false),
		m_bSprite(false),
		m_bPreMult(false),
		m_pBitmap(pBitmap),
		m_bMarked(false),
		m_bLocked(true)

//	CObjectImage constructor

	{
	ASSERT(pBitmap);
	}

CObjectImage::~CObjectImage (void)

//	CObjectImage destructor

	{
	//	If we get created with an UNID of 0 it means that someone else owns the bitmap
	//	This is needed by CObjectImageArray.

	if (m_pBitmap && m_dwUNID)
		delete m_pBitmap;
	}

ALERROR CObjectImage::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CObjectImage **retpImage)

//	CreateFromXML
//
//	Creates the object from and XML element

	{
	CObjectImage *pNewImage = new CObjectImage;

	pNewImage->m_dwUNID = LoadUNID(Ctx, pDesc->GetAttribute(UNID_ATTRIB));
	pNewImage->m_sResourceDb = Ctx.sResDb;
	if (Ctx.sFolder.IsBlank())
		{
		pNewImage->m_sBitmap = pDesc->GetAttribute(BITMAP_ATTRIB);
		pNewImage->m_sBitmask = pDesc->GetAttribute(BITMASK_ATTRIB);
		}
	else
		{
		CString sFilespec;
		if (pDesc->FindAttribute(BITMAP_ATTRIB, &sFilespec))
			pNewImage->m_sBitmap = pathAddComponent(Ctx.sFolder, sFilespec);

		if (pDesc->FindAttribute(BITMASK_ATTRIB, &sFilespec))
			pNewImage->m_sBitmask = pathAddComponent(Ctx.sFolder, sFilespec);
		}

	//	Transparent color

	CString sTransColor;
	if (pNewImage->m_bTransColor = pDesc->FindAttribute(BACK_COLOR_ATTRIB, &sTransColor))
		pNewImage->m_wTransColor = LoadRGBColor(sTransColor);

	//	Sprite

	pNewImage->m_bSprite = pDesc->GetAttributeBool(SPRITE_ATTRIB);

	//	Pre-multiply transparency

	pNewImage->m_bPreMult = !pDesc->GetAttributeBool(NO_PM_ATTRIB);

	//	Initialize

	pNewImage->m_bMarked = false;
	pNewImage->m_bLocked = false;
	pNewImage->m_pBitmap = NULL;

	//	Done

	*retpImage = pNewImage;

	return NOERROR;
	}

CG16bitImage *CObjectImage::GetImage (CString *retsError)

//	GetImage
//
//	Returns the image, loading it if necessary

	{
	ALERROR error;
	SDL_Surface *img = NULL;
	SDL_Surface *bitmask = NULL;

	if (m_pBitmap)
		return m_pBitmap;

	if (!m_sBitmap.IsBlank())
		{
		if (!strEquals(m_sResourceDb, g_pUniverse->GetResourceDb()))
			{
			m_sBitmap = pathAddComponent(pathGetPath(m_sResourceDb), m_sBitmap);
			}

		// Load the bitmap.
		img = IMG_Load(pathSanitize(m_sBitmap).GetPointer());
		if (!img)
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("Unable to load bitmap image '%s': %s"), pathSanitize(m_sBitmap).GetPointer(), IMG_GetError());
			return NULL;
			}
		}

	if (!m_sBitmask.IsBlank())
		{
		if (!strEquals(m_sResourceDb, g_pUniverse->GetResourceDb()))
			{
			m_sBitmask = pathAddComponent(pathGetPath(m_sResourceDb), m_sBitmask);
			}

		// Load the bitmask.
		bitmask = IMG_Load(pathSanitize(m_sBitmask).GetPointer());
		if (!bitmask)
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("Unable to load bitmask image '%s': %s"), pathSanitize(m_sBitmask).GetPointer(), IMG_GetError());
			SDL_FreeSurface(img);
			return NULL;
			}
		}

	// Create a new CG16bitImage for the result.
	m_pBitmap = new CG16bitImage;
	if (m_pBitmap == NULL)
		{
		if (retsError)
			*retsError = CONSTLIT("Out of memory");
		SDL_FreeSurface(img);
		SDL_FreeSurface(bitmask);
		return NULL;
		}

	// Place the new image data into the CG16bitImage
	error = m_pBitmap->CreateFromSurface(img, bitmask, m_bPreMult);
	if (error)
		{
		if (retsError)
			*retsError = strPatternSubst(CONSTLIT("Unable to create bitmap from image: '%s'"), m_sBitmap.GetPointer());
		delete m_pBitmap;
		m_pBitmap = NULL;
		SDL_FreeSurface(img);
		SDL_FreeSurface(bitmask);
		return NULL;
		}
	
	if (m_bTransColor)
		m_pBitmap->SetTransparentColor(m_wTransColor);

	// XXX Useful snippet to save sample files.
	//CString s;
	//s = strPatternSubst(CONSTLIT("res/%s.full.png"), pathGetFilename(pathSanitize(m_sBitmap).GetPointer()).GetPointer());
	//SaveImg(m_pBitmap, s.GetPointer());

	//	Convert to sprite
	if (m_bSprite)
		{
		m_pBitmap->ConvertToSprite();
		}
	
	return m_pBitmap;
	}

ALERROR CObjectImage::Lock (SDesignLoadCtx &Ctx)

//	Locks the image so that it is always loaded

	{
	CG16bitImage *pImage = GetImage(&Ctx.sError);
	if (pImage == NULL)
		return ERR_FAIL;

	m_bLocked = true;

	return NOERROR;
	}

void CObjectImage::Sweep (void)

//	Sweep
//
//	Garbage collect the image, if it is not marked (i.e., in use)

	{
	if (!m_bLocked && !m_bMarked && m_pBitmap)
		{
		delete m_pBitmap;
		m_pBitmap = NULL;
		}
	}
