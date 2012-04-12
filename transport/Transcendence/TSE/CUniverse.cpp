//	CUniverse.cpp
//
//	CUniverse class
//
//	VERSION HISTORY
//
//	 0: Pre-0.98
//
//	 1: 0.98
//		version marker in CUniverse
//		version in CUniverse
//		m_Data in CTopologyNode
//
//	 2: 0.99
//		Adventure UNID in CUniverse
//
//	 3: 0.99
//		All design type info is together

#include "PreComp.h"
#include "Kernel.h"

#include "CSoundMgr.h"

const DWORD UNIVERSE_SAVE_VERSION =						3;
const DWORD UNIVERSE_VERSION_MARKER =					0xffffffff;

CUniverse *g_pUniverse = NULL;
Metric g_KlicksPerPixel = KLICKS_PER_PIXEL;
Metric g_TimeScale = TIME_SCALE;
Metric g_SecondsPerUpdate =	g_TimeScale / g_TicksPerSecond;

static CObjectClass<CUniverse>g_Class(OBJID_CUNIVERSE, NULL);

#ifdef DEBUG_PROGRAMSTATE
ProgramStates g_iProgramState = psUnknown;
CSpaceObject *g_pProgramObj = NULL;
CTimedEvent *g_pProgramEvent = NULL;
#endif

#ifdef DEBUG_PERFORMANCE
DWORD g_dwPerformanceTimer;
#endif

CUniverse::CUniverse (void) : CObject(&g_Class),
		m_pSystemTables(NULL),
		m_Sounds(FALSE, TRUE),
		m_LevelEncounterTables(TRUE),
		m_iTick(0),
		m_pAdventure(NULL),
		m_pPOV(NULL),
		m_pPlayer(NULL),
		m_pCurrentSystem(NULL),
		m_StarSystems(TRUE, FALSE),
		m_dwNextID(1),
		m_Topology(FALSE, TRUE),
		m_pSoundMgr(NULL),
		m_pDebug(NULL),
		m_bDebugMode(false),
		m_bNoSound(false)

//	CUniverse constructor

	{
	ASSERT(g_pUniverse == NULL);
	g_pUniverse = this;
	InitTrig();
	}

CUniverse::~CUniverse (void)

//	CUniverse destructor

	{
	int i;

	SetPOV(NULL);

	//	Destroy all star systems. We do this here because we want to
	//	guarantee that we destroy all objects before we destruct
	//	codechain, et al

	m_StarSystems.RemoveAll();

	if (m_pSystemTables)
		delete m_pSystemTables;

	//	Destroy topology objects

	for (i = 0; i < m_Topology.GetCount(); i++)
		{
		CTopologyNode *pNode = (CTopologyNode *)m_Topology.GetValue(i);
		delete pNode;
		}

	//	Free up various arrays whose cleanup requires m_CC

	m_Design.RemoveAll();

	g_pUniverse = NULL;
	}

ALERROR CUniverse::AddStarSystem (CTopologyNode *pTopology, CSystem *pSystem)

//	AddStarSystem
//
//	Adds a system to the universe

	{
	ALERROR error;

	//	Add it to our list

	if ((error = m_StarSystems.ReplaceEntry((int)pSystem->GetID(), 
			pSystem,
			TRUE,
			NULL)))
		{
		ASSERT(false);
		return error;
		}

	//	Set the system for the topoplogy

	if (pTopology)
		{
		ASSERT(pTopology->GetSystemID() == pSystem->GetID());
		pTopology->SetSystem(pSystem);
		}

	return NOERROR;
	}

ALERROR CUniverse::CreateEmptyStarSystem (CSystem **retpSystem)

//	CreateEmptyStarSystem
//
//	Creates an empty star system

	{
	ALERROR error;
	CSystem *pSystem;

	if ((error = CSystem::CreateEmpty(this, NULL, &pSystem)))
		return error;

	//	Add to our list

	DWORD dwID = CreateGlobalID();
	if ((error = m_StarSystems.AddEntry((int)dwID, pSystem)))
		{
		delete pSystem;
		return error;
		}

	pSystem->SetID(dwID);

	//	Done

	if (retpSystem)
		*retpSystem = pSystem;

	return NOERROR;
	}

ALERROR CUniverse::CreateFirstStarSystem (const CString &sStartNode, CSystem **retpSystem, CString *retsError)

