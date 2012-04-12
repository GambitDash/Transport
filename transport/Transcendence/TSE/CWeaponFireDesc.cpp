//	CWeaponFireDesc.cpp
//
//	CWeaponFireDesc class

#include "PreComp.h"

#define FRAGMENT_TAG					CONSTLIT("Fragment")
#define ENHANCED_TAG					CONSTLIT("Enhanced")
#define IMAGE_TAG						CONSTLIT("Image")
#define MISSILE_EXHAUST_TAG				CONSTLIT("Exhaust")
#define EFFECT_TAG						CONSTLIT("Effect")
#define HIT_EFFECT_TAG					CONSTLIT("HitEffect")
#define FIRE_EFFECT_TAG					CONSTLIT("FireEffect")

#define FIRE_TYPE_ATTRIB				CONSTLIT("type")
#define BEAM_TYPE_ATTRIB				CONSTLIT("beamType")
#define BEAM_CONTINUOUS_ATTRIB			CONSTLIT("repeating")
#define AMMO_ID_ATTRIB					CONSTLIT("ammoID")
#define DIRECTIONAL_ATTRIB				CONSTLIT("directional")
#define TRACKING_ATTRIB					CONSTLIT("tracking")
#define FRAGMENT_COUNT_ATTRIB			CONSTLIT("fragmentCount")
#define HIT_EFFECT_ATTRIB				CONSTLIT("hitEffect")
#define FIRE_EFFECT_ATTRIB				CONSTLIT("fireEffect")
#define EFFECT_ATTRIB					CONSTLIT("effect")
#define AREA_EFFECT_ATTRIB				CONSTLIT("areaEffect")
#define TRAIL_ATTRIB					CONSTLIT("trail")
#define DAMAGE_ATTRIB					CONSTLIT("damage")
#define PASSTHROUGH_ATTRIB				CONSTLIT("passthrough")
#define EXHAUST_RATE_ATTRIB				CONSTLIT("creationRate")
#define EXHAUST_LIFETIME_ATTRIB			CONSTLIT("lifetime")
#define EXHAUST_DRAG_ATTRIB				CONSTLIT("drag")
#define PRIMARY_COLOR_ATTRIB			CONSTLIT("primaryColor")
#define SECONDARY_COLOR_ATTRIB			CONSTLIT("secondaryColor")
#define SOUND_ATTRIB					CONSTLIT("sound")
#define INTENSITY_ATTRIB				CONSTLIT("intensity")
#define LIFETIME_ATTRIB					CONSTLIT("lifetime")
#define MISSILE_SPEED_ATTRIB			CONSTLIT("missileSpeed")
#define EXPANSION_SPEED_ATTRIB			CONSTLIT("expansionSpeed")
#define PARTICLE_COUNT_ATTRIB			CONSTLIT("particleCount")
#define PARTICLE_EMIT_TIME_ATTRIB		CONSTLIT("particleEmitTime")
#define PARTICLE_SPREAD_ANGLE_ATTRIB	CONSTLIT("particleSpreadAngle")
#define PARTICLE_SPREAD_WIDTH_ATTRIB	CONSTLIT("particleSpreadWidth")
#define MANEUVERABILITY_ATTRIB			CONSTLIT("maneuverability")
#define ACCELERATION_FACTOR_ATTRIB		CONSTLIT("accelerationFactor")
#define MAX_MISSILE_SPEED_ATTRIB		CONSTLIT("maxMissileSpeed")
#define FAILSAFE_ATTRIB					CONSTLIT("failsafe")
#define VAPOR_TRAIL_ATTRIB				CONSTLIT("vaporTrail")
#define VAPOR_TRAIL_WIDTH_ATTRIB		CONSTLIT("vaporTrailWidth")
#define VAPOR_TRAIL_COLOR_ATTRIB		CONSTLIT("vaporTrailColor")
#define VAPOR_TRAIL_LENGTH_ATTRIB		CONSTLIT("vaporTrailLength")
#define VAPOR_TRAIL_WIDTH_INC_ATTRIB	CONSTLIT("vaporTrailWidthInc")
#define CAN_HIT_SOURCE_ATTRIB			CONSTLIT("canHitSource")
#define FRAGMENT_TARGET_ATTRIB			CONSTLIT("fragmentTarget")
#define COUNT_ATTRIB					CONSTLIT("count")
#define MULTI_TARGET_ATTRIB				CONSTLIT("multiTarget")
#define MIN_RADIUS_ATTRIB				CONSTLIT("minRadius")
#define MAX_RADIUS_ATTRIB				CONSTLIT("maxRadius")
#define AUTO_TARGET_ATTRIB				CONSTLIT("autoAcquireTarget")
#define STEALTH_ATTRIB					CONSTLIT("stealth")
#define INITIAL_DELAY_ATTRIB			CONSTLIT("initialDelay")
#define HIT_POINTS_ATTRIB				CONSTLIT("hitPoints")
#define INTERACTION_ATTRIB				CONSTLIT("interaction")

