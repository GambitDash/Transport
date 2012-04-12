//	CDesignType.cpp
//
//	CDesignType class

#include "PreComp.h"

#define EFFECT_TAG								CONSTLIT("Effect")
#define SHIP_ENERGY_FIELD_TYPE_TAG				CONSTLIT("ShipEnergyFieldType")
#define SPACE_ENVIRONMENT_TYPE_TAG				CONSTLIT("SpaceEnvironmentType")
#define ITEM_TABLE_TAG							CONSTLIT("ItemTable")
#define DOCK_SCREEN_TAG							CONSTLIT("DockScreen")
#define ITEM_TYPE_TAG							CONSTLIT("ItemType")
#define SHIP_CLASS_TAG							CONSTLIT("ShipClass")
#define STATION_TYPE_TAG						CONSTLIT("StationType")
#define SYSTEM_TYPE_TAG							CONSTLIT("SystemType")
#define SOVEREIGN_TAG							CONSTLIT("Sovereign")
#define ENCOUNTER_TABLE_TAG						CONSTLIT("EncounterTable")
#define POWER_TAG								CONSTLIT("Power")
#define ADVENTURE_DESC_TAG						CONSTLIT("AdventureDesc")
#define STATIC_DATA_TAG							CONSTLIT("StaticData")
#define GLOBAL_DATA_TAG							CONSTLIT("GlobalData")
#define EVENTS_TAG								CONSTLIT("Events")

#define UNID_ATTRIB								CONSTLIT("UNID")

#define ON_GLOBAL_TOPOLOGY_CREATED_EVENT		CONSTLIT("OnGlobalTopologyCreated")
#define ON_GLOBAL_SYSTEM_CREATED_EVENT			CONSTLIT("OnGlobalSystemCreated")

ALERROR CDesignType::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CDesignType **retpType)

//	CreateFromXML
//
//	Creates a design type from an XML element

	{
	ALERROR error;
	CDesignType *pType = NULL;

	if (strEquals(pDesc->GetTag(), ITEM_TYPE_TAG))
		pType = new CItemType;
	else if (strEquals(pDesc->GetTag(), ITEM_TABLE_TAG))
		pType = new CItemTable;
	else if (strEquals(pDesc->GetTag(), EFFECT_TAG))
		{
		//	CEffectCreators create different classes depending on the XML

		if (error = CEffectCreator::CreateFromXML(Ctx, pDesc, NULL_STR, (CEffectCreator **)&pType))
			return error;
		}
	else if (strEquals(pDesc->GetTag(), SHIP_CLASS_TAG))
		pType = new CShipClass;
	else if (strEquals(pDesc->GetTag(), SHIP_ENERGY_FIELD_TYPE_TAG))
		pType = new CEnergyFieldType;
	else if (strEquals(pDesc->GetTag(), SYSTEM_TYPE_TAG))
		pType = new CSystemType;
	else if (strEquals(pDesc->GetTag(), STATION_TYPE_TAG))
		pType = new CStationType;
	else if (strEquals(pDesc->GetTag(), SOVEREIGN_TAG))
		pType = new CSovereign;
	else if (strEquals(pDesc->GetTag(), DOCK_SCREEN_TAG))
		pType = new CDockScreenType;
	else if (strEquals(pDesc->GetTag(), POWER_TAG))
		pType = new CPower;
	else if (strEquals(pDesc->GetTag(), SPACE_ENVIRONMENT_TYPE_TAG))
		pType = new CSpaceEnvironmentType;
	else if (strEquals(pDesc->GetTag(), ENCOUNTER_TABLE_TAG))
		pType = new CShipTable;
	else if (strEquals(pDesc->GetTag(), ADVENTURE_DESC_TAG))
		{
		//	Only valid if we are inside an Adventure

		if (Ctx.pExtension != NULL && Ctx.pExtension->iType != extAdventure)
			{
			Ctx.sError = CONSTLIT("<AdventureDesc> element is only valid for Adventures");
			return ERR_FAIL;
			}

		pType = new CAdventureDesc;
		}
	else
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Unknown design element: <%s>"), pDesc->GetTag().GetASCIIZPointer());
		return ERR_FAIL;
		}

	//	Load UNID

	pType->m_dwUNID = ::LoadUNID(Ctx, pDesc->GetAttribute(UNID_ATTRIB));

	//	Load static data

	CXMLElement *pStaticData = pDesc->GetContentElementByTag(STATIC_DATA_TAG);
	if (pStaticData)
		pType->m_StaticData.SetFromXML(pStaticData);

	//	Load global data

	CXMLElement *pGlobalData = pDesc->GetContentElementByTag(GLOBAL_DATA_TAG);
	if (pGlobalData)
		{
		pType->m_InitGlobalData.SetFromXML(pGlobalData);
		pType->m_GlobalData = pType->m_InitGlobalData;
		}

	//	Load Events

	CXMLElement *pEventsDesc = pDesc->GetContentElementByTag(EVENTS_TAG);
	if (pEventsDesc)
		{
		if (error = pType->m_Events.InitFromXML(Ctx, pEventsDesc))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Unable to load event: %s"), Ctx.sError.GetPointer());
			return error;
			}
		}

	//	Load specific data

	if (error = pType->OnCreateFromXML(Ctx, pDesc))
		return error;

	//	Done

	if (retpType)
		*retpType = pType;

	return NOERROR;
	}

