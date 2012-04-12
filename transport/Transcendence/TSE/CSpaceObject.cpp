//	CSpaceObject.cpp
//
//	CSpaceObject class

#include "PreComp.h"
#include "Kernel.h"

#define MAX_DELTA								(2.0 * g_KlicksPerPixel)
#define MAX_DELTA2								(MAX_DELTA * MAX_DELTA)
#define MAX_DELTA_VEL							(g_KlicksPerPixel / 2.0)
#define MAX_DELTA_VEL2							(MAX_DELTA_VEL * MAX_DELTA_VEL)

#define MAX_DISTANCE							(700.0 * g_KlicksPerPixel)

const Metric g_rMaxCommsRange =					(LIGHT_MINUTE * 2.0);
const Metric g_rMaxCommsRange2 =				(g_rMaxCommsRange * g_rMaxCommsRange);

#define BOUNDS_CHECK_DIST 						(256.0 * g_KlicksPerPixel)
#define BOUNDS_CHECK_DIST2						(BOUNDS_CHECK_DIST * BOUNDS_CHECK_DIST)

static DATADESCSTRUCT g_DataDesc[] =
	{	{ DATADESC_OPCODE_REFERENCE,	1,	0 },		//	m_pSystem
		{ DATADESC_OPCODE_INT,			1,	0 },		//	m_iDestiny
		{ DATADESC_OPCODE_INT,			2,	0 },		//	m_vPos
		{ DATADESC_OPCODE_INT,			2,	0 },		//	m_vVel
		{ DATADESC_OPCODE_INT,			1,	0 },		//	m_dwFlags
		{ DATADESC_OPCODE_STOP,	0,	0 } };
static CObjectClass<CSpaceObject>g_Class(OBJID_CSPACEOBJECT, g_DataDesc);

#define HIGHLIGHT_CORNER_WIDTH					8
#define HIGHLIGHT_CORNER_HEIGHT					8

#define STR_G_ITEM								CONSTLIT("gItem")
#define STR_G_SOURCE							CONSTLIT("gSource")

#define STR_UNCHARTED							CONSTLIT("uncharted")

#define GET_EXPLOSION_TYPE_EVENT				CONSTLIT("GetExplosionType")
#define ON_ATTACKED_EVENT						CONSTLIT("OnAttacked")
#define ON_ATTACKED_BY_PLAYER_EVENT				CONSTLIT("OnAttackedByPlayer")
#define ON_CREATE_EVENT							CONSTLIT("OnCreate")
#define ON_DESTROY_EVENT						CONSTLIT("OnDestroy")
#define ON_ENTERED_GATE_EVENT					CONSTLIT("OnEnteredGate")
#define ON_OBJ_DESTROYED_EVENT					CONSTLIT("OnObjDestroyed")
#define ON_OBJ_DOCKED_EVENT						CONSTLIT("OnObjDocked")
#define ON_OBJ_ENTERED_GATE_EVENT				CONSTLIT("OnObjEnteredGate")
#define ON_OBJ_JUMPED_EVENT						CONSTLIT("OnObjJumped")
#define ON_OBJ_RECONNED_EVENT					CONSTLIT("OnObjReconned")
#define ON_ORDER_CHANGED_EVENT					CONSTLIT("OnOrderChanged")
#define ON_ORDERS_COMPLETED_EVENT				CONSTLIT("OnOrdersCompleted")
#define ON_MINING_EVENT							CONSTLIT("OnMining")
#define ON_DAMAGE_EVENT							CONSTLIT("OnDamage")

static Metric g_rMaxPerceptionRange[CSpaceObject::perceptMax+1] =
	{
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	};

CString ParseParam (char **ioPos);

CSpaceObject::CSpaceObject (void) : CObject(&g_Class)

//	CSpaceObject constructor

	{
	}

CSpaceObject::CSpaceObject (IObjectClass *pClass) : CObject(pClass),
		m_pSystem(NULL),
		m_iIndex(-1),
		m_fHookObjectDestruction(false),
		m_fNoObjectDestructionNotify(false),
		m_fCannotBeHit(false),
		m_fSpare(0),
		m_iControlsFrozen(0),
		m_iSpare(0),
		m_wHighlightColor(0),
		m_iHighlightCountdown(0),
		m_fSelected(false),
		m_fPlayerTarget(false),
		m_fCanBounce(false),
		m_fIsBarrier(false),
		m_fCannotMove(false),
		m_fNoFriendlyFire(false),
		m_fNoFriendlyTarget(false),
		m_rBoundsX(0.0),
		m_rBoundsY(0.0),
		m_pFirstEffect(NULL),
		m_fTimeStop(false),
		m_fAutomatedWeapon(false),
		m_fHasOnObjDockedEvent(false),
		m_fHasOnAttackedEvent(false),
		m_fHasOnDamageEvent(false),
		m_fOnCreateCalled(false),
		m_fItemEventsValid(false)

//	CSpaceObject constructor

	{
	m_iDestiny = mathRandom(0, g_DestinyRange - 1);
	m_dwID = g_pUniverse->CreateGlobalID();
	}

CSpaceObject::~CSpaceObject (void)

//	CSpaceObject destructor

	{
	//	Can't turn this on until system destroys spaces objects
	//	explicitly.
#if 0
	ASSERT(m_pSystem == NULL);
#endif

	//	Delete the list of effects

	SEffectNode *pNext = m_pFirstEffect;
	while (pNext)
		{
		SEffectNode *pDelete = pNext;
		pNext = pNext->pNext;

		pDelete->pPainter->Delete();
		delete pDelete;
		}

#ifdef DEBUG
	//	Make sure the object is not being held by anyone else

	if (g_pUniverse && g_pUniverse->GetPOV())
		{
		CSystem *pSystem = g_pUniverse->GetPOV()->GetSystem();
		for (int i = 0; i < pSystem->GetObjectCount(); i++)
			{
			CSpaceObject *pObj = pSystem->GetObject(i);
			if (pObj)
				{
				if (pObj->m_Data.FindObjRefData(this))
					ASSERT(false);
				}
			}
		}
#endif
	}

void CSpaceObject::Accelerate (const CVector &vPush, Metric rSeconds)

//	Accelerate
//
//	Accelerates the given object along the given vector. The magnitude of
//	the vector is the force used in gigaNewtons(!). The acceleration is
//	maintained for rSeconds

	{
	Metric rMass = GetMass();

	if (rMass)
		{
		//	rAccel needs to be in klicks per second (we assume here
		//	that 1 klick = 1,000 meters).
		CVector rAccel = (vPush * 1000.0) / rMass;
		m_vVel = m_vVel + (rAccel * rSeconds);
		}
	}

void CSpaceObject::AccelerateStop (Metric rPush, Metric rSeconds)

//	AccelerateStop
//
//	Slows down the object with the given thrust

	{
	Metric rMass = GetMass();

	if (rMass)
		{
		Metric rAccel = rPush * 1000.0 / rMass;

		Metric rLength;
		CVector vDir = m_vVel.Normal(&rLength);

		if (rAccel > rLength)
			m_vVel = NullVector;
		else
			m_vVel = m_vVel - (vDir * rAccel);
		}
	}

void CSpaceObject::AddEffect (IEffectPainter *pPainter, const CVector &vPos)

//	AddEffect
//
//	Adds an effect to the object

	{
	ASSERT(pPainter->GetCreator()->IsValidUNID());

	int xOffset = (int)(((vPos.GetX() - m_vPos.GetX()) / g_KlicksPerPixel) + 0.5);
	int yOffset = (int)(((m_vPos.GetY() - vPos.GetY()) / g_KlicksPerPixel) + 0.5);

	SEffectNode *pNewNode = new SEffectNode;
	pNewNode->pPainter = pPainter;
	pNewNode->xOffset = xOffset;
	pNewNode->yOffset = yOffset;
	pNewNode->iTick = 0;
	pNewNode->pNext = m_pFirstEffect;

	m_pFirstEffect = pNewNode;
	}

void CSpaceObject::AddEffect (IEffectPainter *pPainter, int xOffset, int yOffset, int iTick)

//	AddEffect
//
//	Adds an effect to the object

	{
	ASSERT(pPainter->GetCreator()->IsValidUNID());

	SEffectNode *pNewNode = new SEffectNode;
	pNewNode->pPainter = pPainter;
	pNewNode->xOffset = xOffset;
	pNewNode->yOffset = yOffset;
	pNewNode->iTick = iTick;
	pNewNode->pNext = m_pFirstEffect;

	m_pFirstEffect = pNewNode;
	}

ALERROR CSpaceObject::AddToSystem (CSystem *pSystem)

//	AddToSystem
//
//	Adds the object to the system

	{
	ALERROR error;

	//	We can get here with m_pSystem already set during load
	ASSERT(m_pSystem == NULL || m_pSystem == pSystem);

	if (error = pSystem->AddToSystem(this, &m_iIndex))
		return error;

	m_pSystem = pSystem;

	return NOERROR;
	}

Metric CSpaceObject::CalculateItemMass (Metric *retrCargoMass)

//	CalculateCargoMass
//
//	Returns the total mass of the items

	{
	CItemListManipulator Items(GetItemList());
	Metric rTotal = 0.0;
	Metric rTotalCargo = 0.0;

	while (Items.MoveCursorForward())
		{
		const CItem &Item = Items.GetItemAtCursor();
		Metric rMass = Item.GetType()->GetMass() * Item.GetCount();

		//	All items count towards item mass

		rTotal += rMass;

		//	Only uninstalled items count in cargo space

		if (!Item.IsInstalled())
			rTotalCargo += rMass;
		}

	if (retrCargoMass)
		*retrCargoMass = rTotalCargo;

	return rTotal;
	}

void CSpaceObject::CalculateMove (void)

//	CalculateMove
//
//	Determines the position for the object for the next tick
//	(barring any collisions)

	{
	if (!m_vVel.IsNull())
		m_vNewPos = m_vPos + (m_vVel * g_SecondsPerUpdate);
	else
		m_vNewPos = m_vPos;
	}

bool CSpaceObject::CanCommunicateWith (CSpaceObject *pSender)

//	CanCommunicateWith
//
//	Returns TRUE if this object can receive communications from pSender

	{
	int i;

	//	We can't communicate if we don't have a handler

	CCommunicationsHandler *pHandler = GetCommsHandler();
	if (pHandler == NULL)
		return false;

	//	We can't communicate if we don't know about the object

	if (!IsKnown())
		return false;

	//	We can't communicate if we are out of range

	if ((pSender->GetPos() - m_vPos).Length2() > g_rMaxCommsRange2)
		return false;

	//	See if any of the messages are valid. If at least
	//	one is, then we can communicate.

	for (i = 0; i < pHandler->GetCount(); i++)
		{
		if (pHandler->GetMessage(i).pOnShow == NULL)
			return true;
		else
			{
			CCodeChain &CC = g_pUniverse->GetCC();

			//	Define parameters

			DefineGlobalSpaceObject(CC, CONSTLIT("gSource"), this);
			DefineGlobalSpaceObject(CC, CONSTLIT("gSender"), pSender);

			//	Execute

			bool bShow;

			ICCItem *pResult = CC.TopLevel(pHandler->GetMessage(i).pOnShow, &g_pUniverse);

			if (pResult->IsNil())
				bShow = false;
			else if (pResult->IsError())
				{
				pSender->SendMessage(this, pResult->GetStringValue());
				bShow = false;
				}
			else
				bShow = true;

			pResult->Discard(&CC);
			if (bShow)
				return bShow;
			}
		}

	return false;
	}

bool CSpaceObject::CanDetect (int iPerception, CSpaceObject *pObj)

//	CanDetect
//
//	Returns TRUE if this object (with given perception) can detect the target

	{
	CVector vDist = pObj->GetPos() - GetPos();
	return (vDist.Length2() < pObj->GetDetectionRange2(iPerception));
	}

bool CSpaceObject::CanFireOnObjHelper (CSpaceObject *pObj)

//	CanFireOnObjHelper
//
//	Return TRUE if a missile fired by this object can hit the given object

	{
	return (
		//	We cannot hit our friends (if our source can't)
		//	(NOTE: we check for sovereign as opposed to IsEnemy because
		//	it is faster. For our purposes, same sovereign is what we want).
		(CanHitFriends() || GetSovereign() != pObj->GetSovereign())
		
		//	We cannot hit if the obj cannot be hit by friends
		&& (pObj->CanBeHitByFriends() || GetSovereign() != pObj->GetSovereign()));
	}

