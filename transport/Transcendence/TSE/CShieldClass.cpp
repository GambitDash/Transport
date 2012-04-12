//	CShieldClass.cpp
//
//	CShieldClass class

#include "PreComp.h"

#define HIT_EFFECT_TAG							CONSTLIT("HitEffect")

#define HIT_POINTS_ATTRIB						CONSTLIT("hitPoints")
#define ABSORB_ADJ_ATTRIB						CONSTLIT("absorbAdj")
#define DAMAGE_ADJ_ATTRIB						CONSTLIT("damageAdj")
#define REGEN_TIME_ATTRIB						CONSTLIT("regenTime")
#define REGEN_HP_ATTRIB							CONSTLIT("regenHP")
#define DEPLETION_DELAY_ATTRIB					CONSTLIT("depletionDelay")
#define WEAPON_SUPPRESS_ATTRIB					CONSTLIT("weaponSuppress")
#define ARMOR_SHIELD_ATTRIB						CONSTLIT("armorShield")
#define POWER_USE_ATTRIB						CONSTLIT("powerUse")
#define REFLECT_ATTRIB							CONSTLIT("reflect")
#define HP_ADJ_PER_CHARGE_ATTRIB				CONSTLIT("HPBonusPerCharge")
#define POWER_ADJ_PER_CHARGE_ATTRIB				CONSTLIT("powerBonusPerCharge")
#define REGEN_ADJ_PER_CHARGE_ATTRIB				CONSTLIT("regenHPBonusPer10Charge")
#define MAX_CHARGES_ATTRIB						CONSTLIT("maxCharges")
#define HIT_EFFECT_ATTRIB						CONSTLIT("hitEffect")

#define ON_SHIELD_DOWN_EVENT					CONSTLIT("OnShieldDown")

#define STR_G_SOURCE							CONSTLIT("gSource")

#define FIELD_HP								CONSTLIT("hp")
#define FIELD_REGEN								CONSTLIT("regen")
#define FIELD_DAMAGE_ADJ						CONSTLIT("damageAdj")
#define FIELD_POWER								CONSTLIT("power")
#define FIELD_HP_BONUS							CONSTLIT("hpBonus")

#define MAX_REFLECTION_CHANCE		95

static int g_StdDamageAdj[MAX_ITEM_LEVEL][damageCount] =
	{
		//	lsr knt par blt  ion thr pos pls  am  nan grv sng  dac dst dlg dfr
		{	100,100,125,125, 200,200,275,275, 350,350,425,425, 500,500,575,575 },
		{	 95, 95,100,100, 175,175,250,250, 325,325,400,400, 475,475,550,550 },
		{	 90, 90, 95, 95, 150,150,225,225, 300,300,375,375, 450,450,525,525 },
		{	 80, 80, 90, 90, 125,125,200,200, 275,275,350,350, 425,425,500,500 },
		{	 70, 70, 80, 80, 100,100,175,175, 250,250,325,325, 400,400,475,475 },

		{	 60, 60, 70, 70,  95, 95,150,150, 225,225,300,300, 375,375,450,450 },
		{	 50, 50, 60, 60,  90, 90,125,125, 200,200,275,275, 350,350,425,425 },
		{	 30, 30, 50, 50,  80, 80,100,100, 175,175,250,250, 325,325,400,400 },
		{	 10, 10, 30, 30,  70, 70, 95, 95, 150,150,225,225, 300,300,375,375 },
		{	 10, 10, 10, 10,  60, 60, 90, 90, 125,125,200,200, 275,275,350,350 },

		{	 10, 10, 10, 10,  50, 50, 80, 80, 100,100,175,175, 250,250,325,325 },
		{	  0,  0, 10, 10,  30, 30, 70, 70,  95, 95,150,150, 225,225,300,300 },
		{	  0,  0,  0,  0,  10, 10, 60, 60,  90, 90,125,125, 200,200,275,275 },
		{	  0,  0,  0,  0,  10, 10, 50, 50,  80, 80,100,100, 175,175,250,250 },
		{	  0,  0,  0,  0,  10, 10, 30, 30,  70, 70, 95, 95, 150,150,225,225 },

		{	  0,  0,  0,  0,   0,  0, 10, 10,  60, 60, 90, 90, 125,125,200,200 },
		{	  0,  0,  0,  0,   0,  0, 10, 10,  50, 50, 80, 80, 100,100,175,175 },
		{	  0,  0,  0,  0,   0,  0, 10, 10,  30, 30, 70, 70,  95, 95,150,150 },
		{	  0,  0,  0,  0,   0,  0,  0,  0,  10, 10, 60, 60,  90, 90,125,125 },
		{	  0,  0,  0,  0,   0,  0,  0,  0,  10, 10, 50, 50,  80, 80,100,100 },

		{	  0,  0,  0,  0,   0,  0,  0,  0,  10, 10, 30, 30,  70, 70, 95, 95 },
		{	  0,  0,  0,  0,   0,  0,  0,  0,   0,  0, 10, 10,  60, 60, 90, 90 },
		{	  0,  0,  0,  0,   0,  0,  0,  0,   0,  0, 10, 10,  50, 50, 80, 80 },
		{	  0,  0,  0,  0,   0,  0,  0,  0,   0,  0, 10, 10,  30, 30, 70, 70 },
		{	  0,  0,  0,  0,   0,  0,  0,  0,   0,  0,  0,  0,  10, 10, 60, 60 },
	};

