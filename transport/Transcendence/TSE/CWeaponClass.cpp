//	CWeaponClass.cpp
//
//	CWeaponClass class

#include "PreComp.h"

#define MISSILES_TAG					CONSTLIT("Missiles")
#define CONFIGURATION_TAG				CONSTLIT("Configuration")

#define POWER_USE_ATTRIB				CONSTLIT("powerUse")
#define RECOIL_ATTRIB					CONSTLIT("recoil")
#define CHARGES_ATTRIB					CONSTLIT("charges")
#define CONFIGURATION_ATTRIB			CONSTLIT("configuration")
#define LAUNCHER_ATTRIB					CONSTLIT("launcher")
#define ANGLE_ATTRIB					CONSTLIT("angle")
#define POS_ANGLE_ATTRIB				CONSTLIT("posAngle")
#define POS_RADIUS_ATTRIB				CONSTLIT("posRadius")
#define AIM_TOLERANCE_ATTRIB			CONSTLIT("aimTolerance")
#define HEATING_ATTRIB					CONSTLIT("heating")
#define COOLING_RATE_ATTRIB				CONSTLIT("coolingRate")
#define MULTI_TARGET_ATTRIB				CONSTLIT("multiTarget")
#define MIN_FIRE_ARC_ATTRIB				CONSTLIT("minFireArc")
#define MAX_FIRE_ARC_ATTRIB				CONSTLIT("maxFireArc")
#define FAILURE_CHANCE_ATTRIB			CONSTLIT("failureChance")
#define COUNTER_ATTRIB					CONSTLIT("counter")
#define COUNTER_UPDATE_RATE_ATTRIB		CONSTLIT("counterUpdateRate")
#define COUNTER_UPDATE_ATTRIB			CONSTLIT("counterUpdate")
#define COUNTER_ACTIVATE_ATTRIB			CONSTLIT("counterActivate")

#define CONFIG_TYPE_DUAL				CONSTLIT("dual")
#define CONFIG_TYPE_WALL				CONSTLIT("wall")
#define CONFIG_TYPE_SPREAD2				CONSTLIT("spread2")
#define CONFIG_TYPE_SPREAD3				CONSTLIT("spread3")
#define CONFIG_TYPE_SPREAD5				CONSTLIT("spread5")
#define CONFIG_TYPE_DUAL_ALTERNATING	CONSTLIT("alternating")

#define COUNTER_TYPE_TEMPERATURE		CONSTLIT("temperature")
#define COUNTER_TYPE_CAPACITOR			CONSTLIT("capacitor")

#define FIELD_MIN_DAMAGE				CONSTLIT("minDamage")
#define FIELD_MAX_DAMAGE				CONSTLIT("maxDamage")
#define FIELD_DAMAGE_TYPE				CONSTLIT("damageType")
#define FIELD_FIRE_RATE					CONSTLIT("fireRate")
#define FIELD_FIRE_DELAY				CONSTLIT("fireDelay")		//	Delay (ticks)
#define FIELD_AVERAGE_DAMAGE			CONSTLIT("averageDamage")	//	Average damage (1000x hp)
#define FIELD_POWER_PER_SHOT			CONSTLIT("powerPerShot")	//	Power used per shot (1000x Megawatt minutes)

static CObjectClass<CWeaponClass>g_Class(OBJID_CWEAPONCLASS, NULL);

static char g_FireRateAttrib[] = "fireRate";
static char g_UNIDAttrib[] = "UNID";
static char g_ItemIDAttrib[] = "itemID";
static char g_OmnidirectionalAttrib[] = "omnidirectional";

const Metric g_DualShotSeparation =		12;					//	Radius of dual shot (pixels)
const int TEMP_DECREASE =				-1;					//	Decrease in temp per cooling rate
const int MAX_TEMP =					120;				//	Max temperature
const int OVERHEAT_TEMP =				100;				//	Weapon overheats
const Metric MAX_TARGET_RANGE =			(24.0 * LIGHT_SECOND);
const int MAX_COUNTER =					100;

CWeaponClass::CWeaponClass (void) : CDeviceClass(&g_Class),
		m_pConfig(NULL)
	{
	}

CWeaponClass::~CWeaponClass (void)
	{
	delete [] m_pShotData;

	if (m_pConfig)
		delete [] m_pConfig;
	}

void CWeaponClass::Activate (CInstalledDevice *pDevice, 
							 CSpaceObject *pSource, 
							 CSpaceObject *pTarget,
							 int iFireAngle,
							 bool *retbSourceDestroyed,
							 bool *retbConsumedItems)

//	Activate
//
//	Activates the device (in this case, fires the weapon)

	{
	CWeaponFireDesc *pShot = GetSelectedShotData(pDevice);

	if (retbSourceDestroyed)
		*retbSourceDestroyed = false;
	if (retbConsumedItems)
		*retbConsumedItems = false;

	if (pShot && pDevice->IsEnabled())
		{
		bool bSourceDestroyed;

		//	Fire the weapon

		FireWeapon(pDevice, pShot, pSource, pTarget, iFireAngle, true, &bSourceDestroyed, retbConsumedItems);

		//	If firing the weapon destroyed the ship, then we bail out

		if (bSourceDestroyed)
			{
			if (retbSourceDestroyed)
				*retbSourceDestroyed = true;
			return;
			}

		//	If this is a continuous fire weapon then set the device data

		if (pShot->m_iContinuous > 0)
			SetContinuousFire(pDevice, pShot->m_iContinuous);
		}
	}

int CWeaponClass::CalcConfigurationMultiplier (CWeaponFireDesc *pShot)

//	CalcConfigurationMultiplier
//
//	Calculates multiplier

	{
	if (pShot == NULL)
		{
		if (m_iShotVariants == 1)
			pShot = &m_pShotData[0];
		else
			return 1;
		}

	int iMult = 1;
	switch (m_Configuration)
		{
		case ctDual:
		case ctSpread2:
			iMult = 2;
			break;

		case ctSpread3:
			iMult = 3;
			break;

		case ctWall:
		case ctSpread5:
			iMult = 5;
			break;

		case ctCustom:
			iMult = m_iConfigCount;
			break;
		}

	if (pShot->m_iContinuous > 1)
		iMult *= pShot->m_iContinuous;

	//	Compute fragment count

	if (pShot->HasFragments()
			&& pShot->GetFirstFragment()->Count.GetMaxValue() > 1)
		{
		int iMin = pShot->GetFirstFragment()->Count.GetMinValue();
		int iMax = pShot->GetFirstFragment()->Count.GetMaxValue();

		//	Compute the average, then divide by two (assume that at
		//	most half the fragments will hit)

		iMult *= ((iMin + iMax) / 2) / 2;
		}

	return iMult;
	}

int CWeaponClass::CalcPowerUsed (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	CalcPowerUsed
//
//	Returns the power consumed

	{
	if (!pDevice->IsEnabled())
		return 0;

	//	We consume less power when we are fully charged

	int iPower = m_iPowerUse;
	if (pDevice->IsReady())
		iPower = iPower / 10;

	//	Adjust based on power efficiency enhancement

	if (pDevice->GetMods())
		iPower = iPower * pDevice->GetMods().GetPowerAdj() / 100;

	return iPower;
	}

bool CWeaponClass::CanRotate (CInstalledDevice *pDevice)

//	CanRotate
//
//	Returns TRUE if the weapon is either omnidirectional or directional

	{
	if (m_bOmnidirectional || (m_iMinFireArc != m_iMaxFireArc))
		return true;
	else if (pDevice && (pDevice->IsOmniDirectional() || pDevice->IsDirectional()))
		return true;
	else
		return false;
	}

ALERROR CWeaponClass::CreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, CItemType *pType, CDeviceClass **retpWeapon)

