//	CShipClass.cpp
//
//	CShipClass class

#include "PreComp.h"

#define WRECK_IMAGE_TAG							CONSTLIT("WreckImage")
#define ITEMS_TAG								CONSTLIT("Items")
#define DRIVE_IMAGES_TAG						CONSTLIT("DriveImages")
#define NOZZLE_IMAGE_TAG						CONSTLIT("NozzleImage")
#define NOZZLE_POS_TAG							CONSTLIT("NozzlePos")
#define DOCK_SCREENS_TAG						CONSTLIT("DockScreens")
#define NAMES_TAG								CONSTLIT("Names")
#define EVENTS_TAG								CONSTLIT("Events")
#define COMMUNICATIONS_TAG						CONSTLIT("Communications")
#define PLAYER_SETTINGS_TAG						CONSTLIT("PlayerSettings")
#define SHIELD_DISPLAY_TAG						CONSTLIT("ShieldDisplay")
#define IMAGE_TAG								CONSTLIT("Image")
#define ARMOR_DISPLAY_TAG						CONSTLIT("ArmorDisplay")
#define REACTOR_DISPLAY_TAG						CONSTLIT("ReactorDisplay")
#define POWER_LEVEL_IMAGE_TAG					CONSTLIT("PowerLevelImage")
#define FUEL_LEVEL_IMAGE_TAG					CONSTLIT("FuelLevelImage")
#define FUEL_LOW_LEVEL_IMAGE_TAG				CONSTLIT("FuelLowLevelImage")
#define REACTOR_TEXT_TAG						CONSTLIT("ReactorText")
#define POWER_LEVEL_TEXT_TAG					CONSTLIT("PowerLevelText")
#define FUEL_LEVEL_TEXT_TAG						CONSTLIT("FuelLevelText")
#define DEVICES_TAG								CONSTLIT("Devices")

#define UNID_ATTRIB								CONSTLIT("UNID")
#define ARMOR_ID_ATTRIB							CONSTLIT("armorID")
#define FIRE_ACCURACY_ATTRIB					CONSTLIT("fireAccuracy")
#define FIRE_RATE_ADJ_ATTRIB					CONSTLIT("fireRateAdj")
#define NO_SHIELD_RETREAT_ATTRIB				CONSTLIT("ignoreShieldsDown")
#define SCORE_ATTRIB							CONSTLIT("score")
#define X_ATTRIB								CONSTLIT("x")
#define Y_ATTRIB								CONSTLIT("y")
#define NON_CRITICAL_ATTRIB						CONSTLIT("nonCritical")
#define EXCLUSION_ATTRIB						CONSTLIT("exclusion")
#define RADIOACTIVE_WRECK_ATTRIB				CONSTLIT("radioactiveWreck")
#define NO_DOGFIGHTS_ATTRIB						CONSTLIT("noDogfights")
#define DOCK_SCREEN_ATTRIB						CONSTLIT("dockScreen")
#define CYBER_DEFENSE_LEVEL_ATTRIB				CONSTLIT("cyberDefenseLevel")
#define ATTRIBUTES_ATTRIB						CONSTLIT("attributes")
#define STAND_OFF_COMBAT_ATTRIB					CONSTLIT("standOffCombat")
#define NON_COMBATANT_ATTRIB					CONSTLIT("nonCombatant")
#define PLAYER_SHIP_ATTRIB						CONSTLIT("playerShip")
#define DESC_ATTRIB								CONSTLIT("desc")
#define INITIAL_CLASS_ATTRIB					CONSTLIT("initialClass")
#define DEBUG_ONLY_ATTRIB						CONSTLIT("debugOnly")
#define NAME_ATTRIB								CONSTLIT("name")
#define DEST_X_ATTRIB							CONSTLIT("destX")
#define DEST_Y_ATTRIB							CONSTLIT("destY")
#define HP_X_ATTRIB								CONSTLIT("hpX")
#define HP_Y_ATTRIB								CONSTLIT("hpY")
#define NAME_Y_ATTRIB							CONSTLIT("nameY")
#define NAME_BREAK_WIDTH						CONSTLIT("nameBreakWidth")
#define NAME_DEST_X_ATTRIB						CONSTLIT("nameDestX")
#define NAME_DEST_Y_ATTRIB						CONSTLIT("nameDestY")
#define CARGO_SPACE_ATTRIB						CONSTLIT("cargoSpace")
#define MAX_CARGO_SPACE_ATTRIB					CONSTLIT("maxCargoSpace")
#define PERCEPTION_ATTRIB						CONSTLIT("perception")
#define WIDTH_ATTRIB							CONSTLIT("width")
#define HEIGHT_ATTRIB							CONSTLIT("height")
#define REACTOR_POWER_ATTRIB					CONSTLIT("reactorPower")
#define MAX_REACTOR_FUEL_ATTRIB					CONSTLIT("fuelCapacity")
#define INERTIALESS_DRIVE_ATTRIB				CONSTLIT("inertialessDrive")
#define TIME_STOP_IMMUNE_ATTRIB					CONSTLIT("timeStopImmune")
#define MAX_ARMOR_ATTRIB						CONSTLIT("maxArmor")
#define MAX_REACTOR_POWER_ATTRIB				CONSTLIT("maxReactorPower")
#define MAX_DEVICES_ATTRIB						CONSTLIT("maxDevices")
#define FLYBY_COMBAT_ATTRIB						CONSTLIT("flybyCombat")
#define STARTING_CREDITS_ATTRIB					CONSTLIT("startingCredits")
#define MAX_WEAPONS_ATTRIB						CONSTLIT("maxWeapons")
#define MAX_NON_WEAPONS_ATTRIB					CONSTLIT("maxNonWeapons")
#define REVERSE_ARTICLE_ATTRIB					CONSTLIT("reverseArticle")
#define COMBAT_STYLE_ATTRIB						CONSTLIT("combatStyle")
#define STARTING_SYSTEM_ATTRIB					CONSTLIT("startingSystem")
#define STARTING_POS_ATTRIB						CONSTLIT("startingPos")
#define DEFAULT_BACKGROUND_ID_ATTRIB			CONSTLIT("defaultBackgroundID")
#define LARGE_IMAGE_ATTRIB						CONSTLIT("largeImage")
#define NO_FRIENDLY_FIRE_ATTRIB					CONSTLIT("noFriendlyFire")
#define ROTATION_COUNT_ATTRIB					CONSTLIT("rotationCount")
#define DEFINITE_ARTICLE_ATTRIB					CONSTLIT("definiteArticle")
#define NO_ARTICLE_ATTRIB						CONSTLIT("noArticle")
#define SHIP_SCREEN_ATTRIB						CONSTLIT("shipScreen")
#define EXPLOSION_TYPE_ATTRIB					CONSTLIT("explosionType")
#define MAX_STRUCTURAL_HIT_POINTS_ATTRIB		CONSTLIT("maxStructuralHitPoints")
#define LEAVES_WRECK_ATTRIB						CONSTLIT("leavesWreck")
#define FIRE_RANGE_ADJ_ATTRIB					CONSTLIT("fireRangeAdj")

#define COMBAT_STYLE_STAND_OFF					CONSTLIT("standOff")
#define COMBAT_STYLE_FLYBY						CONSTLIT("flyby")
#define COMBAT_STYLE_NO_RETREAT					CONSTLIT("noRetreat")

#define FIELD_CARGO_SPACE						CONSTLIT("cargoSpace")
#define FIELD_EXPLOSION_TYPE					CONSTLIT("explosionType")
#define FIELD_FIRE_ACCURACY						CONSTLIT("fireAccuracy")
#define FIELD_FIRE_RANGE_ADJ					CONSTLIT("fireRangeAdj")
#define FIELD_FIRE_RATE_ADJ						CONSTLIT("fireRateAdj")
#define FIELD_LAUNCHER							CONSTLIT("launcher")
#define FIELD_LAUNCHER_UNID						CONSTLIT("launcherUNID")
#define FIELD_LEVEL								CONSTLIT("level")
#define FIELD_MAX_SPEED							CONSTLIT("maxSpeed")
#define FIELD_MANEUVER							CONSTLIT("maneuver")
#define FIELD_MANUFACTURER						CONSTLIT("manufacturer")
#define FIELD_NAME								CONSTLIT("name")
#define FIELD_PRIMARY_ARMOR						CONSTLIT("primaryArmor")
#define FIELD_PRIMARY_ARMOR_UNID				CONSTLIT("primaryArmorUNID")
#define FIELD_PRIMARY_WEAPON					CONSTLIT("primaryWeapon")
#define FIELD_PRIMARY_WEAPON_RANGE				CONSTLIT("primaryWeaponRange")
#define FIELD_PRIMARY_WEAPON_RANGE_ADJ			CONSTLIT("primaryWeaponRangeAdj")
#define FIELD_PRIMARY_WEAPON_UNID				CONSTLIT("primaryWeaponUNID")
#define FIELD_SCORE								CONSTLIT("score")
#define FIELD_SHIELD							CONSTLIT("shield")
#define FIELD_SHIELD_UNID						CONSTLIT("shieldsUNID")
#define FIELD_THRUST_TO_WEIGHT					CONSTLIT("thrustToWeight")

