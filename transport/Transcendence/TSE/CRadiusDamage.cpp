//	CRadiusDamage.cpp
//
//	CRadiusDamage class

#include "PreComp.h"

static CObjectClass<CRadiusDamage>g_Class(OBJID_CRADIUSDAMAGE, NULL);

CRadiusDamage::CRadiusDamage (void) : CSpaceObject(&g_Class),
		m_pPainter(NULL)

//	CRadiusDamage constructor

	{
	}

CRadiusDamage::~CRadiusDamage (void)

//	CRadiusDamage destructor

	{
	if (m_pPainter)
		m_pPainter->Delete();
	}

ALERROR CRadiusDamage::Create (CSystem *pSystem,
							   CWeaponFireDesc *pDesc,
							   int iBonus,
							   DestructionTypes iCause,
							   const CDamageSource &Source,
							   const CVector &vPos,
							   const CVector &vVel,
							   CSpaceObject *pTarget,
							   CRadiusDamage **retpObj)

//	Create
//
//	Create the object

	{
	ALERROR error;

	//	Make sure we have a valid CWeaponFireDesc (otherwise we won't be
	//	able to save the descriptor).
	ASSERT(!pDesc->m_sUNID.IsBlank());

	//	Create the area

	CRadiusDamage *pArea = new CRadiusDamage;
	if (pArea == NULL)
		return ERR_MEMORY;

	pArea->SetPos(vPos);
	pArea->SetVel(vVel);
	pArea->CalculateMove();

	//	Get notifications when other objects are destroyed
	pArea->SetObjectDestructionHook();

	pArea->m_iLifeLeft = pDesc->GetLifetime();
	pArea->m_pDesc = pDesc;
	pArea->m_iBonus = iBonus;
	pArea->m_iCause = iCause;
	pArea->m_Source = Source;
	pArea->m_pTarget = pTarget;
	pArea->m_iTick = 0;

	Metric rRadius = pDesc->GetMaxRadius();

	//	Remember the sovereign of the source (in case the source is destroyed)

	if (Source.GetObj())
		pArea->m_pSovereign = Source.GetObj()->GetSovereign();
	else
		pArea->m_pSovereign = NULL;

	//	Create a painter instance

	if (pDesc->m_pEffect)
		{
		pArea->m_pPainter = pDesc->m_pEffect->CreatePainter();

		//	The lifetime of the object is based on the painter

		pArea->m_iLifeLeft = Max(pDesc->m_pEffect->GetLifetime(), pArea->m_iLifeLeft);

		//	The radius is also adjusted

		rRadius = Max(pArea->m_pPainter->GetRadius(), rRadius);
		}
	else
		pArea->m_pPainter = NULL;

	//	Our bounds are based on the max radius

	pArea->SetBounds(rRadius);

	//	Add to system

	if (error = pArea->AddToSystem(pSystem))
		{
		delete pArea;
		return error;
		}

	//	Done

	if (retpObj)
		*retpObj = pArea;

	return NOERROR;
	}

CString CRadiusDamage::GetName (DWORD *retdwFlags)

//	GetName
//
//	Returns the name of the object

	{
	//	This name is used only if the source has been destroyed

	if (retdwFlags)
		*retdwFlags = 0;
	return CONSTLIT("enemy weapon");
	}

void CRadiusDamage::OnMove (const CVector &vOldPos, Metric rSeconds)

//	OnMove
//
//	Handle moving

	{
	}

void CRadiusDamage::ObjectDestroyedHook (const SDestroyCtx &Ctx)

//	ObjectDestroyedHook
//
//	Called when another object is destroyed

	{
	m_Source.OnObjDestroyed(Ctx.pObj);

	if (Ctx.pObj == m_pTarget)
		m_pTarget = NULL;
	}

void CRadiusDamage::OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	OnPaint
//
//	Paint

	{
	if (m_pPainter)
		{
		Ctx.iTick = m_iTick;
		Ctx.iVariant = 0;
		Ctx.iDestiny = GetDestiny();
		m_pPainter->Paint(Dest, x, y, Ctx);
		}
	}

void CRadiusDamage::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Restore from stream

	{
	DWORD dwLoad;

#ifdef DEBUG_LOAD
	::OutputDebugString("CRadiusDamage::OnReadFromStream\n");
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
	if (Ctx.dwVersion >= 19)
		Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_pTarget);
	else
		m_pTarget = NULL;

	Ctx.pSystem->ReadSovereignRefFromStream(Ctx, &m_pSovereign);
	Ctx.pStream->Read((char *)&m_iTick, sizeof(m_iTick));

	//	Load painter

	if (m_pDesc->m_pEffect)
		m_pPainter = m_pDesc->m_pEffect->CreatePainterFromStreamAndCreator(Ctx);
	}