void CSpaceObject::ClipSpeed (Metric rMaxSpeed)

//	ClipSpeed
//
//	Make sure object is not exceeding absolute speed limit

	{
	Metric rSpeed2 = GetVel().Dot(GetVel());
	if (rSpeed2 > (rMaxSpeed * rMaxSpeed))
		{
		CVector vNewVel = GetVel().Normal() * rMaxSpeed;
		SetVel(vNewVel);
		}
	}

void CSpaceObject::CommsMessageFrom (CSpaceObject *pSender, int iIndex)

//	CommsMessageFrom
//
//	Handle comms message from the sender

	{
	CCommunicationsHandler *pHandler = GetCommsHandler();
	ASSERT(pHandler && iIndex < pHandler->GetCount());
	const CCommunicationsHandler::SMessage &Msg = pHandler->GetMessage(iIndex);

	if (Msg.pCode)
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		//	Define parameters

		DefineGlobalSpaceObject(CC, CONSTLIT("gSource"), this);
		DefineGlobalSpaceObject(CC, CONSTLIT("gSender"), pSender);

		//	Execute

		ICCItem *pResult = CC.TopLevel(Msg.pCode, &g_pUniverse);
		if (pResult->IsError())
			pSender->SendMessage(this, pResult->GetStringValue());

		pResult->Discard(&CC);
		}
	}

void CSpaceObject::CreateFromStream (SLoadCtx &Ctx, CSpaceObject **retpObj)

//	CreateFromStream
//
//	Creates an object from the stream
//
//	DWORD		ObjID
//	DWORD		m_iIndex
//	DWORD		m_dwID
//	DWORD		m_iDestiny
//	Vector		m_vPos
//	Vector		m_vVel
//	Metric		m_rBoundsX
//	Metric		m_rBoundsY
//	DWORD		low = m_wHighlightColor; hi = m_iHighlightCountdown
//	CItemList	m_ItemList
//	DWORD		m_iControlsFrozen
//	DWORD		flags
//	CAttributeDataBlock	m_Data
//
//	For each effect:
//	IEffectPainter (0 == no more)
//	DWORD		x
//	DWORD		y
//	DWORD		iTick

	{
	//	Create the object

	DWORD dwLoad;
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	CSpaceObject *pObj = dynamic_cast<CSpaceObject *>(CObjectClassFactory::Create((OBJCLASSID)dwLoad));

	//	Load the index. This will not be the final index (because the
	//	index will change relative to the new system). But this is the
	//	index that other objects will refer to during load.

	Ctx.pStream->Read((char *)&pObj->m_iIndex, sizeof(DWORD));

	//	Load the global ID

	if (Ctx.dwVersion >= 13)
		Ctx.pStream->Read((char *)&pObj->m_dwID, sizeof(DWORD));
	else
		pObj->m_dwID = g_pUniverse->CreateGlobalID();

	//	Set the system as soon as possible because we rely on it during loading

	pObj->m_pSystem = Ctx.pSystem;

	//	Load other stuff

	Ctx.pStream->Read((char *)&pObj->m_iDestiny, sizeof(DWORD));
	Ctx.pStream->Read((char *)&pObj->m_vPos, sizeof(CVector));
	Ctx.pStream->Read((char *)&pObj->m_vVel, sizeof(CVector));
	Ctx.pStream->Read((char *)&pObj->m_rBoundsX, sizeof(Metric));
	Ctx.pStream->Read((char *)&pObj->m_rBoundsY, sizeof(Metric));
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	pObj->m_wHighlightColor = LOWORD(dwLoad);
	pObj->m_iHighlightCountdown = HIWORD(dwLoad);
	pObj->m_ItemList.ReadFromStream(Ctx);

	/*
	 * XXX Instead of breaking backwards compatability, modify 16 bit color
	 * and promote to 32 bits.
	 */
	BYTE r,g,b;	/* 5,6,5 RGB */
	double color;
	color = (pObj->m_wHighlightColor & 0xF800) >> 11;
	r = (color / 0x1F) * 255.0;
	color = (pObj->m_wHighlightColor & 0x07E0) >> 5;
	g = (color / 0x3F) * 255.0;
	color = pObj->m_wHighlightColor & 0x001F;
	b = (color / 0x1F) * 255.0;
	pObj->m_wHighlightColor = CGImage::RGBColor(r, g, b);

	//	Load other stuff

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	pObj->m_iControlsFrozen = dwLoad;

	//	Load flags

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	pObj->m_fHookObjectDestruction =	((dwLoad & 0x00000001) ? true : false);
	pObj->m_fNoObjectDestructionNotify = ((dwLoad & 0x00000002) ? true : false);
	pObj->m_fCannotBeHit =				((dwLoad & 0x00000004) ? true : false);
	pObj->m_fSelected =					((dwLoad & 0x00000008) ? true : false);
	pObj->m_fInPOVLRS =					((dwLoad & 0x00000010) ? true : false);
	pObj->m_fCanBounce =				((dwLoad & 0x00000020) ? true : false);
	pObj->m_fIsBarrier =				((dwLoad & 0x00000040) ? true : false);
	pObj->m_fCannotMove =				((dwLoad & 0x00000080) ? true : false);
	pObj->m_fNoFriendlyFire =			((dwLoad & 0x00000100) ? true : false);
	pObj->m_fTimeStop =					((dwLoad & 0x00000200) ? true : false);
	pObj->m_fPlayerTarget =				((dwLoad & 0x00000400) ? true : false);
	pObj->m_fAutomatedWeapon =			((dwLoad & 0x00000800) ? true : false);
	pObj->m_fNoFriendlyTarget =			((dwLoad & 0x00001000) ? true : false);

	//	At this point, OnCreate has always been called--no need to save

	pObj->m_fOnCreateCalled = true;

	//	Load opaque data

	pObj->m_Data.ReadFromStream(Ctx);

	//	Load the effect list

	IEffectPainter *pEffect = CEffectCreator::CreatePainterFromStream(Ctx);
	while (pEffect)
		{
		int x, y, iTick;
		Ctx.pStream->Read((char *)&x, sizeof(DWORD));
		Ctx.pStream->Read((char *)&y, sizeof(DWORD));
		Ctx.pStream->Read((char *)&iTick, sizeof(DWORD));
		pObj->AddEffect(pEffect, x, y, iTick);

		pEffect = CEffectCreator::CreatePainterFromStream(Ctx);
		}

	//	Let the subclass read its part

	pObj->OnReadFromStream(Ctx);

	//	Done

	*retpObj = pObj;
	}

ALERROR CSpaceObject::CreateRandomItems (IItemGenerator *pItems)

//	CreateRandomItems
//
//	Creates items based on item table

	{
	if (pItems)
		{
		CItemListManipulator ItemList(GetItemList());
		pItems->AddItems(ItemList);
		InvalidateItemListAddRemove();
		}

	return NOERROR;
	}

ALERROR CSpaceObject::CreateRandomItems (CXMLElement *pItems)

//	CreateRandomItems
//
//	Creates items based on lookup

	{
	ALERROR error;

	//	Load the table

	SDesignLoadCtx Ctx;

	IItemGenerator *pGenerator;
	if (error = IItemGenerator::CreateFromXML(Ctx, pItems, &pGenerator))
		return error;

	if (error = pGenerator->OnDesignLoadComplete(Ctx))
		{
		CString sError = strPatternSubst(CONSTLIT("ERROR: Unable to create random items: %s\r\n"), Ctx.sError.GetASCIIZPointer());
		ReportCrashObj(&sError, this);
		kernelDebugLogMessage(sError.GetASCIIZPointer());
		ASSERT(false);
		return error;
		}

	//	Add the items

	CItemListManipulator ItemList(GetItemList());
	pGenerator->AddItems(ItemList);
	InvalidateItemListAddRemove();

	delete pGenerator;
	return NOERROR;
	}

CString CSpaceObject::DebugDescribe (CSpaceObject *pObj)

//	DebugDescribe
//
//	Describe object

	{
	if (pObj == NULL)
		return CONSTLIT("none");

	try
		{
		return strPatternSubst(CONSTLIT("%x %s (%s)"), (DWORD)pObj, pObj->GetName().GetASCIIZPointer(), pObj->GetObjClassName().GetASCIIZPointer());
		}
	catch (...)
		{
		}

	return strPatternSubst(CONSTLIT("%x [invalid]"), (DWORD)pObj);
	}

void CSpaceObject::Destroy (DestructionTypes iCause, CSpaceObject *pCause)

//	Destroy
//
//	Destroy this object

	{
	//	Give our descendants a chance to do something

	bool bResurrectPending = false;
	CSpaceObject *pWreck = NULL;
	OnDestroyed(iCause, pCause, &bResurrectPending, &pWreck);

	//	Destroy

	CSystem *pSystem = m_pSystem;

	m_pSystem = NULL;
	if (m_iIndex != -1)
		{
		pSystem->RemoveObject(m_iIndex, iCause, pCause, pWreck);
		m_iIndex = -1;

		//	If this was the player, remove ship variables

		if (IsPlayer())
			{
			CCodeChain &CC = g_pUniverse->GetCC();
			CC.DefineGlobal(CONSTLIT("gPlayer"), CC.CreateNil());
			CC.DefineGlobal(CONSTLIT("gPlayerShip"), CC.CreateNil());
			}

		//	Delete

		if (!bResurrectPending)
			delete this;
		}
	}

void CSpaceObject::EnterGate (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate)

//	EnterGate
//
//	Enter a stargate

	{
	int i;

	//	If we're going to the same system, then do nothing

	if (pDestNode && pDestNode->GetSystem() == m_pSystem)
		return;

	//	Tell all listeners that this object entered a stargate

	for (i = 0; i < m_pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = m_pSystem->GetObject(i);

		if (pObj && pObj != this)
			pObj->OnObjEnteredGate(this, pStargate);
		}

	//	Let the object do the appropriate thing when entering a gate
	//	Note: Objects rely on this happening after other objects
	//	are notified.

	GateHook(pDestNode, sDestEntryPoint, pStargate);
	}

void CSpaceObject::FireCustomEvent (const CString &sEvent, ICCItem **retpResult)

//	FireCustomEvent
//
//	Fires a named event and optionally returns result

	{
	CCodeChain &CC = g_pUniverse->GetCC();
	ICCItem *pCode;

	if (FindEventHandler(sEvent, &pCode))
		{
		ICCItem *pOldStation = CC.LookupGlobal(STR_G_SOURCE, &g_pUniverse);
		DefineGlobalSpaceObject(CC, STR_G_SOURCE, this);

		ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
		if (pResult->IsError())
			ReportEventError(sEvent, pResult);

		//	Either return the event result or discard it

		if (retpResult)
			*retpResult = pResult;
		else
			pResult->Discard(&CC);

		//	Restore variable

		CC.DefineGlobal(STR_G_SOURCE, pOldStation);
		pOldStation->Discard(&CC);
		}
	else
		{
		if (retpResult)
			*retpResult = CC.CreateNil();
		}
	}

void CSpaceObject::FireCustomItemEvent (const CString &sEvent, const CItem &Item, ICCItem **retpResult)

//	FireCustomItemEvent
//
//	Fires a named event to an item and optionally returns result

	{
	CCodeChain &CC = g_pUniverse->GetCC();
	ICCItem *pCode;

	if (Item.GetType()->FindEventHandler(sEvent, &pCode))
		{
		//	Define some globals

		ICCItem *pOldSource = CC.LookupGlobal(STR_G_SOURCE, &g_pUniverse);
		DefineGlobalSpaceObject(CC, STR_G_SOURCE, this);

		ICCItem *pOldItem = CC.LookupGlobal(STR_G_ITEM, &g_pUniverse);
		ICCItem *pItem = CreateListFromItem(CC, Item);
		CC.DefineGlobal(STR_G_ITEM, pItem);
		pItem->Discard(&CC);

		//	Run code

		ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
		if (pResult->IsError())
			ReportEventError(strPatternSubst(CONSTLIT("Item %x %s"), Item.GetType()->GetUNID(), sEvent), pResult);

		//	Either return the event result or discard it

		if (retpResult)
			*retpResult = pResult;
		else
			pResult->Discard(&CC);

		//	Restore variable

		CC.DefineGlobal(STR_G_SOURCE, pOldSource);
		pOldSource->Discard(&CC);

		CC.DefineGlobal(STR_G_ITEM, pOldItem);
		pOldItem->Discard(&CC);
		}
	else
		{
		if (retpResult)
			*retpResult = CC.CreateNil();
		}
	}