#define ERR_OUT_OF_MEMORY						CONSTLIT("out of memory")
#define ERR_BAD_IMAGE							CONSTLIT("invalid ship image")
#define ERR_MISSING_ARMOR_TAG					CONSTLIT("missing <Armor> element")
#define ERR_MISSING_DEVICES_TAG					CONSTLIT("missing <Devices> element")
#define ERR_TOO_MANY_DEVICES					CONSTLIT("too many devices")
#define ERR_BAD_EXHAUST_IMAGE					CONSTLIT("invalid drive image")
#define ERR_DRIVE_IMAGE_FORMAT					CONSTLIT("invalid element in <DriveImages>")
#define ERR_DOCK_SCREEN_NEEDED					CONSTLIT("docking ports specified but no docking screen defined")
#define ERR_SHIELD_DISPLAY_NEEDED				CONSTLIT("missing or invalid <ShieldDisplay> element")
#define ERR_ARMOR_DISPLAY_NEEDED				CONSTLIT("missing or invalid <ArmorDisplay> element")
#define ERR_REACTOR_DISPLAY_NEEDED				CONSTLIT("missing or invalid <ReactorDisplay> element")
#define ERR_INVALID_STARTING_CREDITS			CONSTLIT("invalid starting credits")

#define WRECK_IMAGE_VARIANTS					3
#define DAMAGE_IMAGE_COUNT						10
#define DAMAGE_IMAGE_WIDTH						24
#define DAMAGE_IMAGE_HEIGHT						24

#define DEFAULT_POWER_USE						20

static char g_ArmorTag[] = "Armor";
static char g_ArmorSectionTag[] = "ArmorSection";
static char g_AISettingsTag[] = "AISettings";

static char g_ManufacturerAttrib[] = "manufacturer";
static char g_ClassAttrib[] = "class";
static char g_TypeAttrib[] = "type";
static char g_MassAttrib[] = "mass";
static char g_ThrustAttrib[] = "thrust";
static char g_ManeuverAttrib[] = "maneuver";
static char g_MaxSpeedAttrib[] = "maxSpeed";
static char g_StartAttrib[] = "start";
static char g_SpanAttrib[] = "span";
static char g_DeviceIDAttrib[] = "deviceID";

static char g_FireRateAttrib[] = "fireRate";

static CG16bitImage *g_pDamageBitmap = NULL;
static CStationType *g_pWreckDesc = NULL;

ALERROR InitRectFromElement (CXMLElement *pItem, RECT *retRect);
DWORD ParseNonCritical (const CString &sList);

SAISettings g_DefaultAISettings =
	{
	10,				//	Normal fire rate
	100,			//	Normal fire range
	100,			//	100% accuracy
	CSpaceObject::perceptNormal,	//	Normal perception
	aicombatStandard,	//	Combat style
	0,				//	Flags
	};

struct ScoreDesc
	{
	int iBaseXP;
	int iSpecialXP;
	int iExceptionalXP;
	int iDrawbackXP;
	int iScoreLevel;
	};

static ScoreDesc g_XP[] =
	{
		//	Base Score
		//			Special Ability
		//					Exceptional Ability
		//							Drawback
		//									Level Score

		{	20,		5,		20,		0,		50 },			//	I
		{	50,		10,		50,		-5,		100 },			//	II
		{	115,	15,		100,	-10,	200 },			//	III
		{	200,	20,		170,	-20,	350 },			//	IV
		{	340,	30,		260,	-35,	600 },			//	V
		{	500,	45,		370,	-50,	900 },			//	VI
		{	750,	60,		500,	-65,	1400 },			//	VII
		{	1050,	80,		650,	-85,	1900 },			//	VIII
		{	1450,	100,	820,	-105,	2600 },			//	IX
		{	1900,	125,	1010,	-130,	3250 },			//	X
		{	2400,	150,	1220,	-155,	4200 },			//	XI
		{	3000,	180,	1450,	-185,	5500 },			//	XII
		{	3600,	210,	1700,	-215,	6750 },			//	XIII
		{	4250,	245,	1970,	-250,	8250 },			//	XIV
		{	5000,	280,	2260,	-285,	10000 },		//	XV
		{	6000,	320,	2570,	-325,	11500 },		//	XVI
		{	7000,	360,	2900,	-365,	13250 },		//	XVII
		{	8000,	405,	3250,	-410,	15000 },		//	XVIII
		{	9000,	450,	3620,	-455,	16750 },		//	XIX
		{	10000,	500,	4010,	-505,	18500 },		//	XX
		{	11000,	550,	4420,	-555,	20500 },		//	XXI
		{	12000,	605,	4850,	-610,	22500 },		//	XXII
		{	13000,	660,	5300,	-665,	25000 },		//	XXIII
		{	14000,	720,	5770,	-725,	26500 },		//	XXIV
		{	15000,	780,	6260,	-785,	30000 },		//	XXV
	};

#define SCORE_DESC_COUNT							(sizeof(g_XP) / sizeof(g_XP[0]))

CShipClass::CShipClass (void) : 
		m_Hull(sizeof(HullSection), 2),
		m_DockingPorts(NULL),
		m_pDockScreens(NULL),
		m_pPlayerSettings(NULL),
		m_pItems(NULL),
		m_pDevices(NULL)

//	CShipClass constructor

	{
	}

CShipClass::~CShipClass (void)

//	CShip destructor

	{
	if (m_pDevices)
		delete m_pDevices;

	if (m_pItems)
		delete m_pItems;

	if (m_DockingPorts)
		delete [] m_DockingPorts;

	if (m_pDockScreens)
		delete m_pDockScreens;

	if (m_pPlayerSettings)
		delete m_pPlayerSettings;
	}

Metric CShipClass::CalcMass (const CDeviceDescList &Devices)

//	CalcMass
//
//	Returns the total mass of the ship class, including devices and armor

	{
	int i;
	Metric rMass = GetHullMass();

	for (i = 0; i < GetHullSectionCount(); i++)
		rMass += GetHullSection(i)->pArmor->GetMass();

	for (i = 0; i < Devices.GetCount(); i++)
		rMass += Devices.GetDeviceClass(i)->GetMass();

	return rMass;
	}

int CShipClass::CalcScore (void)

//	CalcScore
//
//	Computes the score of the ships class

	{
	//	Compute the movement stats

	int iSpeed, iThrust, iManeuver;
	ComputeMovementStats(m_AverageDevices, &iSpeed, &iThrust, &iManeuver);

	//	Figure out what armor we've got

	int iArmorSections = GetHullSectionCount();
	int iArmorLevel = GetHullSection(0)->pArmor->GetItemType()->GetLevel();

	//	Figure out what devices we've got

	int iWeaponLevel = 0;
	int iWeapon = -1;
	int iLauncherLevel = 0;
	int iLauncher = -1;
	int iShieldLevel = 0;

	for (int j = 0; j < m_AverageDevices.GetCount(); j++)
		{
		CDeviceClass *pDevice = m_AverageDevices.GetDeviceDesc(j).Item.GetType()->GetDeviceClass();
		switch (pDevice->GetCategory())
			{
			case itemcatWeapon:
				{
				if (pDevice->GetLevel() > iWeaponLevel)
					{
					iWeaponLevel = pDevice->GetLevel();
					iWeapon = j;
					}
				break;
				}

			case itemcatLauncher:
				{
				iLauncherLevel = ComputeDeviceLevel(pDevice);
				iLauncher = j;
				break;
				}

			case itemcatShields:
				{
				iShieldLevel = pDevice->GetLevel();
				break;
				}
			default:
				break;
			}
		}

	int iPrimaryWeapon;
	if (iLauncherLevel > iWeaponLevel)
		iPrimaryWeapon = iLauncher;
	else
		iPrimaryWeapon = iWeapon;

	//	Compute score and level

	return ComputeScore(m_AverageDevices,
			iArmorLevel, 
			iPrimaryWeapon, 
			iSpeed, 
			iThrust, 
			iManeuver,
			iLauncherLevel > iWeaponLevel);
	}

CString CShipClass::ComposeLoadError (const CString &sError)

//	ComposeLoadError
//
//	Compose an error loading XML

	{
	return strPatternSubst(CONSTLIT("%s ship class: %s"),
			GetName().GetASCIIZPointer(),
			sError.GetASCIIZPointer());
	}

int CShipClass::ComputeDeviceLevel (CDeviceClass *pDevice)

//	ComputeDeviceLevel
//
//	Computes the level of the given device

	{
	if (pDevice->GetCategory() == itemcatLauncher)
		{
		//int iLevel = pDevice->GetItemType()->GetLevel();
		int iLevel = 0;

		//	Look for the highest missile level and use that as the
		//	weapon level.

		IItemGenerator *pItems = GetRandomItemTable();
		FindBestMissile(pItems, &iLevel);

		return iLevel;
		}
	else
		return pDevice->GetLevel();
	}


int CShipClass::ComputeLevel (int iScore)

//	ComputeLevel
//
//	Returns the level of the ship class

	{
	int i = 0;
	while (i < SCORE_DESC_COUNT-1 && g_XP[i].iScoreLevel < iScore)
		i++;

	return i+1;
	}

enum LowMediumHigh
	{
	enumLow = 0,
	enumMedium = 1,
	enumHigh = 2,
	};

void CShipClass::ComputeMovementStats (CDeviceDescList &Devices, int *retiSpeed, int *retiThrust, int *retiManeuver)
	{
	const DriveDesc *pDrive = GetHullDriveDesc();

	//	Figure out the speed of the ship

	if (pDrive->rMaxSpeed > 0.20 * LIGHT_SPEED)
		*retiSpeed = enumHigh;
	else if (pDrive->rMaxSpeed > 0.15 * LIGHT_SPEED)
		*retiSpeed = enumMedium;
	else
		*retiSpeed = enumLow;

	//	Figure out the mass of the ship (including all installed
	//	weapons and armor)

	Metric rFullMass = CalcMass(Devices);

	//	Figure out the thrust of the ship

	Metric rRatio = (Metric)pDrive->iThrust / rFullMass;
	if (rRatio >= 7.0)
		*retiThrust = enumHigh;
	else if (rRatio >= 3.0)
		*retiThrust = enumMedium;
	else
		*retiThrust = enumLow;

	//	Figure out the maneuverability of the ship

	if (GetManeuverability() >= 7)
		*retiManeuver = enumLow;
	else if (GetManeuverability() > 2)
		*retiManeuver = enumMedium;
	else
		*retiManeuver = enumHigh;
	}

