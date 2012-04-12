//	CShip.cpp
//
//	CShip class

#include "PreComp.h"


#define STR_G_SOURCE						(CONSTLIT("gSource"))
#define STR_G_ITEM							(CONSTLIT("gItem"))

static CObjectClass<CShip>g_Class(OBJID_CSHIP, NULL);

#define FUEL_CHECK_CYCLE					4
#define LIFESUPPORT_FUEL_USE_PER_CYCLE		1
#define ARMOR_UPDATE_CYCLE					10
#define GATE_ANIMATION_LENGTH				30
#define PARALYSIS_ARC_COUNT					5
#define SPACE_ENV_CYCLE						15

#define MAP_LABEL_X							10
#define MAP_LABEL_Y							(-6)

#define FUEL_GRACE_PERIOD					(30 * 30)

const Metric MAX_MANEUVER_DELAY	=			8.0;
const Metric MANEUVER_MASS_FACTOR =			0.4;

#define MAX_DELTA						(2.0 * g_KlicksPerPixel)
#define MAX_DELTA2						(MAX_DELTA * MAX_DELTA)
#define MAX_DELTA_VEL					(g_KlicksPerPixel / 2.0)
#define MAX_DELTA_VEL2					(MAX_DELTA_VEL * MAX_DELTA_VEL)
#define MAX_DISTANCE					(400 * g_KlicksPerPixel)

CShip::CShip (void) : CSpaceObject(&g_Class),
		m_Armor(sizeof(CInstalledArmor), 2),
		m_pDocked(NULL),
		m_pDriveDesc(NULL),
		m_pReactorDesc(NULL),
		m_pController(NULL),
		m_iPowerDrain(0),
		m_iMaxPower(0),
		m_iDeviceCount(0),
		m_Devices(NULL),
		m_pEncounterInfo(NULL),
		m_dwNameFlags(0),
		m_iFuelLeft(0)

//	CShip constructor

	{
	}

CShip::~CShip (void)

//	CShip destructor

	{
	if (m_pController)
		delete dynamic_cast<CObject *>(m_pController);

	if (m_Devices)
		delete [] m_Devices;
	}

bool CShip::AbsorbWeaponFire (CInstalledDevice *pWeapon)

//	AbsorbWeaponFire
//
//	Returns TRUE if another device on the ship (e.g., shields) prevent
//	the given weapon from firing

	{
	if (ShieldsAbsorbFire(pWeapon))
		{
		CEffectCreator *pEffect = g_pUniverse->FindEffectType(g_ShieldEffectUNID);
		if (pEffect)
			pEffect->CreateEffect(GetSystem(),
					this,
					pWeapon->GetPos(this),
					GetVel());

		return true;
		}
	else
		return false;
	}

void CShip::AddEnergyField (CEnergyFieldType *pType, int iLifeLeft)

//	AddEnergyField
//
//	Adds an energy field to the ship

	{
	m_EnergyFields.AddField(pType, iLifeLeft);

	//	Recalc bonuses, etc.

	CalcArmorBonus();
	CalcDeviceBonus();
	m_pController->OnWeaponStatusChanged();
	m_pController->OnArmorRepaired(-1);
	}

void CShip::Behavior (void)

//	Behavior
//
//	Implements behavior

	{
	if (!IsInGate())
		m_pController->Behavior(); 
	}

void CShip::CalcArmorBonus (void)

//	CalcArmorBonus
//
//	Check to see if all the armor segments for the ship are of the same class
//	Mark the m_fComplete flag appropriately for all armor segments.

	{
	int i;
	bool bComplete = true;

	//	Figure out if the armor set is complete

	CArmorClass *pClass = NULL;
	for (i = 0; i < GetArmorSectionCount(); i++)
		{
		CInstalledArmor *pArmor = GetArmorSection(i);

		if (pClass == NULL)
			pClass = pArmor->pArmorClass;
		else if (pArmor->pArmorClass != pClass)
			{
			bComplete = false;
			break;
			}
		}

	//	Compute

	int iStealth = stealthMax;
	for (i = 0; i < GetArmorSectionCount(); i++)
		{
		CInstalledArmor *pArmor = GetArmorSection(i);

		//	Set armor complete

		bool bOldComplete = pArmor->m_fComplete;
		pArmor->m_fComplete = bComplete;

		if (bOldComplete != bComplete)
			pArmor->pArmorClass->AddArmorComplete(pArmor);

		//	Compute stealth

		if (pArmor->pArmorClass->GetStealth() < iStealth)
			iStealth = pArmor->pArmorClass->GetStealth();
		}

	m_iStealth = iStealth;
	}

void CShip::CalcDeviceBonus (void)

//	CalcDeviceBonus
//
//	Calculate the appropriate bonus for all devices
//	(particularly weapons)

	{
	int i;

	for (i = 0; i < GetDeviceCount(); i++)
		if (!m_Devices[i].IsEmpty())
			{
			switch (m_Devices[i].GetCategory())
				{
				case itemcatLauncher:
				case itemcatWeapon:
					{
					int iBonus = 0;

					//	Add bonus from any enhancements

					iBonus += m_Devices[i].GetMods().GetDamageBonus();

					//	Other devices add a bonus as well

					int j;
					CSymbolTable Bonuses(FALSE, TRUE);
					for (j = 0; j < GetDeviceCount(); j++)
						if (i != j && !m_Devices[j].IsEmpty())
							{
							CString sBonusType;
							int iAdd = m_Devices[j].GetWeaponBonus(this, &m_Devices[i], &sBonusType);
							CObject *pDummy;
							if (iAdd != 0 
									&& Bonuses.Lookup(sBonusType, &pDummy) == ERR_NOTFOUND)
								{
								iBonus += iAdd;
								Bonuses.AddEntry(sBonusType, NULL);
								}
							}

					//	Energy fields add a bonus

					iBonus += m_EnergyFields.GetWeaponBonus(&m_Devices[i], this);

					//	Set the bonus

					m_Devices[i].SetBonus(iBonus);
					break;
					}

				case itemcatDrive:
					{
					if (m_Devices[i].IsEnabled())
						SetDriveDesc(m_Devices[i].GetDriveDesc(this));
					else
						SetDriveDesc(NULL);
					break;
					}

				case itemcatReactor:
					{
					m_pReactorDesc = m_Devices[i].GetReactorDesc(this);
					break;
					}
				default:
					break;
				}
			}

	//	Make sure we don't overflow fuel (in case we downgrade the reactor)

	if (!m_fOutOfFuel)
		m_iFuelLeft = Min(m_iFuelLeft, GetMaxFuel());
	}

int CShip::CalcMaxCargoSpace (void) const

//	CalcMaxCargoSpace
//
//	Returns the max cargo space for this ship (based on class and
//	additional devices)

	{
	//	Compute total cargo space. Start with the space specified
	//	by the class. Then see if any devices add to it.

	int iCargoSpace = m_pClass->GetCargoSpace();
	for (int i = 0; i < GetDeviceCount(); i++)
		if (!m_Devices[i].IsEmpty())
			iCargoSpace += m_Devices[i].GetClass()->GetCargoSpace();

	//	We never exceed the max cargo space for the class (even
	//	with extra devices)

	return Min(iCargoSpace, m_pClass->GetMaxCargoSpace());
	}

void CShip::CalcReactorStats (void)

//	CalcReactorStats
//
//	Computes power consumption and generation

	{
	int i;

	//	Calculate power generation

	m_iMaxPower = m_pReactorDesc->iMaxPower;

	//	Calculate power usage

	m_iPowerDrain = 0;

	//	We always consume some power for life-support

	m_iPowerDrain += 5;

	//	If we're thrusting, then we consume power

	if (!IsParalyzed() && m_pController->GetThrust())
		m_iPowerDrain += m_pDriveDesc->iPowerUse;

	//	Compute power drain of all devices

	for (i = 0; i < GetDeviceCount(); i++)
		m_iPowerDrain += m_Devices[i].CalcPowerUsed(this);

	//	Compute power drain from armor

	for (i = 0; i < GetArmorSectionCount(); i++)
		{
		CInstalledArmor *pArmor = GetArmorSection(i);
		m_iPowerDrain += pArmor->pArmorClass->CalcPowerUsed(pArmor);
		}
	}

bool CShip::CanAttack (void) const

//	CanAttack
//
//	TRUE if the ship can attack. NOTE: The semantic of CanAttack is whether or not
//	this is the kind of object that seems threatening. Even if the ship cannot actually
//	do damage (perhaps because it has no weapons) we still mark it as CanAttack.

	{
	return !IsInGate();
	}

CShip::InstallArmorStatus CShip::CanInstallArmor (const CItem &Item) const

//	CanInstallArmor
//
//	Returns whether or not the armor can be installed

	{
	//	See if the armor is too heavy

	int iMaxArmor = m_pClass->GetMaxArmorMass();
	if (iMaxArmor && Item.GetType()->GetMassKg() > iMaxArmor)
		return insArmorTooHeavy;

	return insArmorOK;
	}

CShip::InstallArmorStatus CShip::CanInstallArmor (CItemListManipulator &ItemList) const
	{
	return CanInstallArmor(ItemList.GetItemAtCursor());
	}

CShip::InstallDeviceStatus CShip::CanInstallDevice (const CItem &Item, bool bReplace)

//	CanInstallDevice
//
//	Returns whether or not the device can be installed

	{
	int i;

	//	Get the item type

	CDeviceClass *pDevice = Item.GetType()->GetDeviceClass();
	if (pDevice == NULL)
		return insNotADevice;

	//	See if the ship's engine core is powerful enough

	if (GetMaxPower() > 0
			&& pDevice->GetPowerRating(&Item) >= GetMaxPower())
		return insEngineCoreTooWeak;

	//	If this is a reactor, then see if the ship class can support it

	if (pDevice->GetCategory() == itemcatReactor
			&& m_pClass->GetMaxReactorPower() > 0
			&& pDevice->GetReactorDesc()->iMaxPower > m_pClass->GetMaxReactorPower())
		return insReactorMaxPower;

	//	See how many device slots we need

	int iSlots = pDevice->GetSlotsRequired();
	int iSlotsToBeFreed = 0;

	//	See if we are replacing an existing device

	DeviceNames iNamedDevice;
	InstallDeviceStatus iAlreadyInstalledError;
	switch (pDevice->GetCategory())
		{
		case itemcatLauncher:
			iNamedDevice = devMissileWeapon;
			iAlreadyInstalledError = insLauncherInstalled;
			break;

		case itemcatShields:
			iNamedDevice = devShields;
			iAlreadyInstalledError = insShieldsInstalled;
			break;

		case itemcatDrive:
			iNamedDevice = devDrive;
			iAlreadyInstalledError = insDriveInstalled;
			break;

		case itemcatCargoHold:
			iNamedDevice = devCargo;
			iAlreadyInstalledError = insCargoInstalled;
			break;

		case itemcatReactor:
			iNamedDevice = devReactor;
			iAlreadyInstalledError = insReactorInstalled;
			break;

		default:
			iNamedDevice = devNone;
			iAlreadyInstalledError = insOK;
		}

	if (iNamedDevice != devNone)
		{
		CInstalledDevice *pAlreadyInstalled = GetNamedDevice(iNamedDevice);
		if (pAlreadyInstalled)
			{
			if (bReplace)
				iSlotsToBeFreed = pAlreadyInstalled->GetClass()->GetSlotsRequired();
			else
				return iAlreadyInstalledError;
			}
		}

	//	Count the number of slots being used up currently

	int iAll = 0;
	int iWeapons = 0;
	int iNonWeapons = 0;
	for (i = 0; i < GetDeviceCount(); i++)
		{
		CInstalledDevice *pDevice = GetDevice(i);
		if (!pDevice->IsEmpty())
			{
			int iSlots = pDevice->GetClass()->GetSlotsRequired();
			iAll += iSlots;

			if (pDevice->GetCategory() == itemcatWeapon || pDevice->GetCategory() == itemcatLauncher)
				iWeapons += iSlots;
			else
				iNonWeapons += iSlots;
			}
		}

	//	See if we can fit the new device

	if (iAll + iSlots - iSlotsToBeFreed > m_pClass->GetMaxDevices())
		return insNoSlots;

	//	See if we have exceeded the maximum number of weapons

	if ((pDevice->GetCategory() == itemcatWeapon || pDevice->GetCategory() == itemcatLauncher)
			&& m_pClass->GetMaxWeapons() < m_pClass->GetMaxDevices()
			&& iWeapons + iSlots - iSlotsToBeFreed > m_pClass->GetMaxWeapons())
		return insNoWeaponSlots;

	//	See if we have exceeded the maximum number of non-weapons

	if ((pDevice->GetCategory() != itemcatWeapon && pDevice->GetCategory() != itemcatLauncher)
			&& m_pClass->GetMaxNonWeapons() < m_pClass->GetMaxDevices()
			&& iNonWeapons + iSlots - iSlotsToBeFreed > m_pClass->GetMaxNonWeapons())
		return insNoGeneralSlots;

	//	Otherwise it is OK

	return insOK;
	}

CShip::InstallDeviceStatus CShip::CanInstallDevice (CItemListManipulator &ItemList)

//	CanInstallDevice
//
//	Returns whether or not the device can be installed

	{
	return CanInstallDevice(ItemList.GetItemAtCursor());
	}

CShip::RemoveDeviceStatus CShip::CanRemoveDevice (const CItem &Item)

//	CanRemoveDevice
//
//	Returns whether or not the device can be removed

	{
	if (!Item.IsInstalled() || !Item.GetType()->IsDevice())
		return remNotInstalled;

	CDeviceClass *pDevice = Item.GetType()->GetDeviceClass();
	if (pDevice == NULL)
		return remNotInstalled;

	//	Check for special device cases

	switch (pDevice->GetCategory())
		{
		case itemcatCargoHold:
			{
			//	Compute how much cargo space we need to be able to hold

			OnComponentChanged(comCargo);
			Metric rCargoSpace = m_rCargoMass + Item.GetType()->GetMass();

			//	If this is larger than the ship class max, then we cannot remove

			if (rCargoSpace > (Metric)m_pClass->GetCargoSpace())
				return remTooMuchCargo;

			break;
			}
		default:
			break;
		}

	//	OK

	return remOK;
	}

void CShip::ClearBlindness (void)

//	ClearBlindness
//
//	Ship is no longer blind

	{
	bool bNotify = (m_iBlindnessTimer != 0);
	m_iBlindnessTimer = 0;

	if (bNotify)
		m_pController->OnBlindnessChanged(false);
	}

void CShip::ClearDisarmed (void)

//	ClearDisarmed
//
//	Ship is no longer disarmed

	{
	m_iDisarmedTimer = 0;
	}

void CShip::ClearLRSBlindness (void)

//	ClearLRSBlindness
//
//	Ship is no longer LRS blind

	{
	m_iLRSBlindnessTimer = 0;
	}

void CShip::ClearParalyzed (void)

//	ClearParalyzed
//
//	Ship is no longer paralyzed

	{
	m_iParalysisTimer = 0;
	}

void CShip::ConsumeFuel (int iFuel)

//	ConsumeFuel
//
//	Consumes some amount of fuel

	{
	ASSERT(!m_fOutOfFuel);
	if (m_fTrackFuel && !m_fOutOfFuel)
		m_iFuelLeft = std::max(0, m_iFuelLeft - iFuel);
	}

ALERROR CShip::CreateFromClass (CSystem *pSystem, 
		CShipClass *pClass,
		IShipController *pController,
		CSovereign *pSovereign,
		const CVector &vPos, 
		const CVector &vVel, 
		int iRotation, 
		CShip **retpShip)

