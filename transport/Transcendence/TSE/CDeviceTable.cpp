//	CDeviceTable.cpp
//
//	IDeviceGenerator objects

#include "PreComp.h"

#define GROUP_TAG								CONSTLIT("Group")
#define TABLE_TAG								CONSTLIT("Table")
#define ITEM_TAG								CONSTLIT("Item")
#define ITEMS_TAG								CONSTLIT("Items")
#define NULL_TAG								CONSTLIT("Null")
#define DEVICE_TAG								CONSTLIT("Device")
#define DEVICES_TAG								CONSTLIT("Devices")

#define UNID_ATTRIB								CONSTLIT("unid")
#define ITEM_ATTRIB								CONSTLIT("item")
#define COUNT_ATTRIB							CONSTLIT("count")
#define TABLE_ATTRIB							CONSTLIT("table")
#define CHANCE_ATTRIB							CONSTLIT("chance")
#define CRITERIA_ATTRIB							CONSTLIT("criteria")
#define LEVEL_ATTRIB							CONSTLIT("level")
#define LEVEL_CURVE_ATTRIB						CONSTLIT("levelCurve")
#define DAMAGED_ATTRIB							CONSTLIT("damaged")
#define CATEGORIES_ATTRIB						CONSTLIT("categories")
#define MODIFIERS_ATTRIB						CONSTLIT("modifiers")
#define LEVEL_FREQUENCY_ATTRIB					CONSTLIT("levelFrequency")
#define ENHANCED_ATTRIB							CONSTLIT("enhanced")
#define ENHANCEMENT_ATTRIB						CONSTLIT("enhancement")
#define DEVICE_ID_ATTRIB						CONSTLIT("deviceID")
#define OMNIDIRECTIONAL_ATTRIB					CONSTLIT("omnidirectional")
#define POS_ANGLE_ATTRIB						CONSTLIT("posAngle")
#define POS_RADIUS_ATTRIB						CONSTLIT("posRadius")
#define MIN_FIRE_ARC_ATTRIB						CONSTLIT("minFireArc")
#define MAX_FIRE_ARC_ATTRIB						CONSTLIT("maxFireArc")
#define SECONDARY_WEAPON_ATTRIB					CONSTLIT("secondaryWeapon")

#define STR_G_ITEM								CONSTLIT("gItem")

class CNullDevice : public IDeviceGenerator
	{
	};

class CSingleDevice : public IDeviceGenerator
	{
	public:
		CSingleDevice (void) : m_pExtraItems(NULL) { }
		~CSingleDevice (void);

		virtual void AddDevices (CDeviceDescList &Result);
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

	private:
		CItemTypeRef m_pItemType;
		DiceRange m_Count;
		int m_iDamaged;
		CRandomEnhancementGenerator m_Enhanced;

		int m_iPosAngle;
		int m_iPosRadius;

		bool m_bOmnidirectional;
		int m_iMinFireArc;
		int m_iMaxFireArc;
		bool m_bSecondary;

		IItemGenerator *m_pExtraItems;
	};

class CTableOfDeviceGenerators : public IDeviceGenerator
	{
	public:
		virtual ~CTableOfDeviceGenerators (void);
		virtual void AddDevices (CDeviceDescList &Result);
		virtual IDeviceGenerator *GetGenerator (int iIndex) { return m_Table[iIndex].pDevice; }
		virtual int GetGeneratorCount (void) { return m_iTableCount; }
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

	private:
		struct SEntry
			{
			IDeviceGenerator *pDevice;
			int iProbability;
			};

		DiceRange m_Count;

		int m_iTableCount;
		SEntry *m_Table;
	};

class CGroupOfDeviceGenerators : public IDeviceGenerator
	{
	public:
		virtual ~CGroupOfDeviceGenerators (void);
		virtual void AddDevices (CDeviceDescList &Result);
		virtual IDeviceGenerator *GetGenerator (int iIndex) { return m_Table[iIndex].pDevice; }
		virtual int GetGeneratorCount (void) { return m_iTableCount; }
		virtual ALERROR LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc);
		virtual ALERROR OnDesignLoadComplete (SDesignLoadCtx &Ctx);

	private:
		struct SEntry
			{
			IDeviceGenerator *pDevice;
			int iChance;
			};

		DiceRange m_Count;

		int m_iTableCount;
		SEntry *m_Table;
	};

ALERROR IDeviceGenerator::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, IDeviceGenerator **retpGenerator)

//	CreateFromXML
//
//	Creates a new generator

	{
	ALERROR error;
	IDeviceGenerator *pGenerator = NULL;

	if (strEquals(pDesc->GetTag(), DEVICE_TAG) || strEquals(pDesc->GetTag(), ITEM_TAG))
		pGenerator = new CSingleDevice;
	else if (strEquals(pDesc->GetTag(), TABLE_TAG))
		pGenerator = new CTableOfDeviceGenerators;
	else if (strEquals(pDesc->GetTag(), GROUP_TAG) || strEquals(pDesc->GetTag(), DEVICES_TAG))
		pGenerator = new CGroupOfDeviceGenerators;
	else if (strEquals(pDesc->GetTag(), NULL_TAG))
		pGenerator = new CNullDevice;
	else
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Unknown device generator: %s"), pDesc->GetTag().GetPointer());
		return ERR_FAIL;
		}

	if ((error = pGenerator->LoadFromXML(Ctx, pDesc)))
		{
		if (pGenerator)
			delete pGenerator;
		return error;
		}

	*retpGenerator = pGenerator;

	return NOERROR;
	}

