//	CSystem.cpp
//
//	CSystem class
//
//	VERSION HISTORY
//
//	 0: 0.95
//	 1: 0.95a
//	 2: 0.96a
//		m_pEncounterInfo in CShip
//		m_rItemMass in CShip
//
//	 3: 0.97
//		m_sDeferredDestruction string in CPlayerShipController.
//		m_iAngryCounter in CStation
//		m_vOldPos in CParticleDamage
//
//	 4: 0.97
//		m_iCountdown in CStandardShipAI
//
//	 5: 0.97
//		m_iStealth in CShip
//
//	 6-14: 0.98
//		m_Blacklist in CBaseShipAI
//		m_iDeviceCount in CShip
//		m_sName in CMarker
//		m_rKlicksPerPixel in CSystem
//		m_rTimeScale in CSystem
//		m_Blacklist in CStation
//		m_iReinforceRequestCount in CStation
//		m_NavPaths in CSystem
//		m_pNavPath in CBaseShipAI
//		m_iNavPathPos in CBaseShipAI
//		m_iBalance in CStation
//		m_dwID in CSpaceObject
//		m_vStart in CNavigationPath
//		m_pCommandCode in CStandardShipAI
//		m_fIdentified in CShip
//
//	15: 0.98a
//		UNIDs for painters are saved as strings
//
//	16: 0.98c
//		m_sStargateDestNode in CStation
//		m_sStargateDestEntryPoint in CStation
//
//	17: 0.98c
//		m_iRotationCount in CObjectImageArray
//
//	18: 0.99
//		m_iCause in DamageDesc
//		m_iCause in CAreaDamage
//		m_iCause in CMissile
//		m_iCause in CBeam
//		m_iCause in CParticleDamage
//		m_iCause in CRadiusDamage
//
//	19: 0.99
//		m_pTarget in CRadiusDamage
//
//	20: 0.99
//		m_iTick in CShockwavePainter
//
//	21: 0.99
//		m_iInitialDelay in CAreaDamage
//		new particle array in CParticleDamage
//
//	22: 0.99
//		Experiment: CannotBeHit is set to TRUE for background objects
//
//	23: 0.99
//		SExtra structure for CItem
//
//	24: 0.99
//		m_iRotation in CDockingPorts
//
//	25: 0.99
//		m_iPower in CPlayerShipController
//		m_iMaxPower in CPlayerShipController
//		m_iPowerRate in CPlayerShipController
//
//	26: 0.99
//		m_dwNameFlags in CShip
//
//	27: 0.99
//		DiceRange uses 32-bits for all values
//
//	28: 0.99
//		m_iHitPoints in CMissile
//
//	29: 0.99
//		m_iDeviceSlot in CInstalledDevice
//
//	30: 0.99
//		CDamageSource
//
//	31: 0.99
//		m_iMaxStructuralHP in CStation
//
//	32: 0.99
//		m_pSecondarySource in CDamageSource
//
//	33: 0.99b
//		m_iManeuverCounter in CBaseShipAI

#include "PreComp.h"
#include "Kernel.h"
#include "math.h"

#include "CG16bitRegion.h"
#include "CRect.h"

#define SYSTEM_SAVE_VERSION								33

#define ENHANCED_SRS_BLOCK_SIZE							6

#define LEVEL_ENCOUNTER_CHANCE							10

#define MAX_TARGET_RANGE								(24.0 * LIGHT_SECOND)

static CObjectClass<CSystem>g_Class(OBJID_CSYSTEM, NULL);

int g_iGateTimer = 0;
int g_iGateTimerTick = -1;

enum LabelPositionTypes
	{
	labelPosNone,

	labelPosRight,
	labelPosLeft,
	labelPosBottom,
	};

struct SLabelEntry
	{
	CSpaceObject *pObj;
	int x;
	int y;
	int cxLabel;

	RECT rcLabel;
	int iPosition;
	int iNewPosition;
	};

const int g_iStarFieldWidth = 2000;
const int g_iStarFieldHeight = 1000;

const COLORREF g_rgbSpaceColor = CGImage::RGBColor(0,0,8);
//const COLORREF g_rgbSpaceColor = RGB(0,0,0);
const Metric g_MetersPerKlick = 1000.0;
const Metric MAP_VERTICAL_ADJUST =						1.4;

const COLORREF ENHANCED_SRS_ENEMY_COLOR	=					CGImage::RGBColor(255, 0, 0);
const COLORREF ENHANCED_SRS_FRIEND_COLOR =					CGImage::RGBColor(0, 255, 0);
const COLORREF RGB_GRID_LINE =								CGImage::RGBColor(65, 68, 77);

const int LABEL_SPACING_X =								8;
const int LABEL_SPACING_Y =								4;
const int LABEL_OVERLAP_Y =								1;

const Metric BACKGROUND_OBJECT_FACTOR =					4.0;

bool CalcOverlap (SLabelEntry *pEntries, int iCount);
void SetLabelBelow (SLabelEntry &Entry, int cyChar);
void SetLabelLeft (SLabelEntry &Entry, int cyChar);
void SetLabelRight (SLabelEntry &Entry, int cyChar);

CSystem::CSystem (void) : CObject(&g_Class),
		m_iTick(0),
		m_AllObjects(TRUE),
		m_NamedObjects(FALSE, FALSE),
		m_StarField(sizeof(CStar), 500),
		m_TimedEvents(FALSE),
		m_pEncounterObj(NULL),
		m_iEncounterObjCount(0),
		m_iTimeStopped(0),
		m_fInCreate(false),
		m_rTimeScale(TIME_SCALE),
		m_rKlicksPerPixel(KLICKS_PER_PIXEL)

//	CSystem constructor

	{
	ASSERT(FALSE);
	}

CSystem::CSystem (CUniverse *pUniv, CTopologyNode *pTopology) : CObject(&g_Class),
		m_pUniv(pUniv),
		m_pTopology(pTopology),
		m_iTick(0),
		m_AllObjects(TRUE),
		m_NamedObjects(FALSE, FALSE),
		m_StarField(sizeof(CStar), 500),
		m_TimedEvents(FALSE),
		m_pEncounterObj(NULL),
		m_iEncounterObjCount(0),
		m_dwID(0xffffffff),
		m_fNoRandomEncounters(false),
		m_pEnvironment(NULL),
		m_iTimeStopped(0),
		m_fInCreate(false),
		m_rTimeScale(TIME_SCALE),
		m_rKlicksPerPixel(KLICKS_PER_PIXEL)

//	CSystem constructor

	{
	//	Make sure our vectors are initialized
	InitTrig();
	}

CSystem::~CSystem (void)

//	CSystem destructor

	{
	int i;

	for (i = 0; i < m_TimedEvents.GetCount(); i++)
		delete (CTimedEvent *)m_TimedEvents.GetObject(i);

	if (m_pEncounterObj)
		delete [] m_pEncounterObj;

	//	Set our topology node to NULL so that a new system is
	//	created next time we access this node.

	if (m_pTopology)
		m_pTopology->SetSystem(NULL);

	if (m_pEnvironment)
		delete m_pEnvironment;
	}

ALERROR CSystem::AddTimedEvent (CTimedEvent *pEvent)

//	AddTimedEvent
//
//	Adds a timed event

	{
	return m_TimedEvents.AppendObject((CObject *)pEvent, NULL);
	}

ALERROR CSystem::AddToSystem (CSpaceObject *pObj, int *retiIndex)

//	AddToSystem
//
//	Adds an object to the system

	{
	int i;

	FlushEnemyObjectCache();

	//	Reuse a slot first

	for (i = 0; i < m_AllObjects.GetCount(); i++)
		{
		if (m_AllObjects.GetObject(i) == NULL)
			{
			m_AllObjects.ReplaceObject(i, pObj);
			if (retiIndex)
				*retiIndex = i;
			return NOERROR;
			}
		}

	//	If we could not find a free place, add a new object

	return m_AllObjects.AppendObject(pObj, retiIndex);
	}

int CSystem::CalculateLightIntensity (const CVector &vPos, CSpaceObject **retpStar)

//	CalculateLightIntensity
//
//	Returns 0-100% the intensity of the light at this point
//	in space.

	{
	int i;

	//	Find the nearest star to the position. We optimize the case where
	//	there is only a single star in the system.

	int iBestDist;
	CSpaceObject *pBestObj;

	if (m_Stars.GetCount() == 1)
		{
		pBestObj = m_Stars.GetObj(0);
		iBestDist = (int)(vPos.Longest() / LIGHT_SECOND);
		}
	else
		{
		pBestObj = NULL;
		iBestDist = 100000000;

		for (i = 0; i < m_Stars.GetCount(); i++)
			{
			CSpaceObject *pStar = m_Stars.GetObj(i);
			CVector vDist = vPos - pStar->GetPos();

			int iDistFromCenter = (int)(vDist.Longest() / LIGHT_SECOND);
			if (iDistFromCenter < iBestDist)
				{
				iBestDist = iDistFromCenter;
				pBestObj = pStar;
				}
			}

		if (pBestObj == NULL)
			{
			if (retpStar)
				*retpStar = NULL;
			return 0;
			}
		}

	//	Compute the percentage

	int iDistFromCenter = (iBestDist < 15 ? 0 : iBestDist - 15);
	int iPercent = 100 - (iDistFromCenter * 100 / 120);

	if (retpStar)
		*retpStar = pBestObj;

	return std::max(0, iPercent);
	}

COLORREF CSystem::CalculateSpaceColor (CSpaceObject *pPOV)

//	CalculateSpaceColor
//
//	Calculates the color of space from the given object

	{
	CSpaceObject *pStar;
	int iPercent = CalculateLightIntensity(pPOV->GetPos(), &pStar);

	COLORREF rgbStarColor = (pStar ? pStar->GetSpaceColor() : 0);

	int iRed = CGImage::RedColor(rgbStarColor) * iPercent / 100;
	int iGreen = CGImage::GreenColor(rgbStarColor) * iPercent / 100;
	int iBlue = CGImage::BlueColor(rgbStarColor) * iPercent / 100;

	return CGImage::RGBColor(iRed, iGreen, iBlue);
	}

void CSystem::CancelTimedEvent (CSpaceObject *pSource, const CString &sEvent)

