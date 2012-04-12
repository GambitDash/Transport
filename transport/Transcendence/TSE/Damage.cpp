//	Damage.cpp
//
//	Damage routines and classes

#include "PreComp.h"


#define SPECIAL_DAMAGE_EMP						CONSTLIT("EMP")
#define SPECIAL_DAMAGE_RADIATION				CONSTLIT("radiation")
#define SPECIAL_DAMAGE_MOMENTUM					CONSTLIT("momentum")
#define SPECIAL_DAMAGE_MEMETIC					CONSTLIT("memetic")
#define SPECIAL_DAMAGE_BLINDING					CONSTLIT("blinding")
#define SPECIAL_DAMAGE_SENSOR					CONSTLIT("sensor")
#define SPECIAL_DAMAGE_SHIELD					CONSTLIT("shield")
#define SPECIAL_DAMAGE_WORMHOLE					CONSTLIT("wormhole")
#define SPECIAL_DAMAGE_FUEL						CONSTLIT("fuel")
#define SPECIAL_DAMAGE_DISINTEGRATION			CONSTLIT("disintegration")
#define SPECIAL_DAMAGE_WMD						CONSTLIT("WMD")
#define SPECIAL_DAMAGE_DEVICE					CONSTLIT("device")
#define SPECIAL_DAMAGE_MINING					CONSTLIT("mining")
#define SPECIAL_DAMAGE_SHATTER					CONSTLIT("shatter")

char *g_pszDamageTypes[damageCount] =
	{
	"laser",
	"kinetic",
	"particle",
	"blast",
	"ion",
	"thermo",
	"positron",
	"plasma",
	"antimatter",
	"nano",
	"graviton",
	"singularity",
	"darkAcid",
	"darkSteel",
	"darkLightning",
	"darkFire",
	};

char *g_pszDamageName[damageCount] =
	{
	"laser",
	"kinetic",
	"particle beam",
	"blast",
	"ion",
	"thermonuclear",
	"positron beam",
	"plasma",
	"antimatter",
	"nanotech",
	"graviton",
	"singularity",
	"dark acid",
	"dark steel",
	"dark lightning",
	"dark fire",
	};

char *g_pszShortDamageName[damageCount] =
	{
	"laser",
	"kinetic",
	"particle",
	"blast",
	"ion",
	"thermo",
	"positron",
	"plasma",
	"antimatter",
	"nano",
	"graviton",
	"singularity",
	"dark acid",
	"dark steel",
	"dark lightning",
	"dark fire",
	};

//	Damage Types

CString GetDamageName (DamageTypes iType)

//	GetDamageName
//
//	Returns the name of this type of damage

	{
	return CString(g_pszDamageName[iType]);
	}

CString GetDamageShortName (DamageTypes iType)

//	GetDamageShortName
//
//	Returns the short name of this type of damage

	{
	return CString(g_pszShortDamageName[iType]);
	}

DamageTypes LoadDamageTypeFromXML (const CString &sAttrib)

//	LoadDamageTypeFromXML
//
//	Converts from string to DamageType

	{
	int iType;

	for (iType = 0; iType < damageCount; iType++)
		if (strEquals(sAttrib, CString(g_pszDamageTypes[iType])))
			return (DamageTypes)iType;

	//	Backwards compatibility

	if (strEquals(sAttrib, CONSTLIT("dark acid")))
		return damageDarkAcid;
	else if (strEquals(sAttrib, CONSTLIT("dark steel")))
		return damageDarkSteel;
	else if (strEquals(sAttrib, CONSTLIT("dark lightning")))
		return damageDarkLightning;
	else if (strEquals(sAttrib, CONSTLIT("dark fire")))
		return damageDarkFire;

	return damageNone;
	}

void LoadDamageAdjFromXML (const CString &sAttrib, int *pAdjTable)

//	LoadDamageAdjFromXML
//
//	Loads a list of damage adjustments into the array. The array must
//	be allocated to damageCount.

	{
	char *pPos = sAttrib.GetASCIIZPointer();
	for (int i = 0; i < damageCount; i++)
		{
		pAdjTable[i] = strParseInt(pPos, 0, &pPos, NULL);
		if (*pPos != '\0')
			pPos++;
		}
	}

