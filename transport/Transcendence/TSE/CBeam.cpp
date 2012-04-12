//	CBeam.cpp
//
//	CBeam class

#include "PreComp.h"

#include "CG16bitRegion.h"

#define LIGHTNING_POINT_COUNT					16		//	Must be a power of 2

struct SBeamDrawCtx
	{
	int xFrom;
	int yFrom;
	int xTo;
	int yTo;

	int iIntensity;
	COLORREF wBackgroundColor;
	COLORREF wPrimaryColor;
	COLORREF wSecondaryColor;
	};

static CObjectClass<CBeam>g_Class(OBJID_CBEAM, NULL);

SPoint g_BlasterShape[8] = 
	{
		{    0,    0	},
		{   30,    6	},
		{   50,   20	},
		{   35,   50	},
		{    0,  100	},
		{  -35,   50	},
		{  -50,   20	},
		{  -30,    6	},
	};

void CreateBlasterShape (int iAngle, int iLength, int iWidth, SPoint *Poly);
void DrawStraightLightning (CG16bitImage &Dest,
							int x1,
							int y1,
							int x2,
							int y2,
							int iWidth,
							COLORREF wColor);
void DrawHeavyBlasterBeam (CG16bitImage &Dest, const SBeamDrawCtx &Ctx);
void DrawLaserBeam (CG16bitImage &Dest, const SBeamDrawCtx &Ctx);
void DrawLightningBeam (CG16bitImage &Dest, const SBeamDrawCtx &Ctx);
void DrawStarBlasterBeam (CG16bitImage &Dest, const SBeamDrawCtx &Ctx);

CBeam::CBeam (void) : CSpaceObject(&g_Class),
//		m_xPaintFrom(0),
//		m_yPaintFrom(0),
		m_iBonus(0),
		m_pHit(NULL),
		m_dwSpareFlags(0)

//	CBeam constructor

	{
	}

void CBeam::ComputeOffsets (void)

//	ComputeOffsets
//
//	Computes offsets

	{
	Metric rLength = LIGHT_SPEED * g_SecondsPerUpdate / g_KlicksPerPixel;
	CVector vFrom = PolarToVector(m_iRotation, -rLength);

	m_xFromOffset = (int)(vFrom.GetX() + 0.5);
	m_yFromOffset = -(int)(vFrom.GetY() + 0.5);
	m_xToOffset = 0;
	m_yToOffset = 0;
	}

ALERROR CBeam::Create (CSystem *pSystem,
					   CWeaponFireDesc *pDesc,
					   int iBonus,
					   DestructionTypes iCause,
					   const CDamageSource &Source,
					   const CVector &vPos,
					   const CVector &vVel,
					   int iRotation,
					   CBeam **retpBeam)

//	Create
//
//	Creates a new beam object

	{
	ALERROR error;
	CBeam *pBeam;

	pBeam = new CBeam;
	if (pBeam == NULL)
		return ERR_MEMORY;

	pBeam->SetPos(vPos);
	pBeam->SetVel(vVel);
	pBeam->CalculateMove();

	//	Don't bother telling others when we are destroyed (Note that
	//	if we do this then we also need to set the CannotBeHit flag;
	//	otherwise we will crash when a beam hits us. This is because
	//	m_pHit is setup in Move and the object can go away between then
	//	and our Update event.)
	pBeam->DisableObjectDestructionNotify();
	pBeam->SetCannotBeHit();

	//	Get notifications when other objects are destroyed
	pBeam->SetObjectDestructionHook();

	pBeam->m_pDesc = pDesc;
	pBeam->m_iBonus = iBonus;
	pBeam->m_iCause = iCause;
	pBeam->m_iRotation = iRotation;
	pBeam->m_vPaintTo = vPos;
	pBeam->m_iTick = 0;
	pBeam->m_iLifeLeft = pDesc->GetLifetime();
	pBeam->m_Source = Source;
	pBeam->m_fReflection = false;

	//	Remember the sovereign of the source (in case the source is destroyed)

	if (Source.GetObj())
		pBeam->m_pSovereign = Source.GetObj()->GetSovereign();
	else
		pBeam->m_pSovereign = NULL;

	//	Set the bounds so that we know if part of the beam is visible in the
	//	viewport.

	pBeam->ComputeOffsets();
	pBeam->SetBounds(g_SecondsPerUpdate * LIGHT_SPEED);

	//	Add to system

	if ((error = pBeam->AddToSystem(pSystem)))
		{
		delete pBeam;
		return error;
		}

	//	Done

	if (retpBeam)
		*retpBeam = pBeam;

	return NOERROR;
	}