//	CancelTimedEvent
//
//	Cancel event by name

	{
	int i;

	for (i = 0; i < GetTimedEventCount(); i++)
		{
		CTimedEvent *pEvent = GetTimedEvent(i);
		if (pEvent->GetEventHandlerObj() == pSource 
				&& strEquals(pEvent->GetEventHandlerName(), sEvent))
			{
			if (pEvent->InDoEvent())
				pEvent->OnObjDestroyed(pSource);
			else
				{
				m_TimedEvents.RemoveObject(i);
				i--;
				}
			}
		}
	}

void CSystem::ComputeMapLabels (void)

//	ComputeMapLabels
//
//	Positions the labels for all objects that need one

	{
	int i;
	const int MAX_LABELS = 100;
	int iLabelCount = 0;
	SLabelEntry Labels[MAX_LABELS];

	//	Compute some font metrics

	int cxChar = g_pUniverse->GetMapLabelFont().GetAverageWidth();
	int cyChar = g_pUniverse->GetMapLabelFont().GetHeight();

	//	Compute a transform for map coordinate

	ViewportTransform Trans(CVector(), 
			g_MapKlicksPerPixel, 
			g_MapKlicksPerPixel * MAP_VERTICAL_ADJUST,
			0, 
			0);

	//	Loop over all objects and see if they have a map label

	for (i = 0; i < GetObjectCount() && iLabelCount < MAX_LABELS; i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj && pObj->HasMapLabel())
			{
			Labels[iLabelCount].pObj = pObj;
			Trans.Transform(pObj->GetPos(), &Labels[iLabelCount].x, &Labels[iLabelCount].y);
			Labels[iLabelCount].cxLabel = g_pUniverse->GetMapLabelFont().MeasureText(pObj->GetName(NULL));

			SetLabelRight(Labels[iLabelCount], cyChar);

			iLabelCount++;
			}
		}

	//	Keep looping until we minimize overlap

	bool bOverlap;
	int iIteration = 0;

	do
		{
		bOverlap = CalcOverlap(Labels, iLabelCount);
		if (bOverlap)
			{
			//	Modify the label location of any overlapping labels

			for (i = 0; i < iLabelCount; i++)
				{
				switch (Labels[i].iNewPosition)
					{
					case labelPosRight:
						{
						SetLabelRight(Labels[i], cyChar);
						break;
						}

					case labelPosLeft:
						{
						SetLabelLeft(Labels[i], cyChar);
						break;
						}

					case labelPosBottom:
						{
						SetLabelBelow(Labels[i], cyChar);
						break;
						}
					}
				}

			iIteration++;
			}
		}
	while (bOverlap && iIteration < 10);

	//	Set the label position for all the objects

	for (i = 0; i < iLabelCount; i++)
		Labels[i].pObj->SetMapLabelPos(Labels[i].rcLabel.left - Labels[i].x, Labels[i].rcLabel.top - Labels[i].y - LABEL_OVERLAP_Y);
	}

void CSystem::ComputeRandomEncounters (int iCount)

//	ComputeRandomEncounters
//
//	Creates the table that lists all objects in the system that
//	can generate random encounters

	{
	int i;

	ASSERT(m_pEncounterObj == NULL);

	if (iCount > 0 && !m_fNoRandomEncounters)
		{
		m_iEncounterObjCount = iCount;
		m_pEncounterObj = new CSpaceObject * [iCount];

		iCount = 0;
		for (i = 0; i < GetObjectCount(); i++)
			{
			CSpaceObject *pObj = GetObject(i);

			if (pObj)
				if (pObj->GetRandomEncounterTable())
					m_pEncounterObj[iCount++] = pObj;
			}

		//	Set the next encounter time

		m_iNextEncounter = m_iTick + mathRandom(500, 2500);
		}
	}

void CSystem::ComputeStars (void)

//	ComputeStars
//
//	Keep a list of the stars in the system

	{
	int i;

	m_Stars.RemoveAll();

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj && pObj->GetScale() == scaleStar)
			m_Stars.Add(pObj);
		}
	}

void CSystem::ConvertSpaceEnvironmentToPointers (CTileMap &UNIDs, CTileMap **retpPointers)

//	ConvertSpaceEnvironmentToPointers
//
//	Converts m_pEnvironment to pointers (restore from save file)

	{
	*retpPointers = new CTileMap(UNIDs.GetSize(), UNIDs.GetScale());

	STileMapEnumerator k;
	while (UNIDs.HasMore(k))
		{
		DWORD dwTile;
		int xTile;
		int yTile;

		UNIDs.GetNext(k, &xTile, &yTile, &dwTile);
		CSpaceEnvironmentType *pEnv = g_pUniverse->FindSpaceEnvironment(dwTile);
		if (pEnv)
			(*retpPointers)->SetTile(xTile, yTile, (DWORD)pEnv);
		}
	}

void CSystem::ConvertSpaceEnvironmentToUNIDs (CTileMap &Pointers, CTileMap **retpUNIDs)

//	ConvertSpaceEnvironmentToUNIDs
//
//	Converts m_pEnvironment to UNIDs

	{
	*retpUNIDs = new CTileMap(Pointers.GetSize(), Pointers.GetScale());

	STileMapEnumerator k;
	while (Pointers.HasMore(k))
		{
		DWORD dwTile;
		int xTile;
		int yTile;

		Pointers.GetNext(k, &xTile, &yTile, &dwTile);
		CSpaceEnvironmentType *pEnv = (CSpaceEnvironmentType *)dwTile;
		if (pEnv)
			(*retpUNIDs)->SetTile(xTile, yTile, pEnv->GetUNID());
		}
	}

ALERROR CSystem::CreateFromStream (CUniverse *pUniv, 
								   IReadStream *pStream, 
								   CSystem **retpSystem,
								   DWORD dwObjID,
								   CSpaceObject **retpObj)

//	CreateFromStream
//
//	Creates the star system from the stream
//
//	DWORD		m_dwID
//	DWORD		m_iTick
//	DWORD		m_iTimeStopped
//	CString		m_sName
//	CString		Topology node ID
//	DWORD		(unused)
//	DWORD		m_iNextEncounter
//	DWORD		flags
//	DWORD		SAVE VERSION (only if [flags & 0x02])
//	Metric		m_rKlicksPerPixel
//	Metric		m_rTimeScale
//
//	DWORD		Number of CNavigationPath
//	CNavigationPath
//
//	DWORD		Number of objects
//	CSpaceObject
//
//	DWORD		Number of named objects
//	CString		entrypoint: name
//	DWORD		entrypoint: CSpaceObject ref
//
//	DWORD		Number of timed events
//	CTimedEvent
//
//	DWORD		Number of environment maps
//	CTileMap

	{
	ALERROR error;
	int i;
	DWORD dwLoad;
	DWORD dwCount;

	//	Create a context block for loading

	SLoadCtx Ctx;
	Ctx.dwVersion = 0;	//	Default to 0
	Ctx.pStream = pStream;

	//	Create the new star system

	Ctx.pSystem = new CSystem(pUniv, NULL);
	if (Ctx.pSystem == NULL)
		return ERR_MEMORY;

	//	Load some misc info

	Ctx.pStream->Read((char *)&Ctx.pSystem->m_dwID, sizeof(DWORD));
	Ctx.pStream->Read((char *)&Ctx.pSystem->m_iTick, sizeof(DWORD));
	Ctx.pStream->Read((char *)&Ctx.pSystem->m_iTimeStopped, sizeof(DWORD));
	Ctx.pSystem->m_sName.ReadFromStream(Ctx.pStream);

	//	Load the topology node

	CString sNodeID;
	sNodeID.ReadFromStream(Ctx.pStream);
	Ctx.pSystem->m_pTopology = pUniv->FindTopologyNode(sNodeID);
	Ctx.pSystem->m_pType = pUniv->FindSystemType(Ctx.pSystem->m_pTopology->GetSystemDescUNID());

	//	More misc info

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	Ctx.pStream->Read((char *)&Ctx.pSystem->m_iNextEncounter, sizeof(DWORD));

	//	Flags

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	Ctx.pSystem->m_fNoRandomEncounters = ((dwLoad & 0x00000001) ? true : false);
	if (dwLoad & 0x00000002)
		Ctx.pStream->Read((char *)&Ctx.dwVersion, sizeof(DWORD));

	//	Scales

	if (Ctx.dwVersion >= 9)
		{
		Ctx.pStream->Read((char *)&Ctx.pSystem->m_rKlicksPerPixel, sizeof(Metric));
		Ctx.pStream->Read((char *)&Ctx.pSystem->m_rTimeScale, sizeof(Metric));
		}
	else
		{
		Ctx.pSystem->m_rKlicksPerPixel = KLICKS_PER_PIXEL;
		Ctx.pSystem->m_rTimeScale = TIME_SCALE;
		}

	//	Load the navigation paths (we load these before objects
	//	because objects might have references to paths)

	if (Ctx.dwVersion >= 10)
		{
		Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
		TListNode<CNavigationPath> *pInsertAt = &Ctx.pSystem->m_NavPaths;
		for (i = 0; i < (int)dwCount; i++)
			{
			CNavigationPath *pNavPath;
			CNavigationPath::CreateFromStream(Ctx, &pNavPath);
			pInsertAt->Insert(pNavPath);
			pInsertAt = pNavPath;
			}
		}

	//	Load all objects

	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	int iEncounterCount = 0;
	for (i = 0; i < (int)dwCount; i++)
		{
		//	Load the object

		CSpaceObject *pObj;
		try
			{
			CSpaceObject::CreateFromStream(Ctx, &pObj);
			}
		catch (...)
			{
			kernelDebugLogMessage("Error loading object");
			return ERR_FAIL;
			}

		//	Add this object to the map

		Ctx.ObjMap.AddEntry(pObj->GetIndex(), pObj);

		//	Update any previous objects that are waiting for this reference

		CIntArray *pList;
		if (Ctx.ForwardReferences.Lookup(pObj->GetIndex(), (CObject **)&pList) == NOERROR)
			{
			for (int j = 0; j < pList->GetCount(); j++)
				{
				CSpaceObject **pAddr = (CSpaceObject **)pList->GetElement(j);
				*pAddr = pObj;
				}

			Ctx.ForwardReferences.RemoveEntry(pObj->GetIndex(), NULL);
			}

		//	Set the system (note: this will change the index to the new
		//	system)

		pObj->AddToSystem(Ctx.pSystem);

		//	Ask the object if it has encounters

		if (pObj->GetRandomEncounterTable())
			iEncounterCount++;
		}

	//	If we've got left over references, then dump debug output

	if (Ctx.ForwardReferences.GetCount() > 0)
		{
		kernelDebugLogMessage("Save file error: %d undefined object reference(s)", Ctx.ForwardReferences.GetCount());

		for (i = 0; i < Ctx.ForwardReferences.GetCount(); i++)
			{
			int iID = Ctx.ForwardReferences.GetKey(i);
			CIntArray *pList = (CIntArray *)Ctx.ForwardReferences.GetValue(i);
			kernelDebugLogMessage("Reference: %d", iID);

			for (int j = 0; j < pList->GetCount(); j++)
				{
				CSpaceObject **pAddr = (CSpaceObject **)pList->GetElement(j);
				kernelDebugLogMessage("Address: %x", (DWORD)pAddr);
				}
			}

		ASSERT(false);
		}

	//	Load named objects table

	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		{
		CString sName;
		sName.ReadFromStream(Ctx.pStream);

		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		CSpaceObject *pObj;
		if (Ctx.ObjMap.Lookup((int)dwLoad, (CObject **)&pObj) != NOERROR)
			{
			ASSERT(false);
			kernelDebugLogMessage("Save file error: Unable to find named object: %s [%x]", sName.GetASCIIZPointer(), dwLoad);
			return ERR_FAIL;
			}

		Ctx.pSystem->NameObject(sName, pObj);
		}

	//	Load all timed events

	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		{
		CTimedEvent *pEvent;
		CTimedEvent::CreateFromStream(Ctx, &pEvent);
		Ctx.pSystem->AddTimedEvent(pEvent);
		}

	//	Load environment map

	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	if (dwCount)
		{
		CTileMap *pEnv;
		CTileMap::CreateFromStream(Ctx.pStream, &pEnv);
		ConvertSpaceEnvironmentToPointers(*pEnv, &Ctx.pSystem->m_pEnvironment);
		delete pEnv;
		}
	else
		Ctx.pSystem->m_pEnvironment = NULL;

	//	Create the background star field

	if (error = Ctx.pSystem->CreateStarField(STARFIELD_COUNT))
		return error;

	//	Compute some tables

	Ctx.pSystem->ComputeStars();
	Ctx.pSystem->ComputeRandomEncounters(iEncounterCount);
	
	//	Map the POV object

	if (retpObj)
		{
		if (Ctx.ObjMap.Lookup((int)dwObjID, (CObject **)retpObj) != NOERROR)
			{
			kernelDebugLogMessage("Save file error: Unable to find POV object: %x", dwObjID);

			//	Look for the player object

			bool bFound = false;
			for (i = 0; i < Ctx.pSystem->GetObjectCount(); i++)
				{
				CSpaceObject *pObj = Ctx.pSystem->GetObject(i);

				if (pObj && pObj->IsPlayer())
					{
					*retpObj = pObj;
					bFound = true;
					break;
					}
				}

			if (!bFound)
				{
				kernelDebugLogMessage("Save file error: Unable to find player ship");
				return ERR_FAIL;
				}

			return NOERROR;
			}
		}

	//	Tell all objects that the system has been loaded

	for (i = 0; i < Ctx.pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = Ctx.pSystem->GetObject(i);

		if (pObj)
			pObj->OnSystemLoaded();
		}

	//	Done

	*retpSystem = Ctx.pSystem;

	return NOERROR;
	}

