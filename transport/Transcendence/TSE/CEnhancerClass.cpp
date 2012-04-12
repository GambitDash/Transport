//	CEnhancerClass.cpp
//
//	CEnhancerClass class

#include "PreComp.h"


#define DAMAGE_ADJ_ATTRIB						CONSTLIT("damageAdj")
#define ENHANCEMENT_TYPE_ATTRIB					CONSTLIT("enhancementType")
#define POWER_USE_ATTRIB						CONSTLIT("powerUse")

CEnhancerClass::CEnhancerClass (void) : CDeviceClass(NULL)

//	CEnhancerClass constructor

	{
	}

int CEnhancerClass::CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	CalcPowerUsed
//
//	Returns the amount of power used per tick

	{
	if (!pDevice->IsEnabled())
		return 0;

	return m_iPowerUse;
	}

ALERROR CEnhancerClass::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice)

//	CreateFromXML
//
//	Load device data from XML

	{
	CEnhancerClass *pDevice;

	pDevice = new CEnhancerClass;
	if (pDevice == NULL)
		return ERR_MEMORY;

	pDevice->InitDeviceFromXML(pDesc, pType);

	LoadDamageAdjFromXML(pDesc->GetAttribute(DAMAGE_ADJ_ATTRIB), pDevice->m_iDamageAdj);
	pDevice->m_iPowerUse = pDesc->GetAttributeInteger(POWER_USE_ATTRIB);
	pDevice->m_sEnhancementType = pDesc->GetAttribute(ENHANCEMENT_TYPE_ATTRIB);

	//	Done

	*retpDevice = pDevice;

	return NOERROR;
	}

int CEnhancerClass::GetWeaponBonus (CInstalledDevice *pDevice, CSpaceObject *pSource, CInstalledDevice *pWeapon, CString *retsBonusType)

//	GetWeaponBonus
//
//	Returns the bonus that we add to weapons

	{
	if (!pDevice->IsEnabled() || pDevice->IsDamaged())
		return 0;

	if (retsBonusType)
		*retsBonusType = m_sEnhancementType;

	int iType = pWeapon->GetDamageType();
	if (iType != -1)
		return m_iDamageAdj[iType];
	else
		return 0;
	}
