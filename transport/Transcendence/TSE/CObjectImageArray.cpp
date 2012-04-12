//	CObjectImageArray.cpp
//
//	CObjectImageArray

#include "PreComp.h"

#include "Kernel.h"
static CObjectClass<CObjectImageArray>g_Class(OBJID_COBJECTIMAGEARRAY, NULL);

#define FLASH_TICKS_ATTRIB				CONSTLIT("flashTicks")
#define ROTATE_OFFSET_ATTRIB			CONSTLIT("rotationOffset")
#define BLENDING_ATTRIB					CONSTLIT("blend")
#define ROTATION_COUNT_ATTRIB			CONSTLIT("rotationCount")
#define X_OFFSET_ATTRIB					CONSTLIT("xOffset")
#define Y_OFFSET_ATTRIB					CONSTLIT("yOffset")

#define LIGHTEN_BLENDING				CONSTLIT("brighten")

#define GLOW_SIZE							4
static char g_ImageIDAttrib[] = "imageID";
static char g_ImageXAttrib[] = "imageX";
static char g_ImageYAttrib[] = "imageY";
static char g_ImageWidthAttrib[] = "imageWidth";
static char g_ImageHeightAttrib[] = "imageHeight";
static char g_ImageFrameCountAttrib[] = "imageFrameCount";
static char g_ImageTicksPerFrameAttrib[] = "imageTicksPerFrame";

CObjectImageArray::CObjectImageArray (void) : CObject(&g_Class),
		m_dwBitmapUNID(0),
		m_pImage(NULL),
		m_pRotationOffset(NULL),
		m_pGlowImages(NULL),
		m_pScaledImages(NULL)

//	CObjectImageArray constructor

	{
	}

CObjectImageArray::CObjectImageArray (const CObjectImageArray &Source) : CObject(&g_Class)

//	CObjectImageArray copy constructor

	{
	CopyFrom(Source);
	}

CObjectImageArray::~CObjectImageArray (void)

//	CObjectImageArray destructor

	{
	if (m_pRotationOffset)
		delete [] m_pRotationOffset;

	if (m_pGlowImages)
		delete [] m_pGlowImages;

	if (m_pScaledImages)
		delete [] m_pScaledImages;

	if (m_pImage && m_dwBitmapUNID == 0)
		delete m_pImage;
	}

CObjectImageArray &CObjectImageArray::operator= (const CObjectImageArray &Source)

//	Operator =

	{
	if (m_pRotationOffset)
		delete [] m_pRotationOffset;

	if (m_pGlowImages)
		delete [] m_pGlowImages;

	if (m_pScaledImages)
		delete [] m_pScaledImages;

	CopyFrom(Source);

	return *this;
	}

int CObjectImageArray::ComputeSourceX (int iTick) const

//	ComputeSourceX
//
//	Computes the x offset of the source based on the tick

	{
	if (m_iFrameCount > 0 && m_iTicksPerFrame > 0)
		{
		if (m_iFlashTicks > 0)
			{
			int iTotal = m_iFlashTicks + m_iTicksPerFrame;
			int iFrame = (((iTick % iTotal) < m_iFlashTicks) ? 1 : 0);
			return m_rcImage.left + iFrame * RectWidth(m_rcImage);
			}
		else
			return m_rcImage.left + ((iTick / m_iTicksPerFrame) % m_iFrameCount) * RectWidth(m_rcImage);
		}
	else
		return m_rcImage.left;
	}

void CObjectImageArray::ComputeRotationOffsets (void)

//	ComputeRotationOffsets
//
//	Compute rotation offsets

	{
	if (m_iRotationOffset != 0 && m_iRotationCount > 0)
		{
		if (m_pRotationOffset)
			delete [] m_pRotationOffset;

		m_pRotationOffset = new OffsetStruct[m_iRotationCount];
		for (int i = 0; i < m_iRotationCount; i++)
			{
			int iRotationAngle = 360 / m_iRotationCount;
			int iAngleAdj = (m_iRotationCount / 4) + 1;
			int iAngle = iRotationAngle * (((m_iRotationCount - (i+1)) + iAngleAdj) % m_iRotationCount);
			CVector vOffset = PolarToVector(iAngle, (Metric)m_iRotationOffset);
			m_pRotationOffset[i].x = (int)vOffset.GetX();
			m_pRotationOffset[i].y = (int)vOffset.GetY();
			}
		}
	}

