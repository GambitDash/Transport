//	CShipTable.cpp
//
//	CShipTable object

#include "PreComp.h"

#include "Kernel.h"

#define ESCORTS_TAG					CONSTLIT("Escorts")
#define GROUP_TAG					CONSTLIT("Group")
#define ITEMS_TAG					CONSTLIT("Items")
#define NAMES_TAG					CONSTLIT("Names")
#define NULL_TAG					CONSTLIT("Null")
#define SHIP_TAG					CONSTLIT("Ship")
#define SHIPS_TAG					CONSTLIT("Ships")
#define TABLE_TAG					CONSTLIT("Table")

#define CHANCE_ATTRIB				CONSTLIT("chance")
#define CLASS_ATTRIB				CONSTLIT("class")
#define CONTROLLER_ATTRIB			CONSTLIT("controller")
#define COUNT_ATTRIB				CONSTLIT("count")
#define INITIAL_DATA_TAG			CONSTLIT("InitialData")
#define MAX_SHIPS_ATTRIB			CONSTLIT("maxShips")
#define NAME_ATTRIB					CONSTLIT("name")
#define ORDERS_ATTRIB				CONSTLIT("orders")
#define PATROL_DIST_ATTRIB			CONSTLIT("patrolDist")
#define SOVEREIGN_ATTRIB			CONSTLIT("sovereign")
#define UNID_ATTRIB					CONSTLIT("unid")

class CNullShip : public IShipGenerator
	{
	};

class CSingleShip : public IShipGenerator
	{
	public:
		CSingleShip (void);
		virtual ~CSingleShip (void);
		virtual void CreateShips (SShipCreateCtx &Ctx);
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		virtual ALERROR ValidateShipTable (SDesignLoadCtx &Ctx);

	private:
		void CreateShip (CSystem *pSystem,
						 CSovereign *pSovereign,
						 const CVector &vPos,
						 CSpaceObject *pExitGate,
						 CSpaceObject *pBase,
						 CSpaceObject *pTarget,
						 bool bAlreadyDocked,
						 bool bAttackNearestEnemy,
						 CShip **retpShip = NULL);

		DiceRange m_Count;							//	Number of ships to create
		int m_iMaxCountInSystem;					//	Do not exceed this number of ship of this class in system (or -1)

		CShipClassRef m_pShipClass;					//	Ship class to create
		CSovereignRef m_pSovereign;					//	Sovereign
		CString m_sShipName;						//	Name of ship
		DWORD m_dwShipNameFlags;					//	Flags for ship name

		IItemGenerator *m_pItems;					//	Random table of items (or NULL)
		IShipGenerator *m_pEscorts;					//	Random table of escorts (or NULL)
		CAttributeDataBlock m_InitialData;			//	Initial data for ship

		CString m_sController;						//	Controller to use (or "" to use default)
		IShipController::OrderTypes m_iOrder;		//	Ship order
		DWORD m_dwOrderData;						//	Order data
	};

class CTableOfShipGenerators : public IShipGenerator
	{
	public:
		CTableOfShipGenerators (void) : m_Table(NULL) { }
		virtual ~CTableOfShipGenerators (void);
		virtual void CreateShips (SShipCreateCtx &Ctx);
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		virtual ALERROR ValidateShipTable (SDesignLoadCtx &Ctx);

	private:
		struct SEntry
			{
			IShipGenerator *pEntry;
			int iProbability;
			};

		DiceRange m_Count;

		int m_iTableCount;
		SEntry *m_Table;
	};

class CGroupOfShipGenerators : public IShipGenerator
	{
	public:
		CGroupOfShipGenerators (void) : m_Table(NULL) { }
		virtual ~CGroupOfShipGenerators (void);
		virtual void CreateShips (SShipCreateCtx &Ctx);
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);
		virtual ALERROR ValidateShipTable (SDesignLoadCtx &Ctx);

	private:
		struct SEntry
			{
			IShipGenerator *pEntry;
			int iChance;
			};

		DiceRange m_Count;

		int m_iTableCount;
		SEntry *m_Table;
	};

ALERROR IShipGenerator::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, IShipGenerator **retpGenerator)

