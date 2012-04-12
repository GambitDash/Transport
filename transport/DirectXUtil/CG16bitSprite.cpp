//	CG16bitSprite.cpp
//
//	Implementation of raw 16-bit sprite object

#include "portage.h"

#include "CMath.h"
#include "CStringArray.h"
#include "CStream.h"

#include "CG16bitImage.h"
#include "CG16bitSprite.h"

CG16bitSprite::CG16bitSprite (void) :
		m_cxWidth(0),
		m_cyHeight(0),
		m_pCode(NULL),
		m_pLineIndex(NULL)

//	CG16bitSprite constructor

	{
	}

CG16bitSprite::~CG16bitSprite (void)

//	CG16bitSprite destructor

	{
	DeleteData();
	}

void CG16bitSprite::ColorTransBlt (CG16bitImage &Dest, int xDest, int yDest, int xSrc, int ySrc, int cxWidth, int cyHeight)

//	ColorTransBlt
//
//	Blt the sprite

	{
	Dest.Blt(xDest, yDest, m_Img, xSrc, ySrc, xSrc + cxWidth, ySrc + cyHeight);
	}

ALERROR CG16bitSprite::CreateFromImage (const CG16bitImage &Source)

//	CreateFromImage
//
//	Creates the sprite from an image

	{
	/* XXX Need to create a RLE enabled surface. */
	m_Img = Source;
	}

void CG16bitSprite::DeleteData (void)

//	DeleteData
//
//	Delete all data

	{
	m_Img.DiscardSurface();
	}
