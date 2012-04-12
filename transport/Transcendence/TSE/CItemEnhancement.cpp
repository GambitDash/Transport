//	CItemEnhancement.cpp
//
//	Item enhancement class

#include "PreComp.h"

EnhanceItemStatus CItemEnhancement::Combine (CItemEnhancement Enhancement)

//	Combine
//
//	Combine the current enhancement with the given one

	{
	DWORD dwNewMods = Enhancement;

	//	If we're losing the enhancement, then clear it

	if (dwNewMods == etLoseEnhancement)
		{
		if (IsEnhancement())
			{
			m_dwMods = 0;
			return eisEnhancementRemoved;
			}
		else
			return eisNoEffect;
		}

	//	If the item is not currently enhanced, then we take the new enhancement

	else if (m_dwMods == etNone)
		{
		//	For stackable strengthening, start at 1

		if (Enhancement.GetType() == etStrengthen
				&& Enhancement.GetLevel() == 0)
			m_dwMods = dwNewMods + 1;

		//	For all others, take the enhancement

		else
			m_dwMods = dwNewMods;

		return eisOK;
		}

	//	If already enhanced

	else if (m_dwMods == dwNewMods)
		{
		if (IsDisadvantage())
			return eisNoEffect;
		else
			return eisAlreadyEnhanced;
		}

	//	We currently have a disadvantage

	else if (IsDisadvantage())
		{
		//	We have a disadvantage and the enhancement is another
		//	disadvantage.

		if (Enhancement.IsDisadvantage())
			{
			switch (Enhancement.GetType())
				{
				//	If we're making the disadvantage worse, then
				//	continue; otherwise, no effect.

				case etStrengthen:
				case etRegenerate:
				case etResist:
				case etResistEnergy:
				case etResistMatter:
				case etPowerEfficiency:
				case etSpeed:
					{
					if (Enhancement.GetType() == GetType()
							&& Enhancement.GetLevel() > GetLevel())
						{
						m_dwMods = dwNewMods;
						return eisWorse;
						}
					else
						return eisNoEffect;
					}

				case etResistByLevel:
				case etResistByDamage:
				case etResistByDamage2:
					{
					if (Enhancement.GetType() == GetType()
							&& Enhancement.GetDamageType() == GetDamageType()
							&& Enhancement.GetLevel() > GetLevel())
						{
						m_dwMods = dwNewMods;
						return eisWorse;
						}
					else
						return eisNoEffect;
					}

				//	Otherwise, a disadvantage does not affect a disadvantage

				default:
					return eisNoEffect;
				}
			}

		//	We have a disadvantage and we use an enhancement.

		else
			{
			switch (Enhancement.GetType())
				{
				//	Regeneration enhancement always repairs a disadvantage

				case etRegenerate:
					{
					m_dwMods = 0;
					return eisRepaired;
					}

				//	If the enhancement is the opposite of the disadvantage
				//	then the disadvantage is repaired.

				case etStrengthen:
				case etResist:
				case etResistEnergy:
				case etResistMatter:
				case etPowerEfficiency:
				case etResistByLevel:
				case etResistByDamage:
				case etResistByDamage2:
				case etReflect:
				case etSpeed:
					{
					if (GetType() == Enhancement.GetType())
						{
						m_dwMods = 0;
						return eisRepaired;
						}
					else
						return eisNoEffect;
					}

				//	Otherwise, no effect

				default:
					return eisNoEffect;
				}
			}
		}

	//	We currently have an enhancement

	else
		{
		if (!Enhancement.IsDisadvantage())
			{
			if (Enhancement.GetType() == GetType())
				{
				switch (Enhancement.GetType())
					{
					case etStrengthen:
						{
						//	If stackable...

						if (Enhancement.GetLevel() == 0)
							{
							if (GetLevel() == 15)
								return eisNoEffect;
							else
								{
								m_dwMods++;
								return eisBetter;
								}
							}

						//	If improving...

						else if (Enhancement.GetLevel() > GetLevel())
							{
							m_dwMods = dwNewMods;
							return eisBetter;
							}
						else
							return eisNoEffect;
						}

					//	If this is the same type of enhancement and it is better,
					//	then take it (otherwise, no effect)

					case etRegenerate:
					case etResist:
					case etResistEnergy:
					case etResistMatter:
					case etPowerEfficiency:
					case etSpeed:
						{
						if (Enhancement.GetLevel() > GetLevel())
							{
							m_dwMods = dwNewMods;
							return eisBetter;
							}
						else
							return eisNoEffect;
						}

					case etResistByLevel:
					case etResistByDamage:
					case etResistByDamage2:
						{
						if (Enhancement.GetDamageType() != GetDamageType())
							{
							m_dwMods = dwNewMods;
							return eisEnhancementReplaced;
							}
						else if (Enhancement.GetLevel() > GetLevel())
							{
							m_dwMods = dwNewMods;
							return eisBetter;
							}
						else
							return eisNoEffect;
						}

					default:
						return eisNoEffect;
					}
				}

			//	No effect if we're already enhanced

			else
				{
				return eisNoEffect;
				}
			}
		else
			{
			//	A disadvantage always destroys any enhancement

			m_dwMods = 0;
			return eisEnhancementRemoved;
			}
		}
	}

