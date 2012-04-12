//	CAreaDamage.cpp
//
//	CAreaDamage class

#include "PreComp.h"

#include "CStream.h"

#define HIT_TEST_STEP						(8.0 * g_KlicksPerPixel)

static CObjectClass<CAreaDamage>g_Class(OBJID_CAREADAMAGE, NULL);

CAreaDamage::CAreaDamage (void) : CSpaceObject(&g_Class),
		m_pPainter(NULL),
		m_Hit(FALSE, TRUE),
		m_pSavedObj(NULL),
		m_pSavedValue(NULL)

//	CAreaDamage constructor

	{
	}

CAreaDamage::~CAreaDamage (void)

//	CAreaDamage destructor

	{
	if (m_pPainter)
		m_pPainter->Delete();
	}

bool CAreaDamage::AlreadyHit (CSpaceObject *pObj)

//	AlreadyHit
//
//	TRUE if we already hit this object

	{
#if 0
	for (int i = 0; i < m_Hit.GetCount(); i++)
		if (m_Hit.GetObject(i) == (CObject *)pObj)
			return true;

#endif
	return false;
	}

ALERROR CAreaDamage::Create (CSystem *pSystem,
							 CWeaponFireDesc *pDesc,
							 int iBonus,
							 DestructionTypes iCause,
							 const CDamageSource &Source,
							 const CVector &vPos,
							 const CVector &vVel,
							 CAreaDamage **retpObj)

//	Create
//
//	Create the object

	{
	ALERROR error;

	//	Make sure we have a valid CWeaponFireDesc (otherwise we won't be
	//	able to save it the area of effect).
	ASSERT(!pDesc->m_sUNID.IsBlank());

	//	Create the area

	CAreaDamage *pArea = new CAreaDamage;
	if (pArea == NULL)
		return ERR_MEMORY;

	pArea->SetPos(vPos);
	pArea->SetVel(vVel);
	pArea->CalculateMove();

	//	Get notifications when other objects are destroyed
	pArea->SetObjectDestructionHook();

	pArea->m_pDesc = pDesc;
	pArea->m_iBonus = iBonus;
	pArea->m_iCause = iCause;
	pArea->m_iLifeLeft = pDesc->GetLifetime();
	pArea->m_Source = Source;
	pArea->m_iInitialDelay = pDesc->GetInitialDelay();
	pArea->m_iTick = 0;

	//	Remember the sovereign of the source (in case the source is destroyed)

	if (Source.GetObj())
		pArea->m_pSovereign = Source.GetObj()->GetSovereign();
	else
		pArea->m_pSovereign = NULL;

	//	Create a painter instance

	if (pDesc->m_pEffect)
		pArea->m_pPainter = pDesc->m_pEffect->CreatePainter();

	//	Add to system

	if ((error = pArea->AddToSystem(pSystem)))
		{
		delete pArea;
		return error;
		}

	//	Done

	if (retpObj)
		*retpObj = pArea;

	return NOERROR;
	}

CString CAreaDamage::GetName (DWORD *retdwFlags)

//	GetName
//
//	Returns the name of the object

	{
	//	This name is used only if the source has been destroyed

	if (retdwFlags)
		*retdwFlags = 0;
	return CONSTLIT("enemy weapon");
	}

void CAreaDamage::OnMove (const CVector &vOldPos, Metric rSeconds)

//	OnMove
//
//	Handle moving

	{
	}

void CAreaDamage::ObjectDestroyedHook (const SDestroyCtx &Ctx)

//	ObjectDestroyedHook
//
//	Called when another object is destroyed

	{
	m_Source.OnObjDestroyed(Ctx.pObj);

	//	Remove the entry

	m_Hit.RemoveEntry((int)Ctx.pObj, NULL);
	}

void CAreaDamage::OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	OnPaint
//
//	Paint

	{
	if (m_iInitialDelay > 0)
		return;

	if (m_pPainter)
		{
		Ctx.iTick = m_iTick;
		Ctx.iVariant = 0;
		Ctx.iDestiny = GetDestiny();
		m_pPainter->Paint(Dest, x, y, Ctx);
		}
	}

