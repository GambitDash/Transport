//	CStationType.cpp
//
//	CStationType class

#include "PreComp.h"

#define REINFORCEMENTS_TAG						CONSTLIT("Reinforcements")
#define SATELLITES_TAG							CONSTLIT("Satellites")
#define IMAGE_TAG								CONSTLIT("Image")
#define DOCK_SCREENS_TAG						CONSTLIT("DockScreens")
#define EVENTS_TAG								CONSTLIT("Events")
#define ENCOUNTERS_TAG							CONSTLIT("Encounters")
#define CONSTRUCTION_TAG						CONSTLIT("Construction")
#define DEVICES_TAG								CONSTLIT("Devices")
#define ANIMATIONS_TAG							CONSTLIT("Animations")
#define DOCKING_PORTS_TAG						CONSTLIT("DockingPorts")
#define ITEMS_TAG								CONSTLIT("Items")
#define TRADE_TAG								CONSTLIT("Trade")
#define COMPOSITE_TAG							CONSTLIT("Composite")
#define IMAGE_VARIANTS_TAG						CONSTLIT("ImageVariants")
#define SHIPS_TAG								CONSTLIT("Ships")

#define MIN_SHIPS_ATTRIB						CONSTLIT("minShips")
#define UNID_ATTRIB								CONSTLIT("UNID")
#define HIT_POINTS_ATTRIB						CONSTLIT("hitPoints")
#define MAX_HIT_POINTS_ATTRIB					CONSTLIT("maxHitPoints")
#define REPAIR_RATE_ATTRIB						CONSTLIT("repairRate")
#define MOBILE_ATTRIB							CONSTLIT("mobile")
#define MASS_ATTRIB								CONSTLIT("mass")
#define SHIPWRECK_UNID_ATTRIB					CONSTLIT("shipwreckID")
#define WALL_ATTRIB								CONSTLIT("barrier")
#define BARRIER_EFFECT_ATTRIB					CONSTLIT("barrierEffect")
#define LEVEL_ATTRIB							CONSTLIT("level")
#define LEVEL_FREQUENCY_ATTRIB					CONSTLIT("levelFrequency")
#define ATTRIBUTES_ATTRIB						CONSTLIT("attributes")
#define LOCATION_CRITERIA_ATTRIB				CONSTLIT("locationCriteria")
#define NAME_ATTRIB								CONSTLIT("name")
#define DOCK_SCREEN_ATTRIB						CONSTLIT("dockScreen")
#define ABANDONED_SCREEN_ATTRIB					CONSTLIT("abandonedScreen")
#define FREQUENCY_ATTRIB						CONSTLIT("frequency")
#define INACTIVE_ATTRIB							CONSTLIT("inactive")
#define DESTROY_WHEN_EMPTY_ATTRIB				CONSTLIT("destroyWhenEmpty")
#define SHIP_REPAIR_RATE_ATTRIB					CONSTLIT("shipRepairRate")
#define SOVEREIGN_ATTRIB						CONSTLIT("sovereign")
#define ALLOW_ENEMY_DOCKING_ATTRIB				CONSTLIT("allowEnemyDocking")
#define SPACE_COLOR_ATTRIB						CONSTLIT("spaceColor")
#define ALERT_WHEN_ATTACKED_ATTRIB				CONSTLIT("alertWhenAttacked")
#define ALERT_WHEN_DESTROYED_ATTRIB				CONSTLIT("alertWhenDestroyed")
#define NO_FRIENDLY_FIRE_ATTRIB					CONSTLIT("noFriendlyFire")
#define CONTROLLING_SOVEREIGN_ATTRIB			CONSTLIT("controllingSovereign")
#define SIGN_ATTRIB								CONSTLIT("sign")
#define BEACON_ATTRIB							CONSTLIT("beacon")
#define FIRE_RATE_ADJ_ATTRIB					CONSTLIT("fireRateAdj")
#define RADIOACTIVE_ATTRIB						CONSTLIT("radioactive")
#define UNIQUE_ATTRIB							CONSTLIT("unique")
#define CAN_ATTACK_ATTRIB						CONSTLIT("canAttack")
#define CONSTRUCTION_RATE_ATTRIB				CONSTLIT("constructionRate")
#define MAX_CONSTRUCTION_ATTRIB					CONSTLIT("maxConstruction")
#define GATE_EFFECT_ATTRIB						CONSTLIT("gateEffect")
#define DEFINITE_ARTICLE_ATTRIB					CONSTLIT("definiteArticle")
#define NO_ARTICLE_ATTRIB						CONSTLIT("noArticle")
#define SHIP_ENCOUNTER_ATTRIB					CONSTLIT("shipEncounter")
#define SCALE_ATTRIB							CONSTLIT("scale")
#define IMMUTABLE_ATTRIB						CONSTLIT("immutable")
#define EJECTA_TYPE_ATTRIB						CONSTLIT("ejectaType")
#define EJECTA_ADJ_ATTRIB						CONSTLIT("ejactaAdj")
#define MAX_STRUCTURAL_HIT_POINTS_ATTRIB		CONSTLIT("maxStructuralHitPoints")
#define NO_MAP_ICON_ATTRIB						CONSTLIT("noMapIcon")
#define MULTI_HULL_ATTRIB						CONSTLIT("multiHull")
#define TIME_STOP_IMMUNE_ATTRIB					CONSTLIT("timeStopImmune")
#define NO_BLACKLIST_ATTRIB						CONSTLIT("noBlacklist")
#define X_ATTRIB								CONSTLIT("x")
#define Y_ATTRIB								CONSTLIT("y")
#define ARMOR_ID_ATTRIB							CONSTLIT("armorID")
#define REVERSE_ARTICLE_ATTRIB					CONSTLIT("reverseArticle")
#define DOCKING_PORTS_ATTRIB					CONSTLIT("dockingPorts")
#define DEFAULT_BACKGROUND_ID_ATTRIB			CONSTLIT("defaultBackgroundID")
#define DEST_NODE_ATTRIB						CONSTLIT("destNodeID")
#define DEST_ENTRY_POINT_ATTRIB					CONSTLIT("destEntryPoint")	
#define EXPLOSION_TYPE_ATTRIB					CONSTLIT("explosionType")
#define BACKGROUND_OBJECT_ATTRIB				CONSTLIT("backgroundObject")
#define NO_FRIENDLY_TARGET_ATTRIB				CONSTLIT("noFriendlyTarget")

