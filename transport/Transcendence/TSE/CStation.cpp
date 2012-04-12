//	CStation.cpp
//
//	CStation class

#include "PreComp.h"

#ifdef DEBUG
//#define DEBUG_DOCKING
//#define DEBUG_ALERTS
#endif

#define ITEM_TAG						CONSTLIT("Item")
#define INITIAL_DATA_TAG				CONSTLIT("InitialData")
#define DEVICES_TAG						CONSTLIT("Devices")
#define NAMES_TAG						CONSTLIT("Names")
#define ITEMS_TAG						CONSTLIT("Items")

#define COUNT_ATTRIB					CONSTLIT("count")
#define ITEM_ATTRIB						CONSTLIT("item")
#define DEVICE_ID_ATTRIB				CONSTLIT("deviceID")
#define X_ATTRIB						CONSTLIT("x")
#define Y_ATTRIB						CONSTLIT("y")
#define NO_MAP_LABEL_ATTRIB				CONSTLIT("noMapLabel")
#define SHIPWRECK_UNID_ATTRIB			CONSTLIT("shipwreckID")
#define NAME_ATTRIB						CONSTLIT("name")

#define STR_TRUE						CONSTLIT("True")

const int TRADE_UPDATE_FREQUENCY =		1801;						//	Interval for checking trade
const int STATION_SCAN_TARGET_FREQUENCY	= 29;
const int STATION_ATTACK_FREQUENCY =	67;
const int STATION_REINFORCEMENT_FREQUENCY =	607;
const int STATION_REPAIR_FREQUENCY =	31;
const int STATION_TARGET_FREQUENCY =	503;

#define MAX_ATTACK_DISTANCE				(g_KlicksPerPixel * 512)
#define BEACON_RANGE					(LIGHT_SECOND * 20)
#define MAX_SUBORDINATES				12
#define BLACKLIST_HIT_LIMIT				3
#define BLACKLIST_DECAY_RATE			150

#define MIN_ANGER						300
#define MAX_ANGER						1800
#define ANGER_INC						30

const COLORREF RGB_SIGN_COLOR =				CGImage::RGBColor(196, 223, 155);
const COLORREF RGB_ORBIT_LINE =				CGImage::RGBColor(115, 149, 229);
const COLORREF RGB_MAP_LABEL =				CGImage::RGBColor(255, 217, 128);

static DATADESCSTRUCT g_DataDesc[] =
	{	{ DATADESC_OPCODE_REFERENCE,	1,	0 },		//	m_pSystem
		{ DATADESC_OPCODE_INT,			2,	0 },		//	m_iIndex, m_iDestiny
		{ DATADESC_OPCODE_INT,			4,	0 },		//	m_vPos
		{ DATADESC_OPCODE_INT,			4,	0 },		//	m_vVel
		{ DATADESC_OPCODE_INT,			1,	0 },		//	m_dwFlags
		{ DATADESC_OPCODE_REFERENCE,	1,	0 },		//	m_pSovereign
		{ DATADESC_OPCODE_EMBED_OBJ,	1,	0 },		//	m_Image
		{ DATADESC_OPCODE_STOP,	0,	0 } };
static CObjectClass<CStation>g_Class(OBJID_CSTATION, g_DataDesc);

static char g_ImageTag[] = "Image";
static char g_ShipsTag[] = "Ships";
static char g_DockScreensTag[] = "DockScreens";
static char g_ShipTag[] = "Ship";

static char g_DockScreenAttrib[] = "dockScreen";
static char g_AbandonedScreenAttrib[] = "abandonedScreen";
static char g_HitPointsAttrib[] = "hitPoints";
static char g_ArmorIDAttrib[] = "armorID";
static char g_ProbabilityAttrib[] = "probability";
static char g_TableAttrib[] = "table";

#define MIN_DOCK_APPROACH_SPEED			(g_KlicksPerPixel * 25.0 / g_TimeScale);
#define MAX_DOCK_APPROACH_SPEED			(g_KlicksPerPixel * 50.0 / g_TimeScale);
#define MAX_DOCK_TANGENT_SPEED			(g_KlicksPerPixel / g_TimeScale);
const Metric g_DockBeamStrength =		1000.0;
const Metric g_DockBeamTangentStrength = 250.0;

const int g_iMapScale = 5;

CStation::CStation (void) : CSpaceObject(&g_Class),
		m_fArmed(false),
		m_dwSpare(0),
		m_pType(NULL),
		m_pMapOrbit(NULL),
		m_pArmorClass(NULL),
		m_pTarget(NULL),
		m_pDevices(NULL),
		m_iAngryCounter(0),
		m_iReinforceRequestCount(0),
		m_iBalance(0)

//	CStation constructor

	{
	}

CStation::~CStation (void)

//	CStation destructor

	{
	if (m_pMapOrbit)
		delete m_pMapOrbit;

	if (m_pDevices)
		delete [] m_pDevices;
	}

void CStation::AddSubordinate (CSpaceObject *pSubordinate)

//	AddSubordinate
//
//	Add this object to our list of subordinates

	{
	m_Subordinates.Add(pSubordinate);
	}

void CStation::Blacklist (CSpaceObject *pObj)

//	Blacklist
//
//	pObj is blacklisted (this only works for the player)

	{
	if (pObj != g_pUniverse->GetPlayer())
		return;

	//	No need if we don't support blacklist

	if (!m_pType->IsBlacklistEnabled())
		return;

	//	Send all our subordinates to attack

	for (int i = 0; i < m_Subordinates.GetCount(); i++)
		Communicate(m_Subordinates.GetObj(i), msgAttack, pObj);

	//	Remember that player is blacklisted

	m_Blacklist.Blacklist();
	}

int CStation::CalcNumberOfShips (void)

//	CalcNumberOfShips
//
//	Returns the number of ships associated with this station

	{
	int i;
	int iCount = 0;

	for (i = 0; i < GetSystem()->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetSystem()->GetObject(i);

		if (pObj
				&& pObj->GetBase() == this
				&& pObj->GetCategory() == catShip
				&& pObj != this)
			iCount++;
		}

	return iCount;
	}

bool CStation::CanAttack (void) const

//	CanAttack
//
//	TRUE if the object can attack

	{
	return (!IsAbandoned() 
			&& (m_fArmed 
				|| (m_Subordinates.GetCount() > 0)
				|| m_pType->CanAttack()));
	}

bool CStation::CanBlock (CSpaceObject *pObj)

//	CanBlock
//
//	Returns TRUE if this object can block the given object

	{
	return (m_pType->IsWall() 
			|| (pObj->GetCategory() == catStation && pObj->IsMobile()));
	}

int CStation::ChargeMoney (int iValue)

//	ChargeMoney
//
//	Charge the amount out of the station's balance
//	Returns the remaining balance (or -1 if there is not enough)

	{
	if (iValue <= m_iBalance)
		{
		if (iValue > 0)
			m_iBalance -= iValue;
		return m_iBalance;
		}
	else
		return -1;
	}

bool CStation::ClassCanAttack (void)

//	ClassCanAttack
//
//	Only returns FALSE if this object can never attack

	{
	return (m_pType->CanAttack());
	}

int CStation::CreditMoney (int iValue)

//	CreditMoney
//
//	Credits the amount to the station's balance. Return the new balance.

	{
	if (iValue > 0)
		m_iBalance += iValue;
	return m_iBalance;
	}

void CStation::CreateDestructionEffect (void)

//	CreateDestructionEffect
//
//	Create the effect when the station is destroyed

	{
	//	Start destruction animation

	m_iDestroyedAnimation = 60;

	//	Explosion effect and damage

	SExplosionType Explosion;
	FireGetExplosionType(&Explosion);
	if (Explosion.pDesc == NULL)
		Explosion.pDesc = m_pType->GetExplosionType();

	if (Explosion.pDesc)
		{
		GetSystem()->CreateWeaponFire(Explosion.pDesc,
				Explosion.iBonus,
				Explosion.iCause,
				CDamageSource(this),
				GetPos(),
				GetVel(),
				0,
				NULL,
				NULL);
		}

	//	Some air leaks

	CParticleEffect *pEffect;
	CParticleEffect::CreateEmpty(GetSystem(),
			GetPos(),
			GetVel(),
			&pEffect);

	int iAirLeaks = mathRandom(0, 5);
	for (int j = 0; j < iAirLeaks; j++)
		{
		CParticleEffect::SParticleType *pType = new CParticleEffect::SParticleType;
		pType->m_fWake = true;
		pType->m_fDamage = false;

		pType->m_fLifespan = true;
		pType->iLifespan = 30;

		pType->m_fRegenerate = true;
		pType->iRegenerationTimer = 300 + mathRandom(0, 200);

		pType->iDirection = mathRandom(0, 359);
		pType->iDirRange = 3;
		pType->rAveSpeed = 0.1 * LIGHT_SPEED;

		pType->m_fMaxRadius = false;
		pType->rRadius = pType->iLifespan * pType->rAveSpeed;
		pType->rDampening = 0.75;

		pType->iPaintStyle = CParticleEffect::paintSmoke;

		pEffect->AddGroup(pType, mathRandom(50, 150));
		}

	//	Sound effects

	g_pUniverse->PlaySound(this, g_pUniverse->FindSound(g_StationExplosionSoundUNID));
	}

void CStation::CreateEjectaFromDamage (int iDamage, const CVector &vHitPos, int iDirection, const DamageDesc &Damage)