//	CreateFromClass
//
//	Creates a new ship based on the class.
//
//	pController is owned by the ship if this function is successful.

	{
	ALERROR error;
	CShip *pShip;
	int i;
	CUniverse *pUniv = pSystem->GetUniverse();

	pShip = new CShip;
	if (pShip == NULL)
		return ERR_MEMORY;

	//	Initialize

	pShip->SetPos(vPos);
	pShip->SetVel(vVel);
	pShip->CalculateMove();
	pShip->SetObjectDestructionHook();
	pShip->SetCanBounce();
	pShip->SetHasOnObjDockedEvent(pClass->HasOnObjDockedEvent());
	pShip->SetHasOnAttackedEvent(pClass->FindEventHandler(CONSTLIT("OnAttacked")));
	pShip->SetHasOnDamageEvent(pClass->FindEventHandler(CONSTLIT("OnDamage")));
	if (pClass->GetAISettings().dwFlags & aiflagNoFriendlyFire)
		pShip->SetNoFriendlyFire();

	pShip->m_pClass = pClass;
	pShip->m_pController = pController;
	pShip->m_pSovereign = pSovereign;
	pShip->m_sName = pClass->GenerateShipName(&pShip->m_dwNameFlags);
	pShip->m_iRotation = pClass->AlignToRotationAngle(iRotation);
	pShip->m_iFireDelay = 0;
	pShip->m_iMissileFireDelay = 0;
	pShip->m_iManeuverDelay = 0;
	pShip->m_iBlindnessTimer = 0;
	pShip->m_iLRSBlindnessTimer = 0;
	pShip->m_iParalysisTimer = 0;
	pShip->m_iDisarmedTimer = 0;
	pShip->m_iExitGateTimer = 0;
	pShip->m_pExitGate = NULL;
	pShip->m_rItemMass = 0.0;
	pShip->m_rCargoMass = 0.0;
	pShip->SetDriveDesc(NULL);
	pShip->m_pReactorDesc = pClass->GetReactorDesc();

	pShip->m_fOutOfFuel = false;
	pShip->m_fTrackFuel = false;
	pShip->m_fTrackMass = false;
	pShip->m_fRadioactive = false;
	pShip->m_fHasAutopilot = false;
	pShip->m_fDestroyInGate = false;
	pShip->m_fFollowPlayerThroughGate = false;
	pShip->m_fHalfSpeed = false;
	pShip->m_fHasTargetingComputer = false;
	pShip->m_fHasSecondaryWeapons = false;
	pShip->m_fSRSEnhanced = false;
	pShip->m_fKnown = false;
	pShip->m_fHiddenByNebula = false;
	pShip->m_fIdentified = false;
	pShip->m_dwSpare = 0;

	//	Create items

	if (error = pShip->CreateRandomItems(pClass->GetRandomItemTable()))
		return error;

	//	Devices

	for (i = 0; i < devNamesCount; i++)
		pShip->m_NamedDevices[i] = -1;

	CDeviceDescList Devices;
	pClass->GenerateDevices(Devices);

	pShip->m_iDeviceCount = Max(Devices.GetCount(), pClass->GetMaxDevices());
	pShip->m_Devices = new CInstalledDevice [pShip->m_iDeviceCount];

	CItemListManipulator ShipItems(pShip->GetItemList());
	for (i = 0; i < Devices.GetCount(); i++)
		{
		const SDeviceDesc &NewDevice = Devices.GetDeviceDesc(i);

		//	Add item

		ShipItems.AddItem(NewDevice.Item);

		//	Install the device

		pShip->m_Devices[i].InitFromDesc(NewDevice);
		pShip->m_Devices[i].Install(pShip, ShipItems, i);

		//	Assign to named devices

		ItemCategories Category = pShip->m_Devices[i].GetCategory();
		if (pShip->m_Devices[i].IsSecondaryWeapon())
			{
			for (int j = devTurretWeapon1; j <= devTurretWeapon6; j++)
				if (pShip->m_NamedDevices[j] == -1)
					{
					pShip->m_NamedDevices[j] = i;
					break;
					}
			pShip->m_fHasSecondaryWeapons = true;
			}
		else if (pShip->m_NamedDevices[devPrimaryWeapon] == -1
				&& Category == itemcatWeapon)
			pShip->m_NamedDevices[devPrimaryWeapon] = i;
		else if (pShip->m_NamedDevices[devMissileWeapon] == -1
				&& Category == itemcatLauncher)
			pShip->m_NamedDevices[devMissileWeapon] = i;
		else if (pShip->m_NamedDevices[devShields] == -1
				&& Category == itemcatShields)
			pShip->m_NamedDevices[devShields] = i;
		else if (pShip->m_NamedDevices[devDrive] == -1
				&& Category == itemcatDrive)
			pShip->m_NamedDevices[devDrive] = i;
		else if (pShip->m_NamedDevices[devCargo] == -1
				&& Category == itemcatCargoHold)
			pShip->m_NamedDevices[devCargo] = i;
		else if (pShip->m_NamedDevices[devReactor] == -1
				&& Category == itemcatReactor)
			pShip->m_NamedDevices[devReactor] = i;

		//	Add extra items (we do this at the end because this will
		//	modify the cursor)

		ShipItems.AddItems(NewDevice.ExtraItems);
		}

	//	Add the ship to the system

	if (error = pShip->AddToSystem(pSystem))
		{
		delete pShip;
		return error;
		}

	//	Initialize the armor from the class

	CArmorClass *pArmorClass = NULL;
	bool bComplete = true;
	for (i = 0; i < pClass->GetHullSectionCount(); i++)
		{
		CShipClass::HullSection *pSect = pClass->GetHullSection(i);

		//	Add item

		CItem ArmorItem(pSect->pArmor->GetItemType(), 1);
		ArmorItem.SetInstalled(i);
		ShipItems.AddItem(ArmorItem);

		//	Install the armor

		CInstalledArmor ArmorSect;
		ArmorSect.pArmorClass = pSect->pArmor;

		//	See if all armor segments are the same

		if (pArmorClass == NULL)
			pArmorClass = pSect->pArmor;
		else if (pArmorClass != pSect->pArmor)
			bComplete = false;

		//	Add armor

		if (error = pShip->m_Armor.AppendStruct(&ArmorSect, NULL))
			return error;
		}

	//	Update armor hit points now that we know whether we have a complete
	//	set of armor.

	for (i = 0; i < pShip->GetArmorSectionCount(); i++)
		{
		CInstalledArmor *pArmor = pShip->GetArmorSection(i);
		pArmor->m_fComplete = bComplete;
		pArmor->iHitPoints = pArmor->pArmorClass->GetHitPoints(pArmor);
		}

	pShip->OnComponentChanged(comCargo);
	pShip->CalcArmorBonus();
	pShip->CalcDeviceBonus();

	//	Initialize fuel now that we know our maximum

	pShip->m_iFuelLeft = pShip->GetMaxFuel();

	//	Set the bounds for this object

	const CObjectImageArray &Image = pShip->m_pClass->GetImage();
	pShip->SetBounds(Image.GetImageRect());

	//	Initialize docking ports (if any)

	pShip->m_DockingPorts.InitPorts(pShip, 
			pShip->m_pClass->GetDockingPortCount(),
			pShip->m_pClass->GetDockingPortPositions());

	//	Give the class a chance to do something

	if (!pSystem->IsCreationInProgress())
		pShip->FireOnCreate();

	//	Done

	if (retpShip)
		*retpShip = pShip;

	return NOERROR;
	}

CSpaceObject::DamageResults CShip::Damage (CSpaceObject *pCause, const CVector &vHitPos, int iDirection, const DamageDesc &Damage)

//	Damage
//
//	Ship takes damage from the given source

	{
	int i;

	//	Compose the damage context so that various devices can deal with it.

	SDamageCtx Ctx;
	Ctx.pObj = this;
	Ctx.Damage = Damage;
	Ctx.iDirection = iDirection;
	Ctx.vHitPos = vHitPos;
	Ctx.pCause = pCause;
	Ctx.pAttacker = (pCause ? pCause->GetDamageCause() : NULL);

	//	Roll for damage

	Ctx.iDamage = Ctx.Damage.RollDamage();
	if (Ctx.iDamage == 0)
		return damageNoDamage;

	//	Tell our controller that someone hit us

	m_pController->OnAttacked(Ctx.pAttacker, Ctx.Damage);

	//	OnAttacked event

	if (HasOnAttackedEvent())
		FireOnAttacked(Ctx);

	//	See if the damage is blocked by some external defense

	if (m_EnergyFields.AbsorbDamage(this, Ctx))
		return damageNoDamage;

	//	If this is a momentum attack then we are pushed

	int iMomentum;
	if ((iMomentum = Ctx.Damage.GetMomentumDamage())
			&& m_pDocked == NULL)
		{
		CVector vAccel = PolarToVector(iDirection, -10 * iMomentum * iMomentum);
		Accelerate(vAccel, g_MomentumConstant);
		ClipSpeed(GetMaxSpeed());
		}

	//	Let our shield generators take a crack at it

	int iOriginalDamage = Ctx.iDamage;
	for (i = 0; i < GetDeviceCount(); i++)
		{
		if (m_Devices[i].AbsorbDamage(this, Ctx))
			return damageAbsorbedByShields;
		}

	//	Damage any devices that are outside the hull (e.g., Patch Spiders)

	for (i = 0; i < GetDeviceCount(); i++)
		if (!m_Devices[i].IsEmpty() && m_Devices[i].IsExternal())
			DamageExternalDevice(i, Ctx);

	//	Map the direction that we got hit from to a ship-relative
	//	direction (i.e., adjust for the ship's rotation)

	int iHitAngle = (Ctx.iDirection + 360 - m_iRotation) % 360;

	//	Figure out which section of armor got hit

	int iSectHit = m_pClass->GetHullSectionAtAngle(iHitAngle);
	CInstalledArmor *pArmor = GetArmorSection(iSectHit);

	//	If this armor section reflects this kind of damage then
	//	send the damage on

	if (pArmor->pArmorClass->IsReflective(pArmor, Ctx.Damage) && Ctx.pCause)
		{
		Ctx.pCause->CreateReflection(Ctx.vHitPos, (Ctx.iDirection + 120 + mathRandom(0, 120)) % 360);
		return damageNoDamage;
		}

	//	If this is a disintegration attack, then disintegrate the ship

	int iDisintegration = Ctx.Damage.GetDisintegrationDamage();
	if (iDisintegration && !pArmor->pArmorClass->IsDisintegrationImmune(pArmor))
		{
		if (!OnDestroyCheck(killedByDisintegration, Ctx.pAttacker))
			return damageNoDamage;

		Destroy(killedByDisintegration, Ctx.pAttacker);
		return damageDestroyed;
		}

	//	If this is a shatter attack, see if the ship is destroyed

	int iShatter = Ctx.Damage.GetShatterDamage();
	if (iShatter)
		{
		//	Compute the threshold mass. Below this size, we shatter the object

		int iMassLimit = 10 * mathPower(5, iShatter);
		if (GetMass() < iMassLimit)
			{
			if (!OnDestroyCheck(killedByShatter, Ctx.pAttacker))
				return damageNoDamage;

			Destroy(killedByShatter, Ctx.pAttacker);
			return damageDestroyed;
			}

		//	Otherwise, we continue with normal damage
		}

	//	If this is a paralysis attack and we've gotten past the shields
	//	then freeze the ship.

	int iEMP = Ctx.Damage.GetEMPDamage();
	if (iEMP)
		{
		if (!pArmor->pArmorClass->IsEMPDamageImmune(pArmor))
			{
			//	The chance of being paralyzed is dependent
			//	on the EMP rating.

			int iChance = 4 * iEMP * iEMP * pArmor->pArmorClass->GetEMPDamageAdj() / 100;
			if (mathRandom(1, 100) <= iChance)
				{
				//	The length of time paralyzed depends on the amount
				//	of damage done.

				MakeParalyzed(Ctx.iDamage * g_TicksPerSecond / 2);
				}
			}

		//	EMP damage does not hurt armor

		Ctx.iDamage = 0;
		}

	//	If this is blinding damage then our sensors are disabled

	int iBlinding = Ctx.Damage.GetBlindingDamage();
	if (iBlinding)
		{
		if (!pArmor->pArmorClass->IsBlindingDamageImmune(pArmor))
			{
			//	The chance of being blinded is dependent
			//	on the rating.

			int iChance = 4 * iBlinding * iBlinding * pArmor->pArmorClass->GetBlindingDamageAdj() / 100;
			if (mathRandom(1, 100) <= iChance)
				{
				//	The length of time blinded depends on the amount
				//	of damage done.

				MakeBlind(Ctx.iDamage * g_TicksPerSecond / 2);
				}
			}

		//	this does not hurt armor

		Ctx.iDamage = 0;
		}

	//	If this attack is radioactive, then contaminate the ship

	int iRadioactive = Ctx.Damage.GetRadiationDamage();
	if (iRadioactive && !pArmor->pArmorClass->IsRadiationImmune(pArmor))
		MakeRadioactive();

	//	If this is device damage, then see if any device is damaged

	int iDeviceDamage = Ctx.Damage.GetDeviceDamage();
	if (iDeviceDamage)
		{
		if (!pArmor->pArmorClass->IsDeviceDamageImmune(pArmor))
			{
			//	The chance of damaging a device depends on the rating.

			int iChance = 4 * iDeviceDamage * iDeviceDamage * pArmor->pArmorClass->GetDeviceDamageAdj() / 100;
			if (mathRandom(1, 100) <= iChance)
				DamageRandomDevice();
			}

		//	Damage is decreased

		Ctx.iDamage = Ctx.iDamage / 2;
		}

	//	Adjust the damage based on the armor

	Ctx.iDamage = pArmor->pArmorClass->CalcAdjustedDamage(pArmor,
			Ctx.Damage, 
			Ctx.iDamage);
	if (Ctx.iDamage == 0)
		return damageNoDamage;

	//	Give events a chance to change the damage

	if (HasOnDamageEvent())
		FireOnDamage(Ctx);

	//	Create a hit effect

	CEffectCreator *pHitEffect = g_pUniverse->FindEffectType(g_HitEffectUNID);
	if (pHitEffect)
		pHitEffect->CreateEffect(GetSystem(),
				NULL,
				vHitPos,
				GetVel());

	//	If we've still got armor left, then we take damage but otherwise
	//	we're OK.

	if (Ctx.iDamage <= pArmor->iHitPoints)
		{
		pArmor->iHitPoints -= Ctx.iDamage;

		//	Tell the controller that we were damaged

		m_pController->OnDamaged(Ctx.pAttacker, iSectHit, Ctx.Damage);

		return damageArmorHit;
		}

	//	Otherwise we're in big trouble

	else
		{
		//	The armor absorbs some of the damage

		Ctx.iDamage -= pArmor->iHitPoints;
		pArmor->iHitPoints = 0;

		//	Figure out which areas of the ship got affected

		CShipClass::HullSection *pSect = m_pClass->GetHullSection(iSectHit);
		DWORD dwDamage = pSect->dwAreaSet;

		//	If this is a non-critical hit, then there is still a random
		//	chance that it will destroy the ship

		if (!(dwDamage & CShipClass::sectCritical))
			{
			int iChanceOfDeath = 5;

			//	We only care about mass destruction damage

			int iWMD = Ctx.Damage.GetMassDestructionAdj();
			int iWMDDamage = Max(1, iWMD * Ctx.iDamage / 100);

			//	Compare the amount of damage that we are taking with the
			//	original strength (HP) of the armor. Increase the chance
			//	of death appropriately.

			int iMaxHP = pSect->pArmor->GetHitPoints(pArmor);
			if (iMaxHP > 0)
				iChanceOfDeath += 20 * iWMDDamage / iMaxHP;

			//	Roll the dice

			if (mathRandom(1, 100) <= iChanceOfDeath)
				dwDamage |= CShipClass::sectCritical;
			}

		//	Ship is destroyed!

		if (dwDamage & CShipClass::sectCritical)
			{
			if (!OnDestroyCheck(Ctx.Damage.GetCause(), Ctx.pAttacker))
				return damageArmorHit;

			Destroy(Ctx.Damage.GetCause(), Ctx.pAttacker);
			return damageDestroyed;
			}

		//	Otherwise, this is a non-critical hit

		else
			{
			return damageArmorHit;
			}
		}
	}

void CShip::DamageArmor (int iSect, DamageTypes Type, int iDamage)

//	DamageArmor
//
//	Damage armor

	{
	CInstalledArmor *pArmor = GetArmorSection(iSect);

	//	Adjust the damage for the armor

	DamageDesc Damage(Type, DiceRange(0, 0, iDamage));
	iDamage = pArmor->pArmorClass->CalcAdjustedDamage(pArmor,
			Damage, 
			iDamage);
	if (iDamage == 0)
		return;

	//	Armor takes damage

	iDamage = std::min(iDamage, pArmor->iHitPoints);
	if (iDamage == 0)
		return;

	pArmor->iHitPoints -= iDamage;

	//	Tell the controller that we were damaged

	m_pController->OnDamaged(NULL, iSect, Damage);
	}

void CShip::DamageRandomDevice (void)

//	DamageRandomDevice
//
//	A random device on the ship is damaged

	{
	int iCount = 0;

	//	Count all the items that can be damaged

	CItemListManipulator Search(GetItemList());
	while (Search.MoveCursorForward())
		{
		const CItem &Item = Search.GetItemAtCursor();
		CItemType *pType = Item.GetType();
		if (pType->IsDevice() 
				&& Item.IsInstalled() 
				&& pType->GetDeviceClass()->CanBeDamaged())
			iCount++;
		}

	//	Choose a random device to damage

	if (iCount > 0)
		{
		int iRoll = mathRandom(0, iCount-1);

		Search.ResetCursor();
		while (Search.MoveCursorForward())
			{
			const CItem &Item = Search.GetItemAtCursor();
			CItemType *pType = Item.GetType();
			if (pType->IsDevice() 
					&& Item.IsInstalled() 
					&& pType->GetDeviceClass()->CanBeDamaged())
				{
				if (iRoll == 0)
					{
					DamageItem(Search);

					int iDevSlot = Item.GetInstalled();
					CInstalledDevice *pDevice = &m_Devices[iDevSlot];

					OnDeviceFailure(pDevice, CDeviceClass::failDeviceHitByDamage);
					return;
					}
				else
					iRoll--;
				}
			}
		}
	}