void CAreaDamage::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Restore from stream

	{
	DWORD dwLoad;

#ifdef DEBUG_LOAD
	::OutputDebugString("CAreaDamage::OnReadFromStream\n");
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

	if (Ctx.dwVersion >= 21)
		Ctx.pStream->Read((char *)&m_iInitialDelay, sizeof(DWORD));
	else
		m_iInitialDelay = 0;

	Ctx.pStream->Read((char *)&m_iLifeLeft, sizeof(m_iLifeLeft));
	m_Source.ReadFromStream(Ctx);
	Ctx.pSystem->ReadSovereignRefFromStream(Ctx, &m_pSovereign);
	Ctx.pStream->Read((char *)&m_iTick, sizeof(m_iTick));

	//	Load painter

	if (m_pDesc->m_pEffect)
		m_pPainter = m_pDesc->m_pEffect->CreatePainterFromStreamAndCreator(Ctx);

	//	Load the m_Hit array. We load all the objects into a temporary
	//	array and then add them to the m_Hit array when we are done.
	//	We need to do this because the m_Hit array is indexed by CSpaceObject pointer.

	Ctx.pStream->Read((char *)&m_iSavedCount, sizeof(DWORD));
	if (m_iSavedCount)
		{
		typedef CSpaceObject *PSPACEOBJECT;
		m_pSavedObj = new PSPACEOBJECT [m_iSavedCount];
		m_pSavedValue = new DWORD [m_iSavedCount];

		for (int i = 0; i < (int)m_iSavedCount; i++)
			{
			Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_pSavedObj[i]);
			Ctx.pStream->Read((char *)&m_pSavedValue[i], sizeof(DWORD));
			}
		}
	}

void CAreaDamage::OnSystemLoaded (void)

//	OnSystemLoaded
//
//	System done loading

	{
	int i;

	for (i = 0; i < m_iSavedCount; i++)
		m_Hit.AddEntry((int)m_pSavedObj[i], (CObject *)m_pSavedValue[i]);

	if (m_pSavedObj)
		{
		delete [] m_pSavedObj;
		m_pSavedObj = NULL;
		}

	if (m_pSavedValue)
		{
		delete [] m_pSavedValue;
		m_pSavedValue = NULL;
		}
	}

void CAreaDamage::OnUpdate (Metric rSecondsPerTick)

//	OnUpdate
//
//	Update

	{
	int i, j;
	bool bDestroy = false;

	if (m_iInitialDelay > 0)
		{
		m_iInitialDelay--;
		return;
		}

	if (m_pPainter == NULL)
		{
		if (--m_iLifeLeft <= 0)
			Destroy(removedFromSystem, NULL);
		return;
		}

	//	Figure out the bounding area of the damage

	RECT rcBounds;
	m_pPainter->GetRect(&rcBounds);
	SetBounds(rcBounds);

	CVector vUR, vLL;
	GetBoundingRect(&vUR, &vLL);

	//	Figure out how many points on the circle we should check for
	//	intersection.

	int iPoints = 32;
	Metric rRadius = (RectWidth(rcBounds) / 2) * g_KlicksPerPixel;
	int iPointOffset = mathRandom(0, 359);

	//	Compute previous radius

	Metric rPrevRadius = Max(0.0, rRadius - m_pDesc->m_rExpansionSpeed);
	Metric rStep = HIT_TEST_STEP;

	int iSegCount = Max(1, (int)((g_Pi * rRadius) / HIT_TEST_STEP));
	int iAngleStep = Max(1, 360 / iSegCount);

	//	Loop over all objects and see if we hit any

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
			//	Get the point bitmap for this object

			DWORD dwBitmap;
			if (m_Hit.Lookup((int)pObj, (CObject **)&dwBitmap) == ERR_NOTFOUND)
				dwBitmap = 0;

			//	Check to see which part of the circle we intersect

			DWORD dwPointTest = 1;
			bool bHit = false;
			bool bObjDestroyed = false;
			for (j = 0; 
					j < iPoints && !bObjDestroyed; 
					j++, (dwPointTest = dwPointTest << 1))
				{
				//	If this point has already hit the object, then continue

				if (dwBitmap & dwPointTest)
					continue;

				//	See if any part of the segment hits the object

				int iAngleStart = iPointOffset + (j * 360 / iPoints);
				int iAngleEnd = iPointOffset + ((j + 1) * 360 / iPoints);

				CVector vPoint;
				bool bHitPoint = false;
				int iAngle = iAngleStart;
				while (iAngle < iAngleEnd && !bHitPoint)
					{
					Metric rRadiusPos = rPrevRadius + rStep;
					while (rRadiusPos < rRadius && !bHitPoint)
						{
						vPoint = GetPos() + PolarToVector(iAngle, rRadiusPos);
						bHitPoint = (pObj->InBox(vPoint, vPoint) && pObj->PointInObject(vPoint));
						rRadiusPos += rStep;
						}

					if (!bHitPoint)
						{
						vPoint = GetPos() + PolarToVector(iAngle, rRadius);
						bHitPoint = (pObj->InBox(vPoint, vPoint) && pObj->PointInObject(vPoint));
						}

					iAngle = iAngle + iAngleStep;
					}

				//	If we hit, then do damage

				if (bHitPoint)
					{
					//	Remember that this point hit

					dwBitmap |= dwPointTest;

					//	Do damage

					DamageResults result;
					DamageDesc Damage = m_pDesc->m_Damage;
					Damage.AddBonus(m_iBonus);
					Damage.SetCause(m_iCause);
					result = pObj->Damage(this,
							vPoint,
							(iAngle + 180) % 360,
							Damage);

					if (result == CSpaceObject::damageDestroyed || result == CSpaceObject::damagePassthrough)
						bObjDestroyed = true;
					else
						bHit = true;
					}
				}

			//	Add it to the list of objects that we've hit
			//	We do this first in case we end up destroying the
			//	object (in which case we will get called back
			//	inside of Damage())

			if (bHit && !bObjDestroyed)
				{
				ASSERT(pObj->NotifyOthersWhenDestroyed());
				m_Hit.ReplaceEntry((int)pObj, (CObject *)dwBitmap, TRUE, NULL);
				}
			}
		}

	//	Update the object

	m_iTick++;
	m_pPainter->OnUpdate();

	//	Destroy

	if (bDestroy || --m_iLifeLeft <= 0)
		Destroy(removedFromSystem, NULL);
	}