#define LARGE_DAMAGE_IMAGE_ID_ATTRIB			CONSTLIT("largeDamageImageID")
#define LARGE_DAMAGE_WIDTH_ATTRIB				CONSTLIT("largeDamageWidth")
#define LARGE_DAMAGE_HEIGHT_ATTRIB				CONSTLIT("largeDamageHeight")
#define LARGE_DAMAGE_COUNT_ATTRIB				CONSTLIT("largeDamageCount")
#define MEDIUM_DAMAGE_IMAGE_ID_ATTRIB			CONSTLIT("mediumDamageImageID")
#define MEDIUM_DAMAGE_WIDTH_ATTRIB				CONSTLIT("mediumDamageWidth")
#define MEDIUM_DAMAGE_HEIGHT_ATTRIB				CONSTLIT("mediumDamageHeight")
#define MEDIUM_DAMAGE_COUNT_ATTRIB				CONSTLIT("mediumDamageCount")

#define STAR_SCALE								CONSTLIT("star")
#define WORLD_SCALE								CONSTLIT("world")
#define STRUCTURE_SCALE							CONSTLIT("structure")
#define SHIP_SCALE								CONSTLIT("ship")
#define FLOTSAM_SCALE							CONSTLIT("flotsam")

#define UNIQUE_IN_SYSTEM						CONSTLIT("inSystem")
#define UNIQUE_IN_UNIVERSE						CONSTLIT("inUniverse")

#define FIELD_LEVEL								CONSTLIT("level")
#define FIELD_CATEGORY							CONSTLIT("category")
#define FIELD_NAME								CONSTLIT("name")
#define FIELD_ARMOR_CLASS						CONSTLIT("armorClass")
#define FIELD_HP								CONSTLIT("hp")
#define FIELD_FIRE_RATE_ADJ						CONSTLIT("fireRateAdj")
#define FIELD_CAN_ATTACK						CONSTLIT("canAttack")
#define FIELD_EXPLOSION_TYPE					CONSTLIT("explosionType")

#define VALUE_FALSE								CONSTLIT("false")
#define VALUE_TRUE								CONSTLIT("true")

#define MAX_ATTACK_DISTANCE						(g_KlicksPerPixel * 512)

struct DamageImageStruct
	{
	CG16bitImage *pImage;
	int cxWidth;
	int cyHeight;
	int iCount;
	};

static DamageImageStruct g_MediumDamage;
static DamageImageStruct g_LargeDamage;

void ApplyDamage (CG16bitImage &Dest, const RECT &rcDest, int iCount, DamageImageStruct &Damage);

CStationType::CStationType (void) : 
		m_pDesc(NULL),
		m_pInitialShips(NULL),
		m_pReinforcements(NULL),
		m_pEncounters(NULL),
		m_iEncounterFrequency(ftNotRandom),
		m_pSatellitesDesc(NULL),
		m_iMinShips(0),
		m_pConstruction(NULL),
		m_iShipConstructionRate(0),
		m_iMaxConstruction(0),
		m_Devices(NULL),
		m_iAnimationsCount(0),
		m_pAnimations(NULL),
		m_pItems(NULL),
		m_pTrade(NULL),
		m_WreckImage(NULL),
		m_WreckImageCache(NULL)

//	CStationType constructor

	{
	}

CStationType::~CStationType (void)

//	CStationType destructor

	{
	if (m_Devices)
		delete [] m_Devices;

	if (m_pDesc)
		delete m_pDesc;

	if (m_pLocalDockScreens)
		delete m_pLocalDockScreens;

	if (m_pItems)
		delete m_pItems;

	if (m_pInitialShips)
		delete m_pInitialShips;

	if (m_pReinforcements)
		delete m_pReinforcements;

	if (m_pEncounters)
		delete m_pEncounters;

	if (m_pSatellitesDesc)
		delete m_pSatellitesDesc;

	if (m_pAnimations)
		delete [] m_pAnimations;

	if (m_pTrade)
		delete m_pTrade;
	}

bool CStationType::CanBeEncountered (CSystem *pSystem)

//	CanBeEncountered
//
//	Returns TRUE if the station type can be encountered in the given system.

	{
	if (pSystem)
		{
		if (IsUniqueInSystem() && pSystem->IsStationInSystem(this))
			return false;
		}

	return (!IsUnique() || !m_fEncountered);
	}

