//	CArmorClass.cpp
//
//	CArmorClass class

#include "PreComp.h"
#include "ArmorClassConstants.h"

#define INSTALL_COST_ATTRIB						(CONSTLIT("installCost"))
#define PHOTO_REPAIR_ATTRIB						(CONSTLIT("photoRepair"))
#define REPAIR_RATE_ATTRIB						(CONSTLIT("repairRate"))
#define DECAY_RATE_ATTRIB						(CONSTLIT("decayRate"))
#define REPAIR_TECH_ATTRIB						(CONSTLIT("repairTech"))
#define UNID_ATTRIB								(CONSTLIT("unid"))
#define RADIATION_IMMUNE_ATTRIB					(CONSTLIT("radiationImmune"))
#define PHOTO_RECHARGE_ATTRIB					(CONSTLIT("photoRecharge"))
#define COMPLETE_BONUS_ATTRIB					(CONSTLIT("completeBonus"))
#define SHIELD_INTERFERENCE_ATTRIB				(CONSTLIT("shieldInterference"))
#define COMPOSITION_ATTRIB						(CONSTLIT("composition"))
#define DISINTEGRATION_IMMUNE_ATTRIB			(CONSTLIT("disintegrationImmune"))
#define EMP_DAMAGE_ADJ_ATTRIB					(CONSTLIT("EMPDamageAdj"))
#define BLINDING_DAMAGE_ADJ_ATTRIB				(CONSTLIT("blindingDamageAdj"))
#define DEVICE_DAMAGE_ADJ_ATTRIB				(CONSTLIT("deviceDamageAdj"))
#define STEALTH_ATTRIB							(CONSTLIT("stealth"))
#define POWER_USE_ATTRIB						(CONSTLIT("powerUse"))
#define REFLECT_ATTRIB							(CONSTLIT("reflect"))

#define FIELD_BALANCE							CONSTLIT("balance")
#define FIELD_HP								CONSTLIT("hp")
#define FIELD_HP_BONUS							CONSTLIT("hpBonus")

#define STR_SEGMENT_OF							(CONSTLIT("segment of"))

static CObjectClass<CArmorClass>g_Class(OBJID_CARMORCLASS, NULL);

static char g_HitPointsAttrib[] = "hitPoints";
static char g_RepairCostAttrib[] = "repairCost";
/*
static char g_DamageAdjAttrib[] = "damageAdj";
static char g_ItemIDAttrib[] = "itemID";
*/
#define MAX_REFLECTION_CHANCE		95

#define MAX_REFLECTION_CHANCE		95

CArmorClass::CArmorClass (void) : CObject(&g_Class),
		m_pItemType(NULL)

//	CArmorClass constructor

	{
	}

void CArmorClass::AddArmorComplete (CInstalledArmor *pArmor)

//	AddArmorComplete
//
//	Adjusts hit points on installed armor depending on whether or not the
//	armor is complete (i.e., all armor segments are of the same class)

	{
	//	If we're armor complete, then add hit points.

	if (pArmor->m_fComplete)
		pArmor->iHitPoints += m_iArmorCompleteBonus;

	//	Make sure the armor doesn't have more HP than it's supposed to.
	//	This can happen if we lose the armor complete flag

	else
		pArmor->iHitPoints = std::min(pArmor->iHitPoints, GetHitPoints(pArmor));
	}

int CArmorClass::CalcAdjustedDamage (CInstalledArmor *pArmor, const DamageDesc &Damage, int iDamage) 

//	CalcAdjustedDamage
//
//	Adjust damage for damage type and armor

	{
	if (pArmor)
		return (GetDamageAdj(pArmor->m_Mods, Damage) * iDamage + 50) / 100;
	else
		return (GetDamageAdj(CItemEnhancement(), Damage) * iDamage + 50) / 100;
	}

int CArmorClass::CalcBalance (void)