ALERROR CSystem::CreateShip (DWORD dwClassID,
							 IShipController *pController,
							 CSovereign *pSovereign,
							 const CVector &vPos,
							 const CVector &vVel,
							 int iRotation,
							 CSpaceObject *pExitGate,
							 CShip **retpShip)

//	CreateShip
//
//	Creates a ship based on the class.
//
//	pController is owned by the ship if this call is successful.

	{
	ALERROR error;

	//	Find the class

	CShipClass *pClass = GetUniverse()->FindShipClass(dwClassID);
	if (pClass == NULL)
		return ERR_FAIL;

	//	Create the controller

	if (pController == NULL)
		{
		pController = new CStandardShipAI;
		if (pController == NULL)
			return ERR_MEMORY;
		}

	//	Create a new ship based on the class

	if (error = CShip::CreateFromClass(this, 
			pClass, 
			pController, 
			pSovereign, 
			vPos, 
			vVel, 
			iRotation, 
			retpShip))
		return error;

	pController->SetShipToControl(*retpShip);

	//	If we're coming out of a gate, set the timer

	if (pExitGate)
		{
		//	We keep on incrementing the timer as long as we are creating ships
		//	in the same tick.

		if (m_iTick != g_iGateTimerTick)
			{
			g_iGateTimer = 0;
			g_iGateTimerTick = m_iTick;
			}

		(*retpShip)->SetInGate(pExitGate, g_iGateTimer);
		g_iGateTimer += mathRandom(22, 44);
		}

	return NOERROR;
	}

ALERROR CSystem::CreateStarField (int iCount)

//	CreateStarField
//
//	Create the system's background star field

	{
	ALERROR error;
	int i;

	for (i = 0; i < iCount; i++)
		{
		CStar Star;

		Star.x = mathRandom(0, g_iStarFieldWidth);
		Star.y = mathRandom(0, g_iStarFieldHeight);
#ifdef BACKGROUND_OBJECTS
		Star.wDistance = mathRandom(4, 20);
#else
		Star.wDistance = mathRandom(2, 20);
#endif

		int iBrightness = 225 - Star.wDistance * mathRandom(1, 10);
		Star.wColor = CGImage::RGBColor(iBrightness + mathRandom(-25, 25),
				iBrightness + mathRandom(-25, 25),
				iBrightness + mathRandom(-25, 25));

		if (error = m_StarField.AppendStruct(&Star, NULL))
			return error;
		}

	return NOERROR;
	}

ALERROR CSystem::CreateStargate (CStationType *pType,
								 CVector &vPos,
								 const CString &sStargateID,
								 const CString &sDestNodeID,
								 const CString &sDestStargateID,
								 CSpaceObject **retpStation)

//	CreateStargate
//
//	Creates a stargate in the system

	{
	ALERROR error;
	CStation *pStation;

	CTopologyNode *pDestNode = g_pUniverse->FindTopologyNode(sDestNodeID);
	if (pDestNode == NULL)
		return ERR_FAIL;

	//	Create the station

	CSpaceObject *pObj;
	if (error = CreateStation(pType, vPos, &pObj))
		return error;

	pStation = pObj->AsStation();
	if (pStation == NULL)
		return ERR_FAIL;

	//	Create stargate stuff

	if (error = StargateCreated(pStation, sStargateID, sDestNodeID, sDestStargateID))
		return error;

	//	Set stargate properties (note: CreateStation also looks at objName and adds the name
	//	to the named-objects system table.)

	pStation->SetStargate(sDestNodeID, sDestStargateID);

	//	If we haven't already set the name, set the name of the stargate
	//	to include the name of the destination system

	if (pStation->GetName(NULL).IsBlank())
		pStation->SetName(strPatternSubst(CONSTLIT("%s Stargate"), pDestNode->GetSystemName().GetPointer()));

	//	Done

	if (retpStation)
		*retpStation = pStation;

	return NOERROR;
	}

ALERROR CSystem::CreateStation (CStationType *pType,
								CVector &vPos,
								CSpaceObject **retpStation)

//	CreateStation
//
//	Creates a station outside of a system definition

	{
	//	Generate context block

	SSystemCreateCtx Ctx;
	Ctx.pTopologyNode = GetTopology();
	Ctx.pSystem = this;
	Ctx.pLocalTables = m_pType->GetLocalSystemTables();
	Ctx.pGlobalTables = g_pUniverse->GetGlobalSystemTables();
	Ctx.pStats = NULL;

	//	Generate an orbit. First we look for the nearest object with
	//	an orbit.

	CSpaceObject *pBestObj = NULL;
	Metric rBestDist2 = g_InfiniteDistance * g_InfiniteDistance;
	for (int i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);
		if (pObj && pObj->GetMapOrbit())
			{
			Metric rDist2 = (pObj->GetPos() - vPos).Length2();
			if (rDist2 < rBestDist2)
				{
				pBestObj = pObj;
				rBestDist2 = rDist2;
				}
			}
		}

	//	If we found an object, create an orbit around it. Otherwise, orbit around origin

	CVector vCenter = (pBestObj ? pBestObj->GetPos() : CVector());
	Metric rRadius;
	Metric rAngle = VectorToPolarRadians(vPos - vCenter, &rRadius);
	Orbit NewOrbit(vCenter, rRadius, rAngle);

	//	Create the station

	return CreateStation(&Ctx,
			pType,
			vPos,
			NewOrbit,
			true,
			NULL,
			retpStation);
	}

ALERROR CSystem::CreateWeaponFire (CWeaponFireDesc *pDesc,
								   int iBonus,
								   DestructionTypes iCause,
								   const CDamageSource &Source,
								   const CVector &vPos,
								   const CVector &vVel,
								   int iDirection,
								   CSpaceObject *pTarget,
								   CSpaceObject **retpShot)

//	CreateWeaponFire
//
//	Creates a shot

	{
	switch (pDesc->m_iFireType)
		{
		case ftBeam:
			{
			CBeam *pBeam;

			CBeam::Create(this,
					pDesc,
					iBonus,
					iCause,
					Source,
					vPos,
					vVel,
					iDirection,
					&pBeam);

			if (retpShot)
				*retpShot = pBeam;

			break;
			}

		case ftMissile:
			{
			CMissile *pMissile;

			CMissile::Create(this,
					pDesc,
					iBonus,
					iCause,
					Source,
					vPos,
					vVel,
					iDirection,
					pTarget,
					&pMissile);

			if (retpShot)
				*retpShot = pMissile;

			break;
			}

		case ftArea:
			{
			CAreaDamage *pArea;

			CAreaDamage::Create(this,
					pDesc,
					iBonus,
					iCause,
					Source,
					vPos,
					vVel,
					&pArea);

			if (retpShot)
				*retpShot = pArea;
			break;
			}

		case ftParticles:
			{
			CParticleDamage *pParticles;

			CParticleDamage::Create(this,
					pDesc,
					iBonus,
					iCause,
					Source,
					vPos,
					vVel,
					iDirection,
					&pParticles);

			if (retpShot)
				*retpShot = pParticles;
			break;
			}

		case ftRadius:
			{
			CRadiusDamage *pRadius;

			CRadiusDamage::Create(this,
					pDesc,
					iBonus,
					iCause,
					Source,
					vPos,
					vVel,
					pTarget,
					&pRadius);

			if (retpShot)
				*retpShot = pRadius;
			break;
			}

		default:
			ASSERT(FALSE);
		}

	return NOERROR;
	}