int CShipClass::ComputeScore (const CDeviceDescList &Devices,
							  int iArmorLevel,
							  int iPrimaryWeapon,
							  int iSpeed,
							  int iThrust,
							  int iManeuver,
							  bool bPrimaryIsLauncher)

//	ComputeScore
//
//	Compute the score of the class based on equipment

	{
	int i;
	int iSpecial = 0;
	int iExceptional = 0;
	int iDrawback = 0;
	int iStdLevel = iArmorLevel;
	int iWeaponLevel = (iPrimaryWeapon == -1 ? 0 : ComputeDeviceLevel(Devices.GetDeviceDesc(iPrimaryWeapon).Item.GetType()->GetDeviceClass()));

	//	If our weapon is better than our armor then adjust the level
	//	depending on the difference.

	if (iWeaponLevel > iArmorLevel)
		{
		switch (iWeaponLevel - iArmorLevel)
			{
			case 1:
				iStdLevel = iWeaponLevel;
				iDrawback++;
				break;

			case 3:
				iStdLevel = iWeaponLevel - 2;
				iSpecial += 2;
				break;

			default:
				iStdLevel = (iWeaponLevel + iArmorLevel) / 2;
			}
		}

	//	If our best weapon is 2 or more levels below our standard
	//	level then take drawbacks exponentially.

	if (iStdLevel > iWeaponLevel + 1)
		iDrawback += std::min(16, (1 << (iStdLevel - (iWeaponLevel + 2))));
	else if (iStdLevel > iWeaponLevel)
		iDrawback++;

	//	If all movement stats are high then this counts as an
	//	exceptional ability

	if (iSpeed == enumHigh && iThrust == enumHigh && iManeuver == enumHigh)
		iExceptional++;

	//	Otherwise, treat them as special abilities or drawbacks

	else
		{
		if (iSpeed == enumLow)
			iDrawback++;
		else if (iSpeed == enumHigh)
			iSpecial++;

		if (iThrust == enumLow)
			iDrawback++;
		else if (iThrust == enumHigh)
			iSpecial++;

		if (iManeuver == enumLow)
			iDrawback++;
		else if (iManeuver == enumHigh)
			iSpecial++;
		}

	//	1 armor segment is a drawback

	int iArmorSections = GetHullSectionCount();
	if (iArmorSections <= 1)
		iDrawback++;

	//	2-3 armor segments is normal

	else if (iArmorSections < 4)
		;

	//	4 or more armor segments is special

	else if (iArmorSections < 8 )
		iSpecial++;
	else if (iArmorSections < 16)
		iSpecial += 2;
	else if (iArmorSections < 32)
		iSpecial += 3;
	else if (iArmorSections < 64)
		iSpecial += 4;
	else
		iSpecial += 5;

	//	Checkout all the devices

	bool bDirectionalBonus = false;
	bool bGoodSecondary = false;
	int iDirectionalBonus = 0;
	for (i = 0; i < Devices.GetCount(); i++)
		{
		const SDeviceDesc &Dev = Devices.GetDeviceDesc(i);
		CDeviceClass *pDevice = Dev.Item.GetType()->GetDeviceClass();
		int iDeviceLevel = ComputeDeviceLevel(pDevice);

		//	Specific devices

		switch (pDevice->GetCategory())
			{
			case itemcatWeapon:
			case itemcatLauncher:
				{
				int iWeaponAdj = (iDeviceLevel - iStdLevel);

				//	If this is a secondary weapon, then add it to the score

				if (i != iPrimaryWeapon)
					{
					//	Calculate any potential bonus based on the weapon level
					//	compared to the base level

					iSpecial += std::max(iWeaponAdj + 3, 0);
					}

				//	Compute fire arc

				int iFireArc = (Dev.bOmnidirectional ? 360 : AngleRange(Dev.iMinFireArc, Dev.iMaxFireArc));

				//	Adjust for turret-mount

				iDirectionalBonus += (std::max(iWeaponAdj + 3, 0) * iFireArc);
				break;
				}

			case itemcatReactor:
				//	Reactors don't count as improvements
				break;

			default:
				{
				//	Other devices are special abilities depending on level

				if (iDeviceLevel > iStdLevel+1)
					iExceptional++;
				else if (iDeviceLevel > iStdLevel)
					iSpecial += 4;
				else if (iDeviceLevel >= iStdLevel-1)
					iSpecial += 2;
				else
					iSpecial++;
				}
			}
		}

	//	If we have no weapons then we have some drawbacks

	if (iPrimaryWeapon == -1)
		iDrawback += 3;

	//	Add bonus if weapon is omnidirectional

	iSpecial += (int)((iDirectionalBonus / 270.0) + 0.5);

	//	Checkout AI settings

	const SAISettings &AI = GetAISettings();
	int iFireAccuracyScore, iFireRateScore;

	if (AI.iFireAccuracy > 97)
		iFireAccuracyScore = 5;
	else if (AI.iFireAccuracy >= 93)
		iFireAccuracyScore = 4;
	else if (AI.iFireAccuracy >= 90)
		iFireAccuracyScore = 3;
	else if (AI.iFireAccuracy < 75)
		iFireAccuracyScore = 1;
	else
		iFireAccuracyScore = 2;

	if (AI.iFireRateAdj <= 10)
		iFireRateScore = 5;
	else if (AI.iFireRateAdj <= 20)
		iFireRateScore = 4;
	else if (AI.iFireRateAdj <= 30)
		iFireRateScore = 3;
	else if (AI.iFireRateAdj >= 60)
		iFireRateScore = 1;
	else
		iFireRateScore = 2;

	int iFireControlScore = iFireRateScore * iFireAccuracyScore;
	if (iFireControlScore >= 20)
		iExceptional++;
	else if (iFireControlScore > 6)
		iSpecial += ((iFireControlScore - 5) / 2);
	else if (iFireControlScore < 2)
		iDrawback += 4;
	else if (iFireControlScore < 4)
		iDrawback += 2;

	//	Compute final score

	ScoreDesc *pBase = &g_XP[iStdLevel-1];
	int iScore = pBase->iBaseXP
			+ iSpecial * pBase->iSpecialXP
			+ iExceptional * pBase->iExceptionalXP
			+ iDrawback * pBase->iDrawbackXP;

	return iScore;
	}

void CShipClass::CreateEmptyWreck (CSystem *pSystem, 
								   CShip *pShip,
								   const CVector &vPos, 
								   const CVector &vVel,
								   CSovereign *pSovereign, 
								   CStation **retpWreck)

//	CreateEmptyWreck
//
//	Create an empty wreck of the given ship class

	{
	//	Create the wreck

	CStation *pWreck;
	CStation::CreateFromType(pSystem,
			GetWreckDesc(),
			vPos,
			vVel,
			NULL,
			&pWreck);

	//	Set properties of the wreck

	pWreck->SetSovereign(pSovereign);
	SetShipwreckImage(pWreck);
	SetShipwreckParams(pWreck, pShip);

	//	Done

	if (retpWreck)
		*retpWreck = pWreck;
	}

void CShipClass::CreateExplosion (CShip *pShip, CSpaceObject *pWreck)

//	CreateExplosion
//
//	Creates an explosion for the given ship

	{
	//	Explosion effect and damage

	SExplosionType Explosion;
	pShip->FireGetExplosionType(&Explosion);
	if (Explosion.pDesc == NULL)
		Explosion.pDesc = GetExplosionType();

	if (Explosion.pDesc)
		{
		pShip->GetSystem()->CreateWeaponFire(Explosion.pDesc,
				Explosion.iBonus,
				Explosion.iCause,
				CDamageSource(pShip, pWreck),
				pShip->GetPos(),
				pShip->GetVel(),
				0,
				NULL,
				NULL);
		}

	//	Otherwise, if no defined explosion, we create a default one

	else
		{
		DWORD dwEffectID;

		//	If this is a large ship, use a large explosion

		if (RectWidth(m_Image.GetImageRect()) > 64)
			dwEffectID = g_LargeExplosionUNID;
		else
			dwEffectID = g_ExplosionUNID;

		CEffectCreator *pEffect = g_pUniverse->FindEffectType(dwEffectID);
		if (pEffect)
			pEffect->CreateEffect(pShip->GetSystem(),
					pWreck,
					pShip->GetPos(),
					pShip->GetVel());

		//	Particles

		CObjectImageArray Image;
		RECT rcRect;
		rcRect.left = 0;
		rcRect.top = 0;
		rcRect.right = 4;
		rcRect.bottom = 4;
		Image.Init(g_ShipExplosionParticlesUNID,
				rcRect,
				8,
				3);

		CParticleEffect::CreateExplosion(pShip->GetSystem(),
				//pWreck,
				NULL,
				pShip->GetPos(),
				pShip->GetVel(),
				mathRandom(1, 50),
				LIGHT_SPEED * 0.25,
				0,
				300,
				Image,
				NULL);

		//	HACK: No image means paint smoke particles

		CObjectImageArray Dummy;
		CParticleEffect::CreateExplosion(pShip->GetSystem(),
				//pWreck,
				NULL,
				pShip->GetPos(),
				pShip->GetVel(),
				mathRandom(25, 150),
				LIGHT_SPEED * 0.1,
				20 + mathRandom(10, 30),
				45,
				Dummy,
				NULL);
		}

	//	Always play default sound

	g_pUniverse->PlaySound(pShip, g_pUniverse->FindSound(g_ShipExplosionSoundUNID));
	}

