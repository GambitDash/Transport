//	CMissile.cpp
//
//	CMissile class

#include "PreComp.h"

#include "CG16bitRegion.h"

#define TRAIL_COUNT							4
#define MAX_TARGET_RANGE					(24.0 * LIGHT_SECOND)

static CObjectClass<CMissile>g_Class(OBJID_CMISSILE, NULL);

CMissile::CMissile (void) : CSpaceObject(&g_Class),
		m_pExhaust(NULL),
		m_iBonus(0),
		m_pPainter(NULL),
		m_pVaporTrailRegions(NULL),
		m_iSavedRotationsCount(0),
		m_pSavedRotations(NULL),
		m_pHit(NULL),
		m_iHitDir(-1)

//	CMissile constructor

	{
	}

CMissile::~CMissile (void)

//	CMissile destructor

	{
	if (m_pPainter)
		m_pPainter->Delete();

	if (m_pExhaust)
		delete m_pExhaust;

	if (m_pVaporTrailRegions)
		delete [] m_pVaporTrailRegions;

	if (m_pSavedRotations)
		delete [] m_pSavedRotations;
	}

int CMissile::ComputeVaporTrail (void)

//	ComputeVaporTrail
//
//	Initialize vapor trail regions

	{
	int i;

	//	We handle this differently for maneuverable, vs non-maneuverable missiles

	if (m_pDesc->m_iManeuverability)
		{
		if (m_iSavedRotationsCount == 0)
			return 0;

		//	Allocate array of regions

		if (m_pVaporTrailRegions)
			delete [] m_pVaporTrailRegions;

		m_pVaporTrailRegions = new CG16bitRegion [m_iSavedRotationsCount];

		//	Compute some constants

		Metric rHalfWidth = (Metric)m_pDesc->m_iVaporTrailWidth / 200.0;
		Metric rWidthInc = m_pDesc->m_iVaporTrailWidthInc / 100.0;
		Metric rLength = m_pDesc->GetRatedSpeed() * g_SecondsPerUpdate / g_KlicksPerPixel;

		//	Start at the beginning

		int iLeft = (m_pSavedRotations[0] + 90) % 360;
		int iRight = (m_pSavedRotations[0] + 270) % 360;

		CVector vCenter;
		CVector vLeft = PolarToVector(iLeft, rHalfWidth);
		CVector vRight = PolarToVector(iRight, rHalfWidth);

		SPoint Segment[4];
		Segment[0].x = (int)(vLeft.GetX() + 0.5);
		Segment[0].y = -(int)(vLeft.GetY() + 0.5);
		Segment[1].x = (int)(vRight.GetX() + 0.5);
		Segment[1].y = -(int)(vRight.GetY() + 0.5);

		//	Create all the regions

		for (i = 0; i < m_iSavedRotationsCount; i++)
			{
			if (i + 1 < m_iSavedRotationsCount)
				{
				int iAngle;
				int iRange = AngleRange(m_pSavedRotations[i], m_pSavedRotations[i + 1]);
				if (iRange <= 180)
					iAngle = (m_pSavedRotations[i] + (iRange / 2)) % 360;
				else
					iAngle = (m_pSavedRotations[i + 1] + ((360 - iRange) / 2)) % 360;

				iLeft = (iAngle + 90) % 360;
				iRight = (iAngle + 270) % 360;
				}
			else
				{
				iLeft = (m_pSavedRotations[i] + 90) % 360;
				iRight = (m_pSavedRotations[i] + 270) % 360;
				}

			//	Compute the slope the trail

			vCenter = vCenter + PolarToVector(m_pSavedRotations[i], rLength);
			rHalfWidth += rWidthInc;
			CVector vNewLeft = vCenter + PolarToVector(iLeft, rHalfWidth);
			CVector vNewRight = vCenter + PolarToVector(iRight, rHalfWidth);

			Segment[2].x = (int)(vNewRight.GetX() + 0.5);
			Segment[2].y = -(int)(vNewRight.GetY() + 0.5);
			Segment[3].x = (int)(vNewLeft.GetX() + 0.5);
			Segment[3].y = -(int)(vNewLeft.GetY() + 0.5);

			m_pVaporTrailRegions[i].CreateFromConvexPolygon(4, Segment);

			Segment[0] = Segment[3];
			Segment[1] = Segment[2];
			vLeft = vNewLeft;
			vRight = vNewRight;
			}

		return m_iSavedRotationsCount;
		}
	else
		{
		int iCount = m_pDesc->m_iVaporTrailLength;

		//	For non-maneuverable missiles, only compute the trail once

		if (m_pVaporTrailRegions)
			return Min(iCount, m_iTick);

		//	Allocate array of regions

		m_pVaporTrailRegions = new CG16bitRegion [iCount];

		int iDirection;
		if (m_pDesc->m_bDirectional)
			iDirection = (AlignToRotationAngle(m_iRotation) + 180) % 360;
		else
			iDirection = (m_iRotation + 180) % 360;

		int iLeft = (iDirection + 90) % 360;
		int iRight = (iDirection + 270) % 360;

		//	We start a few pixels away from the center line

		Metric rHalfWidth = (Metric)m_pDesc->m_iVaporTrailWidth / 200.0;
		CVector vLeft = PolarToVector(iLeft, rHalfWidth);
		CVector vRight = PolarToVector(iRight, rHalfWidth);

		//	Compute the slope the trail

		Metric rLength = m_pDesc->GetRatedSpeed() * g_SecondsPerUpdate / g_KlicksPerPixel;
		Metric rWidthInc = m_pDesc->m_iVaporTrailWidthInc / 100.0;
		CVector vLeftInc = PolarToVector(iDirection, rLength) + PolarToVector(iLeft, rWidthInc);
		CVector vRightInc = PolarToVector(iDirection, rLength) + PolarToVector(iRight, rWidthInc);

		//	Start at the beginning

		SPoint Segment[4];
		Segment[0].x = (int)(vLeft.GetX() + 0.5);
		Segment[0].y = -(int)(vLeft.GetY() + 0.5);
		Segment[1].x = (int)(vRight.GetX() + 0.5);
		Segment[1].y = -(int)(vRight.GetY() + 0.5);

		//	Create all the regions

		for (i = 0; i < iCount; i++)
			{
			CVector vNewLeft = vLeft + vLeftInc;
			CVector vNewRight = vRight + vRightInc;

			Segment[2].x = (int)(vNewRight.GetX() + 0.5);
			Segment[2].y = -(int)(vNewRight.GetY() + 0.5);
			Segment[3].x = (int)(vNewLeft.GetX() + 0.5);
			Segment[3].y = -(int)(vNewLeft.GetY() + 0.5);

			m_pVaporTrailRegions[i].CreateFromConvexPolygon(4, Segment);

			Segment[0] = Segment[3];
			Segment[1] = Segment[2];
			vLeft = vNewLeft;
			vRight = vNewRight;
			}

		return Min(iCount, m_iTick);
		}
	}

