//	CEnergyFieldType.cpp
//
//	CEnergyFieldType class

#include "PreComp.h"

#define EFFECT_WHEN_HIT_TAG						CONSTLIT("EffectWhenHit")
#define EFFECT_TAG								CONSTLIT("Effect")
#define HIT_EFFECT_TAG							CONSTLIT("HitEffect")

#define UNID_ATTRIB								CONSTLIT("UNID")
#define ABSORB_ADJ_ATTRIB						CONSTLIT("absorbAdj")
#define BONUS_ADJ_ATTRIB						CONSTLIT("weaponBonusAdj")
#define WEAPON_SUPPRESS_ATTRIB					CONSTLIT("weaponSuppress")

#define SUPPRESS_ALL							CONSTLIT("*")

CEnergyFieldType::CEnergyFieldType (void) : 
		m_pEffect(NULL),
		m_pHitEffect(NULL)

//	CEnergyFieldType constructor

	{
	}

CEnergyFieldType::~CEnergyFieldType (void)

//	CEnergyFieldType destructor

	{
	if (m_pEffect)
		delete m_pEffect;

	if (m_pHitEffect)
		delete m_pHitEffect;
	}

bool CEnergyFieldType::AbsorbsWeaponFire (CInstalledDevice *pWeapon)

//	AbsorbsWeaponFire
//
//	Absorbs weapon fire from the ship

	{
	int iType = pWeapon->GetClass()->GetDamageType(pWeapon);
	if (iType != -1 && m_WeaponSuppress.InSet(iType))
		return true;
	else
		return false;
	}

int CEnergyFieldType::GetDamageAbsorbed (CSpaceObject *pSource, SDamageCtx &Ctx)

//	GetDamageAbsorbed
//
//	Returns the amount of damage absorbed

	{
	return (Ctx.iDamage * m_iAbsorbAdj[Ctx.Damage.GetDamageType()]) / 100;
	}

int CEnergyFieldType::GetWeaponBonus (CInstalledDevice *pDevice, CSpaceObject *pSource)

//	GetWeaponBonus
//
//	Returns the bonus for this weapon

	{
	int iType = pDevice->GetDamageType();
	if (iType != -1)
		return m_iBonusAdj[iType];
	else
		return 0;
	}

ALERROR CEnergyFieldType::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind design

	{
	ALERROR error;

	if (m_pEffect)
		if (error = m_pEffect->BindDesign(Ctx))
			return error;

	if (m_pHitEffect)
		if (error = m_pHitEffect->BindDesign(Ctx))
			return error;

	return NOERROR;
	}

ALERROR CEnergyFieldType::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Create from XML

	{
	ALERROR error;
	int i;

	//	Effect

	CXMLElement *pEffect = pDesc->GetContentElementByTag(EFFECT_TAG);
	if (pEffect)
		{
		if (error = CEffectCreator::CreateFromXML(Ctx, pEffect, NULL_STR, &m_pEffect))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("energy field %x: Unable to load effect"), GetUNID());
			return error;
			}
		}

	pEffect = pDesc->GetContentElementByTag(HIT_EFFECT_TAG);
	if (pEffect == NULL)
		pEffect = pDesc->GetContentElementByTag(EFFECT_WHEN_HIT_TAG);

	if (pEffect)
		{
		if (error = CEffectCreator::CreateFromXML(Ctx, pEffect, NULL_STR, &m_pHitEffect))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("energy field %x: Unable to load hit effect"), GetUNID());
			return error;
			}
		}

	//	Damage adjustment

	LoadDamageAdj(pDesc, ABSORB_ADJ_ATTRIB, m_iAbsorbAdj);

	//	Bonus adjustment

	LoadDamageAdj(pDesc, BONUS_ADJ_ATTRIB, m_iBonusAdj);

	//	Load the weapon suppress

	CString sSuppress = pDesc->GetAttribute(WEAPON_SUPPRESS_ATTRIB);
	if (strEquals(sSuppress, SUPPRESS_ALL))
		{
		for (i = 0; i < damageCount; i++)
			m_WeaponSuppress.Add(i);
		}
	else
		{
		CStringArray WeaponSuppress;
		if (error = strDelimit(sSuppress,
				';',
				0,
				&WeaponSuppress))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("energy field %x: Unable to load weapon suppress attribute"), GetUNID());
			return error;
			}

		for (i = 0; i < WeaponSuppress.GetCount(); i++)
			{
			CString sType = strTrimWhitespace(WeaponSuppress.GetStringValue(i));
			int iType = LoadDamageTypeFromXML(sType);
			if (iType == -1)
				{
				Ctx.sError = strPatternSubst(CONSTLIT("energy field %x: Unable to load weapon suppress attribute"), GetUNID());
				return ERR_FAIL;
				}

			m_WeaponSuppress.Add(iType);
			}
		}

	//	Done

	return NOERROR;
	}