void CSpaceObject::FireGetExplosionType (SExplosionType *retExplosion)

//	FireGetExplosionType
//
//	Allows the object to compute the kind of explosion

	{
	ICCItem *pCode;

	if (FindEventHandler(GET_EXPLOSION_TYPE_EVENT, &pCode))
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		ICCItem *pOldSource = CC.LookupGlobal(STR_G_SOURCE, &g_pUniverse);
		DefineGlobalSpaceObject(CC, STR_G_SOURCE, this);

		ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
		if (pResult->IsError())
			ReportEventError(GET_EXPLOSION_TYPE_EVENT, pResult);

		//	Return UNID, bonus, and cause

		DWORD dwUNID;
		int iBonus;
		DestructionTypes iCause;

		//	If the result is a list, then we expect a list with the following values:
		//
		//	The UNID of the explosion
		//	The bonus
		//	The cause (e.g., "explosion" or "playerCreatedExplosion")

		if (pResult->IsNil())
			{
			dwUNID = 0;
			iBonus = 0;
			iCause = killedByExplosion;
			}
		else if (pResult->IsList())
			{
			dwUNID = (DWORD)pResult->GetElement(0)->GetIntegerValue();
			iBonus = pResult->GetElement(1)->GetIntegerValue();
			if (pResult->GetElement(2)->IsNil())
				iCause = killedByExplosion;
			else
				{
				iCause = ::GetDestructionCause(pResult->GetElement(2)->GetStringValue());
				if (iCause == killedNone)
					iCause = killedByExplosion;
				}
			}

		//	Otherwise, expect just an UNID
		else
			{
			dwUNID = (DWORD)pResult->GetIntegerValue();
			iBonus = 0;
			iCause = killedByExplosion;
			}

		//	Done

		pResult->Discard(&CC);

		//	Restore variable

		CC.DefineGlobal(STR_G_SOURCE, pOldSource);
		pOldSource->Discard(&CC);

		//	Return

		retExplosion->pDesc = (dwUNID ? g_pUniverse->FindWeaponFireDesc(dwUNID) : NULL);
		retExplosion->iBonus = iBonus;
		retExplosion->iCause = iCause;
		}
	else
		{
		retExplosion->pDesc = NULL;
		retExplosion->iBonus = 0;
		retExplosion->iCause = killedByExplosion;
		}
	}

void CSpaceObject::FireOnAttacked (SDamageCtx &Ctx)

//	FireOnAttacked
//
//	Fire OnAttacked event

	{
	ICCItem *pCode;

	if (FindEventHandler(ON_ATTACKED_EVENT, &pCode))
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		ICCItem *pOldSource = CC.LookupGlobal(STR_G_SOURCE, &g_pUniverse);
		DefineGlobalSpaceObject(CC, STR_G_SOURCE, this);
		DefineGlobalSpaceObject(CC, CONSTLIT("aAttacker"), Ctx.pAttacker);
		DefineGlobalVector(CC, CONSTLIT("aHitPos"), Ctx.vHitPos);
		CC.DefineGlobalInteger(CONSTLIT("aHitDir"), Ctx.iDirection);
		CC.DefineGlobalInteger(CONSTLIT("aDamageHP"), Ctx.iDamage);
		CC.DefineGlobalString(CONSTLIT("aDamageType"), GetDamageShortName(Ctx.Damage.GetDamageType()));

		ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
		if (pResult->IsError())
			ReportEventError(ON_ATTACKED_EVENT, pResult);

		//	Done

		pResult->Discard(&CC);

		//	Restore variable

		CC.DefineGlobal(STR_G_SOURCE, pOldSource);
		pOldSource->Discard(&CC);
		}
	}

void CSpaceObject::FireOnAttackedByPlayer (void)

//	FireOnAttackedByPlayer
//
//	Fire OnAttackedByPlayer event

	{
	ICCItem *pCode;

	if (FindEventHandler(ON_ATTACKED_BY_PLAYER_EVENT, &pCode))
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		ICCItem *pOldStation = CC.LookupGlobal(STR_G_SOURCE, &g_pUniverse);
		DefineGlobalSpaceObject(CC, STR_G_SOURCE, this);

		ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
		if (pResult->IsError())
			ReportEventError(ON_ATTACKED_BY_PLAYER_EVENT, pResult);
		pResult->Discard(&CC);

		//	Restore variable

		CC.DefineGlobal(STR_G_SOURCE, pOldStation);
		pOldStation->Discard(&CC);
		}
	}

void CSpaceObject::FireOnCreate (void)

//	FireOnCreate
//
//	Fire OnCreate event

	{
	ICCItem *pCode;

	if (!m_fOnCreateCalled 
			&& FindEventHandler(ON_CREATE_EVENT, &pCode))
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		ICCItem *pOldStation = CC.LookupGlobal(STR_G_SOURCE, &g_pUniverse);
		DefineGlobalSpaceObject(CC, STR_G_SOURCE, this);

		ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
		if (pResult->IsError())
			ReportEventError(ON_CREATE_EVENT, pResult);
		pResult->Discard(&CC);

		//	Restore variable

		CC.DefineGlobal(STR_G_SOURCE, pOldStation);
		pOldStation->Discard(&CC);

		//	Remember that we already called OnCreate. This is helpful in case we
		//	create an object inside another object's OnCreate

		m_fOnCreateCalled = true;
		}
	}

void CSpaceObject::FireOnDamage (SDamageCtx &Ctx)

//	FireOnDamage
//
//	Fire OnDamage event

	{
	ICCItem *pCode;

	if (FindEventHandler(ON_DAMAGE_EVENT, &pCode))
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		ICCItem *pOldSource = CC.LookupGlobal(STR_G_SOURCE, &g_pUniverse);
		DefineGlobalSpaceObject(CC, STR_G_SOURCE, this);
		DefineGlobalSpaceObject(CC, CONSTLIT("aAttacker"), Ctx.pAttacker);
		DefineGlobalVector(CC, CONSTLIT("aHitPos"), Ctx.vHitPos);
		CC.DefineGlobalInteger(CONSTLIT("aHitDir"), Ctx.iDirection);
		CC.DefineGlobalInteger(CONSTLIT("aDamageHP"), Ctx.iDamage);
		CC.DefineGlobalString(CONSTLIT("aDamageType"), GetDamageShortName(Ctx.Damage.GetDamageType()));

		ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
		if (pResult->IsError())
			ReportEventError(ON_DAMAGE_EVENT, pResult);

		//	Result is the amount of damage

		Ctx.iDamage = pResult->GetIntegerValue();
		pResult->Discard(&CC);

		//	Restore variable

		CC.DefineGlobal(STR_G_SOURCE, pOldSource);
		pOldSource->Discard(&CC);
		}
	}

void CSpaceObject::FireOnDestroy (const SDestroyCtx &Ctx)

//	FireOnDestroy
//
//	Fire OnDestroy event

	{
	ICCItem *pCode;

	if (FindEventHandler(ON_DESTROY_EVENT, &pCode))
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		ICCItem *pOldStation = CC.LookupGlobal(STR_G_SOURCE, &g_pUniverse);
		DefineGlobalSpaceObject(CC, STR_G_SOURCE, this);
		DefineGlobalSpaceObject(CC, CONSTLIT("aDestroyer"), Ctx.pDestroyer);
		DefineGlobalSpaceObject(CC, CONSTLIT("aWreckObj"), Ctx.pWreck);
		CC.DefineGlobalString(CONSTLIT("aDestroyReason"), GetDestructionName(Ctx.iCause));

		//	Run code

		ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
		if (pResult->IsError())
			ReportEventError(ON_DESTROY_EVENT, pResult);
		pResult->Discard(&CC);

		//	Restore variable

		CC.DefineGlobal(STR_G_SOURCE, pOldStation);
		pOldStation->Discard(&CC);
		}
	}

void CSpaceObject::FireOnEnteredGate (CSpaceObject *pGate)

//	FireOnEnteredGate
//
//	Fire event when this object has entered a gate

	{
	ICCItem *pCode;

	if (FindEventHandler(ON_ENTERED_GATE_EVENT, &pCode))
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		ICCItem *pOldStation = CC.LookupGlobal(STR_G_SOURCE, &g_pUniverse);
		DefineGlobalSpaceObject(CC, STR_G_SOURCE, this);
		DefineGlobalSpaceObject(CC, CONSTLIT("aGateObj"), pGate);

		//	Run code

		ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
		if (pResult->IsError())
			ReportEventError(ON_ENTERED_GATE_EVENT, pResult);
		pResult->Discard(&CC);

		//	Restore variable

		CC.DefineGlobal(STR_G_SOURCE, pOldStation);
		pOldStation->Discard(&CC);
		}
	}

void CSpaceObject::FireOnItemAIUpdate (void)

//	FireOnItemAIUpdate
//
//	Fires OnAIUpdate event for all items

	{
	if (!m_fItemEventsValid)
		InitItemEvents();

	m_ItemEvents.FireEvent(this, eventOnAIUpdate);
	}

void CSpaceObject::FireOnItemUpdate (void)

//	FireOnItemUpdate
//
//	Fires OnUpdate event for all items

	{
	if (!m_fItemEventsValid)
		InitItemEvents();

	m_ItemEvents.FireEvent(this, eventOnUpdate);
	}

void CSpaceObject::FireOnMining (const SDamageCtx &Ctx)

//	FireOnMining
//
//	Fire OnMining event

	{
	ICCItem *pCode;

	if (FindEventHandler(ON_MINING_EVENT, &pCode))
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		ICCItem *pOldStation = CC.LookupGlobal(STR_G_SOURCE, &g_pUniverse);
		DefineGlobalSpaceObject(CC, STR_G_SOURCE, this);
		DefineGlobalSpaceObject(CC, CONSTLIT("aMiner"), Ctx.pAttacker);
		DefineGlobalVector(CC, CONSTLIT("aMinePos"), Ctx.vHitPos);
		CC.DefineGlobalInteger(CONSTLIT("aMineDir"), Ctx.iDirection);
		CC.DefineGlobalInteger(CONSTLIT("aMineProbability"), Ctx.Damage.GetMiningAdj());
		CC.DefineGlobalInteger(CONSTLIT("aHP"), Ctx.iDamage);
		CC.DefineGlobalString(CONSTLIT("aDamageType"), GetDamageShortName(Ctx.Damage.GetDamageType()));

		ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
		if (pResult->IsError())
			ReportEventError(ON_MINING_EVENT, pResult);
		pResult->Discard(&CC);

		//	Restore variable

		CC.DefineGlobal(STR_G_SOURCE, pOldStation);
		pOldStation->Discard(&CC);
		}
	}

void CSpaceObject::FireOnObjDestroyed (const SDestroyCtx &Ctx)

//	FireOnObjDestroyed
//
//	Fire OnObjDestroyed event

	{
	ICCItem *pCode;

	if (FindEventHandler(ON_OBJ_DESTROYED_EVENT, &pCode))
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		ICCItem *pOldStation = CC.LookupGlobal(STR_G_SOURCE, &g_pUniverse);
		DefineGlobalSpaceObject(CC, STR_G_SOURCE, this);
		DefineGlobalSpaceObject(CC, CONSTLIT("aObjDestroyed"), Ctx.pObj);
		DefineGlobalSpaceObject(CC, CONSTLIT("aDestroyer"), Ctx.pDestroyer);
		DefineGlobalSpaceObject(CC, CONSTLIT("aWreckObj"), Ctx.pWreck);
		CC.DefineGlobalString(CONSTLIT("aDestroyReason"), GetDestructionName(Ctx.iCause));

		ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
		if (pResult->IsError())
			ReportEventError(ON_OBJ_DESTROYED_EVENT, pResult);
		pResult->Discard(&CC);

		//	Restore variable

		CC.DefineGlobal(STR_G_SOURCE, pOldStation);
		pOldStation->Discard(&CC);
		}
	}

void CSpaceObject::FireOnObjDocked (CSpaceObject *pObj, CSpaceObject *pDockTarget)