//	CreateEjectaFromDamage
//
//	Create ejecta when hit by damage

	{
	int i;

	int iEjectaAdj;
	if ((iEjectaAdj = m_pType->GetEjectaAdj()))
		{
		//	Ignore if damage came from ejecta (so that we avoid chain reactions)

		if (Damage.GetCause() == killedByEjecta && mathRandom(1, 100) <= 90)
			return;

		//	Adjust for the station propensity to create ejecta.

		iDamage = iEjectaAdj * iDamage / 100;
		if (iDamage == 0)
			return;

		//	Compute the number of pieces of ejecta

		int iCount;
		if (iDamage <= 5)
			iCount = ((mathRandom(1, 100) <= (iDamage * 20)) ? 1 : 0);
		else if (iDamage <= 12)
			iCount = mathRandom(1, 3);
		else if (iDamage <= 24)
			iCount = mathRandom(2, 6);
		else
			iCount = mathRandom(4, 12);

		//	Generate ejecta

		CWeaponFireDesc *pEjectaType = m_pType->GetEjectaType();
		for (i = 0; i < iCount; i++)
			{
			int iTrajectoryAngle = iDirection 
					+ (mathRandom(0, 12) + mathRandom(0, 12) + mathRandom(0, 12) + mathRandom(0, 12) + mathRandom(0, 12))
					+ (360 - 30);
			iTrajectoryAngle = iTrajectoryAngle % 360;

			Metric rSpeed = pEjectaType->GetInitialSpeed();
			CVector vVel = GetVel() + PolarToVector(iTrajectoryAngle, rSpeed);

			GetSystem()->CreateWeaponFire(pEjectaType,
					0,
					killedByEjecta,
					CDamageSource(this),
					vHitPos,
					vVel,
					iTrajectoryAngle,
					NULL,
					NULL);
			}

		//	Create geyser effect

		CParticleEffect::CreateGeyser(GetSystem(),
				this,
				vHitPos,
				NullVector,
				mathRandom(5, 15),
				mathRandom(50, 150),
				CParticleEffect::paintFlame,
				iDamage + 2,
				0.15 * LIGHT_SPEED,
				iDirection,
				20,
				NULL);
		}
	}

ALERROR CStation::CreateFromType (CSystem *pSystem,
		CStationType *pType,
		const CVector &vPos,
		const CVector &vVel,
		CXMLElement *pExtraData,
		CStation **retpStation)

//	CreateFromType
//
//	Creates a new station based on the type

	{
	ALERROR error;
	CStation *pStation;
	CXMLElement *pDesc = pType->GetDesc();
	int i;

	if (!pType->CanBeEncountered())
		return ERR_FAIL;

	//	Create the new station

	pStation = new CStation;
	if (pStation == NULL)
		return ERR_MEMORY;

	//	Initialize

	pStation->m_pType = pType;
	pStation->SetPos(vPos);
	pStation->SetVel(vVel);
	pStation->CalculateMove();
	pStation->m_pMapOrbit = NULL;
	pStation->m_iDestroyedAnimation = 0;
	pStation->m_fKnown = false;
	pStation->m_fReconned = false;
	pStation->m_fFireReconEvent = false;
	pStation->m_fNoMapLabel = false;
	pStation->m_fActive = pType->IsActive();
	pStation->m_fCustomImage = false;
	pStation->m_fNoReinforcements = false;
	pStation->m_fRadioactive = false;
	pStation->m_fNoArticle = false;
	pStation->m_xMapLabel = 10;
	pStation->m_yMapLabel = -6;
	pStation->m_rMass = pType->GetMass();
	pStation->m_dwWreckUNID = 0;

	//	We generally don't move

	if (!pType->IsMobile())
		pStation->SetCannotMove();
	else
		pStation->SetCanBounce();

	//	Background objects cannot be hit

	if (pType->IsBackgroundObject())
		pStation->SetCannotBeHit();

	//	Friendly fire?

	if (!pType->CanHitFriends())
		pStation->SetNoFriendlyFire();

	if (!pType->CanBeHitByFriends())
		pStation->SetNoFriendlyTarget();

	//	Other

	pStation->SetHasOnObjDockedEvent(pType->HasOnObjDockedEvent());
	pStation->SetHasOnAttackedEvent(pType->FindEventHandler(CONSTLIT("OnAttacked")));
	pStation->SetHasOnDamageEvent(pType->FindEventHandler(CONSTLIT("OnDamage")));

	//	Name

	CXMLElement *pNames = pDesc->GetContentElementByTag(NAMES_TAG);
	if (pNames)
		{
		CString sList = pNames->GetContentText(0);
		pStation->m_sName = GenerateRandomName(sList, pSystem->GetName());
		}
	else
		{
		//	Set the name of the station only if it is not an internal name
		//	(Internal names are in parens. We use internal names when we
		//	want to name the station type, but not station instances).
		CString sName = pDesc->GetAttribute(NAME_ATTRIB);
		if (*sName.GetPointer() != '(')
			pStation->m_sName = sName;
		}

	//	Stargates

	pStation->m_sStargateDestNode = pType->GetDestNodeID();
	pStation->m_sStargateDestEntryPoint = pType->GetDestEntryPoint();

	//	Get the scale

	pStation->m_Scale = pType->GetScale();

	if (pDesc->GetAttributeBool(NO_MAP_LABEL_ATTRIB))
		pStation->m_fNoMapLabel = true;

	//	We block others (CanBlock returns TRUE only for other stations)

	if (pStation->m_Scale != scaleStar && pStation->m_Scale != scaleWorld)
		pStation->SetIsBarrier();

	//	Load hit points and armor information

	pStation->m_iHitPoints = pType->GetInitialHitPoints();
	pStation->m_pArmorClass = pType->GetArmorClass();
	pStation->m_iMaxStructuralHP = pType->GetMaxStructuralHitPoints();
	pStation->m_iStructuralHP = pStation->m_iMaxStructuralHP;

	//	Pick an appropriate image. This call will set the shipwreck image, if
	//	necessary or the variant (if appropriate). HACK: In the case of a shipwreck,
	//	this call also sets the name and other properties (such as structuralHP)

	pType->SetCustomImage(pStation);

	//	Create any items on the station

	if ((error = pStation->CreateRandomItems(pType->GetRandomItemTable())))
		{
		delete pStation;
		return error;
		}

	//	Initialize devices

	CXMLElement *pDevices = pDesc->GetContentElementByTag(DEVICES_TAG);
	if (pDevices)
		{
		CItemListManipulator Items(pStation->GetItemList());

		for (i = 0; 
				(i < pDevices->GetContentElementCount() && i < maxDevices);
				i++)
			{
			CXMLElement *pDeviceDesc = pDevices->GetContentElement(i);
			DWORD dwDeviceID = pDeviceDesc->GetAttributeInteger(DEVICE_ID_ATTRIB);
			CDeviceClass *pClass = g_pUniverse->FindDeviceClass(dwDeviceID);
			if (pClass == NULL)
				return ERR_FAIL;

			//	Allocate the devices structure

			if (pStation->m_pDevices == NULL)
				pStation->m_pDevices = new CInstalledDevice [maxDevices];

			//	Add as an item

			CItem DeviceItem(pClass->GetItemType(), 1);
			Items.AddItem(DeviceItem);

			//	Initialize properties of the device slot

			SDesignLoadCtx Ctx;
			pStation->m_pDevices[i].InitFromXML(Ctx, pDeviceDesc);
			pStation->m_pDevices[i].OnDesignLoadComplete(Ctx);

			//	Install the device

			pStation->m_pDevices[i].Install(pStation, Items, i);

			//	Is this a weapon? If so, set a flag so that we know that
			//	this station is armed. This is an optimization so that we don't
			//	do a lot of work for stations that have no weapons (e.g., asteroids)

			if (pStation->m_pDevices[i].GetCategory() == itemcatWeapon
					|| pStation->m_pDevices[i].GetCategory() == itemcatLauncher)
				{
				pStation->m_fArmed = true;
				pStation->m_pDevices[i].SelectFirstVariant(pStation);
				}
			}
		}

	//	Get notifications when other objects are destroyed

	pStation->SetObjectDestructionHook();

	//	Figure out the sovereign

	pStation->m_pSovereign = pType->GetSovereign();

	//	Initialize docking ports structure

	pStation->m_DockingPorts.InitPortsFromXML(pStation, pDesc);

	//	Make radioactive, if necessary

	if (pType->IsRadioactive())
		pStation->MakeRadioactive();

	//	Add to system (note that we must add the station to the system
	//	before creating any ships).

	if ((error = pStation->AddToSystem(pSystem)))
		{
		delete pStation;
		return error;
		}

	//	Initialize any CodeChain data

	CXMLElement *pInitialData = pDesc->GetContentElementByTag(INITIAL_DATA_TAG);
	if (pInitialData)
		pStation->SetDataFromXML(pInitialData);

	if (pExtraData)
		pStation->SetDataFromXML(pExtraData);

	//	Create any ships registered to this station

	IShipGenerator *pShipGenerator = pType->GetInitialShips();
	if (pShipGenerator)
		pStation->CreateRandomDockedShips(pShipGenerator);

	//	If this is a world or a star, create a small image

	pStation->CreateMapImage();

	//	Set the bounds for this object

	int iTick, iRotation;
	const CObjectImageArray &Image = pStation->GetImage(&iTick, &iRotation);
	const RECT &rcImage = Image.GetImageRect();
	pStation->SetBounds(rcImage);

	//	This type has now been encountered

	pType->SetEncountered();

	//	If we're not in the middle of creating the system, call OnCreate
	//	(otherwise we will call OnCreate in OnSystemCreated)

	if (!pSystem->IsCreationInProgress())
		pStation->FireOnCreate();

	//	If this is a stargate, tell the system that we've got a stargate

	if (!pStation->m_sStargateDestNode.IsBlank())
		pSystem->StargateCreated(pStation, NULL_STR, pStation->m_sStargateDestNode, pStation->m_sStargateDestEntryPoint);

	//	Return station

	if (retpStation)
		*retpStation = pStation;

	return NOERROR;
	}