void CShip::DeactivateShields (void)

//	DeactivateShields
//
//	Lowers shields because some other item on the ship is
//	incompatible with shields (e.g., particular kinds of
//	armor.

	{
	int iShieldDev = m_NamedDevices[devShields];
	if (iShieldDev != -1 && m_Devices[iShieldDev].IsEnabled())
		EnableDevice(iShieldDev, false);
	}

CString CShip::DebugCrashInfo (void)

//	DebugCrashInfo
//
//	Return info that might be useful in a crash.

	{
	return m_pController->DebugCrashInfo();
	}

void CShip::Decontaminate (void)

//	Decontaminate
//
//	Decontaminates the ship

	{
	if (m_fRadioactive)
		{
		m_iContaminationTimer = 0;
		m_fRadioactive = false;
		m_pController->OnRadiationCleared();
		}
	}

void CShip::DamageDevice (CInstalledDevice *pDevice, CDeviceClass::DeviceFailureType iCause)

//	DamageDevice
//
//	Damage the given device

	{
	int iDev = FindDeviceIndex(pDevice);
	CItemListManipulator Search(GetItemList());
	SetCursorAtDevice(Search, iDev);

	DamageItem(Search);
	OnDeviceFailure(pDevice, iCause);
	}

void CShip::DamageExternalDevice (int iDev, SDamageCtx &Ctx)

//	DamageExternalDevice
//
//	See if an external device got hit

	{
	CInstalledDevice *pDevice = GetDevice(iDev);

	//	If we're already damaged, then nothing more can happen

	if (pDevice->IsEmpty() || pDevice->IsDamaged())
		return;

	//	The chance that the device got hit depends on the number of armor segments
	//	A device takes up 1/9th of the surface area of a segment.

	if (mathRandom(1, GetArmorSectionCount() * 9) != 7)
		return;

	//	If the device gets hit, see if it gets damaged

	int iLevel = pDevice->GetClass()->GetLevel();
	int iMaxLevel = 0;
	int iChanceOfDamage = Ctx.iDamage * ((26 - iLevel) * 4) / 100;

	switch (Ctx.Damage.GetDamageType())
		{
		case damageLaser:
		case damageKinetic:
			iMaxLevel = 6;
			break;

		case damageParticle:
		case damageBlast:
			iMaxLevel = 9;
			break;

		case damageIonRadiation:
			iMaxLevel = 12;
			iChanceOfDamage = iChanceOfDamage * 120 / 100;
			break;

		case damageThermonuclear:
			iMaxLevel = 12;
			break;

		case damagePositron:
		case damagePlasma:
			iMaxLevel = 15;
			break;

		case damageAntiMatter:
		case damageNano:
			iMaxLevel = 18;
			break;

		case damageGravitonBeam:
			iMaxLevel = 21;
			iChanceOfDamage = iChanceOfDamage * 75 / 100;
			break;

		case damageSingularity:
			iMaxLevel = 21;
			break;

		case damageDarkAcid:
		case damageDarkSteel:
			iMaxLevel = 24;
			break;

		default:
			iMaxLevel = 27;
		}

	//	If the device is too high-level for the damage type, then nothing
	//	happens

	if (iLevel > iMaxLevel)
		return;

	//	Calculate the chance of damaging the device based on the damage
	//	and the level of the device

	if (mathRandom(1, 100) > iChanceOfDamage)
		return;

	//	Damage the device

	DamageDevice(pDevice, CDeviceClass::failDeviceHitByDamage);
	}

void CShip::DamageItem (CItemListManipulator &ItemList)

//	DamageItem
//
//	Damages an item

	{
	const CItem &Item = ItemList.GetItemAtCursor();

	//	Figure out the current mods on this item

	CItemEnhancement Mods(Item.GetMods());

	//	If the item has an enhancement mod, then we remove it

	if (Mods.IsEnhancement())
		{
		EnhanceItem(ItemList, etLoseEnhancement);
		}

	//	If the item is enhanced, then damaging it removes the enhancements

	else if (Item.IsEnhanced())
		{
		//	If this item is installed then we need to damage
		//	the device structure as well.

		if (Item.IsInstalled())
			{
			int iDevSlot = Item.GetInstalled();
			CInstalledDevice *pDevice = &m_Devices[iDevSlot];
			pDevice->SetEnhanced(false);
			}

		//	De-enhance the item

		ItemList.SetEnhancedAtCursor(false);

		//	Recalc bonuses

		CalcDeviceBonus();
		InvalidateItemListState();
		}

	//	Otherwise, damage the item

	else if (!Item.IsDamaged())
		{
		//	If this item is installed then we need to damage
		//	the device structure as well.

		if (Item.IsInstalled())
			{
			int iDevSlot = Item.GetInstalled();
			CInstalledDevice *pDevice = &m_Devices[iDevSlot];
			pDevice->SetDamaged(true);
			}

		//	Enhance the item

		ItemList.SetDamagedAtCursor(true);

		//	Recalc bonuses

		CalcDeviceBonus();
		InvalidateItemListState();
		}
	}

void CShip::DepleteShields (void)

//	DepleteShields
//
//	Deplete shields to 0

	{
	CInstalledDevice *pShields = GetNamedDevice(devShields);
	if (pShields)
		pShields->Deplete(this);
	}

void CShip::DisableDevice (CInstalledDevice *pDevice)

//	DisableDevice
//
//	Disable the given device

	{
	int iDev = FindDeviceIndex(pDevice);
	if (iDev != -1)
		EnableDevice(iDev, false);
	}

void CShip::EnableDevice (int iDev, bool bEnable)

//	EnableDevice
//
//	Enable or disable a device

	{
	ASSERT(iDev >= 0 && iDev < GetDeviceCount());
	if (iDev < 0 || iDev >= GetDeviceCount())
		return;

	m_Devices[iDev].SetEnabled(bEnable);

	//	Recalc bonuses, etc.

	CalcDeviceBonus();
	m_pController->OnWeaponStatusChanged();
	m_pController->OnArmorRepaired(-1);
	m_pController->OnDeviceEnabledDisabled(iDev, bEnable);
	}

#if 0
EnhanceArmorStatus CShip::EnhanceArmor (CItemListManipulator &ItemList, DWORD dwMods)

//	EnhanceArmor
//
//	Enhances an armor item

	{
	CItemType *pType = ItemList.GetItemAtCursor().GetType();
	if (!pType->IsArmor())
		return easNoEffect;

	CArmorClass *pArmor = pType->GetArmorClass();

	//	If the armor is installed, update the m_dwMods var

	if (ItemList.GetItemAtCursor().IsInstalled())
		{
		CInstalledArmor *pInsArmor = GetArmorSection(ItemList.GetItemAtCursor().GetInstalled());
		int iOldMaxHP = pArmor->GetHitPoints(pInsArmor);

		//	Enhance the armor

		EnhanceArmorStatus iResult = pArmor->EnhanceArmor(ItemList, dwMods);

		//	Update the installed armor

		pInsArmor->m_Mods = ItemList.GetItemAtCursor().GetMods();

		//	Make sure we don't have more hp than we're supposed to

		int iHP = pArmor->GetHitPoints(pInsArmor);
		if (iHP > iOldMaxHP)
			pInsArmor->iHitPoints += (iHP - iOldMaxHP);
		pInsArmor->iHitPoints = Min(pInsArmor->iHitPoints, iHP);

		//	Update UI

		CalcDeviceBonus();
		m_pController->OnWeaponStatusChanged();
		m_pController->OnArmorRepaired(-1);

		return iResult;
		}
	else
		return pArmor->EnhanceArmor(ItemList, dwMods);
	}
#endif

EnhanceItemStatus CShip::EnhanceItem (CItemListManipulator &ItemList, DWORD dwMods)

//	EnhanceItem
//
//	Enhances the item at cursor (either installed or in cargo hold)

	{
	CItemType *pType = ItemList.GetItemAtCursor().GetType();

	//	If this is an old-style enhancement, then handle it the old way

	if (dwMods == etBinaryEnhancement)
		{
		EnhanceItem(ItemList);
		return eisOK;
		}

	//	Handle some special cases

	if (pType->IsArmor())
		{
		CArmorClass *pArmor = pType->GetArmorClass();
		CInstalledArmor *pInsArmor = NULL;
		if (ItemList.GetItemAtCursor().IsInstalled())
			pInsArmor = GetArmorSection(ItemList.GetItemAtCursor().GetInstalled());
		
		//	If we're trying to make armor immune to radiation and it is already immune
		//	then we return already enhanced

		if (dwMods == 0x0B00)
			{
			if (pArmor->IsRadiationImmune(pInsArmor))
				return eisAlreadyEnhanced;
			}
		}

	//	Figure out the effect of the enhancement on the item

	CItemEnhancement Enhancement = ItemList.GetItemAtCursor().GetMods();
	EnhanceItemStatus iResult = Enhancement.Combine(dwMods);

	//	Enhance

	ItemList.SetModsAtCursor(Enhancement);

	//	Deal with installed items

	if (ItemList.GetItemAtCursor().IsInstalled())
		{
		if (pType->IsArmor())
			{
			CArmorClass *pArmor = pType->GetArmorClass();
			CInstalledArmor *pInsArmor = GetArmorSection(ItemList.GetItemAtCursor().GetInstalled());
			int iOldMaxHP = pArmor->GetHitPoints(pInsArmor);

			//	Update the installed armor

			pInsArmor->m_Mods = ItemList.GetItemAtCursor().GetMods();

			//	Make sure we don't have more hp than we're supposed to

			int iHP = pArmor->GetHitPoints(pInsArmor);
			if (iHP > iOldMaxHP)
				pInsArmor->iHitPoints += (iHP - iOldMaxHP);
			pInsArmor->iHitPoints = Min(pInsArmor->iHitPoints, iHP);
			}
		else if (pType->IsDevice())
			{
			int iDevSlot = ItemList.GetItemAtCursor().GetInstalled();
			CInstalledDevice *pDevice = &m_Devices[iDevSlot];
			pDevice->SetMods(ItemList.GetItemAtCursor().GetMods());
			}

		//	Update UI

		CalcDeviceBonus();
		InvalidateItemListState();
		m_pController->OnWeaponStatusChanged();
		m_pController->OnArmorRepaired(-1);
		}

	//	Done

	return iResult;
	}

void CShip::EnhanceItem (CItemListManipulator &ItemList)

//	EnhanceItem
//
//	Enhances an item

	{
	const CItem &Item = ItemList.GetItemAtCursor();

	//	If the item is damaged, then enhancing it repairs it

	if (Item.IsDamaged())
		{
		//	Repair the item

		ItemList.SetDamagedAtCursor(false);

		//	If this item is installed then we need to enhance
		//	the device structure as well.

		if (Item.IsInstalled())
			{
			int iDevSlot = Item.GetInstalled();
			CInstalledDevice *pDevice = &m_Devices[iDevSlot];
			pDevice->SetDamaged(false);
			}

		//	Recalc bonuses

		CalcArmorBonus();
		CalcDeviceBonus();
		InvalidateItemListState();
		}

	//	Otherwise, enhance it if it is not already enhanced

	else if (!Item.IsEnhanced())
		{
		//	If this item is installed then we need to enhance
		//	the device structure as well.

		if (Item.IsInstalled())
			{
			int iDevSlot = Item.GetInstalled();
			CInstalledDevice *pDevice = &m_Devices[iDevSlot];
			pDevice->SetEnhanced(true);
			}

		//	Enhance the item

		ItemList.SetEnhancedAtCursor(true);

		//	Recalc bonuses

		CalcArmorBonus();
		CalcDeviceBonus();
		InvalidateItemListState();
		}
	}

CInstalledDevice *CShip::FindDevice (const CItem &Item)

//	FindDevice
//
//	Returns the device given the item

	{
	if (Item.IsInstalled() && Item.GetType()->IsDevice())
		{
		int iDevSlot = Item.GetInstalled();
		return &m_Devices[iDevSlot];
		}
	else
		return NULL;
	}

int CShip::FindDeviceIndex (CInstalledDevice *pDevice) const

//	FindDeviceIndex
//
//	Finds the index for the given device

	{
	for (int i = 0; i < GetDeviceCount(); i++)
		if (pDevice == GetDevice(i))
			return i;

	return -1;
	}

int CShip::FindFreeDeviceSlot (void)

//	FindFreeDeviceSlot
//
//	Returns the index of a free device slot; -1 if none left

	{
	int i;

	for (i = 0; i < GetDeviceCount(); i++)
		if (m_Devices[i].IsEmpty())
			return i;

	//	We need to allocate a new slot

	int iNewDeviceCount = m_iDeviceCount + 1;
	CInstalledDevice *NewDevices = new CInstalledDevice [iNewDeviceCount];

	for (i = 0; i < GetDeviceCount(); i++)
		NewDevices[i] = m_Devices[i];

	delete [] m_Devices;
	m_Devices = NewDevices;
	m_iDeviceCount = iNewDeviceCount;

	return GetDeviceCount() - 1;
	}

int CShip::FindNextDevice (int iStart, ItemCategories Category)

//	FindNextDevice
//
//	Finds the next device of the given category

	{
	int iStartingSlot;
	if (iStart == -1)
		iStartingSlot = 0;
	else
		iStartingSlot = (iStart + 1) % GetDeviceCount();

	for (int i = 0; i < GetDeviceCount(); i++)
		{
		int iDevice = (i + iStartingSlot) % GetDeviceCount();
		if (!m_Devices[iDevice].IsEmpty() 
				&& m_Devices[iDevice].GetCategory() == Category)
			return iDevice;
		}

	return -1;
	}

int CShip::FindRandomDevice (bool bEnabledOnly)

//	FindRandomDevice
//
//	Returns a random device

	{
	int i;

	//	Count the number of valid devices

	int iCount = 0;
	for (i = 0; i < GetDeviceCount(); i++)
		if (!m_Devices[i].IsEmpty() 
				&& (!bEnabledOnly || m_Devices[i].IsEnabled()))
			iCount++;

	if (iCount == 0)
		return -1;

	int iDev = mathRandom(1, iCount);

	//	Return the device

	for (i = 0; i < GetDeviceCount(); i++)
		if (!m_Devices[i].IsEmpty()
				&& (!bEnabledOnly || m_Devices[i].IsEnabled()))
			{
			if (--iDev == 0)
				return i;
			}

	return -1;
	}

void CShip::GateHook (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate)

//	GateHook
//
//	Ship passes through stargate

	{
	FireOnEnteredGate(pStargate);
	m_pController->OnEnterGate(pDestNode, sDestEntryPoint, pStargate);
	}

CSpaceObject *CShip::GetBase (void) const

//	GetBase
//
//	Get the base for this ship

	{
	//	If we're docked, then that's our base

	if (m_pDocked)
		return m_pDocked;

	//	Otherwise, ask the controller

	else
		return m_pController->GetBase();
	}

Metric CShip::GetCargoSpaceLeft (void)

//	GetCargoSpaceLeft
//
//	Returns the amount of cargo space left in tons

	{
	//	Compute total cargo space. Start with the space specified
	//	by the class. Then see if any devices add to it.

	Metric rCargoSpace = (Metric)CalcMaxCargoSpace();
	
	//	Compute cargo mass

	OnComponentChanged(comCargo);

	//	Compute space left

	if (m_rCargoMass > rCargoSpace)
		return 0.0;
	else
		return (rCargoSpace - m_rCargoMass);
	}

ItemCategories CShip::GetCategoryForNamedDevice (DeviceNames iDev)

//	GetCategoryForNamedDevice
//
//	Returns the device category that is selected for
//	this named device slot

	{
	switch (iDev)
		{
		case devPrimaryWeapon:
			return itemcatWeapon;

		case devMissileWeapon:
			return itemcatLauncher;

		case devShields:
			return itemcatShields;

		case devDrive:
			return itemcatDrive;

		case devCargo:
			return itemcatCargoHold;

		case devReactor:
			return itemcatReactor;
		default:
			break;
		}

	return itemcatMiscDevice;
	}

int CShip::GetCombatPower (void)

//	GetCombatPower
//
//	Returns the relative strength of this object in combat
//	Scale from 0 to 100 (Note: This works for ships up to
//	level XV).

	{
	return m_pController->GetCombatPower();
	}

DamageTypes CShip::GetDamageType (void)

//	GetDamageType
//
//	Returns the type of damage that this ship does

	{
	CInstalledDevice *pWeapon = GetNamedDevice(devPrimaryWeapon);
	if (pWeapon)
		return (DamageTypes)pWeapon->GetDamageType();
	else
		return damageNone;
	}

DeviceNames CShip::GetDeviceNameForCategory (ItemCategories iCategory)