void CObjectImageArray::ComputeRotationOffsets (int xOffset, int yOffset)

//	ComputeRotationOffsets
//
//	Computer rotation offsets from a fixed offset

	{
	if (m_iRotationCount != 0)
		{
		if (m_pRotationOffset)
			delete [] m_pRotationOffset;

		m_pRotationOffset = new OffsetStruct[m_iRotationCount];
		for (int i = 0; i < m_iRotationCount; i++)
			{
			m_pRotationOffset[i].x = xOffset;
			m_pRotationOffset[i].y = yOffset;
			}
		}
	}

void CObjectImageArray::CopyFrom (const CObjectImageArray &Source)

//	CopyFrom
//
//	Copy image

	{
	m_dwBitmapUNID = Source.m_dwBitmapUNID;
	if (m_dwBitmapUNID)
		m_pImage = Source.m_pImage;
	else
		m_pImage = new CObjectImage(Source.m_pImage->GetImage());
	m_rcImage = Source.m_rcImage;
	m_iFrameCount = Source.m_iFrameCount;
	m_iRotationCount = Source.m_iRotationCount;
	m_iTicksPerFrame = Source.m_iTicksPerFrame;
	m_iFlashTicks = Source.m_iFlashTicks;
	m_iBlending = Source.m_iBlending;
	m_pGlowImages = NULL;
	m_pScaledImages = NULL;

	m_iRotationOffset = Source.m_iRotationOffset;
	if (Source.m_pRotationOffset)
		{
		m_pRotationOffset = new OffsetStruct[m_iRotationCount];
		for (int i = 0; i < m_iRotationCount; i++)
			m_pRotationOffset[i] = Source.m_pRotationOffset[i];
		}
	else
		m_pRotationOffset = NULL;
	}

void CObjectImageArray::CopyImage (CG16bitImage &Dest, int x, int y, int iFrame, int iRotation) const

//	CopyImage
//
//	Copies entire image to the destination

	{
	if (m_pImage)
		{
		CG16bitImage &Source(*m_pImage->GetImage());

		int xSrc = m_rcImage.left + (iFrame * RectWidth(m_rcImage));
		int ySrc = m_rcImage.top + (iRotation * RectHeight(m_rcImage));

		Dest.Blt(x, y, Source, xSrc, ySrc, xSrc + RectWidth(m_rcImage),
				ySrc + RectHeight(m_rcImage), false);
		}
	}

void CObjectImageArray::GenerateGlowImage (int iRotation) const

//	GenerateGlowImage
//
//	Generates a mask that looks like a glow. The mask is 0 for all image pixels
//	and for all pixels where there is no glow (thus we can optimize painting
//	of the glow by ignoring 0 values)

	{
	ASSERT(iRotation >= 0 && iRotation < m_iRotationCount);

	//	Source

	if (m_pImage == NULL)
		return;

	CG16bitImage &Source(*m_pImage->GetImage());

	//	Allocate the array of images (if not already allocated)

	if (m_pGlowImages == NULL)
		m_pGlowImages = new CG16bitImage[m_iRotationCount];

	//	If the image for this rotation has already been initialized, then
	//	we're done

	if (!m_pGlowImages[iRotation].IsEmpty())
		return;

	//	Otherwise we need to create the glow mask. The glow image is larger
	//	than the object image (by GLOW_SIZE)

	int cxSrcWidth = RectWidth(m_rcImage);
	int cySrcHeight = RectHeight(m_rcImage);
	int cxGlowWidth = cxSrcWidth + 2 * GLOW_SIZE;
	int cyGlowHeight = cySrcHeight + 2 * GLOW_SIZE;

	if (m_pGlowImages[iRotation].Create(cxGlowWidth, cyGlowHeight, 8) != NOERROR)
		{
		m_pGlowImages[iRotation].DiscardSurface();
		kernelDebugLogMessage("Unable to create image");
		return;
		}

	RECT rcSrc;
	rcSrc.left = ComputeSourceX(0);
	rcSrc.top = m_rcImage.top + (iRotation * cySrcHeight);
	rcSrc.right = rcSrc.left + cxSrcWidth;
	rcSrc.bottom = rcSrc.top + cySrcHeight;

	m_pGlowImages[iRotation].DrawGlowImage(0, 0, Source, rcSrc.left,
			rcSrc.top, rcSrc.right, rcSrc.bottom, 4);
	}