ALERROR CSystem::CreateWeaponFragments (CWeaponFireDesc *pDesc,
									    int iBonus,
									    DestructionTypes iCause,
									    const CDamageSource &Source,
									    CSpaceObject *pTarget,
									    const CVector &vPos,
										const CVector &vVel,
									    CSpaceObject *pMissileSource)

//	CreateWeaponFragments
//
//	Creates the fragments from a fragmentation weapon

	{
	ALERROR error;
	int i;

	CWeaponFireDesc::SFragmentDesc *pFragDesc = pDesc->GetFirstFragment();
	while (pFragDesc)
		{
		int iFragmentCount = pFragDesc->Count.Roll();
		if (iFragmentCount > 0)
			{
			int *iAngle = new int [iFragmentCount];

			//	If we have lots of fragments then we just pick random angles

			if (iFragmentCount > 90)
				{
				for (i = 0; i < iFragmentCount; i++)
					iAngle[i] = mathRandom(0, 359);
				}

			//	Otherwise, we try to distribute evenly

			else
				{
				int iAngleOffset = mathRandom(0, 359);
				int iAngleVar = 90 / iFragmentCount;

				//	Compute angles for each fragment

				int iAngleInc = 360 / iFragmentCount;
				for (i = 0; i < iFragmentCount; i++)
					iAngle[i] = (360 + iAngleOffset + (iAngleInc * i) + mathRandom(-iAngleVar, iAngleVar)) % 360;
				}

			//	For multitargets, we need to find a target 
			//	for each fragment

			if (pFragDesc->bMIRV)
				{
				CIntArray TargetList;
				int iFound = pMissileSource->GetNearestVisibleEnemies(iFragmentCount, MAX_TARGET_RANGE, &TargetList);

				Metric rSpeed = pFragDesc->pDesc->GetInitialSpeed();

				for (i = 0; i < iFound; i++)
					{
					CSpaceObject *pTarget = (CSpaceObject *)TargetList.GetElement(i);

					//	Calculate direction to fire in

					CVector vTarget = pTarget->GetPos() - vPos;
					Metric rTimeToIntercept = CalcInterceptTime(vTarget, pTarget->GetVel(), rSpeed);
					CVector vInterceptPoint = vTarget + pTarget->GetVel() * rTimeToIntercept;
					iAngle[i] = VectorToPolar(vInterceptPoint, NULL);
					}
				}

			//	The initial velocity is the velocity of the missile
			//	(unless we are MIRVed)

			CVector vInitVel;
			if (!pFragDesc->bMIRV)
				vInitVel = vVel;

			//	Create the fragments

			for (i = 0; i < iFragmentCount; i++)
				{
				CSpaceObject *pNewObj;
				Metric rSpeed = pFragDesc->pDesc->GetInitialSpeed();

				if (error = CreateWeaponFire(pFragDesc->pDesc,
						iBonus,
						iCause,
						Source,
						vPos,
						vInitVel + PolarToVector(iAngle[i], rSpeed),
						iAngle[i],
						pTarget,
						&pNewObj))
					return error;

				//	Preserve automated weapon flag

				if (pMissileSource->IsAutomatedWeapon())
					pNewObj->SetAutomatedWeapon();
				}

			delete iAngle;
			}

		pFragDesc = pFragDesc->pNext;
		}

	return NOERROR;
	}

void CSystem::DestroyObject (int iIndex, DestructionTypes iCause, CSpaceObject *pCause)

//	DestroyObject
//
//	Destroys the given object created by AddToSystem

	{
	CSpaceObject *pObjDestroyed = GetObject(iIndex);
	RemoveObject(iIndex, iCause, pCause, NULL);
	delete pObjDestroyed;
	}

void CSystem::FlushEnemyObjectCache (void)

//	FlushEnemyObjectCache
//
//	Flush the enemy object cache

	{
	int i;

	for (i = 0; i < g_pUniverse->GetSovereignCount(); i++)
		g_pUniverse->GetSovereign(i)->FlushEnemyObjectCache();
	}

int CSystem::GetLevel (void)

//	GetLevel
//
//	Returns the level of the system

	{
	if (m_pTopology)
		return m_pTopology->GetLevel();
	else
		return 1;
	}

CSpaceObject *CSystem::GetNamedObject (const CString &sName)

//	GetNamedObject
//
//	Returns the object by name

	{
	CSpaceObject *pPoint;

	if (m_NamedObjects.Lookup(sName, (CObject **)&pPoint) == NOERROR)
		return pPoint;
	else
		return NULL;
	}

CNavigationPath *CSystem::GetNavPath (CSovereign *pSovereign, CSpaceObject *pStart, CSpaceObject *pEnd)

//	GetNavPath
//
//	Returns the navigation path for the given parameters

	{
	CNavigationPath *pNext = m_NavPaths.GetNext();
	while (pNext)
		{
		if (pNext->Matches(pSovereign, pStart, pEnd))
			return pNext;

		pNext = pNext->GetNext();
		}

	//	If we cannot find an appropriate path, we create a new one

	CNavigationPath *pPath;
	CNavigationPath::Create(this, pSovereign, pStart, pEnd, &pPath);

	m_NavPaths.Insert(pPath);

	return pPath;
	}

CNavigationPath *CSystem::GetNavPathByID (DWORD dwID)

//	GetNavPathByID
//
//	Returns the nav path with the given ID (or NULL if not found)

	{
	CNavigationPath *pNext = m_NavPaths.GetNext();
	while (pNext)
		{
		if (pNext->GetID() == dwID)
			return pNext;

		pNext = pNext->GetNext();
		}

	return NULL;
	}

CSpaceEnvironmentType *CSystem::GetSpaceEnvironment (int xTile, int yTile)

//	GetSpaceEnvironment
//
//	Returns the given tile

	{
	if (m_pEnvironment)
		return (CSpaceEnvironmentType *)m_pEnvironment->GetTile(xTile, yTile);
	else
		return NULL;
	}

CSpaceEnvironmentType *CSystem::GetSpaceEnvironment (const CVector &vPos, int *retxTile, int *retyTile)

//	GetSpaceEnvironment
//
//	Returns the tile at the given position

	{
	if (m_pEnvironment)
		{
		int x, y;

		VectorToTile(vPos, &x, &y);

		if (retxTile)
			*retxTile = x;

		if (retyTile)
			*retyTile = y;

		return (CSpaceEnvironmentType *)m_pEnvironment->GetTile(x, y);
		}
	else
		{
		if (retxTile)
			*retxTile = -1;

		if (retyTile)
			*retyTile = -1;

		return NULL;
		}
	}

CTopologyNode *CSystem::GetStargateDestination (const CString &sStargate, CString *retsEntryPoint)

//	GetStargateDestination
//
//	Get the destination topology node and entry point

	{
	return m_pTopology->GetGateDest(sStargate, retsEntryPoint);
	}

bool CSystem::IsStationInSystem (CStationType *pType)

//	IsStationInSystem
//
//	Returns TRUE if the given station type has already been created in the system

	{
	int i;

	for (i = 0; i < GetObjectCount(); i++)
		{
		for (i = 0; i < GetObjectCount(); i++)
			{
			CSpaceObject *pObj = GetObject(i);
			if (pObj && pObj->GetEncounterInfo() == pType)
				return true;
			}
		}

	return false;
	}

void CSystem::LoadImages (void)

//	LoadImages
//
//	Load images for use

	{
	int i;

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj)
			pObj->LoadImages();
		}
	}

void CSystem::MarkImages (void)

//	MarkImages
//
//	Mark images in use

	{
	int i;

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj)
			pObj->MarkImages();
		}
	}

void CSystem::NameObject (const CString &sName, CSpaceObject *pObj)

//	NameObject
//
//	Name an object

	{
	m_NamedObjects.AddEntry(sName, pObj);
	}

void CSystem::PaintDestinationMarker (CG16bitImage &Dest, int x, int y, CSpaceObject *pObj, CSpaceObject *pCenter)

//	PaintDestinationMarker
//
//	Paints a directional indicator

	{
	CVector vPos;

	//	Figure out the bearing for the destination object
	//	(We want the angle of the center with respect to the object because we
	//	start at the edge of the screen and point inward).

	int iBearing = VectorToPolar(pCenter->GetPos() - pObj->GetPos());

	//	Generate a set of points for the directional indicator

	SPoint Poly[5];

	//	Start at the origin

	Poly[0].x = 0;
	Poly[0].y = 0;

	//	Do one side first

	vPos = PolarToVector(iBearing + 30, ENHANCED_SRS_BLOCK_SIZE);
	Poly[1].x = (int)vPos.GetX();
	Poly[1].y = -(int)vPos.GetY();

	vPos = vPos + PolarToVector(iBearing, 3 * ENHANCED_SRS_BLOCK_SIZE);
	Poly[2].x = (int)vPos.GetX();
	Poly[2].y = -(int)vPos.GetY();

	//	The other side

	vPos = PolarToVector(iBearing + 330, ENHANCED_SRS_BLOCK_SIZE);
	CVector vPos2 = vPos + PolarToVector(iBearing, 3 * ENHANCED_SRS_BLOCK_SIZE);

	Poly[3].x = (int)vPos2.GetX();
	Poly[3].y = -(int)vPos2.GetY();

	Poly[4].x = (int)vPos.GetX();
	Poly[4].y = -(int)vPos.GetY();

	//	Paint the directional indicator

	CG16bitRegion Region;
	Region.CreateFromConvexPolygon(5, Poly);
	COLORREF wColor = (pObj->IsEnemy(pCenter) ? ENHANCED_SRS_ENEMY_COLOR : ENHANCED_SRS_FRIEND_COLOR);
	Region.Fill(Dest, x, y, wColor);

	//	Paint the text

	CG16bitFont &Font = g_pUniverse->GetSignFont();
	vPos = PolarToVector(iBearing, 5 * ENHANCED_SRS_BLOCK_SIZE);
	int xText = x + (int)vPos.GetX();
	int yText = y - (int)vPos.GetY();

	if (iBearing > 180)
		yText += 2 * ENHANCED_SRS_BLOCK_SIZE;
	else
		yText -= (2 * ENHANCED_SRS_BLOCK_SIZE) + Font.GetHeight();

	CString sName;
	if (pObj->IsIdentified())
		sName = pObj->GetNounPhrase(0);
	else if (pCenter->IsEnemy(pObj))
		sName = CONSTLIT("Unknown Hostile");
	else
		sName = CONSTLIT("Unknown Friendly");

	int cxText = Font.MeasureText(sName);

	//	Center text

	const RECT &rcClip = Dest.GetClipRect();
	xText = xText - cxText / 2;
	xText = Max((int)rcClip.left, xText);
	xText = Min((int)(rcClip.right - cxText), xText);

	//	Paint
	Font.DrawText(Dest, xText, yText, wColor, sName);
	}

