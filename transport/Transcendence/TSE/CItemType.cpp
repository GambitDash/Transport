//	CItemType.cpp
//
//	CItemType class

#include "PreComp.h"

#include "Kernel.h"

#define NAMES_TAG					CONSTLIT("Names")
#define IMAGE_TAG					CONSTLIT("Image")
#define COCKPIT_USE_TAG				CONSTLIT("Invoke")
#define ARMOR_TAG					CONSTLIT("Armor")
#define WEAPON_CLASS_TAG			CONSTLIT("Weapon")
#define SHIELD_CLASS_TAG			CONSTLIT("Shields")
#define DRIVE_CLASS_TAG				CONSTLIT("DriveDevice")
#define CARGO_HOLD_CLASS_TAG		CONSTLIT("CargoHoldDevice")
#define REPAIRER_CLASS_TAG			CONSTLIT("RepairerDevice")
#define SOLAR_DEVICE_CLASS_TAG		CONSTLIT("SolarDevice")
#define ENHANCER_CLASS_TAG			CONSTLIT("EnhancerDevice")
#define CYBER_DECK_CLASS_TAG		CONSTLIT("CyberDeckDevice")
#define AUTO_DEFENSE_CLASS_TAG		CONSTLIT("AutoDefenseDevice")
#define MISCELLANEOUS_CLASS_TAG		CONSTLIT("MiscellaneousDevice")
#define EVENTS_TAG					CONSTLIT("Events")
#define REACTOR_CLASS_TAG			CONSTLIT("ReactorDevice")
#define ON_REFUEL_TAG				CONSTLIT("OnRefuel")

#define UNID_ATTRIB					CONSTLIT("UNID")
#define DATA_ATTRIB					CONSTLIT("data")
#define USE_SCREEN_ATTRIB			CONSTLIT("useScreen")
#define UNKNOWN_TYPE_ATTRIB			CONSTLIT("unknownType")
#define LEVEL_ATTRIB				CONSTLIT("level")
#define SORT_NAME_ATTRIB			CONSTLIT("sortName")
#define MODIFIERS_ATTRIB			CONSTLIT("modifiers")
#define FREQUENCY_ATTRIB			CONSTLIT("frequency")
#define NUMBER_APPEARING_ATTRIB		CONSTLIT("numberAppearing")
#define ES_PLURAL_ATTRIB			CONSTLIT("esPlural")
#define PLURAL_NAME_ATTRIB			CONSTLIT("pluralName")
#define SHOW_REFERENCE_ATTRIB		CONSTLIT("showReference")
#define SECOND_PLURAL_ATTRIB		CONSTLIT("secondPlural")
#define INSTANCE_DATA_ATTRIB		CONSTLIT("charges")
#define REVERSE_ARTICLE_ATTRIB		CONSTLIT("reverseArticle")
#define VIRTUAL_ATTRIB				CONSTLIT("virtual")
#define KEY_ATTRIB					CONSTLIT("key")
#define INSTALLED_ONLY_ATTRIB		CONSTLIT("installedOnly")
#define VALUE_CHARGES_ATTRIB		CONSTLIT("valueCharges")
#define UNINSTALLED_ONLY_ATTRIB		CONSTLIT("uninstalledOnly")

#define STR_FUEL					CONSTLIT("Fuel")
#define STR_MISSILE					CONSTLIT("Missile")

#define FIELD_LEVEL					CONSTLIT("level")
#define FIELD_CATEGORY				CONSTLIT("category")
#define FIELD_FREQUENCY				CONSTLIT("frequency")
#define FIELD_NAME					CONSTLIT("name")
#define FIELD_MASS					CONSTLIT("mass")
#define FIELD_SHORT_NAME			CONSTLIT("shortName")
#define FIELD_REFERENCE				CONSTLIT("reference")
#define FIELD_COST					CONSTLIT("cost")
#define FIELD_AVERAGE_COUNT			CONSTLIT("averageCount")
#define FIELD_INSTALL_COST			CONSTLIT("installCost")