static int STD_POWER[MAX_ITEM_LEVEL] =
	{
		10,
		20,
		50,
		100,
		200,

		300,
		500,
		1000,
		2000,
		3000,

		4000,
		6000,
		8000,
		10000,
		12000,

		15000,
		20000,
		25000,
		30000,
		35000,

		40000,
		50000,
		60000,
		70000,
		80000
	};

CShieldClass::CShieldClass (void) : CDeviceClass(NULL)
	{
	}

bool CShieldClass::AbsorbDamage (CInstalledDevice *pDevice, CShip *pShip, SDamageCtx &Ctx)

//	AbsorbDamage
//
//	Absorbs damage

	{
	int iHPLeft = GetHPLeft(pDevice, pShip);

	//	If we're depleted then we cannot absorb anything

	if (iHPLeft == 0)
		return false;

	//	See if we're reflective

	int iReflectChance;
	if (!pDevice->GetMods().IsReflective(Ctx.Damage, &iReflectChance))
		iReflectChance = 0;
	if (m_Reflective.InSet(Ctx.Damage.GetDamageType()))
		iReflectChance = Max(iReflectChance, MAX_REFLECTION_CHANCE);
	if (iReflectChance && Ctx.pCause)
		{
		//	Compute the chance that we will reflect (based on the strength of
		//	our shields)

		int iMaxHP = GetMaxHP(pDevice, pShip);
		int iEfficiency = 50 + (iHPLeft * 50 / iMaxHP);
		int iChance = (iMaxHP > 0 ? (iEfficiency * iReflectChance / 100) : 0);

		//	See if we reflect

		if (mathRandom(1, 100) <= iChance)
			{
			//	Create the reflection

			Ctx.pCause->CreateReflection(Ctx.vHitPos, (Ctx.iDirection + 120 + mathRandom(0, 120)) % 360);

			//	Create shield effect

			if (m_pHitEffect)
				m_pHitEffect->CreateEffect(pShip->GetSystem(),
						NULL,
						Ctx.vHitPos,
						pShip->GetVel());

			//	No more damage

			Ctx.iDamage = 0;
			return true;
			}
		}

	//	Calculate how much we will absorb

	int iAbsorb = (Ctx.iDamage * m_iAbsorbAdj[Ctx.Damage.GetDamageType()]) / 100;
	if (pDevice->GetMods())
		iAbsorb = iAbsorb * pDevice->GetMods().GetAbsorbAdj(Ctx.Damage) / 100;

	//	Compute how much damage we take

	int iAdj = GetDamageAdj(pDevice->GetMods(), Ctx.Damage);
	int iShieldDamage = (iAbsorb * iAdj) / 100;

	//	If shield generator is damaged then sometimes we take extra damage

	if (pDevice->IsDamaged())
		{
		int iRoll = mathRandom(1, 100);

		if (iRoll <= 10)
			iAbsorb = 75 * iAbsorb / 100;
		else if (iRoll <= 25)
			iShieldDamage = iShieldDamage * 2;
		}

	//	If the damage that we're sustaining is more than the shield level
	//	then our shields are depleted.

	if (iShieldDamage >= iHPLeft)
		{
		SetDepleted(pDevice, pShip);

		//	Figure out how much we were actually able to absorb.

		if (iAdj)
			iAbsorb = Min(iAbsorb, (100 * iHPLeft) / iAdj);
		}
	else
		SetHPLeft(pDevice, iHPLeft - iShieldDamage);

	//	Create shield effect

	if (iAbsorb)
		{
		if (m_pHitEffect)
			m_pHitEffect->CreateEffect(pShip->GetSystem(),
					NULL,
					Ctx.vHitPos,
					pShip->GetVel());
		}

	//	Done

	pShip->OnComponentChanged(comShields);

	//	Set the remaining damage

	Ctx.iDamage -= iAbsorb;
	return (Ctx.iDamage == 0);
	}