//	CreateFromXML
//
//	Creates a new generator

	{
	ALERROR error;
	IShipGenerator *pGenerator = NULL;

	if (strEquals(pDesc->GetTag(), SHIP_TAG))
		pGenerator = new CSingleShip;
	else if (strEquals(pDesc->GetTag(), TABLE_TAG))
		pGenerator = new CTableOfShipGenerators;
	else if (strEquals(pDesc->GetTag(), GROUP_TAG) 
			|| strEquals(pDesc->GetTag(), SHIPS_TAG) 
			|| strEquals(pDesc->GetTag(), ESCORTS_TAG))
		pGenerator = new CGroupOfShipGenerators;
	else if (strEquals(pDesc->GetTag(), NULL_TAG))
		pGenerator = new CNullShip;
	else
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Unknown ship generator: %s"), pDesc->GetTag().GetASCIIZPointer());
		return ERR_FAIL;
		}

	if (error = pGenerator->LoadFromXML(Ctx, pDesc))
		{
		if (pGenerator)
			delete pGenerator;
		return error;
		}

	*retpGenerator = pGenerator;

	return NOERROR;
	}

ALERROR IShipGenerator::CreateFromXMLAsGroup (SDesignLoadCtx &Ctx, CXMLElement *pDesc, IShipGenerator **retpGenerator)

//	CreateFromXMLAsGroup
//
//	Creates a new generator

	{
	ALERROR error;

	IShipGenerator *pGenerator = new CGroupOfShipGenerators;
	if (error = pGenerator->LoadFromXML(Ctx, pDesc))
		{
		if (pGenerator)
			delete pGenerator;
		return error;
		}

	*retpGenerator = pGenerator;

	return NOERROR;
	}

//	CShipTable ----------------------------------------------------------------

CShipTable::CShipTable (void) : 
		m_pGenerator(NULL)

//	CShipTable constructor

	{
	}

CShipTable::~CShipTable (void)

//	CShipTable destructor

	{
	if (m_pGenerator)
		delete m_pGenerator;
	}

ALERROR CShipTable::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind design

	{
	ALERROR error;

	if (m_pGenerator)
		{
		if (error = m_pGenerator->OnDesignLoadComplete(Ctx))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("ShipTable (%x): %s"), GetUNID(), Ctx.sError.GetASCIIZPointer());
			return error;
			}

		//	Make sure that the table is valid for a shiptable (i.e., that it has a proper
		//	value set for sovereign)

		if (error = m_pGenerator->ValidateShipTable(Ctx))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("ShipTable (%x): %s"), GetUNID(), Ctx.sError.GetASCIIZPointer());
			return error;
			}
		}

	return NOERROR;
	}

ALERROR CShipTable::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Create from XML

	{
	ALERROR error;

	CXMLElement *pElement = pDesc->GetContentElement(0);
	if (pElement)
		{
		if (error = IShipGenerator::CreateFromXML(Ctx, pElement, &m_pGenerator))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("ShipTable (%x): %s"), GetUNID(), Ctx.sError.GetASCIIZPointer());
			return error;
			}
		}

	return NOERROR;
	}

//	CSingleShip ---------------------------------------------------------------

CSingleShip::CSingleShip (void) :
		m_pItems(NULL),
		m_pEscorts(NULL)

//	CSingleShip constructor

	{
	}

CSingleShip::~CSingleShip (void)

//	CSingleShip destructor

	{
	if (m_pItems)
		delete m_pItems;

	if (m_pEscorts)
		delete m_pEscorts;
	}

void CSingleShip::CreateShip (CSystem *pSystem,
							  CSovereign *pSovereign,
							  const CVector &vPos,
							  CSpaceObject *pExitGate,
							  CSpaceObject *pBase,
							  CSpaceObject *pTarget,
							  bool bAlreadyDocked,
							  bool bAttackNearestEnemy,
							  CShip **retpShip)