//	FireOnObjDocked
//
//	Fire OnObjDocked event

	{
	ICCItem *pCode;

	if (FindEventHandler(ON_OBJ_DOCKED_EVENT, &pCode))
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		ICCItem *pOldStation = CC.LookupGlobal(STR_G_SOURCE, &g_pUniverse);
		DefineGlobalSpaceObject(CC, STR_G_SOURCE, this);
		DefineGlobalSpaceObject(CC, CONSTLIT("aObjDocked"), pObj);
		DefineGlobalSpaceObject(CC, CONSTLIT("aDockTarget"), pDockTarget);

		ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
		if (pResult->IsError())
			ReportEventError(ON_OBJ_DOCKED_EVENT, pResult);
		pResult->Discard(&CC);

		//	Restore variable

		CC.DefineGlobal(STR_G_SOURCE, pOldStation);
		pOldStation->Discard(&CC);
		}
	}

void CSpaceObject::FireOnObjEnteredGate (CSpaceObject *pObj, CSpaceObject *pStargate)

//	FireOnObjEnteredGate
//
//	Fire OnObjEnteredGate event

	{
	ICCItem *pCode;

	if (FindEventHandler(ON_OBJ_ENTERED_GATE_EVENT, &pCode))
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		ICCItem *pOldStation = CC.LookupGlobal(STR_G_SOURCE, &g_pUniverse);
		DefineGlobalSpaceObject(CC, STR_G_SOURCE, this);
		DefineGlobalSpaceObject(CC, CONSTLIT("aObj"), pObj);
		DefineGlobalSpaceObject(CC, CONSTLIT("aGateObj"), pStargate);

		ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
		if (pResult->IsError())
			ReportEventError(ON_OBJ_ENTERED_GATE_EVENT, pResult);
		pResult->Discard(&CC);

		//	Restore variable

		CC.DefineGlobal(STR_G_SOURCE, pOldStation);
		pOldStation->Discard(&CC);
		}
	}

void CSpaceObject::FireOnObjJumped (CSpaceObject *pObj)

//	FireOnObjJumped
//
//	Fire OnObjJumped event

	{
	ICCItem *pCode;

	if (FindEventHandler(ON_OBJ_JUMPED_EVENT, &pCode))
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		ICCItem *pOldStation = CC.LookupGlobal(STR_G_SOURCE, &g_pUniverse);
		DefineGlobalSpaceObject(CC, STR_G_SOURCE, this);
		DefineGlobalSpaceObject(CC, CONSTLIT("aObj"), pObj);

		ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
		if (pResult->IsError())
			ReportEventError(ON_OBJ_JUMPED_EVENT, pResult);
		pResult->Discard(&CC);

		//	Restore variable

		CC.DefineGlobal(STR_G_SOURCE, pOldStation);
		pOldStation->Discard(&CC);
		}
	}

void CSpaceObject::FireOnObjReconned (CSpaceObject *pObj)

//	FireOnObjReconned
//
//	Fire OnObjReconned event

	{
	ICCItem *pCode;

	if (FindEventHandler(ON_OBJ_RECONNED_EVENT, &pCode))
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		ICCItem *pOldStation = CC.LookupGlobal(STR_G_SOURCE, &g_pUniverse);
		DefineGlobalSpaceObject(CC, STR_G_SOURCE, this);
		DefineGlobalSpaceObject(CC, CONSTLIT("aObj"), pObj);

		ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
		if (pResult->IsError())
			ReportEventError(ON_OBJ_RECONNED_EVENT, pResult);
		pResult->Discard(&CC);

		//	Restore variable

		CC.DefineGlobal(STR_G_SOURCE, pOldStation);
		pOldStation->Discard(&CC);
		}
	}

void CSpaceObject::FireOnOrderChanged (void)

//	FireOnOrderChanged
//
//	Fire OnOrderChanged event

	{
	ICCItem *pCode;

	if (FindEventHandler(ON_ORDER_CHANGED_EVENT, &pCode))
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		ICCItem *pOldStation = CC.LookupGlobal(STR_G_SOURCE, &g_pUniverse);
		DefineGlobalSpaceObject(CC, STR_G_SOURCE, this);

		ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
		if (pResult->IsError())
			ReportEventError(ON_ORDER_CHANGED_EVENT, pResult);
		pResult->Discard(&CC);

		//	Restore variable

		CC.DefineGlobal(STR_G_SOURCE, pOldStation);
		pOldStation->Discard(&CC);
		}
	}

void CSpaceObject::FireOnOrdersCompleted (void)

//	FireOnOrdersCompleted
//
//	Fire OnOrdersCompleted event

	{
	ICCItem *pCode;

	if (FindEventHandler(ON_ORDERS_COMPLETED_EVENT, &pCode))
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		ICCItem *pOldStation = CC.LookupGlobal(STR_G_SOURCE, &g_pUniverse);
		DefineGlobalSpaceObject(CC, STR_G_SOURCE, this);

		ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
		if (pResult->IsError())
			ReportEventError(ON_ORDERS_COMPLETED_EVENT, pResult);
		pResult->Discard(&CC);

		//	Restore variable

		CC.DefineGlobal(STR_G_SOURCE, pOldStation);
		pOldStation->Discard(&CC);
		}
	}

void CSpaceObject::GetBoundingRect (CVector *retvUR, CVector *retvLL)

//	GetBoundingRect
//
//	Returns the bounding RECT of the object image centered on the object position

	{
	CVector vDiag(m_rBoundsX, m_rBoundsY);
	*retvUR = m_vPos + vDiag;
	*retvLL = m_vPos - vDiag;
	}

int CSpaceObject::GetCommsMessageCount (void)

//	GetCommsMessageCount
//
//	Returns the number of messages that his object understands

	{
	CCommunicationsHandler *pHandler = GetCommsHandler();
	if (pHandler == NULL)
		return 0;
	else
		return pHandler->GetCount();
	}

int CSpaceObject::GetDataInteger (const CString &sAttrib) const

//	GetDataInteger
//
//	Get integer value

	{
	CCodeChain &CC = g_pUniverse->GetCC();

	CString sData = GetData(sAttrib);
	ICCItem *pResult = CC.Link(sData, 0, NULL);
	int iResult = pResult->GetIntegerValue();
	pResult->Discard(&CC);

	return iResult;
	}

Metric CSpaceObject::GetDetectionRange2 (int iPerception)

//	GetDetectionRange2
//
//	Returns the square of the range at which this object can be detected by
//	the given perception.

	{
	Metric rRange = RangeIndex2Range(GetDetectionRangeIndex(iPerception));
	return rRange * rRange;
	}

int CSpaceObject::GetDetectionRangeIndex (int iPerception)

//	GetDetectionRangeIndex
//
//	Returns the range index at which this object can be detected by
//	the given perception.
//
//	0 = 500 light-seconds
//	1 = 340 light-seconds
//	2 = 225 light-seconds
//	3 = 150 light-seconds
//	4 = 100 light-seconds
//	5 = 50 light-seconds
//	6 = 25 light-seconds
//	7 = 13 light-seconds
//	8 = 6 light-seconds

	{
	int iStealth = GetStealth();
	int iResult = (iStealth - iPerception) + 4;

	//	We are easily visible at any range

	if (iResult <= 0)
		return 0;

	//	If we are not cloaked, then we are at least visible at SRS range

	else if (iStealth != stealthMax)
		return Min(iResult, VISUAL_RANGE_INDEX);

	//	Otherwise, we could be invisible

	else
		return Min(iResult, RANGE_INDEX_COUNT - 1);
	}

CSovereign::Disposition CSpaceObject::GetDispositionTowards (CSpaceObject *pObj)

//	GetDispositionTowards
//
//	Returns the disposition of this objects towards the given object

	{
	CSovereign *pOurSovereign = GetSovereign();
	if (pOurSovereign)
		return pOurSovereign->GetDispositionTowards(pObj->GetSovereign());
	else
		return CSovereign::dispFriend;
	}

const CObjectImageArray &CSpaceObject::GetImage (void)

//	GetImage
//
//	Returns the image for the object

	{
	static CObjectImageArray NullImage;
	return NullImage;
	}

CSpaceObject *CSpaceObject::GetNearestEnemy (Metric rMaxRange, bool bIncludeStations)

//	GetNearest
//
//	Returns the nearest enemy ship or station

	{
	int i;

	//	Get the sovereign

	CSovereign *pSovereign = GetSovereignToDefend();
	if (pSovereign == NULL || GetSystem() == NULL)
		return NULL;

	//	Get the list of enemy objects

	const CSpaceObjectList &ObjList = pSovereign->GetEnemyObjectList(GetSystem());

	//	Start a max range

	Metric rBestDist = rMaxRange * rMaxRange;
	CSpaceObject *pBestObj = NULL;

	//	Loop for all enemy objects

	int iCount = ObjList.GetCount();
	for (i = 0; i < iCount; i++)
		{
		CSpaceObject *pObj = ObjList.GetObj(i);

		if ((pObj->GetCategory() == catShip
					|| (bIncludeStations && pObj->GetCategory() == catStation))
				&& pObj->CanAttack())
			{
			CVector vDist = GetPos() - pObj->GetPos();
			Metric rDist = vDist.Length2();

			if (rDist < rBestDist
					&& !pObj->IsEscortingFriendOf(this))
				{
				rBestDist = rDist;
				pBestObj = pObj;
				}
			}
		}

	return pBestObj;
	}

CSpaceObject *CSpaceObject::GetNearestEnemyStation (Metric rMaxRange)

//	GetNearestEnemyStation
//
//	Returns the nearest enemy station that could threaten the object

	{
	int i;

	//	Get the sovereign

	CSovereign *pSovereign = GetSovereign();
	if (pSovereign == NULL || GetSystem() == NULL)
		return NULL;

	//	Get the list of enemy objects

	const CSpaceObjectList &ObjList = pSovereign->GetEnemyObjectList(GetSystem());

	//	Start a max range

	Metric rBestDist = rMaxRange * rMaxRange;
	CSpaceObject *pBestObj = NULL;

	//	Loop for all enemy objects

	int iCount = ObjList.GetCount();
	for (i = 0; i < iCount; i++)
		{
		CSpaceObject *pObj = ObjList.GetObj(i);

		if ((pObj->GetCategory() == catStation)
				&& pObj->CanAttack())
			{
			CVector vDist = GetPos() - pObj->GetPos();
			Metric rDist = vDist.Length2();

			if (rDist < rBestDist)
				{
				rBestDist = rDist;
				pBestObj = pObj;
				}
			}
		}

	return pBestObj;
	}

CSpaceObject *CSpaceObject::GetNearestStargate (bool bExcludeUncharted)

//	GetNearestStargate
//
//	Returns the nearest stargate

	{
	int i;
	Metric rNearestGateDist2 = (g_InfiniteDistance * g_InfiniteDistance);
	CSpaceObject *pNearestGate = NULL;
	for (i = 0; i < GetSystem()->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetSystem()->GetObject(i);

		if (pObj 
				&& pObj->IsStargate()
				//	Do not include uncharted stargates (such as Huaramarca gate)
				&& (!bExcludeUncharted || !pObj->HasAttribute(STR_UNCHARTED)))
			{
			Metric rDist2 = (GetPos() - pObj->GetPos()).Length2();
			if (rDist2 < rNearestGateDist2)
				{
				rNearestGateDist2 = rDist2;
				pNearestGate = pObj;
				}
			}
		}

	return pNearestGate;
	}

int CSpaceObject::GetNearestVisibleEnemies (int iMaxEnemies, Metric rMaxDist, CIntArray *pretList, CSpaceObject *pExcludeObj)