//	CreateFromXML
//
//	Creates from an XML element

	{
	ALERROR error;
	int i;
	CWeaponClass *pWeapon;

	pWeapon = new CWeaponClass;
	if (pWeapon == NULL)
		return ERR_MEMORY;

	pWeapon->InitDeviceFromXML(pDesc, pType);

	pWeapon->m_iFireRateSecs = pDesc->GetAttributeInteger(CONSTLIT(g_FireRateAttrib));
	pWeapon->m_iFireRate = (int)((pWeapon->m_iFireRateSecs / STD_SECONDS_PER_UPDATE) + 0.5);
	pWeapon->m_iPowerUse = pDesc->GetAttributeInteger(POWER_USE_ATTRIB);
	pWeapon->m_iRecoil = pDesc->GetAttributeInteger(RECOIL_ATTRIB);
	pWeapon->m_bCharges = pDesc->GetAttributeBool(CHARGES_ATTRIB);
	pWeapon->m_iFailureChance = pDesc->GetAttributeInteger(FAILURE_CHANCE_ATTRIB);
	pWeapon->m_iMinFireArc = pDesc->GetAttributeInteger(MIN_FIRE_ARC_ATTRIB);
	pWeapon->m_iMaxFireArc = pDesc->GetAttributeInteger(MAX_FIRE_ARC_ATTRIB);

	//	Configuration

	CString sConfig = pDesc->GetAttribute(CONFIGURATION_ATTRIB);
	if (strEquals(sConfig, CONFIG_TYPE_DUAL))
		pWeapon->m_Configuration = ctDual;
	else if (strEquals(sConfig, CONFIG_TYPE_WALL))
		pWeapon->m_Configuration = ctWall;
	else if (strEquals(sConfig, CONFIG_TYPE_SPREAD2))
		pWeapon->m_Configuration = ctSpread2;
	else if (strEquals(sConfig, CONFIG_TYPE_SPREAD3))
		pWeapon->m_Configuration = ctSpread3;
	else if (strEquals(sConfig, CONFIG_TYPE_SPREAD5))
		pWeapon->m_Configuration = ctSpread5;
	else if (strEquals(sConfig, CONFIG_TYPE_DUAL_ALTERNATING))
		pWeapon->m_Configuration = ctDualAlternating;
	else
		{
		CXMLElement *pConfig = pDesc->GetContentElementByTag(CONFIGURATION_TAG);
		if (pConfig && pConfig->GetContentElementCount())
			{
			pWeapon->m_Configuration = ctCustom;

			pWeapon->m_iConfigCount = pConfig->GetContentElementCount();
			pWeapon->m_pConfig = new SConfigDesc [pWeapon->m_iConfigCount];

			for (i = 0; i < pWeapon->m_iConfigCount; i++)
				{
				CXMLElement *pShotConfig = pConfig->GetContentElement(i);
				pWeapon->m_pConfig[i].Angle.LoadFromXML(pShotConfig->GetAttribute(ANGLE_ATTRIB));
				pWeapon->m_pConfig[i].iPosAngle = (pShotConfig->GetAttributeInteger(POS_ANGLE_ATTRIB) + 360) % 360;
				pWeapon->m_pConfig[i].rPosRadius = g_KlicksPerPixel * pShotConfig->GetAttributeInteger(POS_RADIUS_ATTRIB);
				}

			pWeapon->m_iConfigAimTolerance = pConfig->GetAttributeInteger(AIM_TOLERANCE_ATTRIB);
			if (pWeapon->m_iConfigAimTolerance == 0)
				pWeapon->m_iConfigAimTolerance = 5;
			}
		else
			pWeapon->m_Configuration = ctSingle;
		}

	//	Counter

	sConfig = pDesc->GetAttribute(COUNTER_ATTRIB);
	if (!sConfig.IsBlank())
		{
		if (strEquals(sConfig, COUNTER_TYPE_TEMPERATURE))
			pWeapon->m_Counter = cntTemperature;
		else if (strEquals(sConfig, COUNTER_TYPE_CAPACITOR))
			pWeapon->m_Counter = cntCapacitor;
		else
			{
			Ctx.sError = CONSTLIT("Invalid weapon counter type");
			return ERR_FAIL;
			}

		pWeapon->m_iCounterActivate = pDesc->GetAttributeInteger(COUNTER_ACTIVATE_ATTRIB);
		pWeapon->m_iCounterUpdate = pDesc->GetAttributeInteger(COUNTER_UPDATE_ATTRIB);
		pWeapon->m_iCounterUpdateRate = pDesc->GetAttributeInteger(COUNTER_UPDATE_RATE_ATTRIB);
		if (pWeapon->m_iCounterUpdateRate <= 0)
			pWeapon->m_iCounterUpdateRate = 1;
		}
	else if ((pWeapon->m_iCounterActivate = pDesc->GetAttributeInteger(HEATING_ATTRIB)) > 0)
		{
		//	Backward compatibility

		pWeapon->m_Counter = cntTemperature;
		pWeapon->m_iCounterUpdate = TEMP_DECREASE;
		pWeapon->m_iCounterUpdateRate = pDesc->GetAttributeInteger(COOLING_RATE_ATTRIB);
		if (pWeapon->m_iCounterUpdateRate <= 0)
			pWeapon->m_iCounterUpdateRate = 1;
		}
	else
		{
		pWeapon->m_Counter = cntNone;
		pWeapon->m_iCounterActivate = 0;
		pWeapon->m_iCounterUpdate = 0;
		pWeapon->m_iCounterUpdateRate = 0;
		}

	//	Flags

	pWeapon->m_bOmnidirectional = pDesc->GetAttributeBool(CONSTLIT(g_OmnidirectionalAttrib));
	pWeapon->m_bMIRV = pDesc->GetAttributeBool(MULTI_TARGET_ATTRIB);

	//	If we have a Missiles tag then this weapon has ammunition; otherwise,
	//	we take the information from the root element

	CXMLElement *pMissiles = pDesc->GetContentElementByTag(MISSILES_TAG);
	if (pMissiles)
		{
		pWeapon->m_iShotVariants = pMissiles->GetContentElementCount();
		pWeapon->m_pShotData = new CWeaponFireDesc[pWeapon->m_iShotVariants];
		if (pWeapon->m_pShotData == NULL)
			return ERR_MEMORY;

		for (i = 0; i < pWeapon->m_iShotVariants; i++)
			{
			CXMLElement *pItem = pMissiles->GetContentElement(i);

			CString sUNID = strPatternSubst(CONSTLIT("%d/%d"), pWeapon->GetUNID(), i);
			if (error = pWeapon->m_pShotData[i].InitFromXML(Ctx, pItem, sUNID))
				return error;
			}
		}
	else
		{
		pWeapon->m_iShotVariants = 1;
		pWeapon->m_pShotData = new CWeaponFireDesc[pWeapon->m_iShotVariants];
		if (pWeapon->m_pShotData == NULL)
			return ERR_MEMORY;

		CString sUNID = strPatternSubst(CONSTLIT("%d/0"), pWeapon->GetUNID());
		if (error = pWeapon->m_pShotData[0].InitFromXML(Ctx, pDesc, sUNID))
			return error;
		}

	//	If this weapon uses different kinds of ammo then it is a launcher

	if (pWeapon->m_iShotVariants > 1)
		pWeapon->m_bLauncher = true;
	else
		pWeapon->m_bLauncher = pDesc->GetAttributeBool(LAUNCHER_ATTRIB);

	//	Done

	*retpWeapon = pWeapon;

	return NOERROR;
	}