ALERROR CStation::CreateMapImage (void)

//	CreateMapImage
//
//	Creates a small version of the station image

	{
	ALERROR error;

	//	Only do this for stars and planets

	if (m_Scale != scaleStar && m_Scale != scaleWorld)
		return NOERROR;

	//	Make sure we have an image

	int iTick, iRotation;
	const CObjectImageArray &Image = GetImage(&iTick, &iRotation);
	if (Image.IsEmpty())
		return NOERROR;

	CG16bitImage &BmpImage = Image.GetImage();
	const RECT &rcImage = Image.GetImageRect();

	if ((error = m_MapImage.CreateBlank(RectWidth(rcImage) / g_iMapScale,
			RectHeight(rcImage) / g_iMapScale,
			false)))
		return error;

	m_MapImage.BltRotoZ(0, 0, m_MapImage.GetWidth(), m_MapImage.GetHeight(),
			BmpImage, rcImage.left, rcImage.top + RectHeight(rcImage) * iRotation,
			rcImage.right, rcImage.top + RectHeight(rcImage) * iRotation + RectHeight(rcImage),
			0, 1, false);
 
	return NOERROR;
	}

void CStation::CreateRandomDockedShips (IShipGenerator *pShipGenerator)

//	CreateRandomDockedShips
//
//	Creates all the ships that are registered at this station

	{
	SShipCreateCtx Ctx;

	Ctx.pSystem = GetSystem();
	Ctx.pBase = this;
	Ctx.vPos = GetPos();
	//	1d8+1 light-second spread
	Ctx.PosSpread = DiceRange(8, 1, 1);
	Ctx.dwFlags = SShipCreateCtx::SHIPS_FOR_STATION;

	//	Create the ships

	pShipGenerator->CreateShips(Ctx);
	}

void CStation::CreateStructuralDestructionEffect (void)

//	CreateStructuralDestructionEffect
//
//	Create effect when station structure is destroyed

	{
	//	Create fracture effect

	int iTick, iVariant;
	const CObjectImageArray &Image = GetImage(&iTick, &iVariant);

	CFractureEffect::Create(GetSystem(),
			GetPos(),
			GetVel(),
			Image,
			iTick,
			iVariant,
			CFractureEffect::styleExplosion,
			NULL);

	//	Create explosion

	SExplosionType Explosion;
	FireGetExplosionType(&Explosion);
	if (Explosion.pDesc == NULL)
		Explosion.pDesc = m_pType->GetExplosionType();

	if (Explosion.pDesc)
		{
		GetSystem()->CreateWeaponFire(Explosion.pDesc,
				Explosion.iBonus,
				Explosion.iCause,
				CDamageSource(this),
				GetPos(),
				GetVel(),
				0,
				NULL,
				NULL);
		}
	else
		{
		//	Create Particles

		CObjectImageArray PartImage;
		RECT rcRect;
		rcRect.left = 0;
		rcRect.top = 0;
		rcRect.right = 4;
		rcRect.bottom = 4;
		PartImage.Init(g_ShipExplosionParticlesUNID,
				rcRect,
				8,
				3);

		CParticleEffect::CreateExplosion(GetSystem(),
				NULL,
				GetPos(),
				GetVel(),
				mathRandom(1, 50),
				LIGHT_SPEED * 0.25,
				0,
				300,
				PartImage,
				NULL);

		CEffectCreator *pEffect = g_pUniverse->FindEffectType(g_ExplosionUNID);
		if (pEffect)
			pEffect->CreateEffect(GetSystem(),
				NULL,
				GetPos(),
				GetVel());
		}

	g_pUniverse->PlaySound(this, g_pUniverse->FindSound(g_ShipExplosionSoundUNID));
	}

CSpaceObject::DamageResults CStation::Damage (CSpaceObject *pCause, const CVector &vHitPos, int iDirection, const DamageDesc &Damage)

//	Damage
//
//	Station takes damage

	{
	//	Create hit effect

	CEffectCreator *pEffect = g_pUniverse->FindEffectType(g_HitEffectUNID);
	if (pEffect)
		pEffect->CreateEffect(GetSystem(),
				this,
				vHitPos,
				GetVel());

	//	If this is a momentum attack then we are pushed

	int iMomentum;
	if (IsMobile() && (iMomentum = Damage.GetMomentumDamage()))
		{
		CVector vAccel = PolarToVector(iDirection, -10 * iMomentum * iMomentum);
		Accelerate(vAccel, g_MomentumConstant);
		ClipSpeed(0.25 * LIGHT_SPEED);
		}

	//	If we're immutable, then nothing else happens.

	if (m_pType->IsImmutable())
		return damageNoDamage;

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

	//	OnAttacked event

	if (HasOnAttackedEvent())
		FireOnAttacked(Ctx);

	//	We go through a different path if we're already abandoned
	//	(i.e., there is no life on the station)

	if (IsAbandoned())
		{
		DamageResults iResult = damageNoDamage;

		//	If this is a paralysis attack then no damage

		int iEMP = Ctx.Damage.GetEMPDamage();
		if (iEMP)
			Ctx.iDamage = 0;

		//	If this is blinding attack then no damage

		int iBlinding = Ctx.Damage.GetBlindingDamage();
		if (iBlinding)
			Ctx.iDamage = 0;

		//	If this is a radioactive attack then there is a chance that we will
		//	be contaminated. (Note: We can only be contaminated by an attack if
		//	we're abandoned)

		int iRadioactive = Damage.GetRadiationDamage();
		if (iRadioactive 
				&& GetScale() != scaleStar
				&& GetScale() != scaleWorld)
			{
			int iChance = 4 * iRadioactive * iRadioactive;
			if (mathRandom(1, 100) <= iChance)
				MakeRadioactive();
			}

		//	If we have mining damage then call OnMining

		if (Damage.GetMiningAdj())
			FireOnMining(Ctx);

		//	Once the station is abandoned, only WMD damage can destroy it

		Ctx.iDamage = Damage.GetMassDestructionAdj() * Ctx.iDamage / 100;

		//	Give events a chance to change the damage

		if (HasOnDamageEvent())
			FireOnDamage(Ctx);

		//	Take damage

		if (Ctx.iDamage > 0)
			{
			//	See if this hit destroyed us

			if (m_iStructuralHP > 0 && m_iStructuralHP <= Ctx.iDamage)
				{
				CreateStructuralDestructionEffect();

				Destroy(killedByDamage, Ctx.pAttacker);
				return damageDestroyed;
				}

			//	See if we should generate ejecta

			CreateEjectaFromDamage(Ctx.iDamage, vHitPos, iDirection, Damage);

			//	If we can be destroyed, subtract from hp

			if (m_iStructuralHP)
				{
				m_iStructuralHP -= Ctx.iDamage;
				iResult = damageStructuralHit;
				}
			}

		//	Otherwise, we can't take any more damage

		return iResult;
		}

	//	If we're not abandoned, we go through a completely different code-path

	CSpaceObject *pOrderGiver;

	if (Ctx.pAttacker 
			&& (pOrderGiver = Ctx.pAttacker->GetOrderGiver(Ctx.Damage.GetCause())) 
			&& pOrderGiver->CanAttack())
		{
		//	If the attacker is a friend then we should keep track of
		//	friendly fire hits

		if (!IsEnemy(pOrderGiver) && !IsBlacklisted(pOrderGiver))
			{
			//	Ignore automated attacks

			if (!Damage.IsAutomatedWeapon())
				{
				Communicate(pOrderGiver, msgWatchTargets);
				FriendlyFire(pOrderGiver);
				}
			}

		//	Otherwise, consider this a hostile act.

		else
			{
			//	Tell our guards that we were attacked

			for (int i = 0; i < m_Subordinates.GetCount(); i++)
				Communicate(m_Subordinates.GetObj(i), msgAttack, Ctx.pAttacker);

			//	Alert other stations

			if (m_pType->AlertWhenAttacked())
				RaiseAlert(pOrderGiver);

			//	We get angry

			SetAngry();
			}
		}

	//	If this armor section reflects this kind of damage then
	//	send the damage on

	if (m_pArmorClass->IsReflective(NULL, Ctx.Damage) && Ctx.pCause)
		{
		Ctx.pCause->CreateReflection(Ctx.vHitPos, (Ctx.iDirection + 120 + mathRandom(0, 120)) % 360);
		return damageNoDamage;
		}

	//	If this is a paralysis attack then no damage

	int iEMP = Ctx.Damage.GetEMPDamage();
	if (iEMP)
		Ctx.iDamage = 0;

	//	If this is blinding attack then no damage

	int iBlinding = Ctx.Damage.GetBlindingDamage();
	if (iBlinding)
		Ctx.iDamage = 0;

	//	If this is device damage, then damage is decreased

	int iDeviceDamage = Ctx.Damage.GetDeviceDamage();
	if (iDeviceDamage)
		Ctx.iDamage = Ctx.iDamage / 2;

	//	Adjust the damage for the armor

	Ctx.iDamage = m_pArmorClass->CalcAdjustedDamage(NULL, Damage, Ctx.iDamage);
	if (Ctx.iDamage == 0)
		return damageNoDamage;

	//	If we're a multi-hull object then we adjust for mass destruction
	//	effects (non-mass destruction weapons don't hurt us very much)

	if (m_pType->IsMultiHull())
		{
		int iWMD = Damage.GetMassDestructionAdj();
		Ctx.iDamage = Max(1, iWMD * Ctx.iDamage / 100);
		}

	//	Give events a chance to change the damage

	if (HasOnDamageEvent())
		FireOnDamage(Ctx);

	//	If we've still got armor left, then we take damage but otherwise
	//	we're OK.

	if (Ctx.iDamage < m_iHitPoints)
		{
		m_iHitPoints -= Ctx.iDamage;
		return damageArmorHit;
		}

	//	Otherwise we're in big trouble

	else
		{
		m_iHitPoints = 0;

		SDestroyCtx DestroyCtx;
		DestroyCtx.pObj = this;
		DestroyCtx.pDestroyer = Ctx.pAttacker;
		DestroyCtx.pWreck = this;
		DestroyCtx.iCause = Damage.GetCause();

		//	Run OnDestroy script

		FireOnDestroy(DestroyCtx);

		//	Station is destroyed. Take all the installed devices and turn
		//	them into normal damaged items

		CItemListManipulator Items(GetItemList());
		while (Items.MoveCursorForward())
			{
			CItem Item = Items.GetItemAtCursor();

			if (Item.IsInstalled())
				{
				//	Uninstall the device

				int iDevSlot = Item.GetInstalled();
				CInstalledDevice *pDevice = &m_pDevices[iDevSlot];
				pDevice->Uninstall(this, Items);

				//	Chance that the item is destroyed

				if (Item.GetType()->IsVirtual() || mathRandom(1, 100) <= 50)
					Items.DeleteAtCursor(1);
				else
					Items.SetDamagedAtCursor(true);

				//	Reset cursor because we may have changed position

				Items.ResetCursor();
				}
			}

		InvalidateItemListAddRemove();

		//	Tell all objects that we've been destroyed

		for (int i = 0; i < GetSystem()->GetObjectCount(); i++)
			{
			CSpaceObject *pObj = GetSystem()->GetObject(i);

			if (pObj && pObj != this)
				pObj->OnStationDestroyed(DestroyCtx);
			}

		//	Alert others, if necessary

		if (pOrderGiver && pOrderGiver->CanAttack())
			{
			if (m_pType->AlertWhenDestroyed())
				RaiseAlert(pOrderGiver);
			}

		//	Explosion effect

		CreateDestructionEffect();

		return damageDestroyed;
		}
	}