//	GetNearestVisibleEnemies
//
//	Returns a list of the nearest n enemies visible to this object

	{
	int i;
	Metric rWorstDist2 = rMaxDist * rMaxDist;

	//	Get the sovereign

	CSovereign *pSovereign = GetSovereignToDefend();
	if (pSovereign == NULL || GetSystem() == NULL)
		return 0;

	//	Get the list of enemy objects

	const CSpaceObjectList &ObjList = pSovereign->GetEnemyObjectList(GetSystem());

	//	Compute this object's perception and perception range

	int iPerception = GetPerception();
	Metric rRange2[RANGE_INDEX_COUNT];
	for (i = 0; i < RANGE_INDEX_COUNT; i++)
		{
		rRange2[i] = RangeIndex2Range(i);
		rRange2[i] = rRange2[i] * rRange2[i];
		}

	//	Allocate an array large enough

	struct Entry
		{
		CSpaceObject *pObj;
		Metric rDist2;
		};
	Entry *pList = new Entry[iMaxEnemies];
	int iCount = 0;

	//	Loop over all enemies

	int iObjCount = ObjList.GetCount();
	for (i = 0; 
			i < iObjCount && iCount < iMaxEnemies; 
			i++)
		{
		CSpaceObject *pObj = ObjList.GetObj(i);

		if (pObj->GetCategory() == catShip
				&& pObj->CanAttack())
			{
			CVector vDist = GetPos() - pObj->GetPos();
			Metric rDist2 = vDist.Length2();

			if (rDist2 < rWorstDist2
					&& rDist2 < rRange2[pObj->GetDetectionRangeIndex(iPerception)]
					&& pObj != pExcludeObj
					&& !pObj->IsEscortingFriendOf(this))
				{
				int iPos = 0;

				//	Figure out where to insert (in sorted order
				//	by ascending distance)

				for (iPos = 0; iPos < iCount; iPos++)
					if (rDist2 < pList[iPos].rDist2)
						break;

				//	Add the entry in sorted order

				for (int j = iCount - 1; j >= iPos; j--)
					pList[j + 1] = pList[j];

				pList[iPos].rDist2 = rDist2;
				pList[iPos].pObj = pObj;

				//	Increment count

				iCount++;

				//	New worst dist

				rWorstDist2 = pList[iCount-1].rDist2;
				}
			}
		}

	//	Add each of the entries in the array to the
	//	output

	for (i = 0; i < iCount; i++)
		pretList->AppendElement((int)pList[i].pObj, NULL);

	//	Done with list

	delete [] pList;

	//	Return the number of enemies found

	return iCount;
	}

CSpaceObject *CSpaceObject::GetNearestVisibleEnemy (Metric rMaxRange, bool bIncludeStations, CSpaceObject *pExcludeObj)

//	GetNearestVisibleEnemy
//
//	Returns the nearest enemy that is visible to this object

	{
	int i;

	//	Get the sovereign

	CSovereign *pSovereign = GetSovereignToDefend();
	if (pSovereign == NULL || GetSystem() == NULL)
		return 0;

	//	Get the list of enemy objects

	const CSpaceObjectList &ObjList = pSovereign->GetEnemyObjectList(GetSystem());

	//	Compute this object's perception and perception range

	int iPerception = GetPerception();
	Metric rRange2[RANGE_INDEX_COUNT];
	for (i = 0; i < RANGE_INDEX_COUNT; i++)
		{
		rRange2[i] = RangeIndex2Range(i);
		rRange2[i] = rRange2[i] * rRange2[i];
		}

	//	Loop over all objects finding the nearest visible enemy

	CSpaceObject *pBestObj = NULL;
	Metric rBestDist2 = rRange2[0];

	//	If the caller has specified a max range, then use that

	Metric rMaxRange2 = rMaxRange * rMaxRange;
	if (rMaxRange2 < rBestDist2)
		rBestDist2 = rMaxRange2;

	int iObjCount = ObjList.GetCount();
	for (i = 0; i < iObjCount; i++)
		{
		CSpaceObject *pObj = ObjList.GetObj(i);

		if ((pObj->GetCategory() == catShip
					|| (bIncludeStations && pObj->GetCategory() == catStation))
				&& pObj->CanAttack())
			{
			CVector vDist = GetPos() - pObj->GetPos();
			Metric rDist2 = vDist.Length2();

			if (rDist2 < rBestDist2
					&& rDist2 < rRange2[pObj->GetDetectionRangeIndex(iPerception)]
					&& pObj != pExcludeObj
					&& !pObj->IsEscortingFriendOf(this))
				{
				rBestDist2 = rDist2;
				pBestObj = pObj;
				}
			}
		}

	return pBestObj;
	}

CString CSpaceObject::GetNounPhrase (DWORD dwFlags)

//	GetNounPhrase
//
//	Get the name of the object as a noun phrase modified by the
//	given flags

	{
	//	Get the name and modifiers from the actual object

	DWORD dwNounFlags;
	CString sName = GetName(&dwNounFlags);

	return ComposeNounPhrase(sName, 1, CString(), dwNounFlags, dwFlags);
	}

CSpaceObject *CSpaceObject::GetOrderGiver (DestructionTypes iCause)

//	GetOrderGiver
//
//	Returns the object that is responsible for this object's attack

	{
	if (iCause == killedByPlayerCreatedExplosion)
		{
		CSpaceObject *pPlayerShip = g_pUniverse->GetPlayer();
		if (pPlayerShip)
			return pPlayerShip;
		else
			return this;
		}
	else
		return OnGetOrderGiver();
	}

CXMLElement *CSpaceObject::GetScreen (const CString &sName)

//	GetScreen
//
//	Returns a screen object

	{
	CDockScreenTypeRef Screen;
	Screen.LoadUNID(sName);
	Screen.Bind(NULL);
	return Screen.GetDesc();
	}

CSovereign *CSpaceObject::GetSovereignToDefend (void) const

//	GetSovereignToDefend
//
//	Returns either our sovereign or the sovereign of the ship that we're
//	escorting (if we're escorting)

	{
	CSpaceObject *pPrincipal = GetEscortPrincipal();
	if (pPrincipal)
		return pPrincipal->GetSovereign();
	else
		return GetSovereign();
	}

bool CSpaceObject::HasFuelItem (void)

//	HasFuelItem
//
//	Returns TRUE if the object has fuel on board

	{
	CItemListManipulator Search(GetItemList());
	while (Search.MoveCursorForward())
		{
		const CItem &Item = Search.GetItemAtCursor();
		if (Item.GetType()->GetCategory() == itemcatFuel)
			return true;
		}

	return false;
	}

CSpaceObject *CSpaceObject::HitTest (const CVector &vStart, 
									 Metric rThreshold, 
									 const DamageDesc &Damage, 
									 CVector *retvHitPos, 
									 int *retiHitDir)

//	HitTest
//
//	Returns the object that the beam hit or NULL if no object was hit.
//	If rThreshold > 0 and the object passes within the threshold distance
//	to some target, then retiHitDir = -1 and retvHitPos is the nearest point.

	{
	const int iSteps = 25;
	const int iMaxList = 1024;
	int iShortListCount = 0;
	CSpaceObject *pShortList[iMaxList];

	Metric iOffset = 128.0 * g_KlicksPerPixel;

	//	Calculate a bounding rectangle for the beam; we use this as
	//	a first cut to see if anything is near it.

	CVector vUR, vLL;
	if (vStart.GetX() > GetPos().GetX())
		{
		vUR.SetX(vStart.GetX() + iOffset);
		vLL.SetX(GetPos().GetX() - iOffset);
		}
	else
		{
		vUR.SetX(GetPos().GetX() + iOffset);
		vLL.SetX(vStart.GetX() - iOffset);
		}

	if (vStart.GetY() > GetPos().GetY())
		{
		vUR.SetY(vStart.GetY() + iOffset);
		vLL.SetY(GetPos().GetY() - iOffset);
		}
	else
		{
		vUR.SetY(GetPos().GetY() + iOffset);
		vLL.SetY(vStart.GetY() - iOffset);
		}

	//	Figure out if we need to compute a proximity hit

	bool bCalcProximity = (rThreshold > 0.0);

	//	Get the interaction of this object

	int iInteraction = GetInteraction();

	//	See if the beam hit anything. We start with a crude first pass.
	//	Any objects near the beam are then analyzed further to see if
	//	the beam hit them.

	int i;
	for (i = 0; i < GetSystem()->GetObjectCount() && iShortListCount < iMaxList; i++)
		{
		CSpaceObject *pObj = GetSystem()->GetObject(i);

		//	If the object is in the bounding box then remember
		//	it so that we can do a more accurate calculation.

		if (pObj
				&& pObj->CanBeHit() 
				&& pObj->InBox(vUR, vLL)
				&& CanHit(pObj) 
				&& pObj->CanBeHitBy(Damage)
				&& pObj != this)
			{
			if (iInteraction < 100)
				{
				//	Compute interaction

				int iInteractChance = Max(iInteraction, pObj->GetInteraction());
				if (iInteractChance == 100 ||
						(iInteractChance > 0 && mathRandom(1, 100) <= iInteractChance))
					pShortList[iShortListCount++] = pObj;
				}
			else
				pShortList[iShortListCount++] = pObj;
			}
		}

	//	If this happens then we somehow found more objects that
	//	we expected in the short list.

	ASSERT(iShortListCount < iMaxList);

	//	Step the object from the start to the current position to see
	//	if it hit any of the objects in the short list.

	if (iShortListCount > 0)
		{
		CVector vTest = vStart;
		CVector vStep = (GetPos() - vStart) / (Metric)iSteps;

		Metric rClosestApproach2 = rThreshold * rThreshold;
		CVector vClosestPos;
		CSpaceObject *pClosestHit = NULL;

		for (i = 0; i < iSteps; i++)
			{
			int j;
			for (j = 0; j < iShortListCount; j++)
				{
				if (pShortList[j]->PointInObject(vTest))
					{
					if (retvHitPos)
						*retvHitPos = vTest;

					//	Figure out the direction that the hit came from

					if (retiHitDir)
						*retiHitDir = VectorToPolar(-vStep, NULL);

					return pShortList[j];
					}

				//	Calculate proximity

				if (bCalcProximity
						&& (pShortList[j]->GetScale() == scaleShip
							|| pShortList[j]->GetScale() == scaleStructure)
						&& IsEnemy(pShortList[j]))
					{
					CVector vDist = vTest - pShortList[j]->GetPos();
					Metric rDist2 = vDist.Length2();

					if (rDist2 < rClosestApproach2)
						{
						rClosestApproach2 = rDist2;
						vClosestPos = vTest;
						pClosestHit = pShortList[j];
						}
					}
				}

			//	Next

			vTest = vTest + vStep;
			}

		//	If we got inside the threshold radius for some object
		//	check to see if we are now farther away. If so, then we
		//	reached the closest point.

		if (bCalcProximity && pClosestHit)
			{
			CVector vDist = vTest - pClosestHit->GetPos();
			Metric rDist2 = vDist.Length2();

			if (rDist2 > rClosestApproach2)
				{
				if (retvHitPos)
					*retvHitPos = vClosestPos;

				if (retiHitDir)
					*retiHitDir = -1;

				return pClosestHit;
				}
			}
		}

	return NULL;
	}

bool CSpaceObject::ImagesIntersect (const CObjectImageArray &Image1, int iTick1, int iRotation1, const CVector &vPos1,
									const CObjectImageArray &Image2, int iTick2, int iRotation2, const CVector &vPos2)

//	ImagesIntersect
//
//	Returns TRUE if the two images intersect

	{
	//	Compute the offset of Image2 relative to Image1 in pixels

	CVector vOffset = vPos2 - vPos1;
	int x = (int)((vOffset.GetX() / g_KlicksPerPixel) + 0.5);
	int y = -(int)((vOffset.GetY() / g_KlicksPerPixel) + 0.5);
	
	//	Images intersect

	return Image1.ImagesIntersect(iTick1, iRotation1, x, y, Image2, iTick2, iRotation2);
	}

bool CSpaceObject::InBarrier (const CVector &vPos)

//	InBarrier
//
//	Returns TRUE if the given position is in a barrier

	{
	int i;

	//	Compute the bounding rect for this object

	CVector vUR, vLL;
	GetBoundingRect(&vUR, &vLL);

	//	Loop over all other objects and see if we bounce off

	for (i = 0; i < m_pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pBarrier = m_pSystem->GetObject(i);
		if (pBarrier 
				&& pBarrier->m_fIsBarrier 
				&& pBarrier != this 
				&& pBarrier->CanBlock(this))
			{
			//	Compute the bounding rect for the barrier.

			CVector vBarrierUR, vBarrierLL;
			pBarrier->GetBoundingRect(&vBarrierUR, &vBarrierLL);

			//	If we intersect then block

			if (IntersectRect(vUR, vLL, vBarrierUR, vBarrierLL)
					&& pBarrier->ObjectInObject(this))
				return true;
			}
		}

	return false;
	}

bool CSpaceObject::IsEnemyInRange (Metric rMaxRange, bool bIncludeStations)