#define FIRE_TYPE_BEAM					CONSTLIT("beam")
#define FIRE_TYPE_MISSILE				CONSTLIT("missile")
#define FIRE_TYPE_AREA					CONSTLIT("area")
#define FIRE_TYPE_PARTICLES				CONSTLIT("particles")
#define FIRE_TYPE_RADIUS				CONSTLIT("radius")

#define BEAM_TYPE_LASER					CONSTLIT("laser")
#define BEAM_TYPE_LIGHTNING				CONSTLIT("lightning")
#define BEAM_TYPE_HEAVY_BLASTER			CONSTLIT("heavyblaster")
#define BEAM_TYPE_STAR_BLASTER			CONSTLIT("starblaster")

#define BEAM_TYPE_GREEN_PARTICLE		CONSTLIT("greenparticle")
#define BEAM_TYPE_BLUE_PARTICLE			CONSTLIT("blueparticle")
#define BEAM_TYPE_BLASTER				CONSTLIT("blaster")
#define BEAM_TYPE_GREEN_LIGHTNING		CONSTLIT("greenlightning")

CWeaponFireDesc::CWeaponFireDesc (void) : m_pEnhanced(NULL)

//	CWeaponFireDesc constructor

	{
	}

CWeaponFireDesc::CWeaponFireDesc (const CWeaponFireDesc &Desc)

//	CWeaponFireDesc constructor

	{
	*this = Desc;

	//	Fragments

	SFragmentDesc *pNext = Desc.m_pFirstFragment;
	SFragmentDesc *pPrev = NULL;
	while (pNext)
		{
		SFragmentDesc *pNew = new SFragmentDesc(*pNext);
		pNew->pDesc = new CWeaponFireDesc(*(pNext->pDesc));
		
		if (pPrev == NULL)
			m_pFirstFragment = pNew;
		else
			pPrev->pNext = pNew;

		pPrev = pNew;
		}
	if (pPrev)
		pPrev->pNext = NULL;

	//	Other

	if (Desc.m_pEnhanced)
		m_pEnhanced = new CWeaponFireDesc(*Desc.m_pEnhanced);
	}

CWeaponFireDesc::~CWeaponFireDesc (void)

//	CWeaponFireDesc destructor

	{
	SFragmentDesc *pNext = m_pFirstFragment;
	while (pNext)
		{
		SFragmentDesc *pDelete = pNext;
		pNext = pNext->pNext;
		delete pDelete->pDesc;
		delete pDelete;
		}

	if (m_pEnhanced)
		delete m_pEnhanced;
	}

CEffectCreator *CWeaponFireDesc::FindEffectCreator (const CString &sUNID)

//	FindEffectCreator
//
//	Finds effect creator from a partial UNID

	{
	char *pPos;

	//	Get the appropriate weapon fire desc and the parse position

	CWeaponFireDesc *pDesc = FindWeaponFireDesc(sUNID, &pPos);
	if (pDesc == NULL)
		return NULL;
	
	//	Parse the effect

	if (*pPos == ':')
		{
		pPos++;

		switch (*pPos)
			{
			case 'e':
				return pDesc->m_pEffect;

			case 'h':
				return pDesc->m_pHitEffect;

			case 'f':
				return pDesc->m_pFireEffect;

			default:
				return NULL;
			}
		}
	else
		return NULL;
	}