CString CStation::DebugCrashInfo (void)

//	DebugCrashInfo
//
//	Dump crash information

	{
	int i;
	CString sResult;

	if (IsAbandoned())
		sResult.Append(CONSTLIT("abandoned\r\n"));

	sResult.Append(strPatternSubst(CONSTLIT("m_pTarget: %s\r\n"), CSpaceObject::DebugDescribe(m_pTarget).GetASCIIZPointer()));

	for (i = 0; i < m_Targets.GetCount(); i++)
		sResult.Append(strPatternSubst(CONSTLIT("m_Targets[%d]: %s\r\n"), i, CSpaceObject::DebugDescribe(m_Targets.GetObj(i)).GetASCIIZPointer()));

	for (i = 0; i < m_Subordinates.GetCount(); i++)
		sResult.Append(strPatternSubst(CONSTLIT("m_Subordinates[%d]: %s\r\n"), i, CSpaceObject::DebugDescribe(m_Subordinates.GetObj(i)).GetASCIIZPointer()));

	for (i = 0; i < m_DockingPorts.GetPortCount(this); i++)
		{
		CSpaceObject *pDockedObj = m_DockingPorts.GetPortObj(this, i);
		if (pDockedObj)
			sResult.Append(strPatternSubst(CONSTLIT("m_DockingPorts[%d]: %s\r\n"), i, CSpaceObject::DebugDescribe(pDockedObj).GetASCIIZPointer()));
		}

	return sResult;
	}

void CStation::FriendlyFire (CSpaceObject *pAttacker)

//	FriendlyFire
//
//	Station is hit by friendly fire. See if we need to blacklist
//	the attacker.

	{
	//	For now, only blacklist the player

	if (pAttacker != g_pUniverse->GetPlayer())
		return;

	//	No need if we don't support blacklist

	if (!m_pType->IsBlacklistEnabled())
		return;

	//	See if we need to blacklist

	if (m_Blacklist.Hit(GetSystem()->GetTick()))
		{
		Blacklist(pAttacker);

		SetAngry();
		}
	}

int CStation::GetBalance (void)

//	GetBalance
//
//	Returns the amount of money the station has left

	{
	return m_iBalance;
	}

int CStation::GetBuyPrice (const CItem &Item, int *retiMaxCount)

//	GetBuyPrice
//
//	Returns the price at which the station will buy the given
//	item. Also returns the max number of items that the station
//	will buy at that price.
//
//	Returns -1 if the station will not buy the item.

	{
	CTradingDesc *pTrade = m_pType->GetTradingDesc();
	if (pTrade)
		{
		int iPrice;
		if (pTrade->Buys(this, Item, &iPrice, retiMaxCount))
			return iPrice;
		else
			return -1;
		}
	else
		return -1;
	}

CXMLElement *CStation::GetDockScreen (void)

//	GetDockScreen
//
//	Returns the screen on dock (NULL if none)

	{
	if (IsAbandoned() && m_pType->GetAbandonedScreen())
		return m_pType->GetAbandonedScreen();
	else
		return m_pType->GetFirstDockScreen();
	}

const CObjectImageArray &CStation::GetImage (int *retiTick, int *retiRotation)

//	GetImage
//
//	Returns the image of this station

	{
	int iTick;
	if (m_fActive && !IsTimeStopped())
		iTick = GetSystem()->GetTick() + GetDestiny();
	else
		iTick = 0;

	if (IsAbandoned() && m_pType->HasWreckImage())
		{
		*retiTick = iTick;
		*retiRotation = 0;
		return m_pType->GetWreckImage(m_iVariant);
		}
	else if (m_fCustomImage)
		{
		*retiTick = iTick + (m_Image.GetTicksPerFrame() * GetDestiny());
		*retiRotation = m_iVariant;
		return m_Image;
		}
	else
		{
		*retiTick = iTick;
		return m_pType->GetImage(m_iVariant, retiRotation);
		}
	}

CString CStation::GetName (DWORD *retdwFlags)

//	GetName
//
//	Returns the name of the station

	{
	if (retdwFlags)
		{
		//	If the instance is overriding the class

		if (m_fNoArticle)
			*retdwFlags |= nounNoArticle;
		else
			{
			*retdwFlags = 0;
			if (m_pType->UsesDefiniteArticle())
				*retdwFlags |= nounDefiniteArticle;
			else if (m_pType->UsesNoArticle())
				*retdwFlags |= nounNoArticle;
			else if (m_pType->UsesReverseArticle())
				*retdwFlags |= nounVowelArticle;
			}
		}

	return m_sName;
	}

CVector CStation::GetNearestDockVector (CSpaceObject *pRequestingObj)

//	GetNearestDockVector
//
//	Returns a vector from the given position to the nearest
//	dock position

	{
	CVector vDistance;
	m_DockingPorts.FindNearestEmptyPort(this, pRequestingObj, &vDistance);
	return vDistance;
	}

IShipGenerator *CStation::GetRandomEncounterTable (int *retiFrequency)

//	GetRandomEncounterTable
//
//	Returns a random encounter table for this station

	{
	if (retiFrequency)
		*retiFrequency = m_pType->GetEncounterFrequency();
	return m_pType->GetEncountersTable();
	}

int CStation::GetSellPrice (const CItem &Item)

//	GetSellPrice
//
//	Returns the price at which the station will sell the given
//	item. Returns 0 if the station cannot or will not sell the
//	item.

	{
	CTradingDesc *pTrade = m_pType->GetTradingDesc();
	if (pTrade)
		{
		//	See if the station sells these kinds of items. If not
		//	then return 0.

		int iPrice;
		if (!pTrade->Sells(this, Item, &iPrice))
			return 0;

		//	See if we have any of the item to sell. If not, then we
		//	return 0.

		CItemListManipulator ItemList(GetItemList());
		if (!ItemList.SetCursorAtItem(Item))
			return 0;

		//	Return the price

		return iPrice;
		}
	else
		return 0;
	}

CString CStation::GetStargateID (void) const

//	GetStargateID
//
//	Returns the stargate ID

	{
	if (m_sStargateDestNode.IsBlank())
		return NULL_STR;

	CSystem *pSystem = GetSystem();
	if (pSystem == NULL)
		return NULL_STR;

	CTopologyNode *pNode = pSystem->GetTopology();
	if (pNode == NULL)
		return NULL_STR;

	return pNode->FindStargateName(m_sStargateDestNode, m_sStargateDestEntryPoint);
	}

int CStation::GetVisibleDamage (void)

//	GetVisibleDamage
//
//	Returns the amount of damage (%) that the object has taken

	{
	int iMaxHP;
	int iHP;

	if (IsAbandoned() && m_iStructuralHP > 0)
		{
		iMaxHP = m_iMaxStructuralHP;
		iHP = m_iStructuralHP;
		}
	else
		{
		iMaxHP = m_pType->GetMaxHitPoints();
		iHP = m_iHitPoints;
		}

	if (iMaxHP > 0)
		return 100 - (iHP * 100 / iMaxHP);
	else
		return 0;
	}

bool CStation::HasAttribute (const CString &sAttribute) const

//	HasAttribute
//
//	Returns TRUE if it has the given attribute

	{
	return ::HasModifier(m_pType->GetAttributes(), sAttribute);
	}

bool CStation::HasMapLabel (void)

//	HasMapLabel
//
//	Returns TRUE if the object has a map label

	{
	return m_Scale != scaleWorld
			&& m_Scale != scaleStar
			&& m_pType->ShowsMapIcon() 
			&& !m_fNoMapLabel;
	}