ALERROR CMissile::Create (CSystem *pSystem,
			CWeaponFireDesc *pDesc,
			int iBonus,
			DestructionTypes iCause,
			const CDamageSource &Source,
			const CVector &vPos,
			const CVector &vVel,
			int iRotation,
			CSpaceObject *pTarget,
			CMissile **retpMissile)

//	Create
//
//	Creates a missile

	{
	ALERROR error;
	CMissile *pMissile;

	pMissile = new CMissile;
	if (pMissile == NULL)
		return ERR_MEMORY;

	pMissile->SetPos(vPos);
	pMissile->SetVel(vVel);
	pMissile->CalculateMove();

	//	We can't save missiles without an UNID
	ASSERT(!pDesc->m_sUNID.IsBlank());

	//	Get notifications when other objects are destroyed
	pMissile->SetObjectDestructionHook();

	pMissile->m_pDesc = pDesc;
	pMissile->m_iBonus = iBonus;
	pMissile->m_iCause = iCause;
	pMissile->m_iHitPoints = pDesc->GetHitPoints();
	pMissile->m_iLifeLeft = pDesc->GetLifetime();
	pMissile->m_iTick = 0;
	pMissile->m_Source = Source;
	pMissile->m_pHit = NULL;
	pMissile->m_iRotation = iRotation;
	pMissile->m_pTarget = pTarget;
	pMissile->m_fDestroyed = false;
	pMissile->m_fReflection = false;
	pMissile->m_fDetonate = false;
	pMissile->m_dwSpareFlags = 0;

	//	Remember the sovereign of the source (in case the source is destroyed)

	if (pMissile->m_Source.GetObj())
		pMissile->m_pSovereign = pMissile->m_Source.GetObj()->GetSovereign();
	else
		pMissile->m_pSovereign = NULL;

	//	Create a painter instance

	if (pDesc->m_pEffect)
		{
		pMissile->m_pPainter = pDesc->m_pEffect->CreatePainter();
		RECT rcRect;
		pMissile->m_pPainter->GetRect(&rcRect);
		pMissile->SetBounds(rcRect);
		}

	//	Create exhaust trail, if necessary

	if (pDesc->m_iExhaustRate > 0)
		{
		int iCount = (pDesc->m_iExhaustLifetime / pDesc->m_iExhaustRate) + 1;
		pMissile->m_pExhaust = new TQueue<SExhaustParticle>(iCount);
		}
	else
		pMissile->m_pExhaust = NULL;

	//	Create vapor trail, if necessary

	if (pDesc->m_iVaporTrailWidth)
		pMissile->SetBounds(2048.0 * g_KlicksPerPixel);

	//	Add to system

	if ((error = pMissile->AddToSystem(pSystem)))
		{
		delete pMissile;
		return error;
		}

	//	Done

	if (retpMissile)
		*retpMissile = pMissile;

	return NOERROR;
	}