ALERROR CDesignType::FireOnGlobalSystemCreated (CString *retsError)

//	FireOnGlobalSystemCreated
//
//	Fire event

	{
	ICCItem *pCode;

	if (FindEventHandler(ON_GLOBAL_SYSTEM_CREATED_EVENT, &pCode))
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		//	Run code

		ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
		if (pResult->IsError())
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("%x %s: %s"), m_dwUNID, ON_GLOBAL_SYSTEM_CREATED_EVENT.GetPointer(), pResult->GetStringValue().GetPointer());
			return ERR_FAIL;
			}

		pResult->Discard(&CC);
		}

	return NOERROR;
	}

ALERROR CDesignType::FireOnGlobalTopologyCreated (CString *retsError)

//	FireOnGlobalTopologyCreated
//
//	Fire event

	{
	ICCItem *pCode;

	if (FindEventHandler(ON_GLOBAL_TOPOLOGY_CREATED_EVENT, &pCode))
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		//	Run code

		ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
		if (pResult->IsError())
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("%x %s: %s"), m_dwUNID, ON_GLOBAL_TOPOLOGY_CREATED_EVENT.GetPointer(), pResult->GetStringValue().GetPointer());
			return ERR_FAIL;
			}

		pResult->Discard(&CC);
		}

	return NOERROR;
	}

ICCItem *CDesignType::GetEventHandler (const CString &sEvent) const

//	GetEventHandler
//
//	Returns an event handler (or NULL)

	{
	ICCItem *pCode;
	if (m_Events.FindEvent(sEvent, &pCode))
		return pCode;
	else
		return NULL;
	}

void CDesignType::ReadFromStream (SUniverseLoadCtx &Ctx)

//	ReadFromStream
//
//	Reads the variant portions of the design type
	
	{
	//	Read global data

	if (Ctx.dwVersion >= 3)
		m_GlobalData.ReadFromStream(Ctx.pStream);

	//	Allow sub-classes to load

	OnReadFromStream(Ctx);
	}

void CDesignType::ReadGlobalData (SUniverseLoadCtx &Ctx)

//	ReadGlobalData
//
//	For compatibility with older save versions

	{
	if (Ctx.dwVersion < 3)
		m_GlobalData.ReadFromStream(Ctx.pStream);
	}

void CDesignType::Reinit (void)

//	Reinit
//
//	Reinitializes the variant portions of the design type
	
	{
	//	Reinit global data

	m_GlobalData = m_InitGlobalData;

	//	Allow sub-classes to reinit

	OnReinit();
	}

void CDesignType::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Writes the variant portions of the design type

	{
	//	Write out global data

	m_GlobalData.WriteToStream(pStream, NULL);

	//	Allow sub-classes to write

	OnWriteToStream(pStream);
	}

//	CItemTypeRef --------------------------------------------------------------

ALERROR CItemTypeRef::Bind (SDesignLoadCtx &Ctx, ItemCategories iCategory)
	{
	ALERROR error;
	if (error = CDesignTypeRef<CItemType>::Bind(Ctx))
		return error;

	if (m_pType && m_pType->GetCategory() != iCategory)
		{
		Ctx.sError = strPatternSubst(CONSTLIT("%s item expected: %x"), ::GetItemCategoryName(iCategory).GetASCIIZPointer(), m_dwUNID);
		return ERR_FAIL;
		}

	return NOERROR;
	}

//	CArmorClassRef -----------------------------------------------------------

