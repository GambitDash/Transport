//	CParticleDamage.cpp
//
//	CParticleDamage class

#include "PreComp.h"

static CObjectClass<CParticleDamage>g_Class(OBJID_CPARTICLEDAMAGE, NULL);

CParticleDamage::CParticleDamage (void) : CSpaceObject(&g_Class),
		m_pPainter(NULL)

//	CParticleDamage constructor

	{
	}

CParticleDamage::~CParticleDamage (void)

//	CParticleDamage destructor

	{
	if (m_pPainter)
		m_pPainter->Delete();
	}

ALERROR CParticleDamage::Create (CSystem *pSystem,
							 CWeaponFireDesc *pDesc,
							 int iBonus,
							 DestructionTypes iCause,
							 const CDamageSource &Source,
							 const CVector &vPos,
							 const CVector &vVel,
							 int iDirection,
							 CParticleDamage **retpObj)

//	Create
//
//	Create the object

	{
	ALERROR error;

	//	Make sure we have a valid CWeaponFireDesc (otherwise we won't be
	//	able to save the object).
	ASSERT(!pDesc->m_sUNID.IsBlank());

	//	Create the area

	CParticleDamage *pParticles = new CParticleDamage;
	if (pParticles == NULL)
		return ERR_MEMORY;

	pParticles->SetPos(vPos);
	pParticles->SetVel(vVel);
	pParticles->m_vOldPos = pParticles->GetPos();

	//	Get notifications when other objects are destroyed
	pParticles->SetObjectDestructionHook();

	pParticles->m_pDesc = pDesc;
	pParticles->m_iBonus = iBonus;
	pParticles->m_iCause = iCause;
	pParticles->m_iEmitDirection = iDirection;
	pParticles->m_vEmitSourcePos = vPos;
	pParticles->m_vEmitSourceVel = (Source.GetObj() ? Source.GetObj()->GetVel() : CVector());
	pParticles->m_iEmitTime = Max(1, pDesc->GetParticleEmitTime());
	pParticles->m_iLifeLeft = pDesc->GetMaxLifetime() + pParticles->m_iEmitTime;
	pParticles->m_Source = Source;
	pParticles->m_iTick = 0;

	pParticles->m_iDamage = pDesc->m_Damage.RollDamage();

	//	Painter

	if (pDesc->m_pEffect)
		pParticles->m_pPainter = pDesc->m_pEffect->CreatePainter();

	//	Remember the sovereign of the source (in case the source is destroyed)

	if (Source.GetObj())
		pParticles->m_pSovereign = Source.GetObj()->GetSovereign();
	else
		pParticles->m_pSovereign = NULL;

	//	Compute the maximum number of particles that we might have

	int iMaxCount = pParticles->m_iEmitTime * pDesc->GetMaxParticleCount();
	pParticles->m_Particles.Init(iMaxCount);

	//	Create the initial particles

	int iInitCount = pDesc->GetParticleCount();
	pParticles->InitParticles(iInitCount, CVector(), iDirection);

	//	Figure out the number of particles that will cause full damage

	if (pParticles->m_iEmitTime > 1)
		pParticles->m_iParticleCount = pParticles->m_iEmitTime * pDesc->GetAveParticleCount();
	else
		pParticles->m_iParticleCount = iInitCount;

	pParticles->m_iParticleCount = Max(1, pParticles->m_iParticleCount);

	//	Add to system

	if (error = pParticles->AddToSystem(pSystem))
		{
		delete pParticles;
		return error;
		}

	//	Done

	if (retpObj)
		*retpObj = pParticles;

	return NOERROR;
	}

CString CParticleDamage::GetName (DWORD *retdwFlags)

//	GetName
//
//	Returns the name of the object

	{
	//	This name is used only if the source has been destroyed

	if (retdwFlags)
		*retdwFlags = 0;
	return CONSTLIT("enemy weapon");
	}

void CParticleDamage::InitParticles (int iCount, const CVector &vSource, int iDirection)