CString CStationType::ComposeLoadError (const CString &sError)

//	ComposeLoadError
//
//	Compose an error loading XML

	{
	return strPatternSubst(CONSTLIT("%s: %s"),
			m_sName.GetASCIIZPointer(),
			sError.GetASCIIZPointer());
	}

void CStationType::CreateWreckImage (void)

//	CreateWreckImage
//
//	Create a wreck image of the station

	{
	int i;

	ASSERT(!m_Image.IsEmpty());

	//	Allocate images

	m_WreckImageCache = new CG16bitImage [m_Image.GetVariantCount()];
	m_WreckImage = new CObjectImageArray [m_Image.GetVariantCount()];

	//	Create the bitmaps

	for (i = 0; i < m_Image.GetVariantCount(); i++)
		{
		CObjectImageArray &Source = m_Image.GetImage(i);
		RECT rcRect = Source.GetImageRect();
		int cxWidth = RectWidth(rcRect);
		int cyHeight = RectHeight(rcRect);

		m_WreckImageCache[i].CreateBlank(cxWidth, cyHeight, true);

		rcRect.left = 0;
		rcRect.top = 0;
		rcRect.right = cxWidth;
		rcRect.bottom = rcRect.top + cyHeight;

		//	Start with undamaged image

		Source.CopyImage(m_WreckImageCache[i],
				0,
				0,
				0,
				0);

		//	Add some large damage

		int iCount = (cxWidth / 32) * (cyHeight / 32);
		ApplyDamage(m_WreckImageCache[i], rcRect, iCount, g_LargeDamage);

		//	Add some medium damage

		iCount = (cxWidth / 4) + (cyHeight / 4);
		ApplyDamage(m_WreckImageCache[i], rcRect, iCount, g_MediumDamage);

		//	Done

		m_WreckImageCache[i].SetTransparentColor(0x0000);

		//	Create the object image array

		m_WreckImage[i].Init(&m_WreckImageCache[i],
				rcRect,
				0,
				0);
		}
	}

bool CStationType::FindDataField (const CString &sField, CString *retsValue)

//	FindDataField
//
//	Returns meta-data

	{
	if (strEquals(sField, FIELD_CATEGORY))
		{
		if (m_sLevelFrequency.IsBlank())
			*retsValue = CONSTLIT("04-Not Random");
		else if (HasAttribute(CONSTLIT("debris")))
			*retsValue = CONSTLIT("03-Debris");
		else if (HasAttribute(CONSTLIT("enemy")))
			*retsValue = CONSTLIT("02-Enemy");
		else if (HasAttribute(CONSTLIT("friendly")))
			*retsValue = CONSTLIT("01-Friendly");
		else
			*retsValue = CONSTLIT("04-Not Random");
		}
	else if (strEquals(sField, FIELD_LEVEL))
		*retsValue = strFromInt(GetLevel());
	else if (strEquals(sField, FIELD_NAME))
		*retsValue = GetName();
	else if (strEquals(sField, FIELD_ARMOR_CLASS))
		{
		if (m_pArmor)
			*retsValue = m_pArmor->GetArmorClass()->GetShortName();
		else
			*retsValue = CONSTLIT("none");
		}
	else if (strEquals(sField, FIELD_HP))
		*retsValue = strFromInt(m_iHitPoints);
	else if (strEquals(sField, FIELD_FIRE_RATE_ADJ))
		*retsValue = strFromInt(10000 / m_iFireRateAdj);
	else if (strEquals(sField, FIELD_CAN_ATTACK))
		*retsValue = (CanAttack() ? VALUE_TRUE : VALUE_FALSE);
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
	else
		return false;

	return true;
	}

CSovereign *CStationType::GetControllingSovereign (void)

//	GetControllingSovereign
//
//	Returns the sovereign that controls the station

	{
	if (m_pControllingSovereign)
		return m_pControllingSovereign;
	else
		return m_pSovereign;
	}

int CStationType::GetFrequencyByLevel (int iLevel)

//	GetFrequencyByLevel
//
//	Returns the chance of this station type appearing at this level

	{
	if (IsUnique() && m_fEncountered)
		return 0;
	else
		return ::GetFrequencyByLevel(m_sLevelFrequency, iLevel);
	}

int CStationType::GetFrequencyForSystem (CSystem *pSystem)

//	GetFrequencyForSystem
//
//	Returns the chance that this station type will appear in the given system

	{
	//	If we're unique in the universe and already encountered, then no chance
	//	of appearing again.

	if (IsUnique() && m_fEncountered)
		return 0;

	//	If this station is unique in the system, see if there are other
	//	stations of this type in the system

	if (IsUniqueInSystem() && pSystem->IsStationInSystem(this))
		return 0;

	//	Otherwise, go by level

	return ::GetFrequencyByLevel(m_sLevelFrequency, pSystem->GetLevel());
	}

int CStationType::GetLevel (void)