void CSystem::PaintStarField(CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pCenter, Metric rKlicksPerPixel)

//	PaintStarField
//
//	Paints the system star field

	{
	int i;

	//	Get the absolute position of the center (we make sure that it is positive
	//	so that we can do a mod)

	int xCenter = (int)(pCenter->GetPos().GetX() / rKlicksPerPixel);
	int yCenter = (int)(pCenter->GetPos().GetY() / rKlicksPerPixel);

	//	Paint each star

	for (i = 0; i < m_StarField.GetCount(); i++)
		{
		CStar *pStar = (CStar *)m_StarField.GetStruct(i);

		//	Adjust the coordinates of the star based on the position
		//	of the center and the distance

		int x = (pStar->x - 4 * xCenter / (pStar->wDistance * pStar->wDistance)) % g_iStarFieldWidth;
		if (x < 0)
			x += g_iStarFieldWidth;
		int y = (pStar->y + 4 * yCenter / (pStar->wDistance * pStar->wDistance)) % g_iStarFieldHeight;
		if (y < 0)
			y += g_iStarFieldHeight;

		//	Blt the star

		if (x < RectWidth(rcView) && y < RectHeight(rcView))
			Dest.DrawPixel(rcView.left + x, rcView.top + y, pStar->wColor);
		}
	}

void CSystem::PaintViewport (CG16bitImage &Dest, 
							 const RECT &rcView, 
							 CSpaceObject *pCenter, 
							 bool bEnhanced)

//	PaintViewport
//
//	Paints the system in the viewport

	{
	int i;
	int iLayer;

	//	Figure out what color space should be. Space gets lighter as we get
	//	near the central star

	COLORREF rgbSpaceColor = CalculateSpaceColor(pCenter);

	//	Clear the rect

	Dest.SetClipRect(rcView);
	Dest.FillRGB(rcView.left, rcView.top, RectWidth(rcView), RectHeight(rcView), rgbSpaceColor);

	//	Paint the star field

	PaintStarField(Dest, rcView, pCenter, g_KlicksPerPixel);

	//	Figure out the boundary of the viewport in system coordinates. 

	CVector vDiagonal(g_KlicksPerPixel * (Metric)(RectWidth(rcView)) / 2,
				g_KlicksPerPixel * (Metric)(RectHeight(rcView)) / 2);
	CVector vUR = pCenter->GetPos() + vDiagonal;
	CVector vLL = pCenter->GetPos() - vDiagonal;

	//	Figure out the extended boundaries. This is used for enhanced display.

	CVector vEnhancedDiagonal(g_KlicksPerPixel * (Metric)(2 * RectWidth(rcView)),
			g_KlicksPerPixel * (Metric)(2 * RectHeight(rcView)));

	CVector vEnhancedUR = pCenter->GetPos() + vEnhancedDiagonal;
	CVector vEnhancedLL = pCenter->GetPos() - vEnhancedDiagonal;

	//	Compose the paint context

	SViewportPaintCtx Ctx;
	Ctx.wSpaceColor = rgbSpaceColor;

	//	Compute the transformation to map world coordinates to the viewport

	int xCenter = rcView.left + RectWidth(rcView) / 2;
	int yCenter = rcView.top + RectHeight(rcView) / 2;
	Ctx.XForm = ViewportTransform(pCenter->GetPos(), g_KlicksPerPixel, xCenter, yCenter);

#ifdef BACKGROUND_OBJECTS
	//	Figure out the background object boundaries.

	CVector vBackgroundDiagonal(g_KlicksPerPixel * (Metric)(BACKGROUND_OBJECT_FACTOR * RectWidth(rcView)),
			g_KlicksPerPixel * (Metric)(BACKGROUND_OBJECT_FACTOR * RectHeight(rcView)));

	CVector vBackgroundUR = pCenter->GetPos() + vBackgroundDiagonal;
	CVector vBackgroundLL = pCenter->GetPos() - vBackgroundDiagonal;

	//	Compute the background object transform

	SViewportPaintCtx BackgroundCtx;
	BackgroundCtx.wSpaceColor = Ctx.wSpaceColor;
	BackgroundCtx.XForm = ViewportTransform(pCenter->GetPos(), BACKGROUND_OBJECT_FACTOR * g_KlicksPerPixel, xCenter, yCenter);
#endif

	//	Paint any space environment

	if (m_pEnvironment)
		{
		int x, y, x1, y1, x2, y2;

		VectorToTile(vUR, &x2, &y1);
		VectorToTile(vLL, &x1, &y2);
		
		//	Increase bounds (so we can paint the edges)

		x1--; y1--;
		x2++; y2++;

		for (x = x1; x <= x2; x++)
			for (y = y1; y <= y2; y++)
				{
				CSpaceEnvironmentType *pEnv = (CSpaceEnvironmentType *)m_pEnvironment->GetTile(x, y);
				if (pEnv)
					{
					int xCenter, yCenter;
					CVector vCenter = TileToVector(x, y);
					Ctx.XForm.Transform(vCenter, &xCenter, &yCenter);

					pEnv->Paint(Dest, xCenter, yCenter);
					}
				}
		}

	//	Paint background objects

#ifdef BACKGROUND_OBJECTS
	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj && pObj->GetPaintLayer() == layerBackground)
			{
			if (pObj->InBox(vBackgroundUR, vBackgroundLL))
				{
				//	Figure out the position of the object in pixels

				int x, y;
				BackgroundCtx.XForm.Transform(pObj->GetPos(), &x, &y);

				//	Paint the object in the viewport

				BackgroundCtx.pObj = pObj;
				pObj->Paint(Dest, 
						x,
						y,
						BackgroundCtx);
				}
			}
		}
#endif

	//	Compute the bounds relative to the center

	RECT rcBounds;
	rcBounds.left = rcView.left - xCenter + (ENHANCED_SRS_BLOCK_SIZE / 2);
	rcBounds.top = rcView.top - yCenter + (ENHANCED_SRS_BLOCK_SIZE / 2);
	rcBounds.right = rcView.right - xCenter - (ENHANCED_SRS_BLOCK_SIZE / 2);
	rcBounds.bottom = rcView.bottom - yCenter - (ENHANCED_SRS_BLOCK_SIZE / 2);

	//	Loop over all objects

	for (iLayer = layerSpace; iLayer < layerCount; iLayer++)
		for (i = 0; i < GetObjectCount(); i++)
			{
			CSpaceObject *pObj = GetObject(i);

			if (pObj && pObj->GetPaintLayer() == iLayer)
				{
				//	If we're in range then paint the object for real

				if (pObj->InBox(vUR, vLL))
					{
					//	Figure out the position of the object in pixels

					int x, y;
					Ctx.XForm.Transform(pObj->GetPos(), &x, &y);

					//	Paint the object in the viewport

					Ctx.pObj = pObj;
					pObj->Paint(Dest, 
							x,
							y,
							Ctx);
					}

				//	Otherwise, if we're in the enhanced display range
				//	then paint an indicator at the edge of the screen
				//	(note that player targets don't have to be scaleShip or scaleStructure)

				else if ((bEnhanced
								&& (pObj->GetScale() == scaleShip || pObj->GetScale() == scaleStructure)
								&& pObj->PosInBox(vEnhancedUR, vEnhancedLL))
							|| pObj->IsPlayerTarget())
					{
					//	Figure out the position of the object in pixels
					//	relative to the center of the screen

					int x, y;
					Ctx.XForm.Transform(pObj->GetPos(), &x, &y);
					x = x - xCenter;
					y = y - yCenter;

					//	Now clip the position to the side of the screen

					if (x >= rcBounds.right)
						{
						y = y * (rcBounds.right - 1) / x;
						x = rcBounds.right - 1;
						}
					else if (x < rcBounds.left)
						{
						y = y * (rcBounds.left) / x;
						x = rcBounds.left;
						}

					if (y >= rcBounds.bottom)
						{
						x = x * (rcBounds.bottom - 1) / y;
						y = rcBounds.bottom - 1;
						}
					else if (y < rcBounds.top)
						{
						x = x * rcBounds.top / y;
						y = rcBounds.top;
						}

					//	Draw the indicator

					if (pObj->IsPlayerTarget())
						{
						PaintDestinationMarker(Dest, 
								xCenter + x, 
								yCenter + y,
								pObj,
								pCenter);
						}
					else
						{
						COLORREF wColor;
						if (pObj->IsEnemy(pCenter))
							wColor = CGImage::RGBColor(255, 0, 0);
						else
							wColor = CGImage::RGBColor(0, 255, 0);

						Dest.Fill(xCenter + x - (ENHANCED_SRS_BLOCK_SIZE / 2), 
								yCenter + y - (ENHANCED_SRS_BLOCK_SIZE / 2),
								ENHANCED_SRS_BLOCK_SIZE, 
								ENHANCED_SRS_BLOCK_SIZE, 
								wColor);
						}
					}
				}
			}

	Dest.ResetClipRect();
	}

void CSystem::PaintViewportGrid (CG16bitImage &Dest, const RECT &rcView, const ViewportTransform &Trans, const CVector &vCenter, Metric rGridSize)