void CBeam::CreateReflection (const CVector &vPos, int iDirection)

//	CreateReflection
//
//	Creates a new object that is a clone of this one but moving
//	in the given direction.

	{
	CBeam *pReflection;

	Create(GetSystem(),
			m_pDesc,
			m_iBonus,
			m_iCause,
			m_Source.GetObj(),
			vPos,
			PolarToVector(iDirection, LIGHT_SPEED),
			iDirection,
			&pReflection);

	pReflection->m_fReflection = true;
	}

CString CBeam::GetName (DWORD *retdwFlags)

//	GetName
//
//	This is only used if the source has been destroyed

	{
	if (retdwFlags)
		*retdwFlags = 0;

	switch (m_pDesc->m_Damage.GetDamageType())
		{
		case damageLaser:
			return CONSTLIT("laser blast");

		case damageParticle:
			return CONSTLIT("particle beam");

		case damageIonRadiation:
			return CONSTLIT("ion beam");

		case damagePositron:
			return CONSTLIT("positron beam");

		case damageGravitonBeam:
			return CONSTLIT("graviton beam");

		default:
			return CONSTLIT("beam"); 
		}
	}

void CBeam::OnMove (const CVector &vOldPos, Metric rSeconds)

//	OnMove
//
//	Move our points

	{
	//	See if the beam hit anything after the move

	if (!m_fReflection || m_iTick > 1)
		m_pHit = HitTest(vOldPos, 0.0, m_pDesc->m_Damage, &m_vPaintTo, &m_iHitDir);

	if (m_pHit == NULL)
		m_vPaintTo = GetPos();
	}

void CBeam::ObjectDestroyedHook (const SDestroyCtx &Ctx)

//	ObjectDestroyedHook
//
//	Handle it when another object is destroyed

	{
	//	If our source is destroyed, clear it

	m_Source.OnObjDestroyed(Ctx.pObj);

	if (Ctx.pObj == m_pHit)
		m_pHit = NULL;
	}