void CRadiusDamage::OnSystemLoaded (void)

//	OnSystemLoaded
//
//	System done loading

	{
	}

void CRadiusDamage::OnUpdate (Metric rSecondsPerTick)

//	OnUpdate
//
//	Update

	{
	int i;
	bool bDestroy = false;

	//	Do damage right away

	if (m_iTick == 0)
		{
		Metric rMinRadius = m_pDesc->GetMinRadius();
		Metric rMaxRadius = m_pDesc->GetMaxRadius();
		Metric rRadiusRange = rMaxRadius - rMinRadius;

		if (rMaxRadius > 0.0)
			{
			CVector vUR, vLL;
			GetBoundingRect(&vUR, &vLL);

			for (i = 0; i < GetSystem()->GetObjectCount(); i++)
				{
				CSpaceObject *pObj = GetSystem()->GetObject(i);
				if (pObj 
						&& CanHit(pObj)
						&& pObj->CanBeHit()
						&& pObj->InBox(vUR, vLL)
						&& pObj->CanBeHitBy(m_pDesc->m_Damage)
						&& pObj != this)
					{
					//	Compute the distance between this object and the center
					//	of the blast

					CVector vDist = (pObj->GetPos() - GetPos());
					Metric rDist;
					int iAngle = VectorToPolar(vDist, &rDist);

					//	Adjust damage for distance

					if (rDist < rMaxRadius)
						{
						DamageDesc Damage = m_pDesc->m_Damage;
						Damage.AddBonus(m_iBonus);
						Damage.SetCause(m_iCause);

						//	If we're beyond the minimum radius, then decrease the damage
						//	to account for distance

						if (rDist > rMinRadius && rRadiusRange > 0.0)
							{
							Metric rMult = (rRadiusRange - (rDist - rMinRadius)) / rRadiusRange;

							int iDamage = (int)(rMult * (Metric)Damage.RollDamage() + 0.5);
							Damage.SetDamage(iDamage);
							}

						//	Find the point where we hit the object

						CVector vHitPos = GetPos();
						CVector vInc = PolarToVector(iAngle, 2.0 * g_KlicksPerPixel);
						int iMax = (int)((rDist / (2.0 * g_KlicksPerPixel)) + 0.5);
						while (!pObj->PointInObject(vHitPos) && iMax-- > 0)
							vHitPos = vHitPos + vInc;

						//	Do damage

						pObj->Damage(this,
								vHitPos,
								(iAngle + 180) % 360,
								Damage);
						}
					}
				}
			}

		//	Spawn fragments, if necessary

		if (m_pDesc->HasFragments())
			{
			GetSystem()->CreateWeaponFragments(m_pDesc,
					m_iBonus,
					m_iCause,
					m_Source,
					m_pTarget,
					GetPos(),
					GetVel(),
					this);
			}
		}

	//	Update the object

	m_iTick++;

	if (m_pPainter)
		m_pPainter->OnUpdate();

	//	Destroy

	if (bDestroy || --m_iLifeLeft <= 0)
		Destroy(removedFromSystem, NULL);
	}

void CRadiusDamage::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write out to stream
//
//	CString			CWeaponFireDesc UNID
//	DWORD			m_iBonus
//	DWORD			m_iCause
//	DWORD			m_iLifeLeft
//	DWORD			m_Source (CSpaceObject ref)
//	DWORD			m_pTarget (CSpaceObject ref)
//	DWORD			m_pSovereign (CSovereign ref)
//	DWORD			m_iTick
//	IEffectPainter	m_pPainter

	{
	DWORD dwSave;

	m_pDesc->m_sUNID.WriteToStream(pStream);
	pStream->Write((char *)&m_iBonus, sizeof(m_iBonus));
	dwSave = m_iCause;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	pStream->Write((char *)&m_iLifeLeft, sizeof(m_iLifeLeft));
	m_Source.WriteToStream(GetSystem(), pStream);
	GetSystem()->WriteObjRefToStream(m_pTarget, pStream);
	GetSystem()->WriteSovereignRefToStream(m_pSovereign, pStream);
	pStream->Write((char *)&m_iTick, sizeof(m_iTick));

	if (m_pPainter)
		m_pPainter->WriteToStream(pStream);
	}

bool CRadiusDamage::PointInObject (CVector vPos)

//	PointInObject
//
//	Returns TRUE if the given point is in the object

	{
	return false;
	}