void CMissile::CreateFragments (const CVector &vPos)

//	CreateFragments
//
//	Create fragments

	{
	//	NOTE: Missile fragments don't inherit the velocity of the missile
	//	(otherwise, fragmentation weapons explode too late to do much damage)

	GetSystem()->CreateWeaponFragments(m_pDesc,
			m_iBonus,
			m_iCause,
			m_Source,
			m_pTarget,
			vPos,
			CVector(),
			this);
	}

void CMissile::CreateHitEffect (const CVector &vPos)

//	CreateHitEffect
//
//	Create hit effect

	{
	if (m_pDesc->m_pHitEffect)
		m_pDesc->m_pHitEffect->CreateEffect(GetSystem(),
				(m_iHitDir == -1 ? NULL : m_pHit),
				vPos,
				CVector());
	}

void CMissile::CreateReflection (const CVector &vPos, int iDirection)

//	CreateReflection
//
//	Creates a new object that is a clone of this one but moving
//	in the given direction.

	{
	CMissile *pReflection;

	Create(GetSystem(),
			m_pDesc,
			m_iBonus,
			m_iCause,
			m_Source.GetObj(),
			vPos,
			PolarToVector(iDirection, GetVel().Length()),
			iDirection,
			NULL,
			&pReflection);

	pReflection->m_fReflection = true;
	}

CSpaceObject::DamageResults CMissile::Damage (CSpaceObject *pCause, const CVector &vHitPos, int iDirection, const DamageDesc &Damage)

//	Damage
//
//	Object takes damage from the given source

	{
	CSpaceObject *pAttacker = pCause->GetDamageCause();

	//	Compute damage

	bool bDestroy = false;
	int iDamage = Damage.RollDamage();
	if (iDamage == 0)
		return damageNoDamage;

	//	If this is a momentum attack then we are pushed

	int iMomentum;
	if (iMomentum = Damage.GetMomentumDamage())
		{
		CVector vAccel = PolarToVector(iDirection, -10 * iMomentum * iMomentum);
		Accelerate(vAccel, g_MomentumConstant);
		ClipSpeed(GetMaxSpeed());
		}

	//	Create a hit effect

	CEffectCreator *pEffect = g_pUniverse->FindEffectType(g_HitEffectUNID);
	if (pEffect)
		pEffect->CreateEffect(GetSystem(),
				this,
				vHitPos,
				GetVel());

	//	Take damage

	if (iDamage < m_iHitPoints)
		{
		m_iHitPoints -= iDamage;
		return damageArmorHit;
		}

	//	We are destroyed

	m_iHitPoints = 0;
	if (m_pDesc->m_iVaporTrailLength)
		{
		m_fDestroyed = true;
		m_iLifeLeft = m_pDesc->m_iVaporTrailLength;
		}
	else
		Destroy(killedByDamage, pAttacker);

	//	A missile might be able to pass through after hitting us

	return damagePassthrough;
	}

CString CMissile::DebugCrashInfo (void)