int CItemEnhancement::GetAbsorbAdj (const DamageDesc &Damage)

//	GetAbsorbAdj
//
//	Returns damage absorbed

	{
	switch (GetType())
		{
		case etReflect:
			{
			if (IsDisadvantage() && Damage.GetDamageType() == GetDamageType())
				return Level2DamageAdj(GetLevel());
			else
				return 100;
			}

		default:
			return 100;
		}
	}

int CItemEnhancement::GetDamageAdj (const DamageDesc &Damage)

//	GetDamageAdj
//
//	Returns the damage adjustment confered by this mod

	{
	switch (GetType())
		{
		case etResist:
			return Level2DamageAdj(GetLevel(), IsDisadvantage());

		case etResistEnergy:
			return (Damage.IsEnergyDamage() ? Level2DamageAdj(GetLevel(), IsDisadvantage()) : 100);

		case etResistMatter:
			return (Damage.IsMatterDamage() ? Level2DamageAdj(GetLevel(), IsDisadvantage()) : 100);

		case etResistByLevel:
			{
			if (Damage.GetDamageType() == GetDamageType()
					|| Damage.GetDamageType() == GetDamageType() + 1)
				return Level2DamageAdj(GetLevel(), IsDisadvantage());
			else
				return 100;
			}

		case etResistByDamage:
			return (Damage.GetDamageType() == GetDamageType() ? Level2DamageAdj(GetLevel(), IsDisadvantage()) : 100);

		case etResistByDamage2:
			{
			if (Damage.GetDamageType() == GetDamageType())
				//	0 = 100			100
				//	1 = 90			111
				//	2 = 80			125
				//	3 = 70			143
				return Level2DamageAdj(GetLevel(), IsDisadvantage());
			else if (Damage.GetDamageType() == GetDamageType() + 2)
				//	0 = 100			100
				//	1 = 95			105
				//	2 = 90			112
				//	3 = 85			121
				return 100 + ((Level2DamageAdj(GetLevel(), IsDisadvantage()) - 100) / 2);
			else
				return 100;
			}

		default:
			return 100;
		}
	}

int CItemEnhancement::GetDamageBonus (void) const

//	GetDamageBonus
//
//	Returns the damage bonus for the weapon

	{
	switch (GetType())
		{
		case etStrengthen:
			return Level2Bonus(GetLevel(), IsDisadvantage());

		default:
			return 0;
		}
	}

CString CItemEnhancement::GetEnhancedDesc (const CItem &Item, CSpaceObject *pInstalled)