//	CalcBalance
//
//	Determines whether the given item is balanced for its level. Negative numbers
//	mean the item is underpowered. Positive numbers mean the item is overpowered.

	{
	int i;
	int iBalance = 0;
	int iLevel = m_pItemType->GetLevel();

	//	Regeneration

	if (m_iSelfRepairCycle > 0)
		{
		if (m_fPhotoRepair)
			iBalance += (240 / m_iSelfRepairCycle);
		else
			iBalance += 4 * 240 / m_iSelfRepairCycle;
		}

	//	Stealth

	if (m_iStealth >= 12)
		iBalance += 4;
	else if (m_iStealth >= 10)
		iBalance += 3;
	else if (m_iStealth >= 8)
		iBalance += 2;
	else if (m_iStealth >= 6)
		iBalance += 1;

	//	Immunities

	if (m_fDisintegrationImmune)
		{
		if (iLevel <= 10)
			iBalance += 3;
		}

	if (m_iBlindingDamageAdj <= 20)
		iBalance += 1;
	if (m_iEMPDamageAdj <= 20)
		iBalance += 2;
	if (m_iDeviceDamageAdj <= 20)
		iBalance += 2;

	if (m_iBlindingDamageAdj > 20 || m_iEMPDamageAdj > 20 || m_iDeviceDamageAdj > 20)
		{
		if (m_iBlindingDamageAdj <= 33 || m_iEMPDamageAdj <= 33 || m_iDeviceDamageAdj <= 33)
			iBalance += 2;
		else if (m_iBlindingDamageAdj <= 50 || m_iEMPDamageAdj <= 50 || m_iDeviceDamageAdj <= 50)
			iBalance += 1;
		}

	if (m_fRadiationImmune)
		{
		if (iLevel <= 5)
			iBalance += 2;
		else if (iLevel <= 10)
			iBalance += 1;
		}

	if (m_fPhotoRecharge)
		iBalance += 2;

	//	Matched sets

	if (m_iArmorCompleteBonus)
		{
		int iPercent = m_iArmorCompleteBonus * 100 / m_iHitPoints;
		iBalance += (iPercent + 5) / 10;
		}

	//	Damage Adjustment

	for (i = 0; i < damageCount; i++)
		{
		int iStdAdj = STD_ARMOR_DAMAGE_ADJ[iLevel - 1][i];
		int iBonus = 0;
		if (m_iDamageAdj[i] > 0)
			iBonus = 100 * (iStdAdj - m_iDamageAdj[i]) / m_iDamageAdj[i];
		else if (iStdAdj > 0)
			iBonus = 100000;

		if (iBonus > 200)
			iBalance += 8;
		else if (iBonus > 0)
			iBalance += (iBonus + 12) / 25;
		else if (iBonus < 0)
			iBalance += (iBonus - 12) / 25;
		}

	//	Reflection

	for (i = 0; i < damageCount; i++)
		{
		if (m_Reflective.InSet((DamageTypes)i))
			iBalance += 8;
		}

	//	Hit Points

	if (m_iHitPoints > 0)
		{
		int iPercent = (m_iHitPoints - STD_ARMOR_HP[iLevel - 1]) * 100 / STD_ARMOR_HP[iLevel - 1];
		if (iPercent > 0)
			iBalance += (iPercent + 2) / 5;
		else if (iPercent < 0)
			iBalance += (iPercent - 2) / 5;
		}

	//	Mass

	int iMass = m_pItemType->GetMassKg();
	if (iMass > 0)
		{
		int iPercent = (iMass - STD_ARMOR_MASS[iLevel - 1]) * 100 / STD_ARMOR_MASS[iLevel - 1];
		if (iPercent > 0)
			iBalance -= (iPercent + 5) / 10;
		else if (iPercent < 0)
			iBalance -= (iPercent - 5) / 10;
		}

	//	Repair tech

	int iRepair = iLevel - m_iRepairTech;
	if (iRepair < 0)
		iBalance -= 2 * iRepair;
	else if (iRepair > 0)
		iBalance += iRepair;

	//	Power consumption

	if (m_iPowerUse)
		{
		int iPercent = m_iPowerUse * 100 / CShieldClass::GetStdPowerByLevel(iLevel);
		iBalance -= iPercent;
		}

	//	Meteorsteel

	if (m_fShieldInterference)
		iBalance -= 12;

	//	Decay

	if (m_iDecayCycle > 0)
		{
		iBalance -= 8 * 240 / m_iDecayCycle;
		}

	return iBalance;
	}

int CArmorClass::CalcPowerUsed (CInstalledArmor *pArmor)

//	CalcPowerUsed
//
//	Computes the amount of power used by this armor segment (this
//	only applies to powered armor)

	{
	return m_iPowerUse;
	}

ALERROR CArmorClass::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CArmorClass **retpArmor)

