//	Devices.cpp
//
//	Ship devices

#include "PreComp.h"

#define OMNIDIRECTIONAL_ATTRIB					CONSTLIT("omnidirectional")
#define POS_ANGLE_ATTRIB						CONSTLIT("posAngle")
#define POS_RADIUS_ATTRIB						CONSTLIT("posRadius")
#define MIN_FIRE_ARC_ATTRIB						CONSTLIT("minFireArc")
#define MAX_FIRE_ARC_ATTRIB						CONSTLIT("maxFireArc")
#define SECONDARY_WEAPON_ATTRIB					CONSTLIT("secondaryWeapon")
#define UNID_ATTRIB								CONSTLIT("UNID")
#define ITEM_ID_ATTRIB							CONSTLIT("itemID")
#define DEVICE_ID_ATTRIB						CONSTLIT("deviceID")
#define DEVICE_SLOTS_ATTRIB						CONSTLIT("deviceSlots")
#define EXTERNAL_ATTRIB							CONSTLIT("external")

//	CDeviceClass

void CDeviceClass::InitDeviceFromXML (CXMLElement *pDesc, CItemType *pType)

//	InitDeviceFromXML
//
//	Initializes the device class base

	{
	m_pItemType = pType;

	//	Number of slots that the device takes up (if the attribute is missing
	//	then we assume 1)

	CString sAttrib;
	if (pDesc->FindAttribute(DEVICE_SLOTS_ATTRIB, &sAttrib))
		m_iSlots = strToInt(sAttrib, 1, NULL);
	else
		m_iSlots = 1;

	//	Is this device external?

	m_fExternal = pDesc->GetAttributeBool(EXTERNAL_ATTRIB);
	}

bool CDeviceClass::FindAmmoDataField (CItemType *pItem, const CString &sField, CString *retsValue)

//	FindAmmoDataField
//
//	Finds the device that fires this item and returns the given field

	{
	int i;

	for (i = 0; i < g_pUniverse->GetItemTypeCount(); i++)
		{
		CItemType *pType = g_pUniverse->GetItemType(i);
		CDeviceClass *pWeapon;

		if (pType->IsDevice() 
				&& (pWeapon = pType->GetDeviceClass()))
			{
			int iVariant = pWeapon->GetAmmoVariant(pItem);
			if (iVariant != -1)
				return pWeapon->FindDataField(iVariant, sField, retsValue);
			}
		}

	return false;
	}

CString CDeviceClass::GetEnhancedDesc (const CItem &Item, CSpaceObject *pInstalled)

//	GetEnhancedDesc
//
//	Return enhanced description string

	{
	if (Item.GetMods())
		{
		CItemEnhancement Mods(Item.GetMods());
		return Mods.GetEnhancedDesc(Item, pInstalled);
		}
	else if (Item.IsEnhanced())
		return CONSTLIT("+Enhanced");
	else
		return NULL_STR;
	}

CString CDeviceClass::GetReference (const CItem *pItem, CSpaceObject *pInstalled)

//	GetReference
//
//	Returns reference string

	{
	return GetReferencePower(pItem);
	}

CString CDeviceClass::GetReferencePower (const CItem *pItem)

//	GetReferencePower
//
//	Returns a string for the reference relating to the power required for
//	this device.

	{
	int iPower = GetPowerRating(pItem);
	int iMW = iPower / 10;
	int iMWDecimal = iPower % 10;

	if (iPower == 0)
		return NULL_STR;
	else if (iMW >= 100 || iMWDecimal == 0)
		return strPatternSubst(CONSTLIT("%dMW"), iMW);
	else
		return strPatternSubst(CONSTLIT("%d.%dMW"), iMW, iMWDecimal);
	}

//	CInstalledDevice class

CInstalledDevice::CInstalledDevice (void) : 
		m_fOmniDirectional(false),
		m_fOffset(false),
		m_fSecondaryWeapon(false),
		m_iPosAngle(0),
		m_iPosRadius(0),
		m_iMinFireArc(0),
		m_iMaxFireArc(0),
		m_iActivationDelay(0),
		m_iBonus(0),
		m_iTemperature(0),
		m_iDeviceSlot(-1)
	{
	}

int CInstalledDevice::CalcPowerUsed (CSpaceObject *pSource)

//	CalcPowerUsed
//
//	Calculates how much power this device used this turn

	{
	if (!IsEmpty()) 
		return m_pClass->CalcPowerUsed(this, pSource);
	else
		return 0;
	}