//	InitParticles
//
//	Initialize particles

	{
	int i;

	//	Generate the number of particles

	if (iCount > 0)
		{
		//	Calculate a few temporaries

		Metric rRadius = (6.0 * m_pDesc->GetRatedSpeed());

		int iSpreadAngle = m_pDesc->GetParticleSpreadAngle();
		if (iSpreadAngle > 0)
			iSpreadAngle = (iSpreadAngle / 2) + 1;
		bool bSpreadAngle = (iSpreadAngle > 0);

		CVector vTemp = PolarToVector(iSpreadAngle, m_pDesc->GetRatedSpeed());
		Metric rTangentV = (6.0 * vTemp.GetY());
		int iTangentAngle = (iDirection + 90) % 360;

		int iSpreadWidth = m_pDesc->GetParticleSpreadWidth();
		Metric rSpreadWidth = iSpreadWidth * g_KlicksPerPixel;
		bool bSpreadWidth = (iSpreadWidth > 0);

		//	Create the particles with appropriate velocity

		for (i = 0; i < iCount; i++)
			{
			Metric rPlace = ((mathRandom(0, 25) + mathRandom(0, 25) + mathRandom(0, 25) + mathRandom(0, 25)) - 50.0) / 100.0;
			Metric rTangentPlace = ((mathRandom(0, 25) + mathRandom(0, 25) + mathRandom(0, 25) + mathRandom(0, 25)) - 50.0) / 100.0;
	
			CVector vPlace = PolarToVector(iDirection, rRadius * rPlace);
			CVector vVel = (0.1 * vPlace)
					+ PolarToVector(iTangentAngle, rTangentV * rTangentPlace);

			//	Compute the spread width

			CVector vPos = vSource + vPlace;
			if (bSpreadWidth)
				vPos = vPos + PolarToVector(iTangentAngle, rSpreadWidth * rTangentPlace);

			//	Compute the travel rotation for these particles

			int iRotation = (bSpreadAngle ? VectorToPolar(GetVel() + vVel) : iDirection);

			//	Create the particle

			m_Particles.AddParticle(vPos, vVel, m_pDesc->GetLifetime(), iRotation);
			}
		}
	}

void CParticleDamage::OnMove (const CVector &vOldPos, Metric rSeconds)

//	OnMove
//
//	Handle moving

	{
	m_vOldPos = vOldPos;

	//	Update particle motion

	bool bAlive;
	m_Particles.UpdateMotionLinear(&bAlive);

	//	If no particles are left alive, then we destroy the object

	if (!bAlive)
		{
		Destroy(removedFromSystem, NULL);
		return;
		}

	//	Set the bounds (note, we make the bounds twice as large to deal
	//	with the fact that we're moving).

	RECT rcBounds = m_Particles.GetBounds();
	SetBounds(g_KlicksPerPixel * Max(RectWidth(rcBounds), RectHeight(rcBounds)));

	//	Update emit source position

	m_vEmitSourcePos = m_vEmitSourcePos + m_vEmitSourceVel;
	}

void CParticleDamage::ObjectDestroyedHook (const SDestroyCtx &Ctx)

//	ObjectDestroyedHook
//
//	Called when another object is destroyed

	{
	m_Source.OnObjDestroyed(Ctx.pObj);
	}

void CParticleDamage::OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	OnPaint
//
//	Paint

	{
	if (m_pPainter)
		{
		Ctx.iTick = m_iTick;

		//	If we can get a paint descriptor, use that because it is faster

		SParticlePaintDesc Desc;
		if (m_pPainter->GetParticlePaintDesc(&Desc))
			{
			Desc.iMaxLifetime = m_pDesc->GetMaxLifetime();
			m_Particles.Paint(Dest, x, y, Ctx, Desc);
			}

		//	Otherwise, we use the painter for each particle

		else
			m_Particles.Paint(Dest, x, y, Ctx, m_pPainter);
		}
	}

void CParticleDamage::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Restore from stream

	{
	DWORD dwLoad;

#ifdef DEBUG_LOAD
	::OutputDebugString("CParticleDamage::OnReadFromStream\n");
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

	Ctx.pStream->Read((char *)&m_iLifeLeft, sizeof(m_iLifeLeft));
	m_Source.ReadFromStream(Ctx);
	Ctx.pSystem->ReadSovereignRefFromStream(Ctx, &m_pSovereign);
	Ctx.pStream->Read((char *)&m_iTick, sizeof(m_iTick));
	Ctx.pStream->Read((char *)&m_iDamage, sizeof(m_iDamage));
	if (Ctx.dwVersion >= 3)
		Ctx.pStream->Read((char *)&m_vOldPos, sizeof(m_vOldPos));
	else
		m_vOldPos = GetPos();

	//	The newer version uses a different particle array

	if (Ctx.dwVersion >= 21)
		{
		Ctx.pStream->Read((char *)&m_vEmitSourcePos, sizeof(CVector));
		Ctx.pStream->Read((char *)&m_vEmitSourceVel, sizeof(CVector));
		Ctx.pStream->Read((char *)&m_iEmitDirection, sizeof(DWORD));
		Ctx.pStream->Read((char *)&m_iEmitTime, sizeof(DWORD));
		Ctx.pStream->Read((char *)&m_iParticleCount, sizeof(DWORD));

		//	Load painter

		if (m_pDesc->m_pEffect)
			m_pPainter = m_pDesc->m_pEffect->CreatePainterFromStreamAndCreator(Ctx);

		m_Particles.ReadFromStream(Ctx);
		}

	//	Read the previous version, but no need to convert

	else
		{
		DWORD dwCount;
		Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
		if (dwCount > 0)
			{
			char *pDummy = new char [5 * sizeof(DWORD) * dwCount];
			Ctx.pStream->Read(pDummy, 5 * sizeof(DWORD) * dwCount);
			delete pDummy;
			}

		m_iEmitTime = 0;
		m_iEmitDirection = -1;
		}
	}