//	CreateFromXML
//
//	Create armor class desc

	{
	ALERROR error;
	CArmorClass *pArmor;
	CString sValue;

	pArmor = new CArmorClass;
	if (pArmor == NULL)
		return ERR_MEMORY;

	pArmor->m_pItemType = pType;
	pArmor->m_iHitPoints = pDesc->GetAttributeIntegerBounded(CONSTLIT(g_HitPointsAttrib), 0);
	pArmor->m_iRepairCost = pDesc->GetAttributeIntegerBounded(CONSTLIT(g_RepairCostAttrib), 0);
	pArmor->m_iInstallCost = pDesc->GetAttributeIntegerBounded(INSTALL_COST_ATTRIB, 0);
	pArmor->m_iSelfRepairCycle = pDesc->GetAttributeIntegerBounded(REPAIR_RATE_ATTRIB, 0);
	pArmor->m_iDecayCycle = pDesc->GetAttributeIntegerBounded(DECAY_RATE_ATTRIB, 0);
	pArmor->m_iRepairTech = pDesc->GetAttributeIntegerBounded(REPAIR_TECH_ATTRIB, 1, MAX_TECH_LEVEL);
	pArmor->m_iArmorCompleteBonus = pDesc->GetAttributeIntegerBounded(COMPLETE_BONUS_ATTRIB, 0);

	//	Load the new damage adjustment structure

	if ((error = LoadDamageAdj(pDesc, STD_ARMOR_DAMAGE_ADJ[pType->GetLevel() - 1], pArmor->m_iDamageAdj)))
		return error;

	//	Load other adjustments

	pArmor->m_iEMPDamageAdj = pDesc->GetAttributeIntegerBounded(EMP_DAMAGE_ADJ_ATTRIB, 0, -1, 100);
	pArmor->m_iBlindingDamageAdj = pDesc->GetAttributeIntegerBounded(BLINDING_DAMAGE_ADJ_ATTRIB, 0, -1, 100);
	pArmor->m_iDeviceDamageAdj = pDesc->GetAttributeIntegerBounded(DEVICE_DAMAGE_ADJ_ATTRIB, 0, -1, 100);

	pArmor->m_fPhotoRepair = pDesc->GetAttributeBool(PHOTO_REPAIR_ATTRIB);
	pArmor->m_fRadiationImmune = pDesc->GetAttributeBool(RADIATION_IMMUNE_ATTRIB);
	pArmor->m_fDisintegrationImmune = pDesc->GetAttributeBool(DISINTEGRATION_IMMUNE_ATTRIB);
	pArmor->m_fPhotoRecharge = pDesc->GetAttributeBool(PHOTO_RECHARGE_ATTRIB);
	pArmor->m_fShieldInterference = pDesc->GetAttributeBool(SHIELD_INTERFERENCE_ATTRIB);

	pArmor->m_iStealth = pDesc->GetAttributeInteger(STEALTH_ATTRIB);
	if (pArmor->m_iStealth == 0)
		pArmor->m_iStealth = CSpaceObject::stealthNormal;

	pArmor->m_iPowerUse = pDesc->GetAttributeIntegerBounded(POWER_USE_ATTRIB, 0);

	//	Load reflection

	if ((error = pArmor->m_Reflective.InitFromXML(pDesc->GetAttribute(REFLECT_ATTRIB))))
		return error;

	//	Done

	*retpArmor = pArmor;

	return NOERROR;
	}

bool CArmorClass::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns meta-data

	{
	int i;

	if (strEquals(sField, FIELD_HP))
		*retsValue = strFromInt(m_iHitPoints);
	else if (strEquals(sField, FIELD_BALANCE))
		*retsValue = strFromInt(CalcBalance());
	else if (strEquals(sField, FIELD_HP_BONUS))
		{
		CString sResult;

		for (i = 0; i < damageCount; i++)
			{
			if (!sResult.IsBlank())
				sResult.Append(CONSTLIT(", "));

			int iStdAdj = STD_ARMOR_DAMAGE_ADJ[m_pItemType->GetLevel() - 1][i];
			if (m_iDamageAdj[i] > 0)
				{
				int iBonus = (int)((100.0 * (iStdAdj - m_iDamageAdj[i]) / m_iDamageAdj[i]) + 0.5);

				//	Prettify. Because of rounding-error, sometimes a bonus of +25 or -25 comes out as
				//	+24 or -24. This is because we store a damage adjustment not the bonus.

				if (((iBonus + 1) % 25) == 0)
					iBonus++;
				else if (((iBonus - 1) % 25) == 0)
					iBonus--;
				else if (iBonus == 48)
					iBonus = 50;

				sResult.Append(strPatternSubst(CONSTLIT("%3d"), iBonus));
				}
			else if (iStdAdj > 0)
				sResult.Append(CONSTLIT("***"));
			else
				sResult.Append(CONSTLIT("  0"));
			}

		*retsValue = sResult;
		}
	else
		return false;

	return true;
	}