void CObjectImageArray::GenerateScaledImages (int iRotation, int cxWidth, int cyHeight) const

//	GenerateScaledImages
//
//	Generate scaled images

	{
	ASSERT(iRotation >= 0 && iRotation < m_iRotationCount);

	//	Allocate the array of images (if not already allocated)

	if (m_pScaledImages == NULL)
		m_pScaledImages = new CG16bitImage [m_iRotationCount];

	//	If the image for this rotation has already been initialized, then
	//	we're done

	if (!m_pScaledImages[iRotation].IsEmpty())
		return;

	//	Otherwise we need to create the scaled image

	m_pScaledImages[iRotation].Create(cxWidth, cyHeight);

	//	Get the extent of the source image

	RECT rcSrc;
	rcSrc.left = ComputeSourceX(0);
	rcSrc.top = m_rcImage.top + (iRotation * RectHeight(m_rcImage));
	rcSrc.right = rcSrc.left + RectWidth(m_rcImage);
	rcSrc.bottom = rcSrc.top + RectHeight(m_rcImage);

	//	Scale the image
	m_pScaledImages[iRotation].BltRotoZ(0, 0, cxWidth, cyHeight,
			*m_pImage->GetImage(), rcSrc.left, rcSrc.top, rcSrc.right,
			rcSrc.bottom, 0, 0, false);
	}

RECT CObjectImageArray::GetImageRect (int iTick, int iRotation, int *retxCenter, int *retyCenter) const

//	GetImageRect
//
//	Returns the rect of the image

	{
	RECT rcRect;

	rcRect.left = ComputeSourceX(iTick);
	rcRect.right = rcRect.left + RectWidth(m_rcImage);
	rcRect.top = m_rcImage.top + (iRotation * RectHeight(m_rcImage));
	rcRect.bottom = rcRect.top + RectHeight(m_rcImage);

	if (retxCenter)
		*retxCenter = rcRect.left + ((RectWidth(m_rcImage) / 2) - (m_pRotationOffset ? m_pRotationOffset[iRotation % m_iRotationCount].x : 0));

	if (retyCenter)
		*retyCenter = rcRect.top + ((RectHeight(m_rcImage) / 2) + (m_pRotationOffset ? m_pRotationOffset[iRotation % m_iRotationCount].y : 0));

	return rcRect;
	}

RECT CObjectImageArray::GetImageRectAtPoint (int x, int y) const

//	GetImageRectAtPoint
//
//	Returns the rect of the image centered at the given coordinates

	{
	RECT rcRect;
	int cxWidth = RectWidth(m_rcImage);
	int cyHeight = RectHeight(m_rcImage);

	rcRect.left = x - (cxWidth / 2);
	rcRect.right = rcRect.left + cxWidth;
	rcRect.top = y - (cyHeight / 2);
	rcRect.bottom = rcRect.top + cyHeight;

	return rcRect;
	}

bool CObjectImageArray::ImagesIntersect (int iTick, int iRotation, int x, int y, const CObjectImageArray &Image2, int iTick2, int iRotation2) const