//	CreateShip
//
//	Creates a single ship

	{
	int i;

	ASSERT(m_pShipClass);
	ASSERT(pSovereign);

	DWORD dwClass = m_pShipClass->GetUNID();

	//	If we've got a maximum, then see if we've already got too many ships of this
	//	ship class.

	if (m_iMaxCountInSystem > 0)
		{
		int iMaxShips = m_iMaxCountInSystem;
		for (i = 0; i < pSystem->GetObjectCount(); i++)
			{
			CSpaceObject *pObj = pSystem->GetObject(i);
			if (pObj && pObj->GetClassUNID() == dwClass)
				{
				if (--iMaxShips == 0)
					{
					if (retpShip)
						*retpShip = NULL;
					return;
					}
				}
			}
		}

	//	Get the controller

	IShipController *pController = ::CreateShipController(m_sController);
	if (pController == NULL)
		{
		kernelDebugLogMessage("Cannot create ship %x; invalid controller: %s", dwClass, m_sController.GetASCIIZPointer());
		if (retpShip)
			*retpShip = NULL;
		return;
		}

	//	Create the ship

	CShip *pShip;
	if (pSystem->CreateShip(dwClass,
			pController,
			pSovereign,
			vPos,
			NullVector,
			mathRandom(0, 359),
			pExitGate,
			&pShip) != NOERROR)
		{
		kernelDebugLogMessage("Error creating ship: %x", dwClass);
		if (retpShip)
			*retpShip = NULL;
		return;
		}

	//	Set the ship's name

	if (!m_sShipName.IsBlank())
		pShip->SetName(::GenerateRandomName(m_sShipName, NULL_STR), m_dwShipNameFlags);

	//	Add items to the ship

	if (m_pItems)
		{
		if (pShip->CreateRandomItems(m_pItems) != NOERROR)
			{
			kernelDebugLogMessage("Unable to create items for ship: %x", dwClass);
			if (retpShip)
				*retpShip = NULL;
			return;
			}
		}

	//	set any initial data

	if (!m_InitialData.IsEmpty())
		pShip->SetDataFromDataBlock(m_InitialData);

	//	Set the orders for the ship

	CSpaceObject *pOrderTarget = NULL;
	bool bDockWithBase = false;
	bool bIsSubordinate = false;
	bool bNeedsDockOrder = false;
	switch (m_iOrder)
		{
		case IShipController::orderNone:
			//	If a ship has no orders and it has a base, then dock with the base
			if (pBase && pBase->SupportsDocking())
				{
				bDockWithBase = true;
				bNeedsDockOrder = true;
				}
			break;

		case IShipController::orderGuard:
			{
			if (pBase == NULL)
				{
				kernelDebugLogMessage("Unable to create ship %x; no base specified", dwClass);
				if (retpShip)
					*retpShip = NULL;
				return;
				}

			pOrderTarget = pBase;
			bIsSubordinate = true;
			bDockWithBase = true;
			break;
			}

		case IShipController::orderMine:
			{
			if (pBase == NULL)
				{
				kernelDebugLogMessage("Unable to create ship %x; no base specified", dwClass);
				if (retpShip)
					*retpShip = NULL;
				return;
				}

			pOrderTarget = pBase;
			bIsSubordinate = true;
			break;
			}

		case IShipController::orderGateOnThreat:
			{
			if (pBase == NULL)
				{
				kernelDebugLogMessage("Unable to create ship %x; no base specified", dwClass);
				if (retpShip)
					*retpShip = NULL;
				return;
				}

			pOrderTarget = pBase;
			bNeedsDockOrder = true;
			bDockWithBase = true;
			break;
			}

		case IShipController::orderGate:
			{
			//	For backwards compatibility...
			if (pBase)
				{
				m_iOrder = IShipController::orderGateOnThreat;
				pOrderTarget = pBase;
				bNeedsDockOrder = true;
				bDockWithBase = true;
				}
			else
				{
				//	OK if this is NULL...we just go to closest gate
				pOrderTarget = pTarget;
				}

			break;
			}

		case IShipController::orderPatrol:
		case IShipController::orderEscort:
		case IShipController::orderFollow:
			{
			if (pBase == NULL)
				{
				kernelDebugLogMessage("Unable to create ship %x; no base specified", dwClass);
				if (retpShip)
					*retpShip = NULL;
				return;
				}

			pOrderTarget = pBase;
			break;
			}

		case IShipController::orderDestroyTarget:
		case IShipController::orderAimAtTarget:
			{
			if (pTarget == NULL)
				{
				kernelDebugLogMessage("Unable to create ship %x; no target specified", dwClass);
				if (retpShip)
					*retpShip = NULL;
				return;
				}

			pOrderTarget = pTarget;
			break;
			}
		}

	//	If we're creating a station and its ships and if we need to dock with 
	//	the base, then position the ship at a docking port

	if (bAlreadyDocked && bDockWithBase)
		{
		ASSERT(pBase);
		pBase->PlaceAtRandomDockPort(pShip);
		}

	//	Otherwise, if we need a dock order to get to our base, add that now
	//	(before the main order)

	else if (bNeedsDockOrder)
		pShip->GetController()->AddOrder(IShipController::orderDock, pBase, 0);

	//	Add main order

	if (m_iOrder != IShipController::orderNone)
		pShip->GetController()->AddOrder(m_iOrder, pOrderTarget, m_dwOrderData);

	//	If necessary, append an order to attack nearest enemy ships

	if (bAttackNearestEnemy)
		pShip->GetController()->AddOrder(IShipController::orderAttackNearestEnemy, NULL, 0);

	//	If this ship is ordered to guard then it counts as a subordinate

	if (bIsSubordinate)
		pBase->AddSubordinate(pShip);

	//	If this ship has escorts then create them as well

	if (m_pEscorts)
		{
		SShipCreateCtx Ctx;
		Ctx.pSystem = pSystem;
		Ctx.vPos = vPos;
		Ctx.pBase = pShip;
		Ctx.pTarget = pTarget;
		Ctx.pGate = pExitGate;

		m_pEscorts->CreateShips(Ctx);
		}

	//	Done

	if (retpShip)
		*retpShip = pShip;
	}