//	CreateFirstStarSystem
//
//	Creates the first star system in the topology

	{
	ALERROR error;

	ASSERT(m_StarSystems.GetCount() == 0);

	//	Initialize the topology. This is the point at which the topology is created

	if ((error = InitTopology(retsError)))
		return error;

	//	Figure out where to start

	CString sFirstNode = sStartNode;
	if (sFirstNode.IsBlank())
		sFirstNode = m_Design.GetTopologyDesc()->GetFirstNodeID();

	//	Create the first star system

	CTopologyNode *pFirstTopology = FindTopologyNode(sFirstNode);
	if ((error = CreateStarSystem(pFirstTopology, retpSystem)))
		{
		*retsError = CONSTLIT("Unable to create first star system");
		return error;
		}

	return NOERROR;
	}

ALERROR CUniverse::CreateRandomItem (const CItemCriteria &Crit, 
									 const CString &sLevelFrequency,
									 CItem *retItem)

//	CreateRandomItem
//
//	Generates a random item

	{
	ALERROR error;

	//	Create the generator

	IItemGenerator *pTable;
	if ((error = IItemGenerator::CreateRandomItemTable(Crit, sLevelFrequency, &pTable)))
		return error;

	//	Pick an item

	CItemList ItemList;
	CItemListManipulator Items(ItemList);
	pTable->AddItems(Items);

	//	Done

	Items.ResetCursor();
	if (!Items.MoveCursorForward())
		return ERR_FAIL;

	*retItem = Items.GetItemAtCursor();
	return NOERROR;
	}

ALERROR CUniverse::CreateStarSystem (CTopologyNode *pTopology, CSystem **retpSystem, CSystemCreateStats *pStats)

//	CreateStarSystem
//
//	Creates a new system based on the description

	{
	ALERROR error;
	CSystem *pSystem;

	//	Get the description

	CSystemType *pSystemType = FindSystemType(pTopology->GetSystemDescUNID());
	if (pSystemType == NULL)
		return ERR_FAIL;

	//	Create the system

	if ((error = CSystem::CreateFromXML(this, pSystemType, pTopology, m_pSystemTables, &pSystem, pStats)))
		return error;

	//	Add to our list

	DWORD dwID = CreateGlobalID();
	if ((error = m_StarSystems.AddEntry(dwID, pSystem)))
		{
		delete pSystem;
		return error;
		}

	pSystem->SetID(dwID);

	//	Set the system for the topoplogy

	pTopology->SetSystem(pSystem);
	pTopology->SetSystemID(pSystem->GetID());

	//	Done

	if (retpSystem)
		*retpSystem = pSystem;

	return NOERROR;
	}

void CUniverse::DebugOutput (char *pszLine, ...)

//	DebugOutput
//
//	Outputs debug line

	{
	if (m_pDebug)
		{
		char *pArgs;
		char szBuffer[1024];
		int iLen;

		pArgs = (char *)&pszLine + sizeof(pszLine);
		iLen = vsprintf(szBuffer, pszLine, pArgs);

		m_pDebug->DebugOutput(CString(szBuffer));
		}
	}

void CUniverse::DestroySystem (CSystem *pSystem)

//	DestroySystem
//
//	Destroys the given system

	{
	//	Change POV, if necessary

	if (m_pPOV && m_pPOV->GetSystem() == pSystem)
		SetPOV(NULL);

	//	Remove and destroy the system

	if (m_StarSystems.RemoveEntry(pSystem->GetID(), NULL) != NOERROR)
		{
		ASSERT(false);
		return;
		}
	}

CArmorClass *CUniverse::FindArmor (DWORD dwUNID)

//	FindArmor
//
//	Returns the armor class by item UNID

	{
	CItemType *pType = FindItemType(dwUNID);
	if (pType == NULL)
		return NULL;

	return pType->GetArmorClass();
	}

CObject *CUniverse::FindByUNID (CIDTable &Table, DWORD dwUNID)

//	FindByUNID
//
//	Returns an object by UNID from the appropriate table

	{
	CObject *pObj;

	if (Table.Lookup((int)dwUNID, &pObj) == NOERROR)
		return pObj;
	else
		return NULL;
	}

CDeviceClass *CUniverse::FindDeviceClass (DWORD dwUNID)

//	FindDeviceClass
//
//	Returns the device class by item UNID

	{
	CItemType *pType = FindItemType(dwUNID);
	if (pType == NULL)
		return NULL;

	return pType->GetDeviceClass();
	}

CSpaceObject *CUniverse::FindObject (DWORD dwID)

