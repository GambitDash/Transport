//	Events.cpp
//
//	Event classes

#include "PreComp.h"


void CTimedEvent::CreateFromStream (SLoadCtx &Ctx, CTimedEvent **retpEvent)

//	CreateFromStream
//
//	Creates a timed event from a stream

	{
	//	Create the appropriate class

	DWORD dwLoad;
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	CTimedEvent *pEvent;
	switch (dwLoad)
		{
		case cTimedEncounterEvent:
			pEvent = new CTimedEncounterEvent;
			break;

		case cTimedCustomEvent:
			pEvent = new CTimedCustomEvent;
			break;

		case cTimedRecurringEvent:
			pEvent = new CTimedRecurringEvent;
			break;
		}

	//	Load stuff

	Ctx.pStream->Read((char *)&pEvent->m_iTick, sizeof(DWORD));

	//	Load subclass data

	pEvent->OnReadFromStream(Ctx);

	//	Done

	*retpEvent = pEvent;
	}

void CTimedEvent::WriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	WriteToStream
//
//	Writes the event to a stream
//
//	DWORD		class
//	DWORD		m_iTick
//	Subclass data

	{
	OnWriteClassToStream(pStream);
	pStream->Write((char *)&m_iTick, sizeof(DWORD));
	OnWriteToStream(pSystem, pStream);
	}

//	CTimedEncounterEvent ------------------------------------------------------

CTimedEncounterEvent::CTimedEncounterEvent (int iTick,
											CSpaceObject *pTarget,
											DWORD dwEncounterTableUNID,
											CSpaceObject *pGate,
											Metric rDistance) :
		CTimedEvent(iTick),
		m_pTarget(pTarget),
		m_dwEncounterTableUNID(dwEncounterTableUNID),
		m_pGate(pGate),
		m_rDistance(rDistance)

//	CTimedEncounterEvent constructor

	{
	}

CString CTimedEncounterEvent::DebugCrashInfo (void)

//	DebugCrashInfo

	{
	CString sResult;

	sResult.Append(CONSTLIT("TimedEncounterEvent\r\n"));
	return sResult;
	}

void CTimedEncounterEvent::DoEvent (CSystem *pSystem, bool *retbDeleteEvent)

//	DoEvent
//
//	Do the actual event

	{
	CShipTable *pTable = g_pUniverse->FindEncounterTable(m_dwEncounterTableUNID);
	if (pTable == NULL)
		{
		*retbDeleteEvent = true;
		return;
		}

	SShipCreateCtx Ctx;
	Ctx.pSystem = pSystem;
	Ctx.pTarget = m_pTarget;
	Ctx.dwFlags = SShipCreateCtx::ATTACK_NEAREST_ENEMY;

	//	Figure out where the encounter will come from

	if (m_rDistance > 0.0)
		{
		if (m_pTarget)
			Ctx.vPos = m_pTarget->GetPos() + ::PolarToVector(mathRandom(0, 359), m_rDistance);
		Ctx.PosSpread = DiceRange(3, 1, 2);
		}
	else if (m_pGate && m_pGate->IsStargate())
		Ctx.pGate = m_pGate;
	else if (m_pGate)
		{
		Ctx.vPos = m_pGate->GetPos();
		Ctx.PosSpread = DiceRange(2, 1, 2);
		}
	else if (m_pTarget)
		Ctx.pGate = m_pTarget->GetNearestStargate(true);

	//	Create ships

	pTable->CreateShips(Ctx);

	//	Delete the event

	*retbDeleteEvent = true;
	}

bool CTimedEncounterEvent::OnObjDestroyed (CSpaceObject *pObj)

//	OnObjDestroyed
//
//	Returns TRUE if the event should be destroyed

	{
	return (m_pTarget == pObj);
	}

void CTimedEncounterEvent::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read data

	{
	Ctx.pStream->Read((char *)&m_dwEncounterTableUNID, sizeof(DWORD));
	Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_pTarget);
	Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_pGate);
	Ctx.pStream->Read((char *)&m_rDistance, sizeof(Metric));
	}

void CTimedEncounterEvent::OnWriteClassToStream (IWriteStream *pStream)