void CSingleShip::CreateShips (SShipCreateCtx &Ctx)

//	CreateShips
//
//	Create the ships

	{
	int i;

	ASSERT(Ctx.pSystem);

	//	Figure out the sovereign

	CSovereign *pSovereign;
	if (m_pSovereign)
		pSovereign = m_pSovereign;
	else if (Ctx.pBase)
		pSovereign = Ctx.pBase->GetSovereign();
	else if (Ctx.pBaseSovereign)
		pSovereign = Ctx.pBaseSovereign;
	else
		{
		ASSERT(false);
		kernelDebugLogMessage("Unable to create ships: no sovereign specified");
		return;
		}

	//	Generate center

	CVector vCenter;
	if (Ctx.pGate)
		vCenter = Ctx.pGate->GetPos();
	else
		vCenter = Ctx.vPos;

	//	Create all the ships

	int iCount = m_Count.Roll();
	for (i = 0; i < iCount; i++)
		{
		CShip *pShip;

		//	Generate a position

		CVector vPos = vCenter + ::PolarToVector(mathRandom(0, 359), Ctx.PosSpread.Roll() * LIGHT_SECOND);

		//	Create the ship

		CreateShip(Ctx.pSystem,
				pSovereign,
				vPos,
				Ctx.pGate,
				Ctx.pBase,
				Ctx.pTarget,
				((Ctx.dwFlags & SShipCreateCtx::SHIPS_FOR_STATION) ? true : false),
				((Ctx.dwFlags & SShipCreateCtx::ATTACK_NEAREST_ENEMY) ? true : false),
				&pShip);

		if (pShip)
			{
			//	Add to result list, if necessary

			if (Ctx.dwFlags & SShipCreateCtx::RETURN_RESULT)
				Ctx.Result.Add(pShip);

			//	Add encounter info, if necessary

			if (Ctx.pEncounterInfo)
				pShip->SetEncounterInfo(Ctx.pEncounterInfo);
			}
		}
	}