//	PaintViewportGrid
//
//	Paints a grid

	{
	int cxWidth = RectWidth(rcView);
	int cyHeight = RectHeight(rcView);

	//	Figure out where the center is

	int xCenter, yCenter;
	Trans.Transform(vCenter, &xCenter, &yCenter);

	//	Figure out the grid spacing

	int xSpacing, ySpacing;
	Trans.Transform(vCenter + CVector(rGridSize, -rGridSize), &xSpacing, &ySpacing);
	xSpacing -= xCenter;
	ySpacing -= yCenter;
	ySpacing = xSpacing;

	if (xSpacing <= 0 || ySpacing <= 0)
		return;

	//	Find the x coordinate of the left-most grid line

	int xStart;
	if (xCenter < rcView.left)
		xStart = xCenter + ((rcView.left - xCenter) / xSpacing) * xSpacing;
	else
		xStart = xCenter - ((xCenter - rcView.left) / xSpacing) * xSpacing;

	//	Paint vertical grid lines

	int x = xStart;
	while (x < rcView.right)
		{
		Dest.FillColumn(x, rcView.top, cyHeight, RGB_GRID_LINE);
		x += xSpacing;
		}

	//	Find the y coordinate of the top-most grid line

	int yStart;
	if (yCenter < rcView.top)
		yStart = yCenter + ((rcView.top - yCenter) / ySpacing) * ySpacing;
	else
		yStart = yCenter - ((yCenter - rcView.top) / ySpacing) * ySpacing;

	//	Paint horizontal grid lines

	int y = yStart;
	while (y < rcView.bottom)
		{
		Dest.FillLine(rcView.left, y, cxWidth, RGB_GRID_LINE);
		y += ySpacing;
		}
	}

void CSystem::PaintViewportObject (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pCenter, CSpaceObject *pObj)

//	PaintViewportObject
//
//	Paints a single object

	{
	//	Figure out the boundary of the viewport in system coordinates

	int xCenter = rcView.left + RectWidth(rcView) / 2;
	int yCenter = rcView.top + RectHeight(rcView) / 2;

	CVector vDiagonal(g_KlicksPerPixel * (Metric)(RectWidth(rcView) + 256) / 2,
				g_KlicksPerPixel * (Metric)(RectHeight(rcView) + 256) / 2);
	CVector vUR = pCenter->GetPos() + vDiagonal;
	CVector vLL = pCenter->GetPos() - vDiagonal;

	//	Compute the transformation to map world coordinates to the viewport

	SViewportPaintCtx Ctx;
	COLORREF rgbSpaceColor = CalculateSpaceColor(pCenter);
	Ctx.wSpaceColor = rgbSpaceColor;
	Ctx.XForm = ViewportTransform(pCenter->GetPos(), g_KlicksPerPixel, xCenter, yCenter);

	//	Paint object

	if (pObj && pObj->InBox(vUR, vLL))
		{
		//	Figure out the position of the object in pixels

		int x, y;
		Ctx.XForm.Transform(pObj->GetPos(), &x, &y);

		//	Paint the object in the viewport

		Ctx.pObj = pObj;
		pObj->Paint(Dest, 
				x,
				y,
				Ctx);
		}
	}

void CSystem::PaintViewportLRS (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pCenter, bool *retbNewEnemies)

//	PaintViewportLRS
//
//	Paints an LRS from the point of view of the given object. We assume that
//	the destination bitmap is already clear; we just draw on top of it.

	{
	int i;
	Metric rKlicksPerPixel = g_LRSRange * 2 / RectWidth(rcView);

	//	Figure out the boundary of the viewport in system coordinates. We generate
	//	a viewport for each detection range 1-5.

	CVector vUR[RANGE_INDEX_COUNT];
	CVector vLL[RANGE_INDEX_COUNT];

	CVector vDiagonal(rKlicksPerPixel * (Metric)(RectWidth(rcView)) / 2,
				rKlicksPerPixel * (Metric)(RectHeight(rcView)) / 2);
	for (i = 0; i < RANGE_INDEX_COUNT; i++)
		{
		Metric rRange = RangeIndex2Range(i);

		//	Player ship can't see beyond LRS range

		if (rRange > g_LRSRange)
			rRange = g_LRSRange;

		CVector vRange(rRange, rRange);
		vUR[i] = pCenter->GetPos() + vRange;
		vLL[i] = pCenter->GetPos() - vRange;
		}

	int iPerception = pCenter->GetPerception();

	//	Compute the transformation to map world coordinates to the viewport

	int xCenter = rcView.left + RectWidth(rcView) / 2;
	int yCenter = rcView.top + RectHeight(rcView) / 2;
	ViewportTransform Trans(pCenter->GetPos(), rKlicksPerPixel, xCenter, yCenter);

	//	Paint any space environment

	if (m_pEnvironment)
		{
		int x, y, x1, y1, x2, y2;

		VectorToTile(vUR[0], &x2, &y1);
		VectorToTile(vLL[0], &x1, &y2);
		
		//	Increase bounds (so we can paint the edges)

		x1--; y1--;
		x2++; y2++;

		for (x = x1; x <= x2; x++)
			for (y = y1; y <= y2; y++)
				{
				CSpaceEnvironmentType *pEnv = (CSpaceEnvironmentType *)m_pEnvironment->GetTile(x, y);
				if (pEnv)
					{
					int xCenter, yCenter;
					CVector vCenter = TileToVector(x, y);
					Trans.Transform(vCenter, &xCenter, &yCenter);

					pEnv->PaintLRS(Dest, xCenter, yCenter);
					}
				}
		}

	//	Loop over all objects

	bool bNewEnemies = false;
	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj)
			{
			int iRange = pObj->GetDetectionRangeIndex(iPerception);

			if (pObj->InBox(vUR[iRange], vLL[iRange]))
				{
				//	Figure out the position of the object in pixels

				int x, y;
				Trans.Transform(pObj->GetPos(), &x, &y);

				//	Paint the object in the viewport

				pObj->PaintLRS(Dest, x, y, Trans);

				//	This object is now in the LRS

				bool bNewInLRS = pObj->SetPOVLRS();
				if (bNewInLRS 
						&& pCenter->IsEnemy(pObj) 
						&& pObj->GetCategory() == CSpaceObject::catShip)
					bNewEnemies = true;
				}
			else
				{
				//	This object is not in the LRS

				pObj->ClearPOVLRS();
				}
			}
		}

	//	If new enemies have appeared in LRS, tell the POV

	if (retbNewEnemies)
		*retbNewEnemies = bNewEnemies;
	}

void CSystem::PaintViewportMap (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pCenter, Metric rMapScale)

//	PaintViewportMap
//
//	Paints a system map

	{
	int i;
	int x, y;

	//	Clear the rect

	Dest.FillRGB(rcView.left, rcView.top, RectWidth(rcView), RectHeight(rcView), g_rgbSpaceColor);

	//	Figure out the boundary of the viewport in system coordinates

	int xCenter = rcView.left + RectWidth(rcView) / 2;
	int yCenter = rcView.top + RectHeight(rcView) / 2;

	CVector vDiagonal(rMapScale * (Metric)(RectWidth(rcView) + 256) / 2,
				rMapScale * (Metric)(RectHeight(rcView) + 256) / 2);
	CVector vUR = pCenter->GetPos() + vDiagonal;
	CVector vLL = pCenter->GetPos() - vDiagonal;

	//	Compute the transformation to map world coordinates to the viewport

	ViewportTransform Trans(pCenter->GetPos(), 
			rMapScale, 
			rMapScale * MAP_VERTICAL_ADJUST,
			xCenter, 
			yCenter);

	//	Paint the grid

	PaintViewportGrid(Dest, rcView, Trans, CVector(), 100.0 * LIGHT_SECOND);

	//	Paint space environment

	if (m_pEnvironment)
		{
		STileMapEnumerator k;
		while (m_pEnvironment->HasMore(k))
			{
			DWORD dwTile;
			int xTile;
			int yTile;

			m_pEnvironment->GetNext(k, &xTile, &yTile, &dwTile);
			CVector vPos = TileToVector(xTile, yTile);

			Trans.Transform(vPos, &x, &y);
			Dest.DrawPixel(x, y, CGImage::RGBColor(255,0,255));
			}
		}

	//	Paint all planets and stars first

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj 
				//&& pObj->InBox(vUR, vLL) 
				&& (pObj->GetScale() == scaleStar
					|| pObj->GetScale() == scaleWorld))
			{
			//	Figure out the position of the object in pixels

			Trans.Transform(pObj->GetPos(), &x, &y);

			//	Paint the object in the viewport

			pObj->PaintMap(Dest, 
					x,
					y,
					Trans);
			}
		}

	//	Paint all structures next

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj 
				&& pObj->InBox(vUR, vLL) 
				&& (pObj->GetScale() == scaleStructure
					|| pObj->GetScale() == scaleShip))
			{
			//	Figure out the position of the object in pixels

			Trans.Transform(pObj->GetPos(), &x, &y);

			//	Paint the object in the viewport

			pObj->PaintMap(Dest, 
					x,
					y,
					Trans);
			}
		}

	//	Paint the POV

	Trans.Transform(pCenter->GetPos(), &x, &y);
	pCenter->PaintMap(Dest, x, y, Trans);
	}

void CSystem::PaintViewportMapObject (CG16bitImage &Dest, const RECT &rcView, CSpaceObject *pCenter, CSpaceObject *pObj)

//	PaintViewportMapObject
//
//	Paints a system map object

	{
	int x, y;

	//	Figure out the boundary of the viewport in system coordinates

	int xCenter = rcView.left + RectWidth(rcView) / 2;
	int yCenter = rcView.top + RectHeight(rcView) / 2;

	CVector vDiagonal(g_MapKlicksPerPixel * (Metric)(RectWidth(rcView) + 256) / 2,
				g_MapKlicksPerPixel * (Metric)(RectHeight(rcView) + 256) / 2);
	CVector vUR = pCenter->GetPos() + vDiagonal;
	CVector vLL = pCenter->GetPos() - vDiagonal;

	//	Compute the transformation to map world coordinates to the viewport

	ViewportTransform Trans(pCenter->GetPos(), 
			g_MapKlicksPerPixel, 
			g_MapKlicksPerPixel * MAP_VERTICAL_ADJUST,
			xCenter, 
			yCenter);

	//	Paint the obj

	Trans.Transform(pObj->GetPos(), &x, &y);
	pObj->PaintMap(Dest, x, y, Trans);
	}

void CSystem::PlayerEntered (CSpaceObject *pPlayer)

//	PlayerEntered
//
//	Tell all objects that the player has entered the system

	{
	int i;

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj && pObj != pPlayer)
			pObj->OnPlayerObj(pPlayer);
		}
	}

void CSystem::ReadObjRefFromStream (SLoadCtx &Ctx, CSpaceObject **retpObj)