void CBeam::OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	OnPaint
//
//	Paint the beam

	{
	//	Setup structure

	SBeamDrawCtx BeamCtx;
	BeamCtx.xFrom = x + m_xFromOffset;
	BeamCtx.yFrom = y + m_yFromOffset;
	if (m_pHit)
		Ctx.XForm.Transform(m_vPaintTo, &BeamCtx.xTo, &BeamCtx.yTo);
	else
		{
		BeamCtx.xTo = x + m_xToOffset;
		BeamCtx.yTo = y + m_yToOffset;
		}
	BeamCtx.iIntensity = m_pDesc->m_iIntensity;
	BeamCtx.wBackgroundColor = Ctx.wSpaceColor;
	BeamCtx.wPrimaryColor = m_pDesc->m_wPrimaryColor;
	BeamCtx.wSecondaryColor = m_pDesc->m_wSecondaryColor;

	//	Paint a line

	switch (m_pDesc->m_iBeamType)
		{
		case beamLaser:
			DrawLaserBeam(Dest, BeamCtx);
			break;

		case beamLightning:
			DrawLightningBeam(Dest, BeamCtx);
			break;

		case beamHeavyBlaster:
			DrawHeavyBlasterBeam(Dest, BeamCtx);
			break;

		case beamStarBlaster:
			DrawStarBlasterBeam(Dest, BeamCtx);
			break;

		//	OLD TYPES

		case beamBlaster:
			{
			COLORREF wStart, wEnd;

			wStart = CG16bitImage::BlendPixel(Ctx.wSpaceColor, CG16bitImage::RGBValue(255,0,0), 155);
			wEnd = Ctx.wSpaceColor;
			Dest.DrawBiColorLine(BeamCtx.xFrom, BeamCtx.yFrom,
					BeamCtx.xTo, BeamCtx.yTo,
					5,
					wEnd,
					wStart);

			wStart = CG16bitImage::RGBValue(255,0,0);
			wEnd = Ctx.wSpaceColor;
			Dest.DrawBiColorLine(BeamCtx.xFrom, BeamCtx.yFrom,
					BeamCtx.xTo, BeamCtx.yTo,
					3,
					wEnd,
					wStart);

			wStart = CG16bitImage::RGBValue(255,255,0);
			wEnd = CG16bitImage::BlendPixel(Ctx.wSpaceColor, CG16bitImage::RGBValue(255,0,0), 155);
			Dest.DrawBiColorLine(BeamCtx.xFrom, BeamCtx.yFrom,
					BeamCtx.xTo, BeamCtx.yTo,
					1,
					wEnd,
					wStart);

			break;
			}

		case beamGreenParticle:
			{
			const int iSteps = 20;
			double xStep = (double)(BeamCtx.xTo - BeamCtx.xFrom) / (double)iSteps;
			double yStep = (double)(BeamCtx.yTo - BeamCtx.yFrom) / (double)iSteps;
			double xPaint = BeamCtx.xFrom;
			double yPaint = BeamCtx.yFrom;
			int i;

			for (i = 0; i < iSteps; i++)
				{
				int x = ((int)xPaint);
				int y = ((int)yPaint);
				COLORREF wColor = CG16bitImage::RGBValue(95,241,42);

				if (mathRandom(1,3) == 1)
					Dest.DrawDot(x, y, wColor, CG16bitImage::markerSmallRound);
				else
					Dest.DrawDot(x + mathRandom(-1,1),
							y + mathRandom(-1,1),
							wColor,
							CG16bitImage::markerPixel);

				xPaint += xStep;
				yPaint += yStep;
				}
			break;
			}

		case beamBlueParticle:
			{
			const int iSteps = 20;
			double xStep = (double)(BeamCtx.xTo - BeamCtx.xFrom) / (double)iSteps;
			double yStep = (double)(BeamCtx.yTo - BeamCtx.yFrom) / (double)iSteps;
			double xPaint = BeamCtx.xFrom;
			double yPaint = BeamCtx.yFrom;
			int i;

			for (i = 0; i < iSteps; i++)
				{
				int x = ((int)xPaint);
				int y = ((int)yPaint);
				COLORREF wCenter = CG16bitImage::RGBValue(255, 255, 255);
				COLORREF wColor = CG16bitImage::RGBValue(64, 83, 255);

				if (mathRandom(1,3) == 1)
					Dest.DrawDot(x, y, wCenter, CG16bitImage::markerSmallRound);
				else
					Dest.DrawDot(x + mathRandom(-1,1),
							y + mathRandom(-1,1),
							wColor,
							CG16bitImage::markerPixel);

				xPaint += xStep;
				yPaint += yStep;
				}
			break;
			}

		case beamGreenLightning:
			{
			DrawStraightLightning(Dest,
					BeamCtx.xFrom,
					BeamCtx.yFrom,
					BeamCtx.xTo,
					BeamCtx.yTo,
					(m_iTick > 9 ? 9 : m_iTick),
					CG16bitImage::RGBValue(0, 255, 0));
			break;
			}
		default:
			break;
		}

	//	Draw the head of the beam if we have an image

	if (!m_pDesc->m_Image.IsEmpty())
		{
		m_pDesc->m_Image.PaintImage(Dest,
				BeamCtx.xTo,
				BeamCtx.yTo,
				m_iTick,
				0);
		}

	//	Remember where we painted

	//m_xPaintFrom = x;
	//m_yPaintFrom = y;
	}

void CBeam::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read object data from a stream
//
//	CString		CWeaponFireDesc UNID
//	DWORD		m_iBonus
//	DWORD		m_iCause
//	DWORD		m_iRotation
//	Vector		m_vPaintTo
//	DWORD		m_iTick
//	DWORD		m_iLifeLeft
//	DWORD		m_Source (CSpaceObject Ref)
//	DWORD		m_pSovereign (UNID)
//	DWORD		m_pHit (CSpaceObject Ref)
//	DWORD		m_iHitDir

	{
	DWORD dwLoad;

#ifdef DEBUG_LOAD
	::OutputDebugString("CBeam::OnReadFromStream\n");
#endif
	//	Load descriptor

	CString sDescUNID;
	sDescUNID.ReadFromStream(Ctx.pStream);
	m_pDesc = g_pUniverse->FindWeaponFireDesc(sDescUNID);

	Ctx.pStream->Read((char *)&m_iBonus, sizeof(DWORD));
	if (Ctx.dwVersion >= 18)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_iCause = (DestructionTypes)dwLoad;
		}
	else
		m_iCause = killedByDamage;

	Ctx.pStream->Read((char *)&m_iRotation, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_vPaintTo, sizeof(CVector));
	Ctx.pStream->Read((char *)&m_iTick, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iLifeLeft, sizeof(m_iLifeLeft));
	m_Source.ReadFromStream(Ctx);
	Ctx.pSystem->ReadSovereignRefFromStream(Ctx, &m_pSovereign);
	Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_pHit);
	Ctx.pStream->Read((char *)&m_iHitDir, sizeof(DWORD));

	ComputeOffsets();
	}