bool CShieldClass::AbsorbsWeaponFire (CInstalledDevice *pDevice, CSpaceObject *pSource, CInstalledDevice *pWeapon)

//	AbsorbsWeaponFire
//
//	Returns TRUE if the shield absorbs fire from the given weapon
//	when installed on the same ship

	{
	int iType = pWeapon->GetClass()->GetDamageType(pWeapon);
	if (iType != -1 
			&& m_WeaponSuppress.InSet(iType)
			&& pDevice->IsEnabled()
			&& !IsDepleted(pDevice))
		return true;
	else
		return false;
	}

void CShieldClass::Activate (CInstalledDevice *pDevice, 
							 CSpaceObject *pSource, 
							 CSpaceObject *pTarget,
							 int iFireAngle,
							 bool *retbSourceDestroyed,
							 bool *retbConsumedItems)

//	Activate
//
//	Activates device

	{
	}

int CShieldClass::CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	CalcPowerUsed
//
//	Returns the amount of power used by the shields

	{
	int iPower = 0;

	//	Only if enabled

	if (!pDevice->IsEnabled())
		return 0;

	//	If we're regenerating shields, then we consume more power
	//	otherwise, we only consume half power

	if (m_iRegenHP > 0
			&& GetHPLeft(pDevice, pSource) < GetMaxHP(pDevice, pSource))
		iPower += m_iPowerUse;
	else
		iPower += m_iPowerUse / 2;

	//	Adjust based on charges

	if (m_iExtraPowerPerCharge)
		iPower += m_iExtraPowerPerCharge * pDevice->GetCharges(pSource);

	//	Adjust based on power efficiency enhancement

	if (pDevice->GetMods())
		iPower = iPower * pDevice->GetMods().GetPowerAdj() / 100;

	return iPower;
	}

ALERROR CShieldClass::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpShield)