//	CSingleDevice -------------------------------------------------------------

CSingleDevice::~CSingleDevice (void)

//	CSingleDevice destructor

	{
	if (m_pExtraItems)
		delete m_pExtraItems;
	}

void CSingleDevice::AddDevices (CDeviceDescList &Result)

//	AddDevices
//
//	Add devices to list

	{
	int i;

	if (m_pItemType == NULL)
		return;

	int iCount = m_Count.Roll();
	for (i = 0; i < iCount; i++)
		{
		//	Initialize the desc

		SDeviceDesc Desc;
		Desc.Item = CItem(m_pItemType, 1);
		if (mathRandom(1, 100) <= m_iDamaged)
			Desc.Item.SetDamaged();
		else
			m_Enhanced.EnhanceItem(Desc.Item);

		Desc.iPosAngle = m_iPosAngle;
		Desc.iPosRadius = m_iPosRadius;

		Desc.bOmnidirectional = m_bOmnidirectional;
		Desc.iMinFireArc = m_iMinFireArc;
		Desc.iMaxFireArc = m_iMaxFireArc;
		Desc.bSecondary = m_bSecondary;

		//	Add extra items

		if (m_pExtraItems)
			{
			CItemListManipulator ItemList(Desc.ExtraItems);
			m_pExtraItems->AddItems(ItemList);
			}

		//	Done
		
		Result.AddDeviceDesc(Desc);
		}
	}

ALERROR CSingleDevice::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load from XML

	{
	ALERROR error;

	//	Load the item type

	CString sUNID = pDesc->GetAttribute(DEVICE_ID_ATTRIB);
	if (sUNID.IsBlank())
		sUNID = pDesc->GetAttribute(ITEM_ATTRIB);

	m_pItemType.LoadUNID(Ctx, sUNID);
	if (m_pItemType.GetUNID() == 0)
		{
		Ctx.sError = strPatternSubst(CONSTLIT("<%s> element missing item attribute."), pDesc->GetTag().GetPointer());
		return ERR_FAIL;
		}

	//	Load the count

	m_Count.LoadFromXML(pDesc->GetAttribute(COUNT_ATTRIB));
	if (m_Count.IsEmpty())
		m_Count.SetConstant(1);

	//	Load damage chance

	m_iDamaged = pDesc->GetAttributeInteger(DAMAGED_ATTRIB);

	//	Load enhancement chance

	if ((error = m_Enhanced.InitFromXML(Ctx, pDesc)))
		return error;

	//	Load device desc attributes

	m_iPosAngle = pDesc->GetAttributeInteger(POS_ANGLE_ATTRIB);
	m_iPosRadius = pDesc->GetAttributeInteger(POS_RADIUS_ATTRIB);

	m_bOmnidirectional = pDesc->GetAttributeBool(OMNIDIRECTIONAL_ATTRIB);
	m_iMinFireArc = pDesc->GetAttributeInteger(MIN_FIRE_ARC_ATTRIB);
	m_iMaxFireArc = pDesc->GetAttributeInteger(MAX_FIRE_ARC_ATTRIB);
	m_bSecondary = pDesc->GetAttributeBool(SECONDARY_WEAPON_ATTRIB);

	//	Load extra items

	CXMLElement *pItems = pDesc->GetContentElementByTag(ITEMS_TAG);
	if (pItems)
		{
		if ((error = IItemGenerator::CreateFromXML(Ctx, pItems, &m_pExtraItems)))
			return error;
		}

	return NOERROR;
	}

ALERROR CSingleDevice::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Resolve references

	{
	ALERROR error;

	if ((error = m_pItemType.Bind(Ctx)))
		return error;

	if (m_pExtraItems)
		if ((error = m_pExtraItems->OnDesignLoadComplete(Ctx)))
			return error;

	//	Error checking

	if (m_pItemType)
		if (m_pItemType->GetDeviceClass() == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("%s is not a device"), m_pItemType->GetName(NULL, true).GetPointer());
			return ERR_FAIL;
			}

	return NOERROR;
	}

//	CTableOfDeviceGenerators --------------------------------------------------

CTableOfDeviceGenerators::~CTableOfDeviceGenerators (void)

//	CTableOfDeviceGenerators destructor

	{
	int i;

	if (m_Table)
		{
		for (i = 0; i < m_iTableCount; i++)
			if (m_Table[i].pDevice)
				delete m_Table[i].pDevice;

		delete [] m_Table;
		}
	}