//	IsEnemyInRange
//
//	Returns TRUE if there is an enemy in the range

	{
	int i;

	//	Get the sovereign

	CSovereign *pSovereign = GetSovereignToDefend();
	if (pSovereign == NULL || GetSystem() == NULL)
		return false;

	//	Get the list of enemy objects

	const CSpaceObjectList &ObjList = pSovereign->GetEnemyObjectList(GetSystem());

	//	Start a max range

	Metric rBestDist = rMaxRange * rMaxRange;
	CSpaceObject *pBestObj = NULL;

	//	Loop for all enemy objects

	int iCount = ObjList.GetCount();
	for (i = 0; i < iCount; i++)
		{
		CSpaceObject *pObj = ObjList.GetObj(i);

		if ((pObj->GetCategory() == catShip
					|| (bIncludeStations && pObj->GetCategory() == catStation))
				&& pObj->CanAttack())
			{
			CVector vDist = GetPos() - pObj->GetPos();
			Metric rDist = vDist.Length2();

			if (rDist < rBestDist
					&& !pObj->IsEscortingFriendOf(this))
				return true;
			}
		}

	return false;
	}

bool CSpaceObject::IsEscortingFriendOf (const CSpaceObject *pObj) const

//	IsEscortingFriendOf
//
//	Returns TRUE if we're escorting a friend of pObj

	{
	CSpaceObject *pPrincipal = GetEscortPrincipal();
	if (pPrincipal)
		return pObj->IsFriend(pPrincipal);
	else
		return false;
	}

bool CSpaceObject::IsStargateInRange (Metric rMaxRange)

//	IsStargateInRange
//
//	Returns TRUE if stargate is in range

	{
	int i;
	Metric rNearestGateDist2 = (rMaxRange * rMaxRange);
	for (i = 0; i < GetSystem()->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetSystem()->GetObject(i);

		if (pObj && pObj->IsStargate())
			{
			Metric rDist2 = (GetPos() - pObj->GetPos()).Length2();
			if (rDist2 < rNearestGateDist2)
				return true;
			}
		}

	return false;
	}

bool CSpaceObject::IsUnderAttack (void)

//	IsUnderAttack
//
//	Returns TRUE if the space object is being attacked
//	by another object.

	{
	//	Iterate over all objects to see who has this object as a target

	int i;
	for (i = 0; i < GetSystem()->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetSystem()->GetObject(i);

		if (pObj
				&& pObj->GetTarget() == this
				&& IsEnemy(pObj)
				&& pObj != this)
			return true;
		}

	return false;
	}

bool CSpaceObject::IsCommsMessageValidFrom (CSpaceObject *pSender, int iIndex, CString *retsMsg, CString *retsKey)

//	IsCommsMessageValidFrom
//
//	Returns TRUE if the given object can send the given comms message to
//	this object

	{
	CCommunicationsHandler *pHandler = GetCommsHandler();
	ASSERT(pHandler && iIndex < pHandler->GetCount());
	const CCommunicationsHandler::SMessage &Msg = pHandler->GetMessage(iIndex);

	//	Init name

	if (retsMsg)
		*retsMsg = Msg.sMessage;

	if (retsKey)
		*retsKey = Msg.sShortcut;

	//	If we have an OnShow code block then see if it evaluates to TRUE

	if (Msg.pOnShow)
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		//	Define parameters

		DefineGlobalSpaceObject(CC, CONSTLIT("gSource"), this);
		DefineGlobalSpaceObject(CC, CONSTLIT("gSender"), pSender);

		//	Execute

		bool bShow;

		ICCItem *pResult = CC.TopLevel(Msg.pOnShow, &g_pUniverse);

		if (pResult->IsNil())
			bShow = false;
		else if (pResult->IsError())
			{
			pSender->SendMessage(this, pResult->GetStringValue());
			bShow = false;
			}
		else
			bShow = true;

		pResult->Discard(&CC);
		return bShow;
		}

	return true;
	}

bool CSpaceObject::IsCovering (CSpaceObject *pObj)

//	IsCovering
//
//	Returns TRUE if this object is covering the given object. That is,
//	we return TRUE if pObj is on top of (or below) this object and
//	this object is bigger than pObj

	{
	if (this != pObj)
		{
		CVector vThisUR;
		CVector vThisLL;

		GetBoundingRect(&vThisUR, &vThisLL);
		return pObj->PosInBox(vThisUR, vThisLL);
		}
	else
		return false;
	}

bool CSpaceObject::IsDestinyTime (int iCycle)

//	IsDestinyTime
//
//	Returns TRUE if the space object's destiny aligns with
//	the given cycle. A cycle of n aligns with a space object
//	once every n ticks. Each object aligns at different times
//	depending on its destiny.
 
	{
	int iOffset = (iCycle * GetDestiny()) / g_DestinyRange;
	return (((GetSystem()->GetTick() + iOffset) % iCycle) == 0);
	}

bool CSpaceObject::IsEnemy (const CSpaceObject *pObj) const

//	IsEnemy
//
//	Returns TRUE if the given object is our enemy

	{
	CSovereign *pOurSovereign = GetSovereign();
	CSovereign *pEnemySovereign = pObj->GetSovereign();

	if (pOurSovereign == pEnemySovereign
			|| pOurSovereign == NULL
			|| pEnemySovereign == NULL)
		return false;
	else
		return pOurSovereign->IsEnemy(pEnemySovereign);
	}

bool CSpaceObject::IsFriend (const CSpaceObject *pObj) const

//	IsFriend
//
//	Returns TRUE if the given object is our friend. Note that this
//	is not equal to !IsEnemy. It is also possible for an object to
//	be "neutral"

	{
	CSovereign *pOurSovereign = GetSovereign();
	CSovereign *pEnemySovereign = pObj->GetSovereign();

	if (pOurSovereign == pEnemySovereign)
		return true;
	else if (pOurSovereign == NULL || pEnemySovereign == NULL)
		return false;
	else
		return pOurSovereign->IsFriend(pEnemySovereign);
	}

bool CSpaceObject::IsLineOfFireClear (const CVector &vSource, 
									  CSpaceObject *pTarget, 
									  int iAngle, 
									  Metric rDistance, 
									  bool bAreaWeapon)

//	IsLineOfFireClear
//
//	Returns TRUE if there are no friendly objects along the given
//	direction for the given distance

	{
	int i;

	//	If we can't hit friends, then this whole function is moot

	if (!CanHitFriends())
		return true;

	//	Compute position of target

	CVector vTarget = (pTarget ? pTarget->GetPos() : vSource);
	Metric rMaxDist2 = rDistance * rDistance;

	//	See if any friendly object is in the line of fire

	for (i = 0; i < GetSystem()->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetSystem()->GetObject(i);

		if (pObj 
				&& pObj->CanAttack()
				&& pObj->CanBeHit()
				//	Only check for ships, structures
				&& (pObj->GetScale() == scaleStructure 
					|| pObj->GetScale() == scaleShip)
				&& CanFireOn(pObj)
				&& pObj != this
				&& pObj != pTarget)
			{
			CSpaceObject::Categories iCategory = pObj->GetCategory();

			//	If this is an enemy and it is a ship, then it is OK 
			//	to hit it (we only hit stations if we're aiming for them).

			if (IsEnemy(pObj) && iCategory == catShip)
				continue;

			//	If the target is right on top of a station, then we
			//	cannot fire.

			if (iCategory == catStation)
				{
				//	Compute the distance of the object from us and from
				//	the target.

				CVector vDistFromTarget = pObj->GetPos() - vTarget;
				Metric rDistFromTarget2 = vDistFromTarget.Length2();

				if (rDistFromTarget2 < BOUNDS_CHECK_DIST2)
					{
					CVector vUR, vLL;
					pObj->GetBoundingRect(&vUR, &vLL);

					if (rDistFromTarget2 < 2.0 * vUR.Length2())
						return false;
					}
				}

			//	Skip if we're too far

			CVector vDist = pObj->GetPos() - vSource;
			if (vDist.Length2() > rMaxDist2)
				continue;

			//	Figure out the object's bearing relative to us

			Metric rDist;
			int iObjAngle = VectorToPolar(vDist, &rDist);
			if (rDist < g_Epsilon)
				return false;

			//	Figure out how big the object is from that distance

			int iHalfAngularSize;
			if (bAreaWeapon)
				iHalfAngularSize = 45;
			else
				{
				CVector vUR;
				CVector vLL;
				pObj->GetBoundingRect(&vUR, &vLL);
				Metric rSize = (vUR.GetX() - vLL.GetX());

				//	This is an approximated version of arctan

				iHalfAngularSize = (int)(20 * rSize / rDist);
				}

			//	See if it is in our line of fire

			if (AreAnglesAligned(iAngle, iObjAngle, iHalfAngularSize))
				return false;
			}
		}

	return true;
	}

void CSpaceObject::Jump (const CVector &vPos)

//	Jump
//
//	Object jumps to a different position in the system

	{
	int i;

	//	Create a gate effect at the old position

	CEffectCreator *pEffect = g_pUniverse->FindEffectType(g_StargateInUNID);
	if (pEffect)
		pEffect->CreateEffect(m_pSystem,
				NULL,
				GetPos(),
				NullVector);

	//	Move the ship to the new position

	Place(vPos);

	//	Create a gate effect at the destination

	pEffect = g_pUniverse->FindEffectType(g_StargateOutUNID);
	if (pEffect)
		pEffect->CreateEffect(m_pSystem,
				NULL,
				vPos,
				NullVector);

	//	Set the ship to hide while coming out of gate

	CShip *pShip = AsShip();
	if (pShip)
		pShip->SetInGate(NULL, 0);

	//	Tell others that this object has jumped

	for (i = 0; i < m_pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = m_pSystem->GetObject(i);

		if (pObj && pObj != this)
			pObj->OnObjJumped(this);
		}
	}

bool CSpaceObject::MatchesCriteria (const Criteria &Crit) const

//	MatchesCriteria
//
//	Returns TRUE if this object matches the criteria

	{
	int i;

	if (Crit.pSource == this)
		return false;

	if (!(Crit.dwCategories & GetCategory()))
		return false;

	if (Crit.bStructureScaleOnly 
			&& GetCategory() == CSpaceObject::catStation
			&& GetScale() != scaleStructure)
		return false;

	if (Crit.bFriendlyObjectsOnly 
			&& Crit.pSource->IsEnemy(this) 
			&& !IsEscortingFriendOf(Crit.pSource) 
			&& !Crit.pSource->IsEscortingFriendOf(this))
		return false;

	if (Crit.bEnemyObjectsOnly 
			&& (!Crit.pSource->IsEnemy(this) || IsEscortingFriendOf(Crit.pSource) || Crit.pSource->IsEscortingFriendOf(this)))
		return false;

	if (Crit.bActiveObjectsOnly && !CanAttack())
		return false;

	if (Crit.bStargatesOnly && !IsStargate())
		return false;

	if (!Crit.sStargateID.IsBlank() && !strEquals(Crit.sStargateID, GetStargateID()))
		return false;

	if (Crit.bManufacturedObjectsOnly 
			&& (GetScale() == scaleWorld || GetScale() == scaleStar))
		return false;

	if (Crit.bHomeBaseIsSource && GetBase() != Crit.pSource)
		return false;

	if (!Crit.sData.IsBlank() && GetData(Crit.sData).IsBlank())
		return false;

	//	Check required attributes

	for (i = 0; i < Crit.AttribsRequired.GetCount(); i++)
		if (!HasAttribute(Crit.AttribsRequired.GetStringValue(i)))
			return false;

	//	Check attributes not allowed

	for (i = 0; i < Crit.AttribsNotAllowed.GetCount(); i++)
		if (HasAttribute(Crit.AttribsNotAllowed.GetStringValue(i)))
			return false;

	//	Distance

	if ((Crit.bNearestOnly || Crit.bFarthestOnly) && !Crit.bSingleObj)
		{
		CVector vDist = GetPos() - Crit.pSource->GetPos();
		Metric rDist2 = vDist.Length2();

		if (Crit.bNearestOnly && rDist2 > (Crit.rMaxRadius * Crit.rMaxRadius))
			return false;
		if (Crit.bFarthestOnly && rDist2 < (Crit.rMinRadius * Crit.rMinRadius))
			return false;
		}

	if (Crit.bEscortingSource && GetEscortPrincipal() != Crit.pSource)
		return false;

	if (Crit.bDockedWithSource)
		{
		CSpaceObject *pObj = const_cast<CSpaceObject *>(this);
		if (Crit.pSource == NULL || !Crit.pSource->IsObjDocked(pObj))
			return false;
		}

	if (Crit.bGuardingSource)
		{
		CSpaceObject *pTarget = NULL;
		CSpaceObject *pObj = const_cast<CSpaceObject *>(this);
		CShip *pShip = pObj->AsShip();
		if (pShip == NULL 
				|| Crit.pSource == NULL
				|| pShip->GetController() == NULL 
				|| pShip->GetController()->GetCurrentOrderEx(&pTarget) != IShipController::orderGuard
				|| pTarget != Crit.pSource)
			return false;
		}

	return true;
	}