//	DebugCrashInfo
//
//	Dump debug information

	{
	CString sResult;

	if (m_fDestroyed)
		sResult.Append(CONSTLIT("m_fDestroyed: true\r\n"));
	sResult.Append(strPatternSubst(CONSTLIT("m_pDesc: %s\r\n"), m_pDesc->m_sUNID.GetPointer()));
	sResult.Append(strPatternSubst(CONSTLIT("m_Source: %s\r\n"), CSpaceObject::DebugDescribe(m_Source.GetObj()).GetPointer()));
	sResult.Append(strPatternSubst(CONSTLIT("m_pHit: %s\r\n"), CSpaceObject::DebugDescribe(m_pHit).GetPointer()));
	sResult.Append(strPatternSubst(CONSTLIT("m_iHitDir: %d\r\n"), m_iHitDir));
	sResult.Append(strPatternSubst(CONSTLIT("m_pTarget: %s\r\n"), CSpaceObject::DebugDescribe(m_pTarget).GetASCIIZPointer()));
	sResult.Append(strPatternSubst(CONSTLIT("m_iDestiny: %d\r\n"), GetDestiny()));
	sResult.Append(strPatternSubst(CONSTLIT("m_iTick: %d\r\n"), m_iTick));
	sResult.Append(strPatternSubst(CONSTLIT("m_iLifeLeft: %d\r\n"), m_iLifeLeft));

	//	m_pExhaust

	try
		{
		if (m_pExhaust)
			{
			int iExhaustCount = m_pExhaust->GetCount();
			sResult.Append(strPatternSubst(CONSTLIT("m_pExhaust: %d\r\n"), iExhaustCount));
			}
		else
			sResult.Append(CONSTLIT("m_pExhaust: none\r\n"));
		}
	catch (...)
		{
		sResult.Append(strPatternSubst(CONSTLIT("m_pExhaust: %x [invalid]\r\n"), (DWORD)m_pExhaust));
		}

	return sResult;
	}

void CMissile::DetonateNow (CSpaceObject *pHit)

//	DetonateNow
//
//	Detonate the fragmentation missile

	{
	m_fDetonate = true;
	}

int CMissile::GetStealth (void)

//	GetStealth
//
//	Returns the stealth of the missile

	{
	return m_pDesc->m_iStealth;
	}

void CMissile::OnMove (const CVector &vOldPos, Metric rSeconds)

//	OnMove
//
//	Move our points

	{
	if (m_fDestroyed)
		return;

	Metric rThreshold;
	if (m_pDesc->m_bProximityBlast && m_iTick >= m_pDesc->m_iProximityFailsafe)
		rThreshold = 128 * g_KlicksPerPixel;
	else
		rThreshold = 0.0;

	//	See if the beam hit anything after the move

	if (m_iTick > 1 || (!m_pDesc->m_bFragment && !m_fReflection))
		m_pHit = HitTest(vOldPos, rThreshold, m_pDesc->m_Damage, &m_vHitPos, &m_iHitDir);

	//	Make sure we are not too close to the source when we trigger
	//	a proximity blast.

	if (m_pHit && m_iHitDir == -1 && !m_Source.IsEmpty())
		{
		CVector vDist = m_vHitPos - m_Source.GetObj()->GetPos();
		Metric rDist2 = vDist.Length2();

		if (rDist2 < (rThreshold * rThreshold) / 4.0)
			m_pHit = NULL;
		}
	}

void CMissile::ObjectDestroyedHook (const SDestroyCtx &Ctx)

//	ObjectDestroyedHook
//
//	Handle it when another object is destroyed

	{
	//	If our source is destroyed, clear it

	m_Source.OnObjDestroyed(Ctx.pObj);

	if (Ctx.pObj == m_pHit)
		m_pHit = NULL;

	if (Ctx.pObj == m_pTarget)
		m_pTarget = NULL;
	}