//	GetDeviceNameForCategory
//
//	Returns a named device id for the given category

	{
	switch (iCategory)
		{
		case itemcatWeapon:
			return devPrimaryWeapon;

		case itemcatLauncher:
			return devMissileWeapon;

		case itemcatShields:
			return devShields;

		case itemcatDrive:
			return devDrive;

		case itemcatCargoHold:
			return devCargo;

		case itemcatReactor:
			return devReactor;

		default:
			return devNone;
		}
	}

CXMLElement *CShip::GetDockScreen (void)

//	GetDockScreen
//
//	Returns the screen on dock (NULL if none)

	{
	if (m_pClass->HasDockingPorts())
		return m_pClass->GetScreen(NULL_STR);
	else
		return NULL;
	}

CSpaceObject *CShip::GetEscortPrincipal (void) const

//	GetEscortPrincipal
//
//	Returns the ship that is being escorted by this ship (or NULL)

	{
	return m_pController->GetEscortPrincipal();
	}

CString CShip::GetInstallationPhrase (const CItem &Item) const

//	GetInstallationPhrase
//
//	Returns a phrase that describes the installation state
//	of the item.

	{
	if (Item.IsInstalled())
		{
		switch (Item.GetType()->GetCategory())
			{
			case itemcatArmor:
				{
				int iSeg = Item.GetInstalled();
				const SPlayerSettings *pSettings = m_pClass->GetPlayerSettings();
				if (pSettings && iSeg >= 0 && iSeg < pSettings->m_iArmorDescCount)
					return strPatternSubst(CONSTLIT("Installed as %s armor"), pSettings->m_pArmorDesc[iSeg].sName.GetASCIIZPointer());
				else
					return CONSTLIT("Installed as armor");
				}

			case itemcatLauncher:
				return CONSTLIT("Installed as launcher");

			case itemcatWeapon:
				return CONSTLIT("Installed as weapon");

			case itemcatCargoHold:
				return CONSTLIT("Installed as cargo hold expansion");

			case itemcatDrive:
				return CONSTLIT("Installed as drive upgrade");

			case itemcatReactor:
				return CONSTLIT("Installed as reactor");

			case itemcatShields:
				return CONSTLIT("Installed as shield generator");

			case itemcatMiscDevice:
				return CONSTLIT("Installed as device");

			default:
				return CONSTLIT("Installed");
			}
		}

	return CString();
	}

int CShip::GetItemDeviceName (const CItem &Item) const

//	GetItemDeviceName
//
//	Returns the DeviceName of the given item. We assume that the
//	item is installed. Returns -1 if not installed.

	{
	if (Item.IsInstalled())
		{
		//	The device slot that this item is installed in is
		//	stored in the data field.

		int iSlot = Item.GetInstalled();

		//	Look to see if this slot has a name

		for (int i = devFirstName; i < devNamesCount; i++)
			if (m_NamedDevices[i] == iSlot)
				return i;
		}

	//	Not named or not installed

	return -1;
	}

int CShip::GetManeuverDelay (void)

//	GetManeuverDelay
//
//	Returns the number of ticks to wait between rotations

	{
	if (m_fTrackMass)
		{
		Metric rManeuver = m_pClass->GetManeuverability();
		Metric rExtraMass = (m_rItemMass - (Metric)m_pClass->GetHullMass()) * MANEUVER_MASS_FACTOR;
		if (m_pClass->GetHullMass() > 0 && rExtraMass > 0.0)
			rManeuver = Min(MAX_MANEUVER_DELAY, rManeuver * (1.0f + (rExtraMass / (Metric)m_pClass->GetHullMass())));

		return (int)((rManeuver / STD_SECONDS_PER_UPDATE) + 0.5);
		}
	else
		return m_pClass->GetManeuverDelay();
	}

bool CShip::FollowsObjThroughGate (CSpaceObject *pLeader)

//	FollowsObjThroughGate
//
//	Returns true if we follow the leader through a gate

	{
	return m_pController->FollowsObjThroughGate(pLeader);
	}

Metric CShip::GetMass (void)

//	GetMass
//
//	Returns the mass of the object in metric tons

	{
	return m_pClass->GetHullMass() + m_rItemMass;
	}

int CShip::GetMaxFuel (void)

//	GetMaxFuel
//
//	Return the maximum amount of fuel that the reactor can hold

	{
	return m_pReactorDesc->iMaxFuel;
	}

int CShip::GetMaxPower (void) const

//	GetMaxPower
//
//	Return max power output

	{
	if (m_fTrackFuel)
		return m_iMaxPower;
	else
		return m_pReactorDesc->iMaxPower;
	}

int CShip::GetMissileCount (void)

//	GetMissileCount
//
//	Returns the number of missile variants

	{
	CInstalledDevice *pDevice = GetNamedDevice(devMissileWeapon);
	if (pDevice)
		{
		CDeviceClass *pClass = pDevice->GetClass();
		return pClass->GetValidVariantCount(this, pDevice);
		}
	else
		return 0;
	}

CString CShip::GetName (DWORD *retdwFlags)

//	GetName
//
//	Returns the name of the ship

	{
	if (m_sName.IsBlank())
		return m_pClass->GetName(retdwFlags);
	else
		{
		if (retdwFlags)
			*retdwFlags = m_dwNameFlags;

		return m_sName;
		}
	}

CInstalledDevice *CShip::GetNamedDevice (DeviceNames iDev)
	{
	if (m_NamedDevices[iDev] == -1)
		return NULL;
	else
		return &m_Devices[m_NamedDevices[iDev]]; 
	}

CDeviceClass *CShip::GetNamedDeviceClass (DeviceNames iDev)
	{
	if (m_NamedDevices[iDev] == -1)
		return NULL;
	else
		return m_Devices[m_NamedDevices[iDev]].GetClass(); 
	}

CItem CShip::GetNamedDeviceItem (DeviceNames iDev)

//	GetNamedDeviceItem
//
//	Returns the item for the named device

	{
	ASSERT(m_NamedDevices[iDev] != -1);

	CItemListManipulator ItemList(GetItemList());
	SetCursorAtNamedDevice(ItemList, iDev);
	return ItemList.GetItemAtCursor();
	}

CVector CShip::GetNearestDockVector (CSpaceObject *pRequestingObj)

//	GetNearestDockVector
//
//	Returns a vector from the given position to the nearest
//	dock position

	{
	CVector vDistance;
	m_DockingPorts.FindNearestEmptyPort(this, pRequestingObj, &vDistance);
	return vDistance;
	}

int CShip::GetPerception (void)

//	GetPerception
//
//	Returns the ship's current perception level (including external
//	factors such as nebulae and blindness)

	{
	//	calculate perception

	int iPerception = m_pClass->GetAISettings().iPerception;

	//	If we're in a nebula, then our perception drops

	if (IsLRSBlind())
		iPerception -= 4;

	return Max((int)perceptMin, iPerception);
	}

int CShip::GetPowerConsumption (void)

//	GetPowerConsumption
//
//	Returns the amount of power consumed this turn

	{
	if (m_fTrackFuel)
		return m_iPowerDrain;
	else
		return 1;
	}

CString CShip::GetReactorName (void)

//	GetReactorName
//
//	Returns he name of the ship's reactor

	{
	CInstalledDevice *pReactor = GetNamedDevice(devReactor);
	if (pReactor)
		return pReactor->GetClass()->GetItemType()->GetNounPhrase();
	else
		return strPatternSubst(CONSTLIT("%s reactor"), m_pClass->GetShortName().GetASCIIZPointer());
	}

int CShip::GetShieldLevel (void)

//	GetShieldLevel
//
//	Returns the % shield level of the ship (or -1 if the ship has no shields)

	{
	CInstalledDevice *pShields = GetNamedDevice(devShields);
	if (pShields == NULL)
		return -1;

	int iHP, iMaxHP;
	pShields->GetStatus(this, &iHP, &iMaxHP);
	if (iMaxHP == 0)
		return -1;

	return iHP * 100 / iMaxHP;
	}

int CShip::GetStealth (void)

//	GetStealth
//
//	Returns the stealth of the ship

	{
	int iStealth = m_iStealth;

	//	+2 stealth if in nebula

	if (m_fHiddenByNebula)
		iStealth += 2;

	return Min((int)stealthMax, iStealth);
	}

CSpaceObject *CShip::GetTarget (void)

//	GetTarget
//
//	Returns the target that this ship is attacking

	{
	return m_pController->GetTarget();
	}

int CShip::GetVisibleDamage (void)

//	GetVisibleDamage
//
//	Returns the amount of damage (%) that the object has taken

	{
	int iMaxPercent = 0;

	//	Compute max and actual HP

	for (int i = 0; i < GetArmorSectionCount(); i++)
		{
		CInstalledArmor *pArmor = GetArmorSection(i);

		int iDamage = 100 - (pArmor->iHitPoints * 100 / pArmor->pArmorClass->GetHitPoints(pArmor));
		if (iDamage > iMaxPercent)
			iMaxPercent = iDamage;
		}

	//	Return % damage of the worst armor segment

	return iMaxPercent;
	}

bool CShip::GetWeaponIsReady (DeviceNames iDev)

//	GetWeaponIsReady
//
//	Returns TRUE if the weapon is ready

	{
	CInstalledDevice *pWeapon = GetNamedDevice(iDev);
	return (pWeapon && pWeapon->IsReady());
	}

CVector CShip::GetWeaponPos (void) const

//	GetWeaponPos
//
//	Returns the default fire source of the weapons. Note: This is only
//	useful for the player ship (which can install and remove weapons).
//	All other ships have weapon position encoded in the device structure.

	{
	return GetPos() 
			+ PolarToVector(GetRotation() % 360,
				20.0 * g_KlicksPerPixel);
	}

bool CShip::HasAttribute (const CString &sAttribute) const

//	HasAttribute
//
//	Returns TRUE if it has the given attribute

	{
	return ::HasModifier(m_pClass->GetAttributes(), sAttribute);
	}

bool CShip::HasNamedDevice (DeviceNames iDev) const

//	HasNamedDevice
//
//	Returns TRUE if the ship has the named device installed

	{
	return (m_NamedDevices[iDev] != -1);
	}

bool CShip::ImageInObject (const CObjectImageArray &Image, int iTick, int iRotation, CVector vPos)

//	ImageInObject
//
//	Returns TRUE if the given image at the given position intersects this
//	object

	{
	return ImagesIntersect(Image,
			iTick,
			iRotation, 
			vPos,
			m_pClass->GetImage(),
			GetSystem()->GetTick(), 
			Angle2Direction(GetRotation()), 
			GetPos());
	}

void CShip::InstallItemAsArmor (CItemListManipulator &ItemList, int iSect)

//	InstallItemAsArmor
//
//	Installs the item at the cursor as armor.

	{
	//	Find the item that is currently installed in this section
	//	and mark it as being prepared for uninstall.

	CItemListManipulator Search(GetItemList());
	bool bFound = false;
	while (Search.MoveCursorForward())
		{
		const CItem &Item = Search.GetItemAtCursor();
		if (Item.IsInstalled()
				&& Item.GetType()->GetCategory() == itemcatArmor
				&& Item.GetInstalled() == iSect)
			{
			ASSERT(Item.GetCount() == 1);
			bFound = true;
			break;
			}
		}

	ASSERT(bFound);
	if (bFound)
		Search.SetInstalledAtCursor(-2);

	//	How damaged is the current armor?

	bool bDestroyArmor = false;
	bool bDamageArmor = false;
	if (!IsArmorRepairable(iSect))
		bDestroyArmor = true;
	else if (IsArmorDamaged(iSect))
		bDamageArmor = true;

	//	Now install the selected item as new armor

	CInstalledArmor *pSect = GetArmorSection(iSect);
	const CItem &NewArmor = ItemList.GetItemAtCursor();
	pSect->pArmorClass = NewArmor.GetType()->GetArmorClass();
	ASSERT(pSect->pArmorClass);
	pSect->m_Mods = NewArmor.GetMods();
	pSect->m_fComplete = false;		//	Must be set before calling GetHitPoints()
	pSect->iHitPoints = pSect->pArmorClass->GetHitPoints(pSect);
	if (NewArmor.IsDamaged())
		pSect->iHitPoints = pSect->iHitPoints / 2;

	//	The item is now known and referenced.
	//	NOTE: This assumes that only the player calls this
	//	function.

	if (g_pUniverse->GetPlayer() == this)
		{
		NewArmor.GetType()->SetKnown();
		NewArmor.GetType()->SetShowReference();
		}

	//	Mark the item as installed

	ItemList.SetInstalledAtCursor(iSect);

	//	Look for the previous item

	Search.ResetCursor();
	bFound = false;
	while (Search.MoveCursorForward())
		{
		const CItem &Item = Search.GetItemAtCursor();
		if (Item.GetType()->GetCategory() == itemcatArmor
				&& Item.GetInstalled() == -2)
			{
			ASSERT(Item.IsInstalled());
			ASSERT(Item.GetCount() == 1);
			bFound = true;
			break;
			}
		}

	ASSERT(bFound);
	if (bFound)
		{
		CItem Item(Search.GetItemAtCursor());
		Search.DeleteAtCursor(1);
		InvalidateItemListAddRemove();

		if (!bDestroyArmor)
			{
			Item.SetInstalled(-1);
			Item.SetCount(1);
			if (bDamageArmor)
				Item.SetDamaged();
			else
				Item.ClearDamaged();

			Search.AddItem(Item);
			}
		}

	//	Restore the cursor to point at the new armor segment

	ItemList.ResetCursor();
	while (ItemList.MoveCursorForward())
		{
		const CItem &Item = ItemList.GetItemAtCursor();
		if (Item.IsInstalled()
				&& Item.GetType()->GetCategory() == itemcatArmor
				&& Item.GetInstalled() == iSect)
			break;
		}

	//	Update

	CalcArmorBonus();
	CalcDeviceBonus();
	InvalidateItemListState();
	m_pController->OnArmorRepaired(iSect);
	}

void CShip::InstallItemAsDevice (CItemListManipulator &ItemList, int iDeviceSlot)

//	InstallItemAsDevice
//
//	Installs the item at the cursor as a device

	{
	CDeviceClass *pNewDevice = ItemList.GetItemAtCursor().GetType()->GetDeviceClass();
	ASSERT(pNewDevice);

	//	If necessary, remove previous item in named slot

	DeviceNames iNamedSlot = GetDeviceNameForCategory(pNewDevice->GetCategory());
	if (iDeviceSlot == -1 
			&& iNamedSlot != devNone && iNamedSlot != devPrimaryWeapon
			&& HasNamedDevice(iNamedSlot))
		{
		//	If we're upgrading/downgrading a reactor, then remember the old fuel level

		int iOldFuel = -1;
		if (iNamedSlot == devReactor)
			iOldFuel = GetFuelLeft();

		//	Remove the item

		CItem ItemToInstall = ItemList.GetItemAtCursor();
		CItemListManipulator RemoveItem(GetItemList());
		SetCursorAtNamedDevice(RemoveItem, iNamedSlot);
		RemoveItemAsDevice(RemoveItem);
		ItemList.Refresh(ItemToInstall);

		//	Reset the fuel level (we are effectively transfering the fuel to the
		//	new reactor. Note that on a downgrade, we will clip the fuel to the
		//	maximum when we do a CalcDeviceBonus).

		if (iOldFuel != -1 && !m_fOutOfFuel)
			m_iFuelLeft = iOldFuel;
		}

	ASSERT(CanInstallDevice(ItemList) == insOK);

	//	Look for a free slot to install to

	if (iDeviceSlot == -1)
		{
		iDeviceSlot = FindFreeDeviceSlot();
		ASSERT(iDeviceSlot != -1);
		}

	CInstalledDevice *pDevice = &m_Devices[iDeviceSlot];

	//	Update the structure

	pDevice->Install(this, ItemList, iDeviceSlot);

	//	The item is now known and referenced

	if (g_pUniverse->GetPlayer() == this)
		{
		const CItem &Item = ItemList.GetItemAtCursor();
		Item.GetType()->SetKnown();
		Item.GetType()->SetShowReference();
		}

	//	Adjust the named devices

	if (pDevice->IsSecondaryWeapon())
		{
		for (int j = devTurretWeapon1; j <= devTurretWeapon6; j++)
			if (m_NamedDevices[j] == -1)
				{
				m_NamedDevices[j] = iDeviceSlot;
				break;
				}
		m_pController->OnWeaponStatusChanged();
		}
	else
		{
		switch (pDevice->GetCategory())
			{
			case itemcatWeapon:
				m_NamedDevices[devPrimaryWeapon] = iDeviceSlot;
				m_pController->OnWeaponStatusChanged();
				break;

			case itemcatLauncher:
				m_NamedDevices[devMissileWeapon] = iDeviceSlot;
				m_pController->OnWeaponStatusChanged();
				break;

			case itemcatShields:
				m_NamedDevices[devShields] = iDeviceSlot;
				//	If we just installed a shield generator, start a 0 energy
				pDevice->Reset(this);
				m_pController->OnArmorRepaired(-1);
				break;

			case itemcatDrive:
				m_NamedDevices[devDrive] = iDeviceSlot;
				SetDriveDesc(pDevice->GetDriveDesc(this));
				break;

			case itemcatCargoHold:
				m_NamedDevices[devCargo] = iDeviceSlot;
				break;

			case itemcatReactor:
				m_NamedDevices[devReactor] = iDeviceSlot;
				m_pReactorDesc = pDevice->GetReactorDesc(this);
				break;
			default:
				break;
			}
		}

	//	Recalc bonuses

	CalcDeviceBonus();
	InvalidateItemListState();
	}