void CShipClass::CreateWreck (CShip *pShip, CSpaceObject **retpWreck)

//	CreateWreck
//
//	Creates a wreck for the given ship

	{
	//	Create the wreck

	CStation *pWreck;
	CreateEmptyWreck(pShip->GetSystem(),
			pShip,
			pShip->GetPos(),
			pShip->GetVel(),
			pShip->GetSovereign(),
			&pWreck);

	//	Add items to the wreck

	CItemListManipulator Source(pShip->GetItemList());
	CItemListManipulator Dest(pWreck->GetItemList());

	while (Source.MoveCursorForward())
		{
		CItem WreckItem = Source.GetItemAtCursor();
		CItemType *pType = WreckItem.GetType();

		//	Skip virtual items

		if (pType->IsVirtual())
			continue;

		//	Installed items may or may not be damaged.

		if (WreckItem.IsInstalled())
			{
			//	Make sure that the armor item reflects the current
			//	state of the ship's armor.

			if (pType->GetCategory() == itemcatArmor)
				{
				//	50% of the time, no armor is left

				if (mathRandom(1, 100) <= 50)
					continue;

				WreckItem.ClearDamaged();

				if (!pShip->IsArmorRepairable(WreckItem.GetInstalled()))
					continue;
				else if (pShip->IsArmorDamaged(WreckItem.GetInstalled()))
					WreckItem.SetDamaged();
				}

			//	Other installed devices have a chance of being
			//	damaged or destroyed.

			else
				{
				int iRoll = mathRandom(1, 100);

				//	50% of the time, no device is left

				if (iRoll <= 50)
					continue;

				//	45% of the time, the device is damaged

				else if (iRoll <= 95)
					{
					if (WreckItem.IsDamaged())
						continue;
					else if (CItemEnhancement(WreckItem.GetMods()).IsEnhancement())
						{
						CItemEnhancement Mods(WreckItem.GetMods());
						Mods.Combine(etLoseEnhancement);
						WreckItem.SetMods(Mods);
						}
					else if (WreckItem.IsEnhanced())
						WreckItem.ClearEnhanced();
					else
						WreckItem.SetDamaged();
					}
				}

			WreckItem.SetInstalled(-1);
			}

		//	Add to wreck

		Dest.AddItem(WreckItem);
		}

	//	The wreck is radioactive if the ship is radioactive (or if this
	//	ship class always has radioactive wrecks)

	if (pShip->IsRadioactive() || m_fRadioactiveWreck)
		pWreck->MakeRadioactive();

	//	Done

	if (retpWreck)
		*retpWreck = pWreck;
	}

void CShipClass::CreateWreckImage (void)

//	CreateWreckImage
//
//	Creates a wreck image randomly

	{
	ASSERT(!m_Image.IsEmpty());
	int cxWidth = RectWidth(m_Image.GetImageRect());
	int cyHeight = RectHeight(m_Image.GetImageRect());

	//	Get the image for damage

	if (g_pDamageBitmap == NULL)
		g_pDamageBitmap = g_pUniverse->GetLibraryBitmap(g_DamageImageUNID);

	//	Create the bitmap

	m_WreckImage.CreateBlank(cxWidth, cyHeight * WRECK_IMAGE_VARIANTS, true);

	//	Blt the images

	for (int i = 0; i < WRECK_IMAGE_VARIANTS; i++)
		{
		//	Pick a random rotation

		m_Image.CopyImage(m_WreckImage,
				0,
				i * cyHeight,
				0,
				mathRandom(0, m_iRotationRange - 1));

		//	Add some destruction

		int iCount = cxWidth * 2;
		for (int j = 0; j < iCount; j++)
			{
			m_WreckImage.ColorTransBlt(DAMAGE_IMAGE_WIDTH * mathRandom(0, DAMAGE_IMAGE_COUNT-1),
					0,
					DAMAGE_IMAGE_WIDTH,
					DAMAGE_IMAGE_COUNT,
					255,
					*g_pDamageBitmap,
					mathRandom(0, cxWidth-1) - (DAMAGE_IMAGE_WIDTH / 2),
					(i * cyHeight) + mathRandom(0, cyHeight-1) - (DAMAGE_IMAGE_HEIGHT / 2));
			}
		}

	m_WreckImage.SetTransparentColor(0x0000);
	}

void CShipClass::FindBestMissile (IItemGenerator *pItems, int *ioLevel)

//	FindBestMissile
//
//	Finds the best missile in the table

	{
	int i;

	for (i = 0; i < pItems->GetItemTypeCount(); i++)
		{
		CItemType *pType = pItems->GetItemType(i);
		if (pType->GetCategory() == itemcatMissile
				&& pType->GetLevel() > *ioLevel)
			*ioLevel = pType->GetLevel();
		}

	//	Recurse

	for (i = 0; i < pItems->GetGeneratorCount(); i++)
		FindBestMissile(pItems->GetGenerator(i), ioLevel);
	}

bool CShipClass::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns meta-data

	{
	int i;

	if (strEquals(sField, FIELD_CARGO_SPACE))
		*retsValue = strFromInt(GetCargoSpace());
	else if (strEquals(sField, FIELD_MAX_SPEED))
		{
		DriveDesc Desc;
		GetDriveDesc(&Desc);
		*retsValue = strFromInt((int)((100.0 * Desc.rMaxSpeed / LIGHT_SPEED) + 0.5), FALSE);
		}
	else if (strEquals(sField, FIELD_NAME))
		*retsValue = GetNounPhrase(0x00);
	else if (strEquals(sField, FIELD_MANUFACTURER))
		*retsValue = m_sManufacturer;
	else if (strEquals(sField, FIELD_PRIMARY_ARMOR))
		{
		CItemType *pItem = g_pUniverse->FindItemType(strToInt(GetDataField(FIELD_PRIMARY_ARMOR_UNID), 0));
		if (pItem)
			*retsValue = pItem->GetNounPhrase(0x80);
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_PRIMARY_ARMOR_UNID))
		{
		CArmorClass *pArmor = NULL;
		for (i = 0; i < GetHullSectionCount(); i++)
			{
			HullSection *pHull = GetHullSection(i);
			if (pArmor == NULL || pHull->pArmor->GetItemType()->GetLevel() > pArmor->GetItemType()->GetLevel())
				pArmor = pHull->pArmor;
			}
		if (pArmor)
			*retsValue = strFromInt(pArmor->GetItemType()->GetUNID());
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_SHIELD))
		{
		CItemType *pItem = g_pUniverse->FindItemType(strToInt(GetDataField(FIELD_SHIELD_UNID), 0));
		if (pItem)
			*retsValue = pItem->GetNounPhrase(0x80);
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_SHIELD_UNID))
		{
		CDeviceClass *pDevice = m_AverageDevices.GetNamedDevice(devShields);
		if (pDevice)
			*retsValue = strFromInt(pDevice->GetUNID());
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_LAUNCHER))
		{
		CItemType *pItem = g_pUniverse->FindItemType(strToInt(GetDataField(FIELD_LAUNCHER_UNID), 0));
		if (pItem)
			*retsValue = pItem->GetNounPhrase(0x80);
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_LAUNCHER_UNID))
		{
		CDeviceClass *pDevice = m_AverageDevices.GetNamedDevice(devMissileWeapon);
		if (pDevice)
			*retsValue = strFromInt(pDevice->GetUNID());
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_PRIMARY_WEAPON))
		{
		CItemType *pItem = g_pUniverse->FindItemType(strToInt(GetDataField(FIELD_PRIMARY_WEAPON_UNID), 0));
		if (pItem)
			*retsValue = pItem->GetNounPhrase(0x80);
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_PRIMARY_WEAPON_UNID))
		{
		CDeviceClass *pDevice = m_AverageDevices.GetNamedDevice(devPrimaryWeapon);
		if (pDevice)
			*retsValue = strFromInt(pDevice->GetUNID());
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_EXPLOSION_TYPE))
		{
		if (m_pExplosionType)
			{
			DWORD dwUNID = (DWORD)strToInt(m_pExplosionType->m_sUNID, 0);
			CWeaponClass *pClass = (CWeaponClass *)g_pUniverse->FindDeviceClass(dwUNID);
			if (pClass)
				{
				*retsValue = pClass->GetItemType()->GetNounPhrase();
				return true;
				}
			}

		*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_FIRE_ACCURACY))
		*retsValue = strFromInt(m_AISettings.iFireAccuracy);
	else if (strEquals(sField, FIELD_FIRE_RANGE_ADJ))
		*retsValue = strFromInt(m_AISettings.iFireRangeAdj);
	else if (strEquals(sField, FIELD_FIRE_RATE_ADJ))
		*retsValue = strFromInt((int)((1000.0 / m_AISettings.iFireRateAdj) + 0.5));
	else if (strEquals(sField, FIELD_MANEUVER))
		{
		int iManeuver = GetManeuverability() * GetRotationRange() / STD_ROTATION_COUNT;
		if (iManeuver <= 0)
			iManeuver = 1;

		*retsValue = strFromInt(30000 / iManeuver);
		}
	else if (strEquals(sField, FIELD_THRUST_TO_WEIGHT))
		{
		DriveDesc Drive;
		GetDriveDesc(&Drive);

		Metric rMass = CalcMass(m_AverageDevices);
		int iRatio = (int)((200.0 * (rMass > 0.0 ? Drive.iThrust / rMass : 0.0)) + 0.5);
		*retsValue = strFromInt(10 * iRatio);
		}
	else if (strEquals(sField, FIELD_PRIMARY_WEAPON_RANGE))
		{
		int iRange = 0;
		CItemType *pItem = g_pUniverse->FindItemType(strToInt(GetDataField(FIELD_PRIMARY_WEAPON_UNID), 0));
		if (pItem)
			{
			CDeviceClass *pDevice = pItem->GetDeviceClass();
			if (pDevice)
				{
				CWeaponClass *pWeapon = pDevice->AsWeaponClass();
				if (pWeapon)
					iRange = (int)((pWeapon->GetMaxEffectiveRange(NULL, NULL, NULL) / LIGHT_SECOND) + 0.5);
				}
			}
		*retsValue = strFromInt(iRange);
		}
	else if (strEquals(sField, FIELD_PRIMARY_WEAPON_RANGE_ADJ))
		{
		int iRange = 0;
		CItemType *pItem = g_pUniverse->FindItemType(strToInt(GetDataField(FIELD_PRIMARY_WEAPON_UNID), 0));
		if (pItem)
			{
			CDeviceClass *pDevice = pItem->GetDeviceClass();
			if (pDevice)
				{
				CWeaponClass *pWeapon = pDevice->AsWeaponClass();
				if (pWeapon)
					{
					iRange = (int)((pWeapon->GetMaxEffectiveRange(NULL, NULL, NULL) / LIGHT_SECOND) + 0.5);
					iRange = iRange * m_AISettings.iFireRangeAdj / 100;
					}
				}
			}
		*retsValue = strFromInt(iRange);
		}
	else if (strEquals(sField, FIELD_SCORE))
		*retsValue = strFromInt(m_iScore);
	else if (strEquals(sField, FIELD_LEVEL))
		*retsValue = strFromInt(m_iLevel);
	else
		return false;

	return true;
	}