int CArmorClass::GetDamageAdj (CItemEnhancement Mods, const DamageDesc &Damage)

//	GetDamageAdj
//
//	Returns the damage adjustment

	{
	if (Mods)
		return GetDamageAdj(Damage.GetDamageType()) * Mods.GetDamageAdj(Damage) / 100;
	else
		return GetDamageAdj(Damage.GetDamageType());
	}

int CArmorClass::GetHitPoints (CItemEnhancement Mods)

//	GetHitPoints
//
//	Returns the hit points for this kind of armor

	{
	if (Mods)
		return m_iHitPoints * Mods.GetHPAdj() / 100;
	else
		return m_iHitPoints;
	}

int CArmorClass::GetHitPoints (CInstalledArmor *pArmor)

//	GetHitPoints
//
//	Returns the hit points for this kind of armor based on the installed instance

	{
	int iHP = GetHitPoints(pArmor->m_Mods);

	if (pArmor->m_fComplete)
		iHP += m_iArmorCompleteBonus;

	return iHP;
	}

CString CArmorClass::GetReference (const CItem *pItem, CSpaceObject *pInstalled)

//	GetReference
//
//	Returns a string that describes the basic attributes
//	of this armor.
//
//	Example:
//
//		30 hp; laser-resistant; impact-resistant

	{
	int i;

	//	If we don't know about the item, return Unknown

	if (pItem && !m_pItemType->ShowReference())
		return NULL_STR;

	//	Get modifications

	CItemEnhancement Mods;
	if (pItem)
		Mods = pItem->GetMods();

	//	Start with the basics

	CString sReference = strPatternSubst(CONSTLIT("%d hp"), 
			GetHitPoints(Mods));

	//	Add various enhancements

	if (m_fRadiationImmune || Mods.IsRadiationImmune())
		sReference.Append(CONSTLIT("; radiation-immune"));

	if (m_iEMPDamageAdj == 0 || Mods.IsEMPImmune())
		sReference.Append(CONSTLIT("; EMP-immune"));
	else if (m_iEMPDamageAdj < 100)
		sReference.Append(CONSTLIT("; EMP-resistant"));

	if (m_iBlindingDamageAdj == 0 || Mods.IsBlindingImmune())
		sReference.Append(CONSTLIT("; blind-immune"));
	else if (m_iBlindingDamageAdj < 100)
		sReference.Append(CONSTLIT("; blind-resistant"));

	if (m_iDeviceDamageAdj < 100 || Mods.IsDeviceDamageImmune())
		sReference.Append(CONSTLIT("; device-protect"));

	if (m_fDisintegrationImmune || Mods.IsDisintegrationImmune())
		sReference.Append(CONSTLIT("; disintegrate-immune"));

	if (m_fShieldInterference || Mods.IsShieldInterfering())
		sReference.Append(CONSTLIT("; no-shields"));

	if (m_fPhotoRepair || Mods.IsPhotoRegenerating())
		sReference.Append(CONSTLIT("; photo-repair"));

	if (m_fPhotoRecharge || Mods.IsPhotoRecharge())
		sReference.Append(CONSTLIT("; solar"));

	if ((m_iSelfRepairCycle && !m_fPhotoRepair) || Mods.IsRegenerating())
		sReference.Append(CONSTLIT("; regenerate"));

	if (m_iDecayCycle || Mods.IsDecaying())
		sReference.Append(CONSTLIT("; decay"));

	//	Reflection

	for (i = 0; i < damageCount; i++)
		{
		if (m_Reflective.InSet((DamageTypes)i)
				|| (Mods.IsReflective() && Mods.GetDamageType() == i))
			sReference.Append(strPatternSubst(CONSTLIT("; %s-reflecting"), GetDamageShortName((DamageTypes)i).GetASCIIZPointer()));
		}

	//	Append special resistance, etc.

	int AdjRow[damageCount];
	for (i = 0; i < damageCount; i++)
		{
		DamageDesc Damage((DamageTypes)i, DiceRange(0, 0, 0));
		AdjRow[i] = GetDamageAdj(Mods, Damage);
		}

	int iLevel = m_pItemType->GetLevel();
	ASSERT(iLevel > 0 && iLevel <= MAX_ITEM_LEVEL);
	sReference.Append(::ComposeDamageAdjReference(AdjRow, STD_ARMOR_DAMAGE_ADJ[iLevel - 1]));

	//	Done

	return sReference;
	}

