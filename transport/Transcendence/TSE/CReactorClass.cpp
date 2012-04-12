//	CReactorClass.cpp
//
//	CReactorClass class

#include "PreComp.h"


#define MAX_POWER_ATTRIB			CONSTLIT("maxPower")
#define MAX_FUEL_ATTRIB				CONSTLIT("maxFuel")
#define MIN_FUEL_TECH_ATTRIB		CONSTLIT("minFuelTech")
#define MAX_FUEL_TECH_ATTRIB		CONSTLIT("maxFuelTech")
#define FUEL_EFFICIENCY_ATTRIB		CONSTLIT("fuelEfficiency")

CReactorClass::CReactorClass (void) : CDeviceClass(NULL)
	{
	}

ALERROR CReactorClass::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice)

//	CreateFromXML
//
//	Creates from an XML element

	{
	CReactorClass *pDevice;

	pDevice = new CReactorClass;
	if (pDevice == NULL)
		return ERR_MEMORY;

	pDevice->InitDeviceFromXML(pDesc, pType);
	pDevice->m_Desc.iMaxPower = pDesc->GetAttributeInteger(MAX_POWER_ATTRIB);
	pDevice->m_Desc.iMaxFuel = pDesc->GetAttributeInteger(MAX_FUEL_ATTRIB);
	pDevice->m_Desc.iMinFuelLevel = pDesc->GetAttributeInteger(MIN_FUEL_TECH_ATTRIB);
	pDevice->m_Desc.iMaxFuelLevel = pDesc->GetAttributeInteger(MAX_FUEL_TECH_ATTRIB);
	pDevice->m_Desc.iPowerPerFuelUnit = pDesc->GetAttributeInteger(FUEL_EFFICIENCY_ATTRIB);
	if (pDevice->m_Desc.iPowerPerFuelUnit == 0)
		pDevice->m_Desc.iPowerPerFuelUnit = g_MWPerFuelUnit;
	pDevice->m_Desc.fDamaged = false;
	pDevice->m_Desc.fEnhanced = false;

	//	Compute descriptor when damaged

	pDevice->m_DamagedDesc.iMaxPower = 80 * pDevice->m_Desc.iMaxPower / 100;
	pDevice->m_DamagedDesc.iMaxFuel = pDevice->m_Desc.iMaxFuel;
	pDevice->m_DamagedDesc.iMinFuelLevel = pDevice->m_Desc.iMinFuelLevel;
	pDevice->m_DamagedDesc.iMaxFuelLevel = pDevice->m_Desc.iMaxFuelLevel;
	pDevice->m_DamagedDesc.iPowerPerFuelUnit = 80 * pDevice->m_Desc.iPowerPerFuelUnit / 100;
	pDevice->m_DamagedDesc.fDamaged = true;
	pDevice->m_DamagedDesc.fEnhanced = false;

	//	Compute descriptor when enhanced

	pDevice->m_EnhancedDesc.iMaxPower = 120 * pDevice->m_Desc.iMaxPower / 100;
	pDevice->m_EnhancedDesc.iMaxFuel = pDevice->m_Desc.iMaxFuel;
	pDevice->m_EnhancedDesc.iMinFuelLevel = pDevice->m_Desc.iMinFuelLevel;
	pDevice->m_EnhancedDesc.iMaxFuelLevel = pDevice->m_Desc.iMaxFuelLevel;
	pDevice->m_EnhancedDesc.iPowerPerFuelUnit = 150 * pDevice->m_Desc.iPowerPerFuelUnit / 100;
	pDevice->m_EnhancedDesc.fDamaged = false;
	pDevice->m_EnhancedDesc.fEnhanced = true;

	//	Done

	*retpDevice = pDevice;

	return NOERROR;
	}

const ReactorDesc *CReactorClass::GetReactorDesc (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	GetReactorDesc
//
//	Returns the reactor descriptor

	{
	if (pDevice == NULL)
		return &m_Desc;
	else if (pDevice->IsDamaged())
		return &m_DamagedDesc;
	else if (pDevice->IsEnhanced())
		return &m_EnhancedDesc;
	else
		return &m_Desc;
	}