CString CShipClass::GenerateShipName (DWORD *retdwFlags)

//	GenerateShipName
//
//	Generate a ship name

	{
	if (!m_sShipNames.IsBlank())
		{
		*retdwFlags = m_dwShipNameFlags;
		return GenerateRandomName(m_sShipNames, NULL_STR);
		}
	else
		{
		*retdwFlags = 0;
		return NULL_STR;
		}
	}

void CShipClass::GetDriveDesc (DriveDesc *retDriveDesc)

//	GetDriveDesc
//
//	Returns the drive desc for the hull plus any device

	{
	*retDriveDesc = *GetHullDriveDesc();

	CDeviceClass *pDrive = m_AverageDevices.GetNamedDevice(devDrive);
	if (pDrive)
		{
		const DriveDesc *pDriveDesc = pDrive->GetDriveDesc();

		retDriveDesc->dwUNID = pDriveDesc->dwUNID;
		retDriveDesc->fInertialess = pDriveDesc->fInertialess;
		retDriveDesc->iPowerUse = pDriveDesc->iPowerUse;
		retDriveDesc->iThrust += pDriveDesc->iThrust;
		retDriveDesc->rMaxSpeed = Max(retDriveDesc->rMaxSpeed, pDriveDesc->rMaxSpeed);
		}
	}

int CShipClass::GetHullSectionAtAngle (int iAngle)

//	GetHullSectionAtAngle
//
//	Returns the index of the hull section at the given
//	angle.

	{
	int i;

	for (i = 0; i < GetHullSectionCount(); i++)
		{
		HullSection *pSect = GetHullSection(i);

		int iStart = pSect->iStartAt;
		int iEnd = (pSect->iStartAt + pSect->iSpan) % 360;

		if (iEnd > iStart)
			{
			if (iAngle >= iStart && iAngle < iEnd)
				return i;
			}
		else
			{
			if (iAngle < iEnd || iAngle >= iStart)
				return i;
			}
		}

	//	The last hull section may wrap around to the beginning again. If we haven't
	//	found the angle yet, assume it is the last section.

	return GetHullSectionCount() - 1;
	}

CString CShipClass::GetName (DWORD *retdwFlags)

//	GetName
//
//	Returns the name of the ship class

	{
	if (retdwFlags)
		*retdwFlags = m_dwClassNameFlags;

	if (GetClassName().IsBlank())
		return strPatternSubst(LITERAL("%s %s"), GetManufacturerName().GetASCIIZPointer(), GetTypeName().GetASCIIZPointer());
	else
		{
		if (GetTypeName().IsBlank())
			return GetClassName();
		else
			return strPatternSubst(LITERAL("%s-class %s"), GetClassName().GetASCIIZPointer(), GetTypeName().GetASCIIZPointer());
		}
	}

CString CShipClass::GetNounPhrase (DWORD dwFlags)

//	GetNounPhrase
//
//	Returns the generic name of the item

	{
	DWORD dwNameFlags;

	CString sName;
	if (dwFlags & nounShort)
		{
		sName = GetShortName();
		dwNameFlags = m_dwClassNameFlags;
		}
	else
		sName = GetName(&dwNameFlags);

	return ::ComposeNounPhrase(sName, 1, NULL_STR, dwNameFlags, dwFlags);
	}

CXMLElement *CShipClass::GetScreen (const CString &sName)

//	GetScreen
//
//	Returns the given dock screen.

	{
	if (sName.IsBlank())
		return m_pDefaultScreen.GetDesc();
	else
		{
		CDockScreenTypeRef Screen;
		Screen.LoadUNID(sName);
		Screen.Bind(m_pDockScreens);
		return Screen.GetDesc();
		}
	}

CString CShipClass::GetShortName (void)

//	GetShortName
//
//	Returns the short name of the class

	{
	if (GetClassName().IsBlank())
		return strPatternSubst(LITERAL("%s %s"), GetManufacturerName().GetASCIIZPointer(), GetTypeName().GetASCIIZPointer());
	else
		return GetClassName();
	}

CStationType *CShipClass::GetWreckDesc (void)
	{
	if (g_pWreckDesc == NULL)
		g_pWreckDesc = g_pUniverse->FindStationType(g_ShipWreckUNID);

	return g_pWreckDesc;
	}

void CShipClass::GetWreckImage (CObjectImageArray *retWreckImage)

//	GetWreckImage
//
//	Returns wreck image

	{
	RECT rcRect;
	rcRect.left = 0;
	rcRect.top = 0;
	rcRect.right = RectWidth(m_Image.GetImageRect());
	rcRect.bottom = RectHeight(m_Image.GetImageRect());
	retWreckImage->Init(&m_WreckImage, rcRect, 0, 0);
	}

int CShipClass::GetWreckImageVariants (void)

//	GetWreckImageVariants
//
//	Returns the number of wreck images

	{
	return WRECK_IMAGE_VARIANTS;
	}

void CShipClass::LoadImages (void)

//	LoadImages
//
//	Loads images used by the ship

	{
	m_Image.LoadImage();

	if (m_pExplosionType)
		m_pExplosionType->LoadImages();
	}

void CShipClass::MarkImages (void)

//	MarkImages
//
//	Marks images used by the ship

	{
	m_Image.MarkImage();

	if (m_pExplosionType)
		m_pExplosionType->MarkImages();
	}

ALERROR CShipClass::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind design

	{
	ALERROR error;
	int i;
	int iPrimaryCount = 0;

	if (error = m_Image.OnDesignLoadComplete(Ctx))
		goto Fail;

	if (error = m_ExhaustImage.OnDesignLoadComplete(Ctx))
		goto Fail;

	if (error = m_pDefaultScreen.Bind(Ctx, m_pDockScreens))
		goto Fail;

	for (i = 0; i < GetHullSectionCount(); i++)
		if (error = GetHullSection(i)->pArmor.Bind(Ctx))
			goto Fail;

	if ((error = m_pExplosionType.Bind(Ctx)))
		goto Fail;

	if (m_pPlayerSettings)
		{
		if (error = m_pPlayerSettings->m_pShipScreen.Bind(Ctx, m_pDockScreens))
			goto Fail;

		for (i = 0; i < m_pPlayerSettings->m_iArmorDescCount; i++)
			{
			SArmorImageDesc &ArmorDesc = m_pPlayerSettings->m_pArmorDesc[i];

			if (error = ArmorDesc.Image.OnDesignLoadComplete(Ctx))
				goto Fail;
			}

		if (error = m_pPlayerSettings->m_ShieldImage.OnDesignLoadComplete(Ctx))
			goto Fail;

		if (error = m_pPlayerSettings->m_ReactorImage.OnDesignLoadComplete(Ctx))
			goto Fail;

		if (error = m_pPlayerSettings->m_PowerLevelImage.OnDesignLoadComplete(Ctx))
			goto Fail;

		if (error = m_pPlayerSettings->m_FuelLevelImage.OnDesignLoadComplete(Ctx))
			goto Fail;

		if (error = m_pPlayerSettings->m_FuelLowLevelImage.OnDesignLoadComplete(Ctx))
			goto Fail;
		}

	//	Items

	if (m_pItems)
		if (error = m_pItems->OnDesignLoadComplete(Ctx))
			goto Fail;

	if (m_pDevices)
		if (error = m_pDevices->OnDesignLoadComplete(Ctx))
			goto Fail;

	//	Create the random wreck images

	if (!m_Image.IsEmpty())
		CreateWreckImage();

	//	Generate an average set of devices

	GenerateDevices(m_AverageDevices);

	//	Compute score and level

	if (m_iScore == 0)
		m_iScore = CalcScore();

	m_iLevel = ComputeLevel(m_iScore);

	return NOERROR;

Fail:

	Ctx.sError = ComposeLoadError(Ctx.sError);
	return error;
	}