void CMissile::OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	OnPaint
//
//	Paint the station

	{
	//	Paint with painter

	if (m_pPainter)
		{
		Ctx.iTick = m_iTick;
		Ctx.iVariant = (m_pDesc->m_bDirectional ? Angle2Direction(m_iRotation) : 0);
		Ctx.iRotation = m_iRotation;
		Ctx.iDestiny = GetDestiny();

		if (!m_fDestroyed && m_pHit == NULL)
			m_pPainter->Paint(Dest, x, y, Ctx);
		else
			m_pPainter->PaintFade(Dest, x, y, Ctx);
		}

	//	Paint image (deprecated method)

	if (m_pPainter == NULL && !m_fDestroyed && m_pHit == NULL)
		{
		m_pDesc->m_Image.PaintImage(Dest,
				x,
				y,
				m_iTick,
				(m_pDesc->m_bDirectional ? Angle2Direction(m_iRotation) : 0));

		//	Paint exhaust trail

		if (m_pExhaust)
			{
			int iCount = m_pExhaust->GetCount();

			for (int i = 0; i < iCount; i++)
				{
				int xParticle, yParticle;

				Ctx.XForm.Transform(m_pExhaust->GetAt(i).vPos, &xParticle, &yParticle);
				m_pDesc->m_ExhaustImage.PaintImage(Dest, 
						xParticle, 
						yParticle, 
						(iCount - i - 1) * m_pDesc->m_iExhaustRate, 
						0);
				}
			}
		}

	//	Paint vapor trail

	if (m_pDesc->m_iVaporTrailLength)
		{
		int iCount = ComputeVaporTrail();
		int iFadeStep = (128 / m_pDesc->m_iVaporTrailLength);
		int iOpacity = (!m_fDestroyed ? 128 : (iFadeStep * m_iLifeLeft));
		int iStart = (!m_fDestroyed ? 0 : 1 + (m_pDesc->m_iVaporTrailLength - m_iLifeLeft));

		for (int i = iStart; i < iCount; i++)
			{
			m_pVaporTrailRegions[i].FillTrans(Dest, x, y, m_pDesc->m_wVaporTrailColor, iOpacity);
			iOpacity -= iFadeStep;
			if (iOpacity <= 0)
				break;
			}
		}
	}

void CMissile::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read object data from a stream
//
//	CString		CWeaponFireDesc UNID
//	DWORD		m_iBonus
//	DWORD		m_iCause
//	DWORD		m_iHitPoints
//	DWORD		m_iLifeLeft
//	DWORD		m_Source (CSpaceObject ref)
//	DWORD		m_pSovereign (CSovereign ref)
//	DWORD		m_pHit (CSpaceObject ref)
//	Vector		m_vHitPos
//	DWORD		m_iHitDir
//	DWORD		m_iRotation
//	DWORD		m_pTarget (CSpaceObject ref)
//	DWORD		m_iTick
//
//	IEffectPainter	m_pPainter
//
//	DWORD		Number of exhaust particles
//	Vector		exhaust: vPos
//	Vector		exhaust: vVel
//
//	-- version > 0 -----------------------
//	DWORD		flags
//	DWORD		Number of saved rotations
//	DWORD		rotation[]

	{
	DWORD dwLoad;

#ifdef DEBUG_LOAD
	::OutputDebugString("CMissile::OnReadFromStream\n");
#endif
	//	Load descriptor

	CString sDescUNID;
	sDescUNID.ReadFromStream(Ctx.pStream);
	m_pDesc = g_pUniverse->FindWeaponFireDesc(sDescUNID);

	//	Load other stuff

	Ctx.pStream->Read((char *)&m_iBonus, sizeof(m_iBonus));
	if (Ctx.dwVersion >= 18)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_iCause = (DestructionTypes)dwLoad;
		}
	else
		m_iCause = killedByDamage;

	if (Ctx.dwVersion >= 28)
		Ctx.pStream->Read((char *)&m_iHitPoints, sizeof(DWORD));
	else
		m_iHitPoints = 0;

	Ctx.pStream->Read((char *)&m_iLifeLeft, sizeof(DWORD));
	m_Source.ReadFromStream(Ctx);
	Ctx.pSystem->ReadSovereignRefFromStream(Ctx, &m_pSovereign);
	Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_pHit);
	Ctx.pStream->Read((char *)&m_vHitPos, sizeof(CVector));
	Ctx.pStream->Read((char *)&m_iHitDir, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iRotation, sizeof(DWORD));
	Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_pTarget);
	Ctx.pStream->Read((char *)&m_iTick, sizeof(DWORD));

	//	Load painter

	if (m_pDesc->m_pEffect)
		m_pPainter = m_pDesc->m_pEffect->CreatePainterFromStreamAndCreator(Ctx);

	//	Load exhaust

	if (m_pDesc->m_iExhaustRate > 0)
		{
		int iCount = (m_pDesc->m_iExhaustLifetime / m_pDesc->m_iExhaustRate) + 1;
		m_pExhaust = new TQueue<SExhaustParticle>(iCount);
		}

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	for (int i = 0; i < (int)dwLoad; i++)
		{
		CVector vPos;
		CVector vVel;
		Ctx.pStream->Read((char *)&vPos, sizeof(CVector));
		Ctx.pStream->Read((char *)&vVel, sizeof(CVector));

		if (m_pExhaust && i < m_pExhaust->GetMaxCount())
			{
			SExhaustParticle &Particle = m_pExhaust->GetAt(m_pExhaust->Queue());
			Particle.vPos = vPos;
			Particle.vVel = vVel;
			}
		}

	//	Load saved rotations

	if (Ctx.dwVersion >= 1)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_fDestroyed =		((dwLoad & 0x00000001) ? true : false);
		m_fReflection =		((dwLoad & 0x00000002) ? true : false);
		m_fDetonate =		((dwLoad & 0x00000004) ? true : false);

		Ctx.pStream->Read((char *)&m_iSavedRotationsCount, sizeof(DWORD));
		if (m_iSavedRotationsCount > 0)
			{
			m_pSavedRotations = new int [m_pDesc->m_iVaporTrailLength];
			Ctx.pStream->Read((char *)m_pSavedRotations, sizeof(DWORD) * m_iSavedRotationsCount);
			}
		else
			m_iSavedRotationsCount = 0;
		}
	}

