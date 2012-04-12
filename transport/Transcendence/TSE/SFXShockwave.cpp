//	SFXShockwave.cpp
//
//	Shockwave SFX

#include "PreComp.h"


#define IMAGE_TAG								CONSTLIT("Image")

#define STYLE_ATTRIB							CONSTLIT("style")
#define SPEED_ATTRIB							CONSTLIT("speed")
#define LIFETIME_ATTRIB							CONSTLIT("lifetime")
#define FADE_START_ATTRIB						CONSTLIT("fadeStart")
#define PRIMARY_COLOR_ATTRIB					CONSTLIT("primaryColor")
#define SECONDARY_COLOR_ATTRIB					CONSTLIT("secondaryColor")
#define WIDTH_ATTRIB							CONSTLIT("width")
#define GLOW_SIZE_ATTRIB						CONSTLIT("glowSize")

#define STYLE_IMAGE								CONSTLIT("image")
#define STYLE_GLOW_RING							CONSTLIT("glowRing")

class CShockwavePainter : public IEffectPainter
	{
	public:
		CShockwavePainter (CShockwaveEffectCreator *pCreator);

		//	IEffectPainter virtuals
		virtual CEffectCreator *GetCreator (void) { return m_pCreator; }
		virtual Metric GetRadius (void) const { return g_KlicksPerPixel * m_iRadius; }
		virtual void GetRect (RECT *retRect) const;
		virtual void OnUpdate (void) { m_iRadius += m_pCreator->GetSpeed(); m_iTick++; }
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);

	protected:
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		CShockwaveEffectCreator *m_pCreator;
		int m_iRadius;
		int m_iTick;
	};

//	CShockwaveEffectCreator object

CShockwaveEffectCreator::CShockwaveEffectCreator (void) :
		m_wColorGradient(NULL),
		m_byOpacityGradient(NULL)

//	CShockwaveEffectCreator constructor

	{
	}

CShockwaveEffectCreator::~CShockwaveEffectCreator (void)

//	CShockwaveEffectCreator destructor

	{
	if (m_wColorGradient)
		delete [] m_wColorGradient;

	if (m_byOpacityGradient)
		delete [] m_byOpacityGradient;
	}

void CShockwaveEffectCreator::CreateGlowGradient (int iSolidWidth, int iGlowWidth, COLORREF wSolidColor, COLORREF wGlowColor)

//	CreateGlowGradient
//
//	Creates a gradient for a glowing ring

	{
	int i;

	ASSERT(m_wColorGradient == NULL);
	ASSERT(m_byOpacityGradient == NULL);
	ASSERT(iSolidWidth >= 0);
	ASSERT(iGlowWidth >= 0);
	ASSERT(iSolidWidth + iGlowWidth > 0);

	m_iGradientCount = iSolidWidth + 2 * iGlowWidth;
	m_wColorGradient = new COLORREF [m_iGradientCount];
	m_byOpacityGradient = new BYTE [m_iGradientCount];

	//	Add glow ramp

	for (i = 0; i < iGlowWidth; i++)
		{
		BYTE iFade = (256 * (i + 1) / (iGlowWidth + 1));

		m_byOpacityGradient[i] = iFade;
		m_byOpacityGradient[m_iGradientCount - (i + 1)] = iFade;

		COLORREF wColor = CG16bitImage::BlendPixel(wGlowColor, wSolidColor, iFade);
		m_wColorGradient[i] = wColor;
		m_wColorGradient[m_iGradientCount - (i + 1)] = wColor;
		}

	//	Add the solid color in the center

	for (i = 0; i < iSolidWidth; i++)
		{
		m_wColorGradient[iGlowWidth + i] = wSolidColor;
		m_byOpacityGradient[iGlowWidth + i] = 255;
		}
	}

IEffectPainter *CShockwaveEffectCreator::CreatePainter (void)

//	CreatePainter
//
//	Creates a new painter

	{
	return new CShockwavePainter(this);
	}