//	ReadObjRefFromStream
//
//	Reads the object reference from the stream

	{
	//	Initialize

	*retpObj = NULL;

	//	Load the index

	int iIndex;
	Ctx.pStream->Read((char *)&iIndex, sizeof(DWORD));
	if (iIndex == -1)
		return;

	//	Lookup the index in the map

	if (Ctx.ObjMap.Lookup(iIndex, (CObject **)retpObj) == NOERROR)
		return;

	//	If we could not find it, add the return pointer to a map so that
	//	we can fill it in later (when we load the actual object)

	CIntArray *pList;
	if (Ctx.ForwardReferences.Lookup(iIndex, (CObject **)&pList) != NOERROR)
		{
		pList = new CIntArray;
		Ctx.ForwardReferences.AddEntry(iIndex, pList);
		}

	pList->AppendElement((int)retpObj, NULL);
	}

void CSystem::ReadSovereignRefFromStream (SLoadCtx &Ctx, CSovereign **retpSovereign)

//	ReadSovereignRefFromStream
//
//	Reads the sovereign reference

	{
	DWORD dwUNID;
	Ctx.pStream->Read((char *)&dwUNID, sizeof(DWORD));
	if (dwUNID == 0xffffffff)
		{
		*retpSovereign = NULL;
		return;
		}

	*retpSovereign = g_pUniverse->FindSovereign(dwUNID);
	}

void CSystem::RemoveObject (int iIndex, DestructionTypes iCause, CSpaceObject *pCause, CSpaceObject *pWreck)

//	RemoveObject
//
//	Removes the object from the system without destroying it

	{
	CSpaceObject *pObjDestroyed = GetObject(iIndex);
	int i;

	//	Tell all other objects that the given object was destroyed

	if (pObjDestroyed->NotifyOthersWhenDestroyed())
		{
		SDestroyCtx Ctx;
		Ctx.pObj = pObjDestroyed;
		Ctx.pDestroyer = (pCause ? pCause->GetDamageCause() : NULL);
		Ctx.pWreck = pWreck;
		Ctx.iCause = iCause;

		for (i = 0; i < GetObjectCount(); i++)
			{
			CSpaceObject *pObj = GetObject(i);

			if (pObj && pObj != pObjDestroyed)
				pObj->OnObjDestroyed(Ctx);
			}
		}

	//	Check to see if the object being destroyed was held by
	//	a timed encounter

	RemoveTimersForObj(pObjDestroyed);

	//	Check to see if this was the POV

	if (pObjDestroyed == g_pUniverse->GetPOV())
		{
		CPOVMarker *pMarker;
		CPOVMarker::Create(this, pObjDestroyed->GetPos(), NullVector, &pMarker);
		g_pUniverse->SetPOV(pMarker);
		}

	m_AllObjects.ReplaceObject(iIndex, NULL, false);

	FlushEnemyObjectCache();

	//	Debug code to see if we ever delete a barrier in the middle of move

#ifdef DEBUG_PROGRAMSTATE
	if (g_iProgramState == psUpdatingMove)
		{
		if (pObjDestroyed->IsBarrier())
			{
			CString sObj = CONSTLIT("ERROR: Destroying barrier during move.\r\n");

			ReportCrashObj(&sObj, pObjDestroyed);
			kernelDebugLogMessage(sObj.GetASCIIZPointer());

#ifdef DEBUG
			DebugBreak();
#endif
			}
		}
#endif
	}

void CSystem::RemoveTimersForObj (CSpaceObject *pObj)

//	RemoveTimersForObj
//
//	Remove timers for the given object

	{
	int i;

	for (i = 0; i < GetTimedEventCount(); i++)
		{
		CTimedEvent *pEvent = GetTimedEvent(i);
		if (pEvent->OnObjDestroyed(pObj))
			{
			m_TimedEvents.RemoveObject(i);
			i--;
			}
		}
	}

void CSystem::RestartTime (void)

//	RestartTime
//
//	Restart time for all

	{
	for (int i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj)
			pObj->RestartTime();
		}
	}

ALERROR CSystem::SaveToStream (IWriteStream *pStream)

//	SaveToStream
//
//	Save the system to a stream
//
//	DWORD		m_dwID
//	DWORD		m_iTick
//	DWORD		m_iTimeStopped
//	CString		m_sName
//	CString		Topology node ID
//	DWORD		(unused)
//	DWORD		m_iNextEncounter
//	DWORD		flags
//	DWORD		SAVE VERSION (only if [flags & 0x02])
//	Metric		m_rKlicksPerPixel
//	Metric		m_rTimeScale
//
//	DWORD		Number of CNavigationPath
//	CNavigationPath
//
//	DWORD		Number of objects
//	CSpaceObject
//
//	DWORD		Number of named objects
//	CString		entrypoint: name
//	DWORD		entrypoint: CSpaceObject ref
//
//	DWORD		Number of timed events
//	CTimedEvent
//
//	DWORD		Number of environment maps
//	CTileMap

	{
	int i;
	DWORD dwSave;

	//	Write basic data

	pStream->Write((char *)&m_dwID, sizeof(DWORD));
	pStream->Write((char *)&m_iTick, sizeof(DWORD));
	pStream->Write((char *)&m_iTimeStopped, sizeof(DWORD));
	m_sName.WriteToStream(pStream);
	m_pTopology->GetID().WriteToStream(pStream);
	dwSave = 0;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	pStream->Write((char *)&m_iNextEncounter, sizeof(DWORD));

	//	Write flags

	dwSave = 0;
	dwSave |= (m_fNoRandomEncounters ? 0x00000001 : 0);
	dwSave |= 0x00000002;	//	Include version (this is a hack for backwards compatibility)
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Save version

	dwSave = SYSTEM_SAVE_VERSION;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Scale

	pStream->Write((char *)&m_rKlicksPerPixel, sizeof(Metric));
	pStream->Write((char *)&m_rTimeScale, sizeof(Metric));

	//	Save navigation paths

	DWORD dwCount = m_NavPaths.GetCount();
	pStream->Write((char *)&dwCount, sizeof(DWORD));
	CNavigationPath *pNavPath = m_NavPaths.GetNext();
	while (pNavPath)
		{
		pNavPath->WriteToStream(this, pStream);
		pNavPath = pNavPath->GetNext();
		}

	//	Save all objects in the system

	dwCount = 0;
	for (i = 0; i < GetObjectCount(); i++)
		if (GetObject(i))
			dwCount++;

	pStream->Write((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj)
			pObj->WriteToStream(pStream);
		}

	//	Save all named objects

	dwCount = m_NamedObjects.GetCount();
	pStream->Write((char *)&dwCount, sizeof(DWORD));

	for (i = 0; i < (int)dwCount; i++)
		{
		CSpaceObject *pObj = (CSpaceObject *)m_NamedObjects.GetValue(i);
		m_NamedObjects.GetKey(i).WriteToStream(pStream);
		WriteObjRefToStream(pObj, pStream);
		}

	//	Save timed events

	dwCount = m_TimedEvents.GetCount();
	pStream->Write((char *)&dwCount, sizeof(DWORD));

	for (i = 0; i < (int)dwCount; i++)
		{
		CTimedEvent *pEvent = GetTimedEvent(i);
		pEvent->WriteToStream(this, pStream);
		}

	//	Save environment maps

	dwCount = (m_pEnvironment ? 1 : 0);
	pStream->Write((char *)&dwCount, sizeof(DWORD));
	if (m_pEnvironment)
		{
		CTileMap *pEnv;
		ConvertSpaceEnvironmentToUNIDs(*m_pEnvironment, &pEnv);
		pEnv->WriteToStream(pStream);
		delete pEnv;
		}

	return NOERROR;
	}

void CSystem::SetObjectSovereign (CSpaceObject *pObj, CSovereign *pSovereign)

//	SetObjectSovereign
//
//	Sets the sovereign for the object. We need to do this through the system
//	because we need to flush the enemy object cache.

	{
	pObj->SetSovereign(pSovereign);
	FlushEnemyObjectCache();
	}

void CSystem::SetPOVLRS (CSpaceObject *pCenter)

//	SetPOVLRS
//
//	Sets the POVLRS flag for all objects in the viewport

	{
	//	Figure out the boundary of the viewport in system coordinates

	CVector vDiagonal(g_LRSRange, g_LRSRange);
	CVector vUR = pCenter->GetPos() + vDiagonal;
	CVector vLL = pCenter->GetPos() - vDiagonal;

	//	Loop over all objects

	for (int i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj)
			{
			if (pObj->InBox(vUR, vLL))
				pObj->SetPOVLRS();
			else
				pObj->ClearPOVLRS();
			}
		}
	}

void CSystem::SetSpaceEnvironment (int xTile, int yTile, CSpaceEnvironmentType *pEnvironment)

//	SetSpaceEnvironment
//
//	Sets the space environment

	{
	if (m_pEnvironment == NULL)
		m_pEnvironment = new CTileMap(seaArraySize, seaScale);

	m_pEnvironment->SetTile(xTile, yTile, (DWORD)pEnvironment);
	}

ALERROR CSystem::StargateCreated (CSpaceObject *pGate, const CString &sStargateID, const CString &sDestNodeID, const CString &sDestEntryPoint)

//	StargateCreated
//
//	Called whenever a stargate object is created in the system. This code will
//	fix up the topology nodes as appropriate.

	{
	//	Get the ID of the stargate

	CString sGateID;
	if (sStargateID.IsBlank())
		sGateID = strPatternSubst(CONSTLIT("ID%x"), pGate->GetID());
	else
		sGateID = sStargateID;

	//	Look for the stargate in the topology; if we don't find it, then we need to add it

	if (!m_pTopology->FindStargate(sGateID))
		m_pTopology->AddStargate(sGateID, sDestNodeID, sDestEntryPoint);

	//	Add this as a named object (so we can come back here)

	if (GetNamedObject(sGateID) == NULL)
		NameObject(sGateID, pGate);

	return NOERROR;
	}

void CSystem::StopTimeForAll (int iDuration, CSpaceObject *pExcept)

//	StopTimeForAll
//
//	Stop time for all objects in the system (except exception)

	{
	if (IsTimeStopped() || iDuration < 1)
		return;

	for (int i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj && pObj != pExcept && !pObj->IsTimeStopImmune())
			pObj->StopTime();
		}

	m_iTimeStopped = iDuration;
	}