//	OnWriteClassToStream
//
//	Writes the class

	{
	DWORD dwSave = cTimedEncounterEvent;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

void CTimedEncounterEvent::OnWriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	OnWriteToStream
//
//	Writes the obj data to stream
//
//	DWORD		m_dwEncounterTableUNID
//	DWORD		m_pTarget (CSpaceObject ref)
//	DWORD		m_pGate (CSpaceObject ref)
//	Metric		m_rDistance

	{
	pStream->Write((char *)&m_dwEncounterTableUNID, sizeof(DWORD));
	pSystem->WriteObjRefToStream(m_pTarget, pStream);
	pSystem->WriteObjRefToStream(m_pGate, pStream);
	pStream->Write((char *)&m_rDistance, sizeof(Metric));
	}

//	CTimedCustomEvent class --------------------------------------------------

CTimedCustomEvent::CTimedCustomEvent (int iTick,
									  CSpaceObject *pObj,
									  const CString &sEvent) :
		CTimedEvent(iTick),
		m_pObj(pObj),
		m_sEvent(sEvent),
		m_bInDoEvent(false)

//	CTimedCustomEvent constructor

	{
	}

CString CTimedCustomEvent::DebugCrashInfo (void)

//	DebugCrashInfo

	{
	CString sResult;

	sResult.Append(CONSTLIT("TimedCustomEvent\r\n"));
	sResult.Append(strPatternSubst(CONSTLIT("m_pObj: %s\r\n"), CSpaceObject::DebugDescribe(m_pObj).GetASCIIZPointer()));
	sResult.Append(strPatternSubst(CONSTLIT("event: %s\r\n"), m_sEvent.GetASCIIZPointer()));
	return sResult;
	}

void CTimedCustomEvent::DoEvent (CSystem *pSystem, bool *retbDeleteEvent)

//	DoEvent
//
//	Do the actual event

	{
	m_bInDoEvent = true;

	if (m_pObj)
		m_pObj->FireCustomEvent(m_sEvent);

	*retbDeleteEvent = true;

	m_bInDoEvent = false;
	}

bool CTimedCustomEvent::OnObjDestroyed (CSpaceObject *pObj)

//	OnObjDestroyed
//
//	Returns TRUE if the event should be destroyed

	{
	return (m_pObj == pObj && !m_bInDoEvent);
	}

void CTimedCustomEvent::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read data

	{
	Ctx.pSystem->ReadObjRefFromStream(Ctx, (CSpaceObject **)&m_pObj);
	m_sEvent.ReadFromStream(Ctx.pStream);
	m_bInDoEvent = false;
	}

void CTimedCustomEvent::OnWriteClassToStream (IWriteStream *pStream)

//	OnWriteClassToStream
//
//	Writes the class

	{
	DWORD dwSave = cTimedCustomEvent;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

void CTimedCustomEvent::OnWriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	OnWriteToStream
//
//	Writes the obj data to stream
//
//	DWORD		m_pObj (CSpaceObject ref)
//	CString		m_sEvent

	{
	pSystem->WriteObjRefToStream(m_pObj, pStream);
	m_sEvent.WriteToStream(pStream);
	}

//	CTimedRecurringEvent class --------------------------------------------------

CTimedRecurringEvent::CTimedRecurringEvent (int iInterval,
											CSpaceObject *pObj,
											const CString &sEvent) :
		CTimedEvent(pObj->GetSystem()->GetTick() + mathRandom(0, iInterval)),
		m_iInterval(iInterval),
		m_pObj(pObj),
		m_sEvent(sEvent),
		m_bDestroy(false),
		m_bInDoEvent(false)

//	CTimedRecurringEvent constructor

	{
	}

CString CTimedRecurringEvent::DebugCrashInfo (void)

//	DebugCrashInfo

	{
	CString sResult;

	sResult.Append(CONSTLIT("TimedRecurringEvent\r\n"));
	sResult.Append(strPatternSubst(CONSTLIT("m_pObj: %s\r\n"), CSpaceObject::DebugDescribe(m_pObj).GetASCIIZPointer()));
	sResult.Append(strPatternSubst(CONSTLIT("event: %s\r\n"), m_sEvent.GetASCIIZPointer()));
	return sResult;
	}

void CTimedRecurringEvent::DoEvent (CSystem *pSystem, bool *retbDeleteEvent)

//	DoEvent
//
//	Do the actual event

	{
	m_bInDoEvent = true;

	if (m_pObj)
		m_pObj->FireCustomEvent(m_sEvent);

	SetTick(pSystem->GetTick() + m_iInterval);
	*retbDeleteEvent = m_bDestroy;

	m_bInDoEvent = false;
	}

bool CTimedRecurringEvent::OnObjDestroyed (CSpaceObject *pObj)

//	OnObjDestroyed
//
//	Returns TRUE if the event should be destroyed

	{
	if (m_pObj == pObj)
		{
		if (m_bInDoEvent)
			{
			m_bDestroy = true;
			return false;
			}
		else
			return true;
		}
	else
		return false;
	}

void CTimedRecurringEvent::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read data

	{
	Ctx.pSystem->ReadObjRefFromStream(Ctx, (CSpaceObject **)&m_pObj);
	m_sEvent.ReadFromStream(Ctx.pStream);
	Ctx.pStream->Read((char *)&m_iInterval, sizeof(DWORD));
	m_bInDoEvent = false;
	m_bDestroy = false;
	}

void CTimedRecurringEvent::OnWriteClassToStream (IWriteStream *pStream)

//	OnWriteClassToStream
//
//	Writes the class

	{
	DWORD dwSave = cTimedRecurringEvent;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

void CTimedRecurringEvent::OnWriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	OnWriteToStream
//
//	Writes the obj data to stream
//
//	DWORD		m_pObj (CSpaceObject ref)
//	CString		m_sEvent
//	DWORD		m_iInterval

	{
	pSystem->WriteObjRefToStream(m_pObj, pStream);
	m_sEvent.WriteToStream(pStream);
	pStream->Write((char *)&m_iInterval, sizeof(DWORD));
	}