CWeaponFireDesc *CWeaponFireDesc::FindWeaponFireDesc (const CString &sUNID, char **retpPos)

//	FindWeaponFireDesc
//
//	Finds the weapon fire desc from a partial UNID

	{
	char *pPos = sUNID.GetASCIIZPointer();

	//	If we're done, then we want this descriptor

	if (*pPos == '\0')
		return this;

	//	If a slash, then we have another weapon fire desc

	else if (*pPos == '/')
		{
		pPos++;

		//	The enhanced section

		if (*pPos == 'e')
			{
			pPos++;
			if (m_pEnhanced)
				return m_pEnhanced->FindWeaponFireDesc(CString(pPos), retpPos);
			else
				return NULL;
			}

		//	A fragment

		else if (*pPos == 'f')
			{
			pPos++;

			int iIndex = strParseInt(pPos, 0, &pPos, NULL);

			SFragmentDesc *pNext = m_pFirstFragment;
			while (iIndex-- > 0 && pNext)
				pNext = pNext->pNext;

			if (pNext)
				return pNext->pDesc->FindWeaponFireDesc(CString(pPos), retpPos);
			else
				return NULL;
			}
		else
			return NULL;
		}

	//	Otherwise, we have an effect

	else if (*pPos == ':')
		{
		if (retpPos)
			*retpPos = pPos;

		return this;
		}
	else
		return NULL;
	}

CWeaponFireDesc *CWeaponFireDesc::FindWeaponFireDesc (DWORD dwUNID, int iOrdinal)

//	FindWeaponFireDesc
//
//	Finds the descriptor by UNID

	{
	CDeviceClass *pDevice = g_pUniverse->FindDeviceClass(dwUNID);
	if (pDevice == NULL)
		return NULL;

	CWeaponClass *pWeapon = pDevice->AsWeaponClass();
	if (pWeapon == NULL)
		return NULL;

	if (iOrdinal >= pWeapon->GetVariantCount())
		return NULL;

	return pWeapon->GetVariant(iOrdinal);
	}

CWeaponFireDesc *CWeaponFireDesc::FindWeaponFireDescFromFullUNID (const CString &sUNID)

//	FindWeaponFireDesc
//
//	Finds the descriptor by name

	{
	char *pPos = sUNID.GetPointer();

	//	Get the weapon UNID and the ordinal

	DWORD dwUNID = (DWORD)strParseInt(pPos, 0, &pPos, NULL);
	ASSERT(*pPos == '/');
	pPos++;
	int iOrdinal = strParseInt(pPos, 0, &pPos, NULL);

	//	Get the weapon descriptor

	CWeaponClass *pClass = (CWeaponClass *)g_pUniverse->FindDeviceClass(dwUNID);
	ASSERT(pClass);
	ASSERT(iOrdinal < pClass->GetVariantCount());
	CWeaponFireDesc *pDesc = pClass->GetVariant(iOrdinal);

	//	Continue parsing

	return pDesc->FindWeaponFireDesc(CString(pPos));
	}

Metric CWeaponFireDesc::GetInitialSpeed (void) const

//	GetInitialSpeed
//
//	Returns the initial speed of the missile (when launched)

	{
	if (m_fVariableInitialSpeed)
		return (double)m_MissileSpeed.Roll() * LIGHT_SPEED / 100;
	else
		return GetRatedSpeed();
	}

ALERROR CWeaponFireDesc::InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc, const CString &sUNID)