//	GetLevel
//
//	Returns the average level of the station.

	{
	if (m_iLevel)
		return m_iLevel;
	else
		{
		int iLevel = 1;
		int iTotal = 0;
		int iCount = 0;
		char *pPos = m_sLevelFrequency.GetASCIIZPointer();
		while (*pPos != '\0')
			{
			int iFreq = 0;

			switch (*pPos)
				{
				case 'C':
				case 'c':
					iFreq = ftCommon;
					break;

				case 'U':
				case 'u':
					iFreq = ftUncommon;
					break;

				case 'R':
				case 'r':
					iFreq = ftRare;
					break;

				case 'V':
				case 'v':
					iFreq = ftVeryRare;
					break;
				}

			iTotal += iFreq * iLevel;
			iCount += iFreq;

			pPos++;
			if (*pPos != ' ')
				iLevel++;
			}

		if (iCount > 0)
			return (int)(((double)iTotal / (double)iCount) + 0.5);
		else
			return 0;
		}
	}

CXMLElement *CStationType::GetScreen (const CString &sName)

//	GetScreen
//
//	Returns a screen description element for the given screen by name.
//	If name is a number then we assume that it is the UNID of a shared
//	screen. Otherwise it is the name of a local screen

	{
	CDockScreenTypeRef Screen;
	Screen.LoadUNID(sName);
	Screen.Bind(m_pLocalDockScreens);
	return Screen.GetDesc();
	}

ALERROR CStationType::InitFromXML (CXMLElement *pDesc)

//	InitFromXML
//
//	Initializes global variables

	{
	g_MediumDamage.pImage = g_pUniverse->GetLibraryBitmap(pDesc->GetAttributeInteger(MEDIUM_DAMAGE_IMAGE_ID_ATTRIB));
	if (g_MediumDamage.pImage == NULL)
		return ERR_FAIL;

	g_MediumDamage.cxWidth = pDesc->GetAttributeInteger(MEDIUM_DAMAGE_WIDTH_ATTRIB);
	g_MediumDamage.cyHeight = pDesc->GetAttributeInteger(MEDIUM_DAMAGE_HEIGHT_ATTRIB);
	g_MediumDamage.iCount = pDesc->GetAttributeInteger(MEDIUM_DAMAGE_COUNT_ATTRIB);

	g_LargeDamage.pImage = g_pUniverse->GetLibraryBitmap(pDesc->GetAttributeInteger(LARGE_DAMAGE_IMAGE_ID_ATTRIB));
	if (g_LargeDamage.pImage == NULL)
		return ERR_FAIL;

	g_LargeDamage.cxWidth = pDesc->GetAttributeInteger(LARGE_DAMAGE_WIDTH_ATTRIB);
	g_LargeDamage.cyHeight = pDesc->GetAttributeInteger(LARGE_DAMAGE_HEIGHT_ATTRIB);
	g_LargeDamage.iCount = pDesc->GetAttributeInteger(LARGE_DAMAGE_COUNT_ATTRIB);

	return NOERROR;
	}

void CStationType::LoadImages (int iVariant)

//	LoadImages
//
//	Loads images used by the station

	{
	m_Image.LoadImage(iVariant);

	if (m_pExplosionType)
		m_pExplosionType->LoadImages();

	if (m_pEjectaType)
		m_pEjectaType->LoadImages();
	}

void CStationType::MarkImages (int iVariant)

//	MarkImages
//
//	Marks images used by the station

	{
	m_Image.MarkImage(iVariant);

	if (m_pExplosionType)
		m_pExplosionType->MarkImages();

	if (m_pEjectaType)
		m_pEjectaType->MarkImages();
	}