void CBeam::OnUpdate (Metric rSecondsPerTick)

//	OnUpdate
//
//	Update the beam

	{
	bool bDestroy = false;

	m_iTick++;

	//	See if the beam hit anything

	if (m_pHit)
		{
		//	Tell the object hit that it has been damaged

		DamageResults result;
		DamageDesc Damage = m_pDesc->m_Damage;
		Damage.AddBonus(m_iBonus);
		Damage.SetCause(m_iCause);
		if (IsAutomatedWeapon())
			Damage.SetAutomatedWeapon();

		result = m_pHit->Damage(this,
				m_vPaintTo,
				(m_iHitDir + 360 + mathRandom(0, 30) - 15) % 360,
				Damage);

		//	Set the beam to destroy itself after a hit

		if (m_pDesc->m_iPassthrough == 0
				|| result == damageNoDamage 
				|| result == damageAbsorbedByShields
				|| mathRandom(1, 100) > m_pDesc->m_iPassthrough)
			bDestroy = true;
		}

	//	See if the beam has faded out

	if (bDestroy || --m_iLifeLeft <= 0)
		Destroy(removedFromSystem, NULL);
	}

void CBeam::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write the object's data to stream
//
//	CString		CWeaponFireDesc UNID
//	DWORD		m_iBonus
//	DWORD		m_iCause
//	DWORD		m_iRotation
//	Vector		m_vPaintTo
//	DWORD		m_iTick
//	DWORD		m_iLifeLeft
//	DWORD		m_Source (CSpaceObject Ref)
//	DWORD		m_pSovereign (UNID)
//	DWORD		m_pHit (CSpaceObject Ref)
//	DWORD		m_iHitDir

	{
	DWORD dwSave;
	m_pDesc->m_sUNID.WriteToStream(pStream);
	pStream->Write((char *)&m_iBonus, sizeof(DWORD));
	dwSave = m_iCause;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	pStream->Write((char *)&m_iRotation, sizeof(DWORD));
	pStream->Write((char *)&m_vPaintTo, sizeof(m_vPaintTo));
	pStream->Write((char *)&m_iTick, sizeof(DWORD));
	pStream->Write((char *)&m_iLifeLeft, sizeof(m_iLifeLeft));
	m_Source.WriteToStream(GetSystem(), pStream);
	GetSystem()->WriteSovereignRefToStream(m_pSovereign, pStream);
	GetSystem()->WriteObjRefToStream(m_pHit, pStream);
	pStream->Write((char *)&m_iHitDir, sizeof(DWORD));
	}

//	Beam Drawing Routines

struct SDrawStraightLightningCtx : public CG16bitImage::SDrawLineCtx
	{
	COLORREF wColor;

	double rHalfHeightWidth;				//	Half the width or height of the line
	int iHalfCount;
	int iCount;								//	Width or height of line in pixels

	double rSlope;							//	Slope of line
	double rDistTopLeft;
	double rDistBottomRight;
	double rDistTopLeftInc;
	double rDistTopLeftDec;
	double rDistBottomRightInc;
	double rDistBottomRightDec;
	double rWidth;
	double rWidthInc;
	};