//	GetEnhancedDesc
//
//	Get short description of enhancement

	{
	switch (GetType())
		{
		case etStrengthen:
			{
			switch (Item.GetType()->GetCategory())
				{
				case itemcatWeapon:
				case itemcatLauncher:
					{
					int iDamageBonus;

					//	See if this device is installed; if so, then the bonus is
					//	calculated and cached in the device; we do this so that we can
					//	include bonuses from all sources.

					CInstalledDevice *pDevice;
					if (pInstalled && Item.IsInstalled() && (pDevice = pInstalled->FindDevice(Item)))
						iDamageBonus = pDevice->GetBonus();
					else
						iDamageBonus = GetDamageBonus();

					//	Bonus

					if (iDamageBonus < 0)
						return strPatternSubst(CONSTLIT("%d%%"), iDamageBonus);
					else
						return strPatternSubst(CONSTLIT("+%d%%"), iDamageBonus);
					}

				default:
					if (IsDisadvantage())
						return strPatternSubst(CONSTLIT("-%d%%"), 100 - GetHPAdj());
					else
						return strPatternSubst(CONSTLIT("+%d%%"), GetHPAdj() - 100);
				}
			}

		case etRegenerate:
			return (IsDisadvantage() ? CONSTLIT("-Decay") : CONSTLIT("+Regen"));

		case etReflect:
			return strPatternSubst((IsDisadvantage() ? CONSTLIT("-%s Trans") : CONSTLIT("+%s Reflect")),
					strCapitalizeWords(::GetDamageShortName(GetDamageType())).GetASCIIZPointer());

		case etRepairOnHit:
			return strPatternSubst(CONSTLIT("+%s Repair"), strCapitalizeWords(::GetDamageShortName(GetDamageType())).GetASCIIZPointer());

		case etResist:
			return (IsDisadvantage() ? CONSTLIT("-Vulnerable") : CONSTLIT("+Resistant"));

		case etResistEnergy:
			return (IsDisadvantage() ? CONSTLIT("-Energy Vulnerable") : CONSTLIT("+Energy Resistant"));

		case etResistMatter:
			return (IsDisadvantage() ? CONSTLIT("-Matter Vulnerable") : CONSTLIT("+Matter Resistant"));

		case etResistByLevel:
			{
			if (IsDisadvantage())
				return strPatternSubst(CONSTLIT("-%s -%s"), strCapitalizeWords(::GetDamageShortName(GetDamageType())).GetASCIIZPointer(), strCapitalizeWords(::GetDamageShortName((DamageTypes)(GetDamageType() + 1))).GetASCIIZPointer());
			else
				return strPatternSubst(CONSTLIT("+%s +%s"), strCapitalizeWords(::GetDamageShortName(GetDamageType())).GetASCIIZPointer(), strCapitalizeWords(::GetDamageShortName((DamageTypes)(GetDamageType() + 1))).GetASCIIZPointer());
			}

		case etResistByDamage:
			return strPatternSubst((IsDisadvantage() ? CONSTLIT("-%s") : CONSTLIT("+%s")),
					strCapitalizeWords(::GetDamageShortName(GetDamageType())).GetASCIIZPointer());

		case etResistByDamage2:
			{
			if (IsDisadvantage())
				return strPatternSubst(CONSTLIT("-%s -%s"), strCapitalizeWords(::GetDamageShortName(GetDamageType())).GetASCIIZPointer(), strCapitalizeWords(::GetDamageShortName((DamageTypes)(GetDamageType() + 2))).GetASCIIZPointer());
			else
				{
				switch (GetDamageType())
					{
					case damageLaser:
						return CONSTLIT("+Ablative");

					case damageKinetic:
						return CONSTLIT("+Reactive");

					default:
						return strPatternSubst(CONSTLIT("-%s -%s"), 
								strCapitalizeWords(::GetDamageShortName(GetDamageType())).GetASCIIZPointer(), 
								strCapitalizeWords(::GetDamageShortName((DamageTypes)(GetDamageType() + 2))).GetASCIIZPointer());
					}
				}
			}

		case etSpecialDamage:
			{
			switch (GetLevel2())
				{
				case specialRadiation:
					return CONSTLIT("+Rad Immune");

				case specialBlinding:
					return CONSTLIT("+Blind Immune");

				case specialEMP:
					return CONSTLIT("+EMP Immune");

				case specialDeviceDamage:
					return CONSTLIT("+Dev Protect");

				case specialDisintegration:
					return CONSTLIT("+Dis Immune");

				default:
					return CONSTLIT("+Unk Immune");
				}
			}

		case etImmunityIonEffects:
			return (IsDisadvantage() ? CONSTLIT("-No Shields") : CONSTLIT("+Ionization Immune"));

		case etPhotoRegenerate:
			return CONSTLIT("+SolRegen");

		case etPhotoRecharge:
			return CONSTLIT("+SolPower");

		case etPowerEfficiency:
			return (IsDisadvantage() ? CONSTLIT("-Drain") : CONSTLIT("+Efficient"));

		case etSpeed:
			return (IsDisadvantage() ? CONSTLIT("-Slow") : CONSTLIT("+Fast"));

		default:
			return CONSTLIT("+Unknown");
		}
	}