static char g_NameAttrib[] = "name";
static char g_ObjectAttrib[] = "object";
static char g_ValueAttrib[] = "value";
static char g_MassAttrib[] = "mass";
static char g_DescriptionAttrib[] = "description";
static char g_FirstPluralAttrib[] = "firstPlural";
static char g_RandomDamagedAttrib[] = "randomDamaged";

CItemType::CItemType (void) : 
		m_dwSpare(0),
		m_pUseCode(NULL),
		m_pArmor(NULL),
		m_pDevice(NULL)

//	CItemType constructor

	{
	}

CItemType::~CItemType (void)

//	CItemType destructor

	{
	if (m_pUseCode)
		m_pUseCode->Discard(&g_pUniverse->GetCC());

	if (m_pArmor)
		delete m_pArmor;

	if (m_pDevice)
		delete m_pDevice;
	}

ALERROR CItemType::ComposeError (const CString &sName, const CString &sError, CString *retsError)

//	ComposeError
//
//	Composes an error string when loading

	{
	*retsError = strPatternSubst("%s: %s", sName.GetASCIIZPointer(), sError.GetASCIIZPointer());
	return ERR_FAIL;
	}

bool CItemType::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns meta-data

	{
	//	Deal with the meta-data that we know about

	if (strEquals(sField, FIELD_LEVEL))
		{
		*retsValue = strFromInt(GetLevel(), FALSE);
		return true;
		}
	else if (strEquals(sField, FIELD_CATEGORY))
		{
		switch (GetCategory())
			{
			case itemcatArmor:
				*retsValue = CONSTLIT("Armor");
				break;

			case itemcatCargoHold:
				*retsValue = CONSTLIT("CargoHold");
				break;

			case itemcatDrive:
				*retsValue = CONSTLIT("Device");
				break;

			case itemcatFuel:
				*retsValue = CONSTLIT("Fuel");
				break;

			case itemcatLauncher:
				*retsValue = CONSTLIT("Launcher");
				break;

			case itemcatMisc:
				*retsValue = CONSTLIT("Misc");
				break;

			case itemcatMissile:
				*retsValue = CONSTLIT("Missile");
				break;

			case itemcatMiscDevice:
				*retsValue = CONSTLIT("Device");
				break;

			case itemcatReactor:
				*retsValue = CONSTLIT("Reactor");
				break;

			case itemcatShields:
				*retsValue = CONSTLIT("Shield");
				break;

			case itemcatUseful:
				*retsValue = CONSTLIT("Useful");
				break;

			case itemcatWeapon:
				*retsValue = CONSTLIT("Weapon");
				break;

			default:
				*retsValue = CONSTLIT("Unknown");
				break;
			}

		return true;
		}
	else if (strEquals(sField, FIELD_FREQUENCY))
		{
		switch (GetFrequency())
			{
			case ftCommon:
				*retsValue = CONSTLIT("C");
				break;

			case ftUncommon:
				*retsValue = CONSTLIT("UC");
				break;

			case ftRare:
				*retsValue = CONSTLIT("R");
				break;

			case ftVeryRare:
				*retsValue = CONSTLIT("VR");
				break;

			default:
				*retsValue = CONSTLIT("NR");
			}

		return true;
		}
	else if (strEquals(sField, FIELD_NAME))
		{
		*retsValue = GetNounPhrase();
		return true;
		}
	else if (strEquals(sField, FIELD_MASS))
		{
		*retsValue = strFromInt(GetMassKg(), FALSE);
		return true;
		}
	else if (strEquals(sField, FIELD_SHORT_NAME))
		{
		CString sName = GetNounPhrase();
		int iPos = strFind(sName, CONSTLIT(" of "));
		if (iPos != -1)
			{
			CString sLead = strSubString(sName, 0, iPos);
			if (strEquals(sLead, CONSTLIT("barrel"))
					|| strEquals(sLead, CONSTLIT("bolt"))
					|| strEquals(sLead, CONSTLIT("case"))
					|| strEquals(sLead, CONSTLIT("chest"))
					|| strEquals(sLead, CONSTLIT("container"))
					|| strEquals(sLead, CONSTLIT("crate"))
					|| strEquals(sLead, CONSTLIT("keg"))
					|| strEquals(sLead, CONSTLIT("kilo"))
					|| strEquals(sLead, CONSTLIT("nodule"))
					|| strEquals(sLead, CONSTLIT("pair"))
					|| strEquals(sLead, CONSTLIT("ROM"))
					|| strEquals(sLead, CONSTLIT("segment"))
					|| strEquals(sLead, CONSTLIT("tank"))
					|| strEquals(sLead, CONSTLIT("tin"))
					|| strEquals(sLead, CONSTLIT("ton"))
					|| strEquals(sLead, CONSTLIT("vial")))
				*retsValue = strSubString(sName, iPos+4, -1);
			else
				*retsValue = sName;
			}
		else
			*retsValue = sName;

		return true;
		}
	else if (strEquals(sField, FIELD_REFERENCE))
		{
		CItem Item(this, 1);
		*retsValue = Item.GetReference();
		return true;
		}
	else if (strEquals(sField, FIELD_COST))
		{
		*retsValue = strFromInt(GetValue(), TRUE);
		return true;
		}
	else if (strEquals(sField, FIELD_INSTALL_COST))
		{
		int iCost = GetInstallCost();
		if (iCost == -1)
			*retsValue = NULL_STR;
		else
			*retsValue = strFromInt(iCost, TRUE);
		return true;
		}
	else if (strEquals(sField, FIELD_AVERAGE_COUNT))
		{
		*retsValue = strFromInt(m_NumberAppearing.GetAveValue(), TRUE);
		return true;
		}

	//	Otherwise, see if the device class knows

	if (m_pDevice)
		return m_pDevice->FindDataField(sField, retsValue);
	else if (m_pArmor)
		return m_pArmor->FindDataField(sField, retsValue);

	//	If this is a missile, then find the device that launches
	//	it and ask it for the properties.

	else if (IsMissile())
		return CDeviceClass::FindAmmoDataField(this, sField, retsValue);

	return false;
	}

