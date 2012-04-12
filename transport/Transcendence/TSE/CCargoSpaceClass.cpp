//	CCargoSpaceClass.cpp
//
//	CCargoSpaceClass class

#include "PreComp.h"


#define CARGO_SPACE_ATTRIB			CONSTLIT("cargoSpace")

CCargoSpaceClass::CCargoSpaceClass (void) : CDeviceClass(NULL)
	{
	}

ALERROR CCargoSpaceClass::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice)

//	CreateFromXML
//
//	Creates from an XML element

	{
	CCargoSpaceClass *pDevice;

	pDevice = new CCargoSpaceClass;
	if (pDevice == NULL)
		return ERR_MEMORY;

	pDevice->InitDeviceFromXML(pDesc, pType);
	pDevice->m_iCargoSpace = pDesc->GetAttributeInteger(CARGO_SPACE_ATTRIB);

	//	Done

	*retpDevice = pDevice;

	return NOERROR;
	}