void DrawStraightLightningXProc (SDrawStraightLightningCtx *pCtx)
	{
	//int y = pCtx->y + mathRandom(-1, 1);
	int y = pCtx->y;

	double rExtra = (pCtx->rDistTopLeft > 1.0 ? 1.0 : pCtx->rDistTopLeft);
	//double rRange = pCtx->rWidth * ((double)pCtx->iHalfCount + rExtra + ((double)((pCtx->x % 5) - 3)) / 2.0);
	double rRange = pCtx->rWidth * ((double)pCtx->iHalfCount + rExtra);
	double rStep = (rRange > 1.0 ? (255.0 / rRange) : 255.0 - rExtra);
	double rTrans = 255.0 - rStep;

	int i;
	for (i = 1; i < (pCtx->iHalfCount + 2); i++)
		{
		pCtx->pDest->DrawPixelTrans(pCtx->x, y - i, pCtx->wColor, (BYTE)rTrans);
		rTrans -= rStep;
		if (rTrans <= 0.0)
			break;
		}

	rExtra = (pCtx->rDistBottomRight > 1.0 ? 1.0 : pCtx->rDistBottomRight);
	//rRange = pCtx->rWidth * ((double)pCtx->iHalfCount + rExtra + ((double)((pCtx->x % 5) - 3)) / 2.0);
	rRange = pCtx->rWidth * ((double)pCtx->iHalfCount + rExtra);
	rStep = (rRange > 1.0 ? (255.0 / rRange) : 255.0 - rExtra);
	rTrans = 255.0 - rStep;
	for (i = 1; i < (pCtx->iHalfCount + 2); i++)
		{
		pCtx->pDest->DrawPixelTrans(pCtx->x, y + i, pCtx->wColor, (BYTE)rTrans);
		rTrans -= rStep;
		if (rTrans <= 0.0)
			break;
		}

	int iRnd = mathRandom(-1, 1);
	pCtx->pDest->DrawPixel(pCtx->x, y + iRnd, pCtx->wColor);
	if (iRnd != 0)
		pCtx->pDest->DrawPixelTrans(pCtx->x, y, pCtx->wColor, 128);

	//	Next

	if (pCtx->d >= 0)
		{
		pCtx->rDistTopLeft += pCtx->rDistTopLeftInc;
		pCtx->rDistBottomRight -= pCtx->rDistBottomRightDec;
		}

	pCtx->rDistTopLeft -= pCtx->rDistTopLeftDec;
	pCtx->rDistBottomRight += pCtx->rDistBottomRightInc;
	pCtx->rWidth += pCtx->rWidthInc;
	}

void DrawStraightLightningYProc (SDrawStraightLightningCtx *pCtx)
	{
	//int x = pCtx->x + mathRandom(-1, 1);
	int x = pCtx->x;

	double rExtra = (pCtx->rDistTopLeft > 1.0 ? 1.0 : pCtx->rDistTopLeft);
	//double rRange = pCtx->rWidth * ((double)pCtx->iHalfCount + rExtra + ((double)((pCtx->x % 5) - 3)) / 2.0);
	double rRange = pCtx->rWidth * ((double)pCtx->iHalfCount + rExtra);
	double rStep = (rRange > 1.0 ? (255.0 / rRange) : 255.0 - rExtra);
	double rTrans = 255.0 - rStep;

	int i;
	for (i = 1; i < (pCtx->iHalfCount + 2); i++)
		{
		pCtx->pDest->DrawPixelTrans(x - i, pCtx->y, pCtx->wColor, (BYTE)rTrans);
		rTrans -= rStep;
		if (rTrans <= 0.0)
			break;
		}

	rExtra = (pCtx->rDistBottomRight > 1.0 ? 1.0 : pCtx->rDistBottomRight);
	//rRange = pCtx->rWidth * ((double)pCtx->iHalfCount + rExtra + ((double)((pCtx->x % 5) - 3)) / 2.0);
	rRange = pCtx->rWidth * ((double)pCtx->iHalfCount + rExtra);
	rStep = (rRange > 1.0 ? (255.0 / rRange) : 255.0 - rExtra);
	rTrans = 255.0 - rStep;
	for (i = 1; i < (pCtx->iHalfCount + 2); i++)
		{
		pCtx->pDest->DrawPixelTrans(x + i, pCtx->y, pCtx->wColor, (BYTE)rTrans);
		rTrans -= rStep;
		if (rTrans <= 0.0)
			break;
		}

	int iRnd = mathRandom(-1, 1);
	pCtx->pDest->DrawPixel(x + iRnd, pCtx->y, pCtx->wColor);
	if (iRnd != 0)
		pCtx->pDest->DrawPixelTrans(x, pCtx->y, pCtx->wColor, 128);

	//	Next point

	if (pCtx->d >= 0)
		{
		pCtx->rDistTopLeft += pCtx->rDistTopLeftInc;
		pCtx->rDistBottomRight -= pCtx->rDistBottomRightDec;
		}

	pCtx->rDistTopLeft -= pCtx->rDistTopLeftDec;
	pCtx->rDistBottomRight += pCtx->rDistBottomRightInc;
	pCtx->rWidth += pCtx->rWidthInc;
	}