bool CStation::ImageInObject (const CObjectImageArray &Image, int iTick, int iRotation, CVector vPos)

//	ImageInObject
//
//	Returns TRUE if the given image at the given position intersects this
//	station

	{
	int iDestTick, iDestVariant;
	const CObjectImageArray &DestImage = GetImage(&iDestTick, &iDestVariant);

	return ImagesIntersect(Image, iTick, iRotation, vPos,
			DestImage, iDestTick, iDestVariant, GetPos());
	}

bool CStation::IsBlacklisted (CSpaceObject *pObj)

//	IsBlacklisted
//
//	Returns TRUE if we are blacklisted
	
	{
	return (pObj == g_pUniverse->GetPlayer() && m_Blacklist.IsBlacklisted());
	}

void CStation::OnDestroyed (DestructionTypes iCause, CSpaceObject *pCause, bool *ioResurrectPending, CSpaceObject **retpWreck)

//	OnDestroyed
//
//	Station has been destroyed

	{
	m_DockingPorts.OnDestroyed();
	}

void CStation::OnMove (const CVector &vOldPos, Metric rSeconds)

//	OnMove
//
//	Do stuff when station moves

	{
	//	If the station is moving then make sure all docked ships
	//	move along with it.

	m_DockingPorts.MoveAll(this);
	}

void CStation::ObjectDestroyedHook (const SDestroyCtx &Ctx)

//	ObjectDestroyedHook
//
//	Notification of another object being destroyed

	{
	bool bAttackDestroyer;

	//	Figure out who gave the orders

	CSpaceObject *pOrderGiver = NULL;
	if (Ctx.pDestroyer)
		pOrderGiver = Ctx.pDestroyer->GetOrderGiver(Ctx.iCause);

	//	If this object is docked with us, remove it from the
	//	docking table.

	m_DockingPorts.OnObjDestroyed(this, Ctx.pObj, &bAttackDestroyer);

	//	Remove the object from any lists that it may be on

	m_Targets.Remove(Ctx.pObj);

	if (Ctx.pObj == m_pTarget)
		m_pTarget = NULL;

	//	If this was a subordinate, then send an alert, if necessary

	if (m_Subordinates.Remove(Ctx.pObj))
		bAttackDestroyer = true;

	//	Retaliate against the attacker

	if (bAttackDestroyer 
			&& pOrderGiver
			&& pOrderGiver->CanAttack())
		{
		if (!IsEnemy(pOrderGiver) && !IsBlacklisted(pOrderGiver))
			Blacklist(pOrderGiver);
		else if (m_pType->AlertWhenAttacked())
			RaiseAlert(pOrderGiver);

		SetAngry();
		}

	//	If this object is registered, call the events

	if (m_RegisteredObjects.FindObj(Ctx.pObj))
		{
		FireOnObjDestroyed(Ctx);

		//	Note: FireOnObjDestroyed might have unregistered the object
		//	so we cannot make any assumptions about whether the object
		//	is in the list or not.

		m_RegisteredObjects.Remove(Ctx.pObj);
		}
	}

bool CStation::ObjectInObject (CSpaceObject *pObj)

//	ObjectInObject
//
//	Returns TRUE if the given object intersects this object

	{
	int iTick, iVariant;
	const CObjectImageArray &Image = GetImage(&iTick, &iVariant);

	return pObj->ImageInObject(Image, iTick, iVariant, GetPos());
	}

DWORD CStation::OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2)

//	OnCommunicate
//
//	Handle communications

	{
	switch (iMessage)
		{
		case msgAttack:
		case msgDestroyBroadcast:
			{
			CSpaceObject *pTarget = pParam1;

			if (!IsEnemy(pTarget))
				{
				Blacklist(pTarget);
				}

			//	Attack the target, unless we've already got it on our
			//	target list.

			else if (!m_Targets.FindObj(pTarget))
				{
				m_Targets.Add(pTarget);

#ifdef DEBUG_ALERTS
				g_pUniverse->DebugOutput("%d: Received msgDestroyBroadcast", this);
#endif

				//	Order out some number of subordinates to attack
				//	the target.

				int iLeft = mathRandom(3, 5);
				int iCount = m_Subordinates.GetCount();
				for (int i = 0; i < iCount && iLeft > 0; i++)
					{
					if (Communicate(m_Subordinates.GetObj(i), 
							msgDestroyBroadcast, 
							pTarget) == resAck)
						{
						iLeft--;
#ifdef DEBUG_ALERTS
						g_pUniverse->DebugOutput("   %d acknowledges attack order", m_Subordinates.GetObj(i));
#endif
						}
					}
				}

			return resAck;
			}

		default:
			return resNoAnswer;
		}
	}

void CStation::OnPaint (CG16bitImage &Dest, int x, int y, SViewportPaintCtx &Ctx)

//	OnPaint
//
//	Paint the station

	{
	//	Known

	m_fKnown = true;
	if (!m_fReconned)
		{
		if (m_fFireReconEvent)
			{
			Reconned();
			m_fFireReconEvent = false;
			}

		m_fReconned = true;
		}

	//	Paint

	int iTick, iVariant;
	const CObjectImageArray &Image = GetImage(&iTick, &iVariant);
	if (m_fRadioactive)
		Image.PaintImageWithGlow(Dest, x, y, iTick, iVariant, CGImage::RGBColor(0, 255, 0));
	else
		Image.PaintImage(Dest, x, y, iTick, iVariant);

	//	Paint animations

	if (!IsAbandoned() && m_pType->HasAnimations())
		m_pType->PaintAnimations(Dest, x, y, iTick);

	//	If this is a sign, then paint the name of the station

	if (m_pType->IsSign() && !IsAbandoned())
		{
		RECT rcRect;

		rcRect.left = x - 26;
		rcRect.top = y - 20;
		rcRect.right = x + 40;
		rcRect.bottom = y + 20;

		g_pUniverse->GetSignFont().DrawText(Dest, rcRect, RGB_SIGN_COLOR, GetName(), -2);
		}

	//	Highlight

	if (IsHighlighted() && !Ctx.fNoSelection)
		PaintHighlight(Dest, Image.GetImageRectAtPoint(x, y));
	}

void CStation::OnObjBounce (CSpaceObject *pObj, const CVector &vPos)

//	OnObjBounce
//
//	An object has just bounced off the station

	{
	CEffectCreator *pEffect;
	if (pEffect = m_pType->GetBarrierEffect())
		pEffect->CreateEffect(GetSystem(),
				this,
				vPos,
				GetVel());
	}

void CStation::OnObjDocked (CSpaceObject *pObj, CSpaceObject *pDockTarget)

//	OnObjDocked
//
//	The given object has docked with the station
	
	{
	if (!m_RegisteredObjects.IsEmpty()
			&& m_RegisteredObjects.FindObj(pObj))
		FireOnObjDocked(pObj, pDockTarget);
	}

void CStation::OnObjEnteredGate (CSpaceObject *pObj, CSpaceObject *pStargate)

//	OnObjEnteredGate
//
//	The given object has just entered some random gate (no necessarily us).

	{
	if (!m_RegisteredObjects.IsEmpty() 
			&& m_RegisteredObjects.FindObj(pObj))
		FireOnObjEnteredGate(pObj, pStargate);
	}

void CStation::OnObjJumped (CSpaceObject *pObj)

//	OnObjJumped
//
//	This is called when another object has jumped in the system

	{
	if (!m_RegisteredObjects.IsEmpty() 
			&& m_RegisteredObjects.FindObj(pObj))
		FireOnObjJumped(pObj);
	}

void CStation::OnObjReconned (CSpaceObject *pObj)

//	OnObjReconned
//
//	This is called when another object has been reconned

	{
	if (!m_RegisteredObjects.IsEmpty() 
			&& m_RegisteredObjects.FindObj(pObj))
		FireOnObjReconned(pObj);
	}

void CStation::OnObjLeaveGate (CSpaceObject *pObj)

//	OnObjLeaveGate
//
//	An object has just passed through gate

	{
	//	Create gating effect

	CEffectCreator *pEffect = m_pType->GetGateEffect();
	if (pEffect)
		pEffect->CreateEffect(GetSystem(),
				NULL,
				GetPos(),
				GetVel());
	}