ALERROR CSingleShip::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load descriptor

	{
	ALERROR error;

	//	Load some data

	m_sController = pDesc->GetAttribute(CONTROLLER_ATTRIB);
	m_iMaxCountInSystem = pDesc->GetAttributeInteger(MAX_SHIPS_ATTRIB);
	m_Count.LoadFromXML(pDesc->GetAttribute(COUNT_ATTRIB));
	if (m_Count.IsEmpty())
		m_Count.SetConstant(1);

	//	Load name

	CXMLElement *pNames = pDesc->GetContentElementByTag(NAMES_TAG);
	if (pNames)
		{
		m_sShipName = pNames->GetContentText(0);
		m_dwShipNameFlags = LoadNameFlags(pNames);
		}
	else
		{
		m_sShipName = pDesc->GetAttribute(NAME_ATTRIB);
		m_dwShipNameFlags = 0;
		}

	//	Load references to other design elements

	m_pShipClass.LoadUNID(Ctx, pDesc->GetAttribute(CLASS_ATTRIB));
	m_pSovereign.LoadUNID(Ctx, pDesc->GetAttribute(SOVEREIGN_ATTRIB));

	//	Load orders
	//	For compatibility, some orders have alternate strings

	CString sOrders = pDesc->GetAttribute(ORDERS_ATTRIB);
	if (strEquals(sOrders, CONSTLIT("trade route")))
		sOrders = CONSTLIT("tradeRoute");

	m_iOrder = GetOrderType(sOrders);
	switch (m_iOrder)
		{
		case IShipController::orderPatrol:
			m_dwOrderData = Max(1, pDesc->GetAttributeInteger(PATROL_DIST_ATTRIB));
			break;

		default:
			m_dwOrderData = 0;
		}

	//	Load items

	CXMLElement *pItems = pDesc->GetContentElementByTag(ITEMS_TAG);
	if (pItems)
		{
		if (error = IItemGenerator::CreateFromXML(Ctx, pItems, &m_pItems))
			return error;
		}
	else
		m_pItems = NULL;

	//	Load initial data

	CXMLElement *pInitialData = pDesc->GetContentElementByTag(INITIAL_DATA_TAG);
	if (pInitialData)
		m_InitialData.SetFromXML(pInitialData);

	//	Load escorts

	CXMLElement *pEscorts = pDesc->GetContentElementByTag(ESCORTS_TAG);
	if (pEscorts)
		{
		if (error = IShipGenerator::CreateFromXMLAsGroup(Ctx, pEscorts, &m_pEscorts))
			return error;
		}
	else
		m_pEscorts = NULL;

	return NOERROR;
	}

ALERROR CSingleShip::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Design load done

	{
	ALERROR error;

	if (error = m_pShipClass.Bind(Ctx))
		return error;

	if (error = m_pSovereign.Bind(Ctx))
		return error;

	if (m_pItems)
		if (error = m_pItems->OnDesignLoadComplete(Ctx))
			return error;

	if (m_pEscorts)
		if (error = m_pEscorts->OnDesignLoadComplete(Ctx))
			return error;

	return NOERROR;
	}

ALERROR CSingleShip::ValidateShipTable (SDesignLoadCtx &Ctx)

//	ValidateShipTable
//
//	Make sure that this generator has enough data for a ship table (<EncounterTable>)

	{
	if (m_pSovereign == NULL)
		{
		Ctx.sError = CONSTLIT("Ship must specify a sovereign");
		return ERR_FAIL;
		}

	//	No need to recurse down to escorts because escorts take on the sovereign of
	//	their leader.

	return NOERROR;
	}

//	CTableOfShipGenerators ---------------------------------------------------------------

CTableOfShipGenerators::~CTableOfShipGenerators (void)

//	CTableOfShipGenerators destructor

	{
	int i;

	if (m_Table)
		{
		for (i = 0; i < m_iTableCount; i++)
			if (m_Table[i].pEntry)
				delete m_Table[i].pEntry;

		delete [] m_Table;
		}
	}

void CTableOfShipGenerators::CreateShips (SShipCreateCtx &Ctx)

//	CreateShips
//
//	Create the ships

	{
	int i, j;

	int iCount = m_Count.Roll();
	for (j = 0; j < iCount; j++)
		{
		int iRoll = mathRandom(1, 100);

		for (i = 0; i < m_iTableCount; i++)
			{
			iRoll -= m_Table[i].iProbability;

			if (iRoll <= 0)
				{
				m_Table[i].pEntry->CreateShips(Ctx);
				break;
				}
			}
		}
	}