void CSystem::Update (Metric rSecondsPerTick)

//	Update
//
//	Updates the system

	{
	int i;
#ifdef DEBUG_PERFORMANCE
	int iUpdateObj = 0;
	int iMoveObj = 0;
#endif

	//	Fire timed events
	//	NOTE: We only do this if we have a player because otherwise, some
	//	of the scripts might crash. We won't have a player when we first
	//	create the universe.

	SetProgramState(psUpdatingEvents);
	if (!IsTimeStopped() && g_pUniverse->GetPlayer())
		{
		for (i = 0; i < GetTimedEventCount(); i++)
			{
			CTimedEvent *pEvent = GetTimedEvent(i);
			SetProgramEvent(pEvent);

			if (pEvent->GetTick() <= m_iTick)
				{
				bool bDeleteEvent;
				pEvent->DoEvent(this, &bDeleteEvent);

				if (bDeleteEvent)
					{
					if (i > GetTimedEventCount() || GetTimedEvent(i) != pEvent)
						{
						// Fired event removed other events and changed
						// the ordering; iterate through looking for our
						// place again.
						for (i = 0; i < GetTimedEventCount(); i++)
							{
							if (GetTimedEvent(i) == pEvent)
								{
								break;
								}
							}
						}
					if (i < GetTimedEventCount())
						{
						m_TimedEvents.RemoveObject(i);
						i--;
						}
					}
				}
			}
		}

	SetProgramEvent(NULL);

	//	Give all objects a chance to react

	DebugStartTimer();
	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj && !pObj->IsTimeStopped())
			{
			SetProgramState(psUpdatingBehavior, pObj);
			pObj->Behavior();

			//	Update the objects

			SetProgramState(psUpdatingObj, pObj);
			pObj->Update();

			//	NOTE: pObj may have been destroyed after
			//	Update(). Do not use the pointer.

#ifdef DEBUG_PERFORMANCE
			iUpdateObj++;
#endif
			}
		}
	DebugStopTimer("Updating objects");

	//	We create a list of all barrier objects

	m_BarrierObjects.RemoveAll();

	//	Calculate the new position of the object (but do not
	//	update m_vPos yet)

	DebugStartTimer();
	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);
		if (pObj)
			{
			if (pObj->IsMobile())
				pObj->CalculateMove();

			if (pObj->IsBarrier())
				m_BarrierObjects.FastAdd(pObj);
			}
		}

	//	Move all objects. Note: We always move last because we want to
	//	paint right after a move. Otherwise, when a laser/missile hits
	//	an object, the laser/missile is deleted (in update) before it
	//	gets a chance to paint.

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj && pObj->IsMobile() && !pObj->IsTimeStopped())
			{
			//	Move the objects

			SetProgramState(psUpdatingMove, pObj);
			pObj->Move(m_BarrierObjects, rSecondsPerTick);

#ifdef DEBUG_PERFORMANCE
			iMoveObj++;
#endif
			}
		}
	DebugStopTimer("Moving objects");

	//	Update random encounters

	SetProgramState(psUpdatingEncounters);
	if (m_iEncounterObjCount 
			&& m_iTick >= m_iNextEncounter
			&& !IsTimeStopped())
		UpdateRandomEncounters();

	//	Update time stopped

	SetProgramState(psUpdating);
	if (IsTimeStopped())
		if (--m_iTimeStopped == 0)
			RestartTime();

#ifdef DEBUG_PERFORMANCE
	{
	char szBuffer[1024];
	wsprintf(szBuffer, "Objects: %d  Updating: %d  Moving: %d  Barriers: %d\n", 
			GetObjectCount(), 
			iUpdateObj, 
			iMoveObj,
			m_BarrierObjects.GetCount());
	::OutputDebugString(szBuffer);
	}
#endif

	//	Next

	m_iTick++;
	}

void CSystem::UpdateExtended (void)

//	UpdateExtended
//
//	Updates the system for many ticks

	{
	int i;

	//	Update for a few seconds

	int iTime = mathRandom(250, 350);
	for (i = 0; i < iTime; i++)
		Update(g_SecondsPerUpdate);

	//	Give all objects a chance to update

	for (i = 0; i < GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetObject(i);

		if (pObj)
			{
			SetProgramState(psUpdatingExtended, pObj);
			pObj->UpdateExtended();
			}
		}

	SetProgramState(psUpdating);
	}

void CSystem::UpdateRandomEncounters (void)

//	UpdateRandomEncounters
//
//	Updates random encounters

	{
	struct SEncounter
		{
		int iWeight;
		CSpaceObject *pObj;
		IShipGenerator *pTable;
		};

	int i;

	if (m_fNoRandomEncounters)
		return;

	//	No need for random encounters if the player isn't in the system
	//	NOTE: We check for CanAttack because pPlayer is something (when the player
	//	has been destroyed) a CPOVMarker object. In that case, we don't want
	//	to send enemy ships to destroy it.

	CSpaceObject *pPlayer = g_pUniverse->GetPlayer();
	if (pPlayer == NULL || pPlayer->GetSystem() != this || !pPlayer->CanAttack())
		return;

	IShipGenerator *pTable = NULL;
	CSpaceObject *pBase = NULL;
	CSovereign *pBaseSovereign = NULL;

	//	Some percent of the time we generate a generic level encounter; the rest of the
	//	time, the encounter is based on the stations in this system.

	if (mathRandom(1, 100) <= LEVEL_ENCOUNTER_CHANCE)
		g_pUniverse->GetRandomLevelEncounter(GetLevel(), &pTable, &pBaseSovereign);
	else
		{
		//	Allocate and fill-in the table

		SEncounter *pMainTable = new SEncounter [m_iEncounterObjCount];
		int iCount = 0;
		int iTotal = 0;
		for (i = 0; i < m_iEncounterObjCount; i++)
			{
			int iFreq;
			IShipGenerator *pTable = m_pEncounterObj[i]->GetRandomEncounterTable(&iFreq);
			if (pTable && iFreq > 0)
				{
				pMainTable[iCount].iWeight = iFreq;
				pMainTable[iCount].pObj = m_pEncounterObj[i];
				pMainTable[iCount].pTable = pTable;

				iTotal += iFreq;
				iCount++;
				}
			}

		//	Pick a random entry in the table

		if (iTotal > 0)
			{
			int iRoll = mathRandom(0, iTotal - 1);
			int iPos = 0;

			//	Get the position

			while (pMainTable[iPos].iWeight <= iRoll)
				iRoll -= pMainTable[iPos++].iWeight;

			//	Done

			pTable = pMainTable[iPos].pTable;
			pBase = pMainTable[iPos].pObj;
			}

		delete [] pMainTable;
		}

	//	Generate the encounter

	if (pTable)
		CreateRandomEncounter(pTable, pBase, pBaseSovereign, pPlayer);

	//	Next encounter

	m_iNextEncounter = m_iTick + mathRandom(6000, 9000);
	}

void CSystem::WriteObjRefToStream (CSpaceObject *pObj, IWriteStream *pStream)

//	WriteObjRefToStream
//
//	DWORD		0xffffffff if NULL
//				Otherwise, index of object in system

	{
	DWORD dwSave = 0xffffffff;
	if (pObj)
		{
		dwSave = pObj->GetIndex();
		ASSERT(dwSave != 0xDDDDDDDD);
		assert(dwSave != 0);
		}

	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

void CSystem::WriteSovereignRefToStream (CSovereign *pSovereign, IWriteStream *pStream)

//	WriteSovereignRefToStream
//
//	DWORD		0xffffffff if NULL
//				Otherwise, UNID

	{
	DWORD dwSave = 0xffffffff;
	if (pSovereign)
		dwSave = pSovereign->GetUNID();

	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

//	Miscellaneous --------------------------------------------------------------

bool CalcOverlap (SLabelEntry *pEntries, int iCount)
	{
	bool bOverlap = false;
	int i, j;

	for (i = 0; i < iCount; i++)
		{
		pEntries[i].iNewPosition = labelPosNone;

		for (j = 0; j < iCount; j++)
			if (i != j)
				{
				if (RectsIntersect(&pEntries[i].rcLabel, &pEntries[j].rcLabel))
					{
					int xDelta = pEntries[j].x - pEntries[i].x;
					int yDelta = pEntries[j].y - pEntries[i].y;

					switch (pEntries[i].iPosition)
						{
						case labelPosRight:
							{
							if (xDelta > 0)
								pEntries[i].iNewPosition = labelPosLeft;
							break;
							}

						case labelPosLeft:
							{
							if (xDelta < 0)
								pEntries[i].iNewPosition = labelPosBottom;
							break;
							}
						}

					bOverlap = true;
					break;
					}
				}
		}

	return bOverlap;
	}

void SetLabelBelow (SLabelEntry &Entry, int cyChar)
	{
	Entry.rcLabel.top = Entry.y + LABEL_SPACING_Y + LABEL_OVERLAP_Y;
	Entry.rcLabel.bottom = Entry.rcLabel.top + cyChar - (2 * LABEL_OVERLAP_Y);
	Entry.rcLabel.left = Entry.x - (Entry.cxLabel / 2);
	Entry.rcLabel.right = Entry.rcLabel.left + Entry.cxLabel;

	Entry.iPosition = labelPosBottom;
	}

void SetLabelLeft (SLabelEntry &Entry, int cyChar)
	{
	Entry.rcLabel.left = Entry.x - (LABEL_SPACING_X + Entry.cxLabel);
	Entry.rcLabel.top = Entry.y - (cyChar / 2) + LABEL_OVERLAP_Y;
	Entry.rcLabel.right = Entry.rcLabel.left + Entry.cxLabel;
	Entry.rcLabel.bottom = Entry.rcLabel.top + cyChar - (2 * LABEL_OVERLAP_Y);

	Entry.iPosition = labelPosLeft;
	}

void SetLabelRight (SLabelEntry &Entry, int cyChar)
	{
	Entry.rcLabel.left = Entry.x + LABEL_SPACING_X;
	Entry.rcLabel.top = Entry.y - (cyChar / 2) + LABEL_OVERLAP_Y;
	Entry.rcLabel.right = Entry.rcLabel.left + Entry.cxLabel;
	Entry.rcLabel.bottom = Entry.rcLabel.top + cyChar - (2 * LABEL_OVERLAP_Y);

	Entry.iPosition = labelPosRight;
	}

