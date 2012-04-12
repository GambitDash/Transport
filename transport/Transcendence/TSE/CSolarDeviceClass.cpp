//	CSolarDeviceClass.cpp
//
//	CSolarDeviceClass class

#include "PreComp.h"


#define REFUEL_ATTRIB								CONSTLIT("refuel")

#define CYCLE_TIME									10

CSolarDeviceClass::CSolarDeviceClass (void) : CDeviceClass(NULL)

//	CSolarDeviceClass constructor

	{
	}

ALERROR CSolarDeviceClass::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice)

//	CreateFromXML
//
//	Load device data from XML

	{
	CSolarDeviceClass *pDevice;

	pDevice = new CSolarDeviceClass;
	if (pDevice == NULL)
		return ERR_MEMORY;

	pDevice->InitDeviceFromXML(pDesc, pType);

	pDevice->m_iRefuel = pDesc->GetAttributeInteger(REFUEL_ATTRIB);

	//	Done

	*retpDevice = pDevice;

	return NOERROR;
	}

void CSolarDeviceClass::Update (CInstalledDevice *pDevice, 
								CSpaceObject *pSource, 
								int iTick,
								bool *retbSourceDestroyed,
								bool *retbConsumedItems)

//	Update
//
//	Update device

	{
	if (((iTick + 7) % CYCLE_TIME) == 0)
		{
		int iIntensity = pSource->GetSystem()->CalculateLightIntensity(pSource->GetPos());
		if (pDevice->IsDamaged())
			iIntensity = iIntensity / 2;
		pSource->Refuel(iIntensity * m_iRefuel / 100);
		}
	}