//	ImagesIntersect
//
//	Returns TRUE if the given image intersects with this image

	{
	if (m_pImage == NULL || Image2.m_pImage == NULL)
		return false;

	//	Compute the rectangle of image1

	RECT rcRect;
	int cxWidth = RectWidth(m_rcImage);
	int cyHeight = RectHeight(m_rcImage);
	rcRect.left = ComputeSourceX(iTick);
	rcRect.top = m_rcImage.top + (iRotation * cyHeight);
	rcRect.right = rcRect.left + cxWidth;
	rcRect.bottom = rcRect.top + cyHeight;

	//	Compute the rectangle of image2

	RECT rcRect2;
	int cxWidth2 = RectWidth(Image2.m_rcImage);
	int cyHeight2 = RectHeight(Image2.m_rcImage);
	rcRect2.left = Image2.ComputeSourceX(iTick);
	rcRect2.top = Image2.m_rcImage.top + (iRotation2 * cyHeight2);
	rcRect2.right = rcRect2.left + cxWidth2;
	rcRect2.bottom = rcRect2.top + cyHeight2;
	
	//	Now figure out the position of image2 on the image1 coordinate space

	int xCenter = rcRect.left + (cxWidth / 2);
	int yCenter = rcRect.top + (cyHeight / 2);

	RECT rcImage2On1;
	rcImage2On1.left = xCenter + x - (cxWidth2 / 2);
	rcImage2On1.top = yCenter + y - (cyHeight2 / 2);

	if (m_pRotationOffset)
		{
		rcImage2On1.left -= m_pRotationOffset[iRotation % m_iRotationCount].x;
		rcImage2On1.top += m_pRotationOffset[iRotation % m_iRotationCount].y;
		}

	if (Image2.m_pRotationOffset)
		{
		rcImage2On1.left += Image2.m_pRotationOffset[iRotation2 % Image2.m_iRotationCount].x;
		rcImage2On1.top -= Image2.m_pRotationOffset[iRotation2 % Image2.m_iRotationCount].y;
		}

	rcImage2On1.right = rcImage2On1.left + cxWidth2;
	rcImage2On1.bottom = rcImage2On1.top + cyHeight2;

	//	Intersect the rectangles

	RECT rcRectInt;
	if (!::IntersectRect(&rcRectInt, &rcRect, &rcImage2On1))
		return false;

	//	Figure out the position of image1 on the image2 coordinate space

	int xOffset = rcRect.left - rcImage2On1.left;
	int yOffset = rcRect.top - rcImage2On1.top;

	RECT rcImage1On2;
	rcImage1On2.left = rcRect2.left + xOffset;
	rcImage1On2.top = rcRect2.top + yOffset;
	rcImage1On2.right = rcImage1On2.left + cxWidth;
	rcImage1On2.bottom = rcImage1On2.top + cyHeight;

	//	Intersect the rectangles

	RECT rcRectInt2;
	::IntersectRect(&rcRectInt2, &rcRect2, &rcImage1On2);
	ASSERT(RectWidth(rcRectInt) == RectWidth(rcRectInt2));
	ASSERT(RectHeight(rcRectInt) == RectHeight(rcRectInt2));

	//	Images

	CG16bitImage &Src1(*m_pImage->GetImage());
	CG16bitImage &Src2(*Image2.m_pImage->GetImage());

	return Src1.Intersect(rcRectInt.left, rcRectInt.top, rcRectInt.right, rcRectInt.bottom, Src2, rcRectInt2.left, rcRectInt2.top);
	}

ALERROR CObjectImageArray::Init (CG16bitImage *pBitmap, const RECT &rcImage, int iFrameCount, int iTicksPerFrame)

//	Init
//
//	Create from parameters

	{
	//	Initialize basic info

	m_dwBitmapUNID = 0;
	m_pImage = new CObjectImage(pBitmap);
	m_rcImage = rcImage;
	m_iFrameCount = iFrameCount;
	m_iRotationCount = STD_ROTATION_COUNT;
	m_iTicksPerFrame = iTicksPerFrame;
	m_iFlashTicks = 0;
	m_iRotationOffset = 0;
	m_pRotationOffset = NULL;
	m_iBlending = blendNormal;

	return NOERROR;
	}

ALERROR CObjectImageArray::Init (DWORD dwBitmapUNID, const RECT &rcImage, int iFrameCount, int iTicksPerFrame)

//	Init
//
//	Create from parameters

	{
	//	Initialize basic info

	m_dwBitmapUNID = dwBitmapUNID;
	m_pImage = g_pUniverse->FindLibraryImage(m_dwBitmapUNID);
	m_rcImage = rcImage;
	m_iFrameCount = iFrameCount;
	m_iRotationCount = STD_ROTATION_COUNT;
	m_iTicksPerFrame = iTicksPerFrame;
	m_iFlashTicks = 0;
	m_iRotationOffset = 0;
	m_pRotationOffset = NULL;
	m_iBlending = blendNormal;

	return NOERROR;
	}

ALERROR CObjectImageArray::InitFromXML (CXMLElement *pDesc)

//	InitFromXML

	{
	SDesignLoadCtx Ctx;

	return InitFromXML(Ctx, pDesc, true);
	}

ALERROR CObjectImageArray::InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, bool bResolveNow)