int CItemType::GetApparentLevel (void) const

//	GetApparentLevel
//
//	Returns the level that the item appears to be

	{
	if (!IsKnown())
		return m_pUnknownType->GetLevel();

	return GetLevel();
	}

ItemCategories CItemType::GetCategory (void) const

//	GetCategory
//
//	Returns the category of the item

	{
	if (m_pArmor)
		return itemcatArmor;
	else if (m_pDevice)
		return m_pDevice->GetCategory();
	else
		{
		if (IsUsable())
			return itemcatUseful;
		else if (IsFuel())
			return itemcatFuel;
		else if (IsMissile())
			return itemcatMissile;
		else
			return itemcatMisc;
		}
	}

const CString &CItemType::GetDesc (void) const

//	GetDesc
//
//	Get description for the item
	
	{
	if (!IsKnown())
		return m_pUnknownType->GetDesc();

	return m_sDescription; 
	}

int CItemType::GetInstallCost (void) const

//	GetInstallCost
//
//	Returns the cost to install the item (or -1 if the item cannot be installed)

	{
	if (m_pArmor)
		return m_pArmor->GetInstallCost();
	else if (m_pDevice)
		return 100 * (((GetApparentLevel() * GetApparentLevel()) + 4) / 5);
	else
		return -1;
	}

CString CItemType::GetName (DWORD *retdwFlags, bool bActualName) const

//	GetName
//
//	Returns the name of the item and flags about the name

	{
	if (!IsKnown() && !bActualName)
		return m_pUnknownType->GetUnknownName(m_iUnknownName, retdwFlags);

	CString sName = m_sName;
	if (retdwFlags)
		{
		*retdwFlags = 0;
		if (m_fFirstPlural)
			*retdwFlags |= nounFirstPlural;
		if (m_fSecondPlural)
			*retdwFlags |= nounSecondPlural;
		if (m_fESPlural)
			*retdwFlags |= nounPluralES;
		if (m_fReverseArticle)
			*retdwFlags |= nounVowelArticle;
		if (!m_sPluralName.IsBlank())
			{
			sName = m_sPluralName;
			*retdwFlags |= nounCustomPlural;
			}
		}

	return sName;
	}