ALERROR CShipClass::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Create from XML

	{
	ALERROR error;
	int i;

	//	Initialize basic info

	CString sAttrib;
	m_sManufacturer = pDesc->GetAttribute(CONSTLIT(g_ManufacturerAttrib));
	m_sName = pDesc->GetAttribute(CONSTLIT(g_ClassAttrib));
	m_sTypeName = pDesc->GetAttribute(CONSTLIT(g_TypeAttrib));
	m_dwClassNameFlags = LoadNameFlags(pDesc);
	m_iScore = pDesc->GetAttributeInteger(SCORE_ATTRIB);
	m_sAttributes = pDesc->GetAttribute(ATTRIBUTES_ATTRIB);

	CXMLElement *pNames = pDesc->GetContentElementByTag(NAMES_TAG);
	if (pNames)
		{
		m_sShipNames = pNames->GetContentText(0);
		m_dwShipNameFlags = LoadNameFlags(pNames);
		}
	else
		m_dwShipNameFlags = 0;

	//	Load the images

	if (error = m_Image.InitFromXML(Ctx, 
			pDesc->GetContentElementByTag(IMAGE_TAG)))
		{
		Ctx.sError = ComposeLoadError(ERR_BAD_IMAGE);
		return error;
		}

	//	Initialize design

	m_iMass = pDesc->GetAttributeInteger(CONSTLIT(g_MassAttrib));
	m_iCargoSpace = pDesc->GetAttributeInteger(CARGO_SPACE_ATTRIB);
	m_iMaxCargoSpace = Max(m_iCargoSpace, pDesc->GetAttributeInteger(MAX_CARGO_SPACE_ATTRIB));
	m_iManeuverability = pDesc->GetAttributeInteger(CONSTLIT(g_ManeuverAttrib));
	m_iManeuverDelay = (int)(((Metric)m_iManeuverability / STD_SECONDS_PER_UPDATE) + 0.5);
	m_iMaxArmorMass = pDesc->GetAttributeInteger(MAX_ARMOR_ATTRIB);
	m_iMaxReactorPower = pDesc->GetAttributeInteger(MAX_REACTOR_POWER_ATTRIB);

	m_iRotationRange = pDesc->GetAttributeInteger(ROTATION_COUNT_ATTRIB);
	if (m_iRotationRange <= 0)
		m_iRotationRange = STD_ROTATION_COUNT;
	m_iRotationAngle = (360 / m_iRotationRange);
	m_Image.SetRotationCount(m_iRotationRange);

	m_DriveDesc.dwUNID = GetUNID();
	m_DriveDesc.rMaxSpeed = (double)pDesc->GetAttributeInteger(CONSTLIT(g_MaxSpeedAttrib)) * LIGHT_SPEED / 100;
	m_DriveDesc.iThrust = pDesc->GetAttributeInteger(CONSTLIT(g_ThrustAttrib));
	m_DriveDesc.iPowerUse = DEFAULT_POWER_USE;
	m_DriveDesc.fInertialess = pDesc->GetAttributeBool(INERTIALESS_DRIVE_ATTRIB);

	m_ReactorDesc.iMaxPower = pDesc->GetAttributeInteger(REACTOR_POWER_ATTRIB);
	if (m_ReactorDesc.iMaxPower == 0)
		m_ReactorDesc.iMaxPower = 100;
	m_ReactorDesc.iMaxFuel = pDesc->GetAttributeInteger(MAX_REACTOR_FUEL_ATTRIB);
	if (m_ReactorDesc.iMaxFuel == 0)
		m_ReactorDesc.iMaxFuel = m_ReactorDesc.iMaxPower * 250;
	m_ReactorDesc.iMinFuelLevel = 1;
	m_ReactorDesc.iMaxFuelLevel = 3;
	m_ReactorDesc.iPowerPerFuelUnit = g_MWPerFuelUnit;
	m_ReactorDesc.fDamaged = false;
	m_ReactorDesc.fEnhanced = false;

	m_iCyberDefenseLevel = Max(1, pDesc->GetAttributeInteger(CYBER_DEFENSE_LEVEL_ATTRIB));

	m_fTimeStopImmune = pDesc->GetAttributeBool(TIME_STOP_IMMUNE_ATTRIB);

	//	Load armor

	CXMLElement *pArmor = pDesc->GetContentElementByTag(CONSTLIT(g_ArmorTag));
	if (pArmor == NULL)
		{
		Ctx.sError = ComposeLoadError(ERR_MISSING_ARMOR_TAG);
		return ERR_FAIL;
		}

	for (i = 0; i < pArmor->GetContentElementCount(); i++)
		{
		CXMLElement *pSectionDesc = pArmor->GetContentElement(i);
		HullSection Section;

		Section.iStartAt = pSectionDesc->GetAttributeInteger(CONSTLIT(g_StartAttrib));
		Section.iSpan = pSectionDesc->GetAttributeInteger(CONSTLIT(g_SpanAttrib));
		Section.pArmor.LoadUNID(Ctx, pSectionDesc->GetAttribute(ARMOR_ID_ATTRIB));
		Section.dwAreaSet = ParseNonCritical(pSectionDesc->GetAttribute(NON_CRITICAL_ATTRIB));

		//	Add the section

		if (error = m_Hull.AppendStruct(&Section, NULL))
			{
			Ctx.sError = ComposeLoadError(ERR_OUT_OF_MEMORY);
			return error;
			}
		}

	//	Load devices

	CXMLElement *pDevices = pDesc->GetContentElementByTag(DEVICES_TAG);
	if (pDevices)
		{
		if (error = IDeviceGenerator::CreateFromXML(Ctx, pDevices, &m_pDevices))
			return error;
		}

	m_iMaxDevices = pDesc->GetAttributeInteger(MAX_DEVICES_ATTRIB);
	if (m_iMaxDevices == 0)
		m_iMaxDevices = -1;
	m_iMaxWeapons = pDesc->GetAttributeInteger(MAX_WEAPONS_ATTRIB);
	if (m_iMaxWeapons == 0)
		m_iMaxWeapons = m_iMaxDevices;
	m_iMaxNonWeapons = pDesc->GetAttributeInteger(MAX_NON_WEAPONS_ATTRIB);
	if (m_iMaxNonWeapons == 0)
		m_iMaxNonWeapons = m_iMaxDevices;

	//	Load AI settings

	CXMLElement *pAISettings = pDesc->GetContentElementByTag(CONSTLIT(g_AISettingsTag));
	if (pAISettings)
		{
		m_AISettings.iFireRateAdj = pAISettings->GetAttributeInteger(FIRE_RATE_ADJ_ATTRIB);
		m_AISettings.iFireRangeAdj = pAISettings->GetAttributeInteger(FIRE_RANGE_ADJ_ATTRIB);
		if (m_AISettings.iFireRangeAdj <= 0)
			m_AISettings.iFireRangeAdj = 100;
		m_AISettings.iFireAccuracy = pAISettings->GetAttributeInteger(FIRE_ACCURACY_ATTRIB);
		m_AISettings.iPerception = pAISettings->GetAttributeInteger(PERCEPTION_ATTRIB);
		m_AISettings.dwFlags = 0;

		if (pAISettings->GetAttributeBool(NO_SHIELD_RETREAT_ATTRIB))
			m_AISettings.dwFlags |= aiflagNoShieldRetreat;

		if (pAISettings->GetAttributeBool(NO_DOGFIGHTS_ATTRIB))
			m_AISettings.dwFlags |= aiflagNoDogfights;

		if (pAISettings->GetAttributeBool(NON_COMBATANT_ATTRIB))
			m_AISettings.dwFlags |= aiflagNonCombatant;

		if (pAISettings->GetAttributeBool(NO_FRIENDLY_FIRE_ATTRIB))
			m_AISettings.dwFlags |= aiflagNoFriendlyFire;

		CString sCombatStyle;
		if (pAISettings->FindAttribute(COMBAT_STYLE_ATTRIB, &sCombatStyle))
			{
			if (strEquals(sCombatStyle, COMBAT_STYLE_STAND_OFF))
				m_AISettings.iCombatStyle = aicombatStandOff;
			else if (strEquals(sCombatStyle, COMBAT_STYLE_FLYBY))
				m_AISettings.iCombatStyle = aicombatFlyby;
			else if (strEquals(sCombatStyle, COMBAT_STYLE_NO_RETREAT))
				m_AISettings.iCombatStyle = aicombatNoRetreat;
			else
				m_AISettings.iCombatStyle = aicombatStandard;

			}
		else
			{
			//	Compatibility with version < 0.97

			if (pAISettings->GetAttributeBool(FLYBY_COMBAT_ATTRIB))
				m_AISettings.iCombatStyle = aicombatFlyby;
			else if (pAISettings->GetAttributeBool(STAND_OFF_COMBAT_ATTRIB))
				m_AISettings.iCombatStyle = aicombatStandOff;
			else
				m_AISettings.iCombatStyle = aicombatStandard;
			}
		}
	else
		m_AISettings = g_DefaultAISettings;

	//	Load items

	CXMLElement *pItems = pDesc->GetContentElementByTag(ITEMS_TAG);
	if (pItems)
		{
		if (error = IItemGenerator::CreateFromXML(Ctx, pItems, &m_pItems))
			return error;
		}

	//	Drive images

	m_iExhaustImageCount = 0;
	CXMLElement *pDriveImages = pDesc->GetContentElementByTag(DRIVE_IMAGES_TAG);
	if (pDriveImages)
		{
		for (i = 0; i < pDriveImages->GetContentElementCount(); i++)
			{
			CXMLElement *pItem = pDriveImages->GetContentElement(i);
			if (strEquals(pItem->GetTag(), NOZZLE_IMAGE_TAG))
				{
				if (error = m_ExhaustImage.InitFromXML(Ctx, pItem))
					{
					Ctx.sError = ComposeLoadError(ERR_BAD_EXHAUST_IMAGE);
					return error;
					}
				}
			else if (strEquals(pItem->GetTag(), NOZZLE_POS_TAG)
					&& m_iExhaustImageCount < maxExhaustImages)
				{
				int x = pItem->GetAttributeInteger(X_ATTRIB);
				int y = -pItem->GetAttributeInteger(Y_ATTRIB);

				m_ptExhaustOffset[m_iExhaustImageCount] = new POINT [m_iRotationRange];

				for (int j = 0; j < m_iRotationRange; j++)
					{
					int iAngle = j * m_iRotationAngle;
					CVector vOffset(x + 0.5, y + 0.5);
					vOffset = vOffset.Rotate(iAngle);
					m_ptExhaustOffset[m_iExhaustImageCount][Angle2Direction(iAngle)].x = (int)vOffset.GetX();
					m_ptExhaustOffset[m_iExhaustImageCount][Angle2Direction(iAngle)].y = -(int)vOffset.GetY();
					}

				m_dwExhaustExclusion[m_iExhaustImageCount] = pItem->GetAttributeInteger(EXCLUSION_ATTRIB);
				m_iExhaustImageCount++;
				}
			else
				{
				Ctx.sError = ComposeLoadError(ERR_DRIVE_IMAGE_FORMAT);
				return ERR_FAIL;
				}
			}
		}

	//	Initialize docking data

	CDockingPorts DockingPorts;
	DockingPorts.InitPortsFromXML(NULL, pDesc);
	m_iDockingPortsCount = DockingPorts.GetPortCount(NULL);
	if (m_iDockingPortsCount > 0)
		{
		m_DockingPorts = new CVector [m_iDockingPortsCount];
		for (i = 0; i < m_iDockingPortsCount; i++)
			m_DockingPorts[i] = DockingPorts.GetPortPos(NULL, i);

		//	Load dock screens

		CXMLElement *pLocals = pDesc->GetContentElementByTag(DOCK_SCREENS_TAG);
		if (pLocals)
			m_pDockScreens = pLocals->OrphanCopy();
		else
			m_pDockScreens = NULL;

		//	Load the default screen

		m_pDefaultScreen.LoadUNID(Ctx, pDesc->GetAttribute(DOCK_SCREEN_ATTRIB));
		if (m_pDefaultScreen.GetUNID().IsBlank())
			{
			Ctx.sError = ComposeLoadError(ERR_DOCK_SCREEN_NEEDED);
			return ERR_FAIL;
			}

		//	Background screens

		m_dwDefaultBkgnd = LoadUNID(Ctx, pDesc->GetAttribute(DEFAULT_BACKGROUND_ID_ATTRIB));

		m_fHasDockingPorts = true;
		}
	else
		{
		m_DockingPorts = NULL;
		m_pDockScreens = NULL;
		m_dwDefaultBkgnd = 0;

		m_fHasDockingPorts = false;
		}

	m_fHasOnOrderChangedEvent = FindEventHandler(CONSTLIT("OnOrderChanged"));
	m_fHasOnAttackedByPlayerEvent = FindEventHandler(CONSTLIT("OnAttackedByPlayer"));
	m_fHasOnObjDockedEvent = FindEventHandler(CONSTLIT("OnObjDocked"));
	m_fHasOnOrdersCompletedEvent = FindEventHandler(CONSTLIT("OnOrdersCompleted"));

	//	Load communications

	CXMLElement *pComms = pDesc->GetContentElementByTag(COMMUNICATIONS_TAG);
	if (pComms)
		if (error = m_CommsHandler.InitFromXML(pComms, &Ctx.sError))
			return error;

	//	Miscellaneous

	m_iLeavesWreck = pDesc->GetAttributeInteger(LEAVES_WRECK_ATTRIB);
	m_fRadioactiveWreck = pDesc->GetAttributeBool(RADIOACTIVE_WRECK_ATTRIB);
	m_iStructuralHP = pDesc->GetAttributeInteger(MAX_STRUCTURAL_HIT_POINTS_ATTRIB);

	//	Explosion

	m_pExplosionType.LoadUNID(Ctx, pDesc->GetAttribute(EXPLOSION_TYPE_ATTRIB));

	//	Load player settings

	CXMLElement *pPlayer = pDesc->GetContentElementByTag(PLAYER_SETTINGS_TAG);
	if (pPlayer)
		{
		m_pPlayerSettings = new SPlayerSettings;
		m_pPlayerSettings->m_sDesc = pPlayer->GetAttribute(DESC_ATTRIB);
		m_pPlayerSettings->m_dwLargeImage = LoadUNID(Ctx, pPlayer->GetAttribute(LARGE_IMAGE_ATTRIB));
		m_pPlayerSettings->m_fDebug = pPlayer->GetAttributeBool(DEBUG_ONLY_ATTRIB);
		m_pPlayerSettings->m_fInitialClass = pPlayer->GetAttributeBool(INITIAL_CLASS_ATTRIB);

		//	Load some miscellaneous data

		CString sAttrib;
		if (pPlayer->FindAttribute(STARTING_CREDITS_ATTRIB, &sAttrib))
			{
			if (error = m_pPlayerSettings->m_StartingCredits.LoadFromXML(sAttrib))
				{
				Ctx.sError = ComposeLoadError(ERR_INVALID_STARTING_CREDITS);
				return ERR_FAIL;
				}
			}
		else
			m_pPlayerSettings->m_StartingCredits = DiceRange(20, 5, 200);

		m_pPlayerSettings->m_sStartNode = pPlayer->GetAttribute(STARTING_SYSTEM_ATTRIB);
		m_pPlayerSettings->m_sStartPos = pPlayer->GetAttribute(STARTING_POS_ATTRIB);
		if (m_pPlayerSettings->m_sStartPos.IsBlank())
			m_pPlayerSettings->m_sStartPos = CONSTLIT("Start");

		//	Load the ship screen

		CString sShipScreenUNID = pPlayer->GetAttribute(SHIP_SCREEN_ATTRIB);
		if (sShipScreenUNID.IsBlank())
			sShipScreenUNID = strFromInt(DEFAULT_SHIP_SCREEN_UNID, FALSE);
		m_pPlayerSettings->m_pShipScreen.LoadUNID(Ctx, sShipScreenUNID);

		//	Load the armor display data

		CXMLElement *pArmorDisplay = pPlayer->GetContentElementByTag(ARMOR_DISPLAY_TAG);
		if (pArmorDisplay == NULL || pArmorDisplay->GetContentElementCount() == 0)
			{
			Ctx.sError = ComposeLoadError(ERR_ARMOR_DISPLAY_NEEDED);
			return ERR_FAIL;
			}

		m_pPlayerSettings->m_iArmorDescCount = pArmorDisplay->GetContentElementCount();
		m_pPlayerSettings->m_pArmorDesc = new SArmorImageDesc [m_pPlayerSettings->m_iArmorDescCount];
		for (i = 0; i < m_pPlayerSettings->m_iArmorDescCount; i++)
			{
			SArmorImageDesc &ArmorDesc = m_pPlayerSettings->m_pArmorDesc[i];
			CXMLElement *pSegment = pArmorDisplay->GetContentElement(i);

			if (error = ArmorDesc.Image.InitFromXML(Ctx, pSegment))
				{
				Ctx.sError = ComposeLoadError(ERR_ARMOR_DISPLAY_NEEDED);
				return ERR_FAIL;
				}

			ArmorDesc.sName = pSegment->GetAttribute(NAME_ATTRIB);
			ArmorDesc.xDest = pSegment->GetAttributeInteger(DEST_X_ATTRIB);
			ArmorDesc.yDest = pSegment->GetAttributeInteger(DEST_Y_ATTRIB);
			ArmorDesc.xHP = pSegment->GetAttributeInteger(HP_X_ATTRIB);
			ArmorDesc.yHP = pSegment->GetAttributeInteger(HP_Y_ATTRIB);
			ArmorDesc.yName = pSegment->GetAttributeInteger(NAME_Y_ATTRIB);
			ArmorDesc.cxNameBreak = pSegment->GetAttributeInteger(NAME_BREAK_WIDTH);
			ArmorDesc.xNameDestOffset = pSegment->GetAttributeInteger(NAME_DEST_X_ATTRIB);
			ArmorDesc.yNameDestOffset = pSegment->GetAttributeInteger(NAME_DEST_Y_ATTRIB);
			}

		//	Load shield display data

		CXMLElement *pShieldDisplay = pPlayer->GetContentElementByTag(SHIELD_DISPLAY_TAG);
		if (pShieldDisplay == NULL)
			{
			Ctx.sError = ComposeLoadError(ERR_SHIELD_DISPLAY_NEEDED);
			return ERR_FAIL;
			}

		if (error = m_pPlayerSettings->m_ShieldImage.InitFromXML(Ctx, 
				pShieldDisplay->GetContentElementByTag(IMAGE_TAG)))
			{
			Ctx.sError = ComposeLoadError(ERR_SHIELD_DISPLAY_NEEDED);
			return error;
			}

		//	Load reactor display data

		CXMLElement *pReactorDisplay = pPlayer->GetContentElementByTag(REACTOR_DISPLAY_TAG);
		if (pReactorDisplay == NULL)
			{
			Ctx.sError = ComposeLoadError(ERR_REACTOR_DISPLAY_NEEDED);
			return ERR_FAIL;
			}

		if (error = m_pPlayerSettings->m_ReactorImage.InitFromXML(Ctx,
				pReactorDisplay->GetContentElementByTag(IMAGE_TAG)))
			{
			Ctx.sError = ComposeLoadError(ERR_REACTOR_DISPLAY_NEEDED);
			return error;
			}

		CXMLElement *pImage = pReactorDisplay->GetContentElementByTag(POWER_LEVEL_IMAGE_TAG);
		if (pImage == NULL || (error = m_pPlayerSettings->m_PowerLevelImage.InitFromXML(Ctx, pImage)))
			{
			Ctx.sError = ComposeLoadError(ERR_REACTOR_DISPLAY_NEEDED);
			return error;
			}

		m_pPlayerSettings->m_xPowerLevelImage = pImage->GetAttributeInteger(DEST_X_ATTRIB);
		m_pPlayerSettings->m_yPowerLevelImage = pImage->GetAttributeInteger(DEST_Y_ATTRIB);

		pImage = pReactorDisplay->GetContentElementByTag(FUEL_LEVEL_IMAGE_TAG);
		if (pImage == NULL || (error = m_pPlayerSettings->m_FuelLevelImage.InitFromXML(Ctx, pImage)))
			{
			Ctx.sError = ComposeLoadError(ERR_REACTOR_DISPLAY_NEEDED);
			return ERR_FAIL;
			}

		m_pPlayerSettings->m_xFuelLevelImage = pImage->GetAttributeInteger(DEST_X_ATTRIB);
		m_pPlayerSettings->m_yFuelLevelImage = pImage->GetAttributeInteger(DEST_Y_ATTRIB);

		pImage = pReactorDisplay->GetContentElementByTag(FUEL_LOW_LEVEL_IMAGE_TAG);
		if (pImage == NULL || (error = m_pPlayerSettings->m_FuelLowLevelImage.InitFromXML(Ctx, pImage)))
			{
			Ctx.sError = ComposeLoadError(ERR_REACTOR_DISPLAY_NEEDED);
			return error;
			}

		if (error = InitRectFromElement(pReactorDisplay->GetContentElementByTag(REACTOR_TEXT_TAG),
				&m_pPlayerSettings->m_rcReactorText))
			{
			Ctx.sError = ComposeLoadError(ERR_REACTOR_DISPLAY_NEEDED);
			return error;
			}

		if (error = InitRectFromElement(pReactorDisplay->GetContentElementByTag(POWER_LEVEL_TEXT_TAG),
				&m_pPlayerSettings->m_rcPowerLevelText))
			{
			Ctx.sError = ComposeLoadError(ERR_REACTOR_DISPLAY_NEEDED);
			return error;
			}

		if (error = InitRectFromElement(pReactorDisplay->GetContentElementByTag(FUEL_LEVEL_TEXT_TAG),
				&m_pPlayerSettings->m_rcFuelLevelText))
			{
			Ctx.sError = ComposeLoadError(ERR_REACTOR_DISPLAY_NEEDED);
			return error;
			}
		}

	//	Done

	return NOERROR;
	}

