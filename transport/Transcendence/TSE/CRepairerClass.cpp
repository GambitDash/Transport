//	CRepairerClass.cpp
//
//	CRepairerClass class

#include "PreComp.h"


#define POWER_USE_ATTRIB			CONSTLIT("powerUse")
#define REPAIR_CYCLE_ATTRIB			CONSTLIT("repairCycle")

#define REPAIR_CYCLE_TIME			10

CRepairerClass::CRepairerClass (void) : CDeviceClass(NULL)
	{
	}

int CRepairerClass::CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	CalcPowerUsed
//
//	Computes the amount of power used by this device each tick

	{
	int i;

	//	Doesn't work if not enabled

	if (!pDevice->IsEnabled())
		return 0;

	int iPower = 0;

	//	Get a ship object and calculate based on armor

	CShip *pShip = pSource->AsShip();
	if (pShip)
		{
		for (i = 0; i < pShip->GetArmorSectionCount(); i++)
			{
			if (pShip->IsArmorDamaged(i))
				{
				//	Figure out the tech of this armor

				CArmorClass *pArmorClass = pShip->GetArmorSection(i)->pArmorClass;
				int iArmorTech = pArmorClass->GetRepairTech();

				//	Figure out the cycle time for repairing this tech

				int iRepairCycle = 0;
				if (iArmorTech <= m_RepairCycle.GetCount())
					iRepairCycle = m_RepairCycle.GetElement(iArmorTech-1);

				//	Repair?

				if (iRepairCycle > 0)
					iPower += m_iPowerUse;
				}
			}
		}

	return iPower;
	}

ALERROR CRepairerClass::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice)

//	CreateFromXML
//
//	Creates from an XML element

	{
	ALERROR error;
	int i;
	CRepairerClass *pDevice;

	pDevice = new CRepairerClass;
	if (pDevice == NULL)
		return ERR_MEMORY;

	pDevice->InitDeviceFromXML(pDesc, pType);

	pDevice->m_iPowerUse = pDesc->GetAttributeInteger(POWER_USE_ATTRIB);
	if (error = pDesc->GetAttributeIntegerList(REPAIR_CYCLE_ATTRIB, &pDevice->m_RepairCycle))
		return error;

	//	Adjust because we actually store time in 10-ticks.

	for (i = 0; i < pDevice->m_RepairCycle.GetCount(); i++)
		pDevice->m_RepairCycle.ReplaceElement(i, pDevice->m_RepairCycle.GetElement(i) / REPAIR_CYCLE_TIME);

	//	Done

	*retpDevice = pDevice;

	return NOERROR;
	}

void CRepairerClass::Update (CInstalledDevice *pDevice, CSpaceObject *pSource, int iTick, bool *retbSourceDestroyed, bool *retbConsumedItems)

//	Update
//
//	Updates the device

	{
	int i;

	CShip *pShip = pSource->AsShip();
	if (pShip)
		{
		if ((iTick % REPAIR_CYCLE_TIME) == 0
				&& pDevice->IsEnabled() && !pDevice->IsDamaged())
			{
			int iCycle = iTick / REPAIR_CYCLE_TIME;

			for (i = 0; i < pShip->GetArmorSectionCount(); i++)
				{
				if (pShip->IsArmorDamaged(i))
					{
					//	Figure out the tech of this armor

					CArmorClass *pArmorClass = pShip->GetArmorSection(i)->pArmorClass;
					int iArmorTech = pArmorClass->GetRepairTech();

					//	Figure out the cycle time for repairing this tech

					int iRepairCycle = 0;
					if (iArmorTech <= m_RepairCycle.GetCount())
						iRepairCycle = m_RepairCycle.GetElement(iArmorTech-1);

					//	Time to repair?

					if (iRepairCycle > 0 && (iCycle % iRepairCycle) == 0)
						{
						//	Repair one point of armor

						pShip->RepairArmor(i, 1);
						}
					}
				}
			}
		}

	if (retbConsumedItems)
		*retbConsumedItems = false;
	}