int CItemEnhancement::GetEnhancedRate (int iRate) const

//	GetEnhancedRate
//
//	Returns the enhanced rate

	{
	switch (GetType())
		{
		case etSpeed:
			return Max(1, iRate * Level2DamageAdj(GetLevel(), IsDisadvantage()) / 100);

		default:
			return iRate;
		}
	}

int CItemEnhancement::GetHPAdj (void) const

//	GetHPAdj
//
//	Get increase/decrease in HP

	{
	switch (GetType())
		{
		case etStrengthen:
			{
			int iLevel = GetLevel();

			if (IsDisadvantage())
				{
				if (iLevel >= 0 && iLevel <= 9)
					return 100 - (10 * iLevel);
				else
					return 10;
				}
			else
				return 100 + (10 * iLevel);
			}

		default:
			return 100;
		}
	}

int CItemEnhancement::GetPowerAdj (void) const

//	GetPowerAdj
//
//	Get the increase/decrease in power usage

	{
	switch (GetType())
		{
		case etPowerEfficiency:
			{
			int iLevel = GetLevel();

			if (IsDisadvantage())
				return 100 + (10 * iLevel);
			else
				{
				if (iLevel >= 0 && iLevel <= 9)
					return 100 - (10 * iLevel);
				else
					return 10;
				}
			}

		default:
			return 100;
		}
	}

int CItemEnhancement::GetValueAdj (const CItem &Item) const

//	GetValueAdj
//
//	Returns adjustment to item value based on modification

	{
	if (IsDisadvantage())
		{
		switch (GetType())
			{
			case etStrengthen:
			case etResistByLevel:
			case etResistByDamage:
			case etResistByDamage2:
			case etSpeed:
				return Min(-80, -10 * GetLevel());

			case etResist:
			case etResistMatter:
			case etResistEnergy:
				return Min(-80, -20 * GetLevel());

			case etPhotoRegenerate:
			case etPhotoRecharge:
			case etRepairOnHit:
			case etPowerEfficiency:
				return -50;

			case etRegenerate:
			case etReflect:
			case etSpecialDamage:
			case etImmunityIonEffects:
				return -80;

			default:
				return 0;
			}
		}
	else
		{
		switch (GetType())
			{
			case etStrengthen:
			case etResistByLevel:
			case etResistByDamage:
			case etResistByDamage2:
			case etSpeed:
				return 20 * GetLevel();

			case etResist:
			case etResistMatter:
			case etResistEnergy:
				return 50 * GetLevel();

			case etPhotoRegenerate:
			case etPhotoRecharge:
			case etRepairOnHit:
			case etPowerEfficiency:
				return 75;

			case etRegenerate:
			case etReflect:
			case etSpecialDamage:
			case etImmunityIonEffects:
				return 100;

			default:
				return 0;
			}
		}
	}

bool CItemEnhancement::IsReflective (const DamageDesc &Damage, int *retiReflectChance) const

//	IsReflective
//
//	Returns TRUE if we reflect the given damage

	{
	switch (GetType())
		{
		case etReflect:
			{
			if (!IsDisadvantage() && Damage.GetDamageType() == GetDamageType())
				{
				if (retiReflectChance)
					*retiReflectChance = 50 + (GetLevel() * 5);

				return true;
				}
			else
				return false;
			}

		default:
			return false;
		}
	}

int CItemEnhancement::Level2Bonus (int iLevel, bool bDisadvantage)

//	Level2Bonus
//
//	Convert from a level (0-15) to a bonus adj (+%)

	{
	if (bDisadvantage)
		return -10 * Min(10, iLevel);
	else
		return 10 * iLevel;
	}

int CItemEnhancement::Level2DamageAdj (int iLevel, bool bDisadvantage)

//	Level2DamageAdj
//
//	Convert from a level (0-15) to a damage adj (%)

	{
	if (bDisadvantage)
		return (10000 / Level2DamageAdj(iLevel, false));
	else
		{
		if (iLevel >= 0 && iLevel <= 9)
			return (100 - (10 * iLevel));
		else
			return 10;
		}
	}