//	FindObject
//
//	Finds the object by ID

	{
	int i;

	if (m_pCurrentSystem)
		{
		for (i = 0; i < m_pCurrentSystem->GetObjectCount(); i++)
			{
			CSpaceObject *pObj = m_pCurrentSystem->GetObject(i);
			if (pObj && pObj->GetID() == dwID)
				return pObj;
			}

		return NULL;
		}
	else
		return NULL;
	}

CWeaponFireDesc *CUniverse::FindWeaponFireDesc (const CString &sName)

//	FindWeaponFireDesc
//
//	Returns a pointer to the descriptor by name

	{
	return CWeaponFireDesc::FindWeaponFireDescFromFullUNID(sName); 
	}

CWeaponFireDesc *CUniverse::FindWeaponFireDesc (DWORD dwUNID)

//	FindWeaponFireDesc
//
//	Returns a pointer to the descriptor by weapon UNID

	{
	return CWeaponFireDesc::FindWeaponFireDesc(dwUNID);
	}

void CUniverse::FlushStarSystem (CTopologyNode *pTopology)

//	FlushStarSystem
//
//	Flushes the star system after it has been saved.

	{
	ASSERT(pTopology->GetSystemID() != 0xffffffff);
	m_StarSystems.RemoveEntry(pTopology->GetSystemID(), NULL);
	}

void CUniverse::GarbageCollectLibraryBitmaps (void)

//	GarbageCollectLibraryBitmaps
//
//	Garbage-collects any unused bitmaps

	{
	//	Mark all bitmaps currently in use

	m_BitmapLibrary.ClearMarks();
	if (m_pCurrentSystem)
		m_pCurrentSystem->MarkImages();

	//	Sweep

	m_BitmapLibrary.Sweep();
	}

CTopologyNode *CUniverse::GetFirstTopologyNode (void)

//	GetFirstTopologyNode
//
//	Returns the starting node

	{
	ASSERT(m_StarSystems.GetCount() == 0);

	//	Initialize the topology

	CString sError;
	InitTopology(&sError);

	//	Create the first star system

	return FindTopologyNode(m_Design.GetTopologyDesc()->GetFirstNodeID());
	}

void CUniverse::GetRandomLevelEncounter (int iLevel, 
										 IShipGenerator **retpTable, 
										 CSovereign **retpBaseSovereign)

//	GetRandomLevelEncounter
//
//	Returns a random encounter appropriate for the given level

	{
	int i;

	iLevel--;	//	m_LevelEncounterTable is 0-based
	if (iLevel < 0 || iLevel >= m_LevelEncounterTables.GetCount())
		{
		*retpTable = NULL;
		*retpBaseSovereign = NULL;
		return;
		}

	//	Get the level table

	CStructArray *pTable = (CStructArray *)m_LevelEncounterTables.GetObject(iLevel);

	//	Compute the totals for the table

	int iTotal = 0;
	for (i = 0; i < pTable->GetCount(); i++)
		iTotal += ((SLevelEncounter *)pTable->GetStruct(i))->iWeight;

	if (iTotal == 0)
		{
		*retpTable = NULL;
		*retpBaseSovereign = NULL;
		return;
		}

	//	Pick a random entry

	int iRoll = mathRandom(0, iTotal - 1);
	int iPos = 0;

	//	Get the position

	while (((SLevelEncounter *)pTable->GetStruct(iPos))->iWeight <= iRoll)
		iRoll -= ((SLevelEncounter *)pTable->GetStruct(iPos++))->iWeight;

	//	Done

	*retpTable = ((SLevelEncounter *)pTable->GetStruct(iPos))->pTable;
	*retpBaseSovereign = ((SLevelEncounter *)pTable->GetStruct(iPos))->pBaseSovereign;
	}

ALERROR CUniverse::LoadFromStream (IReadStream *pStream, DWORD *retdwSystemID, DWORD *retdwPlayerID, CString *retsError)