void CParticleDamage::OnUpdate (Metric rSecondsPerTick)

//	OnUpdate
//
//	Update

	{
	int i, j;

	m_iTick++;

	//	Update the painter

	if (m_pPainter)
		m_pPainter->OnUpdate();

	//	Prepare a list of all objects that intersect with us

	CVector vUR;
	CVector vLL;
	GetBoundingRect(&vUR, &vLL);

	const int MAX_OBJECTS = 128;
	CSpaceObject *ShortList[MAX_OBJECTS];
	int iShortListCount = 0;

	for (i = 0; i < GetSystem()->GetObjectCount() && iShortListCount < MAX_OBJECTS; i++)
		{
		CSpaceObject *pObj = GetSystem()->GetObject(i);

		//	If the object is in the bounding box then remember
		//	it so that we can do a more accurate calculation.

		if (pObj 
				&& CanHit(pObj) 
				&& pObj->CanBeHit() 
				&& pObj->InBox(vUR, vLL)
				&& pObj->CanBeHitBy(m_pDesc->m_Damage)
				&& pObj != this)
			ShortList[iShortListCount++] = pObj;
		}

	//	Loop over every object, see how many particles have hit it

	for (j = 0; j < iShortListCount; j++)
		{
		//	Hit test

		CVector vHitPos;
		int iTotalHit = m_Particles.HitTest(ShortList[j], m_vOldPos, GetPos(), &vHitPos);

		//	Hit the object, adjusting the damage based on the number of
		//	particles that hit

		if (iTotalHit > 0)
			{
			int iTotal = m_iDamage * (iTotalHit * 100 / m_iParticleCount);
			int iDamage = iTotal / 100;
			if (mathRandom(1, 100) <= (iTotal % 100))
				iDamage++;

			DamageDesc Damage = m_pDesc->m_Damage;
			Damage.SetDamage(iDamage);
			Damage.AddBonus(m_iBonus);
			Damage.SetCause(m_iCause);

			int iHitDir = VectorToPolar(vHitPos - ShortList[j]->GetPos());

			ShortList[j]->Damage(this,
					vHitPos,
					iHitDir,
					Damage);
			}
		}

	//	Expired?

	if (--m_iLifeLeft <= 0)
		{
		Destroy(removedFromSystem, NULL);
		return;
		}

	//	Emit new particles

	if (m_iTick < m_iEmitTime && !m_Source.IsEmpty())
		{
		InitParticles(m_pDesc->GetParticleCount(),
				m_vEmitSourcePos - GetPos(),
				m_iEmitDirection);
		}
	}

void CParticleDamage::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write out to stream
//
//	CString			CWeaponFireDesc UNID
//	DWORD			m_iBonus
//	DWORD			m_iLifeLeft
//	DWORD			m_Source (CSpaceObject ref)
//	DWORD			m_pSovereign (CSovereign ref)
//	DWORD			m_iTick
//	DWORD			m_iDamage
//	CVector			m_vOldPos
//
//	CVector			m_vEmitSourcePos
//	CVector			m_vEmitSourceVel
//	DWORD			m_iEmitDirection
//	DWORD			m_iEmitTime
//	DWORD			m_iParticleCount
//	IEffectPainter
//	CParticleArray

	{
	DWORD dwSave;
	m_pDesc->m_sUNID.WriteToStream(pStream);
	pStream->Write((char *)&m_iBonus, sizeof(m_iBonus));
	dwSave = m_iCause;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	pStream->Write((char *)&m_iLifeLeft, sizeof(m_iLifeLeft));
	m_Source.WriteToStream(GetSystem(), pStream);
	GetSystem()->WriteSovereignRefToStream(m_pSovereign, pStream);
	pStream->Write((char *)&m_iTick, sizeof(m_iTick));
	pStream->Write((char *)&m_iDamage, sizeof(m_iDamage));
	pStream->Write((char *)&m_vOldPos, sizeof(m_vOldPos));
	pStream->Write((char *)&m_vEmitSourcePos, sizeof(CVector));
	pStream->Write((char *)&m_vEmitSourceVel, sizeof(CVector));
	pStream->Write((char *)&m_iEmitDirection, sizeof(DWORD));
	pStream->Write((char *)&m_iEmitTime, sizeof(DWORD));
	pStream->Write((char *)&m_iParticleCount, sizeof(DWORD));

	if (m_pPainter)
		m_pPainter->WriteToStream(pStream);

	m_Particles.WriteToStream(pStream);
	}

bool CParticleDamage::PointInObject (CVector vPos)

//	PointInObject
//
//	Returns TRUE if the given point is in the object

	{
	return false;
	}