//	InitFromXML
//
//	Loads shot data from an element

	{
	ALERROR error;
	int i;

	m_fVariableInitialSpeed = false;
	m_bFragment = false;

	//	Load basic attributes

	m_sUNID = sUNID;
	m_Lifetime.LoadFromXML(pDesc->GetAttribute(LIFETIME_ATTRIB));
	int iMaxLifetime = m_Lifetime.GetMaxValue();
	m_bCanDamageSource = pDesc->GetAttributeBool(CAN_HIT_SOURCE_ATTRIB);
	m_bAutoTarget = pDesc->GetAttributeBool(AUTO_TARGET_ATTRIB);
	m_InitialDelay.LoadFromXML(pDesc->GetAttribute(INITIAL_DELAY_ATTRIB));

	//	Load missile speed

	CString sData;
	if (pDesc->FindAttribute(MISSILE_SPEED_ATTRIB, &sData))
		{
		if (error = m_MissileSpeed.LoadFromXML(sData))
			{
			Ctx.sError = CONSTLIT("Invalid missile speed attribute");
			return ERR_FAIL;
			}

		m_fVariableInitialSpeed = !m_MissileSpeed.IsConstant();
		m_rMissileSpeed = (double)m_MissileSpeed.GetAveValue() * LIGHT_SPEED / 100;
		}
	else
		{
		m_fVariableInitialSpeed = false;
		m_rMissileSpeed = LIGHT_SPEED;
		}

	//	Load the effect to use

	if (error = m_pEffect.LoadEffect(Ctx, 
			strPatternSubst("%s:e", sUNID.GetASCIIZPointer()),
			pDesc->GetContentElementByTag(EFFECT_TAG),
			pDesc->GetAttribute(EFFECT_ATTRIB)))
		return error;

	//	The effect should have the same lifetime as the shot

	if (m_pEffect)
		{
		m_pEffect->SetLifetime(iMaxLifetime);
		}

	//	Load stealth

	m_iStealth = pDesc->GetAttributeInteger(STEALTH_ATTRIB);
	if (m_iStealth == 0)
		m_iStealth = CSpaceObject::stealthNormal;

	//	Load specific properties

	CString sValue = pDesc->GetAttribute(FIRE_TYPE_ATTRIB);
	if (strEquals(sValue, FIRE_TYPE_BEAM))
		{
		m_iFireType = ftBeam;

		sValue = pDesc->GetAttribute(BEAM_TYPE_ATTRIB);
		if (strEquals(sValue, BEAM_TYPE_LASER))
			m_iBeamType = beamLaser;
		else if (strEquals(sValue, BEAM_TYPE_LIGHTNING))
			m_iBeamType = beamLightning;
		else if (strEquals(sValue, BEAM_TYPE_HEAVY_BLASTER))
			m_iBeamType = beamHeavyBlaster;
		else if (strEquals(sValue, BEAM_TYPE_GREEN_PARTICLE))
			m_iBeamType = beamGreenParticle;
		else if (strEquals(sValue, BEAM_TYPE_BLUE_PARTICLE))
			m_iBeamType = beamBlueParticle;
		else if (strEquals(sValue, BEAM_TYPE_BLASTER))
			m_iBeamType = beamBlaster;
		else if (strEquals(sValue, BEAM_TYPE_STAR_BLASTER))
			m_iBeamType = beamStarBlaster;
		else if (strEquals(sValue, BEAM_TYPE_GREEN_LIGHTNING))
			m_iBeamType = beamGreenLightning;
		else
			{
			Ctx.sError = CONSTLIT("Invalid weapon beam type");
			return ERR_FAIL;
			}

		m_wPrimaryColor = LoadRGBColor(pDesc->GetAttribute(PRIMARY_COLOR_ATTRIB));
		m_wSecondaryColor = LoadRGBColor(pDesc->GetAttribute(SECONDARY_COLOR_ATTRIB));
		m_iIntensity = pDesc->GetAttributeInteger(INTENSITY_ATTRIB);

		m_rMaxMissileSpeed = LIGHT_SPEED;
		m_iManeuverability = 0;

		//	We optionally use an image for the head of the beam

		CXMLElement *pImage = pDesc->GetContentElementByTag(IMAGE_TAG);
		if (pImage)
			if (error = m_Image.InitFromXML(Ctx, pImage))
				return error;
		}
	else if (strEquals(sValue, FIRE_TYPE_MISSILE))
		{
		m_iFireType = ftMissile;

		//	Load the image for the missile

		CXMLElement *pImage = pDesc->GetContentElementByTag(IMAGE_TAG);
		if (pImage)
			if (error = m_Image.InitFromXML(Ctx, pImage))
				return error;

		m_bDirectional = pDesc->GetAttributeBool(DIRECTIONAL_ATTRIB);
		if (m_bDirectional && m_pEffect)
			m_pEffect->SetVariants(g_RotationRange);

		m_iManeuverability = pDesc->GetAttributeInteger(MANEUVERABILITY_ATTRIB);
		m_iAccelerationFactor = pDesc->GetAttributeInteger(ACCELERATION_FACTOR_ATTRIB);
		int iMaxSpeed = pDesc->GetAttributeInteger(MAX_MISSILE_SPEED_ATTRIB);
		if (iMaxSpeed == 0)
			m_rMaxMissileSpeed = m_rMissileSpeed;
		else
			m_rMaxMissileSpeed = (Metric)iMaxSpeed * LIGHT_SPEED / 100.0;

		//	Hit points and interaction

		m_iHitPoints = pDesc->GetAttributeInteger(HIT_POINTS_ATTRIB);
		CString sInteraction;
		if (pDesc->FindAttribute(INTERACTION_ATTRIB, &sInteraction))
			m_iInteraction = strToInt(sInteraction, 100);
		else
			m_iInteraction = 100;

		//	Load exhaust data

		CXMLElement *pExhaust = pDesc->GetContentElementByTag(MISSILE_EXHAUST_TAG);
		if (pExhaust)
			{
			m_iExhaustRate = pExhaust->GetAttributeInteger(EXHAUST_RATE_ATTRIB);
			m_iExhaustLifetime = pExhaust->GetAttributeInteger(EXHAUST_LIFETIME_ATTRIB);
			m_rExhaustDrag = pExhaust->GetAttributeInteger(EXHAUST_DRAG_ATTRIB) / 100.0;

			CXMLElement *pImage = pExhaust->GetContentElementByTag(IMAGE_TAG);
			if (error = m_ExhaustImage.InitFromXML(Ctx, pImage))
				return error;
			}
		else
			{
			m_iExhaustRate = 0;
			m_iExhaustLifetime = 0;
			m_rExhaustDrag = 0.0;
			}
		}
	else if (strEquals(sValue, FIRE_TYPE_AREA))
		{
		m_iFireType = ftArea;

		m_rMaxMissileSpeed = m_rMissileSpeed;
		m_rExpansionSpeed = (double)pDesc->GetAttributeInteger(EXPANSION_SPEED_ATTRIB) * LIGHT_SPEED / 100;
		m_iManeuverability = 0;

		//	Initialize the effect based on the properties of the damage.
		//	NOTE: The effect must not be shared with other damages.

		if (m_pEffect == NULL)
			{
			Ctx.sError = CONSTLIT("Must have <Effect> for area damage.");
			return ERR_FAIL;
			}

		m_pEffect->SetSpeed((int)((m_rExpansionSpeed * g_SecondsPerUpdate / g_KlicksPerPixel) + 0.5));
		}
	else if (strEquals(sValue, FIRE_TYPE_PARTICLES))
		{
		m_iFireType = ftParticles;

		m_rMaxMissileSpeed = m_rMissileSpeed;

		if (error = m_ParticleCount.LoadFromXML(pDesc->GetAttribute(PARTICLE_COUNT_ATTRIB)))
			{
			Ctx.sError = CONSTLIT("Invalid particle count.");
			return error;
			}

		if (error = m_ParticleEmitTime.LoadFromXML(pDesc->GetAttribute(PARTICLE_EMIT_TIME_ATTRIB)))
			{
			Ctx.sError = CONSTLIT("Invalid particle emit time.");
			return error;
			}

		m_iParticleSpread = pDesc->GetAttributeInteger(PARTICLE_SPREAD_ANGLE_ATTRIB);
		m_iParticleSpreadWidth = pDesc->GetAttributeInteger(PARTICLE_SPREAD_WIDTH_ATTRIB);
		m_iManeuverability = 0;
		}
	else if (strEquals(sValue, FIRE_TYPE_RADIUS))
		{
		m_iFireType = ftRadius;

		m_rMaxMissileSpeed = m_rMissileSpeed;
		m_iManeuverability = 0;

		m_rMinRadius = LIGHT_SECOND * (Metric)pDesc->GetAttributeInteger(MIN_RADIUS_ATTRIB);
		m_rMaxRadius = LIGHT_SECOND * (Metric)pDesc->GetAttributeInteger(MAX_RADIUS_ATTRIB);
		}
	else
		{
		Ctx.sError = CONSTLIT("Invalid weapon fire type");
		return ERR_FAIL;
		}

	//	We initialize this with the UNID, and later resolve the reference
	//	during OnDesignLoadComplete

	m_pAmmoType.LoadUNID(Ctx, pDesc->GetAttribute(AMMO_ID_ATTRIB));

	m_iContinuous = pDesc->GetAttributeInteger(BEAM_CONTINUOUS_ATTRIB);
	if (pDesc->FindAttributeInteger(PASSTHROUGH_ATTRIB, &m_iPassthrough))
		{
		if (m_iPassthrough == 0)
			m_iPassthrough = 50;
		}
	else
		m_iPassthrough = 0;

	if (error = m_Damage.LoadFromXML(pDesc->GetAttribute(DAMAGE_ATTRIB)))
		{
		Ctx.sError = CONSTLIT("Invalid damage specification");
		return error;
		}

	//	Fragments

	m_pFirstFragment = NULL;
	SFragmentDesc *pLastFragment = NULL;
	int iFragCount = 0;
	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pFragDesc = pDesc->GetContentElement(i);
		if (!strEquals(FRAGMENT_TAG, pFragDesc->GetTag()))
			continue;

		//	Create a new fragmentation descriptor

		SFragmentDesc *pNewDesc = new SFragmentDesc;
		pNewDesc->pNext = NULL;
		if (pLastFragment)
			pLastFragment->pNext = pNewDesc;
		else
			m_pFirstFragment = pNewDesc;

		pLastFragment = pNewDesc;

		//	Load fragment data

		pNewDesc->pDesc = new CWeaponFireDesc;
		CString sFragUNID = strPatternSubst("%s/f%d", sUNID.GetASCIIZPointer(), iFragCount++);
		if (error = pNewDesc->pDesc->InitFromXML(Ctx, pFragDesc, sFragUNID))
			return error;

		pNewDesc->pDesc->m_bFragment = true;

		//	Set the fragment count

		CString sCount = pFragDesc->GetAttribute(COUNT_ATTRIB);
		if (sCount.IsBlank())
			sCount = pDesc->GetAttribute(FRAGMENT_COUNT_ATTRIB);
		pNewDesc->Count.LoadFromXML(sCount);

		//	Set MIRV flag

		pNewDesc->bMIRV = (pFragDesc->GetAttributeBool(MULTI_TARGET_ATTRIB) 
				|| pDesc->GetAttributeBool(FRAGMENT_TARGET_ATTRIB));
		}

	//	If we have fragments, then set proximity appropriately

	if (iFragCount)
		{
		m_bProximityBlast = true;
		m_iProximityFailsafe = pDesc->GetAttributeInteger(FAILSAFE_ATTRIB);
		}
	else
		{
		m_bProximityBlast = false;
		m_iProximityFailsafe = 0;
		}

	//	Compute max effective range

	if (m_iFireType == ftArea)
		m_rMaxEffectiveRange = m_rExpansionSpeed * Ticks2Seconds(iMaxLifetime) * 0.75;
	else
		{
		Metric rEffectiveLifetime;
		if (m_iManeuverability > 0)
			rEffectiveLifetime = Ticks2Seconds(iMaxLifetime) * 0.75;
		else
			rEffectiveLifetime = Min(Ticks2Seconds(iMaxLifetime), 100.0);

		Metric rSpeed = (m_rMissileSpeed + m_rMaxMissileSpeed) / 2;
		m_rMaxEffectiveRange = rSpeed * rEffectiveLifetime;

		//	If we have fragments, add to the effective range

		if (m_pFirstFragment)
			m_rMaxEffectiveRange += m_pFirstFragment->pDesc->m_rMaxEffectiveRange;
		}

	//	Effects

	if (error = m_pHitEffect.LoadEffect(Ctx,
			strPatternSubst("%s:h", sUNID.GetASCIIZPointer()),
			pDesc->GetContentElementByTag(HIT_EFFECT_TAG),
			pDesc->GetAttribute(HIT_EFFECT_ATTRIB)))
		return error;

	if (error = m_pFireEffect.LoadEffect(Ctx,
			strPatternSubst("%s:f", sUNID.GetASCIIZPointer()),
			pDesc->GetContentElementByTag(FIRE_EFFECT_TAG),
			pDesc->GetAttribute(FIRE_EFFECT_ATTRIB)))
		return error;

	//	Vapor trail

	if (!pDesc->FindAttributeInteger(VAPOR_TRAIL_WIDTH_ATTRIB, &m_iVaporTrailWidth))
		m_iVaporTrailWidth = 100 * pDesc->GetAttributeInteger(VAPOR_TRAIL_ATTRIB);

	if (m_iVaporTrailWidth)
		{
		m_wVaporTrailColor = LoadRGBColor(pDesc->GetAttribute(VAPOR_TRAIL_COLOR_ATTRIB));
		m_iVaporTrailLength = pDesc->GetAttributeInteger(VAPOR_TRAIL_LENGTH_ATTRIB);
		if (m_iVaporTrailLength <= 0)
			m_iVaporTrailLength = 64;

		if (!pDesc->FindAttributeInteger(VAPOR_TRAIL_WIDTH_INC_ATTRIB, &m_iVaporTrailWidthInc))
			m_iVaporTrailWidthInc = 25;
		}
	else
		m_iVaporTrailLength = 0;

	//	Sound

	DWORD dwSoundID = LoadUNID(Ctx, pDesc->GetAttribute(SOUND_ATTRIB));
	if (dwSoundID)
		m_iFireSound = g_pUniverse->FindSound(dwSoundID);
	else
		m_iFireSound = -1;

	//	Check to see if this element has an enhanced sub-element. If so, then we
	//	recurse.

	CXMLElement *pEnhanced = pDesc->GetContentElementByTag(ENHANCED_TAG);
	if (pEnhanced)
		{
		m_pEnhanced = new CWeaponFireDesc(*this);
		if (error = m_pEnhanced->OverrideDesc(pEnhanced))
			return error;
		}
	else
		m_pEnhanced = NULL;

	return NOERROR;
	}

