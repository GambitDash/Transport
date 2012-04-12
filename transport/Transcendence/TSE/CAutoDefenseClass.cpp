//	CAutoDefenseClass.cpp
//
//	CAutoDefenseClass class
//	Copyright (c) 2004 by George Moromisato. All Rights Reserved.

#include "PreComp.h"


#define RECHARGE_TIME_ATTRIB					CONSTLIT("rechargeTime")
#define TARGET_ATTRIB							CONSTLIT("target")
#define WEAPON_ATTRIB							CONSTLIT("weapon")

#define MISSILES_TARGET							CONSTLIT("missiles")

#define MAX_INTERCEPT_DISTANCE					(8.0 * LIGHT_SECOND)

CAutoDefenseClass::CAutoDefenseClass (void) : CDeviceClass(NULL)

//	CAutoDefenseClass constructor

	{
	}

int CAutoDefenseClass::CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	CalcPowerUsed
//
//	Return power used by device

	{
	CDeviceClass *pWeapon = GetWeapon();

	if (pWeapon == NULL || !pDevice->IsEnabled())
		return 0;

	return pWeapon->CalcPowerUsed(pDevice, pSource);
	}

int CAutoDefenseClass::GetDamageType (CInstalledDevice *pDevice)

//	GetDamageType
//
//	Returns the type of damage done by this device

	{
	CDeviceClass *pWeapon = GetWeapon();

	if (pWeapon)
		return pWeapon->GetDamageType(pDevice);
	else
		return -1;
	}

int CAutoDefenseClass::GetPowerRating (const CItem *pItem)

//	GetPowerRating
//
//	Returns the minimum reactor power needed for this device

	{
	CDeviceClass *pWeapon = GetWeapon();

	if (pWeapon)
		return pWeapon->GetPowerRating(pItem);
	else
		return 0;
	}

void CAutoDefenseClass::Update (CInstalledDevice *pDevice, 
								CSpaceObject *pSource, 
								int iTick,
								bool *retbSourceDestroyed,
								bool *retbConsumedItems)

//	Update
//
//	Update device

	{
	if (pDevice->IsReady() && pDevice->IsEnabled())
		{
		int i;

		//	Look for a target

		CSpaceObject *pBestTarget = NULL;
		Metric rBestDist2 = MAX_INTERCEPT_DISTANCE * MAX_INTERCEPT_DISTANCE;

		for (i = 0; i < pSource->GetSystem()->GetObjectCount(); i++)
			{
			CSpaceObject *pObj = pSource->GetSystem()->GetObject(i);

			if (pObj
					&& pObj->GetCategory() == CSpaceObject::catMissile
					&& pObj->GetSource() != pSource
					&& (pObj->GetSource() == NULL || pSource->IsEnemy(pObj->GetSource())))
				{
				CVector vRange = pObj->GetPos() - pSource->GetPos();
				Metric rDistance2 = vRange.Dot(vRange);

				if (rDistance2 < rBestDist2)
					{
					pBestTarget = pObj;
					rBestDist2 = rDistance2;
					}
				}
			}

		//	If we found a target, try to shoot at it

		if (pBestTarget)
			{
			CDeviceClass *pWeapon = GetWeapon();

			if (pWeapon)
				{
				int iFireAngle;
				if (pWeapon->IsWeaponAligned(pSource, pDevice, pBestTarget, &iFireAngle))
					{
					pDevice->SetFireAngle(iFireAngle);
					pWeapon->Activate(pDevice, pSource, pBestTarget, iFireAngle, retbSourceDestroyed, retbConsumedItems);
					pDevice->SetActivationDelay(m_iRechargeTicks);
					}
				}
			}
		}
	}

ALERROR CAutoDefenseClass::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice)

//	CreateFromXML
//
//	Load the class

	{
	CAutoDefenseClass *pDevice;

	pDevice = new CAutoDefenseClass;
	if (pDevice == NULL)
		return ERR_MEMORY;

	pDevice->InitDeviceFromXML(pDesc, pType);

	pDevice->m_iRechargeTicks = (int)((pDesc->GetAttributeInteger(RECHARGE_TIME_ATTRIB) / STD_SECONDS_PER_UPDATE) + 0.5);
	pDevice->m_pWeapon.LoadUNID(Ctx, pDesc->GetAttribute(WEAPON_ATTRIB));

	//	Target

	CString sTarget = pDesc->GetAttribute(TARGET_ATTRIB);
	if (strEquals(sTarget, MISSILES_TARGET))
		pDevice->m_iTarget = trgMissiles;
	else
		return ERR_FAIL;

	//	Done

	*retpDevice = pDevice;

	return NOERROR;
	}

ALERROR CAutoDefenseClass::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	All design elements loaded

	{
	ALERROR error;

	if ((error = m_pWeapon.Bind(Ctx)))
		return error;

	return NOERROR;
	}