void DrawStraightLightning (CG16bitImage &Dest,
							int x1,
							int y1,
							int x2,
							int y2,
							int iWidth,
							COLORREF wColor)

	{
	SDrawStraightLightningCtx Ctx;

	Dest.DrawLineProcInit(x1, y1, x2, y2, &Ctx);
	Ctx.wColor = wColor;

	if (Ctx.IsXDominant())
		{
		Ctx.rHalfHeightWidth = (double)iWidth * sqrt((double)(Ctx.dx*Ctx.dx + Ctx.dy*Ctx.dy)) / (double)(2 * abs(Ctx.dx));
		Ctx.iHalfCount = (int)(((2 * Ctx.rHalfHeightWidth) - 1) / 2);
		Ctx.iCount = 2 * Ctx.iHalfCount + 1;

		Ctx.rSlope = (double)Ctx.dy / (double)Ctx.dx;
		Ctx.rDistTopLeft = Ctx.rHalfHeightWidth - (double)Ctx.iHalfCount - 0.25;
		Ctx.rDistBottomRight = Ctx.rDistTopLeft;

		Ctx.rDistTopLeftInc = Ctx.sy;
		Ctx.rDistTopLeftDec = Ctx.rSlope * Ctx.sx;
		Ctx.rDistBottomRightInc = Ctx.rSlope * Ctx.sx;
		Ctx.rDistBottomRightDec = Ctx.sy;

		Ctx.rWidth = 1.0;
		//Ctx.rWidthInc = 1.0 / Absolute(Ctx.dx);
		Ctx.rWidthInc = 0.0;

		Dest.DrawLineProc(&Ctx, (CG16bitImage::DRAWLINEPROC)DrawStraightLightningXProc);
		}
	else
		{
		Ctx.rHalfHeightWidth = (double)iWidth * sqrt((double)(Ctx.dx*Ctx.dx + Ctx.dy*Ctx.dy)) / (double)(2 * abs(Ctx.dy));
		Ctx.iHalfCount = (int)(((2 * Ctx.rHalfHeightWidth) - 1) / 2);
		Ctx.iCount = 2 * Ctx.iHalfCount + 1;

		Ctx.rSlope = (double)Ctx.dx / (double)Ctx.dy;
		Ctx.rDistTopLeft = Ctx.rHalfHeightWidth - (double)Ctx.iHalfCount - 0.25;
		Ctx.rDistBottomRight = Ctx.rDistTopLeft;

		Ctx.rDistTopLeftInc = Ctx.sx;
		Ctx.rDistTopLeftDec = Ctx.rSlope * Ctx.sy;
		Ctx.rDistBottomRightInc = Ctx.rSlope * Ctx.sy;
		Ctx.rDistBottomRightDec = Ctx.sx;

		Ctx.rWidth = 1.0;
		//Ctx.rWidthInc = 1.0 / Absolute(Ctx.dy);
		Ctx.rWidthInc = 0.0;

		Dest.DrawLineProc(&Ctx, (CG16bitImage::DRAWLINEPROC)DrawStraightLightningYProc);
		}
	}

//	Beam Drawing Routines -----------------------------------------------------

void CreateBlasterShape (int iAngle, int iLength, int iWidth, SPoint *Poly)

//	CreateBlasterShape
//
//	Creates a blaster-shaped polygon

	{
	//	Define a transformation for this shape

	XForm Trans(xformScale, ((Metric)iWidth)/100.0, ((Metric)iLength)/100.0);
	Trans = Trans * XForm(xformRotate, iAngle + 270);

	//	Generate the points

	for (int i = 0; i < 8; i++)
		{
		Metric x, y;
		Trans.Transform(g_BlasterShape[i].x, g_BlasterShape[i].y, &x, &y);
		Poly[i].x = (int)x;
		Poly[i].y = (int)y;
		}
	}

void DrawHeavyBlasterBeam (CG16bitImage &Dest, const SBeamDrawCtx &Ctx)