bool CShip::IsAngryAt (CSpaceObject *pObj)

//	IsAngryAt
//
//	Returns true if the ship is angry at the given object

	{
	return (IsEnemy(pObj) || m_pController->GetTarget() == pObj);
	}

bool CShip::IsArmorDamaged (int iSect)

//	IsArmorDamaged
//
//	Returns TRUE if the given armor section is damaged

	{
	CInstalledArmor *pSect = GetArmorSection(iSect);
	return (pSect->iHitPoints < pSect->pArmorClass->GetHitPoints(pSect));
	}

bool CShip::IsArmorRepairable (int iSect)

//	IsArmorRepairable
//
//	Returns TRUE if the given armor section can be repaired

	{
	CInstalledArmor *pSect = GetArmorSection(iSect);
	return (pSect->iHitPoints >= (pSect->pArmorClass->GetHitPoints(pSect) / 4));
	}

bool CShip::IsDeviceSlotAvailable (void)

//	IsDeviceSlotAvailable
//
//	Returns TRUE if it is possible to install a new device

	{
	return true;
	}

bool CShip::IsFuelCompatible (const CItem &Item)

//	IsFuelCompatible
//
//	Returns TRUE if the given fuel is compatible

	{
	int iLevel = Item.GetType()->GetLevel();
	return (iLevel >= m_pReactorDesc->iMinFuelLevel 
			&& iLevel <= m_pReactorDesc->iMaxFuelLevel);
	}

bool CShip::IsPlayer (void)

//	IsPlayer
//
//	Returns TRUE if this is the player ship

	{
	return m_pController->IsPlayer();
	}

bool CShip::IsRadiationImmune (void)

//	IsRadiationImmune
//
//	Return TRUE if the ship is immune to radiation. The ship is immune if all
//	armor segments are immune.

	{
	for (int i = 0; i < GetArmorSectionCount(); i++)
		{
		CInstalledArmor *pSection = GetArmorSection(i);
		if (!pSection->pArmorClass->IsRadiationImmune(pSection))
			return false;
		}

	return true;
	}

bool CShip::IsWeaponAligned (DeviceNames iDev, CSpaceObject *pTarget, int *retiAimAngle, int *retiFireAngle)

//	IsWeaponAligned
//
//	Returns TRUE if the weapon is aligned on target

	{
	CInstalledDevice *pWeapon = GetNamedDevice(iDev);
	if (pWeapon)
		{
		int iAimAngle;
		bool bAligned = pWeapon->IsWeaponAligned(this, pTarget, &iAimAngle, retiFireAngle);

		//	Return the direction that the ship should face in order to fire.

		if (retiAimAngle)
			{
			iAimAngle = (iAimAngle + 360 - pWeapon->GetRotation()) % 360;
			*retiAimAngle = iAimAngle;
			}

		return bAligned;
		}
	else
		{
		if (retiAimAngle)
			*retiAimAngle = -1;

		if (retiFireAngle)
			*retiFireAngle = -1;

		return false;
		}
	}

void CShip::MakeBlind (int iTickCount)

//	MakeBlind
//
//	Ship is blind

	{
	bool bNotify = (m_iBlindnessTimer == 0);
	if (m_iBlindnessTimer != -1)
		{
		if (iTickCount == -1)
			m_iBlindnessTimer = -1;
		else
			m_iBlindnessTimer += iTickCount;
		}

	if (bNotify)
		m_pController->OnBlindnessChanged(true);
	}

void CShip::MakeDisarmed (int iTickCount)

//	MakeDisarmed
//
//	Ship cannot use weapons

	{
	if (m_iDisarmedTimer != -1)
		{
		if (iTickCount == -1)
			m_iDisarmedTimer = -1;
		else
			m_iDisarmedTimer += iTickCount;
		}
	}

void CShip::MakeLRSBlind (int iTickCount)

//	MakeLRSBlind
//
//	Ship is LRS blind

	{
	if (m_iLRSBlindnessTimer != -1)
		{
		if (iTickCount == -1)
			m_iLRSBlindnessTimer = -1;
		else
			m_iLRSBlindnessTimer += iTickCount;
		}
	}

void CShip::MakeParalyzed (int iTickCount)

//	MakeParalyzed
//
//	Ship is paralyzed

	{
	if (m_iParalysisTimer != -1)
		{
		if (iTickCount == -1)
			m_iParalysisTimer = -1;
		else
			m_iParalysisTimer += iTickCount;
		}
	}

void CShip::MakeRadioactive (void)

//	MakeRadioactive
//
//	Ship is radioactive

	{
	if (!m_fRadioactive)
		{
		m_iContaminationTimer = g_TicksPerSecond * 180;
		m_fRadioactive = true;
		m_pController->OnRadiationWarning(m_iContaminationTimer / g_TicksPerSecond);
		}
	}

void CShip::ObjectDestroyedHook (const SDestroyCtx &Ctx)

//	ObjectDestroyedHook
//
//	If another object got destroyed, we do something

	{
	//	If this object is registered, call the events

	if (!m_RegisteredObjects.IsEmpty()
			&& m_RegisteredObjects.FindObj(Ctx.pObj, NULL))
		{
		FireOnObjDestroyed(Ctx);
		m_RegisteredObjects.Remove(Ctx.pObj);
		}

	//	Give the controller a chance to handle it

	m_pController->OnObjDestroyed(Ctx);

	//	If what we're docked with got destroyed, clear it

	if (m_pDocked == Ctx.pObj)
		m_pDocked = NULL;

	//	If this object is docked with us, remove it from the
	//	docking table.

	if (m_pClass->HasDockingPorts())
		m_DockingPorts.OnObjDestroyed(this, Ctx.pObj);

	//	If our exit gate got destroyed, then we're OK (this can happen if
	//	a carrier gets destroyed while gunships are being launched)

	if (m_pExitGate == Ctx.pObj)
		{
		m_iExitGateTimer = 0;
		ClearCannotBeHit();
		m_pExitGate = NULL;
		}
	}

bool CShip::ObjectInObject (CSpaceObject *pObj)

//	ObjectInObject
//
//	Returns TRUE if the given object intersects this object

	{
	return pObj->ImageInObject(m_pClass->GetImage(),
			GetSystem()->GetTick(),
			Angle2Direction(GetRotation()),
			GetPos());
	}

DWORD CShip::OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2)

//	Communicate
//
//	Handle communications from other objects

	{
	if (!IsInGate())
		return m_pController->OnCommunicate(pSender, iMessage, pParam1, dwParam2);
	else
		return resNoAnswer;
	}

void CShip::OnComponentChanged (ObjectComponentTypes iComponent)

//	OnComponentChanged
//
//	Some part of the object has changed

	{
	switch (iComponent)
		{
		case comArmor:
		case comShields:
			m_pController->OnArmorRepaired(-1);
			break;

		case comCargo:
			{
			//	Calculate new mass

			m_rItemMass = CalculateItemMass(&m_rCargoMass);

			//	If one of our weapons doesn't have a variant selected, then
			//	try to select it now (if we just got some new ammo, this will
			//	select the ammo)

			int i;
			for (i = devPrimaryWeapon; i <= devTurretWeapon6; i++)
				if (m_NamedDevices[i] != -1)
					{
					CInstalledDevice *pWeapon = GetNamedDevice((DeviceNames)i);
					if (!pWeapon->IsVariantSelected(this))
						pWeapon->SelectFirstVariant(this);
					}

			//	Update weapons display (in case it changed)

			m_pController->OnWeaponStatusChanged();
			break;
			}

		case comDrive:
			{
			CInstalledDevice *pDrive = GetNamedDevice(devDrive);
			if (pDrive && pDrive->IsEnabled())
				SetDriveDesc(pDrive->GetDriveDesc(this));
			else
				SetDriveDesc(NULL);
			break;
			}

		case comReactor:
			{
			CInstalledDevice *pReactor = GetNamedDevice(devReactor);
			if (pReactor)
				m_pReactorDesc = pReactor->GetReactorDesc(this);
			else
				m_pReactorDesc = m_pClass->GetReactorDesc();
			break;
			}

		default:
			m_pController->OnComponentChanged(iComponent);
		}
	}

bool CShip::OnDestroyCheck (DestructionTypes iCause, CSpaceObject *pCause)

//	OnDestroyCheck
//
//	Checks to see if any devices/powers prevent us from being destroyed.
//	Returns TRUE if the ship is destroyed; FALSE otherwise

	{
	//	Check to see if the controller can prevent the destruction
	//	(For the player, this will invoke Domina powers, if possible)

	if (!m_pController->OnDestroyCheck(iCause, pCause))
		return false;

	return true;
	}

void CShip::OnDeviceFailure (CInstalledDevice *pDev, int iFailureType)

//	OnDeviceFailure
//
//	Called when a device fails in some way

	{ 
	m_pController->OnDeviceFailure(pDev, iFailureType);
	}

void CShip::OnDestroyed (DestructionTypes iCause, CSpaceObject *pCause, bool *ioResurrectPending, CSpaceObject **retpWreck)

//	OnDestroyed
//
//	Ship has been destroyed

	{
	CSpaceObject *pAttacker = NULL;
	if (pCause)
		pAttacker = pCause->GetDamageCause();

	//	Figure out if we're creating a wreck or not

	bool bCreateWreck = (iCause != removedFromSystem)
			&& (iCause != killedByDisintegration)
			&& (iCause != killedByShatter)
			&& (*ioResurrectPending
				|| iCause == killedByRadiationPoisoning
				|| iCause == killedByRunningOutOfFuel
				|| mathRandom(1, 100) <= m_pClass->GetWreckChance());

	//	Create wreck (Note: CreateWreck may not create a wreck
	//	because not all destructions leave a wreck. In that case,
	//	pWreck will be NULL).

	CSpaceObject *pWreck = NULL;
	if (bCreateWreck)
		m_pClass->CreateWreck(this, &pWreck);

	*retpWreck = pWreck;

	//	Run OnDestroy script

	SDestroyCtx Ctx;
	Ctx.pObj = this;
	Ctx.pDestroyer = pAttacker;
	Ctx.pWreck = pWreck;
	Ctx.iCause = iCause;

	FireOnDestroy(Ctx);

	//	Tell controller

	m_pController->OnDestroyed(iCause, pCause, ioResurrectPending);

	//	Tell the controller that we created a wreck (Note: I'm not sure if
	//	this needs to be here--maybe we can move this to when the wreck
	//	gets created.

	if (pWreck)
		m_pController->OnWreckCreated(pWreck);

	//	Release any docking objects

	if (m_pClass->HasDockingPorts())
		m_DockingPorts.OnDestroyed();

	//	Create an effect appropriate to the cause of death

	switch (iCause)
		{
		case removedFromSystem:
			//	No effect
			break;

		case killedByRadiationPoisoning:
		case killedByRunningOutOfFuel:
			//	No effect
			break;

		case killedByDisintegration:
			CDisintegrationEffect::Create(GetSystem(),
					GetPos(),
					GetVel(),
					m_pClass->GetImage(),
					0,
					Angle2Direction(GetRotation()),
					NULL);
			break;

		case killedByShatter:
			{
			int iTick = GetSystem()->GetTick();
			if (IsTimeStopped())
				iTick = GetDestiny();

			CFractureEffect::Create(GetSystem(),
					GetPos(),
					GetVel(),
					GetImage(),
					iTick,
					Angle2Direction(GetRotation()),
					CFractureEffect::styleExplosion,
					NULL);

			g_pUniverse->PlaySound(this, g_pUniverse->FindSound(g_ShipExplosionSoundUNID));
			break;
			}

		default:
			m_pClass->CreateExplosion(this, pWreck);
		}
	}

void CShip::OnDocked (CSpaceObject *pObj)

//	OnDocked
//
//	Object has docked

	{
	m_pDocked = pObj;

	//	If we've docked with a radioactive object then we become radioactive
	//	unless our armor is immune

	if (pObj->IsRadioactive() && !IsRadiationImmune())
		MakeRadioactive();

	//	Tell our controller

	m_pController->OnDocked(pObj);
	}

void CShip::OnDockedObjChanged (void)

//	OnDockedObjChanged
//
//	This is called when the object that we're currently docked
//	with has changed the items that it has

	{
	m_pController->OnDockedObjChanged();
	}

CSpaceObject *CShip::OnGetOrderGiver (void)

//	GetOrderGiver
//
//	Returns the object that is the ultimate order giver
//	(e.g., for an auton, the order giver is the ship that controls
//	the auton).

	{
	return m_pController->GetOrderGiver();
	}

void CShip::OnItemsModified (void)

//	OnItemsModified
//
//	Items on the station have been modified by some external
//	factor (not the player)

	{
	//	Tell all the ships that are docked here that items have been modified

	m_DockingPorts.OnOwnerChanged(this);
	}

void CShip::OnMove (const CVector &vOldPos, Metric rSeconds)

//	OnMove
//
//	Do stuff when station moves

	{
	//	If the station is moving then make sure all docked ships
	//	move along with it.

	if (m_pClass->HasDockingPorts())
		m_DockingPorts.MoveAll(this);
	}

void CShip::OnObjDocked (CSpaceObject *pObj, CSpaceObject *pDockTarget)

//	OnObjDocked
//
//	The given object has docked
	
	{
	if (!m_RegisteredObjects.IsEmpty()
			&& m_RegisteredObjects.FindObj(pObj))
		FireOnObjDocked(pObj, pDockTarget);
	}

void CShip::OnObjEnteredGate (CSpaceObject *pObj, CSpaceObject *pStargate)

//	OnObjEnteredGate
//
//	This is called when another object enters a stargate

	{
	m_pController->OnObjEnteredGate(pObj, pStargate);

	//	Fire events

	if (!m_RegisteredObjects.IsEmpty() 
			&& m_RegisteredObjects.FindObj(pObj))
		FireOnObjEnteredGate(pObj, pStargate);
	}

void CShip::OnObjJumped (CSpaceObject *pObj)

//	OnObjJumped
//
//	This is called when another object jumps within the system

	{
	//	Fire events

	if (!m_RegisteredObjects.IsEmpty() 
			&& m_RegisteredObjects.FindObj(pObj))
		FireOnObjJumped(pObj);
	}

void CShip::OnObjReconned (CSpaceObject *pObj)

//	OnObjReconned
//
//	This is called when another object has been reconned

	{
	//	Fire events

	if (!m_RegisteredObjects.IsEmpty() 
			&& m_RegisteredObjects.FindObj(pObj))
		FireOnObjReconned(pObj);
	}

void CShip::OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	OnPaint
//
//	Paint the ship

	{
	if (IsInGate())
		return;

	int iTick = GetSystem()->GetTick();
	if (IsTimeStopped())
		iTick = GetDestiny();

	m_pClass->Paint(Dest, 
			x, 
			y, 
			Ctx.XForm, 
			Angle2Direction(GetRotation()), 
			iTick,
			m_pController->GetThrust() && !IsParalyzed(),
			IsRadioactive()
			);

	if (IsHighlighted() && !Ctx.fNoSelection)
		{
		const CObjectImageArray *pImage;
		pImage = &m_pClass->GetImage();
		PaintHighlight(Dest, pImage->GetImageRectAtPoint(x, y));
		}

	//	Paint energy fields

	Ctx.iTick = iTick;
	Ctx.iVariant = 0;
	Ctx.iDestiny = GetDestiny();
	m_EnergyFields.Paint(Dest, x, y, Ctx);

	//	Paint effects

	PaintEffects(Dest, x, y, Ctx);

	//	If paralyzed, draw energy arcs

	if (IsParalyzed() || m_iDisarmedTimer > 0)
		{
		Metric rSize = (Metric)RectWidth(m_pClass->GetImage().GetImageRect()) / 2;
		for (int i = 0; i < PARALYSIS_ARC_COUNT; i++)
			{
			//	Compute the beginning of this arc

			int iAngle = ((GetDestiny() + iTick) * (15 + i * 7)) % 360;
			Metric rRadius = rSize * (((GetDestiny() + iTick) * (i * 3716)) % 1000) / 1000.0;
			CVector vFrom(PolarToVector(iAngle, rRadius));

			//	Compute the end of the arc

			iAngle = ((GetDestiny() + iTick + 1) * (15 + i * 7)) % 360;
			rRadius = rSize * (((GetDestiny() + iTick + 1) * (i * 3716)) % 1000) / 1000.0;
			CVector vTo(PolarToVector(iAngle, rRadius));

			//	Draw

			DrawLightning(Dest,
					(int)vFrom.GetX() + x,
					(int)vFrom.GetY() + y,
					(int)vTo.GetX() + x,
					(int)vTo.GetY() + y,
					CG16bitImage::RGBValue(0x00, 0xa9, 0xff),
					16,
					0.4);
			}
		}

	if (m_pClass->HasDockingPorts())
		m_fKnown = true;

	//	Identified

	m_fIdentified = true;
	}