ALERROR CShockwaveEffectCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnEffectCreateFromXML
//
//	Initializes from XML

	{
	ALERROR error;

	CXMLElement *pImage = pDesc->GetContentElementByTag(IMAGE_TAG);
	if (pImage)
		if ((error = m_Image.InitFromXML(Ctx, pImage)))
			return error;

	//	Load style

	CString sAttrib = pDesc->GetAttribute(STYLE_ATTRIB);
	if (sAttrib.IsBlank() || strEquals(sAttrib, STYLE_IMAGE))
		m_iStyle = styleImage;
	else if (strEquals(sAttrib, STYLE_GLOW_RING))
		m_iStyle = styleGlowRing;
	else
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Invalid Shockwave style: %s"), sAttrib.GetASCIIZPointer());
		return ERR_FAIL;
		}

	//	Load other stuff

	m_iSpeed = Max(1, (int)(((pDesc->GetAttributeInteger(SPEED_ATTRIB) * LIGHT_SPEED / 100.0) * g_SecondsPerUpdate / g_KlicksPerPixel) + 0.5));
	m_iLifetime = pDesc->GetAttributeInteger(LIFETIME_ATTRIB);
	m_wPrimaryColor = ::LoadRGBColor(pDesc->GetAttribute(PRIMARY_COLOR_ATTRIB));
	m_wSecondaryColor = ::LoadRGBColor(pDesc->GetAttribute(SECONDARY_COLOR_ATTRIB));

	if (pDesc->FindAttribute(FADE_START_ATTRIB, &sAttrib))
		m_iFadeStart = strToInt(sAttrib, 100);
	else
		m_iFadeStart = 100;

	//	Load glow ring attributes

	if (m_iStyle == styleGlowRing)
		{
		m_iWidth = pDesc->GetAttributeInteger(WIDTH_ATTRIB);
		m_iGlowWidth = pDesc->GetAttributeInteger(GLOW_SIZE_ATTRIB);

		m_iWidth = Max(0, m_iWidth);
		m_iGlowWidth = Max(0, m_iGlowWidth);

		if (m_iWidth + m_iGlowWidth <= 0)
			{
			Ctx.sError = CONSTLIT("Shockwave width and glowSize must be positive integers");
			return ERR_FAIL;
			}

		CreateGlowGradient(m_iWidth, m_iGlowWidth, m_wPrimaryColor, m_wSecondaryColor);
		}

	return NOERROR;
	}

ALERROR CShockwaveEffectCreator::OnEffectBindDesign (SDesignLoadCtx &Ctx)

//	OnEffectBindDesign
//
//	Resolve loading

	{
	ALERROR error;

	if ((error = m_Image.OnDesignLoadComplete(Ctx)))
		return error;

	return NOERROR;
	}

//	CShockwavePainter object

CShockwavePainter::CShockwavePainter (CShockwaveEffectCreator *pCreator) : m_pCreator(pCreator),
		m_iRadius(1),
		m_iTick(0)

//	CShockwavePainter constructor

	{
	}

void CShockwavePainter::GetRect (RECT *retRect) const

//	GetRect
//
//	Returns the RECT of the effect centered on 0,0

	{
	retRect->left = -m_iRadius;
	retRect->top = -m_iRadius;
	retRect->right = m_iRadius + 1;
	retRect->bottom = m_iRadius + 1;
	}

void CShockwavePainter::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Load from stream

	{
	Ctx.pStream->Read((char *)&m_iRadius, sizeof(DWORD));
	if (Ctx.dwVersion >= 20)
		Ctx.pStream->Read((char *)&m_iTick, sizeof(DWORD));
	}

void CShockwavePainter::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write to stream

	{
	pStream->Write((char *)&m_iRadius, sizeof(DWORD));
	pStream->Write((char *)&m_iTick, sizeof(DWORD));
	}

void CShockwavePainter::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint

	{
	int iLifetime = m_pCreator->GetLifetime();
	int iStartDecay = m_pCreator->GetFadeStart() * iLifetime / 100;
	int iDecayRange = iLifetime - iStartDecay;

	BYTE byOpacity = 255;
	if (m_iTick > iLifetime)
		byOpacity = 0;
	else if (m_iTick > iStartDecay && iDecayRange > 0)
		byOpacity = byOpacity * (iDecayRange - (m_iTick - iStartDecay)) / iDecayRange;

	switch (m_pCreator->GetStyle())
		{
		case CShockwaveEffectCreator::styleImage:
			{
			CG16bitImage &Image = m_pCreator->GetImage().GetImage();
			RECT rcImage = m_pCreator->GetImage().GetImageRect();

			DrawBltCircle(Dest, 
					x, 
					y, 
					m_iRadius, 
					Image, 
					rcImage.left, 
					rcImage.top, 
					RectWidth(rcImage), 
					RectHeight(rcImage),
					byOpacity);
			break;
			}

		case CShockwaveEffectCreator::styleGlowRing:
			{
			DrawGlowRing(Dest,
					x,
					y,
					m_iRadius,
					m_pCreator->GetRingThickness(),
					m_pCreator->GetColorGradient(),
					m_pCreator->GetOpacityGradient());
			break;
			}

		default:
			ASSERT(false);
		}
	}
