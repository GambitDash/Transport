//	SFXFlare.cpp
//
//	Shockwave SFX

#include "PreComp.h"

#include "CG16bitRegion.h"

#define STYLE_ATTRIB							CONSTLIT("style")
#define RADIUS_ATTRIB							CONSTLIT("radius")
#define PRIMARY_COLOR_ATTRIB					(CONSTLIT("primaryColor"))
#define LIFETIME_ATTRIB							CONSTLIT("lifetime")

#define STYLE_FADING_BLAST						CONSTLIT("fadingBlast")

const int MAIN_SPIKE_COUNT =					3;
const int MAIN_SPIKE_WIDTH_RATIO =				32;
const DWORD MAIN_SPIKE_OPACITY =				128;

class CFlarePainter : public IEffectPainter
	{
	public:
		CFlarePainter (CFlareEffectCreator *pCreator);

		//	IEffectPainter virtuals
		virtual CEffectCreator *GetCreator (void) { return m_pCreator; }
		virtual void GetRect (RECT *retRect) const;
		virtual void OnUpdate (void) { m_iTick++; }
		virtual void Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx);

	protected:
		virtual void OnReadFromStream (SLoadCtx &Ctx);
		virtual void OnWriteToStream (IWriteStream *pStream);

	private:
		CFlareEffectCreator *m_pCreator;
		int m_iTick;
	};

//	CFlareEffectCreator object

void CFlareEffectCreator::CreateFlareSpike (int iAngle, int iLength, int iWidth, SPoint *Poly)

//	CreateFlareSpike
//
//	Creates a thin diamond spike centered at the origin (i.e., spikes out of each side of the origin)

	{
	CVector vPos;

	//	Start with one tip

	vPos = PolarToVector(iAngle, iLength);
	Poly[0].x = (int)vPos.GetX();
	Poly[0].y = -(int)vPos.GetY();

	//	Clockwise to the right short vertext

	vPos = PolarToVector(iAngle + 270, iWidth);
	Poly[1].x = (int)vPos.GetX();
	Poly[1].y = -(int)vPos.GetY();

	//	Now the opposite tip

	vPos = PolarToVector(iAngle + 180, iLength);
	Poly[2].x = (int)vPos.GetX();
	Poly[2].y = -(int)vPos.GetY();

	//	And finally the other short vertex

	vPos = PolarToVector(iAngle + 90, iWidth);
	Poly[3].x = (int)vPos.GetX();
	Poly[3].y = -(int)vPos.GetY();
	}

IEffectPainter *CFlareEffectCreator::CreatePainter (void)

//	CreatePainter
//
//	Creates a new painter

	{
	return new CFlarePainter(this);
	}

ALERROR CFlareEffectCreator::OnEffectCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnEffectCreateFromXML
//
//	Initializes from XML

	{
	CString sStyle = pDesc->GetAttribute(STYLE_ATTRIB);
	if (strEquals(sStyle, STYLE_FADING_BLAST))
		m_iStyle = styleFadingBlast;
	else
		m_iStyle = styleFadingBlast;

	m_iRadius = pDesc->GetAttributeInteger(RADIUS_ATTRIB);
	m_iLifetime = pDesc->GetAttributeInteger(LIFETIME_ATTRIB);
	m_wPrimaryColor = ::LoadRGBColor(pDesc->GetAttribute(PRIMARY_COLOR_ATTRIB));

	return NOERROR;
	}

//	CFlarePainter object

CFlarePainter::CFlarePainter (CFlareEffectCreator *pCreator) : m_pCreator(pCreator),
		m_iTick(0)

//	CFlarePainter constructor

	{
	}

void CFlarePainter::GetRect (RECT *retRect) const

//	GetRect
//
//	Returns the RECT of the effect centered on 0,0

	{
	int iRadius = m_pCreator->GetRadius();

	retRect->left = -iRadius;
	retRect->top = -iRadius;
	retRect->right = iRadius + 1;
	retRect->bottom = iRadius + 1;
	}

void CFlarePainter::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Load from stream

	{
	Ctx.pStream->Read((char *)&m_iTick, sizeof(DWORD));
	}

void CFlarePainter::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write to stream

	{
	pStream->Write((char *)&m_iTick, sizeof(DWORD));
	}

void CFlarePainter::Paint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	Paint
//
//	Paint

	{
	int i;
	COLORREF wPrimaryColor = m_pCreator->GetPrimaryColor();

	switch (m_pCreator->GetStyle())
		{
		case CFlareEffectCreator::styleFadingBlast:
			{
			//	Radius shrinks proportionally each tick

			int iRadius;
			if (m_iTick < m_pCreator->GetLifetime())
				iRadius = (int)(m_pCreator->GetRadius() * ((Metric)(m_pCreator->GetLifetime() - m_iTick) / (Metric)m_pCreator->GetLifetime()));
			else
				iRadius = 0;

			//	Paint

			if (iRadius)
				{
				//	Paint each of the spikes first

				int iAngle = 360 / MAIN_SPIKE_COUNT;
				for (i = 0; i < MAIN_SPIKE_COUNT; i++)
					{
					SPoint Spike[4];
					m_pCreator->CreateFlareSpike(i * iAngle, iRadius, iRadius / MAIN_SPIKE_WIDTH_RATIO, Spike);

					CG16bitRegion Region;
					Region.CreateFromConvexPolygon(4, Spike);
					Region.FillTrans(Dest, x, y, wPrimaryColor, 128);
					}

				//	Paint central glow

#if 0
				DrawAlphaGradientCircle(Dest,
						x,
						y,
						iRadius / 8,
						wPrimaryColor);
#endif

				//	Paint the extended glow

				DrawAlphaGradientCircle(Dest,
						x,
						y,
						iRadius,
						wPrimaryColor);
				}

			break;
			}
		}
	}