ALERROR CTableOfShipGenerators::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load descriptor

	{
	int i;
	ALERROR error;

	m_Count.LoadFromXML(pDesc->GetAttribute(COUNT_ATTRIB));
	if (m_Count.IsEmpty())
		m_Count.SetConstant(1);

	m_iTableCount = pDesc->GetContentElementCount();
	if (m_iTableCount > 0)
		{
		int iTotal = 0;

		m_Table = new SEntry [m_iTableCount];
		memset(m_Table, 0, sizeof(SEntry) * m_iTableCount);

		for (i = 0; i < m_iTableCount; i++)
			{
			CXMLElement *pEntry = pDesc->GetContentElement(i);
			
			m_Table[i].iProbability = pEntry->GetAttributeInteger(CHANCE_ATTRIB);
			iTotal += m_Table[i].iProbability;

			if (error = IShipGenerator::CreateFromXML(Ctx, pEntry, &m_Table[i].pEntry))
				return error;
			}

		if (iTotal != 100)
			{
			Ctx.sError = CONSTLIT("Ship table probabilities do not add up to 100%");
			return ERR_FAIL;
			}
		}
	else
		m_Table = NULL;

	return NOERROR;
	}

ALERROR CTableOfShipGenerators::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Design load done

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_iTableCount; i++)
		{
		if (error = m_Table[i].pEntry->OnDesignLoadComplete(Ctx))
			return error;
		}

	return NOERROR;
	}

ALERROR CTableOfShipGenerators::ValidateShipTable (SDesignLoadCtx &Ctx)

//	ValidateShipTable
//
//	Make sure the generator is valid for a ship table

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_iTableCount; i++)
		{
		if (error = m_Table[i].pEntry->ValidateShipTable(Ctx))
			return error;
		}

	return NOERROR;
	}

//	CGroupOfShipGenerators ---------------------------------------------------------------

CGroupOfShipGenerators::~CGroupOfShipGenerators (void)

//	CGroupOfShipGenerators destructor

	{
	int i;

	if (m_Table)
		{
		for (i = 0; i < m_iTableCount; i++)
			if (m_Table[i].pEntry)
				delete m_Table[i].pEntry;

		delete [] m_Table;
		}
	}

void CGroupOfShipGenerators::CreateShips (SShipCreateCtx &Ctx)

//	CreateShips
//
//	Create the ships

	{
	int i, j;

	int iCount = m_Count.Roll();
	for (j = 0; j < iCount; j++)
		{
		for (i = 0; i < m_iTableCount; i++)
			{
			if (mathRandom(1, 100) <= m_Table[i].iChance)
				m_Table[i].pEntry->CreateShips(Ctx);
			}
		}
	}

ALERROR CGroupOfShipGenerators::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load descriptor

	{
	ALERROR error;
	int i;

	m_Count.LoadFromXML(pDesc->GetAttribute(COUNT_ATTRIB));
	if (m_Count.IsEmpty())
		m_Count.SetConstant(1);

	m_iTableCount = pDesc->GetContentElementCount();
	if (m_iTableCount > 0)
		{
		m_Table = new SEntry [m_iTableCount];
		memset(m_Table, 0, sizeof(SEntry) * m_iTableCount);

		for (i = 0; i < m_iTableCount; i++)
			{
			CXMLElement *pEntry = pDesc->GetContentElement(i);
			
			m_Table[i].iChance = pEntry->GetAttributeInteger(CHANCE_ATTRIB);
			if (m_Table[i].iChance == 0)
				m_Table[i].iChance = 100;

			if (error = IShipGenerator::CreateFromXML(Ctx, pEntry, &m_Table[i].pEntry))
				return error;
			}
		}
	else
		m_Table = NULL;

	return NOERROR;
	}

ALERROR CGroupOfShipGenerators::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Design load done

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_iTableCount; i++)
		{
		if (error = m_Table[i].pEntry->OnDesignLoadComplete(Ctx))
			return error;
		}

	return NOERROR;
	}

ALERROR CGroupOfShipGenerators::ValidateShipTable (SDesignLoadCtx &Ctx)

//	ValidateShipTable
//
//	Make sure the generator is valid for a ship table

	{
	ALERROR error;
	int i;

	for (i = 0; i < m_iTableCount; i++)
		{
		if (error = m_Table[i].pEntry->ValidateShipTable(Ctx))
			return error;
		}

	return NOERROR;
	}