void CStation::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Read object data from a stream
//
//	DWORD		station type UNID
//	CString		m_sName
//	DWORD		sovereign UNID
//	DWORD		m_Scale
//	Metric		m_rMass
//	DWORD		m_iVariant
//	DWORD		m_iDestroyedAnimation
//	DWORD		1 if orbit, 0xffffffff if no orbit
//	Orbit		System orbit
//	DWORD		m_xMapLabel
//	DWORD		m_yMapLabel
//	CString		m_sStargateDestNode
//	CString		m_sStargateDestEntryPoint
//	DWORD		armor class UNID, 0xffffffff if no armor
//	DWORD		m_iHitPoints
//	DWORD		m_iStructuralHP
//	DWORD		m_iMaxStructuralHP
//
//	DWORD		No of devices
//	DWORD		device: class UNID
//	DWORD		device: m_dwData
//	DWORD		device: low = m_iPosAngle; hi = m_iPosRadius
//	DWORD		device: low = m_iMinFireArc; hi = m_iMaxFireArc
//	DWORD		device: low = m_iActivationDelay
//	DWORD		device: flags
//
//	DWORD		No of registered objects
//	DWORD		registered object (CSpaceObject ref)
//
//	DWORD		No of docking ports
//	DWORD		port: iStatus
//	DWORD		port: pObj (CSpaceObject ref)
//	Vector		port: vPos
//
//	DWORD		No of subordinates
//	DWORD		subordinate (CSpaceObject ref)
//
//	DWORD		No of targets
//	DWORD		target (CSpaceObject ref)
//
//	CAttackDetector m_Blacklist
//	DWORD		m_iAngryCounter
//	DWORD		m_iReinforceRequestCount
//	DWORD		m_iBalance
//
//	DWORD		m_dwWreckUNID
//	DWORD		flags

	{
#ifdef DEBUG_LOAD
	::OutputDebugString("CStation::OnReadFromStream\n");
#endif
	int i;
	DWORD dwLoad;

	//	Station type

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_pType = g_pUniverse->FindStationType(dwLoad);

	SetHasOnObjDockedEvent(m_pType->HasOnObjDockedEvent());
	SetHasOnAttackedEvent(m_pType->FindEventHandler(CONSTLIT("OnAttacked")));
	SetHasOnDamageEvent(m_pType->FindEventHandler(CONSTLIT("OnDamage")));

	//	Stuff

	m_sName.ReadFromStream(Ctx.pStream);
	Ctx.pSystem->ReadSovereignRefFromStream(Ctx, &m_pSovereign);
	Ctx.pStream->Read((char *)&m_Scale, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_rMass, sizeof(Metric));
	Ctx.pStream->Read((char *)&m_iVariant, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iDestroyedAnimation, sizeof(DWORD));

	//	Load orbit

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad != 0xffffffff)
		{
		m_pMapOrbit = new CSystem::Orbit;
		Ctx.pStream->Read((char *)m_pMapOrbit, sizeof(CSystem::Orbit));
		}

	//	More stuff

	Ctx.pStream->Read((char *)&m_xMapLabel, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_yMapLabel, sizeof(DWORD));

	//	Load the stargate info

	if (Ctx.dwVersion >= 16)
		{
		m_sStargateDestNode.ReadFromStream(Ctx.pStream);
		m_sStargateDestEntryPoint.ReadFromStream(Ctx.pStream);
		}
	else
		{
		CString sStargate;
		sStargate.ReadFromStream(Ctx.pStream);

		if (!sStargate.IsBlank())
			{
			CTopologyNode *pNode = Ctx.pSystem->GetStargateDestination(sStargate, &m_sStargateDestEntryPoint);
			if (pNode)
				m_sStargateDestNode = pNode->GetID();
			}
		}

	//	Armor class

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad != 0xffffffff)
		m_pArmorClass = g_pUniverse->FindArmor(dwLoad);

	Ctx.pStream->Read((char *)&m_iHitPoints, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iStructuralHP, sizeof(DWORD));
	if (Ctx.dwVersion >= 31)
		Ctx.pStream->Read((char *)&m_iMaxStructuralHP, sizeof(DWORD));

	//	Devices

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad)
		{
		m_pDevices = new CInstalledDevice [dwLoad];

		for (i = 0; i < (int)dwLoad; i++)
			m_pDevices[i].ReadFromStream(Ctx);
		}

	//	Registered objects

	m_RegisteredObjects.ReadFromStream(Ctx);

	//	Docking ports

	m_DockingPorts.ReadFromStream(this, Ctx);

	//	Subordinates

	m_Subordinates.ReadFromStream(Ctx);
	m_Targets.ReadFromStream(Ctx);

	//	More

	if (Ctx.dwVersion >= 9)
		m_Blacklist.ReadFromStream(Ctx);
	else
		{
		CSovereign *pBlacklist;
		int iCounter;

		Ctx.pSystem->ReadSovereignRefFromStream(Ctx, &pBlacklist);
		Ctx.pStream->Read((char *)&iCounter, sizeof(DWORD));

		if (pBlacklist != NULL)
			m_Blacklist.Blacklist();
		}

	if (Ctx.dwVersion >= 3)
		Ctx.pStream->Read((char *)&m_iAngryCounter, sizeof(DWORD));
	else
		m_iAngryCounter = 0;

	if (Ctx.dwVersion >= 9)
		Ctx.pStream->Read((char *)&m_iReinforceRequestCount, sizeof(DWORD));
	else
		m_iReinforceRequestCount = 0;

	if (Ctx.dwVersion >= 12)
		Ctx.pStream->Read((char *)&m_iBalance, sizeof(DWORD));
	else
		m_iBalance = 0;

	//	Wreck UNID

	Ctx.pStream->Read((char *)&m_dwWreckUNID, sizeof(DWORD));
	if (m_dwWreckUNID)
		{
		CShipClass *pClass = g_pUniverse->FindShipClass(m_dwWreckUNID);
		pClass->SetShipwreckImage(this);
		}

	//	Flags

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_fArmed =			((dwLoad & 0x00000001) ? true : false);
	m_fKnown =			((dwLoad & 0x00000002) ? true : false);
	m_fNoMapLabel =		((dwLoad & 0x00000004) ? true : false);
	m_fRadioactive =	((dwLoad & 0x00000008) ? true : false);
	m_fCustomImage =	((dwLoad & 0x00000010) ? true : false);
	m_fActive =			((dwLoad & 0x00000020) ? true : false);
	m_fNoReinforcements =((dwLoad & 0x00000040) ? true : false);
	m_fReconned =		((dwLoad & 0x00000080) ? true : false);
	m_fFireReconEvent =	((dwLoad & 0x00000100) ? true : false);
	m_fNoArticle =		((dwLoad & 0x00000200) ? true : false);

	//	If this is a world or a star, create a small image

	CreateMapImage();

	//	For previous version, we have to set CannotBeHit manually

	if (m_pType->IsBackgroundObject())
		SetCannotBeHit();
	}

void CStation::OnStationDestroyed (const SDestroyCtx &Ctx)

//	OnStationDestroyed
//
//	Station in the system has been destroyed

	{
	int i;
	if (!m_RegisteredObjects.IsEmpty()
			&& m_RegisteredObjects.FindObj(Ctx.pObj, &i))
		{
		FireOnObjDestroyed(Ctx);
		m_RegisteredObjects.Remove(Ctx.pObj);
		}
	}

void CStation::OnSystemCreated (void)

//	OnSystemCreated
//
//	Called when the system is created

	{
	int i;

	//	If this is a beacon, scan all stations in range

	if (m_pType->IsBeacon())
		{
		for (i = 0; i < GetSystem()->GetObjectCount(); i++)
			{
			CSpaceObject *pObj = GetSystem()->GetObject(i);

			if (pObj 
					&& pObj->GetScale() == scaleStructure
					&& pObj != this)
				{
				CVector vDist = pObj->GetPos() - GetPos();
				Metric rDist = vDist.Length();
				if (rDist < BEACON_RANGE)
					pObj->SetKnown();
				}
			}
		}

	//	Initialize trading

	CTradingDesc *pTrade = m_pType->GetTradingDesc();
	if (pTrade)
		pTrade->OnCreate(this);

	//	Fire OnCreate

	FireOnCreate();
	}