void DamageDesc::SetDamage (int iDamage)

//	SetDamage
//
//	Sets a constant damage

	{
	m_Damage = DiceRange(0, 0, iDamage);
	}

Metric DamageDesc::GetAverageDamage (void)
	{
	if (m_Damage.GetFaces() > 0)
		return m_Damage.GetCount() * ((1.0 + m_Damage.GetFaces()) / 2.0) + m_Damage.GetBonus();
	else
		return (Metric)m_Damage.GetBonus();
	}

CString DamageDesc::GetDesc (void)

//	GetDesc
//
//	Returns a description of the damage:
//
//	laser 1-4 (+50%)

	{
	int iMin = m_Damage.GetCount() + m_Damage.GetBonus();
	int iMax = m_Damage.GetCount() * m_Damage.GetFaces() + m_Damage.GetBonus();

	char szBuffer[1024];
	if (m_iBonus == 0)
		{
		int iLen;
		if (iMin == iMax)
			iLen = sprintf(szBuffer, "%s %d", g_pszShortDamageName[m_iType], iMax);
		else
			iLen = sprintf(szBuffer, "%s %d-%d", g_pszShortDamageName[m_iType], iMin, iMax);

		return CString(szBuffer, iLen);
		}
	else if (m_iBonus > 0)
		{
		int iLen;
		if (iMin == iMax)
			iLen = sprintf(szBuffer, "%s %d (+%d%%)", g_pszShortDamageName[m_iType], iMax, m_iBonus);
		else
			iLen = sprintf(szBuffer, "%s %d-%d (+%d%%)", g_pszShortDamageName[m_iType], iMin, iMax, m_iBonus);

		return CString(szBuffer, iLen);
		}
	else
		{
		int iLen;
		if (iMin == iMax)
			iLen = sprintf(szBuffer, "%s %d (-%d%%)", g_pszShortDamageName[m_iType], iMax, -m_iBonus);
		else
			iLen = sprintf(szBuffer, "%s %d-%d (-%d%%)", g_pszShortDamageName[m_iType], iMin, iMax, -m_iBonus);

		return CString(szBuffer, iLen);
		}
	}

int DamageDesc::GetMinDamage (void)
	{
	return m_Damage.GetCount() + m_Damage.GetBonus();
	}

int DamageDesc::GetMaxDamage (void)
	{
	return m_Damage.GetCount() * m_Damage.GetFaces() + m_Damage.GetBonus();
	}

ALERROR DamageDesc::LoadFromXML (const CString &sAttrib)