//	InitFromXML
//
//	Create from XML description

	{
	//	Initialize basic info

	m_rcImage.left = pDesc->GetAttributeInteger(CONSTLIT(g_ImageXAttrib));
	m_rcImage.top = pDesc->GetAttributeInteger(CONSTLIT(g_ImageYAttrib));
	m_rcImage.right = m_rcImage.left + pDesc->GetAttributeInteger(CONSTLIT(g_ImageWidthAttrib));
	m_rcImage.bottom = m_rcImage.top + pDesc->GetAttributeInteger(CONSTLIT(g_ImageHeightAttrib));
	m_iFrameCount = pDesc->GetAttributeInteger(CONSTLIT(g_ImageFrameCountAttrib));
	m_iRotationCount = pDesc->GetAttributeInteger(ROTATION_COUNT_ATTRIB);
	if (m_iRotationCount <= 0)
		m_iRotationCount = 1;
	m_iTicksPerFrame = pDesc->GetAttributeInteger(CONSTLIT(g_ImageTicksPerFrameAttrib));
	m_iFlashTicks = pDesc->GetAttributeInteger(FLASH_TICKS_ATTRIB);

	CString sBlending = pDesc->GetAttribute(BLENDING_ATTRIB);
	if (strEquals(sBlending, LIGHTEN_BLENDING))
		m_iBlending = blendLighten;
	else
		m_iBlending = blendNormal;

	//	Compute rotation offsets

	m_iRotationOffset = pDesc->GetAttributeInteger(ROTATE_OFFSET_ATTRIB);
	if (m_iRotationOffset)
		ComputeRotationOffsets();
	else
		{
		int xOffset = pDesc->GetAttributeInteger(X_OFFSET_ATTRIB);
		int yOffset = pDesc->GetAttributeInteger(Y_OFFSET_ATTRIB);
		if (xOffset != 0 || yOffset != 0)
			ComputeRotationOffsets(xOffset, yOffset);
		}

	//	Get the image from the universe

	m_dwBitmapUNID = LoadUNID(Ctx, pDesc->GetAttribute(CONSTLIT(g_ImageIDAttrib)));
	if (bResolveNow)
		m_pImage = g_pUniverse->FindLibraryImage(m_dwBitmapUNID);
	else
		m_pImage = NULL;

	return NOERROR;
	}

void CObjectImageArray::LoadImage (void)

//	LoadImage
//
//	Make sure that the image is loaded

	{
	if (m_pImage)
		m_pImage->GetImage();
	}

void CObjectImageArray::MarkImage (void)

//	MarkImage
//
//	Mark image so that the sweeper knows that it is in use

	{
	if (m_pImage)
		m_pImage->Mark();
	}

ALERROR CObjectImageArray::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	All design elements have been loaded

	{
	if (Ctx.bNoResources)
		return NOERROR;

	if (m_dwBitmapUNID)
		{
		m_pImage = g_pUniverse->FindLibraryImage(m_dwBitmapUNID);

		if (m_pImage == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Unknown image: %x"), m_dwBitmapUNID);
			return ERR_FAIL;
			}
		}

	return NOERROR;
	}

void CObjectImageArray::PaintImage (CG16bitImage &Dest, int x, int y, int iTick, int iRotation, bool srcAlpha) const

//	PaintImage
//
//	Paints the image on the destination

	{
	if (m_pImage)
		{
		CG16bitImage &Source(*m_pImage->GetImage());
		int xSrc = ComputeSourceX(iTick);
		int ySrc;

		if (m_pRotationOffset)
			{
			x += m_pRotationOffset[iRotation % m_iRotationCount].x;
			y -= m_pRotationOffset[iRotation % m_iRotationCount].y;
			}

		if (m_iBlending == blendLighten)
			{
			Dest.BltLighten(xSrc,
					m_rcImage.top + (iRotation * RectHeight(m_rcImage)),
					RectWidth(m_rcImage),
					RectHeight(m_rcImage),
					255,
					Source,
					x - (RectWidth(m_rcImage) / 2),
					y - (RectHeight(m_rcImage) / 2));
			}
		else
			{
			ySrc = m_rcImage.top + (iRotation * RectHeight(m_rcImage));
			Dest.Blt(x - (RectWidth(m_rcImage) / 2), y - (RectHeight(m_rcImage) / 2), Source, xSrc,
					ySrc, xSrc + RectWidth(m_rcImage), ySrc + RectHeight(m_rcImage), srcAlpha);
			}
		}
	}