ALERROR CStationType::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind design

	{
	int i;
	ALERROR error;
	bool bHasDockScreens;
	Metric rBestRange;

	//	Images

	if (error = m_Image.OnDesignLoadComplete(Ctx))
		goto Fail;

	for (i = 0; i < m_iAnimationsCount; i++)
		if (error = m_pAnimations[i].m_Image.OnDesignLoadComplete(Ctx))
			goto Fail;

	//	Sovereigns

	if (error = m_pSovereign.Bind(Ctx))
		goto Fail;

	if (error = m_pControllingSovereign.Bind(Ctx))
		goto Fail;

	//	Armor

	if (error = m_pArmor.Bind(Ctx, itemcatArmor))
		goto Fail;

	//	If this station has hit points then it also needs a
	//	wreck image.

	if (m_iMaxHitPoints > 0 && !g_pUniverse->NoImages())
		CreateWreckImage();

	//	Resolve screen

	if (error = m_pAbandonedDockScreen.Bind(Ctx, m_pLocalDockScreens))
		goto Fail;

	if (error = m_pFirstDockScreen.Bind(Ctx, m_pLocalDockScreens))
		goto Fail;

	//	Resolve the devices pointer

	rBestRange = MAX_ATTACK_DISTANCE;
	for (i = 0; i < m_iDevicesCount; i++)
		{
		if (error = m_Devices[i].OnDesignLoadComplete(Ctx))
			goto Fail;

		if (m_Devices[i].GetCategory() == itemcatWeapon
				|| m_Devices[i].GetCategory() == itemcatLauncher)
			{
			Metric rRange = m_Devices[i].GetMaxEffectiveRange(NULL);
			if (rRange > rBestRange)
				rBestRange = rRange;
			}
		}
	m_rMaxAttackDistance = rBestRange;

	//	Items

	if (m_pItems)
		if (error = m_pItems->OnDesignLoadComplete(Ctx))
			goto Fail;

	if (m_pTrade)
		if (error = m_pTrade->OnDesignLoadComplete(Ctx))
			goto Fail;

	//	Ships

	if (m_pInitialShips)
		if (error = m_pInitialShips->OnDesignLoadComplete(Ctx))
			goto Fail;

	if (m_pReinforcements)
		if (error = m_pReinforcements->OnDesignLoadComplete(Ctx))
			goto Fail;

	if (m_pConstruction)
		if (error = m_pConstruction->OnDesignLoadComplete(Ctx))
			goto Fail;

	if (m_pEncounters)
		if (error = m_pEncounters->OnDesignLoadComplete(Ctx))
			goto Fail;

	//	Resolve the explosion pointer

	if (error = m_pExplosionType.Bind(Ctx))
		goto Fail;

	//	Resolve ejecta pointer

	if (error = m_pEjectaType.Bind(Ctx))
		goto Fail;

	//	Resolve effects

	if (error = m_pBarrierEffect.Bind(Ctx))
		goto Fail;

	if (error = m_pGateEffect.Bind(Ctx))
		goto Fail;

	//	If this station has a dock screen, then it must have docking ports

	bHasDockScreens = (GetAbandonedScreen() != NULL) || (GetFirstDockScreen() != NULL);
	if (bHasDockScreens)
		{
		CXMLElement *pDockingPorts = NULL;
		bool bHasDockingPorts = (m_pDesc->GetAttributeInteger(DOCKING_PORTS_ATTRIB) > 0)
				|| ((pDockingPorts = m_pDesc->GetContentElementByTag(DOCKING_PORTS_TAG))
					&& pDockingPorts->GetContentElementCount() > 0);

		if (!bHasDockingPorts)
			{
			Ctx.sError = ComposeLoadError(CONSTLIT("Station needs docking ports."));
			return ERR_FAIL;
			}
		}

	//	Figure out if this is static

	m_fStatic = (m_iMaxHitPoints == 0)
			&& (m_iStructuralHP == 0)
			&& (m_iDevicesCount == 0)
			&& (!bHasDockScreens)
			&& (m_pInitialShips == NULL)
			&& (m_pReinforcements == NULL)
			&& (m_pEncounters == NULL)
			&& (m_pConstruction == NULL)
			&& (m_pItems == NULL)
			&& (!HasEvents())
			&& (m_pBarrierEffect == NULL)
			&& (!m_fMobile)
			&& (!m_fWall)
			&& (!m_fSign)
			&& (!m_fBeacon)
			&& (!m_fShipEncounter);

	return NOERROR;

Fail:

	Ctx.sError = ComposeLoadError(Ctx.sError);
	return ERR_FAIL;
	}