//	DrawHeavyBlasterBeam
//
//	Draws a heavy blaster beam

	{
	//	Convert to an angle relative to xTo, yTo

	CVector vVec(Ctx.xFrom - Ctx.xTo, Ctx.yFrom - Ctx.yTo);
	Metric rRadius;
	int iAngle = VectorToPolar(vVec, &rRadius);
	int iRadius = (int)rRadius;

	//	Can't deal with 0 sized lines

	if (iRadius == 0)
		return;

	CG16bitRegion Region;
	SPoint Poly[8];

	//	Compute some metrics

	int iLengthUnit = iRadius * (10 + Ctx.iIntensity) / 40;
	int iWidthUnit = Max(1, iRadius * Ctx.iIntensity / 60);

	//	Paint the outer-most glow

	COLORREF wColor = CG16bitImage::BlendPixel(Ctx.wBackgroundColor, Ctx.wSecondaryColor, 100);
	CreateBlasterShape(iAngle, 4 * iLengthUnit, 3 * iWidthUnit / 2, Poly);
	Region.CreateFromConvexPolygon(8, Poly);
	Region.Fill(Dest, Ctx.xTo, Ctx.yTo, wColor);

	//	Paint the inner transition

	wColor = CG16bitImage::BlendPixel(Ctx.wSecondaryColor, Ctx.wPrimaryColor, 128);
	wColor = CG16bitImage::BlendPixel(Ctx.wBackgroundColor, wColor, 200);
	CreateBlasterShape(iAngle, 3 * iLengthUnit, iWidthUnit, Poly);
	Region.CreateFromConvexPolygon(8, Poly);
	Region.Fill(Dest, Ctx.xTo, Ctx.yTo, wColor);

	//	Paint the inner core

	CreateBlasterShape(iAngle, iLengthUnit, iWidthUnit - 1, Poly);
	Region.CreateFromConvexPolygon(8, Poly);
	Region.Fill(Dest, Ctx.xTo, Ctx.yTo, Ctx.wPrimaryColor);
	}

void DrawLaserBeam (CG16bitImage &Dest, const SBeamDrawCtx &Ctx)

//	DrawLaserBeam
//
//	Draws a simple laser beam

	{
	COLORREF wGlow = CG16bitImage::BlendPixel(Ctx.wBackgroundColor, Ctx.wSecondaryColor, 100);

	Dest.DrawLine(Ctx.xFrom, Ctx.yFrom,
			Ctx.xTo, Ctx.yTo,
			3,
			wGlow);

	Dest.DrawLine(Ctx.xFrom, Ctx.yFrom,
			Ctx.xTo, Ctx.yTo,
			1,
			Ctx.wPrimaryColor);
	}

void DrawLightningBeam (CG16bitImage &Dest, const SBeamDrawCtx &Ctx)