void CMissile::OnUpdate (Metric rSecondsPerTick)

//	OnUpdate
//
//	Update the beam

	{
	//	If we're already destroyed, then just update the timer until the
	//	vapor trail fades out

	if (m_fDestroyed)
		{
		//	Update the painter

		if (m_pPainter)
			{
			m_pPainter->OnUpdate();

			RECT rcRect;
			m_pPainter->GetRect(&rcRect);
			SetBounds(rcRect);
			}

		//	Done?

		if (--m_iLifeLeft <= 0)
			{
			Destroy(removedFromSystem, NULL);
			return;
			}
		}

	//	Otherwise, update

	else
		{
		int i;
		CSystem *pSystem = GetSystem();
		int iTick = m_iTick + GetDestiny();
		bool bDestroy = false;

		//	Accelerate, if necessary

		if (m_pDesc->m_iAccelerationFactor > 0 
				&& (iTick % 10 ) == 0)
			{
			if (m_pDesc->m_iAccelerationFactor < 100
					|| GetVel().Length() < m_pDesc->m_rMaxMissileSpeed)
				SetVel(GetVel() * (Metric)(m_pDesc->m_iAccelerationFactor / 100.0));
			}

		//	If we can choose new targets, see if we need one now

		if (m_pDesc->CanAutoTarget() && m_pTarget == NULL)
			m_pTarget = GetNearestEnemy(MAX_TARGET_RANGE, false);

		//	If this is a tracking missile, change direction to face the target

		if ((m_pDesc->m_iManeuverability > 0)
				&& m_pTarget 
				&& ((iTick % m_pDesc->m_iManeuverability) == 0))
			{
			//	Get the position and velocity of the target

			CVector vTarget = m_pTarget->GetPos() - GetPos();
			CVector vTargetVel = m_pTarget->GetVel() - GetVel();

			//	Figure out which direction to move in

			Metric rCurrentSpeed = GetVel().Length();
			Metric rTimeToIntercept = CalcInterceptTime(vTarget, vTargetVel, rCurrentSpeed);
			if (rTimeToIntercept > 0.0)
				{
				CVector vInterceptPoint = vTarget + vTargetVel * rTimeToIntercept;
				int iFireAngle = VectorToPolar(vInterceptPoint, NULL);

				//	If we are directional, then we are constrained to specific angles

				if (m_pDesc->m_bDirectional)
					{
					if (!AreAnglesAligned(iFireAngle, m_iRotation, g_RotationAngle / 2))
						{
						int iTurn = (iFireAngle + 360 - m_iRotation) % 360;

						if (iTurn >= 180)
							m_iRotation = (m_iRotation + 360 - g_RotationAngle) % 360;
						else
							m_iRotation = (m_iRotation + g_RotationAngle) % 360;

						}
					}
				else
					{
					if (!AreAnglesAligned(iFireAngle, m_iRotation, 1))
						{
						int iTurn = (iFireAngle + 360 - m_iRotation) % 360;

						if (iTurn >= 180)
							{
							int iTurnAngle = Min((360 - iTurn), g_RotationAngle);
							m_iRotation = (m_iRotation + 360 - iTurnAngle) % 360;
							}
						else
							{
							int iTurnAngle = Min(iTurn, g_RotationAngle);
							m_iRotation = (m_iRotation + iTurnAngle) % 360;
							}

						SetVel(PolarToVector(m_iRotation, rCurrentSpeed));
						}
					}

				SetVel(PolarToVector(m_iRotation, rCurrentSpeed));
				}
			}

		//	Update exhaust

		if (m_pExhaust)
			{
			if (iTick % m_pDesc->m_iExhaustRate)
				{
				if (m_pExhaust->GetCount() == m_pExhaust->GetMaxCount())
					m_pExhaust->Dequeue();

				SExhaustParticle &New = m_pExhaust->GetAt(m_pExhaust->Queue());
				New.vPos = GetPos();
				New.vVel = GetVel();
				}

			for (int i = 0; i < m_pExhaust->GetCount(); i++)
				{
				SExhaustParticle &Particle = m_pExhaust->GetAt(i);
				Particle.vVel = m_pDesc->m_rExhaustDrag * Particle.vVel;
				Particle.vPos = Particle.vPos + Particle.vVel * g_SecondsPerUpdate;
				}
			}

		//	Update the painter

		if (m_pPainter)
			{
			m_pPainter->OnUpdate();

			RECT rcRect;
			m_pPainter->GetRect(&rcRect);
			SetBounds(rcRect);
			}

		//	If we have a vapor trail and need to save rotation, do it

		if (m_pDesc->m_iVaporTrailLength 
				&& m_pDesc->m_iManeuverability)
			{
			//	Compute the current rotation

			int iDirection;
			if (m_pDesc->m_bDirectional)
				iDirection = (AlignToRotationAngle(m_iRotation) + 180) % 360;
			else
				iDirection = (m_iRotation + 180) % 360;

			//	Add the current rotation to the list of saved rotations

			if (m_pSavedRotations == NULL)
				{
				m_pSavedRotations = new int [m_pDesc->m_iVaporTrailLength];
				m_iSavedRotationsCount = 0;
				}

			int iStart = Min(m_iSavedRotationsCount, m_pDesc->m_iVaporTrailLength - 1);
			for (i = iStart; i > 0; i--)
				m_pSavedRotations[i] = m_pSavedRotations[i - 1];

			m_pSavedRotations[0] = iDirection;
			if (m_iSavedRotationsCount < m_pDesc->m_iVaporTrailLength)
				m_iSavedRotationsCount++;
			}

		//	See if the missile hit anything

		if (m_fDetonate && m_pDesc->HasFragments())
			{
			CreateFragments(GetPos());
			CreateHitEffect(GetPos());
			bDestroy = true;
			}
		else if (m_pHit)
			{
			//	If we have fragments, then explode now

			if (m_iHitDir == -1
					&& m_pDesc->HasFragments()
					&& m_iTick >= m_pDesc->m_iProximityFailsafe)
				{
				CreateFragments(m_vHitPos);
				CreateHitEffect(m_vHitPos);
				bDestroy = true;
				}

			//	Otherwise, if this was a direct hit, then we do damage

			else if (m_iHitDir != -1)
				{
				DamageResults result;
				DamageDesc Damage = m_pDesc->m_Damage;
				Damage.AddBonus(m_iBonus);
				Damage.SetCause(m_iCause);
				if (IsAutomatedWeapon())
					Damage.SetAutomatedWeapon();

				result = m_pHit->Damage(this,
						m_vHitPos,
						(m_iHitDir + 360 + mathRandom(0, 30) - 15) % 360,
						Damage);

				//	If we hit another missile (or some small object) there is a chance
				//	that we continue

				if (result == damagePassthrough)
					{
					m_iHitPoints = m_iHitPoints / 2;
					bDestroy = (m_iHitPoints == 0);
					}

				//	Set the missile to destroy itself after a hit

				else if (m_pDesc->m_iPassthrough == 0
						|| result == damageNoDamage 
						|| result == damageAbsorbedByShields
						|| mathRandom(1, 100) > m_pDesc->m_iPassthrough)
					bDestroy = true;

				CreateHitEffect(m_vHitPos);
				}
			}

		//	See if the missile has faded out

		if (bDestroy || --m_iLifeLeft <= 0)
			{
			//	If this is a fragmentation weapon, then we explode at the end of life

			if (!bDestroy && m_pDesc->HasFragments())
				{
				CreateFragments(GetPos());
				CreateHitEffect(GetPos());
				}

			//	If we've got a vapor trail effect, then keep the missile object alive
			//	but mark it destroyed

			int iFadeLife;
			if (m_pDesc->m_iVaporTrailLength)
				{
				m_fDestroyed = true;
				m_iLifeLeft = m_pDesc->m_iVaporTrailLength;
				}

			//	If we've got an effect that needs time to fade out, then keep
			//	the missile object alive

			else if (m_pPainter && (iFadeLife = m_pPainter->GetFadeLifetime()))
				{
				m_pPainter->OnBeginFade();

				m_fDestroyed = true;
				m_iLifeLeft = iFadeLife;
				}

			//	Otherwise, destroy the missile

			else
				{
				Destroy(removedFromSystem, NULL);
				return;
				}
			}
		}

	m_iTick++;
	}

