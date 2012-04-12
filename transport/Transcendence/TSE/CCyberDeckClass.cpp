//	CCyberDeckClass.cpp
//
//	CCyberDeckClass class
//	Copyright (c) 2004 by George Moromisato. All Rights Reserved.

#include "PreComp.h"


#define RANGE_ATTRIB							CONSTLIT("range")
#define ATTACK_CHANCE_ATTRIB					CONSTLIT("attackChance")
#define AI_LEVEL_ATTRIB							CONSTLIT("aiLevel")
#define PROGRAM_ATTRIB							CONSTLIT("program")
#define PROGRAM_NAME_ATTRIB						CONSTLIT("programName")

#define SHIELDS_DOWN_PROGRAM					CONSTLIT("ShieldsDown")
#define REBOOT_PROGRAM							CONSTLIT("Reboot")
#define DISARM_PROGRAM							CONSTLIT("Disarm")

CCyberDeckClass::CCyberDeckClass (void) : CDeviceClass(NULL)

//	CCyberDeckClass constructor

	{
	}

void CCyberDeckClass::Activate (CInstalledDevice *pDevice, 
							    CSpaceObject *pSource, 
							    CSpaceObject *pTarget,
								int iFireAngle,
								bool *retbSourceDestroyed,
							    bool *retbConsumedItems)

//	Activate
//
//	Activate device

	{
	//	Init

	if (retbConsumedItems)
		*retbConsumedItems = false;

	//	Won't work if not enabled

	if (!pDevice->IsEnabled() || pDevice->IsDamaged())
		return;

	//	We better have a target

	if (pTarget == NULL)
		return;

	//	The attack has a random chance of succeeding. If it did not
	//	succeed, we're done.

	if (mathRandom(1, 100) > m_iAttackChance)
		{
		//	There is a chance that the target gets to find out about the
		//	attempted attack.


		return;
		}

	//	See if the attack is blocked by defenses

	if (m_Program.iAILevel < pTarget->GetCyberDefenseLevel())
		{
		//	There is a chance that the attacker will need to reboot

		return;
		}

	//	Run the program

	pTarget->ProgramDamage(pSource, m_Program);
	}

ALERROR CCyberDeckClass::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpDevice)

//	CreateFromXML
//
//	Load the class

	{
	CCyberDeckClass *pDevice;

	pDevice = new CCyberDeckClass;
	if (pDevice == NULL)
		return ERR_MEMORY;

	pDevice->InitDeviceFromXML(pDesc, pType);

	pDevice->m_iRange = pDesc->GetAttributeInteger(RANGE_ATTRIB);
	pDevice->m_iAttackChance = pDesc->GetAttributeInteger(ATTACK_CHANCE_ATTRIB);

	//	Program

	CString sProgram = pDesc->GetAttribute(PROGRAM_ATTRIB);
	if (strEquals(sProgram, SHIELDS_DOWN_PROGRAM))
		pDevice->m_Program.iProgram = progShieldsDown;
	else if (strEquals(sProgram, REBOOT_PROGRAM))
		pDevice->m_Program.iProgram = progReboot;
	else if (strEquals(sProgram, DISARM_PROGRAM))
		pDevice->m_Program.iProgram = progDisarm;
	else
		return ERR_FAIL;

	pDevice->m_Program.sProgramName = pDesc->GetAttribute(PROGRAM_NAME_ATTRIB);
	pDevice->m_Program.iAILevel = pDesc->GetAttributeInteger(AI_LEVEL_ATTRIB);

	//	Done

	*retpDevice = pDevice;

	return NOERROR;
	}

Metric CCyberDeckClass::GetMaxEffectiveRange (CSpaceObject *pSource, CInstalledDevice *pDevice, CSpaceObject *pTarget)

//	GetMaxEffectiveRange
//
//	Returns the maximum effective range of the weapon

	{
	return m_iRange * LIGHT_SECOND;
	}

CString CCyberDeckClass::GetReference (const CItem *pItem, CSpaceObject *pInstalled)

//	GetReference
//
//	Returns reference information

	{
	return strPatternSubst("%s (%d); %s", 
			m_Program.sProgramName.GetASCIIZPointer(), 
			m_Program.iAILevel,
			GetReferencePower(pItem).GetASCIIZPointer());
	}

void CCyberDeckClass::GetSelectedVariantInfo (CSpaceObject *pSource, 
											  CInstalledDevice *pDevice,
											  CString *retsLabel,
											  int *retiAmmoLeft,
											  CItemType **retpType)

//	GetSelectedVariantInfo
//
//	Returns weapons info

	{
	if (retsLabel)
		*retsLabel = CString();

	if (retiAmmoLeft)
		*retiAmmoLeft = -1;

	if (retpType)
		*retpType = GetItemType();
	}

int CCyberDeckClass::GetWeaponEffectiveness (CSpaceObject *pSource, CInstalledDevice *pDevice, CSpaceObject *pTarget)

//	GetWeaponEffectiveness
//
//	Returns:
//
//	< 0		If the weapon is ineffective against the target
//	0		If the weapon has normal effect on target
//	1-100	If the weapon is particularly effective against the target
//
//	This call is used to figure out whether we should use an EMP or blinder
//	cannon against the target.

	{
	switch (m_Program.iProgram)
		{
		case progShieldsDown:
			{
			//	If the target's shields are down then this weapon is not
			//	very effective

			int iLevel = pTarget->GetShieldLevel();
			if (iLevel <= 0)
				return -100;
			else
				return iLevel;
			}

		case progDisarm:
			return ((pTarget->IsDisarmed() || pTarget->IsParalyzed()) ? -100 : 100);

		default:
			return 0;
		}
	}