ALERROR CArmorClassRef::Bind (SDesignLoadCtx &Ctx)
	{
	if (m_dwUNID)
		{
		CDesignType *pBaseType = g_pUniverse->FindDesignType(m_dwUNID);
		if (pBaseType == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Unknown design type: %x"), m_dwUNID);
			return ERR_FAIL;
			}

		CItemType *pItemType = CItemType::AsType(pBaseType);
		if (pItemType == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Armor item type expected: %x"), m_dwUNID);
			return ERR_FAIL;
			}

		m_pType = pItemType->GetArmorClass();
		if (m_pType == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Armor item type expected: %x"), m_dwUNID);
			return ERR_FAIL;
			}
		}

	return NOERROR;
	}

//	CDeviceClassRef -----------------------------------------------------------

ALERROR CDeviceClassRef::Bind (SDesignLoadCtx &Ctx)
	{
	if (m_dwUNID)
		{
		CDesignType *pBaseType = g_pUniverse->FindDesignType(m_dwUNID);
		if (pBaseType == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Unknown design type: %x"), m_dwUNID);
			return ERR_FAIL;
			}

		CItemType *pItemType = CItemType::AsType(pBaseType);
		if (pItemType == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Device item type expected: %x"), m_dwUNID);
			return ERR_FAIL;
			}

		m_pType = pItemType->GetDeviceClass();
		if (m_pType == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Device item type expected: %x"), m_dwUNID);
			return ERR_FAIL;
			}
		}

	return NOERROR;
	}

void CDeviceClassRef::Set (CDeviceClass *pDevice)
	{
	if (pDevice)
		{
		m_pType = pDevice;
		m_dwUNID = pDevice->GetUNID();
		}
	else
		{
		m_pType = NULL;
		m_dwUNID = 0;
		}
	}

//	CWeaponFireDescRef -----------------------------------------------------------

ALERROR CWeaponFireDescRef::Bind (SDesignLoadCtx &Ctx)
	{
	if (m_dwUNID)
		{
		CDesignType *pBaseType = g_pUniverse->FindDesignType(m_dwUNID);
		if (pBaseType == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Unknown design type: %x"), m_dwUNID);
			return ERR_FAIL;
			}

		CItemType *pItemType = CItemType::AsType(pBaseType);
		if (pItemType == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Weapon item type expected: %x"), m_dwUNID);
			return ERR_FAIL;
			}

		CDeviceClass *pDevice = pItemType->GetDeviceClass();
		if (pDevice == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Weapon item type expected: %x"), m_dwUNID);
			return ERR_FAIL;
			}

		CWeaponClass *pWeapon = pDevice->AsWeaponClass();
		if (pWeapon == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Weapon item type expected: %x"), m_dwUNID);
			return ERR_FAIL;
			}

		m_pType = pWeapon->GetVariant(0);
		if (m_pType == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Invalid weapon type: %x"), m_dwUNID);
			return ERR_FAIL;
			}
		}

	return NOERROR;
	}

//	CEffectCreatorRef ---------------------------------------------------------

CEffectCreatorRef::~CEffectCreatorRef (void)
	{
	if (m_bDelete && m_pType)
		delete m_pType;
	}

ALERROR CEffectCreatorRef::Bind (SDesignLoadCtx &Ctx)
	{
	if (m_dwUNID)
		return CDesignTypeRef<CEffectCreator>::Bind(Ctx);
	else if (m_pType)
		return m_pType->BindDesign(Ctx);

	return NOERROR;
	}

ALERROR CEffectCreatorRef::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)
	{
	ALERROR error;

	if (error = CEffectCreator::CreateFromXML(Ctx, pDesc, sUNID, &m_pType))
		return error;

	m_dwUNID = 0;
	m_bDelete = true;

	return NOERROR;
	}

ALERROR CEffectCreatorRef::LoadEffect (SDesignLoadCtx &Ctx, const CString &sUNID, CXMLElement *pDesc, const CString &sAttrib)
	{
	ALERROR error;

	if (pDesc)
		{
		if (error = CreateFromXML(Ctx, pDesc, sUNID))
			return error;
		}
	else
		LoadUNID(Ctx, sAttrib);

	return NOERROR;
	}

void CEffectCreatorRef::Set (CEffectCreator *pEffect)

//	Set
//
//	Sets the effect

	{
	if (m_bDelete && m_pType)
		delete m_pType;

	m_pType = pEffect;
	m_bDelete = false;
	if (m_pType)
		m_dwUNID = m_pType->GetUNID();
	else
		m_dwUNID = 0;
	}