void CShip::OnPlayerObj (CSpaceObject *pPlayer)

//	OnPlayerObj
//
//	Player has entered the system

	{
	m_pController->OnPlayerObj(pPlayer);
	}

void CShip::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read object data from a stream
//
//	DWORD		Class UNID
//	DWORD		m_pSovereign (CSovereign ref)
//	CString		m_sName;
//	DWORD		m_dwNameFlags
//	DWORD		low = m_iRotation; hi = m_iPrevRotation
//	DWORD		low = m_iFireDelay; hi = m_iMissileFireDelay
//	DWORD		low = m_iManeuverDelay; hi = m_iContaminationTimer
//	DWORD		low = m_iBlindnessTimer; hi = m_iParalysisTimer
//	DWORD		low = m_iExitGateTimer; hi = m_iDisarmedTimer
//	DWORD		low = m_iLRSBlindnessTimer;
//	DWORD		m_iFuelLeft
//	Metric		m_rItemMass (V2)
//	Metric		m_rCargoMass
//	DWORD		m_pDocked (CSpaceObject ref)
//	DWORD		m_pExitGate (CSpaceObject ref)
//	DWORD		flags
//
//	DWORD		Number of armor structs
//	DWORD		armor: class UNID
//	DWORD		armor: hit points
//	DWORD		armor: mods
//	DWORD		armor: flags
//
//	DWORD		m_iStealth
//
//	DWORD		Number of devices
//	DWORD		device: class UNID
//	DWORD		device: m_dwData
//	DWORD		device: low = m_iPosAngle; hi = m_iPosRadius
//	DWORD		device: low = m_iMinFireArc; hi = m_iMaxFireArc
//	DWORD		device: low = m_iActivationDelay
//	DWORD		device: flags
//
//	DWORD		drivedesc UNID
//
//	DWORD		No of energy fields
//	DWORD		field: type UNID
//	DWORD		field: iLifeLeft
//	IEffectPainter field: pPainter
//
//	CDockingPorts
//
//	DWORD		No of registered objects
//	DWORD		registered object (CSpaceObject ref)
//	DWORD		encounter info (CStationType UNID) (V2)
//
//	DWORD		Controlled ObjID
//	Controller Data

	{
#ifdef DEBUG_LOAD
	::OutputDebugString("CShip::OnReadFromStream\n");
#endif
	int i;
	DWORD dwLoad;

	//	Load class

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_pClass = g_pUniverse->FindShipClass(dwLoad);

	//	In previous versions there was a bug in which some effects were not
	//	read properly. In those cases, the class UNID is invalid.
	//	If m_pClass is indeed NULL, then skip forward until we find
	//	a valid class

	if (m_pClass == NULL && Ctx.dwVersion < 15)
		{
		int iCount = 20;
		while (iCount-- > 0 && m_pClass == NULL)
			{
			Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
			m_pClass = g_pUniverse->FindShipClass(dwLoad);
			}
		}

	//	Cache a bit for this event

	SetHasOnObjDockedEvent(m_pClass->HasOnObjDockedEvent());
	SetHasOnAttackedEvent(m_pClass->FindEventHandler(CONSTLIT("OnAttacked")));
	SetHasOnDamageEvent(m_pClass->FindEventHandler(CONSTLIT("OnDamage")));

	//	Load misc stuff

	Ctx.pSystem->ReadSovereignRefFromStream(Ctx, &m_pSovereign);

	//	Load name

	m_sName.ReadFromStream(Ctx.pStream);
	if (Ctx.dwVersion >= 26)
		Ctx.pStream->Read((char *)&m_dwNameFlags, sizeof(DWORD));
	else
		{
		if (!m_sName.IsBlank())
			m_dwNameFlags = m_pClass->GetShipNameFlags();
		else
			m_dwNameFlags = 0;
		}

	//	Load more

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iRotation = (int)LOWORD(dwLoad);
	m_iPrevRotation = (int)HIWORD(dwLoad);
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iFireDelay = (int)LOWORD(dwLoad);
	m_iMissileFireDelay = (int)HIWORD(dwLoad);
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iManeuverDelay = (int)LOWORD(dwLoad);
	m_iContaminationTimer = (int)HIWORD(dwLoad);
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iBlindnessTimer = (int)LOWORD(dwLoad);
	m_iParalysisTimer = (int)HIWORD(dwLoad);
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iExitGateTimer = (int)LOWORD(dwLoad);
	m_iDisarmedTimer = (int)HIWORD(dwLoad);
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_iLRSBlindnessTimer = (int)LOWORD(dwLoad);
	Ctx.pStream->Read((char *)&m_iFuelLeft, sizeof(DWORD));
	if (Ctx.dwVersion >= 2)
		Ctx.pStream->Read((char *)&m_rItemMass, sizeof(Metric));
	Ctx.pStream->Read((char *)&m_rCargoMass, sizeof(Metric));
	Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_pDocked);
	Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_pExitGate);

	//	Load flags

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_fOutOfFuel =				((dwLoad & 0x00000001) ? true : false);
	m_fRadioactive =			((dwLoad & 0x00000002) ? true : false);
	m_fHasAutopilot =			((dwLoad & 0x00000004) ? true : false);
	m_fDestroyInGate =			((dwLoad & 0x00000008) ? true : false);
	m_fHalfSpeed =				((dwLoad & 0x00000010) ? true : false);
	m_fHasTargetingComputer =	((dwLoad & 0x00000020) ? true : false);
	m_fTrackFuel =				((dwLoad & 0x00000040) ? true : false);
	m_fHasSecondaryWeapons =	((dwLoad & 0x00000080) ? true : false);
	m_fSRSEnhanced =			((dwLoad & 0x00000100) ? true : false);
	m_fFollowPlayerThroughGate =((dwLoad & 0x00000200) ? true : false);
	m_fKnown =					((dwLoad & 0x00000400) ? true : false);
	m_fHiddenByNebula =			((dwLoad & 0x00000800) ? true : false);
	m_fTrackMass =				((dwLoad & 0x00001000) ? true : false);
	if (Ctx.dwVersion >= 14)
		m_fIdentified =			((dwLoad & 0x00002000) ? true : false);
	else
		m_fIdentified = true;

	//	Load armor

	DWORD dwCount;
	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		{
		CInstalledArmor Armor;

		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		Armor.pArmorClass = g_pUniverse->FindArmor(dwLoad);
		Ctx.pStream->Read((char *)&Armor.iHitPoints, sizeof(DWORD));
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		Armor.m_Mods = dwLoad;

		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		Armor.m_fComplete =		((dwLoad & 0x00000001) ? true : false);

		m_Armor.AppendStruct(&Armor, NULL);
		}

	//	Stealth

	if (Ctx.dwVersion >= 5)
		Ctx.pStream->Read((char *)&m_iStealth, sizeof(DWORD));
	else
		m_iStealth = stealthNormal;

	//	Initialize named devices

	for (int j = devFirstName; j < devNamesCount; j++)
		m_NamedDevices[j] = -1;

	//	Load devices

	if (Ctx.dwVersion >= 7)
		Ctx.pStream->Read((char *)&m_iDeviceCount, sizeof(DWORD));
	else
		{
		CDeviceDescList Devices;
		m_pClass->GenerateDevices(Devices);
		m_iDeviceCount = Max(Devices.GetCount(), m_pClass->GetMaxDevices());
		}

	m_Devices = new CInstalledDevice [m_iDeviceCount];
	for (i = 0; i < m_iDeviceCount; i++)
		{
		m_Devices[i].ReadFromStream(Ctx);

		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		if (dwLoad != 0xffffffff)
			m_NamedDevices[dwLoad] = i;

		if (Ctx.dwVersion < 29)
			m_Devices[i].SetDeviceSlot(i);
		}

	//	Drive desc

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	if (m_NamedDevices[devDrive] != -1 && m_Devices[m_NamedDevices[devDrive]].IsEnabled())
		SetDriveDesc(m_Devices[m_NamedDevices[devDrive]].GetDriveDesc(this));
	else
		SetDriveDesc(NULL);

	//	Engine core

	if (m_NamedDevices[devReactor] != -1)
		m_pReactorDesc = m_Devices[m_NamedDevices[devReactor]].GetReactorDesc(this);
	else
		m_pReactorDesc = m_pClass->GetReactorDesc();

	//	Energy fields

	m_EnergyFields.ReadFromStream(Ctx);

	//	Docking ports and registered objects

	m_DockingPorts.ReadFromStream(this, Ctx);
	m_RegisteredObjects.ReadFromStream(Ctx);

	//	Encounter UNID

	if (Ctx.dwVersion >= 2)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		if (dwLoad)
			m_pEncounterInfo = g_pUniverse->FindStationType(dwLoad);
		}

	//	Calculate item mass, if appropriate. We do this for previous versions
	//	that did not store item mass.
	
	if (Ctx.dwVersion < 2)
		m_rItemMass = CalculateItemMass(&m_rCargoMass);

	//	Controller

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad)
		{
		m_pController = dynamic_cast<IShipController *>(CObjectClassFactory::Create((OBJCLASSID)dwLoad));
		m_pController->ReadFromStream(Ctx);
		}
	}

void CShip::OnStationDestroyed (const SDestroyCtx &Ctx)

//	OnStationDestroyed
//
//	Station in the system has been destroyed

	{
	m_pController->OnStationDestroyed(Ctx);

	//	Fire events

	int i;
	if (!m_RegisteredObjects.IsEmpty()
			&& m_RegisteredObjects.FindObj(Ctx.pObj, &i))
		{
		FireOnObjDestroyed(Ctx);
		m_RegisteredObjects.Remove(Ctx.pObj);
		}
	}

void CShip::OnSystemCreated (void)

//	OnSystemCreated
//
//	The system has just been created

	{
	//	Fire OnCreate

	FireOnCreate();
	}

void CShip::OnSystemLoaded (void)

//	OnSystemLoaded
//
//	The system has just been loaded

	{
	m_pController->OnSystemLoaded(); 
	}

void CShip::OnUpdate (Metric rSecondsPerTick)

//	OnUpdate
//
//	Update

	{
	int i;
	bool bWeaponStatusChanged = false;
	bool bArmorStatusChanged = false;
	bool bCalcDeviceBonus = false;
	bool bCargoChanged = false;

	//	If we passed through a gate and are following the player, then
	//	remove ourselves from the system (when the player entered the gate
	//	we added ourselves to a list of henchmen).

	if (m_fFollowPlayerThroughGate)
		{
		m_fFollowPlayerThroughGate = false;
		m_pExitGate = NULL;
		m_iExitGateTimer = 0;
		Remove(removedFromSystem, NULL);
		return;
		}

	//	If we're in a gate, then all we do is update the in gate timer

	if (IsInGate())
		{
		//	Gate effect

		if (m_iExitGateTimer == GATE_ANIMATION_LENGTH)
			if (m_pExitGate)
				m_pExitGate->OnObjLeaveGate(this);

		//	Done?

		if (--m_iExitGateTimer == 0)
			{
			ClearCannotBeHit();
			m_pExitGate = NULL;
			}

		return;
		}

	//	If we passed through a gate, then destroy ourselves

	if (m_fDestroyInGate)
		{
		Destroy(removedFromSystem, NULL);
		return;
		}

	//	Allow docking

	if (m_pClass->HasDockingPorts())
		m_DockingPorts.UpdateAll(this);

	//	Initialize

	int iTick = GetSystem()->GetTick() + GetDestiny();
	m_iPrevRotation = m_iRotation;

	//	Check controls

	if (!IsParalyzed())
		{
		//	See if we're firing. Note that we fire before we rotate so that the
		//	fire behavior code can know which way we're aiming.

		if (m_iDisarmedTimer == 0)
			{
			for (i = devPrimaryWeapon; i <= devTurretWeapon6; i++)
				if (m_NamedDevices[i] != -1
						&& m_pController->GetWeaponFire(i))
					{
					CInstalledDevice *pWeapon = GetNamedDevice((DeviceNames)i);
					if (pWeapon->IsReady())
						{
						bool bSourceDestroyed = false;
						bool bConsumedItems = false;

						pWeapon->Activate(this, 
								m_pController->GetWeaponTarget(i),
								-1,
								&bSourceDestroyed,
								&bConsumedItems);
						if (bSourceDestroyed)
							return;

						if (bConsumedItems)
							{
							bWeaponStatusChanged = true;
							bCargoChanged = true;
							}

						//	Set delay for next activation

						SetFireDelay(pWeapon);
						}
					}
			}
		else
			m_iDisarmedTimer--;

		//	See if we're rotating

		if (m_pController->GetManeuver() != IShipController::NoRotation
				&& m_iManeuverDelay == 0)
			{
			if (m_pController->GetManeuver() == IShipController::RotateLeft)
				m_iRotation += m_pClass->GetRotationAngle();
			else
				m_iRotation += 360 - m_pClass->GetRotationAngle();

			m_iRotation = m_iRotation % 360;
			m_iManeuverDelay = GetManeuverDelay();
			}

		//	See if we're accelerating

		if (IsInertialess())
			{
			if (m_pController->GetThrust())
				{
				CVector vVel = PolarToVector(GetRotation(), GetMaxSpeed());
				SetVel(vVel);
				}
			else if (CanBeControlled())
				ClipSpeed(0.0);
			}
		else if (m_pController->GetThrust())
			{
			CVector vAccel = PolarToVector(GetRotation(), GetThrust());

			Accelerate(vAccel, rSecondsPerTick);

			//	Check to see if we're exceeding the ship's speed limit. If we
			//	are then adjust the speed

			ClipSpeed(GetMaxSpeed());
			}
		else if (m_pController->GetStopThrust())
			{
			//	Stop thrust is proportional to main engine thrust and maneuverability

			Metric rManeuverDelay = Max(2.0, (Metric)GetManeuverDelay());
			Metric rThrust = GetThrust() / rManeuverDelay;

			AccelerateStop(rThrust, rSecondsPerTick);
			}
		}

	//	If we're paralyzed, rotate in one direction

	else
		{
		//	Rotate wildly

		if (m_iManeuverDelay == 0 && m_pDocked == NULL)
			{
			if (GetDestiny() % 2)
				m_iRotation += m_pClass->GetRotationAngle();
			else
				m_iRotation += 360 - m_pClass->GetRotationAngle();

			m_iRotation = m_iRotation % 360;
			m_iManeuverDelay = GetManeuverDelay();
			}

		//	Slow down

		SetVel(CVector(GetVel().GetX() * g_SpaceDragFactor, GetVel().GetY() * g_SpaceDragFactor));

		m_iParalysisTimer--;
		}

	//	Update armor

	if ((iTick % ARMOR_UPDATE_CYCLE) == 0)
		{
		for (i = 0; i < GetArmorSectionCount(); i++)
			{
			bool bModified;
			CInstalledArmor *pArmor = GetArmorSection(i);
			pArmor->pArmorClass->Update(pArmor, this, iTick, &bModified);
			if (bModified)
				bArmorStatusChanged = true;
			}
		}

	//	Update each device

	for (i = 0; i < GetDeviceCount(); i++)
		{
		bool bSourceDestroyed = false;
		bool bConsumedItems = false;
		m_Devices[i].Update(this, iTick, &bSourceDestroyed, &bConsumedItems);
		if (bSourceDestroyed)
			return;

		if (bConsumedItems)
			{
			bWeaponStatusChanged = true;
			bCargoChanged = true;
			}
		}

	//	Update reactor

	if (m_fTrackFuel)
		{
		if (!m_fOutOfFuel)
			{
			CalcReactorStats();

			//	See if reactor is overloaded

			if ((iTick % FUEL_CHECK_CYCLE) == 0)
				{
				if (m_iPowerDrain > m_iMaxPower)
					{
					m_pController->OnReactorOverloadWarning(iTick / FUEL_CHECK_CYCLE);

					//	Consequences of reactor overload

					ReactorOverload();
					}
				}

			//	Consume fuel

			ConsumeFuel(m_iPowerDrain / m_pReactorDesc->iPowerPerFuelUnit);

			//	Check to see if we've run out of fuel

			if ((iTick % FUEL_CHECK_CYCLE) == 0)
				{
				int iFuelLeft = GetFuelLeft();
				if (iFuelLeft == 0)
					{
					//	See if the player has any fuel on board. If they do, then there
					//	is a small grace period

					if (HasFuelItem())
						{
						//	Disable all devices

						for (i = 0; i < GetDeviceCount(); i++)
							{
							if (!m_Devices[i].IsEmpty() 
									&& m_Devices[i].IsEnabled()
									&& m_Devices[i].CanBeDisabled())
								{
								EnableDevice(i, false);
								}
							}

						//	Out of fuel

						m_fOutOfFuel = true;
						m_iFuelLeft = FUEL_GRACE_PERIOD;
						m_pController->OnFuelLowWarning(-1);
						}

					//	Otherwise, the player is out of luck

					else
						{
						Destroy(killedByRunningOutOfFuel, NULL);

						//	Shouldn't do anything else after being destroyed
						return;
						}
					}
				else if (iFuelLeft < (GetMaxFuel() / 8))
					m_pController->OnFuelLowWarning(iTick / FUEL_CHECK_CYCLE);
				}
			}
		else
			{
			//	Countdown grace period

			if (--m_iFuelLeft <= 0)
				{
				Destroy(killedByRunningOutOfFuel, NULL);

				//	Shouldn't do anything else after being destroyed

				return;
				}
			else
				m_pController->OnLifeSupportWarning(m_iFuelLeft / g_TicksPerSecond);
			}
		}

	//	Radiation

	if (m_fRadioactive)
		{
		m_iContaminationTimer--;
		if (m_iContaminationTimer > 0)
			{
			if ((iTick % 10) == 0)
				m_pController->OnRadiationWarning(m_iContaminationTimer / g_TicksPerSecond);
			}
		else
			{
			Destroy(killedByRadiationPoisoning, NULL);

			//	Shouldn't do anything else after being destroyed
			return;
			}
		}

	//	Blindness

	if (m_iBlindnessTimer > 0)
		if (--m_iBlindnessTimer == 0)
			m_pController->OnBlindnessChanged(false);

	//	LRS blindness

	if (m_iLRSBlindnessTimer > 0)
		--m_iLRSBlindnessTimer;

	//	Energy fields

	bool bModified;
	m_EnergyFields.Update(&bModified);
	if (bModified)
		{
		bWeaponStatusChanged = true;
		bArmorStatusChanged = true;
		bCalcDeviceBonus = true;
		}

	//	Check space environment

	if ((iTick % SPACE_ENV_CYCLE) == 0)
		{
		CSpaceEnvironmentType *pEnvironment = GetSystem()->GetSpaceEnvironment(GetPos());
		if (pEnvironment)
			{
			//	See if our LRS is affected

			if (pEnvironment->IsLRSJammer())
				{
				MakeLRSBlind(SPACE_ENV_CYCLE);
				m_fHiddenByNebula = true;
				}

			//	See if the environment causes drag

			Metric rDrag = pEnvironment->GetDragFactor();
			if (rDrag != 1.0)
				{
				SetVel(CVector(GetVel().GetX() * rDrag,
						GetVel().GetY() * rDrag));
				}
			}
		else
			{
			m_fHiddenByNebula = false;
			}
		}

	//	Invalidate

	if (bCalcDeviceBonus)
		CalcDeviceBonus();

	if (bWeaponStatusChanged)
		m_pController->OnWeaponStatusChanged();

	if (bArmorStatusChanged)
		m_pController->OnArmorRepaired(-1);

	if (m_fTrackMass && bCargoChanged)
		OnComponentChanged(comCargo);

	if (bCargoChanged)
		InvalidateItemListAddRemove();

	//	Update

	if (m_iFireDelay > 0)
		m_iFireDelay--;

	if (m_iMissileFireDelay > 0)
		m_iMissileFireDelay--;

	if (m_iManeuverDelay > 0)
		m_iManeuverDelay--;
	}