CString CArmorClass::GetShortModifierDesc (const CItem &Item, CSpaceObject *pInstalled)

//	GetShortModifierDesc
//
//	Returns the short description of the armor modifications

	{
	return strToUpper(CItemEnhancement(Item.GetMods()).GetEnhancedDesc(Item, pInstalled));
	}

CString CArmorClass::GetShortName (void)

//	GetShortName
//
//	Returns the short name. This will basically omit the leading "segment of"
//	from the item name.

	{
	if (strFind(GetName(), STR_SEGMENT_OF) == 0)
		return strSubString(GetName(), 11, -1);
	else
		return GetName();
	}

bool CArmorClass::IsReflective (CInstalledArmor *pArmor, const DamageDesc &Damage)

//	IsReflective
//
//	Returns TRUE if the armor reflects this damage

	{
	int iReflectChance = 0;

	//	Base armor chance

	if (m_Reflective.InSet(Damage.GetDamageType()))
		iReflectChance = MAX_REFLECTION_CHANCE;

	//	Mods

	int iModReflect;
	if (pArmor && pArmor->m_Mods && pArmor->m_Mods.IsReflective(Damage, &iModReflect))
		iReflectChance = Max(iReflectChance, iModReflect);

	//	Done

	if (iReflectChance)
		{
		//	Adjust based on how damaged the armor is

		if (pArmor)
			iReflectChance = pArmor->iHitPoints * iReflectChance / GetHitPoints(pArmor);

		return (mathRandom(1, 100) <= iReflectChance);
		}
	else
		return false;
	}

void CArmorClass::Update (CInstalledArmor *pArmor, CSpaceObject *pObj, int iTick, bool *retbModified)

//	Update
//
//	Updates the armor. This should be called once every 10 ticks

	{
	bool bModified = false;

	//	See if we're regenerating

	if (pArmor->m_Mods.IsRegenerating()
			|| pArmor->m_Mods.IsPhotoRegenerating()
			|| m_iSelfRepairCycle)
		{
		int iRepairRate = 80;
		if (m_iSelfRepairCycle == 0)
			iRepairRate = 80;
		else if (pArmor->m_Mods.IsRegenerating() || pArmor->m_Mods.IsPhotoRegenerating())
			iRepairRate = Min(80, m_iSelfRepairCycle);

		if ((iTick % iRepairRate) == 0
				&& pArmor->iHitPoints < GetHitPoints(pArmor))
			{
			bool bRepair = true;

			//	If this is photo-repair armor then adjust the cycle
			//	based on how far away we are from the sun.

			if (m_fPhotoRepair || pArmor->m_Mods.IsPhotoRegenerating())
				{
				int iIntensity = pObj->GetSystem()->CalculateLightIntensity(pObj->GetPos());
				if (mathRandom(1, 100) > iIntensity)
					bRepair = false;
				}

			//	Repair one hit point

			if (bRepair)
				{
				pArmor->iHitPoints++;
				bModified = true;
				}
			}
		}

	//	See if we're decaying

	if (pArmor->iHitPoints > 0
			&& (pArmor->m_Mods.IsDecaying() || m_iDecayCycle))
		{
		int iDecayRate = 240;
		if (m_iDecayCycle == 0)
			iDecayRate = 240;
		else if (pArmor->m_Mods.IsDecaying())
			iDecayRate = Min(m_iDecayCycle, 240);

		if ((iTick % iDecayRate) == 0)
			{
			pArmor->iHitPoints--;
			bModified = true;
			}
		}

	//	If this is solar armor then recharge the object

	if (pArmor->m_Mods.IsPhotoRecharge() || m_fPhotoRecharge)
		{
		int iIntensity = pObj->GetSystem()->CalculateLightIntensity(pObj->GetPos());

		//	Intensity varies from 0 to 100 so this will recharge up to
		//	100 units of fuel every 10 ticks or 10 units per tick. At 1.5MW per fuel
		//	unit, this means that a single armor plate can support up to 15MW when
		//	right next to the sun.

		pObj->Refuel(iIntensity);
		}

	//	If this armor interferes with shields, then lower shields now

	if (pArmor->m_Mods.IsShieldInterfering() || m_fShieldInterference)
		pObj->DeactivateShields();

	//	Done

	*retbModified = bModified;
	}