bool CSpaceObject::MissileCanHitObj (CSpaceObject *pObj, CSpaceObject *pSource, bool bCanDamageSource)

//	MissileCanHitObj
//
//	Return TRUE if this object (a missile) can hit the given object (and assuming
//	that the this object was fired by pSource).

	{
	//	If we have a source...

	if (pSource)
		{
		//	If we can damage our source, then we don't need to check further

		if (bCanDamageSource)
			return true;

		//	Otherwise, we can only hit if we're not hitting our source, etc.

		else
			return 
				//	We cannot hit the source of the beam...
				((pObj != pSource) 

				//	We cannot hit another beam/missile from the same source...
				&& (pObj->GetSource() != pSource)

				//	We cannot hit our friends (if our source can't)
				//	(NOTE: we check for sovereign as opposed to IsEnemy because
				//	it is faster. For our purposes, same sovereign is what we want).
				&& (pSource->CanHitFriends() || pSource->GetSovereign() != pObj->GetSovereign())
				
				//	We cannot hit if the object cannot be hit by friends
				&& (pObj->CanBeHitByFriends() || pSource->GetSovereign() != pObj->GetSovereign()));
		}

	//	If we don't have a source...

	else
		{
		//	If we don't have a primary source, then don't hit our secondary source either
		//	(For ship explosions, the secondary source is the wreck; the wreck cannot be the
		//	primary source or else the tombstone message will be wrong)

		if (pObj == GetSecondarySource())
			return false;

		//	If we are part of an explosion, then we cannot hit other parts of an explosion
		//	that also have no source. This is so that fragments from an explosion where the source
		//	got destroyed (i.e., pSource == NULL) do not hit each other.

		else
			return (pObj->GetSource()
					|| (GetDamageCauseType() != pObj->GetDamageCauseType())
					|| (GetDamageCauseType() != killedByExplosion && GetDamageCauseType() != killedByPlayerCreatedExplosion));
		}
	}

void CSpaceObject::Move (const CSpaceObjectList &Barriers, Metric rSeconds)

//	Move
//
//	Moves the object in a straight line based on its current
//	velocity

	{
	CVector vOldPos = m_vPos;

	if (!m_vVel.IsNull())
		{
		//	Move the object on a straight line along the velocity vector

		m_vPos = m_vNewPos;

		//	Check to see if we've bounced against some other object

		if (m_fCanBounce)
			{
			int i;

			//	Compute the bounding rect for this object

			CVector vUR, vLL;
			GetBoundingRect(&vUR, &vLL);

			//	Loop over all other objects and see if we bounce off

			bool bBlocked = false;
			int iBarrierCount = Barriers.GetCount();
			for (i = 0; i < iBarrierCount; i++)
				{
				CSpaceObject *pBarrier = Barriers.GetObj(i);
				if (pBarrier != this 
						&& pBarrier->CanBlock(this))
					{
					//	Compute the bounding rect for the barrier.

					CVector vBarrierUR, vBarrierLL;
					pBarrier->GetBoundingRect(&vBarrierUR, &vBarrierLL);

					//	If we intersect then block

					if (IntersectRect(vUR, vLL, vBarrierUR, vBarrierLL)
							&& pBarrier->ObjectInObject(this))
						{
						//	Compute the resulting velocities depending
						//	on whether the barrier moves or not

						if (pBarrier->CanMove())
							{
							//	For a head-on elastic collision where
							//	the second object has velocity 0, the equations are:
							//
							//		  (m1 - m2)
							//	v1' = --------- v1
							//		  (m1 + m2)
							//
							//		    2m1
							//	v2' = --------- v1
							//		  (m1 + m2)
							
							Metric rInvM1plusM2 = g_BounceCoefficient / (GetMass() + pBarrier->GetMass());
							Metric rM1minusM2 = GetMass() - pBarrier->GetMass();
							Metric r2M1 = 2.0 * GetMass();
							CVector vVel = GetVel();

							m_vPos = vOldPos;

							SetVel(rM1minusM2 * rInvM1plusM2 * vVel);
							pBarrier->SetVel(r2M1 * rInvM1plusM2 * vVel);
							}
						else
							{
							//	If we've already been blocked, then make sure that we are not inside
							//	the second barrier. If we are, then revert the position

							if (bBlocked)
								{
								if (pBarrier->PointInObject(m_vPos))
									m_vPos = vOldPos;
								}

							//	Otherwise, deal with the first barrier

							else
								{
								//	Revert the position to before the move

								m_vPos = vOldPos;

								//	If the old position is not blocked, then bounce and carry on

								if (!pBarrier->ObjectInObject(this))
									SetVel(-g_BounceCoefficient * GetVel());

								//	Otherwise, move slowly towards the new position, but make sure that we never
								//	move the center of the object inside the barrier.

								else
									{
									CVector vNewPos = m_vPos + (g_KlicksPerPixel * m_vVel.Normal());
									if (!pBarrier->PointInObject(vNewPos))
										m_vPos = vNewPos;

									ClipSpeed(0.01 * LIGHT_SPEED);
									}
								}
							}

						//	Tell the barrier

						pBarrier->OnObjBounce(this, m_vPos);

#if 0
						//	If this object changed orientation or shape this
						//	round then check to see if we are still blocked.
						//	If we are, then revert the orientation change.

						if (OrientationChanged())
							{
							if (pBarrier->ObjectInObject(this))
								RevertOrientationChange();
							}
#endif

						//	Remember that we already dealt with one barrier

						bBlocked = true;
						}
					}
				}
			}
		}

	//	Let descendents process the move (if necessary)

	OnMove(vOldPos, rSeconds);
	}

void CSpaceObject::OnObjDestroyed (const SDestroyCtx &Ctx)

//	OnObjDestroyed
//
//	Called whenever another object in the system is destroyed

	{
	//	Give our subclasses a chance to do something (note that we need
	//	to do this before we NULL-out the references because some objects
	//	need to check their references.

	if (IsObjectDestructionHooked())
		ObjectDestroyedHook(Ctx);

	//	NULL-out any references to the object

	m_Data.OnObjDestroyed(Ctx.pObj);
	}

void CSpaceObject::PaintEffects (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	PaintEffects
//
//	Paints the list of effects for this object

	{
	SEffectNode *pEffect = m_pFirstEffect;
	while (pEffect)
		{
		Ctx.iTick = pEffect->iTick++;
		Ctx.iVariant = 0;
		Ctx.iDestiny = GetDestiny();

		pEffect->pPainter->Paint(Dest, 
				x + pEffect->xOffset,
				y + pEffect->yOffset,
				Ctx);

		pEffect = pEffect->pNext;
		}
	}

void CSpaceObject::PaintHighlight (CG16bitImage &Dest, const RECT &rcRect)

//	PaintHighlight
//
//	Paints a highlight around the object

	{
	//	Figure out the color of the highlight

	COLORREF wColor;
	if (m_iHighlightCountdown)
		wColor = m_wHighlightColor;
	else
		wColor = CG16bitImage::RGBValue(255,0,0);

	//	Paint the corners

	Dest.DrawLine(rcRect.left, rcRect.top,
			rcRect.left + HIGHLIGHT_CORNER_WIDTH, rcRect.top,
			1, wColor);

	Dest.DrawLine(rcRect.left, rcRect.top,
			rcRect.left, rcRect.top + HIGHLIGHT_CORNER_HEIGHT,
			1, wColor);

	Dest.DrawLine(rcRect.right, rcRect.top,
			rcRect.right - HIGHLIGHT_CORNER_WIDTH, rcRect.top,
			1, wColor);

	Dest.DrawLine(rcRect.right, rcRect.top,
			rcRect.right, rcRect.top + HIGHLIGHT_CORNER_HEIGHT,
			1, wColor);

	Dest.DrawLine(rcRect.left, rcRect.bottom,
			rcRect.left, rcRect.bottom - HIGHLIGHT_CORNER_HEIGHT,
			1, wColor);

	Dest.DrawLine(rcRect.left, rcRect.bottom,
			rcRect.left + HIGHLIGHT_CORNER_WIDTH, rcRect.bottom,
			1, wColor);

	Dest.DrawLine(rcRect.right, rcRect.bottom,
			rcRect.right - HIGHLIGHT_CORNER_WIDTH, rcRect.bottom,
			1, wColor);

	Dest.DrawLine(rcRect.right, rcRect.bottom,
			rcRect.right, rcRect.bottom - HIGHLIGHT_CORNER_HEIGHT,
			1, wColor);
	}

void CSpaceObject::PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans)

//	PaintLRS
//
//	Paints the object on an LRS

	{
	Dest.DrawDot(x, y, 
			CG16bitImage::RGBValue(255, 255, 0), 
			CG16bitImage::markerSmallRound);
	}

void CSpaceObject::ParseCriteria (CSpaceObject *pSource, const CString &sCriteria, Criteria *retCriteria)

//	ParseCriteria
//
//	Parses a string and returns criteria structure
//
//		G			Stargates only
//		G:xyz;		Stargate with ID 'xyz'
//		s			Include ships
//		t			Include stations (including planets)
//		T			Include structure-scale stations
//		T:xyz;		Include stations with attribute 'xyz'
//
//		A			Active objects only (i.e., objects that can attack)
//		B:xyz;		Only objects with attribute 'xyz'
//		D:xyz;		Only objects with data 'xyz'
//		E			Enemy objects only
//		F			Friendly objects only
//		M			Manufactured objects only (i.e., no planets or asteroids)
//		N			Return only the nearest object to the source
//		N:nn;		Return only objects within nn light-seconds
//		O:docked;	Ships that are currently docked at source
//		O:escort;	Ships ordered to escort source
//		R:nn;		Return only objects greater than nn light-seconds away

	{
	CString sParam;

	//	Initialize

	retCriteria->pSource = pSource;
	retCriteria->dwCategories = 0;
	retCriteria->bActiveObjectsOnly = false;
	retCriteria->bFriendlyObjectsOnly = false;
	retCriteria->bEnemyObjectsOnly = false;
	retCriteria->bManufacturedObjectsOnly = false;
	retCriteria->bStructureScaleOnly = false;
	retCriteria->bStargatesOnly = false;
	retCriteria->bNearestOnly = false;
	retCriteria->bFarthestOnly = false;
	retCriteria->bEscortingSource = false;
	retCriteria->bGuardingSource = false;
	retCriteria->bDockedWithSource = false;
	retCriteria->bHomeBaseIsSource = false;
	retCriteria->rMinRadius = 0.0;
	retCriteria->rMaxRadius = g_InfiniteDistance;
	retCriteria->bSingleObj = false;

	//	Parse

	char *pPos = sCriteria.GetPointer();
	while (*pPos != '\0')
		{
		switch (*pPos)
			{
			case 'A':
				retCriteria->bActiveObjectsOnly = true;
				break;

			case 'B':
				retCriteria->AttribsRequired.AppendString(ParseParam(&pPos), NULL);
				break;

			case 'D':
				retCriteria->sData = ParseParam(&pPos);
				break;

			case 'E':
				retCriteria->bEnemyObjectsOnly = true;
				break;

			case 'F':
				retCriteria->bFriendlyObjectsOnly = true;
				break;

			case 'G':
				retCriteria->dwCategories |= CSpaceObject::catStation;
				retCriteria->sStargateID = ParseParam(&pPos);
				retCriteria->bStargatesOnly = true;
				break;

			case 'H':
				retCriteria->bHomeBaseIsSource = true;
				break;

			case 'M':
				retCriteria->bManufacturedObjectsOnly = true;
				break;

			case 'N':
				retCriteria->bNearestOnly = true;
				sParam = ParseParam(&pPos);
				if (sParam.IsBlank())
					retCriteria->bSingleObj = true;
				else
					retCriteria->rMaxRadius = LIGHT_SECOND * strToInt(sParam, 0, NULL);
				break;

			case 'O':
				{
				CString sAttrib = ParseParam(&pPos);
				if (strEquals(sAttrib, CONSTLIT("escort")))
					retCriteria->bEscortingSource = true;
				else if (strEquals(sAttrib, CONSTLIT("docked")))
					retCriteria->bDockedWithSource = true;
				else if (strEquals(sAttrib, CONSTLIT("guard")))
					retCriteria->bGuardingSource = true;
				break;
				}

			case 'R':
				retCriteria->bFarthestOnly = true;
				sParam = ParseParam(&pPos);
				if (sParam.IsBlank())
					retCriteria->bSingleObj = true;
				else
					retCriteria->rMinRadius = LIGHT_SECOND * strToInt(sParam, 0, NULL);
				break;

			case 's':
				retCriteria->dwCategories |= CSpaceObject::catShip;
				break;

			case 't':
				retCriteria->dwCategories |= CSpaceObject::catStation;
				retCriteria->AttribsRequired.AppendString(ParseParam(&pPos), NULL);
				break;

			case 'T':
				retCriteria->dwCategories |= CSpaceObject::catStation;
				retCriteria->AttribsRequired.AppendString(ParseParam(&pPos), NULL);
				retCriteria->bStructureScaleOnly = true;
				break;

			case '+':
			case '-':
				{
				char chChar = *pPos;
				pPos++;

				//	Get the modifier

				char *pStart = pPos;
				while (*pPos != '\0' && *pPos != ';')
					pPos++;

				CString sAttrib = CString(pStart, pPos - pStart);

				//	Required or Not Allowed

				if (chChar == '+')
					retCriteria->AttribsRequired.AppendString(sAttrib, NULL);
				else
					retCriteria->AttribsNotAllowed.AppendString(sAttrib, NULL);

				//	No trailing semi

				if (*pPos == '\0')
					pPos--;

				break;
				}
			}

		pPos++;
		}

	//	If we have no source, make sure we don't select anything that requires
	//	a source

	if (retCriteria->pSource == NULL)
		{
		retCriteria->bFriendlyObjectsOnly = false;
		retCriteria->bEnemyObjectsOnly = false;
		retCriteria->bHomeBaseIsSource = false;
		retCriteria->bNearestOnly = false;
		retCriteria->bFarthestOnly = false;
		retCriteria->bEscortingSource = false;
		retCriteria->bGuardingSource = false;
		retCriteria->bDockedWithSource = false;
		retCriteria->bSingleObj = false;
		}
	}