void CStation::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Write the object's data to stream
//
//	DWORD		station type UNID
//	CString		m_sName
//	DWORD		sovereign UNID
//	DWORD		m_Scale
//	Metric		m_rMass
//	DWORD		m_iVariant
//	DWORD		m_iDestroyedAnimation
//	DWORD		1 if orbit, 0xffffffff if no orbit
//	Orbit		System orbit
//	DWORD		m_xMapLabel
//	DWORD		m_yMapLabel
//	CString		m_sStargateDestNode
//	CString		m_sStargateDestEntryPoint
//	DWORD		armor class UNID, 0xffffffff if no armor
//	DWORD		m_iHitPoints
//	DWORD		m_iStructualHP
//
//	DWORD		device: class UNID
//	DWORD		device: m_dwData
//	DWORD		device: low = m_iPosAngle; hi = m_iPosRadius
//	DWORD		device: low = m_iMinFireArc; hi = m_iMaxFireArc
//	DWORD		device: low = m_iActivationDelay
//	DWORD		device: flags
//
//	DWORD		No of registered objects
//	DWORD		registered object (CSpaceObject ref)
//
//	DWORD		No of docking ports
//	DWORD		port: iStatus
//	DWORD		port: pObj (CSpaceObject ref)
//	Vector		port: vPos
//
//	DWORD		No of subordinates
//	DWORD		subordinate (CSpaceObject ref)
//
//	DWORD		No of targets
//	DWORD		target (CSpaceObject ref)
//
//	CAttackDetector m_Blacklist
//	DWORD		m_iAngryCounter
//	DWORD		m_iReinforceRequestCount
//	DWORD		m_iBalance
//
//	DWORD		m_dwWreckUNID
//	DWORD		flags

	{
	int i;
	DWORD dwSave;

	dwSave = m_pType->GetUNID();
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	m_sName.WriteToStream(pStream);
	GetSystem()->WriteSovereignRefToStream(m_pSovereign, pStream);
	pStream->Write((char *)&m_Scale, sizeof(DWORD));
	pStream->Write((char *)&m_rMass, sizeof(Metric));
	pStream->Write((char *)&m_iVariant, sizeof(DWORD));
	pStream->Write((char *)&m_iDestroyedAnimation, sizeof(DWORD));

	if (m_pMapOrbit)
		{
		dwSave = 1;
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		pStream->Write((char *)m_pMapOrbit, sizeof(CSystem::Orbit));
		}
	else
		{
		dwSave = 0xffffffff;
		pStream->Write((char *)&dwSave, sizeof(DWORD));
		}

	pStream->Write((char *)&m_xMapLabel, sizeof(DWORD));
	pStream->Write((char *)&m_yMapLabel, sizeof(DWORD));
	m_sStargateDestNode.WriteToStream(pStream);
	m_sStargateDestEntryPoint.WriteToStream(pStream);

	if (m_pArmorClass)
		dwSave = m_pArmorClass->GetUNID();
	else
		dwSave = 0xffffffff;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	pStream->Write((char *)&m_iHitPoints, sizeof(DWORD));
	pStream->Write((char *)&m_iStructuralHP, sizeof(DWORD));
	pStream->Write((char *)&m_iMaxStructuralHP, sizeof(DWORD));

	dwSave = (m_pDevices ? maxDevices : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	for (i = 0; i < (int)dwSave; i++)
		m_pDevices[i].WriteToStream(pStream);

	m_RegisteredObjects.WriteToStream(GetSystem(), pStream);

	m_DockingPorts.WriteToStream(this, pStream);
	m_Subordinates.WriteToStream(GetSystem(), pStream);
	m_Targets.WriteToStream(GetSystem(), pStream);

	m_Blacklist.WriteToStream(pStream);
	pStream->Write((char *)&m_iAngryCounter, sizeof(DWORD));
	pStream->Write((char *)&m_iReinforceRequestCount, sizeof(DWORD));
	pStream->Write((char *)&m_iBalance, sizeof(DWORD));

	pStream->Write((char *)&m_dwWreckUNID, sizeof(DWORD));

	dwSave = 0;
	dwSave |= (m_fArmed ?		0x00000001 : 0);
	dwSave |= (m_fKnown ?		0x00000002 : 0);
	dwSave |= (m_fNoMapLabel ?	0x00000004 : 0);
	dwSave |= (m_fRadioactive ?	0x00000008 : 0);
	dwSave |= (m_fCustomImage ?	0x00000010 : 0);
	dwSave |= (m_fActive ?		0x00000020 : 0);
	dwSave |= (m_fNoReinforcements ? 0x00000040 : 0);
	dwSave |= (m_fReconned ?	0x00000080 : 0);
	dwSave |= (m_fFireReconEvent ? 0x00000100 : 0);
	dwSave |= (m_fNoArticle ?	0x00000200 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

void CStation::PaintLRS (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans)

//	PaintLRS
//
//	Paints the object on an LRS

	{
	//	Paint worlds and stars fully

	if (m_Scale == scaleWorld || m_Scale == scaleStar)
		{
		Dest.ColorTransBlt(0, 0, m_MapImage.GetWidth(), m_MapImage.GetHeight(), 255,
				m_MapImage,
				x - (m_MapImage.GetWidth() / 2),
				y - (m_MapImage.GetHeight() / 2));
		}

	//	Other kinds of stations are just dots

	else
		{
		//	Paint red if enemy, green otherwise

		COLORREF wColor;
		if (IsWreck())
			wColor = CGImage::RGBColor(64, 128, 64);
		else if (IsEnemy(GetUniverse()->GetPOV()))
			wColor = CGImage::RGBColor(255, 0, 0);
		else
			wColor = CGImage::RGBColor(0, 192, 0);

		if (m_Scale == scaleStructure && m_rMass > 100000.0)
			{
			if (IsStargate())
				{
				Dest.DrawDot(x, y, wColor, CG16bitImage::markerSmallSquare);
				Dest.DrawDot(x, y, wColor, CG16bitImage::markerMediumCross);
				}
			else if (!IsAbandoned() || m_pType->IsImmutable())
				{
				Dest.DrawDot(x+1, y+1, 0, CG16bitImage::markerSmallSquare);
				Dest.DrawDot(x, y, wColor, CG16bitImage::markerSmallFilledSquare);
				}
			else
				{
				Dest.DrawDot(x+1, y+1, 0, CG16bitImage::markerSmallSquare);
				Dest.DrawDot(x, y, wColor, CG16bitImage::markerSmallSquare);
				}
			}
		else
			Dest.DrawDot(x, y, 
					wColor, 
					CG16bitImage::markerSmallRound);
		}
	}

void CStation::PaintMap (CG16bitImage &Dest, int x, int y, const ViewportTransform &Trans)

//	PaintMap
//
//	Paint the station

	{
	//	Draw an orbit

	if (m_pMapOrbit)
		m_pMapOrbit->Paint(Dest, Trans, RGB_ORBIT_LINE);

	//	Draw the station

	if (m_Scale == scaleWorld || m_Scale == scaleStar)
		{
		Dest.Blt(x - (m_MapImage.GetWidth() / 2), y - (m_MapImage.GetHeight() / 2),
				m_MapImage, 0, 0, m_MapImage.GetWidth(), m_MapImage.GetHeight(), true);
		}
	else if (m_pType->ShowsMapIcon() && m_fKnown)
		{
		//	Figure out the color

		COLORREF wColor;
		if (IsEnemy(GetUniverse()->GetPOV()))
			wColor = CGImage::RGBColor(255, 0, 0);
		else
			wColor = CGImage::RGBColor(0, 192, 0);

		//	Paint the marker

		if (m_Scale == scaleStructure && m_rMass > 100000.0)
			{
			if (IsStargate())
				{
				Dest.DrawDot(x, y, wColor, CG16bitImage::markerSmallSquare);
				Dest.DrawDot(x, y, wColor, CG16bitImage::markerMediumCross);
				}
			else if (!IsAbandoned() || m_pType->IsImmutable())
				{
				Dest.DrawDot(x+1, y+1, 0, CG16bitImage::markerSmallSquare);
				Dest.DrawDot(x, y, wColor, CG16bitImage::markerSmallFilledSquare);
				}
			else
				{
				Dest.DrawDot(x+1, y+1, 0, CG16bitImage::markerSmallSquare);
				Dest.DrawDot(x, y, wColor, CG16bitImage::markerSmallSquare);
				}
			}
		else
			Dest.DrawDot(x, y, 
					wColor, 
					CG16bitImage::markerSmallRound);

		//	Paint the label

		if (!m_fNoMapLabel)
			{
			g_pUniverse->GetMapLabelFont().DrawText(Dest, 
					x + m_xMapLabel + 1, 
					y + m_yMapLabel + 1, 
					0,
					m_sName);
			g_pUniverse->GetMapLabelFont().DrawText(Dest, 
					x + m_xMapLabel, 
					y + m_yMapLabel, 
					RGB_MAP_LABEL,
					m_sName);
			}
		}
	}

bool CStation::PointInObject (CVector vPos)

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

	int iTick, iVariant;
	const CObjectImageArray &Image = GetImage(&iTick, &iVariant);

	return Image.PointInImage(x, y, iTick, iVariant);
	}

void CStation::RaiseAlert (CSpaceObject *pTarget)

//	RaiseAlert
//
//	Raise an alert (if we're that kind of station)

	{
	if (pTarget == NULL)
		return;
	
	if (m_Targets.FindObj(pTarget))
		return;

	//	Add this to our target list

	m_Targets.Add(pTarget);

#ifdef DEBUG_ALERTS
	g_pUniverse->DebugOutput("%d: Raising alert...", this);
#endif

	//	Tell all other friendly stations in the system that they
	//	should attack the target.

	CSovereign *pSovereign = GetSovereign();
	for (int i = 0; i < GetSystem()->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = GetSystem()->GetObject(i);

		if (pObj 
				&& pObj->GetCategory() == catStation
				&& pObj->GetSovereign() == pSovereign
				&& pObj->CanAttack()
				&& pObj != this)
			Communicate(pObj, msgAttack, pTarget);
		}
	}

bool CStation::RequestDock (CSpaceObject *pObj)

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

	if (!IsAbandoned() 
			&& !m_pType->IsEnemyDockingAllowed()
			&& (IsEnemy(pObj) || IsBlacklisted(pObj)))
		{
		pObj->SendMessage(this, CONSTLIT("Docking request denied"));
		return false;
		}

	//	If we don't have any docking screens then do not let the
	//	object dock.

	if (GetDockScreen() == NULL)
		{
		pObj->SendMessage(this, CONSTLIT("No docking services available"));
		return false;
		}

	//	Get the nearest free port

	return m_DockingPorts.RequestDock(this, pObj);
	}

bool CStation::RequestGate (CSpaceObject *pObj)

//	RequestGate
//
//	Requests that the given object be transported through the gate

	{
	//	Create gating effect

	CEffectCreator *pEffect = m_pType->GetGateEffect();
	if (pEffect)
		pEffect->CreateEffect(GetSystem(),
				NULL,
				GetPos(),
				GetVel());

	//	Get the destination node for this gate

	CTopologyNode *pNode = g_pUniverse->FindTopologyNode(m_sStargateDestNode);
	if (pNode == NULL)
		return false;

	//	Let the object gate itself

	pObj->EnterGate(pNode, m_sStargateDestEntryPoint, this);

	return true;
	}

void CStation::SetAngry (void)

//	SetAngry
//
//	Station is angry

	{
	if (m_iAngryCounter < MAX_ANGER)
		m_iAngryCounter = Max(MIN_ANGER, m_iAngryCounter + ANGER_INC);
	}

void CStation::SetMapOrbit (const CSystem::Orbit &oOrbit)

//	SetMapOrbit
//
//	Sets the orbit description

	{
	if (m_pMapOrbit)
		delete m_pMapOrbit;

	m_pMapOrbit = new CSystem::Orbit(oOrbit);
	}

void CStation::SetName (const CString &sName, DWORD dwFlags)

//	SetName
//
//	Sets the name of the station

	{
	m_sName = sName;
	if (dwFlags & nounNoArticle)
		m_fNoArticle = true;
	}

void CStation::SetStargate (const CString &sDestNode, const CString &sDestEntryPoint)

//	SetStargate
//
//	Sets the stargate label

	{
	m_sStargateDestNode = sDestNode;
	m_sStargateDestEntryPoint = sDestEntryPoint;
	}

void CStation::SetWreckImage (DWORD dwWreckUNID, const CObjectImageArray &Image, int iVariants)

//	SetImage
//
//	Sets the image for the station

	{
	m_dwWreckUNID = dwWreckUNID;
	m_Image = Image;
	if (iVariants > 1)
		m_iVariant = mathRandom(0, iVariants-1);
	else
		m_iVariant = 0;
	m_fCustomImage = true;
	}

void CStation::Undock (CSpaceObject *pObj)

//	Undock
//
//	Undocks from the station

	{
	m_DockingPorts.Undock(this, pObj);

	//	If we're set to destroy when empty AND we're empty
	//	AND no one else is docked, then destroy the station

	if (m_pType->IsDestroyWhenEmpty() 
			&& GetItemList().GetCount() == 0
			&& m_DockingPorts.GetPortsInUseCount(this) == 0)
		{
		Destroy(removedFromSystem, NULL);
		}
	}

void CStation::OnItemsModified (void)

//	OnItemsModified
//
//	Items on the station have been modified by some external
//	factor (not the player)

	{
	//	Tell all the ships that are docked here that items have been modified

	m_DockingPorts.OnOwnerChanged(this);
	}

void CStation::OnUpdate (Metric rSecondsPerTick)

//	OnUpdate
//
//	Update

	{
	int i;
	int iTick = GetSystem()->GetTick() + GetDestiny();

	//	Basic update

	UpdateAttacking(iTick);
	m_DockingPorts.UpdateAll(this);
	UpdateReinforcements(iTick);

	//	Trade

	CTradingDesc *pTrade = m_pType->GetTradingDesc();
	if (pTrade && (iTick % TRADE_UPDATE_FREQUENCY) == 0)
		pTrade->OnUpdate(this);

	//	Update each device

	if (m_pDevices)
		{
		bool bSourceDestroyed = false;
		for (i = 0; i < maxDevices; i++)
			{
			m_pDevices[i].Update(this, iTick, &bSourceDestroyed);
			if (bSourceDestroyed)
				return;
			}
		}

	//	Update destroy animation

	if (m_iDestroyedAnimation)
		{
		int iTick, iRotation;
		const CObjectImageArray &Image = GetImage(&iTick, &iRotation);
		int cxWidth = RectWidth(Image.GetImageRect());

		CEffectCreator *pEffect = g_pUniverse->FindEffectType(g_StationDestroyedUNID);
		if (pEffect)
			{
			for (int i = 0; i < mathRandom(1, 3); i++)
				{
				CVector vPos = GetPos() 
						+ PolarToVector(mathRandom(0, 359), g_KlicksPerPixel * mathRandom(1, cxWidth / 3));

				pEffect->CreateEffect(GetSystem(),
						this,
						vPos,
						GetVel());
				}
			}

		m_iDestroyedAnimation--;
		}

	//	If we're moving, slow down

	if (IsMobile() && !GetVel().IsNull())
		{
		//	If we're moving really slowly, force to 0. We do this so that we can optimize calculations
		//	and not have to compute wreck movement down to infinitesimal distances.

		if (GetVel().Length2() < g_MinSpeed2)
			SetVel(NullVector);
		else
			SetVel(CVector(GetVel().GetX() * g_SpaceDragFactor, GetVel().GetY() * g_SpaceDragFactor));
		}
	}

void CStation::UpdateAttacking (int iTick)

//	UpdateAttacking
//
//	Station attacks any enemies in range

	{
	int i;
	
	//	Update blacklist counter
	//	NOTE: Once the player is blacklisted by this station, there is
	//	no way to get off the blacklist. (At least no automatic way).

	m_Blacklist.Update(iTick);

	//	If we're abandoned or if we have no weapons then
	//	there's nothing we can do

	if (IsAbandoned() || !m_fArmed)
		return;

	//	Compute the range at which we attack enemies

	Metric rAttackRange;
	if (m_iAngryCounter > 0)
		{
		rAttackRange = Max(MAX_ATTACK_DISTANCE, m_pType->GetMaxEffectiveRange());
		m_iAngryCounter--;
		}
	else
		rAttackRange = MAX_ATTACK_DISTANCE;

	//	Look for the nearest enemy ship to attack

	if ((iTick % STATION_SCAN_TARGET_FREQUENCY) == 0)
		{
		//	Look for a target

		m_pTarget = NULL;
		Metric rBestDist = rAttackRange * rAttackRange;
		CSystem *pSystem = GetSystem();
		for (i = 0; i < pSystem->GetObjectCount(); i++)
			{
			CSpaceObject *pObj = pSystem->GetObject(i);

			if (pObj
					&& pObj->GetCategory() == catShip
					&& (IsEnemy(pObj) || IsBlacklisted(pObj))
					&& pObj->CanAttack()
					&& pObj != this)
				{
				CVector vDist = pObj->GetPos() - GetPos();
				Metric rDist = vDist.Length2();

				if (rDist < rBestDist
						&& !pObj->IsEscortingFriendOf(this))
					{
					rBestDist = rDist;
					m_pTarget = pObj;
					}
				}
			}
		}

	//	Fire with all weapons (if we've got a target)

	if (m_pTarget && m_pDevices)
		{
		bool bSourceDestroyed = false;

		for (i = 0; i < maxDevices; i++)
			{
			CInstalledDevice *pWeapon = &m_pDevices[i];
			int iFireAngle;

			if (!pWeapon->IsEmpty() 
					&& pWeapon->GetCategory() == itemcatWeapon
					&& pWeapon->IsReady()
					&& pWeapon->IsWeaponAligned(this, m_pTarget, NULL, &iFireAngle)
					&& IsLineOfFireClear(pWeapon->GetPos(this), m_pTarget, iFireAngle, rAttackRange))
				{
				pWeapon->SetFireAngle(iFireAngle);
				pWeapon->Activate(this, m_pTarget, iFireAngle, &bSourceDestroyed);
				if (bSourceDestroyed)
					return;

				pWeapon->SetActivationDelay(m_pType->GetFireRateAdj() * pWeapon->GetActivateDelay(this) / 10);
				}
			}
		}
	}

void CStation::UpdateReinforcements (int iTick)

//	UpdateReinforcements
//
//	Check to see if it is time to send reinforcements to the station

	{
	//	Nothing to do if we're abandoned

	if (IsAbandoned())
		return;

	//	Repair damage

	if ((iTick % STATION_REPAIR_FREQUENCY) == 0)
		{
		//	Repair damage to station

		if (m_pType->GetRepairRate() > 0 && m_iHitPoints < m_pType->GetMaxHitPoints())
			m_iHitPoints = std::min(m_pType->GetMaxHitPoints(), m_iHitPoints + m_pType->GetRepairRate());

		//	Repair damage to ships

		m_DockingPorts.RepairAll(this, m_pType->GetShipRepairRate());
		}

	//	Construction

	if (m_pType->GetShipConstructionRate()
			&& (iTick % m_pType->GetShipConstructionRate()) == 0)
		{
		//	Iterate over all ships and count the number that are
		//	associated with the station.

		int iCount = CalcNumberOfShips();

		//	If we already have the maximum number, then don't bother

		if (iCount < m_pType->GetMaxShipConstruction())
			{
			SShipCreateCtx Ctx;
			Ctx.pSystem = GetSystem();
			Ctx.pBase = this;
			Ctx.pGate = this;
			m_pType->GetConstructionTable()->CreateShips(Ctx);
			}
		}

	//	Get reinforcements

	if ((iTick % STATION_REINFORCEMENT_FREQUENCY) == 0
			&& m_pType->GetMinShips() > 0
			&& !m_fNoReinforcements)
		{
		//	Iterate over all ships and count the number that are
		//	associated with the station.

		int iCount = CalcNumberOfShips();

		//	If we don't have the minimum number of ships at the
		//	station then send reinforcements.

		if (iCount < m_pType->GetMinShips())
			{
			//	If we've requested several rounds of reinforcements but have
			//	never received any, then it's likely that they are being
			//	destroyed at the gate, so we stop requesting so many

			if (m_iReinforceRequestCount > 0)
				{
				int iLongTick = (iTick / STATION_REINFORCEMENT_FREQUENCY);
				int iCycle = Min(32, m_iReinforceRequestCount * m_iReinforceRequestCount);
				if ((iLongTick % iCycle) != 0)
					return;
				}

			//	Find a stargate

			CSpaceObject *pGate = GetNearestStargate(true);
			if (pGate == NULL)
				return;

			//	Generate reinforcements

			SShipCreateCtx Ctx;
			Ctx.pSystem = GetSystem();
			Ctx.pBase = this;
			Ctx.pGate = pGate;
			m_pType->GetReinforcementsTable()->CreateShips(Ctx);

			//	Increment counter

			m_iReinforceRequestCount++;
			}

		//	If we have the required number of ships, then reset the reinforcement
		//	request count

		else
			{
			m_iReinforceRequestCount = 0;
			}
		}

	//	Attack targets on the target list

	if ((iTick % STATION_TARGET_FREQUENCY) == 0)
		{
		int i;

#ifdef DEBUG_ALERTS
		g_pUniverse->DebugOutput("%d: Attack target list", this);
#endif

		for (i = 0; i < m_Targets.GetCount(); i++)
			{
			CSpaceObject *pTarget = m_Targets.GetObj(i);

			int iLeft = mathRandom(3, 5);
			int iCount = m_Subordinates.GetCount();
			for (int j = 0; j < iCount && iLeft > 0; j++)
				{
				if (Communicate(m_Subordinates.GetObj(j), 
						msgDestroyBroadcast, 
						pTarget) == resAck)
					{
					iLeft--;
#ifdef DEBUG_ALERTS
					g_pUniverse->DebugOutput("   %d acknowledges attack order", m_Subordinates.GetObj(i));
#endif
					}
				}
			}
		}
	}