void CTableOfDeviceGenerators::AddDevices (CDeviceDescList &Result)

//	AddDevices
//
//	Add devices

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
				m_Table[i].pDevice->AddDevices(Result);
				break;
				}
			}
		}
	}

ALERROR CTableOfDeviceGenerators::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load from XML

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

			if ((error = IDeviceGenerator::CreateFromXML(Ctx, pEntry, &m_Table[i].pDevice)))
				return error;
			}

		if (iTotal != 100)
			{
			Ctx.sError = CONSTLIT("Device table probabilities do not add up to 100%");
			return ERR_FAIL;
			}
		}
	else
		m_Table = NULL;

	return NOERROR;
	}

ALERROR CTableOfDeviceGenerators::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Resolve references

	{
	int i;
	ALERROR error;

	for (i = 0; i < m_iTableCount; i++)
		{
		if ((error = m_Table[i].pDevice->OnDesignLoadComplete(Ctx)))
			return error;
		}

	return NOERROR;
	}

//	CGroupOfDeviceGenerators --------------------------------------------------

CGroupOfDeviceGenerators::~CGroupOfDeviceGenerators (void)

//	CGroupOfDeviceGenerators destructor

	{
	int i;

	if (m_Table)
		{
		for (i = 0; i < m_iTableCount; i++)
			if (m_Table[i].pDevice)
				delete m_Table[i].pDevice;

		delete [] m_Table;
		}
	}

void CGroupOfDeviceGenerators::AddDevices (CDeviceDescList &Result)

//	AddDevices
//
//	Add devices

	{
	int i, j;

	int iCount = m_Count.Roll();
	for (j = 0; j < iCount; j++)
		{
		for (i = 0; i < m_iTableCount; i++)
			{
			if (mathRandom(1, 100) <= m_Table[i].iChance)
				m_Table[i].pDevice->AddDevices(Result);
			}
		}
	}

ALERROR CGroupOfDeviceGenerators::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load from XML

	{
	int i;
	ALERROR error;

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

			if ((error = IDeviceGenerator::CreateFromXML(Ctx, pEntry, &m_Table[i].pDevice)))
				return error;
			}
		}
	else
		m_Table = NULL;

	return NOERROR;
	}

ALERROR CGroupOfDeviceGenerators::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Resolve references

	{
	int i;
	ALERROR error;

	for (i = 0; i < m_iTableCount; i++)
		{
		if ((error = m_Table[i].pDevice->OnDesignLoadComplete(Ctx)))
			return error;
		}

	return NOERROR;
	}

//	CDeviceDescList -----------------------------------------------------------

const int ALLOC_SIZE =							16;

CDeviceDescList::CDeviceDescList (void) :
		m_pDesc(NULL),
		m_iCount(0),
		m_iAlloc(0)

//	CDeviceDescList constructor

	{
	}

CDeviceDescList::~CDeviceDescList (void)

//	CDeviceDescList destructor

	{
	if (m_pDesc)
		delete [] m_pDesc;
	}

void CDeviceDescList::AddDeviceDesc (const SDeviceDesc &Desc)

//	AddDeviceDesc
//
//	Adds a device desc to the list

	{
	//	Allocate, if necessary

	if (m_iCount == m_iAlloc)
		{
		int iNewAlloc = m_iAlloc + ALLOC_SIZE;
		SDeviceDesc *pNewDesc = new SDeviceDesc [iNewAlloc];

		for (int i = 0; i < m_iCount; i++)
			pNewDesc[i] = m_pDesc[i];

		if (m_pDesc)
			delete [] m_pDesc;

		m_pDesc = pNewDesc;
		m_iAlloc = iNewAlloc;
		}

	//	Add to the end

	m_pDesc[m_iCount++] = Desc;
	}

CDeviceClass *CDeviceDescList::GetNamedDevice (DeviceNames iDev) const

//	GetNamedDevice
//
//	Returns the named device (or NULL if not found)

	{
	int i;

	for (i = 0; i < m_iCount; i++)
		{
		CDeviceClass *pDevice = GetDeviceClass(i);

		if (iDev == devPrimaryWeapon && pDevice->GetCategory() == itemcatWeapon)
			return pDevice;
		else if (iDev == devMissileWeapon && pDevice->GetCategory() == itemcatLauncher)
			return pDevice;
		else if (iDev == devShields && pDevice->GetCategory() == itemcatShields)
			return pDevice;
		else if (iDev == devDrive && pDevice->GetCategory() == itemcatDrive)
			return pDevice;
		else if (iDev == devCargo && pDevice->GetCategory() == itemcatCargoHold)
			return pDevice;
		else if (iDev == devReactor && pDevice->GetCategory() == itemcatReactor)
			return pDevice;
		}

	return NULL;
	}

void CDeviceDescList::RemoveAll (void)

//	RemoveAll
//
//	Removes all devices

	{
	if (m_pDesc)
		{
		delete [] m_pDesc;
		m_pDesc = NULL;
		}

	m_iAlloc = 0;
	m_iCount = 0;
	}