//	LoadFromStream
//
//	Loads the universe from a stream
//
//	DWORD		UNIVERSE_VERSION_MARKER
//	DWORD		universe version
//	DWORD		m_iTick
//	DWORD		m_dwNextID
//
//	DWORD		Adventure UNID
//	DWORD		ID of POV system (0xffffffff if none)
//	DWORD		index of POV (0xffffffff if none)
//	CTimeSpan	time that we've spent playing the game
//
//	DWORD		No of item types
//	DWORD		type: UNID
//	DWORD		type: flags
//
//	DWORD		No of topology nodes
//	CString		node: Node ID
//	DWORD		node: ID of system instance (0xffffffff if not yet created)
//
//	DWORD		No of station types
//	DWORD		type: UNID
//	DWORD		type: flags
//	data		Attribute data block
//
//	DWORD		No of ship classes
//	DWORD		type: UNID
//	DWORD		type: flags
//	data		Attribute data block
	
	{
	int i;
	DWORD dwLoad;
	DWORD dwVersion;
	DWORD dwCount;

	//	Load the version. Version 0 we did not even have a place to store the
	//	version, so we see if the first entry is -1. If it is not, then we
	//	are at version 0.

	pStream->Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad == UNIVERSE_VERSION_MARKER)
		{
		pStream->Read((char *)&dwVersion, sizeof(DWORD));
		pStream->Read((char *)&m_iTick, sizeof(DWORD));
		}
	else
		{
		dwVersion = 0;
		m_iTick = (int)dwLoad;
		}

	//	Create load structure

	SUniverseLoadCtx Ctx;
	Ctx.dwVersion = dwVersion;
	Ctx.pStream = pStream;

	//	Load basic data

	pStream->Read((char *)&m_dwNextID, sizeof(DWORD));

	//	Load adventure UNID

	if (dwVersion >= 2)
		pStream->Read((char *)&dwLoad, sizeof(DWORD));
	else
		dwLoad = DEFAULT_ADVENTURE_UNID;

	CString sError;
	if (InitAdventure(dwLoad, &sError) != NOERROR)
		{
		*retsError = strPatternSubst(CONSTLIT("Unable to load universe: %s"), sError.GetPointer());
		return ERR_FAIL;
		}

	//	More data

	pStream->Read((char *)retdwSystemID, sizeof(DWORD));
	pStream->Read((char *)retdwPlayerID, sizeof(DWORD));

	CTimeSpan Time;
	Time.ReadFromStream(pStream);
	CTimeDate Now(CTimeDate::Now);
	m_StartTime = timeSubtractTime(Now, Time);

	//	Load item type data

	if (dwVersion < 3)
		{
		pStream->Read((char *)&dwCount, sizeof(DWORD));
		for (i = 0; i < (int)dwCount; i++)
			{
			pStream->Read((char *)&dwLoad, sizeof(DWORD));
			CItemType *pType = FindItemType(dwLoad);
			if (pType == NULL)
				{
				*retsError = strPatternSubst(CONSTLIT("Unable to find type: %x (missing mod?)"), dwLoad);
				return ERR_FAIL;
				}

			pType->ReadFromStream(Ctx);
			}
		}

	//	Load topology data

	pStream->Read((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		{
		CTopologyNode *pNode;
		CTopologyNode::CreateFromStream(Ctx, &pNode);

		m_Topology.AddEntry(pNode->GetID(), (CObject *)pNode);
		}

	//	Load type data

	if (dwVersion >= 3)
		{
		pStream->Read((char *)&dwCount, sizeof(DWORD));
		for (i = 0; i < (int)dwCount; i++)
			{
			pStream->Read((char *)&dwLoad, sizeof(DWORD));
			CDesignType *pType = FindDesignType(dwLoad);
			if (pType == NULL)
				{
				*retsError = strPatternSubst(CONSTLIT("Unable to find type: %x (missing mod?)"), dwLoad);
				return ERR_FAIL;
				}

			pType->ReadFromStream(Ctx);
			}
		}

	//	Load station type data

	if (dwVersion < 3)
		{
		pStream->Read((char *)&dwCount, sizeof(DWORD));
		for (i = 0; i < (int)dwCount; i++)
			{
			pStream->Read((char *)&dwLoad, sizeof(DWORD));
			CStationType *pType = FindStationType(dwLoad);
			if (pType == NULL)
				{
				*retsError = strPatternSubst(CONSTLIT("Unable to find type: %x (missing mod?)"), dwLoad);
				return ERR_FAIL;
				}

			pType->ReadFromStream(Ctx);
			}
		}

	//	If we've still got data left, then this is a newer version and we
	//	can read the ship class data

	if (dwVersion < 3)
		{
		if (pStream->Read((char *)&dwCount, sizeof(DWORD)) == NOERROR)
			{
			for (i = 0; i < (int)dwCount; i++)
				{
				pStream->Read((char *)&dwLoad, sizeof(DWORD));
				CShipClass *pClass = FindShipClass(dwLoad);
				if (pClass == NULL)
					{
					*retsError = strPatternSubst(CONSTLIT("Unable to find type: %x (missing mod?)"), dwLoad);
					return ERR_FAIL;
					}

				pClass->ReadFromStream(Ctx);
				}
			}
		}

	//	If we've still got data left, then read the sovereign data

	if (dwVersion < 3)
		{
		if (pStream->Read((char *)&dwCount, sizeof(DWORD)) == NOERROR)
			{
			for (i = 0; i < (int)dwCount; i++)
				{
				pStream->Read((char *)&dwLoad, sizeof(DWORD));
				CSovereign *pSovereign = FindSovereign(dwLoad);
				if (pSovereign == NULL)
					{
					*retsError = strPatternSubst(CONSTLIT("Unable to find type: %x (missing mod?)"), dwLoad);
					return ERR_FAIL;
					}

				pSovereign->ReadFromStream(Ctx);
				}
			}
		}

	return NOERROR;
	}

void CUniverse::LoadLibraryBitmaps (void)

//	LoadLibraryBitmaps
//
//	Make sure the bitmaps that we need are loaded

	{
	if (m_pCurrentSystem)
		m_pCurrentSystem->LoadImages();
	}

void CUniverse::PaintObject (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pObj)

//	PaintObject
//
//	Paints this object only

	{
	if (m_pPOV)
		m_pPOV->GetSystem()->PaintViewportObject(Dest, rcView, m_pPOV, pObj);
	}

void CUniverse::PaintObjectMap (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pObj)

//	PaintObjectMap
//
//	Paints this object only

	{
	if (m_pPOV)
		m_pPOV->GetSystem()->PaintViewportMapObject(Dest, rcView, m_pPOV, pObj);
	}

void CUniverse::PaintPOV (CG16bitImage &Dest, const RECT &rcView, bool bEnhanced)

//	PaintPOV
//
//	Paint the current point of view

	{
	if (m_pPOV)
		m_pPOV->GetSystem()->PaintViewport(Dest, rcView, m_pPOV, bEnhanced);
	}

void CUniverse::PaintPOVLRS (CG16bitImage &Dest, const RECT &rcView, bool *retbNewEnemies)

//	PaintPOVLRS
//
//	Paint the LRS from the current POV

	{
	if (m_pPOV)
		m_pPOV->GetSystem()->PaintViewportLRS(Dest, rcView, m_pPOV, retbNewEnemies);
	}

void CUniverse::PaintPOVMap (CG16bitImage &Dest, const RECT &rcView, Metric rMapScale)

//	PaintPOVMap
//
//	Paint the system map

	{
	if (m_pPOV)
		m_pPOV->GetSystem()->PaintViewportMap(Dest, rcView, m_pPOV, rMapScale);
	}

void CUniverse::PlaySound (CSpaceObject *pSource, int iChannel)

//	PlaySound
//
//	Plays a sound from the given source

	{
	if (!m_bNoSound && m_pSoundMgr && m_pPOV)
		{
		//	Default to full volume

		int iVolume = 0;

		//	Figure out how close the source is to the POV. The sound fades as we get
		//	further away.

		if (pSource)
			{
			CVector vDist = pSource->GetPos() - m_pPOV->GetPos();
			Metric rDist2 = vDist.Length2();
			Metric rMaxSound2 = (MAX_SOUND_DISTANCE * g_KlicksPerPixel) * (MAX_SOUND_DISTANCE * g_KlicksPerPixel);
			iVolume = -(int)(10000 * rDist2 / rMaxSound2);

			//	If below a certain level, then it is silent anyway

			if (iVolume <= -10000)
				return;
			}

		m_pSoundMgr->Play(iChannel, iVolume, 0);
		}
	}

ALERROR CUniverse::Reinit (void)

//	Reinit
//
//	Reinitializes the universe

	{
	int i;

	//	Clear some basic variables

	m_iTick = 0;
	m_pPOV = NULL;
	SetCurrentSystem(NULL);
	m_StarSystems.RemoveAll();
	m_dwNextID = 1;

	//	Reinit design

	for (i = 0; i < m_Design.GetCount(); i++)
		{
		CDesignType *pType = m_Design.GetEntry(i);
		pType->Reinit();
		}

	//	Clear the topology nodes

	for (i = 0; i < m_Topology.GetCount(); i++)
		{
		CTopologyNode *pNode = (CTopologyNode *)m_Topology.GetValue(i);
		delete pNode;
		}

	m_Topology.RemoveAll();

	//	Clear out player variables

	CCodeChain &CC = GetCC();
	CC.DefineGlobal(CONSTLIT("gPlayer"), CC.CreateNil());
	CC.DefineGlobal(CONSTLIT("gPlayerShip"), CC.CreateNil());

	return NOERROR;
	}

ALERROR CUniverse::SaveToStream (IWriteStream *pStream)

//	SaveToStream
//
//	Saves the universe to a stream.
//
//	DWORD		UNIVERSE_VERSION_MARKER
//	DWORD		universe version
//	DWORD		m_iTick
//	DWORD		m_dwNextID
//
//	DWORD		Adventure UNID
//	DWORD		ID of POV system (0xffffffff if none)
//	DWORD		index of POV (0xffffffff if none)
//	DWORD		milliseconds that we've spent playing the game
//
//	DWORD		No of topology nodes
//	CString		node: Node ID
//	DWORD		node: ID of system instance (0xffffffff if not yet created)
//
//	DWORD		No of types
//	DWORD		type: UNID
//	CDesignType

	{
	int i;
	DWORD dwSave;

	//	Write out version

	dwSave = UNIVERSE_VERSION_MARKER;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	dwSave = UNIVERSE_SAVE_VERSION;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Write basic data

	pStream->Write((char *)&m_iTick, sizeof(DWORD));
	pStream->Write((char *)&m_dwNextID, sizeof(DWORD));

	//	Adventure UNID

	dwSave = m_pAdventure->GetUNID();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Write the ID of POV system

	dwSave = 0xffffffff;
	if (m_pPOV && m_pPOV->GetSystem())
		dwSave = m_pPOV->GetSystem()->GetID();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Write the ID of the POV

	dwSave = 0xffffffff;
	if (m_pPOV && m_pPOV->GetSystem())
		m_pPOV->GetSystem()->WriteObjRefToStream(m_pPOV, pStream);

	//	Calculate the amount of time that we've been playing the game

	CTimeSpan GameLength = StopGameTime();
	GameLength.WriteToStream(pStream);

	//	Save out topology node data

	DWORD dwCount = GetTopologyNodeCount();
	pStream->Write((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		{
		CTopologyNode *pNode = GetTopologyNode(i);
		pNode->WriteToStream(pStream);
		}

	//	Save out type data

	dwCount = m_Design.GetCount();
	pStream->Write((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		{
		CDesignType *pType = m_Design.GetEntry(i);

		dwSave = pType->GetUNID();
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		pType->WriteToStream(pStream);
		}

	return NOERROR;
	}

void CUniverse::SetCurrentSystem (CSystem *pSystem)

//	SetCurrentSystem
//
//	Sets the current system

	{
	m_pCurrentSystem = pSystem;

	if (pSystem)
		{
		g_KlicksPerPixel = pSystem->GetSpaceScale();
		g_TimeScale = pSystem->GetTimeScale();
		g_SecondsPerUpdate = g_TimeScale / g_TicksPerSecond;
		}
	else
		{
		g_KlicksPerPixel = KLICKS_PER_PIXEL;
		g_TimeScale = TIME_SCALE;
		g_SecondsPerUpdate = g_TimeScale / g_TicksPerSecond;
		}
	}

void CUniverse::SetPOV (CSpaceObject *pPOV)

//	SetPOV
//
//	Sets the current point of view. The POV is used for both painting
//	and to determine what should be updated.

	{
	if (m_pPOV)
		{
		CSpaceObject *pOldPOV = m_pPOV;
		m_pPOV = NULL;
		pOldPOV->OnLosePOV();
		}

	m_pPOV = pPOV;

	if (m_pPOV)
		SetCurrentSystem(m_pPOV->GetSystem());
	else
		SetCurrentSystem(NULL);
	}

void CUniverse::StartGameTime (void)

//	StartGameTime
//
//	Starts timing the game

	{
	m_StartTime = CTimeDate(CTimeDate::Now);
	}

CTimeSpan CUniverse::StopGameTime (void)

//	StopGameTime
//
//	Stops timing the game and returns the amount of
//	time elapsed from the start.

	{
	CTimeDate StopTime(CTimeDate::Now);
	return timeSpan(m_StartTime, StopTime);
	}

void CUniverse::Update (Metric rSecondsPerTick)

//	Update
//
//	Update the system of the current point of view

	{
	if (m_pPOV)
		m_pPOV->GetSystem()->Update(rSecondsPerTick);

	m_iTick++;
	}