CString CItemType::GetNounPhrase (DWORD dwFlags) const

//	GetNounPhrase
//
//	Returns the generic name of the item

	{
	DWORD dwNameFlags;

	CString sName;
	if (dwFlags & nounShort)
		sName = GetShortName(&dwNameFlags, ((dwFlags & nounActual) ? true : false));
	else
		sName = GetName(&dwNameFlags, ((dwFlags & nounActual) ? true : false));

	return ::ComposeNounPhrase(sName, 1, NULL_STR, dwNameFlags, dwFlags);
	}

CString CItemType::GetReference (void) const

//	GetReference
//
//	Returns reference string

	{
	//	Return armor reference, if this is armor

	if (m_pArmor)
		return m_pArmor->GetReference();

	//	Return device reference, if this is a device

	else if (m_pDevice)
		return m_pDevice->GetReference();

	//	Otherwise, nothing

	else
		return NULL_STR;
	}

CString CItemType::GetShortName (DWORD *retdwFlags, bool bActualName) const

//	GetShortName
//
//	Returns the short name for the item

	{
	DWORD dwFlags;
	CString sName = GetName(&dwFlags, bActualName);

	int iPos = strFind(sName, CONSTLIT(" of "));
	if (iPos != -1)
		{
		CString sLead = strSubString(sName, 0, iPos);
		if (strEquals(sLead, CONSTLIT("barrel"))
				|| strEquals(sLead, CONSTLIT("bolt"))
				|| strEquals(sLead, CONSTLIT("case"))
				|| strEquals(sLead, CONSTLIT("chest"))
				|| strEquals(sLead, CONSTLIT("container"))
				|| strEquals(sLead, CONSTLIT("crate"))
				|| strEquals(sLead, CONSTLIT("keg"))
				|| strEquals(sLead, CONSTLIT("kilo"))
				|| strEquals(sLead, CONSTLIT("nodule"))
				|| strEquals(sLead, CONSTLIT("pair"))
				|| strEquals(sLead, CONSTLIT("ROM"))
				|| strEquals(sLead, CONSTLIT("segment"))
				|| strEquals(sLead, CONSTLIT("tank"))
				|| strEquals(sLead, CONSTLIT("tin"))
				|| strEquals(sLead, CONSTLIT("ton")))
			sName = strSubString(sName, iPos+4, -1);
		}

	if (retdwFlags)
		*retdwFlags = dwFlags;

	return sName;
	}

CString CItemType::GetSortName (void) const

//	GetSortName
//
//	Returns the sort name

	{
	if (!IsKnown())
		return m_pUnknownType->GetUnknownName(m_iUnknownName, NULL);
	else
		return m_sSortName;
	}

CString CItemType::GetUnknownName (int iIndex, DWORD *retdwFlags)

//	GetUnknownName
//
//	Returns the unknown name of the item

	{
	CString sName = GetName(retdwFlags);
	if (iIndex != -1 && iIndex < m_UnknownNames.GetCount())
		sName = m_UnknownNames.GetStringValue(iIndex);

	return sName;
	}

int CItemType::GetValue (bool bActual) const

//	GetValue
//
//	Returns the value of the item

	{
	if (!IsKnown() && !bActual)
		return m_pUnknownType->GetValue();

	return m_iValue;
	}

void CItemType::InitRandomNames (void)