CVector CInstalledDevice::GetPos (CSpaceObject *pSource)

//	GetPos
//
//	Returns the position of the device

	{
	if (m_iPosRadius)
		{
		return pSource->GetPos() 
				+ PolarToVector((m_iPosAngle + pSource->GetRotation()) % 360,
					m_iPosRadius * g_KlicksPerPixel);
		}
	else
		return pSource->GetWeaponPos();
	}

void CInstalledDevice::InitFromDesc (const SDeviceDesc &Desc)

//	InitFromDesc
//
//	Initializes from a desc

	{
	m_fOmniDirectional = Desc.bOmnidirectional;
	m_fSecondaryWeapon = Desc.bSecondary;

	m_iPosAngle = Desc.iPosAngle;
	m_iPosRadius = Desc.iPosRadius;
	m_iMinFireArc = Desc.iMinFireArc;
	m_iMaxFireArc = Desc.iMaxFireArc;
	}

void CInstalledDevice::InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	InitFromXML
//
//	Initializes each device slot from an XML structure

	{
	m_fOmniDirectional = pDesc->GetAttributeBool(OMNIDIRECTIONAL_ATTRIB);
	m_fSecondaryWeapon = pDesc->GetAttributeBool(SECONDARY_WEAPON_ATTRIB);

	m_iPosAngle = pDesc->GetAttributeInteger(POS_ANGLE_ATTRIB);
	m_iPosRadius = pDesc->GetAttributeInteger(POS_RADIUS_ATTRIB);
	m_iMinFireArc = pDesc->GetAttributeInteger(MIN_FIRE_ARC_ATTRIB);
	m_iMaxFireArc = pDesc->GetAttributeInteger(MAX_FIRE_ARC_ATTRIB);

	m_pClass.LoadUNID(Ctx, pDesc->GetAttribute(DEVICE_ID_ATTRIB));
	}

void CInstalledDevice::Install (CSpaceObject *pObj, CItemListManipulator &ItemList, int iDeviceSlot)

//	Install
//
//	Installs a new device of the given class

	{
	const CItem &Item = ItemList.GetItemAtCursor();

	m_pClass.Set(Item.GetType()->GetDeviceClass());
	m_iDeviceSlot = iDeviceSlot;
	m_dwData = 0;
	m_iTemperature = 0;
	m_fEnhanced = Item.IsEnhanced();
	m_fDamaged = Item.IsDamaged();
	m_fExternal = m_pClass->IsExternal();
	m_fWaiting = false;
	m_fEnabled = true;

	//	Transfer the mods

	m_Mods = Item.GetMods();

	//	Initialize charges cache

	m_iCharges = Item.GetCharges();

	//	Call the class

	m_pClass->OnInstall(this, pObj, ItemList);
	m_pClass->SelectFirstVariant(pObj, this);

	//	Mark the item as installed

	ItemList.SetInstalledAtCursor(iDeviceSlot);
	}

ALERROR CInstalledDevice::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Done loading all design elements

	{
	ALERROR error;

	if (error = m_pClass.Bind(Ctx))
		return error;

	return NOERROR;
	}

void CInstalledDevice::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Read object from stream
//
//	DWORD		device: class UNID (0xffffffff if not installed)
//	DWORD		device: m_dwData
//	DWORD		device: low = m_iPosAngle; hi = m_iPosRadius
//	DWORD		device: low = m_iMinFireArc; hi = m_iMaxFireArc
//	DWORD		device: low = m_iActivationDelay; hi = m_iFireAngle
//	DWORD		device: low = m_iBonus; hi = m_iTemperature
//	DWORD		device: low = m_iCharges; hi = m_iDeviceSlot
//	DWORD		device: mods
//	DWORD		device: flags

	{
	DWORD dwLoad;
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad != 0xffffffff)
		{
		m_pClass.Set(g_pUniverse->FindDeviceClass(dwLoad));
		Ctx.pStream->Read((char *)&m_dwData, sizeof(DWORD));
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_iPosAngle = (int)LOWORD(dwLoad);
		m_iPosRadius = (int)HIWORD(dwLoad);
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_iMinFireArc = (int)LOWORD(dwLoad);
		m_iMaxFireArc = (int)HIWORD(dwLoad);
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_iActivationDelay = (int)LOWORD(dwLoad);
		m_iFireAngle = (int)HIWORD(dwLoad);
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_iBonus = (int)LOWORD(dwLoad);
		m_iTemperature = (int)HIWORD(dwLoad);
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_iCharges = (int)LOWORD(dwLoad);
		if (Ctx.dwVersion >= 29)
			m_iDeviceSlot = (int)HIWORD(dwLoad);

		Ctx.pStream->Read((char *)&m_Mods, sizeof(m_Mods));

		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_fOmniDirectional =	((dwLoad & 0x00000001) ? true : false);
		m_fOffset =				((dwLoad & 0x00000002) ? true : false);
		m_fEnhanced =			((dwLoad & 0x00000004) ? true : false);
		m_fOverdrive =			((dwLoad & 0x00000008) ? true : false);
		m_fOptimized =			((dwLoad & 0x00000010) ? true : false);
		m_fSecondaryWeapon =	((dwLoad & 0x00000020) ? true : false);
		m_fDamaged =			((dwLoad & 0x00000040) ? true : false);
		m_fEnabled =			((dwLoad & 0x00000080) ? true : false);
		m_fWaiting =			((dwLoad & 0x00000100) ? true : false);

		m_fExternal = m_pClass->IsExternal();
		}
	}