//	LoadFromXML
//
//	Loads damage of the form:
//
//	damagetype:1d1+1; ion6; momentum5; radiation4; memetic6

	{
	ALERROR error;

	//	Initialize

	m_iType = damageNone;
	m_iBonus = 0;
	m_iCause = killedByDamage;
	m_EMPDamage = 0;
	m_MomentumDamage = 0;
	m_RadiationDamage = 0;
	m_DisintegrationDamage = 0;
	m_MemeticDamage = 0;
	m_BlindingDamage = 0;
	m_SensorDamage = 0;
	m_ShieldDamage = 0;
	m_WormholeDamage = 0;
	m_FuelDamage = 0;
	m_fAutomatedWeapon = false;
	m_DeviceDamage = 0;
	m_MassDestructionAdj = 0;
	m_MiningAdj = 0;
	m_ShatterDamage = 0;

	//	Loop over all segments separated by semi-colons

	char *pPos = sAttrib.GetASCIIZPointer();
	char *pSegStart = pPos;
	bool bDone = false;
	while (!bDone)
		{
		if (*pPos == ';' || *pPos == '\0')
			{
			//	The segment is from pSegStart to pPos

			CString sSegment(pSegStart, pPos - pSegStart);

			//	Look for the separator

			char *pDamPos = sSegment.GetASCIIZPointer();
			char *pDamStart = pDamPos;
			while (*pDamPos != ':' && *pDamPos != '\0')
				pDamPos++;

			//	If there is no separator then this is a normal damage
			//	segment.

			if (*pDamPos == ':')
				{
				CString sType(pDamStart, pDamPos - pDamStart);
				CString sRoll(pDamPos+1);

				m_iType = LoadDamageTypeFromXML(sType);
				if (m_iType == damageNone)
					return ERR_FAIL;

				if (error = m_Damage.LoadFromXML(sRoll))
					return error;
				}

			//	Otherwise it is a special damage segment

			else
				{
				//	The last character is the count

				if (pDamPos > pDamStart)
					{
					//	Look for a number at the end

					char *pNumber = pDamPos;
					while (*(pNumber - 1) >= '0' && *(pNumber - 1) <= '9')
						pNumber--;

					int iCount = strParseInt(pNumber, 0, NULL, NULL);

					CString sType(pDamStart, pNumber - pDamStart);
					if (strEquals(sType, SPECIAL_DAMAGE_EMP))
						m_EMPDamage = (DWORD)iCount;
					else if (strEquals(sType, SPECIAL_DAMAGE_RADIATION))
						m_RadiationDamage = (DWORD)iCount;
					else if (strEquals(sType, SPECIAL_DAMAGE_MOMENTUM))
						m_MomentumDamage = (DWORD)iCount;
					else if (strEquals(sType, SPECIAL_DAMAGE_DISINTEGRATION))
						m_DisintegrationDamage = (DWORD)iCount;
					else if (strEquals(sType, SPECIAL_DAMAGE_MEMETIC))
						m_MemeticDamage = (DWORD)iCount;
					else if (strEquals(sType, SPECIAL_DAMAGE_BLINDING))
						m_BlindingDamage = (DWORD)iCount;
					else if (strEquals(sType, SPECIAL_DAMAGE_SENSOR))
						m_SensorDamage = (DWORD)iCount;
					else if (strEquals(sType, SPECIAL_DAMAGE_SHIELD))
						m_ShieldDamage = (DWORD)iCount;
					else if (strEquals(sType, SPECIAL_DAMAGE_WORMHOLE))
						m_WormholeDamage = (DWORD)iCount;
					else if (strEquals(sType, SPECIAL_DAMAGE_FUEL))
						m_FuelDamage = (DWORD)iCount;
					else if (strEquals(sType, SPECIAL_DAMAGE_WMD))
						m_MassDestructionAdj = (DWORD)iCount;
					else if (strEquals(sType, SPECIAL_DAMAGE_DEVICE))
						m_DeviceDamage = (DWORD)iCount;
					else if (strEquals(sType, SPECIAL_DAMAGE_MINING))
						m_MiningAdj = (DWORD)iCount;
					else if (strEquals(sType, SPECIAL_DAMAGE_SHATTER))
						m_ShatterDamage = (DWORD)iCount;
					else
						return ERR_FAIL;
					}
				else
					return ERR_FAIL;
				}

			//	Next segment

			while (*pPos == ';' || *pPos == ' ')
				pPos++;

			if (*pPos == '\0')
				bDone = true;
			else
				pSegStart = pPos;
			}
		else
			pPos++;
		}

	if (m_iType == damageNone)
		return ERR_FAIL;

	return NOERROR;
	}