//	InitRandomNames
//
//	Initialize random names

	{
	int i;

	//	If we don't have random names for other items then we're done

	int iCount = m_UnknownNames.GetCount();
	if (iCount == 0)
		return;

	//	Randomize the names

	CIntArray Randomize;
	for (i = 0; i < iCount; i++)
		Randomize.AppendElement(i, NULL);

	Randomize.Shuffle();

	//	Loop over all items and assign each item that has us as the
	//	unknown placeholder.

	int j = 0;
	for (i = 0; i < g_pUniverse->GetItemTypeCount(); i++)
		{
		CItemType *pType = g_pUniverse->GetItemType(i);
		if (pType->m_pUnknownType.GetUNID() == GetUNID())
			{
			pType->m_iUnknownName = Randomize.GetElement(j % iCount);
			j++;
			}
		}
	}

bool CItemType::IsFuel (void) const

//	IsFuel
//
//	Returns TRUE if this is fuel

	{
	return HasModifier(STR_FUEL);
	}

bool CItemType::IsMissile (void) const

//	IsMissile
//
//	Returns TRUE if this is a missile

	{
	return HasModifier(STR_MISSILE);
	}

ALERROR CItemType::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind all pointers to other design elements

	{
	ALERROR error;

	InitRandomNames();

	//	Images

	if ((error = m_Image.OnDesignLoadComplete(Ctx)))
		return error;

	//	Resolve some pointers

	if ((error = m_pUnknownType.Bind(Ctx)))
		return error;

	if ((error = m_pUseScreen.Bind(Ctx)))
		return error;

	//	Call contained objects

	if (m_pDevice)
		if ((error = m_pDevice->OnDesignLoadComplete(Ctx)))
			return error;

	return NOERROR;
	}