//	DrawLightningBeam
//
//	Draws a lighting beam

	{
	//	The central part of the beam is different depending on the
	//	intensity.

	if (Ctx.iIntensity < 4)
		{
		COLORREF wStart = CG16bitImage::BlendPixel(Ctx.wBackgroundColor, Ctx.wPrimaryColor, 128);
		Dest.DrawBiColorLine(Ctx.xFrom, Ctx.yFrom,
				Ctx.xTo, Ctx.yTo,
				3,
				Ctx.wBackgroundColor,
				wStart);

		COLORREF wEnd = CG16bitImage::BlendPixel(Ctx.wBackgroundColor, Ctx.wPrimaryColor, 155);
		Dest.DrawBiColorLine(Ctx.xFrom, Ctx.yFrom,
				Ctx.xTo, Ctx.yTo,
				1,
				wEnd,
				Ctx.wPrimaryColor);
		}
	else if (Ctx.iIntensity < 10)
		{
		COLORREF wStart = CG16bitImage::BlendPixel(Ctx.wBackgroundColor, Ctx.wSecondaryColor, 155);
		Dest.DrawBiColorLine(Ctx.xFrom, Ctx.yFrom,
				Ctx.xTo, Ctx.yTo,
				5,
				Ctx.wBackgroundColor,
				wStart);

		Dest.DrawBiColorLine(Ctx.xFrom, Ctx.yFrom,
				Ctx.xTo, Ctx.yTo,
				3,
				Ctx.wBackgroundColor,
				Ctx.wSecondaryColor);

		COLORREF wEnd = CG16bitImage::BlendPixel(Ctx.wBackgroundColor, Ctx.wPrimaryColor, 155);
		Dest.DrawBiColorLine(Ctx.xFrom, Ctx.yFrom,
				Ctx.xTo, Ctx.yTo,
				1,
				wEnd,
				Ctx.wPrimaryColor);
		}
	else
		{
		//	Convert to an angle relative to xTo, yTo

		CVector vVec(Ctx.xFrom - Ctx.xTo, Ctx.yFrom - Ctx.yTo);
		Metric rRadius;
		int iAngle = VectorToPolar(vVec, &rRadius);
		int iRadius = (int)rRadius;

		//	Can't deal with 0 sized lines

		if (iRadius == 0)
			return;

		CG16bitRegion Region;
		SPoint Poly[8];

		//	Paint the outer-most glow

		COLORREF wColor = CG16bitImage::BlendPixel(Ctx.wBackgroundColor, Ctx.wSecondaryColor, 100);
		CreateBlasterShape(iAngle, iRadius, iRadius / 6, Poly);
		Region.CreateFromConvexPolygon(8, Poly);
		Region.Fill(Dest, Ctx.xTo, Ctx.yTo, wColor);

		//	Paint the inner transition

		wColor = CG16bitImage::BlendPixel(Ctx.wSecondaryColor, Ctx.wPrimaryColor, 128);
		wColor = CG16bitImage::BlendPixel(Ctx.wBackgroundColor, wColor, 200);
		CreateBlasterShape(iAngle, iRadius * 2 / 3, iRadius / 7, Poly);
		Region.CreateFromConvexPolygon(8, Poly);
		Region.Fill(Dest, Ctx.xTo, Ctx.yTo, wColor);

		//	Paint the inner core

		CreateBlasterShape(iAngle, iRadius / 2, iRadius / 8, Poly);
		Region.CreateFromConvexPolygon(8, Poly);
		Region.Fill(Dest, Ctx.xTo, Ctx.yTo, Ctx.wPrimaryColor);
		}

	//	Compute the half-way point

	int xHalf = (Ctx.xFrom + Ctx.xTo) / 2;
	int yHalf = (Ctx.yFrom + Ctx.yTo) / 2;

	//	Draw lightning

	int iCount = Ctx.iIntensity + mathRandom(0, 2);
	for (int j = 0; j < iCount; j++)
		if (mathRandom(1, 2) == 1)
			DrawLightning(Dest, 
					xHalf, 
					yHalf, 
					Ctx.xTo, 
					Ctx.yTo, 
					Ctx.wPrimaryColor, 
					LIGHTNING_POINT_COUNT, 
					0.5);
		else
			DrawLightning(Dest, 
					Ctx.xFrom, 
					Ctx.yFrom, 
					Ctx.xTo, 
					Ctx.yTo, 
					Ctx.wSecondaryColor, 
					LIGHTNING_POINT_COUNT, 
					0.3);
	}

void DrawStarBlasterBeam (CG16bitImage &Dest, const SBeamDrawCtx &Ctx)

//	DrawStarBlasterBeam
//
//	Draws a star blaster beam

	{
	COLORREF wStart, wEnd;

	wStart = CG16bitImage::BlendPixel(Ctx.wBackgroundColor, Ctx.wPrimaryColor, 155);
	wEnd = Ctx.wBackgroundColor;
	Dest.DrawBiColorLine(Ctx.xFrom, Ctx.yFrom,
			Ctx.xTo, Ctx.yTo,
			3,
			wEnd,
			wStart);

	wStart = Ctx.wSecondaryColor;
	wEnd = CG16bitImage::BlendPixel(Ctx.wBackgroundColor, Ctx.wPrimaryColor, 155);
	Dest.DrawBiColorLine(Ctx.xFrom, Ctx.yFrom,
			Ctx.xTo, Ctx.yTo,
			1,
			wEnd,
			wStart);

	//	Draw starburst

	int iCount = mathRandom(4, 9);
	for (int i = 0; i < iCount; i++)
		{
		CVector vLine(Ctx.xTo, Ctx.yTo);
		vLine = vLine + PolarToVector(mathRandom(0,359), mathRandom(5, 15));
		Dest.DrawBiColorLine(Ctx.xTo, Ctx.yTo,
				(int)vLine.GetX(), (int)vLine.GetY(),
				1,
				wStart,
				Ctx.wBackgroundColor);
		}
	}