void DamageDesc::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Reads from a stream
//
//	DWORD		m_iType
//	DWORD		DiceRange
//	DWORD		m_iBonus
//	DWORD		m_iCause
//	DWORD		Extra damage
//	DWORD		Extra damage 2

	{
	DWORD dwLoad;

	Ctx.pStream->Read((char *)&m_iType, sizeof(DWORD));
	m_Damage.ReadFromStream(Ctx);
	Ctx.pStream->Read((char *)&m_iBonus, sizeof(DWORD));

	if (Ctx.dwVersion >= 18)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		m_iCause = (DestructionTypes)dwLoad;
		}
	else
		m_iCause = killedByDamage;

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_EMPDamage = dwLoad & 0x07;
	m_MomentumDamage = (dwLoad >> 3) & 0x07;
	m_RadiationDamage = (dwLoad >> 6) & 0x07;
	m_MemeticDamage = (dwLoad >> 9) & 0x07;
	m_BlindingDamage = (dwLoad >> 12) & 0x07;
	m_SensorDamage = (dwLoad >> 15) & 0x07;
	m_ShieldDamage = (dwLoad >> 18) & 0x07;
	m_WormholeDamage = (dwLoad >> 21) & 0x07;
	m_FuelDamage = (dwLoad >> 24) & 0x07;
	m_DisintegrationDamage = (dwLoad >> 27) & 0x07;
	m_fAutomatedWeapon = (dwLoad >> 31) & 0x01;

	//	In previous versions we had a bit that determined whether this damage
	//	came from weapons malfunction

	if (Ctx.dwVersion < 18 && ((dwLoad >> 30) & 0x01))
		m_iCause = killedByWeaponMalfunction;

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_DeviceDamage = dwLoad & 0x07;
	m_MassDestructionAdj = (dwLoad >> 3) & 0x07;
	m_MiningAdj = (dwLoad >> 6) & 0x07;
	m_ShatterDamage = (dwLoad >> 9) & 0x07;
	m_dwSpare2 = 0;
	}

int DamageDesc::RollDamage (void) const

//	RollDamage
//
//	Computes hp damage

	{
	int iDamage = m_Damage.Roll();
	if (m_iBonus > 0)
		{
		int iNum = iDamage * m_iBonus;
		int iWhole = iNum / 100;
		int iPartial = iNum % 100;
		if (mathRandom(1, 100) <= iPartial)
			iWhole++;

		return iDamage + iWhole;
		}
	else
		return iDamage;
	}

void DamageDesc::WriteToStream (IWriteStream *pStream) const

//	WriteToStream
//
//	Writes out to a stream
//
//	DWORD		m_iType
//	DWORD		DiceRange
//	DWORD		m_iBonus
//	DWORD		m_iCause
//	DWORD		Extra damage
//	DWORD		Extra damage 2

	{
	DWORD dwSave;

	pStream->Write((char *)&m_iType, sizeof(DWORD));
	m_Damage.WriteToStream(pStream);
	pStream->Write((char *)&m_iBonus, sizeof(DWORD));

	dwSave = m_iCause;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	dwSave = m_EMPDamage;
	dwSave |= m_MomentumDamage << 3;
	dwSave |= m_RadiationDamage << 6;
	dwSave |= m_MemeticDamage << 9;
	dwSave |= m_BlindingDamage << 12;
	dwSave |= m_SensorDamage << 15;
	dwSave |= m_ShieldDamage << 18;
	dwSave |= m_WormholeDamage << 21;
	dwSave |= m_FuelDamage << 24;
	dwSave |= m_DisintegrationDamage << 27;
	//SPARE dwSave |= m_fWeaponMisfire << 30;
	dwSave |= m_fAutomatedWeapon << 31;
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	dwSave = m_DeviceDamage;
	dwSave |= m_MassDestructionAdj << 3;
	dwSave |= m_MiningAdj << 6;
	dwSave |= m_ShatterDamage << 9;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

//	DamageTypeSet --------------------------------------------------------------

ALERROR DamageTypeSet::InitFromXML (const CString &sAttrib)

//	InitFromXML
//
//	Initialize set from semi-comma separated list

	{
	ALERROR error;
	int i;

	if (sAttrib.IsBlank())
		return NOERROR;

	CStringArray ArraySet;
	if (error = strDelimit(sAttrib,
			';',
			0,
			&ArraySet))
		return error;

	for (i = 0; i < ArraySet.GetCount(); i++)
		{
		CString sType = strTrimWhitespace(ArraySet.GetStringValue(i));
		int iType = LoadDamageTypeFromXML(sType);
		if (iType == -1)
			return ERR_FAIL;

		Add(iType);
		}

	return NOERROR;
	}