int CInstalledDevice::IncCharges (CSpaceObject *pSource, int iChange)

//	IncCharges
//
//	Increments charges

	{
	CShip *pShip = pSource->AsShip();
	if (pShip == NULL)
		return -1;

	CItemListManipulator ItemList(pSource->GetItemList());
	pShip->SetCursorAtDevice(ItemList, m_iDeviceSlot);
	pShip->RechargeItem(ItemList, iChange);

	return ItemList.GetItemAtCursor().GetCharges();
	}

void CInstalledDevice::Uninstall (CSpaceObject *pObj, CItemListManipulator &ItemList)

//	Uninstall
//
//	Uninstalls the device

	{
	//	Mark the item as uninstalled

	ItemList.SetInstalledAtCursor(-1);

	//	Let the class clean up also

	m_pClass->OnUninstall(this, pObj, ItemList);

	//	Done

	m_pClass.Set(NULL);
	}

void CInstalledDevice::Update (CSpaceObject *pSource, 
							   int iTick,
							   bool *retbSourceDestroyed,
							   bool *retbConsumedItems)
	{ 
	if (!IsEmpty()) 
		{
		m_pClass->Update(this, pSource, iTick, retbSourceDestroyed, retbConsumedItems);

		if (m_iActivationDelay > 0)
			m_iActivationDelay--;
		}
	}

void CInstalledDevice::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Write object to stream
//
//	DWORD		device: class UNID (0xffffffff if not installed)
//	DWORD		device: m_dwData
//	DWORD		device: low = m_iPosAngle; hi = m_iPosRadius
//	DWORD		device: low = m_iMinFireArc; hi = m_iMaxFireArc
//	DWORD		device: low = m_iActivationDelay; hi = m_iFireAngle
//	DWORD		device: low = m_iBonus; hi = m_iTemperature
//	DWORD		device: low = m_iCharges; hi = m_iDeviceSlot
//	DWORD		device: mods
//	DWORD		device: flags

	{
	DWORD dwSave;

	if (m_pClass)
		dwSave = m_pClass->GetUNID();
	else
		dwSave = 0xffffffff;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	if (m_pClass)
		{
		pStream->Write((char *)&m_dwData, sizeof(DWORD));
		dwSave = MAKELONG(m_iPosAngle, m_iPosRadius);
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		dwSave = MAKELONG(m_iMinFireArc, m_iMaxFireArc);
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		dwSave = MAKELONG(m_iActivationDelay, m_iFireAngle);
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		dwSave = MAKELONG(m_iBonus, m_iTemperature);
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		dwSave = MAKELONG(m_iCharges, m_iDeviceSlot);
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		pStream->Write((char *)&m_Mods, sizeof(m_Mods));

		dwSave = 0;
		dwSave |= (m_fOmniDirectional ?		0x00000001 : 0);
		dwSave |= (m_fOffset ?				0x00000002 : 0);
		dwSave |= (m_fEnhanced ?			0x00000004 : 0);
		dwSave |= (m_fOverdrive ?			0x00000008 : 0);
		dwSave |= (m_fOptimized ?			0x00000010 : 0);
		dwSave |= (m_fSecondaryWeapon ?		0x00000020 : 0);
		dwSave |= (m_fDamaged ?				0x00000040 : 0);
		dwSave |= (m_fEnabled ?				0x00000080 : 0);
		dwSave |= (m_fWaiting ?				0x00000100 : 0);
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		}
	}