ALERROR CStationType::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Create from XML

	{
	int i;
	ALERROR error;

	//	Initialize basic info

#ifdef DEBUG_SOURCE_LOAD_TRACE
	kernelDebugLogMessage("Loading station %x...", GetUNID());
#endif
	m_sName = pDesc->GetAttribute(NAME_ATTRIB);
	m_iLevel = pDesc->GetAttributeInteger(LEVEL_ATTRIB);
	m_pSovereign.LoadUNID(Ctx, pDesc->GetAttribute(SOVEREIGN_ATTRIB));
	m_pControllingSovereign.LoadUNID(Ctx, pDesc->GetAttribute(CONTROLLING_SOVEREIGN_ATTRIB));
	m_iRepairRate = pDesc->GetAttributeInteger(REPAIR_RATE_ATTRIB);
#ifdef BACKGROUND_OBJECTS
	m_fBackground = pDesc->GetAttributeBool(BACKGROUND_OBJECT_ATTRIB);
#else
	m_fBackground = false;
#endif
	m_fMobile = pDesc->GetAttributeBool(MOBILE_ATTRIB);
	m_fWall = pDesc->GetAttributeBool(WALL_ATTRIB);
	m_fNoFriendlyFire = pDesc->GetAttributeBool(NO_FRIENDLY_FIRE_ATTRIB);
	m_fNoFriendlyTarget = pDesc->GetAttributeBool(NO_FRIENDLY_TARGET_ATTRIB);
	m_fInactive = pDesc->GetAttributeBool(INACTIVE_ATTRIB);
	m_fDestroyWhenEmpty = pDesc->GetAttributeBool(DESTROY_WHEN_EMPTY_ATTRIB);
	m_fAllowEnemyDocking = pDesc->GetAttributeBool(ALLOW_ENEMY_DOCKING_ATTRIB);
	m_fSign = pDesc->GetAttributeBool(SIGN_ATTRIB);
	m_fBeacon = pDesc->GetAttributeBool(BEACON_ATTRIB);
	m_fRadioactive = pDesc->GetAttributeBool(RADIOACTIVE_ATTRIB);
	m_fEncountered = false;
	m_fNoMapIcon = pDesc->GetAttributeBool(NO_MAP_ICON_ATTRIB);
	m_fMultiHull = pDesc->GetAttributeBool(MULTI_HULL_ATTRIB);
	m_fTimeStopImmune = pDesc->GetAttributeBool(TIME_STOP_IMMUNE_ATTRIB);
	m_fCanAttack = pDesc->GetAttributeBool(CAN_ATTACK_ATTRIB);
	m_fDefiniteArticle = pDesc->GetAttributeBool(DEFINITE_ARTICLE_ATTRIB);
	m_fNoArticle = pDesc->GetAttributeBool(NO_ARTICLE_ATTRIB);
	m_fReverseArticle = pDesc->GetAttributeBool(REVERSE_ARTICLE_ATTRIB);
	m_fShipEncounter = pDesc->GetAttributeBool(SHIP_ENCOUNTER_ATTRIB);
	m_fImmutable = pDesc->GetAttributeBool(IMMUTABLE_ATTRIB);
	m_fNoBlacklist = pDesc->GetAttributeBool(NO_BLACKLIST_ATTRIB);
	m_sLevelFrequency = pDesc->GetAttribute(LEVEL_FREQUENCY_ATTRIB);
	m_sAttributes = pDesc->GetAttribute(ATTRIBUTES_ATTRIB);
	m_sLocationCriteria = pDesc->GetAttribute(LOCATION_CRITERIA_ATTRIB);
	m_iShipRepairRate = pDesc->GetAttributeInteger(SHIP_REPAIR_RATE_ATTRIB);
	m_rgbSpaceColor = LoadCOLORREF(pDesc->GetAttribute(SPACE_COLOR_ATTRIB));
	m_iAlertWhenAttacked = pDesc->GetAttributeInteger(ALERT_WHEN_ATTACKED_ATTRIB);
	m_iAlertWhenDestroyed = pDesc->GetAttributeInteger(ALERT_WHEN_DESTROYED_ATTRIB);
	m_iFireRateAdj = strToInt(pDesc->GetAttribute(FIRE_RATE_ADJ_ATTRIB), 80);
	m_rMaxAttackDistance = MAX_ATTACK_DISTANCE;
	if (m_fImmutable)
		m_iStructuralHP = 0;
	else
		m_iStructuralHP = pDesc->GetAttributeInteger(MAX_STRUCTURAL_HIT_POINTS_ATTRIB);

	//	Get unique attributes

	CString sUnique = pDesc->GetAttribute(UNIQUE_ATTRIB);
	m_fUnique = false;
	m_fUniqueInSystem = false;
	if (strEquals(sUnique, UNIQUE_IN_SYSTEM))
		m_fUniqueInSystem = true;
	else if (strEquals(sUnique, UNIQUE_IN_UNIVERSE))
		m_fUnique = true;
	else if (strEquals(sUnique, VALUE_TRUE))
		m_fUnique = true;

	//	Get hit points and max hit points

	m_pArmor.LoadUNID(Ctx, pDesc->GetAttribute(ARMOR_ID_ATTRIB));
	m_iMaxHitPoints = pDesc->GetAttributeInteger(MAX_HIT_POINTS_ATTRIB);
	m_iHitPoints = strToInt(pDesc->GetAttribute(HIT_POINTS_ATTRIB), m_iMaxHitPoints);

	//	Get the scale

	CString sScale = pDesc->GetAttribute(SCALE_ATTRIB);
	if (strEquals(sScale, STAR_SCALE))
		m_iScale = scaleStar;
	else if (strEquals(sScale, WORLD_SCALE))
		m_iScale = scaleWorld;
	else if (strEquals(sScale, SHIP_SCALE))
		m_iScale = scaleShip;
	else if (strEquals(sScale, FLOTSAM_SCALE))
		m_iScale = scaleFlotsam;
	else
		m_iScale = scaleStructure;

	//	Mass
	
	int iMass = pDesc->GetAttributeInteger(MASS_ATTRIB);
	if (iMass != 0)
		m_rMass = iMass;
	else
		m_rMass = 1000000;

	//	Load devices

	CXMLElement *pDevices = pDesc->GetContentElementByTag(DEVICES_TAG);
	if (pDevices)
		{
		m_iDevicesCount = pDevices->GetContentElementCount();
		if (m_iDevicesCount)
			{
			m_Devices = new CInstalledDevice [m_iDevicesCount];

			for (i = 0; i < m_iDevicesCount; i++)
				{
				CXMLElement *pDeviceDesc = pDevices->GetContentElement(i);
				m_Devices[i].InitFromXML(Ctx, pDeviceDesc);
				}
			}

#ifdef DEBUG_SOURCE_LOAD_TRACE
		kernelDebugLogMessage("...loaded devices");
#endif
		}
	else
		m_iDevicesCount = 0;

	//	Make a copy of the descriptor (We should stop doing this when we
	//	move all the data in the descriptor to private member variables.
	//	The only problem will be fixing the m_pReinforcements and m_pEncounters
	//	field which assumes that we have a copy of this).
	m_pDesc = pDesc->OrphanCopy();
	if (m_pDesc == NULL)
		{
		Ctx.sError = CONSTLIT("Out of memory");
		return ERR_MEMORY;
		}

	//	Load items

	CXMLElement *pItems = pDesc->GetContentElementByTag(ITEMS_TAG);
	if (pItems)
		{
		if (error = IItemGenerator::CreateFromXML(Ctx, pItems, &m_pItems))
			return error;

#ifdef DEBUG_SOURCE_LOAD_TRACE
		kernelDebugLogMessage("...loaded items table");
#endif
		}

	//	Load trade

	CXMLElement *pTrade = pDesc->GetContentElementByTag(TRADE_TAG);
	if (pTrade)
		{
		if (error = CTradingDesc::CreateFromXML(Ctx, pTrade, &m_pTrade))
			return error;
		}

	//	Find an element describing the image

	CXMLElement *pImage = pDesc->GetContentElementByTag(IMAGE_TAG);
	if (pImage == NULL)
		pImage = pDesc->GetContentElementByTag(IMAGE_VARIANTS_TAG);
	if (pImage == NULL)
		pImage = pDesc->GetContentElementByTag(COMPOSITE_TAG);

	//	Load the image

	if (pImage)
		{
		//	If this image comes from a shipwreck then load it from
		//	a ship class object. Otherwise, initialize the image from the XML

		if (pImage->AttributeExists(SHIPWRECK_UNID_ATTRIB))
			{
			if (error = pImage->GetAttributeIntegerList(SHIPWRECK_UNID_ATTRIB, &m_ShipWrecks))
				{
				Ctx.sError = ComposeLoadError(CONSTLIT("Unable to load ship wreck list"));
				return error;
				}

			if (m_ShipWrecks.GetCount() == 0)
				{
				Ctx.sError = ComposeLoadError(CONSTLIT("Expected ship wreck list"));
				return ERR_FAIL;
				}

			m_iImageVariants = 0;
			}

		//	Otherwise, load the image

		else
			{
			if (error = m_Image.InitFromXML(Ctx, pImage))
				{
				Ctx.sError = ComposeLoadError(Ctx.sError);
				return error;
				}

			m_iImageVariants = m_Image.GetVariantCount();
			}

#ifdef DEBUG_SOURCE_LOAD_TRACE
		kernelDebugLogMessage("...loaded image");
#endif
		}

	//	Load animations

	CXMLElement *pAnimations = pDesc->GetContentElementByTag(ANIMATIONS_TAG);
	if (pAnimations)
		{
		m_iAnimationsCount = pAnimations->GetContentElementCount();
		m_pAnimations = new SAnimationSection [m_iAnimationsCount];

		for (i = 0; i < m_iAnimationsCount; i++)
			{
			CXMLElement *pSection = pAnimations->GetContentElement(i);
			m_pAnimations[i].m_x = pSection->GetAttributeInteger(X_ATTRIB);
			m_pAnimations[i].m_y = pSection->GetAttributeInteger(Y_ATTRIB);

			if (pSection->GetContentElementCount() > 0)
				{
				CXMLElement *pImage = pSection->GetContentElement(0);
				if (error = m_pAnimations[i].m_Image.InitFromXML(Ctx, pImage))
					{
					Ctx.sError = ComposeLoadError(CONSTLIT("Unable to load animation image"));
					return error;
					}
				}
			}

#ifdef DEBUG_SOURCE_LOAD_TRACE
		kernelDebugLogMessage("...loaded animations");
#endif
		}

	//	Get the local dock screens (these are screens that are only used by
	//	this station type)

	CXMLElement *pLocals = pDesc->GetContentElementByTag(DOCK_SCREENS_TAG);
	if (pLocals)
		m_pLocalDockScreens = pLocals->OrphanCopy();
	else
		m_pLocalDockScreens = NULL;

	//	Now get the first dock screen (note that m_pLocalDockScreens must be set up
	//	correctly first)

	m_pFirstDockScreen.LoadUNID(Ctx, pDesc->GetAttribute(DOCK_SCREEN_ATTRIB));
	m_pAbandonedDockScreen.LoadUNID(Ctx, pDesc->GetAttribute(ABANDONED_SCREEN_ATTRIB));

	//	Background screens

	m_dwDefaultBkgnd = LoadUNID(Ctx, pDesc->GetAttribute(DEFAULT_BACKGROUND_ID_ATTRIB));
	
	//	Load initial ships

	CXMLElement *pShips = pDesc->GetContentElementByTag(SHIPS_TAG);
	if (pShips)
		{
		if (error = IShipGenerator::CreateFromXMLAsGroup(Ctx, pShips, &m_pInitialShips))
			{
			Ctx.sError = ComposeLoadError(strPatternSubst(CONSTLIT("<Ships>: %s"), Ctx.sError.GetASCIIZPointer()));
			return error;
			}
		}

	//	Load reinforcements

	CXMLElement *pReinforcements = m_pDesc->GetContentElementByTag(REINFORCEMENTS_TAG);
	if (pReinforcements)
		{
		if (error = IShipGenerator::CreateFromXMLAsGroup(Ctx, pReinforcements, &m_pReinforcements))
			{
			Ctx.sError = ComposeLoadError(strPatternSubst(CONSTLIT("<Reinforcements>: %s"), Ctx.sError.GetASCIIZPointer()));
			return error;
			}

		//	Figure out the minimum number of reinforcements at this base

		m_iMinShips = pReinforcements->GetAttributeInteger(MIN_SHIPS_ATTRIB);

#ifdef DEBUG_SOURCE_LOAD_TRACE
		kernelDebugLogMessage("...loaded reinforcements");
#endif
		}

	//	Load encounter table

	CXMLElement *pEncounters = m_pDesc->GetContentElementByTag(ENCOUNTERS_TAG);
	if (pEncounters)
		{
		if (error = IShipGenerator::CreateFromXMLAsGroup(Ctx, pEncounters, &m_pEncounters))
			{
			Ctx.sError = ComposeLoadError(strPatternSubst(CONSTLIT("<Encounters>: %s"), Ctx.sError.GetASCIIZPointer()));
			return error;
			}

		m_iEncounterFrequency = GetFrequency(pEncounters->GetAttribute(FREQUENCY_ATTRIB));

#ifdef DEBUG_SOURCE_LOAD_TRACE
		kernelDebugLogMessage("...loaded encounter table");
#endif
		}

	//	Load construction table

	CXMLElement *pConstruction = m_pDesc->GetContentElementByTag(CONSTRUCTION_TAG);
	if (pConstruction)
		{
		if (error = IShipGenerator::CreateFromXMLAsGroup(Ctx, pConstruction, &m_pConstruction))
			{
			Ctx.sError = ComposeLoadError(strPatternSubst(CONSTLIT("<Construction>: %s"), Ctx.sError.GetASCIIZPointer()));
			return error;
			}

		m_iShipConstructionRate = pConstruction->GetAttributeInteger(CONSTRUCTION_RATE_ATTRIB);
		m_iMaxConstruction = pConstruction->GetAttributeInteger(MAX_CONSTRUCTION_ATTRIB);

#ifdef DEBUG_SOURCE_LOAD_TRACE
		kernelDebugLogMessage("...loaded construction table");
#endif
		}

	//	Load satellites

	CXMLElement *pSatellitesDesc = pDesc->GetContentElementByTag(SATELLITES_TAG);
	if (pSatellitesDesc)
		m_pSatellitesDesc = pSatellitesDesc->OrphanCopy();

	//	Explosion

	m_pExplosionType.LoadUNID(Ctx, pDesc->GetAttribute(EXPLOSION_TYPE_ATTRIB));

	//	Ejecta

	m_pEjectaType.LoadUNID(Ctx, pDesc->GetAttribute(EJECTA_TYPE_ATTRIB));
	if (m_pEjectaType.GetUNID())
		{
		m_iEjectaAdj = pDesc->GetAttributeInteger(EJECTA_ADJ_ATTRIB);
		if (m_iEjectaAdj == 0)
			m_iEjectaAdj = 100;
		}
	else
		m_iEjectaAdj = 0;

	//	Miscellaneous

	m_pBarrierEffect.LoadUNID(Ctx, pDesc->GetAttribute(BARRIER_EFFECT_ATTRIB));

	m_sStargateDestNode = pDesc->GetAttribute(DEST_NODE_ATTRIB);
	m_sStargateDestEntryPoint = pDesc->GetAttribute(DEST_ENTRY_POINT_ATTRIB);
	m_pGateEffect.LoadUNID(Ctx, pDesc->GetAttribute(GATE_EFFECT_ATTRIB));

	m_fHasOnObjDockedEvent = FindEventHandler(CONSTLIT("OnObjDocked"), NULL);

#ifdef DEBUG_SOURCE_LOAD_TRACE
	kernelDebugLogMessage("Done loading station");
#endif
	//	Done

	return NOERROR;
	}