void CSpaceObject::Reconned (void)

//	Reconned
//
//	Tell all other objects that this object has been reconned
//	(used for missions)

	{
	int i;

	for (i = 0; i < m_pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = m_pSystem->GetObject(i);

		if (pObj)
			pObj->OnObjReconned(this);
		}
	}

void CSpaceObject::Remove (DestructionTypes iCause, CSpaceObject *pCause)

//	Remove
//
//	Removes this object from the system
//
//	pCause is the object that caused us to be removed (may be NULL)

	{
	CSystem *pSystem = m_pSystem;

	m_pSystem = NULL;
	if (m_iIndex != -1)
		{
		pSystem->RemoveObject(m_iIndex, iCause, pCause, NULL);
		m_iIndex = -1;
		}
	}

void CSpaceObject::ReportEventError (const CString &sEvent, ICCItem *pError)

//	ReportEventError
//
//	Report an error during an event

	{
	CString sError = strPatternSubst(CONSTLIT("%s [%s]: %s"), sEvent.GetASCIIZPointer(), GetName().GetASCIIZPointer(), pError->GetStringValue().GetASCIIZPointer());
	CSpaceObject *pPlayer = g_pUniverse->GetPlayer();
	if (pPlayer)
		pPlayer->SendMessage(this, sError);

	kernelDebugLogMessage(sError.GetASCIIZPointer());
	}

bool CSpaceObject::RequestGate (CSpaceObject *pObj)

//	RequestGate
//
//	Requests that the given object be transported through the gate

	{
	pObj->EnterGate(NULL, NULL_STR, this);
	return true;
	}

void CSpaceObject::SetDataInteger (const CString &sAttrib, int iValue)

//	SetDataInteger
//
//	Set integer value

	{
	CCodeChain &CC = g_pUniverse->GetCC();

	ICCItem *pValue = CC.CreateInteger(iValue);
	CString sData = CC.Unlink(pValue);
	pValue->Discard(&CC);

	SetData(sAttrib, sData);
	}

void CSpaceObject::Update (void)

//	Update
//
//	Update the object

	{
	//	Update the highlight

	if (m_iHighlightCountdown > 0)
		m_iHighlightCountdown--;

	//	Update the effects

	SEffectNode *pEffect = m_pFirstEffect;
	SEffectNode *pPrev = NULL;
	while (pEffect)
		{
		SEffectNode *pNext = pEffect->pNext;

		if (pEffect->iTick >= pEffect->pPainter->GetInitialLifetime())
			{
			if (pPrev)
				pPrev->pNext = pNext;
			else
				m_pFirstEffect = pNext;

			pEffect->pPainter->Delete();
			delete pEffect;
			}
		else
			{
			pEffect->pPainter->OnUpdate();

			pPrev = pEffect;
			}

		pEffect = pNext;
		}

	//	Update items

	if (IsDestinyTime(30))
		FireOnItemUpdate();

	//	Update the specific object

	OnUpdate(g_SecondsPerUpdate);
	}

void CSpaceObject::UseItem (CItem &Item, CString *retsError)

//	UseItem
//
//	Uses the given item

	{
	CCodeChain &CC = g_pUniverse->GetCC();

	//	If this item is a device and it is installed, get the
	//	installed device structure.

	CInstalledDevice *pDevice = NULL;
	int iActivationDelay = 0;
	if (Item.GetType()->IsDevice() && Item.IsInstalled())
		{
		pDevice = FindDevice(Item);
		ASSERT(pDevice);

		if (pDevice)
			{
			//	If the device is disabled, then we can't use it

			if (!pDevice->IsEnabled())
				{
				*retsError = strPatternSubst(CONSTLIT("%s not enabled"), Item.GetNounPhrase(nounCapitalize).GetASCIIZPointer());
				return;
				}

			//	If the device is not ready, then we can't use it

			if (!pDevice->IsReady())
				{
				*retsError = strPatternSubst(CONSTLIT("%s not yet recharged"), Item.GetNounPhrase(nounCapitalize).GetASCIIZPointer());
				return;
				}

			//	See if the device has an activation delay

			iActivationDelay = pDevice->GetActivateDelay(this);
			}
		}

	//	Define parameters

	DefineGlobalSpaceObject(CC, CONSTLIT("gSource"), this);

	ICCItem *pItem = CreateListFromItem(CC, Item);
	CC.DefineGlobal(CONSTLIT("gItem"), pItem);
	pItem->Discard(&CC);

	//	Execute

	ICCItem *pCode = Item.GetType()->GetUseCode();
	if (pCode)
		{
		ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
		if (retsError)
			{
			if (pResult->IsError())
				*retsError = pResult->GetStringValue();
			else
				*retsError = NULL_STR;
			}

		pResult->Discard(&CC);
		}

	//	Reset the activation delay, if necessary

	if (iActivationDelay)
		pDevice->SetActivationDelay(iActivationDelay);
	}

void CSpaceObject::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Write the object to a stream
//
//	DWORD		ObjID
//	DWORD		m_iIndex
//	DWORD		m_dwID
//	DWORD		m_iDestiny
//	Vector		m_vPos
//	Vector		m_vVel
//	Metric		m_rBoundsX
//	Metric		m_rBoundsY
//	DWORD		low = m_wHighlightColor; hi = m_iHighlightCountdown
//	CItemList	m_ItemList
//	DWORD		m_iControlsFrozen
//	DWORD		flags
//	CAttributeDataBlock	m_Data
//
//	For each effect:
//	IEffectPainter (0 == no more)
//	DWORD		x
//	DWORD		y
//	DWORD		iTick

	{
	//	Write out the Kernel object ID

	DWORD dwSave = (DWORD)GetClass()->GetObjID();

	/*
	 * XXX To avoid breaking backwards compatability, decompose m_wHighlightColor
	 * to a 16-bit WORD.
	 */
	WORD wColor;
	BYTE r, g, b;
	r = CGImage::RedColor(m_wHighlightColor);
	g = CGImage::GreenColor(m_wHighlightColor);
	b = CGImage::BlueColor(m_wHighlightColor);

	r = (r / 255.0) * 0x1F;
	g = (g / 255.0) * 0x3F;
	b = (b / 255.0) * 0x1F;

	wColor = (r << 11) | (g < 5) | b;

	//	Save out stuff

	pStream->Write((char *)&dwSave, sizeof(DWORD));
	pStream->Write((char *)&m_iIndex, sizeof(DWORD));
	pStream->Write((char *)&m_dwID, sizeof(DWORD));
	pStream->Write((char *)&m_iDestiny, sizeof(DWORD));
	pStream->Write((char *)&m_vPos, sizeof(m_vPos));
	pStream->Write((char *)&m_vVel, sizeof(m_vVel));
	pStream->Write((char *)&m_rBoundsX, sizeof(m_rBoundsX));
	pStream->Write((char *)&m_rBoundsY, sizeof(m_rBoundsY));
	dwSave = MAKELONG(wColor, m_iHighlightCountdown);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Write out the list of items

	m_ItemList.WriteToStream(pStream);

	//	More Data

	dwSave = m_iControlsFrozen;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Write out flags

	dwSave = 0;
	dwSave |= (m_fHookObjectDestruction		? 0x00000001 : 0);
	dwSave |= (m_fNoObjectDestructionNotify ? 0x00000002 : 0);
	dwSave |= (m_fCannotBeHit				? 0x00000004 : 0);
	dwSave |= (m_fSelected					? 0x00000008 : 0);
	dwSave |= (m_fInPOVLRS					? 0x00000010 : 0);
	dwSave |= (m_fCanBounce					? 0x00000020 : 0);
	dwSave |= (m_fIsBarrier					? 0x00000040 : 0);
	dwSave |= (m_fCannotMove				? 0x00000080 : 0);
	dwSave |= (m_fNoFriendlyFire			? 0x00000100 : 0);
	dwSave |= (m_fTimeStop					? 0x00000200 : 0);
	dwSave |= (m_fPlayerTarget				? 0x00000400 : 0);
	dwSave |= (m_fAutomatedWeapon			? 0x00000800 : 0);
	dwSave |= (m_fNoFriendlyTarget			? 0x00001000 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Write out the opaque data

	m_Data.WriteToStream(pStream, m_pSystem);

	//	Write out the effect list

	SEffectNode *pNext = m_pFirstEffect;
	while (pNext)
		{
		pNext->pPainter->WriteToStream(pStream);
		pStream->Write((char *)&pNext->xOffset, sizeof(DWORD));
		pStream->Write((char *)&pNext->yOffset, sizeof(DWORD));
		pStream->Write((char *)&pNext->iTick, sizeof(DWORD));

		pNext = pNext->pNext;
		}

	//	Effects list ends in a NULL

	CString sUNID = NULL_STR;
	sUNID.WriteToStream(pStream);

	//	Let the subclass write out its part

	OnWriteToStream(pStream);
	}

CString ParseParam (char **ioPos)
	{
	char *pPos = *ioPos;
	if (pPos[1] == ':')
		{
		pPos++;
		pPos++;

		char *pStart = pPos;
		while (*pPos != ';' && *pPos != '\0')
			pPos++;

		*ioPos = pPos;
		return CString(pStart, pPos - pStart);
		}
	else
		return NULL_STR;
	}

#ifdef DEBUG_VECTOR

void CSpaceObject::PaintDebugVector (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)
	{
	if (!m_vDebugVector.IsNull())
		{
		int xDest, yDest;

		Ctx.XForm.Transform(GetPos() + m_vDebugVector, &xDest, &yDest);

		Dest.DrawLine(x, y,
				xDest, yDest,
				3,
				CG16bitImage::RGBValue(0,255,0));
		}
	}

#endif