void CShipClass::OnReadFromStream (SUniverseLoadCtx &Ctx)

//	ReadFromStream
//
//	Read from stream
//
//	DWORD		flags
//	CAttributeDataBlock	m_Data

	{
	DWORD dwLoad;
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	
	//	Load opaque data

	ReadGlobalData(Ctx);
	}

void CShipClass::OnReinit (void)

//	OnReinit
//
//	Reinitialize the class

	{
	g_pWreckDesc = NULL;
	}

void CShipClass::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Writes out the variable portions of the item type
//
//	DWORD		flags

	{
	DWORD dwSave = 0;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

void CShipClass::Paint (CG16bitImage &Dest, 
						int x, 
						int y, 
						const ViewportTransform &Trans, 
						int iDirection, 
						int iTick,
						bool bThrusting,
						bool bRadioactive)

//	Paint
//
//	Paints the ship class

	{
	if (bRadioactive)
		m_Image.PaintImageWithGlow(Dest, x, y, iTick, iDirection, CGImage::RGBColor(0, 255, 0));
	else
		m_Image.PaintImage(Dest, x, y, iTick, iDirection);

	if (bThrusting && ((iTick % 4) != 0))
		{
		DWORD dwExclusion = (1 << iDirection);

		for (int i = 0; i < m_iExhaustImageCount; i++)
			if (!(m_dwExhaustExclusion[i] & dwExclusion))
				m_ExhaustImage.PaintImage(Dest,
						x + m_ptExhaustOffset[i][iDirection].x,
						y + m_ptExhaustOffset[i][iDirection].y,
						iTick,
						iDirection);
		}
	}

void CShipClass::PaintMap (CG16bitImage &Dest, 
						int x, 
						int y, 
						const ViewportTransform &Trans, 
						int iDirection, 
						int iTick,
						bool bThrusting,
						bool bRadioactive)

//	PaintMap
//
//	Paints the ship class on the map

	{
	m_Image.PaintScaledImage(Dest, x, y, iTick, iDirection, 24, 24);
	}

void CShipClass::SetShipwreckImage (CStation *pStation)

//	SetShipwreckImage
//
//	Sets the image for the given station to be a wreck of
//	this ship class

	{
	CObjectImageArray WreckImage;
	RECT rcRect;
	rcRect.left = 0;
	rcRect.top = 0;
	rcRect.right = RectWidth(m_Image.GetImageRect());
	rcRect.bottom = RectHeight(m_Image.GetImageRect());
	WreckImage.Init(&m_WreckImage, rcRect, 0, 0);

	pStation->SetWreckImage(GetUNID(), WreckImage, WRECK_IMAGE_VARIANTS);
	}

void CShipClass::SetShipwreckParams (CStation *pStation, CShip *pShip)

//	SetShipwreckParams
//
//	Sets shipwreck parameters such as name and hit points

	{
	//	If the station doesn't have a name, set it now

	CString sOldName = pStation->GetName(NULL);
	if (sOldName.IsBlank())
		{
		CString sName;

		//	If the ship has a proper name, then use that

		if (!m_sShipNames.IsBlank() && pShip)
			{
			DWORD dwNounFlags;
			sName = pShip->GetName(&dwNounFlags);
			sName = strPatternSubst(CONSTLIT("Wreck of %s"),
					ComposeNounPhrase(sName, 1, CString(), dwNounFlags, nounArticle).GetASCIIZPointer());
			}

		//	Otherwise, use the generic name for the class

		else
			sName = strPatternSubst(CONSTLIT("%s wreck"), GetName().GetASCIIZPointer());

		pStation->SetName(sName);
		}

	//	Set the mass

	pStation->SetMass(GetHullMass());

	//	Set hit points for the structure

	int iHP;
	if (m_iStructuralHP)
		iHP = m_iStructuralHP;
	else
		iHP = (m_iLevel + 1) * (mathSqrt(m_iMass) + 10) * mathRandom(75, 125) / 200;

	pStation->SetStructuralHitPoints(iHP);
	pStation->SetMaxStructuralHitPoints(iHP);
	}

ALERROR InitRectFromElement (CXMLElement *pItem, RECT *retRect)

//	InitRectFromElement
//
//	Initializes the rect from x, y, width, height attributes

	{
	if (pItem == NULL)
		return ERR_FAIL;

	retRect->left = pItem->GetAttributeInteger(X_ATTRIB);
	retRect->top = pItem->GetAttributeInteger(Y_ATTRIB);
	retRect->right = retRect->left + pItem->GetAttributeInteger(WIDTH_ATTRIB);
	retRect->bottom = retRect->top + pItem->GetAttributeInteger(HEIGHT_ATTRIB);

	return NOERROR;
	}

DWORD ParseNonCritical (const CString &sList)

//	ParseNonCritical
//
//	Returns the set of non-critical areas

	{
	//	These must match the order of VitalSections in TSE.h

	static char *g_pszNonCritical[] =
		{
		"dev0",
		"dev1",
		"dev2",
		"dev3",
		"dev4",
		"dev5",
		"dev6",
		"dev7",

		"maneuver",
		"drive",
		"scanners",
		"tactical",
		"cargo",

		"",
		};

	//	Blank means critical

	if (sList.IsBlank())
		return CShipClass::sectCritical;

	//	Loop over list

	DWORD dwSet = 0;
	char *pPos = sList.GetASCIIZPointer();
	while (*pPos != '\0')
		{
		//	Trim spaces

		while (*pPos != '\0' && *pPos == ' ')
			pPos++;

		//	Which of the items do we match?

		int i = 0;
		DWORD dwArea = 0x1;
		char *pFind;
		while (*(pFind = g_pszNonCritical[i]))
			{
			char *pSource = pPos;

			while (*pFind != '\0' && *pFind == *pSource)
				{
				pFind++;
				pSource++;
				}

			//	If we matched then we've got this area

			if (*pFind == '\0' && (*pSource == ' ' || *pSource == ';' || *pSource == '\0'))
				{
				dwSet |= dwArea;
				pPos = pSource;
				break;
				}

			//	Next

			i++;
			dwArea = dwArea << 1;
			}

		//	Skip to the next modifier

		while (*pPos != '\0' && *pPos != ';')
			pPos++;

		if (*pPos == ';')
			pPos++;
		}

	//	Done

	return dwSet;
	}