void CStationType::OnReadFromStream (SUniverseLoadCtx &Ctx)

//	ReadFromStream
//
//	Read from stream
//
//	DWORD		flags
//	CAttributeDataBlock	m_Data

	{
	DWORD dwLoad;
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	
	m_fEncountered =	((dwLoad & 0x00000001) ? true : false);

	//	Load opaque data

	ReadGlobalData(Ctx);
	}

void CStationType::OnReinit (void)

//	OnReinit
//
//	Reinitialize the type

	{
	m_fEncountered = false;
	}

void CStationType::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Writes out the variable portions of the item type
//
//	DWORD		flags

	{
	DWORD dwSave;

	dwSave = 0;
	dwSave |= (m_fEncountered ?	0x00000001 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

void CStationType::PaintAnimations (CG16bitImage &Dest, int x, int y, int iTick)

//	PaintAnimations
//
//	Paint animations

	{
	int i;

	for (i = 0; i < m_iAnimationsCount; i++)
		{
		m_pAnimations[i].m_Image.PaintImage(Dest,
				x + m_pAnimations[i].m_x,
				y - m_pAnimations[i].m_y,
				iTick,
				0);
		}
	}

void CStationType::SetCustomImage (CStation *pStation)

//	SetCustomImage
//
//	Sets the image for the station (if necessary). Also, sets the variant

	{
	if (m_ShipWrecks.GetCount())
		{
		DWORD dwShipwreckID = (DWORD)m_ShipWrecks.GetElement(mathRandom(0, m_ShipWrecks.GetCount()-1));
		CShipClass *pClass = g_pUniverse->FindShipClass(dwShipwreckID);
		if (pClass == NULL)
			return;

		pClass->SetShipwreckImage(pStation);
		pClass->SetShipwreckParams(pStation, NULL);
		}
	else
		{
		int iVariantCount = GetImageVariants();
		if (iVariantCount)
			pStation->SetImageVariant(mathRandom(0, iVariantCount-1));
		else
			pStation->SetImageVariant(0);
		}
	}

void ApplyDamage (CG16bitImage &Dest, const RECT &rcDest, int iCount, DamageImageStruct &Damage)

//	ApplyDamage
//
//	Applies damage to the image

	{
	for (int i = 0; i < iCount; i++)
		{
		Dest.ColorTransBlt(Damage.cxWidth * mathRandom(0, Damage.iCount-1),
				0,
				Damage.cxWidth,
				Damage.cyHeight,
				255,
				*Damage.pImage,
				rcDest.left + mathRandom(0, RectWidth(rcDest)-1) - (Damage.cxWidth / 2),
				rcDest.top + mathRandom(0, RectHeight(rcDest)-1) - (Damage.cyHeight / 2));
		}
	}