void CAreaDamage::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write out to stream
//
//	CString			CWeaponFireDesc UNID
//	DWORD			m_iBonus
//	DWORD			m_iCause
//	DWORD			m_iInitialDelay
//	DWORD			m_iLifeLeft
//	DWORD			m_Source (CDamageSource)
//	DWORD			m_pSovereign (CSovereign ref)
//	DWORD			m_iTick
//	IEffectPainter	m_pPainter
//
//	DWORD			No. of objects in m_Hit
//	DWORD			[m_Hit] (CSpaceObject ref)

	{
	DWORD dwSave;

	m_pDesc->m_sUNID.WriteToStream(pStream);
	pStream->Write((char *)&m_iBonus, sizeof(m_iBonus));
	dwSave = m_iCause;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	pStream->Write((char *)&m_iInitialDelay, sizeof(DWORD));
	pStream->Write((char *)&m_iLifeLeft, sizeof(m_iLifeLeft));
	m_Source.WriteToStream(GetSystem(), pStream);
	GetSystem()->WriteSovereignRefToStream(m_pSovereign, pStream);
	pStream->Write((char *)&m_iTick, sizeof(m_iTick));

	if (m_pPainter)
		m_pPainter->WriteToStream(pStream);

	//	Write out the m_Hit array

	dwSave = m_Hit.GetCount();
	pStream->Write((char *)&dwSave, sizeof(dwSave));
	for (int i = 0; i < m_Hit.GetCount(); i++)
		{
		GetSystem()->WriteObjRefToStream((CSpaceObject *)m_Hit.GetKey(i), pStream);
		dwSave = (DWORD)m_Hit.GetValue(i);
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		}
	}

void CAreaDamage::PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans)

//	PaintLRS
//
//	Paints the object on an LRS

	{
	if (m_pPainter)
		{
		int i;
		Metric rRadius = m_pPainter->GetRadius();
		int iRadius = (int)((rRadius / g_KlicksPerPixel) + 0.5);
		int iCount = Min(64, 3 * iRadius);

		for (i = 0; i < iCount; i++)
			{
			CVector vPos = GetPos() + PolarToVector(mathRandom(0, 359), rRadius);
			Trans.Transform(vPos, &x, &y);

			int iColor = mathRandom(128, 255);
			Dest.DrawDot(x, y, 
					CG16bitImage::RGBValue(iColor, iColor, 0), 
					CG16bitImage::markerSmallRound);
			}
		}
	}

bool CAreaDamage::PointInObject (CVector vPos)

//	PointInObject
//
//	Returns TRUE if the given point is in the object

	{
	if (m_pPainter)
		{
		RECT rcBounds;
		m_pPainter->GetRect(&rcBounds);
		Metric rRadius = (RectWidth(rcBounds) / 2) * g_KlicksPerPixel;
		Metric rRadius2 = rRadius * rRadius;
		Metric rInnerRadius = rRadius * 0.8;
		Metric rInnerRadius2 = rInnerRadius * rInnerRadius;
		CVector vDist = GetPos() - vPos;
		Metric rDist2 = vDist.Length2();
		return (rDist2 < rRadius2 && rDist2 > rInnerRadius2);
		}
	else
		return false;
	}