//	CreateFromXML
//
//	Creates from an XML element

	{
	ALERROR error;
	CShieldClass *pShield;
	int i;

	pShield = new CShieldClass;
	if (pShield == NULL)
		return ERR_MEMORY;

	pShield->InitDeviceFromXML(pDesc, pType);

	pShield->m_iHitPoints = pDesc->GetAttributeInteger(HIT_POINTS_ATTRIB);
	pShield->m_iArmorShield = pDesc->GetAttributeInteger(ARMOR_SHIELD_ATTRIB);
	pShield->m_iRegenHP = pDesc->GetAttributeInteger(REGEN_HP_ATTRIB);
	pShield->m_iRegenRate = (int)((pDesc->GetAttributeInteger(REGEN_TIME_ATTRIB) / STD_SECONDS_PER_UPDATE) + 0.5);
	pShield->m_iDepletionDelay = pDesc->GetAttributeInteger(DEPLETION_DELAY_ATTRIB);
	pShield->m_iPowerUse = pDesc->GetAttributeInteger(POWER_USE_ATTRIB);
	pShield->m_iExtraHPPerCharge = pDesc->GetAttributeInteger(HP_ADJ_PER_CHARGE_ATTRIB);
	pShield->m_iExtraPowerPerCharge = pDesc->GetAttributeInteger(POWER_ADJ_PER_CHARGE_ATTRIB);
	pShield->m_iExtraRegenPerCharge = pDesc->GetAttributeInteger(REGEN_ADJ_PER_CHARGE_ATTRIB);
	pShield->m_iMaxCharges = pDesc->GetAttributeInteger(MAX_CHARGES_ATTRIB);

	//	Load damage adjustment

	if (error = LoadDamageAdj(pDesc, g_StdDamageAdj[pType->GetLevel() - 1], pShield->m_iDamageAdj))
		return error;

	CIntArray AbsorbAdj;
	if (error = pDesc->GetAttributeIntegerList(ABSORB_ADJ_ATTRIB, &AbsorbAdj))
		return error;

	for (i = 0; i < damageCount; i++)
		pShield->m_iAbsorbAdj[i] = (i < AbsorbAdj.GetCount() ? AbsorbAdj.GetElement(i) : 0);

	//	Load the weapon suppress

	if (error = pShield->m_WeaponSuppress.InitFromXML(pDesc->GetAttribute(WEAPON_SUPPRESS_ATTRIB)))
		return error;

	//	Load reflection

	if (error = pShield->m_Reflective.InitFromXML(pDesc->GetAttribute(REFLECT_ATTRIB)))
		return error;

	//	Load events

	pShield->m_pOnShieldDown = pType->GetEventHandler(ON_SHIELD_DOWN_EVENT);

	//	Effects

	if (error = pShield->m_pHitEffect.LoadEffect(Ctx,
			strPatternSubst(CONSTLIT("%d:h"), pType->GetUNID()),
			pDesc->GetContentElementByTag(HIT_EFFECT_TAG),
			pDesc->GetAttribute(HIT_EFFECT_ATTRIB)))
		return error;

	//	Done

	*retpShield = pShield;

	return NOERROR;
	}