ALERROR CItemType::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Load data from XML

	{
	ALERROR error;
	int i;

	//	Initialize basic info

	m_sName = pDesc->GetAttribute(CONSTLIT(g_NameAttrib));
	m_iUnknownName = -1;
	m_sPluralName = pDesc->GetAttribute(PLURAL_NAME_ATTRIB);
	m_sSortName = pDesc->GetAttribute(SORT_NAME_ATTRIB);
	if (m_sSortName.IsBlank())
		m_sSortName = m_sName;
	m_iLevel = pDesc->GetAttributeInteger(LEVEL_ATTRIB);
	if (m_iLevel <= 0 || m_iLevel > MAX_ITEM_LEVEL)
		m_iLevel = 1;
	m_iValue = pDesc->GetAttributeInteger(CONSTLIT(g_ValueAttrib));
	m_iMass = pDesc->GetAttributeInteger(CONSTLIT(g_MassAttrib));

	//	Initialize frequency

	m_Frequency = (FrequencyTypes)::GetFrequency(pDesc->GetAttribute(FREQUENCY_ATTRIB));
	CString sNumberAppearing = pDesc->GetAttribute(NUMBER_APPEARING_ATTRIB);
	if (sNumberAppearing.IsBlank())
		m_NumberAppearing = DiceRange(0, 0, 1);
	else
		{
		if ((error = m_NumberAppearing.LoadFromXML(sNumberAppearing)))
			return ComposeError(m_sName, CONSTLIT("Unable to parse numberAppearing"), &Ctx.sError);
		}

	//	Get the unknown type info

	m_pUnknownType.LoadUNID(Ctx, pDesc->GetAttribute(UNKNOWN_TYPE_ATTRIB));
	m_fKnown = (m_pUnknownType.GetUNID() == 0);

	//	Get the reference info

	m_fDefaultReference = pDesc->GetAttributeBool(SHOW_REFERENCE_ATTRIB);
	m_fReference = m_fDefaultReference;

	//	More fields

	m_sDescription = pDesc->GetAttribute(CONSTLIT(g_DescriptionAttrib));
	m_sData = pDesc->GetAttribute(DATA_ATTRIB);
	m_sModifiers = pDesc->GetAttribute(MODIFIERS_ATTRIB);

	//	Initial data value

	CString sInstData;
	if ((m_fInstanceData = pDesc->FindAttribute(INSTANCE_DATA_ATTRIB, &sInstData)))
		m_InitDataValue.LoadFromXML(sInstData);
	m_fValueCharges = pDesc->GetAttributeBool(VALUE_CHARGES_ATTRIB);

	//	Flags

	m_fFirstPlural = pDesc->GetAttributeBool(CONSTLIT(g_FirstPluralAttrib));
	m_fSecondPlural = pDesc->GetAttributeBool(SECOND_PLURAL_ATTRIB);
	m_fESPlural = pDesc->GetAttributeBool(ES_PLURAL_ATTRIB);
	m_fRandomDamaged = pDesc->GetAttributeBool(CONSTLIT(g_RandomDamagedAttrib));
	m_fReverseArticle = pDesc->GetAttributeBool(REVERSE_ARTICLE_ATTRIB);
	m_fVirtual = pDesc->GetAttributeBool(VIRTUAL_ATTRIB);
	if (m_fVirtual)
		m_Frequency = ftNotRandom;

	//	Use screen

	m_pUseScreen.LoadUNID(Ctx, pDesc->GetAttribute(USE_SCREEN_ATTRIB));

	//	Process sub-elements

	m_fUseInstalled = false;
	m_fUseUninstalled = false;
	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pSubDesc = pDesc->GetContentElement(i);

		//	Process image

		if (strEquals(pSubDesc->GetTag(), IMAGE_TAG))
			{
			if ((error = m_Image.InitFromXML(Ctx, pSubDesc)))
				return ComposeError(m_sName, CONSTLIT("Unable to load image"), &Ctx.sError);
			}

		//	Process unknown names

		else if (strEquals(pSubDesc->GetTag(), NAMES_TAG))
			strDelimitEx(pSubDesc->GetContentText(0), ';', DELIMIT_TRIM_WHITESPACE, 0, &m_UnknownNames);

		//	Process use code

		else if (strEquals(pSubDesc->GetTag(), COCKPIT_USE_TAG))
			{
			m_pUseCode = g_pUniverse->GetCC().Link(pSubDesc->GetContentText(0), 0, NULL);

			m_sUseKey = pSubDesc->GetAttribute(KEY_ATTRIB);
			m_fUseInstalled = pSubDesc->GetAttributeBool(INSTALLED_ONLY_ATTRIB);
			m_fUseUninstalled = pSubDesc->GetAttributeBool(UNINSTALLED_ONLY_ATTRIB);
			}

		//	Process events (skip processing here since we processed above)

		else if (strEquals(pSubDesc->GetTag(), EVENTS_TAG))
			;

		//	Process on refuel code

		else if (strEquals(pSubDesc->GetTag(), ON_REFUEL_TAG))
			{
			if ((error = AddEventHandler(ON_REFUEL_TAG, pSubDesc->GetContentText(0), &Ctx.sError)))
				return ComposeError(m_sName, CONSTLIT("Unable to load OnRefuel event"), &Ctx.sError);
			}

		//	Armor

		else if (strEquals(pSubDesc->GetTag(), ARMOR_TAG))
			{
			if ((error = CArmorClass::CreateFromXML(Ctx, pSubDesc, this, &m_pArmor)))
				return ComposeError(m_sName, CONSTLIT("Unable to load Armor descriptor"), &Ctx.sError);
			}

		//	Devices

		else if (strEquals(pSubDesc->GetTag(), WEAPON_CLASS_TAG))
			{
			if ((error = CWeaponClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice)))
				return ComposeError(m_sName, Ctx.sError, &Ctx.sError);
			}
		else if (strEquals(pSubDesc->GetTag(), SHIELD_CLASS_TAG))
			{
			if ((error = CShieldClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice)))
				return ComposeError(m_sName, CONSTLIT("Unable to load shield descriptor"), &Ctx.sError);
			}
		else if (strEquals(pSubDesc->GetTag(), DRIVE_CLASS_TAG))
			{
			if ((error = CDriveClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice)))
				return ComposeError(m_sName, CONSTLIT("Unable to load drive descriptor"), &Ctx.sError);
			}
		else if (strEquals(pSubDesc->GetTag(), CARGO_HOLD_CLASS_TAG))
			{
			if ((error = CCargoSpaceClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice)))
				return ComposeError(m_sName, CONSTLIT("Unable to load cargo hold descriptor"), &Ctx.sError);
			}
		else if (strEquals(pSubDesc->GetTag(), ENHANCER_CLASS_TAG))
			{
			if ((error = CEnhancerClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice)))
				return ComposeError(m_sName, CONSTLIT("Unable to load enhancer descriptor"), &Ctx.sError);
			}
		else if (strEquals(pSubDesc->GetTag(), REPAIRER_CLASS_TAG))
			{
			if ((error = CRepairerClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice)))
				return ComposeError(m_sName, CONSTLIT("Unable to load repairer descriptor"), &Ctx.sError);
			}
		else if (strEquals(pSubDesc->GetTag(), SOLAR_DEVICE_CLASS_TAG))
			{
			if ((error = CSolarDeviceClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice)))
				return ComposeError(m_sName, CONSTLIT("Unable to load solar device descriptor"), &Ctx.sError);
			}
		else if (strEquals(pSubDesc->GetTag(), CYBER_DECK_CLASS_TAG))
			{
			if ((error = CCyberDeckClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice)))
				return ComposeError(m_sName, CONSTLIT("Unable to load cyber device descriptor"), &Ctx.sError);
			}
		else if (strEquals(pSubDesc->GetTag(), AUTO_DEFENSE_CLASS_TAG))
			{
			if ((error = CAutoDefenseClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice)))
				return ComposeError(m_sName, CONSTLIT("Unable to load defense device descriptor"), &Ctx.sError);
			}
		else if (strEquals(pSubDesc->GetTag(), MISCELLANEOUS_CLASS_TAG))
			{
			if ((error = CMiscellaneousClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice)))
				return ComposeError(m_sName, CONSTLIT("Unable to load miscellaneous device descriptor"), &Ctx.sError);
			}
		else if (strEquals(pSubDesc->GetTag(), REACTOR_CLASS_TAG))
			{
			if ((error = CReactorClass::CreateFromXML(Ctx, pSubDesc, this, &m_pDevice)))
				return ComposeError(m_sName, CONSTLIT("Unable to load reactor descriptor"), &Ctx.sError);
			}
		else
			kernelDebugLogMessage("Unknown sub-element for ItemType: %s", pSubDesc->GetTag().GetASCIIZPointer());
		}

	//	Done

	return NOERROR;
	}