void CWeaponFireDesc::LoadImages (void)

//	LoadImages
//
//	Loads images used by weapon

	{
	if (m_pEffect)
		m_pEffect->LoadImages();

	if (m_pHitEffect)
		m_pHitEffect->LoadImages();

	if (m_pFireEffect)
		m_pFireEffect->LoadImages();
	}

void CWeaponFireDesc::MarkImages (void)

//	MarkImages
//
//	Marks images used by weapon

	{
	if (m_pEffect)
		m_pEffect->MarkImages();

	if (m_pHitEffect)
		m_pHitEffect->MarkImages();

	if (m_pFireEffect)
		m_pFireEffect->MarkImages();
	}

ALERROR CWeaponFireDesc::OnDesignLoadComplete (SDesignLoadCtx &Ctx)

//	OnDesignLoadComplete
//
//	Done loading all design elements

	{
	ALERROR error;

	if (error = m_Image.OnDesignLoadComplete(Ctx))
		return error;

	if (error = m_ExhaustImage.OnDesignLoadComplete(Ctx))
		return error;

	if (error = m_pAmmoType.Bind(Ctx))
		return error;

	if (error = m_pEffect.Bind(Ctx))
		return error;

	if (error = m_pHitEffect.Bind(Ctx))
		return error;

	if (error = m_pFireEffect.Bind(Ctx))
		return error;

	if (m_pEnhanced)
		if (error = m_pEnhanced->OnDesignLoadComplete(Ctx))
			return error;

	//	Fragment

	SFragmentDesc *pNext = m_pFirstFragment;
	while (pNext)
		{
		if (error = pNext->pDesc->OnDesignLoadComplete(Ctx))
			return error;

		pNext = pNext->pNext;
		}

	return NOERROR;
	}

ALERROR CWeaponFireDesc::OverrideDesc (CXMLElement *pDesc)

//	OverrideDesc
//
//	Override shot data (used for enhanced weapons)

	{
	ALERROR error;

	//	Damage

	CString sAttrib;
	if (pDesc->FindAttribute(DAMAGE_ATTRIB, &sAttrib))
		{
		if (error = m_Damage.LoadFromXML(sAttrib))
			return error;
		}

	//	Add enhanced UNID

	m_sUNID.Append(CONSTLIT("/e"));

	return NOERROR;
	}