void CMissile::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write the object's data to stream
//
//	CString		CWeaponFireDesc UNID
//	DWORD		m_iBonus
//	DWORD		m_iCause
//	DWORD		m_iHitPoints
//	DWORD		m_iLifeLeft
//	DWORD		m_Source (CSpaceObject ref)
//	DWORD		m_pSovereign (CSovereign ref)
//	DWORD		m_pHit (CSpaceObject ref)
//	Vector		m_vHitPos
//	DWORD		m_iHitDir
//	DWORD		m_iRotation
//	DWORD		m_pTarget (CSpaceObject ref)
//	DWORD		m_iTick
//
//	IEffectPainter	m_pPainter
//
//	DWORD		Number of exhaust particles
//	Vector		exhaust: vPos
//	Vector		exhaust: vVel
//
//	DWORD		flags
//	DWORD		Number of saved rotations
//	DWORD		rotation[]

	{
	DWORD dwSave;

	m_pDesc->m_sUNID.WriteToStream(pStream);
	pStream->Write((char *)&m_iBonus, sizeof(m_iBonus));
	dwSave = m_iCause;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	pStream->Write((char *)&m_iHitPoints, sizeof(DWORD));
	pStream->Write((char *)&m_iLifeLeft, sizeof(DWORD));
	m_Source.WriteToStream(GetSystem(), pStream);
	GetSystem()->WriteSovereignRefToStream(m_pSovereign, pStream);
	GetSystem()->WriteObjRefToStream(m_pHit, pStream);
	pStream->Write((char *)&m_vHitPos, sizeof(m_vHitPos));
	pStream->Write((char *)&m_iHitDir, sizeof(DWORD));
	pStream->Write((char *)&m_iRotation, sizeof(DWORD));
	GetSystem()->WriteObjRefToStream(m_pTarget, pStream);
	pStream->Write((char *)&m_iTick, sizeof(DWORD));

	//	Write effect

	if (m_pPainter)
		m_pPainter->WriteToStream(pStream);

	//	Exhaust

	if (m_pExhaust)
		{
		dwSave = m_pExhaust->GetCount();
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		for (int i = 0; i < (int)dwSave; i++)
			{
			SExhaustParticle &Particle = m_pExhaust->GetAt(i);
			pStream->Write((char *)&Particle.vPos, sizeof(Particle.vPos));
			pStream->Write((char *)&Particle.vVel, sizeof(Particle.vVel));
			}
		}
	else
		{
		dwSave = 0;
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		}

	//	Flags

	dwSave = 0;
	dwSave |= (m_fDestroyed ?	0x00000001 : 0);
	dwSave |= (m_fReflection ?	0x00000002 : 0);
	dwSave |= (m_fDetonate ?	0x00000004 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Saved rotations

	pStream->Write((char *)&m_iSavedRotationsCount, sizeof(DWORD));
	if (m_iSavedRotationsCount)
		pStream->Write((char *)m_pSavedRotations, sizeof(DWORD) * m_iSavedRotationsCount);
	}

void CMissile::PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans)

//	PaintLRS
//
//	Paints the object on an LRS

	{
	if (!m_fDestroyed)
		Dest.DrawDot(x, y, 
				CG16bitImage::RGBValue(255, 255, 0), 
				CG16bitImage::markerSmallRound);
	}

bool CMissile::PointInObject (CVector vPos)

//	PointInObject
//
//	Returns TRUE if the given point is inside the object

	{
	if (m_fDestroyed)
		return false;

	//	Figure out the coordinates of vPos relative to the center of the
	//	object, in pixels.

	CVector vOffset = vPos - GetPos();
	int x = (int)((vOffset.GetX() / g_KlicksPerPixel) + 0.5);
	int y = -(int)((vOffset.GetY() / g_KlicksPerPixel) + 0.5);

	if (m_pPainter)
		return m_pPainter->PointInImage(x, y, m_iTick, (m_pDesc->m_bDirectional ? Angle2Direction(m_iRotation) : 0));
	else
		return m_pDesc->m_Image.PointInImage(x, y, m_iTick, (m_pDesc->m_bDirectional ? Angle2Direction(m_iRotation) : 0));
	}