void CObjectImageArray::PaintImageUL (CG16bitImage &Dest, int x, int y, int iTick, int iRotation, bool srcAlpha) const

//	PaintImageUL
//
//	Paints the image. x,y is the upper-left corner of the destination
//
//	Note: This should not use the rotation offsets

	{
	if (m_pImage)
		{
		CG16bitImage &Source(*m_pImage->GetImage());
		int xSrc = ComputeSourceX(iTick);
		int ySrc;

		if (m_iBlending == blendLighten)
			{
			Dest.BltLighten(xSrc,
					m_rcImage.top + (iRotation * RectHeight(m_rcImage)),
					RectWidth(m_rcImage),
					RectHeight(m_rcImage),
					255,
					Source,
					x,
					y);
			}
		else
			{
			ySrc = m_rcImage.top + (iRotation * RectHeight(m_rcImage));
			Dest.Blt(x, y, Source, xSrc,
					ySrc, xSrc + RectWidth(m_rcImage), ySrc + RectHeight(m_rcImage), srcAlpha);
			}
		}
	}

void CObjectImageArray::PaintImageWithGlow (CG16bitImage &Dest,
											int x,
											int y,
											int iTick,
											int iRotation,
											COLORREF rgbGlowColor) const

//	PaintImageWithGlow
//
//	Paints the image on the destination with a pulsating glow around
//	it of the specified color.
//
//	This effect does not work with blending modes.

	{
	//	Paint the image

	PaintImage(Dest, x, y, iTick, iRotation);

	if (m_pRotationOffset)
		{
		x += m_pRotationOffset[iRotation % m_iRotationCount].x;
		y -= m_pRotationOffset[iRotation % m_iRotationCount].y;
		}

	//	Make sure we have the glow image

	GenerateGlowImage(iRotation);

	//	Glow strength

	int iStrength = 64 + (4 * Absolute((iTick % 65) - 32));
	if (iStrength > 255)
		iStrength = 255;

	//	Paint the glow

	Dest.FillMask(0,
			0,
			RectWidth(m_rcImage) + 2 * GLOW_SIZE,
			RectHeight(m_rcImage) + 2 * GLOW_SIZE,
			m_pGlowImages[iRotation],
			CG16bitImage::PixelFromRGB(rgbGlowColor),
			x - (RectWidth(m_rcImage) / 2) - GLOW_SIZE,
			y - (RectHeight(m_rcImage) / 2) - GLOW_SIZE,
			(BYTE)iStrength);
	}

void CObjectImageArray::PaintScaledImage (CG16bitImage &Dest,
										  int x,
										  int y,
										  int iTick,
										  int iRotation,
										  int cxWidth,
										  int cyHeight) const

//	PaintScaledImage
//
//	Paint scaled image

	{
	//	Make sure we have the scaled image

	GenerateScaledImages(iRotation, cxWidth, cyHeight);

	//	Paint the image

	Dest.ColorTransBlt(0,
			0,
			cxWidth,
			cyHeight,
			255,
			m_pScaledImages[iRotation],
			x - (cxWidth / 2),
			y - (cyHeight / 2));
	}

void CObjectImageArray::PaintSilhoutte (CG16bitImage &Dest,
										int x,
										int y,
										int iTick,
										int iRotation,
										COLORREF wColor) const

//	PaintSilhouette
//
//	Paints a silhouette of the object

	{
	if (m_pImage)
		{
		CG16bitImage &Source(*m_pImage->GetImage());
		int xSrc = ComputeSourceX(iTick);

		if (m_pRotationOffset)
			{
			x += m_pRotationOffset[iRotation % m_iRotationCount].x;
			y -= m_pRotationOffset[iRotation % m_iRotationCount].y;
			}

		Dest.FillMask(xSrc,
				m_rcImage.top + (iRotation * RectHeight(m_rcImage)),
				RectWidth(m_rcImage),
				RectHeight(m_rcImage),
				Source,
				wColor,
				x - (RectWidth(m_rcImage) / 2),
				y - (RectHeight(m_rcImage) / 2));
		}
	}