void CShip::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write the object's data to stream
//
//	DWORD		Class UNID
//	DWORD		m_pSovereign (CSovereign ref)
//	CString		m_sName
//	DWORD		m_dwNameFlags
//	DWORD		low = m_iRotation; hi = m_iPrevRotation
//	DWORD		low = m_iFireDelay; hi = m_iMissileFireDelay
//	DWORD		low = m_iManeuverDelay; hi = m_iContaminationTimer
//	DWORD		low = m_iBlindnessTimer; hi = m_iParalysisTimer
//	DWORD		low = m_iExitGateTimer; hi = m_iDisarmedTimer
//	DWORD		low = m_iLRSBlindnessTimer;
//	DWORD		m_iFuelLeft
//	Metric		m_rItemMass
//	Metric		m_rCargoMass
//	DWORD		m_pDocked (CSpaceObject ref)
//	DWORD		m_pExitGate (CSpaceObject ref)
//	DWORD		flags
//
//	DWORD		Number of armor structs
//	DWORD		armor: class UNID
//	DWORD		armor: hit points
//	DWORD		armor: mods
//	DWORD		armor: flags
//
//	DWORD		m_iStealth
//
//	DWORD		device: class UNID
//	DWORD		device: m_dwData
//	DWORD		device: low = m_iPosAngle; hi = m_iPosRadius
//	DWORD		device: low = m_iMinFireArc; hi = m_iMaxFireArc
//	DWORD		device: low = m_iActivationDelay
//	DWORD		device: flags
//	DWORD		named device index (0xffffffff if not named)
//
//	DWORD		drivedesc UNID
//	
//	DWORD		No of energy fields
//	DWORD		field: type UNID
//	DWORD		field: iLifeLeft
//
//	CDockingPorts
//
//	DWORD		No of registered objects
//	DWORD		registered object (CSpaceObject ref)
//	DWORD		encounter info (CStationType UNID)
//
//	DWORD		Controller ObjID
//	Controller Data

	{
	int i;
	DWORD dwSave;

	dwSave = m_pClass->GetUNID();
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	GetSystem()->WriteSovereignRefToStream(m_pSovereign, pStream);

	m_sName.WriteToStream(pStream);
	pStream->Write((char *)&m_dwNameFlags, sizeof(DWORD));

	dwSave = MAKELONG(m_iRotation, m_iPrevRotation);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	dwSave = MAKELONG(m_iFireDelay, m_iMissileFireDelay);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	dwSave = MAKELONG(m_iManeuverDelay, m_iContaminationTimer);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	dwSave = MAKELONG(m_iBlindnessTimer, m_iParalysisTimer);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	dwSave = MAKELONG(m_iExitGateTimer, m_iDisarmedTimer);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	dwSave = MAKELONG(m_iLRSBlindnessTimer, 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	pStream->Write((char *)&m_iFuelLeft, sizeof(DWORD));
	pStream->Write((char *)&m_rItemMass, sizeof(Metric));
	pStream->Write((char *)&m_rCargoMass, sizeof(Metric));
	GetSystem()->WriteObjRefToStream(m_pDocked, pStream);
	GetSystem()->WriteObjRefToStream(m_pExitGate, pStream);

	dwSave = 0;
	dwSave |= (m_fOutOfFuel ?			0x00000001 : 0);
	dwSave |= (m_fRadioactive ?			0x00000002 : 0);
	dwSave |= (m_fHasAutopilot ?		0x00000004 : 0);
	dwSave |= (m_fDestroyInGate ?		0x00000008 : 0);
	dwSave |= (m_fHalfSpeed ?			0x00000010 : 0);
	dwSave |= (m_fHasTargetingComputer ? 0x00000020 : 0);
	dwSave |= (m_fTrackFuel ?			0x00000040 : 0);
	dwSave |= (m_fHasSecondaryWeapons ?	0x00000080 : 0);
	dwSave |= (m_fSRSEnhanced ?			0x00000100 : 0);
	dwSave |= (m_fFollowPlayerThroughGate ? 0x00000200 : 0);
	dwSave |= (m_fKnown ?				0x00000400 : 0);
	dwSave |= (m_fHiddenByNebula ?		0x00000800 : 0);
	dwSave |= (m_fTrackMass ?			0x00001000 : 0);
	dwSave |= (m_fIdentified ?			0x00002000 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Armor

	dwSave = GetArmorSectionCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	for (i = 0; i < GetArmorSectionCount(); i++)
		{
		CInstalledArmor *pArmor = GetArmorSection(i);
		dwSave = pArmor->pArmorClass->GetUNID();
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		pStream->Write((char *)&pArmor->iHitPoints, sizeof(DWORD));
		dwSave = pArmor->m_Mods;
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		dwSave = 0;
		dwSave |= (pArmor->m_fComplete ?	0x00000001 : 0);
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		}

	//	Stealth

	pStream->Write((char *)&m_iStealth, sizeof(DWORD));

	//	Devices

	dwSave = GetDeviceCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	for (i = 0; i < GetDeviceCount(); i++)
		{
		m_Devices[i].WriteToStream(pStream);

		int j;
		for (j = devFirstName; j < devNamesCount; j++)
			if (m_NamedDevices[j] == i)
				break;

		dwSave = ((j == devNamesCount) ? 0xffffffff : j);
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		}

	pStream->Write((char *)&m_pDriveDesc->dwUNID, sizeof(DWORD));

	//	Energy fields

	m_EnergyFields.WriteToStream(pStream);

	//	Docking ports and registered objects

	m_DockingPorts.WriteToStream(this, pStream);
	m_RegisteredObjects.WriteToStream(GetSystem(), pStream);

	//	Encounter info

	dwSave = (m_pEncounterInfo ? m_pEncounterInfo->GetUNID() : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Controller

	if (m_pController)
		m_pController->WriteToStream(pStream);
	else
		{
		dwSave = 0;
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		}
	}

bool CShip::OrientationChanged (void)

//	OrientationChanged
//
//	Returns TRUE if the object's orientation changed this tick

	{
	return (m_iRotation != m_iPrevRotation);
	}

void CShip::PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans)

//	PaintLRS
//
//	Paints the object on an LRS

	{
	if (IsInGate())
		return;

	//	Paint red if enemy, blue otherwise

	COLORREF wColor;
	if (IsEnemy(GetUniverse()->GetPOV()))
		wColor = CG16bitImage::RGBValue(255, 0, 0);
	else if (GetUniverse()->GetPOV() == this)
		wColor = CG16bitImage::RGBValue(255, 255, 255);
	else
		wColor = CG16bitImage::RGBValue(0, 255, 0);

	Dest.DrawDot(x, y, 
			wColor, 
			CG16bitImage::markerSmallRound);

	//	Identified

	m_fIdentified = true;
	}

void CShip::PaintMap (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans)

//	Paint
//
//	Paint the ship

	{
	if (IsInGate())
		return;

	//	Do not paint ships on the map unless we are the point of view

	if (GetUniverse()->GetPOV() == this)
		{
		m_pClass->PaintMap(Dest, 
				x, 
				y, 
				Trans, 
				Angle2Direction(GetRotation()), 
				GetSystem()->GetTick(),
				m_pController->GetThrust() && !IsParalyzed(),
				IsRadioactive()
				);
		}

	//	Or if it has docking services and the player knows about it

	else if (m_fKnown && m_pClass->HasDockingPorts())
		{
		COLORREF wColor;
		if (IsEnemy(GetUniverse()->GetPOV()))
			wColor = CG16bitImage::RGBValue(255, 0, 0);
		else
			wColor = CG16bitImage::RGBValue(0, 192, 0);

		Dest.DrawDot(x+1, y+1, 0, CG16bitImage::markerSmallSquare);
		Dest.DrawDot(x, y, wColor, CG16bitImage::markerSmallSquare);

		CString sName = GetName();
		g_pUniverse->GetMapLabelFont().DrawText(Dest, 
				x + MAP_LABEL_X + 1, 
				y + MAP_LABEL_Y + 1, 
				0,
				sName);
		g_pUniverse->GetMapLabelFont().DrawText(Dest, 
				x + MAP_LABEL_X, 
				y + MAP_LABEL_Y, 
				CG16bitImage::RGBValue(128, 128, 128),
				sName);
		}
	}

bool CShip::PointInObject (CVector vPos)

//	PointInObject
//
//	Returns TRUE if the given point is inside the object

	{
	//	Figure out the coordinates of vPos relative to the center of the
	//	ship, in pixels.

	CVector vOffset = vPos - GetPos();
	int x = (int)((vOffset.GetX() / g_KlicksPerPixel) + 0.5);
	int y = -(int)((vOffset.GetY() / g_KlicksPerPixel) + 0.5);

	//	Ask the image if the point is inside or not

	return m_pClass->GetImage().PointInImage(x, y, GetSystem()->GetTick(), Angle2Direction(GetRotation()));
	}

void CShip::ProgramDamage (CSpaceObject *pHacker, const ProgramDesc &Program)

//	ProgramDamage
//
//	Take damage from a program

	{
	switch (Program.iProgram)
		{
		case progShieldsDown:
			{
			CInstalledDevice *pShields = GetNamedDevice(devShields);
			if (pShields)
				{
				//	The chance of success is 50% plus 10% for every level
				//	that the program is greater than the shields

				int iSuccess = 50 + 10 * (Program.iAILevel - pShields->GetClass()->GetLevel());
				if (mathRandom(1, 100) <= iSuccess)
					pShields->Deplete(this);
				}

			break;
			}

		case progDisarm:
			{
			CInstalledDevice *pWeapon = GetNamedDevice(devPrimaryWeapon);
			if (pWeapon)
				{
				//	The chance of success is 50% plus 10% for every level
				//	that the program is greater than the primary weapon

				int iSuccess = 50 + 10 * (Program.iAILevel - pWeapon->GetClass()->GetLevel());
				if (mathRandom(1, 100) <= iSuccess)
					MakeDisarmed(Program.iAILevel * mathRandom(30, 60));
				}
			break;
			}

		case progReboot:
			break;

		case progCustom:
			{
			//	The chance of success is based on the AI level of the hacker
			//	and the cyber defense levels of the target (us)

			int iChance = 90 + 10 * (Program.iAILevel - GetCyberDefenseLevel());

			//	If the program succeeds, execute the program

			if (mathRandom(1, 100) <= iChance)
				Program.pCtx->pCC->Eval(Program.pCtx, Program.ProgramCode);

			break;
			}
		}

	//	Tell controller that we were attacked

	m_pController->OnProgramDamage(pHacker, Program);
	}

void CShip::ReactorOverload (void)

//	ReactorOverload
//
//	This is called every FUEL_CHECK_CYCLE when the reactor is overloading

	{
	int i;

	//	There is a 1 in 10 chance that something bad will happen
	//	(or, if the overload is severe, something bad always happens)

	if (mathRandom(1, 100) <= 10
			|| (m_iPowerDrain > 2 * m_iMaxPower))
		{
		CItemListManipulator ItemList(GetItemList());

		//	Choose the device that drains the most

		int iBestDev = -1;
		int iBestPower = 0;
		for (i = 0; i < GetDeviceCount(); i++)
			{
			if (!m_Devices[i].IsEmpty() 
					&& m_Devices[i].IsEnabled()
					&& m_Devices[i].CanBeDisabled())
				{
				SetCursorAtDevice(ItemList, i);

				CItem Item = ItemList.GetItemAtCursor();
				if (m_Devices[i].GetPowerRating(&Item) > iBestPower)
					{
					iBestDev = i;
					iBestPower = m_Devices[i].GetPowerRating(&Item);
					}
				}
			}

		//	Disable the device

		if (iBestDev != -1)
			EnableDevice(iBestDev, false);
		}
	}

void CShip::ReadyFirstMissile (void)

//	ReadyFirstMissile
//
//	Selects the first missile

	{
	CInstalledDevice *pDevice = GetNamedDevice(devMissileWeapon);
	if (pDevice)
		pDevice->SelectFirstVariant(this);
	}

void CShip::ReadyFirstWeapon (void)

//	ReadyFirstWeapon
//
//	Selects the first primary weapon

	{
	int iNextWeapon = FindNextDevice(-1, itemcatWeapon);
	if (iNextWeapon != -1)
		{
		m_NamedDevices[devPrimaryWeapon] = iNextWeapon;

		CInstalledDevice *pDevice = GetNamedDevice(devPrimaryWeapon);
		CDeviceClass *pClass = pDevice->GetClass();
		pClass->ValidateSelectedVariant(this, pDevice);
		}
	}

void CShip::ReadyNextMissile (void)

//	ReadyNextMissile
//
//	Selects the next missile

	{
	CInstalledDevice *pDevice = GetNamedDevice(devMissileWeapon);
	if (pDevice)
		pDevice->SelectNextVariant(this);
	}

void CShip::ReadyNextWeapon (void)

//	ReadyNextWeapon
//
//	Selects the next primary weapon

	{
	int iNextWeapon = FindNextDevice(m_NamedDevices[devPrimaryWeapon], itemcatWeapon);
	if (iNextWeapon != -1)
		{
		m_NamedDevices[devPrimaryWeapon] = iNextWeapon;

		CInstalledDevice *pDevice = GetNamedDevice(devPrimaryWeapon);
		CDeviceClass *pClass = pDevice->GetClass();
		pClass->ValidateSelectedVariant(this, pDevice);
		}
	}

void CShip::RechargeItem (CItemListManipulator &ItemList, int iCharges)

//	RechargeItem
//
//	Recharges the given item

	{
	const CItem &Item = ItemList.GetItemAtCursor();

	//	Set charges

	int iNewCharges = Max(0, Min(MAX_ITEM_CHARGES, ItemList.GetItemAtCursor().GetCharges() + iCharges));
	ItemList.SetChargesAtCursor(iNewCharges);
	InvalidateItemListState();

	//	If the item is installed, then we update bonuses

	if (Item.IsInstalled())
		{
		int iDevSlot = Item.GetInstalled();
		CInstalledDevice *pDevice = &m_Devices[iDevSlot];
		pDevice->SetChargesCache(iNewCharges);

		CalcDeviceBonus();
		}
	}

void CShip::Refuel (int iFuel)

//	Refuel
//
//	Refuels the ship

	{
	if (m_fOutOfFuel)
		{
		m_iFuelLeft = 0;
		m_fOutOfFuel = false;
		}

	m_iFuelLeft = std::min(GetMaxFuel(), m_iFuelLeft + iFuel);
	}

void CShip::Refuel (const CItem &Fuel)

//	Refuel
//
//	Refuels the ship

	{
	CItemType *pFuelType = Fuel.GetType();
	int iFuelPerItem = strToInt(pFuelType->GetData(), 0, NULL);
	int iFuel = iFuelPerItem * Fuel.GetCount();

	Refuel(iFuel);

	//	Invoke refueling code

	if (pFuelType->HasOnRefuelCode())
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		//	Define parameters

		ICCItem *pOldSource = CC.LookupGlobal(STR_G_SOURCE, &g_pUniverse);
		DefineGlobalSpaceObject(CC, CONSTLIT("gSource"), this);

		ICCItem *pOldItem = CC.LookupGlobal(STR_G_ITEM, &g_pUniverse);
		ICCItem *pItem = CreateListFromItem(CC, Fuel);
		CC.DefineGlobal(CONSTLIT("gItem"), pItem);
		pItem->Discard(&CC);

		//	Execute

		ICCItem *pCode = pFuelType->GetOnRefuelCode();
		if (pCode)
			{
			ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
			if (pResult->IsError())
				SendMessage(NULL, pResult->GetStringValue());

			pResult->Discard(&CC);
			}

		//	Restore

		CC.DefineGlobal(STR_G_SOURCE, pOldSource);
		pOldSource->Discard(&CC);
		CC.DefineGlobal(STR_G_ITEM, pOldItem);
		pOldItem->Discard(&CC);
		}
	}

void CShip::RemoveItemAsDevice (CItemListManipulator &ItemList)

//	RemoveItemAsDevice
//
//	Uninstalls the item

	{
	int i;

	//	Get the item at the cursor

	const CItem &Item = ItemList.GetItemAtCursor();
	ASSERT(Item.IsInstalled());

	//	Get the device slot that this item is at

	int iDevSlot = Item.GetInstalled();
	CInstalledDevice *pDevice = &m_Devices[iDevSlot];
	ItemCategories DevCat = pDevice->GetCategory();

	//	Clear the device

	pDevice->Uninstall(this, ItemList);

	//	Adjust named devices list

	if (pDevice->IsSecondaryWeapon())
		{
		for (i = devTurretWeapon1; i <= devTurretWeapon6; i++)
			if (m_NamedDevices[i] == iDevSlot)
				m_NamedDevices[i] = -1;

		m_pController->OnWeaponStatusChanged();
		}
	else
		{
		switch (DevCat)
			{
			case itemcatWeapon:
				if (m_NamedDevices[devPrimaryWeapon] == iDevSlot)
					m_NamedDevices[devPrimaryWeapon] = FindNextDevice(iDevSlot, itemcatWeapon);
				m_pController->OnWeaponStatusChanged();
				break;

			case itemcatLauncher:
				m_NamedDevices[devMissileWeapon] = -1;
				m_pController->OnWeaponStatusChanged();
				break;

			case itemcatShields:
				m_NamedDevices[devShields] = -1;
				m_pController->OnArmorRepaired(-1);
				break;

			case itemcatDrive:
				m_NamedDevices[devDrive] = -1;
				SetDriveDesc(NULL);
				break;

			case itemcatCargoHold:
				m_NamedDevices[devCargo] = -1;
				break;

			case itemcatReactor:
				m_NamedDevices[devReactor] = -1;
				m_pReactorDesc = m_pClass->GetReactorDesc();
				break;
			}
		}

	//	Recalc bonuses

	CalcDeviceBonus();
	InvalidateItemListState();
	}

void CShip::RepairAllArmor (void)

//	RepairAllArmor
//
//	Repair all the ship's armor

	{
	for (int i = 0; i < GetArmorSectionCount(); i++)
		{
		CInstalledArmor *pSection = GetArmorSection(i);
		pSection->iHitPoints = pSection->pArmorClass->GetHitPoints(pSection);
		m_pController->OnArmorRepaired(i);
		}
	}

void CShip::RepairArmor (int iSect, int iHitPoints)

//	RepairArmor
//
//	Repairs the armor

	{
	CInstalledArmor *pSection = GetArmorSection(iSect);
	int iDamage = pSection->pArmorClass->GetHitPoints(pSection) - pSection->iHitPoints;

	if (iHitPoints == -1)
		iHitPoints = iDamage;
	else
		iHitPoints = std::min(iDamage, iHitPoints);

	pSection->iHitPoints += iHitPoints;

	m_pController->OnArmorRepaired(iSect);
	}

void CShip::RepairDamage (int iHitPoints)

//	RepairDamage
//
//	Repairs the given number of hit points of damage

	{
	bool bRepaired = false;
	int iSect = 0;
	while (iHitPoints > 0 && iSect < GetArmorSectionCount())
		{
		CInstalledArmor *pSect = GetArmorSection(iSect);
		int iDamage = pSect->pArmorClass->GetHitPoints(pSect) - pSect->iHitPoints;
		if (iDamage > 0)
			{
			int iRepair = std::min(iDamage, iHitPoints);
			pSect->iHitPoints += iRepair;
			iHitPoints -= iRepair;
			bRepaired = true;
			}

		iSect++;
		}

	if (bRepaired)
		m_pController->OnArmorRepaired(-1);
	}

bool CShip::RequestDock (CSpaceObject *pObj)

//	RequestDock
//
//	pObj requests docking services with the station. Returns TRUE
//	if docking is engaged.

	{
	//	If time has stopped for this object, then we cannot allow docking

	if (IsTimeStopped())
		{
		pObj->SendMessage(this, CONSTLIT("Unable to dock"));
		return false;
		}

	//	If the object requesting docking services is an enemy,
	//	then deny docking services.

	if (IsEnemy(pObj))
		{
		pObj->SendMessage(this, CONSTLIT("Docking request denied"));
		return false;
		}

	//	Get the nearest free port

	return m_DockingPorts.RequestDock(this, pObj);
	}

void CShip::RevertOrientationChange (void)

//	RevertOrientationChange
//
//	Revert the ship's rotation this tick

	{
	m_iRotation = m_iPrevRotation;
	}

DeviceNames CShip::SelectWeapon (int iDev, int iVariant)

//	SelectWeapon
//
//	Selects the given weapon to fire. Returns whether the named
//	class for this weapon

	{
	CInstalledDevice *pWeapon = GetDevice(iDev);

	if (pWeapon == NULL)
		return devNone;
	else if (pWeapon->GetCategory() == itemcatWeapon)
		{
		m_NamedDevices[devPrimaryWeapon] = iDev;
		m_pController->OnWeaponStatusChanged();
		return devPrimaryWeapon;
		}
	else if (pWeapon->GetCategory() == itemcatLauncher)
		{
		m_NamedDevices[devMissileWeapon] = iDev;
		pWeapon->SelectFirstVariant(this);
		while (iVariant > 0)
			{
			pWeapon->SelectNextVariant(this);
			iVariant--;
			}

		m_pController->OnWeaponStatusChanged();
		return devMissileWeapon;
		}
	else
		return devNone;
	}

void CShip::SendMessage (CSpaceObject *pSender, const CString &sMsg)

//	SendMessage
//
//	Receives a message from some other object

	{
	m_pController->OnMessage(pSender, sMsg);
	}

void CShip::SetCommandCode (ICCItem *pCode)

//	SetCommandCode
//
//	Sets command code for the ship

	{
	m_pController->SetCommandCode(pCode);
	}

void CShip::SetController (IShipController *pController, bool bFreeOldController)

//	SetController

	{
	ASSERT(pController);

	if (bFreeOldController && m_pController)
		delete dynamic_cast<CObject *>(m_pController);

	m_pController = pController;
	m_pController->SetShipToControl(this);
	}

void CShip::SetCursorAtArmor (CItemListManipulator &ItemList, int iSect)

//	SetCursorAtArmor
//
//	Set the item list cursor to the item for the given armor segment

	{
	ItemList.ResetCursor();

	while (ItemList.MoveCursorForward())
		{
		const CItem &Item = ItemList.GetItemAtCursor();
		if (Item.GetType()->IsArmor()
				&& Item.GetInstalled() == iSect)
			{
			ASSERT(Item.GetCount() == 1);
			break;
			}
		}
	}

void CShip::SetCursorAtDevice (CItemListManipulator &ItemList, int iDev)

//	SetCursorAtDevice
//
//	Set the item list cursor to the item for the given device

	{
	ItemList.ResetCursor();

	CInstalledDevice *pDevice = GetDevice(iDev);
	if (pDevice->IsEmpty())
		return;

	while (ItemList.MoveCursorForward())
		{
		const CItem &Item = ItemList.GetItemAtCursor();
		if (Item.IsInstalled()
				&& Item.GetType() == pDevice->GetClass()->GetItemType()
				&& Item.GetInstalled() == iDev)
			{
			ASSERT(Item.GetCount() == 1);
			break;
			}
		}
	}

void CShip::SetCursorAtNamedDevice (CItemListManipulator &ItemList, DeviceNames iDev)

//	SetCursorAtNamedDevice
//
//	Set the item list cursor to the given named device

	{
	if (m_NamedDevices[iDev] != -1)
		SetCursorAtDevice(ItemList, m_NamedDevices[iDev]);
	}

void CShip::SetDriveDesc (const DriveDesc *pDesc)

//	SetDriveDesc
//
//	Sets the drive descriptor (or NULL to set back to class)

	{
	if (pDesc)
		{
		m_pDriveDesc = pDesc;
		m_iThrust = m_pClass->GetHullDriveDesc()->iThrust + m_pDriveDesc->iThrust;
		m_rMaxSpeed = Max(m_pClass->GetHullDriveDesc()->rMaxSpeed, m_pDriveDesc->rMaxSpeed);
		}
	else
		{
		m_pDriveDesc = m_pClass->GetHullDriveDesc();
		m_iThrust = m_pDriveDesc->iThrust;
		m_rMaxSpeed = m_pDriveDesc->rMaxSpeed;
		}
	}

void CShip::SetFireDelay (CInstalledDevice *pWeapon)

//	SetFireDelay
//
//	Sets the fire delay for the weapon based on the intrinsic
//	fire rate of the weapon and the AISettings

	{
	pWeapon->SetActivationDelay(m_pController->GetFireRateAdj() * pWeapon->GetActivateDelay(this) / 10);
	}

void CShip::SetInGate (CSpaceObject *pGate, int iTickCount)

//	SetInGate
//
//	While timer is on, ship is inside of stargate. When timer expires
//	the ship comes out (with gate flash)

	{
	//	Set in gate context (Note: It is OK if we get called here while
	//	IsInGate is true--this can happen if the player passes back and forth
	//	through the same gate).

	m_pExitGate = pGate;
	m_iExitGateTimer = GATE_ANIMATION_LENGTH + iTickCount;
	SetCannotBeHit();

	//	Clear this flag--we might not have gotten a chance to clear it
	//	before we switched systems.

	m_fFollowPlayerThroughGate = false;
	}

bool CShip::ShieldsAbsorbFire (CInstalledDevice *pWeapon)

//	ShieldsAbsorbFire
//
//	Returns TRUE if the shields absorb the shot from the given weapon

	{
	//	Check to see if shields prevent firing

	CInstalledDevice *pShields = GetNamedDevice(devShields);
	if (pShields && pShields->GetClass()->AbsorbsWeaponFire(pShields, this, pWeapon))
		return true;

	//	Now check to see if energy fields prevent firing

	if (m_EnergyFields.AbsorbsWeaponFire(pWeapon))
		return true;

	//	Done

	return false;
	}

void CShip::Undock (void)

//	Undock
//
//	Undock from station

	{
	if (m_pDocked)
		{
		m_pDocked->Undock(this);
		m_pDocked = NULL;

		//	Update our cargo mass (in case it has changed during docking)

		OnComponentChanged(comCargo);
		}
	}

void CShip::Undock (CSpaceObject *pObj)

//	Undock
//
//	Undocks from the station

	{
	m_DockingPorts.Undock(this, pObj);
	}

void CShip::UninstallArmor (CItemListManipulator &ItemList)

//	UninstallArmor
//
//	Uninstalls the armor at the cursor

	{
	}

void CShip::UpdateArmorItems (void)

//	UpdateArmorItems
//
//	Make sure that all the armor items have the correct damage flag

	{
	CItemListManipulator ItemList(GetItemList());
	while (ItemList.MoveCursorForward())
		{
		const CItem &Item = ItemList.GetItemAtCursor();
		if (Item.IsInstalled()
				&& Item.GetType()->GetCategory() == itemcatArmor)
			{
			CInstalledArmor *pSect = GetArmorSection(Item.GetInstalled());
			bool bDamaged = (pSect->iHitPoints < pSect->pArmorClass->GetHitPoints(pSect));
			if (Item.IsDamaged() != bDamaged)
				{
				ItemList.SetDamagedAtCursor(bDamaged);
				InvalidateItemListState();
				}
			}
		}
	}

void CShip::UpdateDockingManeuver(const CVector &vDest, const CVector &vDestVel, int iDestFacing)

//	UpdateDockingManeuver
//
//	Maneuvers the ship to a docking position

	{
	//	Figure out how far we are from where we want to be

	CVector vDelta = vDest - GetPos();
	CVector vDeltaVel = vDestVel - GetVel();
	Metric rMaxSpeed = GetMaxSpeed();

	//	If our position and velocity are pretty close, then stay
	//	where we are (though we cheat a little by adjusting our velocity
	//	manually)

	Metric rDelta2 = vDelta.Length2();
	Metric rDeltaVel2 = vDeltaVel.Length2();
	bool bCloseEnough = (rDelta2 < MAX_DELTA2);

	//	Decompose our position delta along the axis of final direction
	//	I.e., we figure out how far we are ahead or behind the destination
	//	and how far we are to the left or right.

	CVector vAxisY = PolarToVector(iDestFacing, 1.0);
	CVector vAxisX = vAxisY.Perpendicular();
	Metric rDeltaX = vDelta.Dot(vAxisX);
	Metric rDeltaY = vDelta.Dot(vAxisY);

	//	Our velocity towards the axis should be proportional to our
	//	distance from it.

	Metric rDesiredVelX;
	if (bCloseEnough)
		rDesiredVelX = 0.0;
	else if (rDeltaX > 0.0)
		rDesiredVelX = (Min(MAX_DISTANCE, rDeltaX) / MAX_DISTANCE) * rMaxSpeed;
	else
		rDesiredVelX = (Max(-MAX_DISTANCE, rDeltaX) / MAX_DISTANCE) * rMaxSpeed;

	//	Same with our velocity along the axis

	Metric rDesiredVelY;
	if (bCloseEnough)
		rDesiredVelY = 0.0;
	else if (rDeltaY > 0.0)
		rDesiredVelY = (Min(MAX_DISTANCE, rDeltaY) / MAX_DISTANCE) * rMaxSpeed;
	else
		rDesiredVelY = (Max(-MAX_DISTANCE, rDeltaY) / MAX_DISTANCE) * rMaxSpeed;

	//	Recompose to our desired velocity

	CVector vDesiredVel = (rDesiredVelX * vAxisX) + (rDesiredVelY * vAxisY);
	vDesiredVel = vDesiredVel + vDestVel;

	//	Figure out the delta v that we need to achieve our desired velocity

	CVector vDiff = vDesiredVel - GetVel();
	Metric rDiff2 = vDiff.Length2();

	//	If we're close enough to the velocity, cheat a little by
	//	accelerating without using the main engine

	if (rDiff2 < MAX_DELTA_VEL2)
		{
		Accelerate(vDiff * GetMass() / 2000.0, g_SecondsPerUpdate);

		m_pController->SetManeuver(CalcTurnManeuver(iDestFacing, GetRotation(), GetRotationAngle()));
		m_pController->SetThrust(false);
		}

	//	Otherwise, thrust with the main engines

	else
		{
		//	Figure out which direction we need to thrust in to achieve the delta v

		int iAngle = VectorToPolar(vDiff);

		//	Turn towards the angle

		m_pController->SetManeuver(CalcTurnManeuver(iAngle, GetRotation(), GetRotationAngle()));

		//	If we don't need to turn, engage thrust

		m_pController->SetThrust(m_pController->GetManeuver() == IShipController::NoRotation);
		}
	}