CEffectCreator *CItemType::OnFindEffectCreator (const CString &sUNID)

//	OnFindEffectCreator
//
//	Find the effect creator. sUNID is the remainder after the item type UNID has been removed

	{
	if (m_pDevice)
		return m_pDevice->FindEffectCreator(sUNID);
	else
		return NULL;
	}

void CItemType::OnReadFromStream (SUniverseLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read from stream
//
//	DWORD		flags
//	DWORD		m_iUnknownName

	{
	DWORD dwLoad;
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	
	m_fKnown =		((dwLoad & 0x00000001) ? true : false);
	m_fReference =	((dwLoad & 0x00000002) ? true : false);

	Ctx.pStream->Read((char *)&m_iUnknownName, sizeof(DWORD));

	//	This can happen if we change the known/unknown status of an
	//	item type across save games

	if (m_pUnknownType == NULL)
		m_fKnown = true;
	}

void CItemType::OnReinit (void)

//	Reinit
//
//	Reinitialize when the game starts again

	{
	if (GetUnknownType())
		m_fKnown = false;

	m_fReference = m_fDefaultReference;

	InitRandomNames();
	}

void CItemType::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Writes out the variable portions of the item type
//
//	DWORD		flags
//	DWORD		m_iUnknownName

	{
	DWORD dwSave;

	dwSave = 0;
	dwSave |= (m_fKnown ?		0x00000001 : 0);
	dwSave |= (m_fReference ?	0x00000002 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	pStream->Write((char *)&m_iUnknownName, sizeof(DWORD));
	}
