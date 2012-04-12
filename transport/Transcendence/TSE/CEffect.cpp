//	CEffect.cpp
//
//	CEffect class

#include "PreComp.h"


static CObjectClass<CEffect>g_Class(OBJID_CEFFECT, NULL);

CEffect::CEffect (void) : CSpaceObject(&g_Class), m_pPainter(NULL)

//	CEffect constructor

	{
	}

CEffect::~CEffect (void)

//	CEffect destructor

	{
	if (m_pPainter)
		m_pPainter->Delete();
	}

ALERROR CEffect::Create (CEffectCreator *pType,
				CSystem *pSystem,
				CSpaceObject *pAnchor,
				const CVector &vPos,
				const CVector &vVel,
				CEffect **retpEffect)

//	Create
//
//	Creates a new effects object

	{
	ALERROR error;
	CEffect *pEffect;

	pEffect = new CEffect;
	if (pEffect == NULL)
		return ERR_MEMORY;

	pEffect->SetPos(vPos);
	if (pAnchor == NULL)
		pEffect->SetVel(vVel);
	pEffect->CalculateMove();
	pEffect->SetObjectDestructionHook();

	ASSERT(pType);
	pEffect->m_pPainter = pType->CreatePainter();
	pEffect->m_pAnchor = pAnchor;
	pEffect->m_iLifetime = pType->GetLifetime();
	pEffect->m_iTick = 0;

	//	Add to system

	if (error = pEffect->AddToSystem(pSystem))
		{
		delete pEffect;
		return error;
		}

	//	Play sound

	pType->PlaySound(pEffect);

	//	Done

	if (retpEffect)
		*retpEffect = pEffect;

	return NOERROR;
	}

void CEffect::OnMove (const CVector &vOldPos, Metric rSeconds)

//	OnMove
//
//	Move hook

	{
	//	If we've got an anchor then move the effect along with the anchor

	if (m_pAnchor)
		SetPos(GetPos() + (m_pAnchor->GetVel() * rSeconds));
	}

void CEffect::ObjectDestroyedHook (const SDestroyCtx &Ctx)

//	ObjectDestroyedHook
//
//	Handle it when another object is destroyed

	{
	//	If our source is destroyed, clear it

	if (Ctx.pObj == m_pAnchor)
		m_pAnchor = NULL;
	}

void CEffect::OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	OnPaint
//
//	Paints the effect

	{
	if (m_pPainter)
		{
		Ctx.iTick = m_iTick;
		Ctx.iVariant = 0;
		Ctx.iDestiny = GetDestiny();
		m_pPainter->Paint(Dest, x, y, Ctx);
		}
	}

void CEffect::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read object data from a stream
//
//	DWORD		IEffectPainter
//	DWORD		m_pAnchor (CSpaceObject Ref)
//	DWORD		m_iLifetime
//	DWORD		m_iTick

	{
#ifdef DEBUG_LOAD
	::OutputDebugString("CEffect::OnReadFromStream\n");
#endif
	m_pPainter = CEffectCreator::CreatePainterFromStream(Ctx);
	Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_pAnchor);
	Ctx.pStream->Read((char *)&m_iLifetime, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iTick, sizeof(DWORD));
	}

void CEffect::OnUpdate (Metric rSecondsPerTick)

//	OnUpdate
//
//	Update the effect

	{
	bool bDestroyEffect = false;

	//	See if the effect has faded out

	m_iTick++;
	if (m_iTick >= m_iLifetime)
		{
		Destroy(removedFromSystem, NULL);
		return;
		}

	//	If we're moving, slow down

	SetVel(CVector(GetVel().GetX() * g_SpaceDragFactor, GetVel().GetY() * g_SpaceDragFactor));

	//	Give the painter a chance to update

	if (m_pPainter)
		m_pPainter->OnUpdate();
	}

void CEffect::OnUpdateExtended (void)

//	OnUpdateExtended
//
//	Update a prolonged time

	{
	//	Updates don't last for an extended period of time

	Destroy(removedFromSystem, NULL);
	}

void CEffect::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write the object's data to stream
//
//	DWORD		IEffectPainter
//	DWORD		m_pAnchor (CSpaceObject Ref)
//	DWORD		m_iLifetime
//	DWORD		m_iTick

	{
	if (m_pPainter)
		m_pPainter->WriteToStream(pStream);
	else
		{
		//	Write out a NULL UNID

		CString sDummy;
		sDummy.WriteToStream(pStream);
		}

	GetSystem()->WriteObjRefToStream(m_pAnchor, pStream);
	pStream->Write((char *)&m_iLifetime, sizeof(DWORD));
	pStream->Write((char *)&m_iTick, sizeof(DWORD));
	}