bool CObjectImageArray::PointInImage (int x, int y, int iTick, int iRotation) const

//	PointInImage
//
//	Returns TRUE if the given point is inside the masked part of the image
//	x, y is relative to the center of the image (GDI directions)

	{
	if (m_pImage)
		{
		CG16bitImage &Source(*m_pImage->GetImage());

		//	Compute the position of the frame

		int cxWidth = RectWidth(m_rcImage);
		int cyHeight = RectHeight(m_rcImage);
		int xSrc = ComputeSourceX(iTick);
		int ySrc = m_rcImage.top + (iRotation * cyHeight);

		//	Adjust the point so that it is relative to the
		//	frame origin (upper left)

		x = xSrc + x + (cxWidth / 2);
		y = ySrc + y + (cyHeight / 2);

		//	Adjust for rotation

		if (m_pRotationOffset)
			{
			x -= m_pRotationOffset[iRotation % m_iRotationCount].x;
			y += m_pRotationOffset[iRotation % m_iRotationCount].y;
			}

		//	Check bounds

		if (x < xSrc || y < ySrc || x >= (xSrc + cxWidth) || y >= (ySrc + cyHeight))
			return false;

		//	Check to see if the point is inside or outside the mask

		return (CGImage::AlphaColor(Source.GetPixel(x, y)) != 0);
		}
	else
		return false;
	}

void CObjectImageArray::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Read the object from a stream

	{
	Ctx.pStream->Read((char *)&m_dwBitmapUNID, sizeof(DWORD));
	if (m_dwBitmapUNID)
		m_pImage = g_pUniverse->FindLibraryImage(m_dwBitmapUNID);
	else
		m_pImage = NULL;
	Ctx.pStream->Read((char *)&m_rcImage, sizeof(RECT));
	Ctx.pStream->Read((char *)&m_iFrameCount, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iTicksPerFrame, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iFlashTicks, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iBlending, sizeof(DWORD));

	if (Ctx.dwVersion >= 17)
		Ctx.pStream->Read((char *)&m_iRotationCount, sizeof(DWORD));
	else
		m_iRotationCount = STD_ROTATION_COUNT;

	DWORD dwLoad;
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad)
		{
		m_pRotationOffset = new OffsetStruct [dwLoad];
		Ctx.pStream->Read((char *)m_pRotationOffset, dwLoad * sizeof(OffsetStruct));
		}
	}

void CObjectImageArray::SetRotationCount (int iRotationCount)

//	SetRotationCount
//
//	Sets the rotation count

	{
	if (iRotationCount != m_iRotationCount)
		{
		m_iRotationCount = iRotationCount;

		ComputeRotationOffsets();

		if (m_pGlowImages)
			{
			delete [] m_pGlowImages;
			m_pGlowImages = NULL;
			}

		if (m_pScaledImages)
			{
			delete [] m_pScaledImages;
			m_pScaledImages = NULL;
			}
		}
	}

void CObjectImageArray::WriteToStream (IWriteStream *pStream) const

//	WriteToStream
//
//	Writes the object to a stream
//
//	DWORD		m_dwBitmapUNID
//	Rect		m_rcImage
//	DWORD		m_iFrameCount
//	DWORD		m_iTicksPerFrame
//	DWORD		m_iFlashTicks
//	DWORD		m_iBlending
//	DWORD		m_iRotationCount

//	DWORD		No of rotation offsets
//	DWORD		x
//	DWORD		y
//				...

	{
	pStream->Write((char *)&m_dwBitmapUNID, sizeof(DWORD));
	pStream->Write((char *)&m_rcImage, sizeof(m_rcImage));
	pStream->Write((char *)&m_iFrameCount, sizeof(DWORD));
	pStream->Write((char *)&m_iTicksPerFrame, sizeof(DWORD));
	pStream->Write((char *)&m_iFlashTicks, sizeof(DWORD));
	pStream->Write((char *)&m_iBlending, sizeof(DWORD));
	pStream->Write((char *)&m_iRotationCount, sizeof(DWORD));

	if (m_pRotationOffset)
		{
		DWORD dwSave = m_iRotationCount;
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		pStream->Write((char *)m_pRotationOffset, m_iRotationCount * sizeof(OffsetStruct));
		}
	else
		{
		DWORD dwSave = 0;
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		}
	}