void CShieldClass::Deplete (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	Disable
//
//	Lower shields

	{
	SetDepleted(pDevice, pSource);
	pSource->OnComponentChanged(comShields);
	}

bool CShieldClass::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns meta-data

	{
	int i;

	if (strEquals(sField, FIELD_HP))
		*retsValue = strFromInt(m_iHitPoints, TRUE);
	else if (strEquals(sField, FIELD_REGEN))
		{
		if (m_iRegenRate == 0)
			*retsValue = CONSTLIT("0");
		else
			*retsValue = strFromInt(m_iRegenHP * 1000 / m_iRegenRate, TRUE);
		}
	else if (strEquals(sField, FIELD_DAMAGE_ADJ))
		{
		retsValue->Truncate(0);

		for (i = 0; i < damageCount; i++)
			{
			if (i > 0)
				retsValue->Append(CONSTLIT("\t"));

			retsValue->Append(strFromInt(m_iDamageAdj[i]));
			}
		}
	else if (strEquals(sField, FIELD_POWER))
		*retsValue = strFromInt(m_iPowerUse * 100, TRUE);
	else if (strEquals(sField, FIELD_HP_BONUS))
		{
		CString sResult;

		for (i = 0; i < damageCount; i++)
			{
			if (!sResult.IsBlank())
				sResult.Append(CONSTLIT(", "));

			int iStdAdj = g_StdDamageAdj[GetLevel() - 1][i];
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

void CShieldClass::FireOnShieldDown (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	FireOnShieldDown
//
//	Fire OnShieldDown event

	{
	ASSERT(m_pOnShieldDown);

	CCodeChain &CC = g_pUniverse->GetCC();

	ICCItem *pOldSource = CC.LookupGlobal(STR_G_SOURCE, &g_pUniverse);
	DefineGlobalSpaceObject(CC, STR_G_SOURCE, pSource);

	ICCItem *pResult = CC.TopLevel(m_pOnShieldDown, &g_pUniverse);
	if (pResult->IsError())
		pSource->ReportEventError(ON_SHIELD_DOWN_EVENT, pResult);
	pResult->Discard(&CC);

	//	Restore variable

	CC.DefineGlobal(STR_G_SOURCE, pOldSource);
	pOldSource->Discard(&CC);
	}

int CShieldClass::GetDamageAdj (CItemEnhancement Mods, const DamageDesc &Damage)

//	GetDamageAdj
//
//	Damage adjustment

	{
	if (Mods)
		return m_iDamageAdj[Damage.GetDamageType()] * Mods.GetDamageAdj(Damage) / 100;
	else
		return m_iDamageAdj[Damage.GetDamageType()];
	}

int CShieldClass::GetHPLeft (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	GetHPLeft
//
//	Returns the number of HP left of shields

	{
	int iHPLeft = (int)pDevice->GetData();

	if (iHPLeft < 0)
		return 0;
	else
		return iHPLeft;
	}

int CShieldClass::GetMaxHP (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	GetMaxHP
//
//	Max HP of shields
	
	{
	int iMax = m_iHitPoints;

	//	Adjust based on enhancements

	if (pDevice->GetMods())
		iMax = iMax * pDevice->GetMods().GetHPAdj() / 100;

	//	Adjust based on charges

	if (m_iExtraHPPerCharge)
		iMax = Max(0, iMax + m_iExtraHPPerCharge * pDevice->GetCharges(pSource));

	//	Adjust if shield is based on armor strength

	CShip *pShip;
	if (m_iArmorShield && (pShip = pSource->AsShip()))
		{
		//	Compute the average HP of all the armor

		int iArmorHP = 0;
		int iArmorCount = pShip->GetArmorSectionCount();
		for (int i = 0; i < iArmorCount; i++)
			iArmorHP += pShip->GetArmorSection(i)->iHitPoints;

		if (iArmorCount > 0)
			iArmorHP = ((m_iArmorShield * iArmorHP / iArmorCount) + 5) / 10;

		//	Return HP left

		iMax = Min(iMax, iArmorHP);
		}

	//	Done

	return iMax;
	}

int CShieldClass::GetPowerRating (const CItem *pItem)

//	GetPowerRating
//
//	Returns the power rating of the item

	{
	CItemEnhancement Mods;
	if (pItem)
		Mods = pItem->GetMods();

	int iPower = m_iPowerUse;

	if (Mods)
		iPower = iPower * Mods.GetPowerAdj() / 100;

	return iPower;
	}

CString CShieldClass::GetReference (const CItem *pItem, CSpaceObject *pInstalled)

//	GetReference
//
//	Returns a string that describes the basic attributes
//	of this shield
//
//	Example:
//
//		20 hp (average regen); 100MW

	{
	int i;

	CString sReference;
	CString sStrength;
	CString sRegeneration;
	CItemEnhancement Mods;
	if (pItem)
		Mods = pItem->GetMods();

	//	If we don't know about the item, return only basic info

	if (pItem && !GetItemType()->ShowReference())
		return CDeviceClass::GetReference(pItem, pInstalled);

	//	Compute the strength string

	int iMax = m_iHitPoints;
	if (Mods)
		iMax = iMax * Mods.GetHPAdj() / 100;

	int iMin = iMax;

	if (m_iArmorShield)
		iMin = m_iArmorShield;

	if (m_iExtraHPPerCharge)
		iMax += m_iExtraHPPerCharge * m_iMaxCharges;

	if (iMax == iMin)
		sStrength = strFromInt(iMax, FALSE);
	else if (iMax > iMin)
		sStrength = strPatternSubst(CONSTLIT("%d-%d"), iMin, iMax);
	else
		sStrength = strPatternSubst(CONSTLIT("%d-%d"), iMax, iMin);

	//	Compute the regeneration

	if (m_iRegenHP > 0)
		{
		int iFullRegenTicks = m_iRegenRate * ((iMax / m_iRegenHP) + 1);
		if (iFullRegenTicks < 30)
			sRegeneration = CONSTLIT("instantaneous");
		else if (iFullRegenTicks < 90)
			sRegeneration = CONSTLIT("very fast");
		else if (iFullRegenTicks < 180)
			sRegeneration = CONSTLIT("fast");
		else if (iFullRegenTicks < 360)
			sRegeneration = CONSTLIT("average");
		else if (iFullRegenTicks < 720)
			sRegeneration = CONSTLIT("slow");
		else
			sRegeneration = CONSTLIT("very slow");
		}
	else
		sRegeneration = CONSTLIT("no");

	sReference = strPatternSubst("%s hp (%s regen); %s", 
			sStrength.GetASCIIZPointer(), 
			sRegeneration.GetASCIIZPointer(),
			GetReferencePower(pItem).GetASCIIZPointer());

	//	Reflection

	for (i = 0; i < damageCount; i++)
		{
		if (m_Reflective.InSet((DamageTypes)i)
				|| (Mods.IsReflective() && Mods.GetDamageType() == i))
			sReference.Append(strPatternSubst(CONSTLIT("; %s-reflecting"), GetDamageShortName((DamageTypes)i).GetASCIIZPointer()));
		}

	//	Compute damage adjustments

	int AdjRow[damageCount];
	for (i = 0; i < damageCount; i++)
		{
		DamageDesc Damage((DamageTypes)i, DiceRange(0, 0, 0));
		int iAbsorb = m_iAbsorbAdj[i] * Mods.GetAbsorbAdj(Damage) / 100;
		if (iAbsorb < 100)
			AdjRow[i] = 1000;
		else
			AdjRow[i] = GetDamageAdj(Mods, Damage);
		}

	int iLevel = GetItemType()->GetLevel();
	ASSERT(iLevel > 0 && iLevel <= MAX_ITEM_LEVEL);
	sReference.Append(::ComposeDamageAdjReference(AdjRow, g_StdDamageAdj[iLevel - 1]));

	return sReference;
	}

void CShieldClass::GetStatus (CInstalledDevice *pDevice, CShip *pShip, int *retiStatus, int *retiMaxStatus)

//	GetStatus
//
//	Returns the status of the shields

	{
	*retiStatus = GetHPLeft(pDevice, pShip);
	*retiMaxStatus = GetMaxHP(pDevice, pShip);
	}

int CShieldClass::GetStdPowerByLevel (int iLevel)

//	GetStdPowerByLevel
//
//	Returns the standard power consumed by a shield at the given level (in 1/10th MW)

	{
	if (iLevel >= 1 && iLevel <= MAX_ITEM_LEVEL)
		return STD_POWER[iLevel - 1];
	else
		return -1;
	}

bool CShieldClass::IsDepleted (CInstalledDevice *pDevice)

//	IsDepleted
//
//	Returns TRUE if shields depleted

	{
	return ((int)pDevice->GetData() < 0); 
	}

ALERROR CShieldClass::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Bind design

	{
	ALERROR error;

	if (error = m_pHitEffect.Bind(Ctx))
		return error;

	//	If the hit effect is NULL, then use default

	if (m_pHitEffect == NULL)
		m_pHitEffect.Set(g_pUniverse->FindEffectType(g_ShieldEffectUNID));

	return NOERROR;
	}

void CShieldClass::OnInstall (CInstalledDevice *pDevice, CSpaceObject *pSource, CItemListManipulator &ItemList)

//	OnInstall
//
//	Called when the device is installed

	{
	//	Set shields to max HP

	SetHPLeft(pDevice, GetMaxHP(pDevice, pSource));
	}

void CShieldClass::OnLoadImages (void)

//	OnLoadImages
//
//	Loads images used by shield

	{
	if (m_pHitEffect)
		m_pHitEffect->LoadImages();
	}

void CShieldClass::OnMarkImages (void)

//	OnMarkImages
//
//	Marks images used by shield

	{
	if (m_pHitEffect)
		m_pHitEffect->MarkImages();
	}

void CShieldClass::Recharge (CInstalledDevice *pDevice, CShip *pShip, int iStatus)

//	Recharge
//
//	Recharges hitpoints (this is a manual recharge, so we ignore whether we are
//	depleted or not).

	{
	int iMaxHP = GetMaxHP(pDevice, pShip);
	int iHPLeft = GetHPLeft(pDevice, pShip);
	SetHPLeft(pDevice, std::min(iMaxHP, iHPLeft + iStatus));
	pShip->OnComponentChanged(comShields);
	}

bool CShieldClass::RequiresItems (void)

//	RequiresItems
//
//	Shield requires some other item to function

	{
#ifdef LATER
	//	Need to explicitly list superconducting coils as a required
	//	item for these shields to function
#else
	return (m_iRegenHP == 0);
#endif
	}

void CShieldClass::Reset (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	Reset
//
//	Lower shields

	{
	//	Note: We do not call Deplete because we don't want to invoke the
	//	OnShieldDown event

	SetHPLeft(pDevice, 0);
	pSource->OnComponentChanged(comShields);
	}

void CShieldClass::SetDepleted (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	SetDepleted
//
//	Makes shields depleted

	{
	pDevice->SetData((DWORD)(-m_iDepletionDelay));

	//	Fire event (We don't fire the event if we're disabled because we
	//	don't want something like the Invincible deflector to disable the ship
	//	if the shield is not enabled)

	if (m_pOnShieldDown && pDevice->IsEnabled())
		FireOnShieldDown(pDevice, pSource);
	}

void CShieldClass::SetHPLeft (CInstalledDevice *pDevice, int iHP)

//	SetHPLeft
//
//	Sets HP left on shields

	{
	pDevice->SetData((DWORD)iHP);
	}

void CShieldClass::Update (CInstalledDevice *pDevice, CSpaceObject *pSource, int iTick, bool *retbSourceDestroyed, bool *retbConsumedItems)

//	Update
//
//	Updates the shield

	{
	//	Regen

	if ((iTick % m_iRegenRate) == 0)
		{
		if (!pDevice->IsEnabled())
			{
			//	Note: We don't call SetDepleted because we don't want to fire the OnShieldDown
			//	event. If necessary, we should add an OnDeviceDisabled event.

			SetHPLeft(pDevice, 0);
			pSource->OnComponentChanged(comShields);
			return;
			}

		//	If we're damaged, then there is a chance that we might be depleted

		if (pDevice->IsDamaged())
			{
			if (mathRandom(1, 100) <= 5)
				{
				Deplete(pDevice, pSource);
				pSource->OnDeviceFailure(pDevice, failShieldFailure);
				}
			}

		//	If we're depleted then update the HP
		//	Otherwise, regenerate

		if (m_iRegenHP > 0 && !UpdateDepleted(pDevice))
			{
			int iMaxHP = GetMaxHP(pDevice, pSource);
			int iHPLeft = GetHPLeft(pDevice, pSource);

			if (iHPLeft != iMaxHP)
				{
				//	Figure out how much to regen

				int iRegenHP = m_iRegenHP;
				if (m_iExtraRegenPerCharge)
					{
					int iCharges = pDevice->GetCharges(pSource);
					int iExtra = (m_iExtraRegenPerCharge * iCharges) / 10;
					int iRemainder = (m_iExtraRegenPerCharge * iCharges) % 10;

					iRegenHP += iExtra;
					if (mathRandom(1, 10) <= iRemainder)
						iRegenHP += 1;
					}

				//	Regen

				SetHPLeft(pDevice, std::min(iMaxHP, iHPLeft + iRegenHP));
				pSource->OnComponentChanged(comShields);
				}
			}
		}

	if (retbConsumedItems)
		*retbConsumedItems = false;
	}

bool CShieldClass::UpdateDepleted (CInstalledDevice *pDevice)

//	UpdateDepleted
//
//	Updates depleted state. Returns FALSE if we are not
//	depleted.

	{
	int iLeft = (int)pDevice->GetData();
	if (iLeft < 0)
		{
		iLeft++;
		pDevice->SetData((DWORD)iLeft);
		return true;
		}
	else
		return false;
	}