bool CWeaponClass::FindDataField (int iVariant, const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns meta-data

	{
	if (iVariant < 0 || iVariant >= m_iShotVariants)
		return false;

	CWeaponFireDesc *pShot = &m_pShotData[iVariant];
	if (pShot == NULL)
		return false;

	if (strEquals(sField, FIELD_MIN_DAMAGE))
		*retsValue = strFromInt(CalcConfigurationMultiplier(pShot) * pShot->m_Damage.GetMinDamage(), FALSE);
	else if (strEquals(sField, FIELD_MAX_DAMAGE))
		*retsValue = strFromInt(CalcConfigurationMultiplier(pShot) * pShot->m_Damage.GetMaxDamage(), FALSE);
	else if (strEquals(sField, FIELD_DAMAGE_TYPE))
		*retsValue = strFromInt(pShot->m_Damage.GetDamageType(), FALSE);
	else if (strEquals(sField, FIELD_FIRE_DELAY))
		*retsValue = strFromInt(m_iFireRate, FALSE);
	else if (strEquals(sField, FIELD_FIRE_RATE))
		{
		if (m_iFireRate)
			*retsValue = strFromInt(1000 / m_iFireRate, FALSE);
		else
			return false;
		}
	else if (strEquals(sField, FIELD_AVERAGE_DAMAGE))
		{
		if (pShot->HasFragments())
			*retsValue = strFromInt((int)(CalcConfigurationMultiplier(pShot) 
					* pShot->GetFirstFragment()->pDesc->m_Damage.GetAverageDamage() 
					* 1000.0 + 0.5), 
					FALSE);
		else
			*retsValue = strFromInt((int)(CalcConfigurationMultiplier(pShot) 
					* pShot->m_Damage.GetAverageDamage() 
					* 1000.0 + 0.5), 
					FALSE);
		}
	else if (strEquals(sField, FIELD_POWER_PER_SHOT))
		*retsValue = strFromInt((int)(((m_iFireRate * m_iPowerUse * STD_SECONDS_PER_UPDATE * 1000) / 600.0) + 0.5), FALSE);
	else
		return false;

	return true;
	}

bool CWeaponClass::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns meta-data

	{
	return FindDataField(0, sField, retsValue);
	}

void CWeaponClass::FireWeapon (CInstalledDevice *pDevice, 
							   CWeaponFireDesc *pShot, 
							   CSpaceObject *pSource, 
							   CSpaceObject *pTarget,
							   int iFireAngle,
							   bool bConsumeItems,
							   bool *retbSourceDestroyed,
							   bool *retbConsumedItems)

//	FireWeapon
//
//	Fires the weapon

	{
	int i;
	bool bFireSuppressed = false;
	bool bNextVariant = false;
	const int iMaxShotCount = 100;
	int iShotCount = 0;
	CVector ShotPos[iMaxShotCount];
	CVector ShotVel[iMaxShotCount];
	int ShotDir[iMaxShotCount];

	//	Pre-init

	if (retbConsumedItems)
		*retbConsumedItems = false;
	if (retbSourceDestroyed)
		*retbSourceDestroyed = false;

	//	Figure out the source of the shot

	CVector vSource = pDevice->GetPos(pSource);

	//	Figure out if fire is suppressed by some other object
	//	on the source

	bFireSuppressed = pSource->AbsorbWeaponFire(pDevice);

	//	If we're prone to failure, then bad things happen

	bool bFailure = false;
	if (m_iFailureChance > 0)
		{
		if (mathRandom(1, 100) <= m_iFailureChance)
			bFailure = true;
		}

	//	If we're damaged, figure out what bad things happen

	if (pDevice->IsDamaged())
		bFailure = true;

	bool bMisfire = false;
	bool bExplosion = false;
	if (bFailure)
		{
		int iRoll = mathRandom(1, 100);

		//	30% of the time, everything works OK

		if (iRoll <= 30)
			;

		//	40% of the time, nothing happens

		else if (iRoll <= 70)
			{
			pSource->OnDeviceFailure(pDevice, failWeaponJammed);
			return;
			}

		//	20% of the time, we fire in a random direction

		else if (iRoll <= 90)
			{
			bMisfire = true;
			pSource->OnDeviceFailure(pDevice, failWeaponMisfire);
			}

		//	10% of the time, the shot explodes

		else
			{
			bExplosion = true;
			bFireSuppressed = true;
			pSource->OnDeviceFailure(pDevice, failWeaponExplosion);
			}
		}

	//	Deal with counter

	switch (m_Counter)
		{
		case cntTemperature:
			{
			if (pDevice->GetTemperature() >= OVERHEAT_TEMP)
				{
				if (pDevice->GetTemperature() >= MAX_TEMP)
					{
					pSource->OnDeviceFailure(pDevice, failWeaponJammed);
					return;
					}
				else
					{
					int iRoll = mathRandom(1, 100);

					//	25% of the time, everything works OK

					if (iRoll <= 25)
						;

					//	25% of the time, the weapon jams

					else if (iRoll <= 50)
						{
						pSource->OnDeviceFailure(pDevice, failWeaponJammed);
						return;
						}

					//	25% of the time, the weapon is disabled

					else if (iRoll <= 75)
						{
						pSource->DisableDevice(pDevice);
						return;
						}

					//	25% of the time, the weapon is damaged

					else
						{
						pSource->DamageDevice(pDevice, failDeviceOverheat);
						}
					}
				}

			//	Update temperature

			pDevice->IncTemperature(m_iCounterActivate);
			pSource->OnComponentChanged(comDeviceCounter);
			break;
			}

		case cntCapacitor:
			{
			//	If we don't have enough capacitor power, then we can't fire

			if (pDevice->GetTemperature() < m_iCounterActivate)
				return;

			//	Consume capacitor

			pDevice->IncTemperature(m_iCounterActivate);
			pSource->OnComponentChanged(comDeviceCounter);
			break;
			}
		}

	//	Figure out the speed of the shot

	Metric rSpeed = pShot->GetInitialSpeed();

	//	Compute the direction of the shot

	if (iFireAngle == -1)
		{
		if (CanRotate(pDevice))
			{
			//	If we've got a target, then calculate the fire angle

			if (pTarget)
				{
				//	Get the position and velocity of the target
				CVector vTarget = pTarget->GetPos() - vSource;
				CVector vTargetVel = pTarget->GetVel() - pSource->GetVel();

				//	Figure out which direction to fire in

				Metric rTimeToIntercept = CalcInterceptTime(vTarget, vTargetVel, rSpeed);
				CVector vInterceptPoint = vTarget + vTargetVel * rTimeToIntercept;
				iFireAngle = VectorToPolar(vInterceptPoint, NULL);

				//	If this is a directional weapon make sure we are in-bounds

				int iMinFireAngle, iMaxFireAngle;
				if (IsDirectional(pDevice, &iMinFireAngle, &iMaxFireAngle))
					{
					int iMin = (pSource->GetRotation() + iMinFireAngle) % 360;
					int iMax = (pSource->GetRotation() + iMaxFireAngle) % 360;

					if (iMin < iMax)
						{
						if (iFireAngle < iMin)
							iFireAngle = iMin;
						else if (iFireAngle > iMax)
							iFireAngle = iMax;
						}
					else
						{
						if (iFireAngle < iMin && iFireAngle > iMax)
							iFireAngle = iMin;
						}
					}

				//	Remember the fire angle (we need it later if this is a continuous
				//	fire device)

				pDevice->SetFireAngle(iFireAngle);
				}

			//	Otherwise, we assume that our caller already set the fire angle

			else
				{
				//	-1 means "just shoot straight". We use this for the player ship
				//	when it has no target

				if (pDevice->GetFireAngle() == -1)
					iFireAngle = (pSource->GetRotation() + pDevice->GetRotation()) % 360;
				else
					iFireAngle = (pDevice->GetFireAngle() % 360);
				}
			}
		else
			iFireAngle = (pSource->GetRotation() + pDevice->GetRotation()) % 360;
		}

	//	If this is a misfire, adjust the angle

	if (bMisfire)
		iFireAngle = (iFireAngle + mathRandom(-60, 60) + 360) % 360;

	//	Figure out how many shots to create

	switch (m_Configuration)
		{
		case ctDual:
			{
			//	Compute a normal perpendicular to the direction of fire

			CVector Perp = PolarToVector(iFireAngle, (g_KlicksPerPixel * g_DualShotSeparation));
			Perp = Perp.Perpendicular();

			//	Create two shots

			iShotCount = 2;
			ShotPos[0] = vSource + Perp;
			ShotVel[0] = pSource->GetVel() + PolarToVector(iFireAngle, rSpeed);
			ShotDir[0] = iFireAngle;

			ShotPos[1] = vSource - Perp;
			ShotVel[1] = ShotVel[0];
			ShotDir[1] = iFireAngle;

			break;
			}

		case ctDualAlternating:
			{
			//	Figure out our polarity

			int iPolarity = GetAlternatingPos(pDevice);

			//	Compute a normal perpendicular to the direction of fire

			CVector Perp = PolarToVector(iFireAngle, (g_KlicksPerPixel * g_DualShotSeparation));
			Perp = Perp.Perpendicular();

			//	Create a shot either from the left or from the right depending
			//	on the continuous shot variable.

			iShotCount = 1;
			ShotVel[0] = pSource->GetVel() + PolarToVector(iFireAngle, rSpeed);
			ShotDir[0] = iFireAngle;

			if (iPolarity)
				{
				ShotPos[0] = vSource + Perp;
				iPolarity = 0;
				}
			else
				{
				ShotPos[0] = vSource - Perp;
				iPolarity = 1;
				}

			SetAlternatingPos(pDevice, iPolarity);

			break;
			}

		case ctWall:
			{
			//	Compute a normal perpendicular to the direction of fire

			CVector Perp = PolarToVector(iFireAngle, (g_KlicksPerPixel * g_DualShotSeparation));
			Perp = Perp.Perpendicular();

			//	Create five shots

			iShotCount = 0;
			CVector vVel = pSource->GetVel() + PolarToVector(iFireAngle, rSpeed);
			for (i = -2; i <= 2; i++)
				{
				ShotPos[iShotCount] = vSource + ((Metric)i * Perp);
				ShotVel[iShotCount] = vVel;
				ShotDir[iShotCount] = iFireAngle;
				iShotCount++;
				}

			break;
			}

		case ctSpread5:
			{
			//	Shots at +2 and -2 degrees

			ShotPos[iShotCount] = vSource;
			ShotVel[iShotCount] = pSource->GetVel() + PolarToVector((iFireAngle + 2) % 360, rSpeed);
			ShotDir[iShotCount] = (iFireAngle + 2) % 360;
			iShotCount++;

			ShotPos[iShotCount] = vSource;
			ShotVel[iShotCount] = pSource->GetVel() + PolarToVector((iFireAngle + 358) % 360, rSpeed);
			ShotDir[iShotCount] = (iFireAngle + 358) % 360;
			iShotCount++;

			//	Fallthrough!
			}

		case ctSpread3:
			{
			//	Shot at 0 degrees

			ShotPos[iShotCount] = vSource;
			ShotVel[iShotCount] = pSource->GetVel() + PolarToVector(iFireAngle, rSpeed);
			ShotDir[iShotCount] = iFireAngle;
			iShotCount++;

			//	Fallthrough!
			}

		case ctSpread2:
			{
			//	Shots at +5 and -5 degrees

			ShotPos[iShotCount] = vSource;
			ShotVel[iShotCount] = pSource->GetVel() + PolarToVector((iFireAngle + 5) % 360, rSpeed);
			ShotDir[iShotCount] = (iFireAngle + 5) % 360;
			iShotCount++;

			ShotPos[iShotCount] = vSource;
			ShotVel[iShotCount] = pSource->GetVel() + PolarToVector((iFireAngle + 355) % 360, rSpeed);
			ShotDir[iShotCount] = (iFireAngle + 355) % 360;
			iShotCount++;

			break;
			}

		case ctCustom:
			{
			iShotCount = Min(iMaxShotCount, m_iConfigCount);
			for (i = 0; i < iShotCount; i++)
				{
				ShotPos[i] = vSource + PolarToVector((iFireAngle + m_pConfig[i].iPosAngle) % 360, m_pConfig[i].rPosRadius);
				ShotDir[i] = (360 + iFireAngle + m_pConfig[i].Angle.Roll()) % 360;
				ShotVel[i] = pSource->GetVel() + PolarToVector(ShotDir[i], rSpeed);
				}
			break;
			}

		default:
			{
			iShotCount = 1;
			ShotPos[0] = vSource;
			ShotVel[0] = pSource->GetVel() + PolarToVector(iFireAngle, rSpeed);
			ShotDir[0] = iFireAngle;
			}
		}

	//	If we're independently targeted, then compute targets for the remaining shots

	if (m_bMIRV && iShotCount > 1)
		{
		CIntArray TargetList;
		int iFound = pSource->GetNearestVisibleEnemies(iShotCount, MAX_TARGET_RANGE, &TargetList);

		int j = 1;
		for (i = 0; i < iFound && j < iShotCount; i++)
			{
			CSpaceObject *pNewTarget = (CSpaceObject *)TargetList.GetElement(i);
			if (pNewTarget != pTarget)
				{
				//	Calculate direction to fire in

				CVector vTarget = pNewTarget->GetPos() - ShotPos[j];
				CVector vTargetVel = pNewTarget->GetVel() - pSource->GetVel();
				Metric rTimeToIntercept = CalcInterceptTime(vTarget, vTargetVel, rSpeed);
				CVector vInterceptPoint = vTarget + pNewTarget->GetVel() * rTimeToIntercept;

				ShotDir[j] = VectorToPolar(vInterceptPoint, NULL);
				ShotVel[j] = pSource->GetVel() + PolarToVector(ShotDir[j], rSpeed);

				j++;
				}
			}
		}

	//	If the shot requires ammo, check to see that the source has
	//	enough ammo for all shots

	if (pShot->m_pAmmoType && bConsumeItems)
		{
		CItemListManipulator ItemList(pSource->GetItemList());
		CItem Item(pShot->m_pAmmoType, iShotCount);
		if (!ItemList.SetCursorAtItem(Item))
			return;

		//	If we've exhausted our ammunition, remember to
		//	select the next variant

		if (ItemList.GetItemAtCursor().GetCount() == 1)
			bNextVariant = true;

		ItemList.DeleteAtCursor(1);

		//	Remember to tell the ship that we've consumed items

		if (retbConsumedItems)
			*retbConsumedItems = true;
		}
	else if (m_bCharges && bConsumeItems)
		{
		if (pDevice->GetCharges(pSource) <= 0)
			return;

		pDevice->IncCharges(pSource, -1);

		if (retbConsumedItems)
			*retbConsumedItems = true;
		}

	//	Create barrel flash effect

	if (!bFireSuppressed && pShot->m_pFireEffect)
		{
		for (i = 0; i < iShotCount; i++)
			{
			IEffectPainter *pPainter = pShot->m_pFireEffect->CreatePainter();

			pPainter->SetDirection(ShotDir[i]);
			pSource->AddEffect(pPainter, ShotPos[i]);
			}
		}

	//	Create all the shots

	CVector vRecoil;
	if (!bFireSuppressed)
		{
		for (i = 0; i < iShotCount; i++)
			{
			CSpaceObject *pNewObj;

			pSource->GetSystem()->CreateWeaponFire(pShot,
					pDevice->GetBonus(),
					killedByDamage,
					CDamageSource(pSource),
					ShotPos[i],
					ShotVel[i],
					ShotDir[i],
					pTarget,
					&pNewObj);

			//	If this shot was created by automated weapon fire, then set flag

			if (pDevice->IsAutomatedWeapon())
				pNewObj->SetAutomatedWeapon();

			//	Add up all the shot directions to end up with a recoil dir

			if (m_iRecoil)
				vRecoil = vRecoil + PolarToVector(ShotDir[i], 1.0);
			}
		}

	//	Sound effect

	if (!bFireSuppressed && pShot->m_iFireSound != -1)
		g_pUniverse->PlaySound(pSource, pShot->m_iFireSound);

	//	Recoil

	if (!bFireSuppressed && m_iRecoil)
		{
		CVector vAccel = vRecoil.Normal() * (Metric)(-10 * m_iRecoil * m_iRecoil);
		pSource->Accelerate(vAccel, g_MomentumConstant);
		pSource->ClipSpeed(pSource->GetMaxSpeed());
		}

	//	Switch to the next variant if necessary

	if (bNextVariant)
		{
		if (!SelectNextVariant(pSource, pDevice))
			SelectFirstVariant(pSource, pDevice);
		}

	//	Create an explosion if weapon damage

	if (bExplosion)
		{
		CSpaceObject::DamageResults iResult;

		DamageDesc Damage = pShot->m_Damage;
		Damage.SetCause(killedByWeaponMalfunction);

		iResult = pSource->Damage(pSource,
				vSource,
				(pDevice->GetPosAngle() + 360 + mathRandom(0, 30) - 15) % 360,
				Damage);

		if (iResult == CSpaceObject::damageDestroyed 
				|| iResult == CSpaceObject::damagePassthrough)
			*retbSourceDestroyed = true;
		}
	}

int CWeaponClass::GetActivateDelay (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	GetActivateDelay
//
//	Returns the number of ticks between shots

	{
	if (pDevice)
		return pDevice->GetMods().GetEnhancedRate(m_iFireRate);
	else
		return m_iFireRate;
	}

int CWeaponClass::GetAmmoVariant (CItemType *pItem)

//	GetAmmoVariant
//
//	Returns the variant that fires the given ammo (or -1 if the weapons
//	does not fire the ammo)

	{
	int i;

	for (i = 0; i < m_iShotVariants; i++)
		{
		if (m_pShotData[i].GetAmmoType() == pItem)
			return i;
		}

	return -1;
	}

ItemCategories CWeaponClass::GetCategory (void)

//	GetCategory
//
//	Returns the weapon class category

	{
	if (m_bLauncher)
		return itemcatLauncher;
	else
		return itemcatWeapon;
	}

int CWeaponClass::GetCounter (CInstalledDevice *pDevice, CounterTypes *retiType)

//	GetCounter
//
//	Return counter

	{
	if (m_Counter == cntNone)
		return 0;

	//	Return the type

	if (retiType)
		*retiType = m_Counter;

	//	If we're a capacitor, then don't show the counter if we are full

	if (m_Counter == cntCapacitor && pDevice->GetTemperature() >= MAX_COUNTER)
		return 0;

	//	Otherwise, return the current value

	return pDevice->GetTemperature();
	}

int CWeaponClass::GetAlternatingPos (CInstalledDevice *pDevice)
	{
	if (m_Configuration == ctDualAlternating)
		return (int)(short)LOWORD(pDevice->GetData());
	else
		return 0;
	}

int CWeaponClass::GetContinuousFire (CInstalledDevice *pDevice) 
	{
	if (!m_bCharges 
			&& m_Configuration != ctDualAlternating)
		return (int)(short)LOWORD(pDevice->GetData());
	else
		return 0;
	}

int CWeaponClass::GetCurrentVariant (CInstalledDevice *pDevice)
	{
	return (int)(short)HIWORD(pDevice->GetData()); 
	}

const DamageDesc *CWeaponClass::GetDamageDesc (CInstalledDevice *pDevice)

//	GetDamageDesc
//
//	Returns the kind of damage caused by this weapon

	{
	CWeaponFireDesc *pShot = GetSelectedShotData(pDevice);
	if (pShot == NULL)
		return NULL;
	else
		return &pShot->m_Damage;
	}

int CWeaponClass::GetDamageType (CInstalledDevice *pDevice)

//	GetDamageType
//
//	Returns the kind of damage caused by this weapon

	{
	if (pDevice)
		{
		CWeaponFireDesc *pShot = GetSelectedShotData(pDevice);
		if (pShot == NULL)
			return -1;
		else
			return pShot->m_Damage.GetDamageType();
		}
	else
		{
		if (m_iShotVariants > 0)
			return m_pShotData[0].m_Damage.GetDamageType();
		else
			return -1;
		}
	}

Metric CWeaponClass::GetMaxEffectiveRange (CSpaceObject *pSource, CInstalledDevice *pDevice, CSpaceObject *pTarget)

//	GetMaxEffectiveRange
//
//	Returns the greatest range at which the weapon is still
//	effective.

	{
	if (pSource)
		{
		CWeaponFireDesc *pShot = GetSelectedShotData(pDevice);
		if (pShot == NULL)
			return 0.0;
		else
			{
			if (pTarget && !pTarget->CanMove())
				{
				Metric rSpeed = (pShot->GetRatedSpeed() + pShot->m_rMaxMissileSpeed) / 2;
				return rSpeed * (pShot->GetMaxLifetime() * 90 / 100);
				}
			else
				return pShot->m_rMaxEffectiveRange;
			}
		}
	else
		{
		if (m_iShotVariants > 0)
			return m_pShotData[0].m_rMaxEffectiveRange;
		else
			return 0.0;
		}
	}

int CWeaponClass::GetPowerRating (const CItem *pItem)

//	GetPowerRating
//
//	Returns the rated power

	{
	CItemEnhancement Mods;
	if (pItem)
		Mods = pItem->GetMods();

	int iPower = m_iPowerUse;

	if (Mods)
		iPower = iPower * Mods.GetPowerAdj() / 100;

	return iPower;
	}

CString GetReferenceFireRate (int iFireRate)
	{
	int iRate = (int)((10 * g_TicksPerSecond / iFireRate) + 0.5);
	if (iRate < 10)
		return CONSTLIT(" @ <1/second");
	else if ((iRate % 10) == 0)
		return strPatternSubst(CONSTLIT(" @ %d/second"), iRate / 10);
	else
		return strPatternSubst(CONSTLIT(" @ %d.%d/second"), iRate / 10, iRate % 10);
	}

CString CWeaponClass::GetReference (const CItem *pItem, CSpaceObject *pInstalled)

//	GetReference
//
//	Returns a string that describes the basic attributes
//	of this weapon.
//
//	Example:
//
//		laser 1-4 (x2); tracking; 100MW

	{
	CString sReference;
	CItemEnhancement Mods;
	if (pItem)
		Mods = pItem->GetMods();

	//	If we don't know about the item, return only basic info

	if (pItem && !GetItemType()->ShowReference())
		return CDeviceClass::GetReference(pItem, pInstalled);

	//	Compute the damage string and special string

	bool bCommaNeeded = false;

	if (m_iShotVariants != 1)
		{
		sReference = CONSTLIT("missile weapon");
		bCommaNeeded = true;

		sReference.Append(GetReferenceFireRate(Mods.GetEnhancedRate(m_iFireRate)));
		}
	else
		{
		//	Get the damage

		CWeaponFireDesc *pShot = &m_pShotData[0];
		DamageDesc Damage = pShot->m_Damage;

		//	Modify the damage based on any enhancements that the ship may have

		if (pInstalled && pItem)
			{
			CInstalledDevice *pDevice = pInstalled->FindDevice(*pItem);
			if (pDevice)
				Damage.AddBonus(pDevice->GetBonus());
			}
		else
			Damage.AddBonus(Mods.GetDamageBonus());

		//	Get description

		CString sDamage = Damage.GetDesc();

		//	Add the multiplier

		int iMult = CalcConfigurationMultiplier(pShot);
		if (iMult > 1)
			sDamage.Append(strPatternSubst(CONSTLIT(" (x%d)"), iMult));

		if (pShot->m_iFireType == ftParticles)
			sDamage.Append(CONSTLIT(" (max)"));

		if (bCommaNeeded)
			sReference.Append(CONSTLIT("; "));

		sReference.Append(sDamage);
		bCommaNeeded = true;

		//	Compute fire rate

		sReference.Append(GetReferenceFireRate(Mods.GetEnhancedRate(m_iFireRate)));

		//	Compute special abilities. Start with tracking

		if (pShot->m_iManeuverability > 0)
			sReference.Append(CONSTLIT("; tracking"));

		//	Radiation

		if (Damage.GetRadiationDamage() > 0)
			sReference.Append(CONSTLIT("; radiation"));

		//	EMP

		if (Damage.GetEMPDamage() > 0)
			sReference.Append(CONSTLIT("; EMP"));

		//	Area of effect

		if (pShot->m_iFireType == ftArea
				|| (pShot->HasFragments() && pShot->GetFirstFragment()->pDesc->m_iFireType == ftArea))
			sReference.Append(CONSTLIT("; area weapon"));

		//	Fragmentation

		if (pShot->HasFragments())
			{
			int iFragments = 0;
			CWeaponFireDesc::SFragmentDesc *pFragDesc = pShot->GetFirstFragment();
			while (pFragDesc)
				{
				iFragments += pFragDesc->Count.GetAveValue();
				pFragDesc = pFragDesc->pNext;
				}

			if (iFragments > 1)
				sReference.Append(strPatternSubst(CONSTLIT("; fragmentation (x%d)"), iFragments));
			}
		}

	//	Power required

	if (bCommaNeeded)
		sReference.Append(CONSTLIT("; "));

	sReference.Append(GetReferencePower(pItem));
	bCommaNeeded = true;

	return sReference;
	}

CWeaponFireDesc *CWeaponClass::GetSelectedShotData (CInstalledDevice *pDevice)

//	GetSelectedShotData
//
//	Returns the currently selected shot data (or NULL if none)

	{
	int iSelection = GetCurrentVariant(pDevice);
	if (iSelection != -1 && iSelection < m_iShotVariants)
		return &m_pShotData[iSelection];
	else
		return NULL;
	}

void CWeaponClass::GetSelectedVariantInfo (CSpaceObject *pSource, 
										   CInstalledDevice *pDevice,
										   CString *retsLabel,
										   int *retiAmmoLeft,
										   CItemType **retpType)

//	GetSelectedVariantInfo
//
//	Returns information about the selected variant

	{
	CItemListManipulator ItemList(pSource->GetItemList());
	CWeaponFireDesc *pShot = GetSelectedShotData(pDevice);
	if (pShot)
		{
		if (pShot->m_pAmmoType == NULL)
			{
			if (retsLabel)
				*retsLabel = CString();
			if (retiAmmoLeft)
				{
				if (m_bCharges)
					*retiAmmoLeft = pDevice->GetCharges(pSource);
				else
					*retiAmmoLeft = -1;
				}
			if (retpType)
				*retpType = GetItemType();
			}
		else
			{
			CItem Item(pShot->m_pAmmoType, 1);

			if (retiAmmoLeft)
				{
				if (ItemList.SetCursorAtItem(Item))
					*retiAmmoLeft = ItemList.GetItemAtCursor().GetCount();
				else
					*retiAmmoLeft = 0;
				}

			if (retsLabel)
				{
				if (GetCategory() == itemcatLauncher)
					*retsLabel = Item.GetNounPhrase(nounCapitalize);
				else
					*retsLabel = CString();
				}

			if (retpType)
				*retpType = pShot->m_pAmmoType;
			}
		}
	else
		{
		if (retsLabel)
			*retsLabel = CString();
		if (retiAmmoLeft)
			*retiAmmoLeft = 0;
		if (retpType)
			*retpType = NULL;
		}
	}

int CWeaponClass::GetValidVariantCount (CSpaceObject *pSource, CInstalledDevice *pDevice)

//	GetValidVariantCount
//
//	Returns the number of valid variants for this weapons

	{
	int iCount = 0;

	for (int i = 0; i < m_iShotVariants; i++)
		{
		if (VariantIsValid(pSource, m_pShotData[i]))
			iCount++;
		}

	return iCount;
	}

int CWeaponClass::GetWeaponEffectiveness (CSpaceObject *pSource, CInstalledDevice *pDevice, CSpaceObject *pTarget)

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
	int iScore = 0;

	CWeaponFireDesc *pShot = GetSelectedShotData(pDevice);
	if (pShot == NULL)
		return -100;

	//	If we don't enough ammo, clearly we will not be effective

	if (pShot->m_pAmmoType)
		{
		CItemListManipulator ItemList(pSource->GetItemList());
		CItem Item(pShot->m_pAmmoType, 1);
		if (!ItemList.SetCursorAtItem(Item))
			return -100;
		}

	//	Check our state

	switch (m_Counter)
		{
		//	If we're overheating, we will not be effective

		case cntTemperature:
			if (pDevice->IsWaiting() && pDevice->GetTemperature() > 0)
				return -100;

			if (pDevice->GetTemperature() + m_iCounterActivate >= MAX_COUNTER)
				{
				pDevice->SetWaiting(true);
				return -100;
				}

			pDevice->SetWaiting(false);
			break;

		//	If our capacitor is discharged, we will not be effective

		case cntCapacitor:
			if (pDevice->IsWaiting() && pDevice->GetTemperature() < MAX_COUNTER)
				return -100;

			if (pDevice->GetTemperature() < m_iCounterActivate)
				{
				pDevice->SetWaiting(true);
				return -100;
				}

			pDevice->SetWaiting(false);
			break;
		}

	//	If the weapon has EMP damage and the target is not paralysed then
	//	this is very effective.

	if (pTarget && pShot->m_Damage.GetEMPDamage() > 0)
		{
		if (pTarget->IsParalyzed())
			iScore -= 50;
		else
			iScore += 100;
		}

	//	If the weapon has blinding damage and the target is not blind then
	//	this is very effective

	if (pTarget && pShot->m_Damage.GetBlindingDamage() > 0)
		{
		if (pTarget->IsBlind())
			iScore -= 50;
		else
			iScore += 100;
		}

	//	Score

	return iScore;
	}

bool CWeaponClass::IsAreaWeapon (CSpaceObject *pSource, CInstalledDevice *pDevice)

//	IsAreaWeapon
//
//	Is this a weapon with an area of effect

	{
	CWeaponFireDesc *pShot = GetSelectedShotData(pDevice);
	if (pShot == NULL)
		return false;

	if (pShot->m_iFireType == ftArea)
		return true;

	if (pShot->HasFragments() && pShot->GetFirstFragment()->pDesc->m_iFireType == ftArea)
		return true;

	return false;
	}

bool CWeaponClass::IsDirectional (CInstalledDevice *pDevice, int *retiMinFireArc, int *retiMaxFireArc)

//	IsDirectional
//
//	Returns TRUE if the weapon can turn but is not omni

	{
	//	The device slot overrides the weapon. If the device slot is directional, then
	//	the weapon is directional. If the device slot is omni directional, then the
	//	weapon is omnidirectional.

	if (pDevice)
		{
		if (pDevice->IsDirectional())
			{
			if (retiMinFireArc)
				*retiMinFireArc = pDevice->GetMinFireAngle();
			if (retiMaxFireArc)
				*retiMaxFireArc = pDevice->GetMaxFireAngle();
			return true;
			}
		else if (pDevice->IsOmniDirectional())
			return false;
		else
			{
			if (retiMinFireArc)
				*retiMinFireArc = m_iMinFireArc;
			if (retiMaxFireArc)
				*retiMaxFireArc = m_iMaxFireArc;
			return (m_iMinFireArc != m_iMaxFireArc);
			}
		}
	else
		{
		if (retiMinFireArc)
			*retiMinFireArc = m_iMinFireArc;
		if (retiMaxFireArc)
			*retiMaxFireArc = m_iMaxFireArc;
		return (m_iMinFireArc != m_iMaxFireArc);
		}
	}

bool CWeaponClass::IsOmniDirectional (CInstalledDevice *pDevice)

//	IsOmniDirectional
//
//	Returns TRUE if the weapon is omnidirectional (not limited)

	{
	//	The device slot overrides the weapon. If the device slot is directional, then
	//	the weapon is directional. If the device slot is omni directional, then the
	//	weapon is omnidirectional.

	if (pDevice)
		return ((pDevice->IsOmniDirectional() || (m_bOmnidirectional && (m_iMinFireArc == m_iMaxFireArc)))
				&& !pDevice->IsDirectional());
	else
		return (m_bOmnidirectional && (m_iMinFireArc == m_iMaxFireArc));
	}

bool CWeaponClass::IsVariantSelected (CSpaceObject *pSource, CInstalledDevice *pDevice)

//	IsVariantSelected
//
//	Returns TRUE if we've selected some variant (i.e., we haven't selected 0xffff)

	{
	return (GetCurrentVariant(pDevice) != -1);
	}

bool CWeaponClass::IsWeaponAligned (CSpaceObject *pShip, 
									CInstalledDevice *pDevice, 
									CSpaceObject *pTarget, 
									int *retiAimAngle, 
									int *retiFireAngle)

//	IsWeaponAligned
//
//	Return TRUE if weapon is aligned on target.
//
//	Note: If the weapon is invalid, we return an aim angle of -1

	{
	CWeaponFireDesc *pShot = GetSelectedShotData(pDevice);
	if (pShot == NULL)
		{
		if (retiAimAngle)
			*retiAimAngle = -1;
		if (retiFireAngle)
			*retiFireAngle = -1;
		return false;
		}

	ASSERT(pTarget);

	//	Compute source

	CVector vSource = pDevice->GetPos(pShip);

	//	We need to calculate the intercept solution.

	Metric rWeaponSpeed = pShot->GetRatedSpeed();
	CVector vTarget = pTarget->GetPos() - vSource;
	CVector vTargetVel = pTarget->GetVel() - pShip->GetVel();

	//	Figure out which direction to fire in

	Metric rDist;
	Metric rTimeToIntercept = CalcInterceptTime(vTarget, vTargetVel, rWeaponSpeed, &rDist);
	CVector vInterceptPoint = vTarget + vTargetVel * rTimeToIntercept;
	int iAim = VectorToPolar(vInterceptPoint, NULL);
	if (retiAimAngle)
		*retiAimAngle = iAim;

	//	Omnidirectional weapons are always aligned

	if (IsOmniDirectional(pDevice))
		{
		if (retiFireAngle)
			*retiFireAngle = iAim;
		return true;
		}

	int iFacingAngle = (pShip->GetRotation() + pDevice->GetRotation()) % 360;

	//	Tracking weapons are always aligned

	if (pShot->m_iManeuverability > 0)
		{
		if (retiFireAngle)
			*retiFireAngle = iFacingAngle;
		return true;
		}

	//	Area weapons are always aligned

	if (pShot->m_iFireType == ftArea)
		{
		if (retiFireAngle)
			*retiFireAngle = iFacingAngle;
		return true;
		}

	//	If this is a directional weapon, figure out whether the target
	//	is in the fire arc

	int iMinFireAngle, iMaxFireAngle;
	if (IsDirectional(pDevice, &iMinFireAngle, &iMaxFireAngle))
		{
		int iMin = (pShip->GetRotation() + iMinFireAngle) % 360;
		int iMax = (pShip->GetRotation() + iMaxFireAngle) % 360;

		if (retiFireAngle)
			*retiFireAngle = iAim;

		if (iMin < iMax)
			{
			//	e.g., iMin == 0; iMax == 180

			return (iAim >= iMin && iAim <= iMax);
			}
		else
			{
			//	e.g., iMin == 315; iMax == 45

			return (iAim >= iMin || iAim <= iMax);
			}
		}

	//	Figure out how far off we are from the direction that we
	//	want to fire in.

	if (iFacingAngle < iAim)
		iFacingAngle += 360;

	int iAimOffset = 180 - abs((iFacingAngle - iAim) - 180);

	//	Figure out our aim tolerance

	int iAimTolerance;
	switch (m_Configuration)
		{
		case ctDual:
		case ctDualAlternating:
			iAimTolerance = 5;
			break;

		case ctSpread2:
			iAimTolerance = 10;
			break;

		case ctSpread3:
		case ctSpread5:
			iAimTolerance = 15;
			break;

		case ctWall:
			iAimTolerance = 30;
			break;

		case ctCustom:
			iAimTolerance = m_iConfigAimTolerance;
			break;

		default:
			iAimTolerance = 3;
		}

	//	Fire angle

	if (retiFireAngle)
		*retiFireAngle = iFacingAngle;

	//	Compute the angular size of the target

	CVector vUR;
	CVector vLL;
	pTarget->GetBoundingRect(&vUR, &vLL);
	Metric rSize = (vUR.GetX() - vLL.GetX());
	int iHalfAngularSize = (int)(30 * rSize / rDist);

	//	If we're facing in the direction that we want to fire, 
	//	then we're aligned...

	if (iAimOffset <= Max(iAimTolerance, iHalfAngularSize))
		return true;

	//	Otherwise, we're not and we need to return the aim direction

	else
		return false;
	}

ALERROR CWeaponClass::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Done loading all design elements

	{
	ALERROR error;

	for (int i = 0; i < m_iShotVariants; i++)
		if (error = m_pShotData[i].OnDesignLoadComplete(Ctx))
			return error;

	return NOERROR;
	}

CEffectCreator *CWeaponClass::OnFindEffectCreator (const CString &sUNID)

//	OnFindEffectCreator
//
//	Find the effect. We start after the weapon class UNID.

	{
	//	We start after the weapon class UNID

	char *pPos = sUNID.GetASCIIZPointer();
	if (*pPos != '/')
		return NULL;

	pPos++;

	//	Parse the variant

	int iOrdinal = strParseInt(pPos, 0, &pPos, NULL);
	if (iOrdinal < 0 || iOrdinal >= GetVariantCount())
		return NULL;

	//	Now ask the weapon fire desc to parse the remaining UNID

	CWeaponFireDesc *pDesc = GetVariant(iOrdinal);
	return pDesc->FindEffectCreator(CString(pPos));
	}

bool CWeaponClass::RequiresItems (void)

//	RequiresItems
//
//	Returns TRUE if this weapon requires ammo

	{
	if (m_iShotVariants == 0)
		return false;
	else if (m_iShotVariants == 1)
		return (m_pShotData[0].m_pAmmoType != NULL);
	else
		return true;
	}

bool CWeaponClass::SelectFirstVariant (CSpaceObject *pSource, CInstalledDevice *pDevice)

//	SelectFirstVariant
//
//	Selects the first valid variant for this weapon

	{
	SetCurrentVariant(pDevice, -1);
	return SelectNextVariant(pSource, pDevice);
	}

bool CWeaponClass::SelectNextVariant (CSpaceObject *pSource, CInstalledDevice *pDevice)

//	SelectNextVariant
//
//	Selects the next valid variant for this weapon

	{
	int iStart = GetCurrentVariant(pDevice);
	if (iStart == -1)
		iStart = 0;
	else
		iStart++;

	//	Loop over all variants

	int iSelection = -1;
	if (m_iShotVariants > 0)
		{
		for (int i = iStart; i < (m_iShotVariants + iStart + 1); i++)
			{
			//	If this variant is valid, then we're done

			if (VariantIsValid(pSource, m_pShotData[i % m_iShotVariants]))
				{
				iSelection = (i % m_iShotVariants);
				break;
				}
			}
		}

	//	If we found a selection, then select the variant

	SetCurrentVariant(pDevice, iSelection);
	return (iSelection != -1);
	}

void CWeaponClass::SetAlternatingPos (CInstalledDevice *pDevice, int iAlternatingPos)

//	SetAlternatingPos
//
//	Sets the alternating position

	{
	pDevice->SetData((pDevice->GetData() & 0xFFFF0000) | (WORD)(short)iAlternatingPos);
	}

void CWeaponClass::SetContinuousFire (CInstalledDevice *pDevice, int iContinuous)

//	SetContinuousFire
//
//	Sets the continuous fire counter for the device

	{
	pDevice->SetData((pDevice->GetData() & 0xFFFF0000) | (WORD)(short)iContinuous);
	}

void CWeaponClass::SetCurrentVariant (CInstalledDevice *pDevice, int iVariant)

//	SetCurrentVariant
//
//	Sets the current variant for the device

	{
	pDevice->SetData((((DWORD)(WORD)(short)iVariant) << 16) | LOWORD(pDevice->GetData()));
	}

void CWeaponClass::Update (CInstalledDevice *pDevice, CSpaceObject *pSource, int iTick, bool *retbSourceDestroyed, bool *retbConsumedItems)

//	Update
//
//	Update

	{
	if (retbConsumedItems)
		*retbConsumedItems = false;
	if (retbSourceDestroyed)
		*retbSourceDestroyed = false;

	//	Change counter on update

	if (IsCounterEnabled() && (iTick % m_iCounterUpdateRate) == 0)
		{
		if (m_iCounterUpdate > 0)
			{
			if (pDevice->GetTemperature() < MAX_COUNTER)
				{
				pDevice->IncTemperature(Min(m_iCounterUpdate, MAX_COUNTER - pDevice->GetTemperature()));
				pSource->OnComponentChanged(comDeviceCounter);
				}
			}
		else
			{
			if (pDevice->GetTemperature() > 0)
				{
				pDevice->IncTemperature(Max(m_iCounterUpdate, -pDevice->GetTemperature()));
				pSource->OnComponentChanged(comDeviceCounter);
				}
			}
		}

	//	Done if we're disabled

	if (!pDevice->IsEnabled())
		return;

	//	See if we continue to fire

	int iContinuous = GetContinuousFire(pDevice);
	if (iContinuous > 0)
		{
		CWeaponFireDesc *pShot = GetSelectedShotData(pDevice);
		if (pShot)
			{
			bool bSourceDestroyed;

			FireWeapon(pDevice, pShot, pSource, NULL, -1, false, &bSourceDestroyed, retbConsumedItems);

			if (bSourceDestroyed)
				{
				if (retbSourceDestroyed)
					*retbSourceDestroyed = true;
				return;
				}
			}

		iContinuous--;
		SetContinuousFire(pDevice, iContinuous);
		}
	}

bool CWeaponClass::ValidateSelectedVariant (CSpaceObject *pSource, CInstalledDevice *pDevice)

//	ValidateSelectedVariant
//
//	If the selected variant is valid, then it returns TRUE. Otherwise,
//	it selects a different valid variant. If not valid variants are found,
//	it returns FALSE

	{
	CWeaponFireDesc *pShot = GetSelectedShotData(pDevice);
	if (pShot && VariantIsValid(pSource, *pShot))
		return true;

	if (SelectNextVariant(pSource, pDevice))
		return true;

	if (SelectFirstVariant(pSource, pDevice))
		return true;

	return false;
	}

bool CWeaponClass::VariantIsValid (CSpaceObject *pSource, CWeaponFireDesc &ShotData)

//	VariantIsValid
//
//	Returns TRUE if the variant is valid

	{
	//	If we do not need ammo, then we're always valid

	if (ShotData.GetAmmoType() == NULL)
		return true;

	//	Otherwise, check to see if we have enough ammo

	CItemListManipulator ItemList(pSource->GetItemList());
	CItem Item(ShotData.GetAmmoType(), 1);
	if (ItemList.SetCursorAtItem(Item))
		return true;

	//	Not valid

	return false;
	}

