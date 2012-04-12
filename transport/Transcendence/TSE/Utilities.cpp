//	Utilities.cpp
//
//	Utility classes

#include "PreComp.h"

#define MAX_NAMES							1000

#define SOVEREIGN_ATTRIB					CONSTLIT("sovereign")
#define CLASS_ATTRIB						CONSTLIT("class")

#define FREQUENCY_COMMON					CONSTLIT("common")
#define FREQUENCY_UNCOMMON					CONSTLIT("uncommon")
#define FREQUENCY_RARE						CONSTLIT("rare")
#define FREQUENCY_VERYRARE					CONSTLIT("veryrare")
#define FREQUENCY_NOTRANDOM					CONSTLIT("notrandom")

#define ARMOR_COMPOSITION_METALLIC			CONSTLIT("metallic")
#define ARMOR_COMPOSITION_CERAMIC			CONSTLIT("ceramic")
#define ARMOR_COMPOSITION_CARBIDE			CONSTLIT("carbide")
#define ARMOR_COMPOSITION_NANOSCALE			CONSTLIT("nanoscale")
#define ARMOR_COMPOSITION_QUANTUM			CONSTLIT("quantum")
#define ARMOR_COMPOSITION_GRAVITIC			CONSTLIT("gravitic")
#define ARMOR_COMPOSITION_DARKMATTER		CONSTLIT("darkmatter")

#define DEFINITE_ARTICLE_ATTRIB				CONSTLIT("definiteArticle")
#define FIRST_PLURAL_ATTRIB					CONSTLIT("firstPlural")
#define ES_PLURAL_ATTRIB					CONSTLIT("esPlural")
#define SECOND_PLURAL_ATTRIB				CONSTLIT("secondPlural")
#define VOWEL_ARTICLE_ATTRIB				CONSTLIT("reverseArticle")
#define CUSTOM_PLURAL_ATTRIB				CONSTLIT("customPlural")
#define NO_ARTICLE_ATTRIB					CONSTLIT("noArticle")
#define HP_BONUS_ATTRIB						CONSTLIT("hpBonus")
#define DAMAGE_ADJ_ATTRIB					CONSTLIT("damageAdj")

#define VERSION_095							(CONSTLIT("0.95"))
#define VERSION_095A						(CONSTLIT("0.95a"))
#define VERSION_095B						(CONSTLIT("0.95b"))
#define VERSION_096							(CONSTLIT("0.96"))
#define VERSION_096A						(CONSTLIT("0.96a"))
#define VERSION_097							(CONSTLIT("0.97"))
#define VERSION_097A						(CONSTLIT("0.97a"))
#define VERSION_098							(CONSTLIT("0.98"))
#define VERSION_098A						(CONSTLIT("0.98a"))
#define VERSION_098B						(CONSTLIT("0.98b"))
#define VERSION_098C						(CONSTLIT("0.98c"))
#define VERSION_098D						(CONSTLIT("0.98d"))
#define VERSION_099							(CONSTLIT("0.99"))
#define VERSION_099A						(CONSTLIT("0.99a"))
#define VERSION_099B						(CONSTLIT("0.99b"))
#define VERSION_099C						(CONSTLIT("0.99c"))

#define CONTROLLER_FLEET					CONSTLIT("fleet")
#define CONTROLLER_FERIAN					CONSTLIT("ferian")
#define CONTROLLER_AUTON					CONSTLIT("auton")
#define CONTROLLER_GLADIATOR				CONSTLIT("gladiator")
#define CONTROLLER_FLEET_COMMAND			CONSTLIT("fleetcommand")
#define CONTROLLER_GAIAN_PROCESSOR			CONSTLIT("gaianprocessor")
#define CONTROLLER_ZOANTHROPE				CONSTLIT("zoanthrope")

static Metric g_RangeIndex[RANGE_INDEX_COUNT] =
	{
	(500.0 * LIGHT_SECOND),
	(340.0 * LIGHT_SECOND),
	(225.0 * LIGHT_SECOND),
	(150.0 * LIGHT_SECOND),
	(100.0 * LIGHT_SECOND),
	(50.0 * LIGHT_SECOND),
	(25.0 * LIGHT_SECOND),
	(13.0 * LIGHT_SECOND),
	(6.0 * LIGHT_SECOND),
	};

static char *g_pszOrderTypes[] =
	{
	"",

	"guard",
	"dock",
	"attack",
	"wait",
	"gate",

	"gateOnThreat",
	"gateOnStationDestroyed",
	"patrol",
	"escort",
	"scavenge",

	"followPlayerThroughGate",
	"attackNearestEnemy",
	"tradeRoute",
	"wander",
	"loot",

	"hold",
	"mine",
	"waitForPlayer",
	"attackPlayerOnReturn",
	"follow",

	"navPath",
	"goto",
	"waitForTarget",
	"waitForEnemy",
	"bombard",

	"approach",
	"aim"
	};

#define ORDER_TYPES_COUNT		(sizeof(g_pszOrderTypes) / sizeof(g_pszOrderTypes[0]))

static char *g_pszDestructionCauses[] =
	{
	"",

	"damage",
	"runningOutOfFuel",
	"radiationPoisoning",
	"self",
	"disintegration",

	"weaponMalfunction",
	"ejecta",
	"explosion",
	"shatter",
	"playerCreatedExplosion",
	};

#define DESTRUCTION_CAUSES_COUNT	(sizeof(g_pszDestructionCauses) / sizeof(g_pszDestructionCauses[0]))

//	CDiceRange ----------------------------------------------------------------

DiceRange::DiceRange (int iFaces, int iCount, int iBonus) :
		m_iFaces(iFaces),
		m_iCount(iCount),
		m_iBonus(iBonus)

//	CDiceRange constructor

	{
	}

ALERROR DiceRange::LoadFromXML (const CString &sAttrib)

//	LoadFromXML
//
//	Loads from an XML attribute

	{
	BOOL bNullValue;
	char *pPos = sAttrib.GetASCIIZPointer();

	//	If empty, then default to 0

	if (*pPos == '\0')
		{
		m_iCount = 0;
		m_iFaces = 0;
		m_iBonus = 0;
		return NOERROR;
		}

	//	First is the number of dice

	int iCount = strParseInt(pPos, 0, &pPos, &bNullValue);
	if (bNullValue)
		return ERR_FAIL;

	//	If we've got a 'd' then we have a dice pattern

	if (*pPos == 'd')
		{
		pPos++;
		m_iCount = iCount;
		if (iCount < 0)
			return ERR_FAIL;

		//	Now parse the sides

		m_iFaces = strParseInt(pPos, -1, &pPos, NULL);
		if (m_iFaces == -1)
			return ERR_FAIL;

		//	Finally, add any bonus

		if (*pPos != '\0')
			m_iBonus = strParseInt(pPos, 0, NULL, NULL);
		else
			m_iBonus = 0;
		}

	//	If we've got a '-' then we have a range pattern

	else if (*pPos == '-')
		{
		pPos++;
		int iEnd = strParseInt(pPos, 0, &pPos, &bNullValue);
		if (bNullValue)
			return ERR_FAIL;

		if (iEnd < iCount)
			Swap(iEnd, iCount);

		m_iCount = 1;
		m_iFaces = (iEnd - iCount) + 1;
		m_iBonus = iCount - 1;
		}

	//	Otherwise, we've got a constant number

	else
		{
		m_iCount = 0;
		m_iFaces = 0;
		m_iBonus = iCount;
		}

	return NOERROR;
	}

void DiceRange::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Read the range

	{
	if (Ctx.dwVersion >= 27)
		{
		Ctx.pStream->Read((char *)&m_iFaces, sizeof(DWORD));
		Ctx.pStream->Read((char *)&m_iCount, sizeof(DWORD));
		Ctx.pStream->Read((char *)&m_iBonus, sizeof(DWORD));
		}
	else
		{
		DWORD dwLoad;
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));

		m_iFaces = HIBYTE(LOWORD(dwLoad));
		m_iCount = LOBYTE(LOWORD(dwLoad));
		m_iBonus = (int)(short)HIWORD(dwLoad);
		}
	}

int DiceRange::Roll (void) const

//	Roll
//
//	Generate a random number

	{
	int iRoll = 0;

	for (int i = 0; i < m_iCount; i++)
		iRoll += mathRandom(1, m_iFaces);

	return iRoll + m_iBonus;
	}

CString DiceRange::SaveToXML (void) const

//	SaveToXML
//
//	Represents range as an attribute value

	{
	if (m_iCount > 0 && m_iFaces > 0)
		{
		if (m_iBonus > 0)
			return strPatternSubst("%dd%d+%d", m_iCount, m_iFaces, m_iBonus);
		else if (m_iBonus < 0)
			return strPatternSubst("%dd%d-%d", m_iCount, m_iFaces, -m_iBonus);
		else
			return strPatternSubst("%dd%d", m_iCount, m_iFaces);
		}
	else
		return strFromInt(m_iBonus, TRUE);
	}

void DiceRange::WriteToStream (IWriteStream *pStream) const

//	WriteToStream
//
//	Writes the range

	{
	pStream->Write((char *)&m_iFaces, sizeof(DWORD));
	pStream->Write((char *)&m_iCount, sizeof(DWORD));
	pStream->Write((char *)&m_iBonus, sizeof(DWORD));
	}

//	Miscellaneous functions

CString AppendModifiers (const CString &sModifierList1, const CString &sModifierList2)

//	AppendModifiers
//
//	Returns a concatenation of the two modifier lists

	{
	if (sModifierList1.IsBlank())
		return sModifierList2;
	else if (sModifierList2.IsBlank())
		return sModifierList1;
	else
		{
		CString sList1 = strTrimWhitespace(sModifierList1);
		if (sList1.IsBlank())
			return sModifierList2;

		CString sList2 = strTrimWhitespace(sModifierList2);
		if (sList2.IsBlank())
			return sModifierList1;

		char *pLastChar = (sList1.GetASCIIZPointer() + sList1.GetLength() - 1);
		if (*pLastChar != ',' && *pLastChar != ';')
			sList1.Append(CONSTLIT(", "));

		sList1.Append(sList2);

		return sList1;
		}
	}

IShipController::ManeuverTypes CalcTurnManeuver (int iDesired, int iCurrent, int iRotationAngle)

//	CalcTurnManeuver
//
//	Calculates the turn maneuver required to face the given direction

	{
	int iTurn = (iDesired + 360 - iCurrent) % 360;

	if ((iTurn >= (360 - (iRotationAngle / 2)))
			|| (iTurn <= (iRotationAngle / 2)))
		return IShipController::NoRotation;
	else
		{
		if (iTurn >= 180)
			return IShipController::RotateRight;
		else
			return IShipController::RotateLeft;
		}
	}

CString ComposeDamageAdjReference (int *AdjRow, int *StdRow)

//	ComposeDamageAdjReference
//
//	Compose a string that describes the damage adjustments

	{
	CString sResult;
	CString sVulnerable;
	CString sResistant;
	CString sImmune;
	int iVulnerableCount = 0;
	int iResistantCount = 0;
	int iImmuneCount = 0;
	int iDamage;
	for (iDamage = 0; iDamage < damageCount; iDamage++)
		{
		int iAdj = AdjRow[iDamage];
		int iStd = StdRow[iDamage];

		if (iAdj != iStd)
			{
			//	Append

			if (iAdj > iStd)
				{
				if (sVulnerable.IsBlank())
					sVulnerable = strPatternSubst(CONSTLIT("%s"), GetDamageShortName((DamageTypes)iDamage).GetASCIIZPointer());
				else
					sVulnerable.Append(strPatternSubst(CONSTLIT(", %s"), GetDamageShortName((DamageTypes)iDamage).GetASCIIZPointer()));

				iVulnerableCount++;
				}
			else if (iAdj < 20)
				{
				if (sImmune.IsBlank())
					sImmune = strPatternSubst(CONSTLIT("%s"), GetDamageShortName((DamageTypes)iDamage).GetASCIIZPointer());
				else
					sImmune.Append(strPatternSubst(CONSTLIT(", %s"), GetDamageShortName((DamageTypes)iDamage).GetASCIIZPointer()));

				iImmuneCount++;
				}
			else
				{
				if (sResistant.IsBlank())
					sResistant = strPatternSubst(CONSTLIT("%s"), GetDamageShortName((DamageTypes)iDamage).GetASCIIZPointer());
				else
					sResistant.Append(strPatternSubst(CONSTLIT(", %s"), GetDamageShortName((DamageTypes)iDamage).GetASCIIZPointer()));

				iResistantCount++;
				}
			}
		}

	if (!sImmune.IsBlank())
		{
		if (iImmuneCount == damageCount)
			sResult.Append(CONSTLIT("; immune to all damage"));
		else if (iImmuneCount == 1)
			sResult.Append(strPatternSubst(CONSTLIT("; %s-immune"), sImmune.GetASCIIZPointer()));
		else
			sResult.Append(strPatternSubst(CONSTLIT("; immune to: %s"), sImmune.GetASCIIZPointer()));
		}

	if (!sResistant.IsBlank())
		{
		if (iResistantCount == damageCount)
			sResult.Append(CONSTLIT("; resistant to all damage"));
		else if (iResistantCount == 1)
			sResult.Append(strPatternSubst(CONSTLIT("; %s-resistant"), sResistant.GetASCIIZPointer()));
		else if (iResistantCount + iImmuneCount == damageCount)
			sResult.Append(CONSTLIT("; resistant to all other damage"));
		else
			sResult.Append(strPatternSubst(CONSTLIT("; resistant to: %s"), sResistant.GetASCIIZPointer()));
		}

	if (!sVulnerable.IsBlank())
		{
		if (iVulnerableCount == damageCount)
			sResult.Append(CONSTLIT("; vulnerable to all damage"));
		else if (iVulnerableCount == 1)
			sResult.Append(strPatternSubst(CONSTLIT("; %s-vulnerable"), sVulnerable.GetASCIIZPointer()));
		else if (iVulnerableCount + iResistantCount + iImmuneCount == damageCount)
			sResult.Append(CONSTLIT("; vulnerable to all other damage"));
		else
			sResult.Append(strPatternSubst(CONSTLIT("; vulnerable to: %s"), sVulnerable.GetASCIIZPointer()));
		}

	return sResult;
	}

CString ComposeNounPhrase (const CString &sNoun, 
						   int iCount, 
						   const CString &sModifier,
						   DWORD dwNounFlags, 
						   DWORD dwComposeFlags)

//	ComposeNounPhrase
//
//	Composes a noun phrase based on the appropriate flags

	{
	bool bPluralize = ((dwComposeFlags & nounPlural) ? true : false);

	//	If this is a custom plural then decompose the noun into
	//	singular and plural

	CString sSingularNoun = sNoun;
	CString sPluralNoun = sNoun;
	if (dwNounFlags & nounCustomPlural)
		{
		char *pStart = sNoun.GetPointer();
		char *pPos = pStart;
		while (*pPos != '\0' && *pPos != ';')
			pPos++;

		sSingularNoun = CString(pStart, pPos - pStart);

		while (*pPos == ';' || *pPos == ' ')
			pPos++;

		sPluralNoun = CString(pPos);
		}

	//	Apply the appropriate noun phrase flags

	CString sArticle;
	if ((dwComposeFlags & nounArticle)
			|| ((dwComposeFlags & nounCount) && iCount == 1))
		{
		if (dwNounFlags & nounNoArticle)
			sArticle = NULL_STR;
		else if (dwNounFlags & nounDefiniteArticle)
			sArticle = CONSTLIT("the ");
		else
			{
			char *pFirstLetter;
			if (sModifier.IsBlank())
				pFirstLetter = sSingularNoun.GetPointer();
			else
				pFirstLetter = sModifier.GetPointer();

			switch (*pFirstLetter)
				{
				case 'A':
				case 'a':
				case 'E':
				case 'e':
				case 'I':
				case 'i':
				case 'O':
				case 'o':
				case 'U':
				case 'u':
					{
					if (dwNounFlags & nounVowelArticle)
						sArticle = CONSTLIT("a ");
					else
						sArticle = CONSTLIT("an ");
					break;
					}

				default:
					{
					if (dwNounFlags & nounVowelArticle)
						sArticle = CONSTLIT("an ");
					else
						sArticle = CONSTLIT("a ");
					}
				}
			}
		}
	else if (dwComposeFlags & nounDemonstrative)
		{
		if (dwNounFlags & nounNoArticle)
			sArticle = NULL_STR;
		else if (dwNounFlags & nounDefiniteArticle)
			sArticle = CONSTLIT("the ");
		else
			{
			if (iCount > 1)
				{
				sArticle = CONSTLIT("these ");
				bPluralize = true;
				}
			else
				sArticle = CONSTLIT("this ");
			}
		}
	else if (iCount > 1
			&& ((dwComposeFlags & nounCount) || (dwComposeFlags & nounCountOnly)))
		{
		sArticle = strFromInt(iCount, false);
		sArticle.Append(CONSTLIT(" "));
		bPluralize = true;
		}

	//	What should the plural be?

	CString sPlural;
	if (bPluralize)
		{
		if (dwNounFlags & nounCustomPlural)
			{
			}
		else if (dwNounFlags & nounPluralES)
			sPlural = CONSTLIT("es");
		else
			sPlural = CONSTLIT("s");
		}

	//	Which word should be pluralized?

	int iPluralWord = 0;
	if (dwNounFlags & nounFirstPlural)
		iPluralWord = 1;
	else if (dwNounFlags & nounSecondPlural)
		iPluralWord = 2;

	//	Pluralize the phrase

	if (bPluralize && iPluralWord > 0)
		{
		//	Point to the space at the end of the first n words.

		char *pStart = sNoun.GetPointer();
		char *pPos = pStart;

		while (iPluralWord > 0)
			{
			while (*pPos != ' ' && *pPos != '\0')
				pPos++;

			if (--iPluralWord > 0)
				pPos++;
			}

		CString sFirstWord = strSubString(sNoun, 0, pPos - pStart);
		CString sRest = CString(pPos);

		if (dwComposeFlags & nounCapitalize)
			return strCapitalize(strPatternSubst(CONSTLIT("%s%s%s%s%s"), sArticle.GetASCIIZPointer(), sModifier.GetASCIIZPointer(), sFirstWord.GetASCIIZPointer(), sPlural.GetASCIIZPointer(), sRest.GetASCIIZPointer()));
		else
			return strPatternSubst(CONSTLIT("%s%s%s%s%s"), sArticle.GetASCIIZPointer(), sModifier.GetASCIIZPointer(), sFirstWord.GetASCIIZPointer(), sPlural.GetASCIIZPointer(), sRest.GetASCIIZPointer());
		}
	if (dwComposeFlags & nounCapitalize)
		return strCapitalize(strPatternSubst(CONSTLIT("%s%s%s%s"), 
				sArticle.GetASCIIZPointer(), 
				sModifier.GetASCIIZPointer(), 
				(bPluralize ? sPluralNoun.GetASCIIZPointer() : sSingularNoun.GetASCIIZPointer()),
				sPlural.GetASCIIZPointer()));
	else
		return strPatternSubst(CONSTLIT("%s%s%s%s"), 
				sArticle.GetASCIIZPointer(), 
				sModifier.GetASCIIZPointer(), 
				(bPluralize ? sPluralNoun.GetASCIIZPointer() : sSingularNoun.GetASCIIZPointer()),
				sPlural.GetASCIIZPointer());
	}

void ComputePercentages (int iCount, int *pTable)

//	ComputePercentages
//
//	Given an array of integers representing weights, this function
//	modifies the array to contain a percentages proportional to the
//	weights.

	{
	struct TableEntry
		{
		int iChance;
		int iRemainder;
		};

	int i;
	TableEntry *pScratch = new TableEntry[iCount];

	//	Add up the total weights of all items

	int iTotalScore = 0;
	for (i = 0; i < iCount; i++)
		iTotalScore += pTable[i];

	//	Done if no scores

	if (iTotalScore <= 0)
		{
		delete [] pScratch;
		return;
		}

	//	Compute the chance

	int iTotalChance = 0;
	for (i = 0; i < iCount; i++)
		{
		pScratch[i].iChance = (pTable[i] * 100) / iTotalScore;
		pScratch[i].iRemainder = (pTable[i] * 100) % iTotalScore;

		iTotalChance += pScratch[i].iChance;
		}

	//	Distribute the remaining chance points

	while (iTotalChance < 100)
		{
		//	Look for the entry with the biggest remainder

		int iBestRemainder = 0;
		int iBestEntry = -1;

		for (i = 0; i < iCount; i++)
			if (pScratch[i].iRemainder > iBestRemainder)
				{
				iBestRemainder = pScratch[i].iRemainder;
				iBestEntry = i;
				}

		pScratch[iBestEntry].iChance++;
		pScratch[iBestEntry].iRemainder = 0;
		iTotalChance++;
		}

	//	Modify original

	for (i = 0; i < iCount; i++)
		pTable[i] = pScratch[i].iChance;

	//	Done

	delete [] pScratch;
	}

#if 0
CVector ConvertObjectPos2Pos (int iAngle, Metric rRadius, Metric rHeight, Metric rImageSize)

//	ConvertObjectPos2Pos
//
//	Convert from a position on a rendered 3D object to a position relative
//	to the center of the object's 2D image.
//
//	rRadius and rHeight are in units, 0 degrees is straight up.
//	rImageSize is the size of the rendered image in pixels.
//
//	Note: We expect rHeight and rRadius to be less than 12.0

	{
	const Metric CAMERA_Z = 12.0;
	const Metric CAMERA_Y = 6.0;
	const Metric CAMERA_DIST = 13.4164;		//	Distance from origin to camera
	const Metric CAMERA_SIN = 0.447214;		//	Sin Theta (angle of camera plane wrt object plane)
	const Metric CAMERA_FIELD = 6.0;		//	Camera field of view in units

	//	If no image size, then assume a simple rotation

	if (rImageSize == 0.0)
		return PolarToVector(iAngle, rRadius * g_KlicksPerPixel);

	//	Convert the object pos to cartessian

	CVector vPos = PolarToVector(iAngle, rRadius);

	//	Adjust the y axis for height

	vPos.SetY(vPos.GetY() + ((rHeight * (vPos.GetY() + CAMERA_Y)) / (CAMERA_Z - rHeight)));

	//	Adjust the y axis for camera projection

	vPos.SetY((CAMERA_DIST * vPos.GetY()) / (CAMERA_DIST + (CAMERA_SIN * vPos.GetY())));

	//	Convert to klicks

	return vPos * (g_KlicksPerPixel * rImageSize / CAMERA_FIELD);
	}
#endif

IShipController *CreateShipController (const CString &sAI)

//	CreateShipController
//
//	Creates the appropriate controller
//
//	The following controllers are valid:
//
//	""				The standard AI
//	"auton"			Auton AI
//	"ferian"		Ferian ship AI
//	"fleet"			The fleet controller for formation flying
//	"fleetcommand"	Controller for commanding a fleet
//	"gaianprocessor"Controller for Gaian processor
//	"gladiator"		Arena Gladiator AI
//	"zoanthrope"	Zoanthrope AI

	{
	if (sAI.IsBlank())
		return new CStandardShipAI;
	else if (strEquals(sAI, CONTROLLER_FLEET))
		return new CFleetShipAI;
	else if (strEquals(sAI, CONTROLLER_FERIAN))
		return new CFerianShipAI;
	else if (strEquals(sAI, CONTROLLER_AUTON))
		return new CAutonAI;
	else if (strEquals(sAI, CONTROLLER_GLADIATOR))
		return new CGladiatorAI;
	else if (strEquals(sAI, CONTROLLER_FLEET_COMMAND))
		return new CFleetCommandAI;
	else if (strEquals(sAI, CONTROLLER_GAIAN_PROCESSOR))
		return new CGaianProcessorAI;
	else if (strEquals(sAI, CONTROLLER_ZOANTHROPE))
		return new CZoanthropeAI;
	else
		return NULL;
	}

CString GenerateRandomName (const CString &sList, const CString &sSubst)

//	GenerateRandomName
//
//	Generates a random name based on a list

	{
	char *pName[MAX_NAMES];
	int iLength[MAX_NAMES];
	int iCount = 0;

	char *pPos = sList.GetASCIIZPointer();
	bool bInName = false;

	while (iCount < MAX_NAMES)
		{
		if (!bInName)
			{
			if (*pPos == '\0')
				break;

			if (!strIsWhitespace(pPos) && *pPos != ';')
				{
				pName[iCount] = pPos;
				bInName = true;
				}
			}
		else
			{
			if (*pPos == ';' || *pPos == '\0')
				{
				iLength[iCount] = pPos - pName[iCount];
				iCount++;
				bInName = false;
				}

			if (*pPos == '\0')
				break;
			}

		pPos++;
		}

	//	Pick a random name

	int iPick = mathRandom(0, iCount-1);
	CString sPick(pName[iPick], iLength[iPick]);

	//	Substitute string

	char szResult[1024];
	char *pDest = szResult;
	pPos = sPick.GetASCIIZPointer();

	while (*pPos)
		{
		switch (*pPos)
			{
			case '%':
				{
				pPos++;
				switch (*pPos)
					{
					case '%':
						*pDest++ = *pPos++;
						break;

					case 's':
						{
						char *pCopy = sSubst.GetASCIIZPointer();
						while (*pCopy)
							*pDest++ = *pCopy++;

						pPos++;
						break;
						}

					case '0':
						*pDest++ = '0' + mathRandom(0, 9);
						pPos++;
						break;

					case '1':
						*pDest++ = '1' + mathRandom(0, 8);
						pPos++;
						break;

					case 'A':
						*pDest++ = 'A' + mathRandom(0, 25);
						pPos++;
						break;

					case 'a':
						*pDest++ = 'a' + mathRandom(0, 25);
						pPos++;
						break;
					}

				break;
				}

			default:
				*pDest++ = *pPos++;
			}
		}

	//	Done

	*pDest = '\0';
	return CString(szResult);
	}

DestructionTypes GetDestructionCause (const CString &sString)

//	GetDestructionCause
//
//	Loads a destruction cause

	{
	DWORD i;

	for (i = 0; i < DESTRUCTION_CAUSES_COUNT; i++)
		if (strEquals(sString, CString(g_pszDestructionCauses[i])))
			return (DestructionTypes)i;

	return killedNone;
	}

CString GetDestructionName (DestructionTypes iCause)

//	GetDestructionName
//
//	Returns the name of the destruction cause

	{
	if (iCause < DESTRUCTION_CAUSES_COUNT)
		return CString(g_pszDestructionCauses[iCause]);
	else
		{
		ASSERT(false);
		return NULL_STR;
		}
	}

int GetDiceCountFromAttribute(const CString &sValue)

//	GetDiceCountFromAttribute
//
//	If sValue is blank, return 1.
//	If sValue is a number, returns the number.
//	Otherwise, assumes that sValue is a dice range and
//	returns the random value.

	{
	if (sValue.IsBlank())
		return 1;
	else
		{
		DiceRange Count;
		if (Count.LoadFromXML(sValue) != NOERROR)
			return 0;

		return Count.Roll();
		}
	}

int GetFrequency (const CString &sValue)

//	GetFrequency
//
//	Returns the frequency from an attribute value

	{
	if (strEquals(sValue, FREQUENCY_COMMON))
		return ftCommon;
	else if (strEquals(sValue, FREQUENCY_UNCOMMON))
		return ftUncommon;
	else if (strEquals(sValue, FREQUENCY_RARE))
		return ftRare;
	else if (strEquals(sValue, FREQUENCY_VERYRARE))
		return ftVeryRare;
	else
		return ftNotRandom;
	}

int GetFrequencyByLevel (const CString &sLevelFrequency, int iLevel)

//	GetFrequencyByLevel
//
//	Returns the frequency for a given level. The frequency is encoded in
//	a string as follows:
//
//	--cur v---- ----- ----- -----
//
//	Where:
//
//	-	Not encountered
//	c	common
//	u	uncommon
//	r	rare
//	v	very rare
//
//	The string is arranged in five sections of five characters each (for
//	human legibility).

	{
	char *pTable = sLevelFrequency.GetPointer();
	if (pTable && pTable[0] == '*')
		return ftCommon;

	ASSERT(iLevel >= 1 && iLevel <= MAX_ITEM_LEVEL);
	int iChar = iLevel + ((iLevel - 1) / 5) - 1;
	if (iChar >= sLevelFrequency.GetLength())
		return ftNotRandom;

	switch (pTable[iChar])
		{
		case 'c':
		case 'C':
			return ftCommon;

		case 'u':
		case 'U':
			return ftUncommon;

		case 'r':
		case 'R':
			return ftRare;

		case 'v':
		case 'V':
			return ftVeryRare;

		default:
			return ftNotRandom;
		}
	}

CString GetItemCategoryName (ItemCategories iCategory)

//	GetItemCategoryName
//
//	Returns the category name

	{
	switch (iCategory)
		{
		case itemcatMisc:
			return CONSTLIT("miscellaneous");

		case itemcatArmor:
			return CONSTLIT("armor");

		case itemcatWeapon:
			return CONSTLIT("weapon");

		case itemcatMiscDevice:
			return CONSTLIT("device");

		case itemcatLauncher:
			return CONSTLIT("launcher");

		case itemcatNano:
			return CONSTLIT("(unused)");

		case itemcatReactor:
			return CONSTLIT("reactor");

		case itemcatShields:
			return CONSTLIT("shield generator");

		case itemcatCargoHold:
			return CONSTLIT("cargo hold");

		case itemcatFuel:
			return CONSTLIT("fuel");

		case itemcatMissile:
			return CONSTLIT("missile or ammo");

		case itemcatDrive:
			return CONSTLIT("drive");

		case itemcatUseful:
			return CONSTLIT("usable");

		default:
			return CONSTLIT("unknown type");
		}
	}

CString GetOrderName (IShipController::OrderTypes iOrder)

//	GetOrderName
//
//	Returns the name of the order

	{
	return CString(g_pszOrderTypes[iOrder]);
	}

IShipController::OrderTypes GetOrderType (const CString &sString)

//	GetOrderType
//
//	Loads an order type

	{
	int iType;

	for (iType = 0; iType < ORDER_TYPES_COUNT; iType++)
		if (strEquals(sString, CString(g_pszOrderTypes[iType])))
			return (IShipController::OrderTypes)iType;

	return IShipController::orderNone;
	}

bool HasModifier (const CString &sModifierList, const CString &sModifier)

//	HadModifier
//
//	Returns TRUE if the item has the given modifier. sModifierList is a list
//	of semicolon (or comma) separated strings. sModifier is a string.

	{
#ifdef DEBUG_HASMODIFIER
	char szBuffer[1024];
	wsprintf(szBuffer, "Looking for: %s; Compare: %s\n", sModifier.GetPointer(), sModifierList.GetPointer());
	::OutputDebugString(szBuffer);
#endif

	//	Blank modifiers always win

	if (sModifier.IsBlank())
		return true;

	//	Loop over modifiers

	char *pPos = sModifierList.GetASCIIZPointer();
	while (*pPos != '\0')
		{
		//	Trim spaces

		while (*pPos == ' ')
			pPos++;

		//	Do we match the modifier?

		char *pFind = sModifier.GetASCIIZPointer();
		while (*pFind != '\0' && *pFind == *pPos)
			{
			pFind++;
			pPos++;
			}

		//	If we matched, then we've got this modifier

		if (*pFind == '\0' && (*pPos == '\0' || *pPos == ';' || *pPos == ',' || *pPos == ' '))
			{
#ifdef DEBUG_HASMODIFIER
			char szBuffer[1024];
			wsprintf(szBuffer, "Looking for: %s; Found: %s\n", sModifier.GetPointer(), sModifierList.GetPointer());
			::OutputDebugString(szBuffer);
#endif
			return true;
			}

		//	Otherwise, skip to the next modifier

		while (*pPos != '\0' && *pPos != ';' && *pPos != ',')
			pPos++;

		if (*pPos == ';' || *pPos == ',')
			pPos++;
		}

	return false;
	}

bool IsEnergyDamage (DamageTypes iType)
	{
	switch (iType)
		{
		case damageLaser:
		case damageParticle:
		case damageIonRadiation:
		case damagePositron:
		case damageAntiMatter:
		case damageGravitonBeam:
		case damageDarkAcid:
		case damageDarkLightning:
			return true;

		default:
			return false;
		}
	}

bool IsMatterDamage (DamageTypes iType)
	{
	switch (iType)
		{
		case damageKinetic:
		case damageBlast:
		case damageThermonuclear:
		case damagePlasma:
		case damageNano:
		case damageSingularity:
		case damageDarkSteel:
		case damageDarkFire:
			return true;

		default:
			return false;
		}
	}

ArmorCompositionTypes LoadArmorComposition (const CString &sString)

//	LoadArmorComposition
//
//	Load armor composition attribute

	{
	if (strEquals(sString, ARMOR_COMPOSITION_METALLIC))
		return compMetallic;
	else if (strEquals(sString, ARMOR_COMPOSITION_CERAMIC))
		return compCeramic;
	else if (strEquals(sString, ARMOR_COMPOSITION_CARBIDE))
		return compCarbide;
	else if (strEquals(sString, ARMOR_COMPOSITION_NANOSCALE))
		return compNanoScale;
	else if (strEquals(sString, ARMOR_COMPOSITION_QUANTUM))
		return compQuantum;
	else if (strEquals(sString, ARMOR_COMPOSITION_GRAVITIC))
		return compGravitic;
	else if (strEquals(sString, ARMOR_COMPOSITION_DARKMATTER))
		return compDarkMatter;
	else
		return compUnknown;
	}

ALERROR LoadCodeBlock (const CString &sCode, ICCItem **retpCode, CString *retsError)

//	LoadCodeBlock
//
//	Loads a block of code

	{
	//	Null case

	if (sCode.IsBlank())
		{
		*retpCode = NULL;
		return NOERROR;
		}

	//	Compile the code

	ICCItem *pCode = g_pUniverse->GetCC().Link(sCode, 0, NULL);
	if (pCode->IsError())
		{
		if (retsError)
			*retsError = pCode->GetStringValue();

		pCode->Discard(&g_pUniverse->GetCC());
		return ERR_FAIL;
		}

	//	Done

	*retpCode = pCode;
	return NOERROR;
	}

ALERROR LoadDamageAdj (CXMLElement *pItem, const CString &sAttrib, int *retiAdj)

//	LoadDamageAdj
//
//	Loads an attribute of the form "x1,x2,x3,..."
//	into an array of damage adjustments.

	{
	ALERROR error;
	int i;
	CIntArray Adj;

	if ((error = pItem->GetAttributeIntegerList(sAttrib, &Adj)))
		return error;

	for (i = 0; i < damageCount; i++)
		retiAdj[i] = (i < Adj.GetCount() ? Adj.GetElement(i) : 0);

	return NOERROR;
	}

ALERROR LoadDamageAdj (CXMLElement *pDesc, int *pDefAdj, int *retiAdj)

//	LoadDamageAdj
//
//	Loads either the damageAdj or the hpBonus attributes into a damage adjument array

	{
	ALERROR error;
	int i;
	CString sValue;

	if (pDesc->FindAttribute(HP_BONUS_ATTRIB, &sValue))
		{
		char *pPos = sValue.GetASCIIZPointer();
		int iDamage = 0;

		while (iDamage < damageCount)
			{
			if (*pPos != '\0')
				{
				//	Skip whitespace

				while (*pPos == ' ')
					pPos++;

				//	A star means no damage

				if (*pPos == '*')
					retiAdj[iDamage] = 0;

				//	Otherwise, expect a number

				else
					{
					BOOL bNull;
					int iValue;
					iValue = strParseInt(pPos, 0, &pPos, &bNull);
					if (bNull)
						return ERR_FAIL;

					if (iValue <= -100)
						iValue = -99;

					retiAdj[iDamage] = (int)(((pDefAdj[iDamage]) * 100.0 / (100.0 + iValue)) + 0.5);
					}

				//	Skip until separator

				while (*pPos != ',' && *pPos != ';' && *pPos != '\0')
					pPos++;

				if (*pPos != '\0')
					pPos++;
				}
			else
				retiAdj[iDamage] = pDefAdj[iDamage];

			iDamage++;
			}
		}
	else if (pDesc->FindAttribute(DAMAGE_ADJ_ATTRIB, &sValue))
		{
		CIntArray DamageAdj;
		if ((error = ParseAttributeIntegerList(sValue, &DamageAdj)))
			return error;

		for (i = 0; i < damageCount; i++)
			if (i < DamageAdj.GetCount())
				retiAdj[i] = DamageAdj.GetElement(i);
			else
				retiAdj[i] = 100;
		}
	else
		{
		for (i = 0; i < damageCount; i++)
			retiAdj[i] = pDefAdj[i];
		}

	return NOERROR;
	}

DWORD LoadExtensionVersion (const CString &sVersion)

//	LoadExtensionVersion
//
//	Returns the extension version (or 0 if this is an unrecognized extension)

	{
	if (strEquals(sVersion, VERSION_095)
			|| strEquals(sVersion, VERSION_095A)
			|| strEquals(sVersion, VERSION_095B)
			|| strEquals(sVersion, VERSION_096)
			|| strEquals(sVersion, VERSION_096A))
		return 1;
	else if (strEquals(sVersion, VERSION_097)
			|| strEquals(sVersion, VERSION_097A)
			|| strEquals(sVersion, VERSION_098)
			|| strEquals(sVersion, VERSION_098A)
			|| strEquals(sVersion, VERSION_098B)
			|| strEquals(sVersion, VERSION_098C)
			|| strEquals(sVersion, VERSION_098D)
			|| strEquals(sVersion, VERSION_099)
			|| strEquals(sVersion, VERSION_099A)
			|| strEquals(sVersion, VERSION_099B)
			|| strEquals(sVersion, VERSION_099C))
		return 2;
	else
		return 0;
	}

DWORD LoadNameFlags (CXMLElement *pDesc)

//	LoadNameFlags
//
//	Returns flags word with NounFlags

	{
	DWORD dwFlags = 0;

	if (pDesc->GetAttributeBool(DEFINITE_ARTICLE_ATTRIB))
		dwFlags |= nounDefiniteArticle;
	if (pDesc->GetAttributeBool(FIRST_PLURAL_ATTRIB))
		dwFlags |= nounFirstPlural;
	if (pDesc->GetAttributeBool(ES_PLURAL_ATTRIB))
		dwFlags |= nounPluralES;
	if (pDesc->GetAttributeBool(CUSTOM_PLURAL_ATTRIB))
		dwFlags |= nounCustomPlural;
	if (pDesc->GetAttributeBool(SECOND_PLURAL_ATTRIB))
		dwFlags |= nounSecondPlural;
	if (pDesc->GetAttributeBool(VOWEL_ARTICLE_ATTRIB))
		dwFlags |= nounVowelArticle;
	if (pDesc->GetAttributeBool(NO_ARTICLE_ATTRIB))
		dwFlags |= nounNoArticle;

	return dwFlags;
	}

COLORREF LoadRGBColor (const CString &sString)

//	LoadRGBColor
//
//	Returns a 16-bit color from an RGB triplet

	{
	char *pPos = sString.GetASCIIZPointer();
	int iRed = strParseInt(pPos, 0, &pPos, NULL); if (*pPos) pPos++;
	int iGreen = strParseInt(pPos, 0, &pPos, NULL); if (*pPos) pPos++;
	int iBlue = strParseInt(pPos, 0, &pPos, NULL);

	return CGImage::RGBColor(iRed, iGreen, iBlue);
	}

COLORREF LoadCOLORREF (const CString &sString)

//	LoadCOLORREF
//
//	Returns a 32-bit color from an RGB triplet

	{
	char *pPos = sString.GetASCIIZPointer();
	int iRed = strParseInt(pPos, 0, &pPos, NULL); if (*pPos) pPos++;
	int iGreen = strParseInt(pPos, 0, &pPos, NULL); if (*pPos) pPos++;
	int iBlue = strParseInt(pPos, 0, &pPos, NULL);

	return CGImage::RGBColor(iRed, iGreen, iBlue);
	}

DWORD LoadUNID (SDesignLoadCtx &Ctx, const CString &sString)

//	LoadUNID
//
//	Returns an UNID either relative (@xxx) or absolute

	{
	if (Ctx.pExtension)
		{
		char *pPos = sString.GetASCIIZPointer();
		if (*pPos == '@')
			{
			WORD wLow = LOWORD(Ctx.pExtension->dwUNID) + (WORD)strParseIntOfBase(pPos+1, 16, 0, NULL, NULL);
			return MAKELONG(wLow, HIWORD(Ctx.pExtension->dwUNID));
			}
		else
			return strToInt(sString, 0, NULL);
		}
	else
		return strToInt(sString, 0, NULL);
	}

int NLCompare (CStringArray &Input, CStringArray &Pattern)

//	NLCompare
//
//	Compares the two word collection.
//
//	For each word in Input that matches a word in Pattern
//	exactly, we return +10
//
//	For each word in Input that matches a word in Pattern
//	partially, we return +2
//
//	For each word in Pattern in excess of the number of words
//	in Input, we return -1

	{
	int i, j;
	int iResult = 0;

	for (i = 0; i < Input.GetCount(); i++)
		{
		//	If this word in the input exactly matches one of the words
		//	in the pattern, then +10

		bool bMatched = false;
		for (j = 0; j < Pattern.GetCount(); j++)
			if (strEquals(Input.GetStringValue(i), Pattern.GetStringValue(j)))
				{
				bMatched = true;
				iResult += 10;
				break;
				}

		if (bMatched)
			continue;

		//	If this word in the input is a number, and if the number as
		//	a roman numeral matches one of the words in the pattern
		//	exactly, then +5

		int iNumber = strToInt(Input.GetStringValue(i), -1, NULL);
		if (iNumber > 0)
			{
			CString sRomanNumeral = strRomanNumeral(iNumber);
			for (j = 0; j < Pattern.GetCount(); j++)
				if (strEquals(sRomanNumeral, Pattern.GetStringValue(j)))
					{
					bMatched = true;
					iResult += 5;
					break;
					}
			}

		if (bMatched)
			continue;

		//	If this word in the input partially matches one of the words
		//	in the pattern, then +2

		for (j = 0; j < Pattern.GetCount(); j++)
			if (strFind(Pattern.GetStringValue(j), Input.GetStringValue(i)) != -1)
				{
				bMatched = true;
				iResult += 2;
				break;
				}

		if (bMatched)
			continue;
		}

	//	Adjust for length

	if (Input.GetCount() < Pattern.GetCount())
		iResult -= 1;

	return iResult;
	}

void NLSplit (const CString &sPhrase, CStringArray *retArray)

//	NLSplit
//
//	Splits a phrase into words for comparison

	{
	//	Generate a list of strings that we will search for

	char *pPos = sPhrase.GetASCIIZPointer();
	CString sToken;
	CString sPrevAppend;
	while (true)
		{
		if ((*pPos >= 'a' && *pPos <= 'z')
				|| (*pPos >= 'A' && *pPos <= 'Z')
				|| (*pPos >= '0' && *pPos <= '9'))
			{
			sToken.Append(CString(pPos, 1));
			}
		else
			{
			if (!sToken.IsBlank())
				{
				//	Add this word to the list

				retArray->AppendString(sToken, NULL);

				//	If we had a hyphen previously, then we append the
				//	concatenation

				if (!sPrevAppend.IsBlank())
					{
					sPrevAppend.Append(sToken);
					retArray->AppendString(sPrevAppend, NULL);
					sPrevAppend = NULL_STR;
					}

				//	If this is a hyphen, then remember this word for
				//	future appending

				if (*pPos == '-')
					sPrevAppend = sToken;

				//	Reset token

				sToken = NULL_STR;
				}

			//	If this is the end, then we're done

			if (*pPos == '\0')
				break;
			}

		pPos++;
		}
	}

Metric RangeIndex2Range (int iIndex)

//	RangeIndex2Range
//
//	Returns the range for the given range index (0-8)

	{
	ASSERT(iIndex >= 0 && iIndex < RANGE_INDEX_COUNT);
	return g_RangeIndex[iIndex];
	}

CString ReactorPower2String (int iPower)

//	ReactorPower2String
//
//	Generates a string for reactor power

	{
	int iReactorPower = (iPower + 5) / 10;

	if (iReactorPower < 1000)
		return strPatternSubst(CONSTLIT("%dMW"), iReactorPower);
	else
		{
		int iGW = iReactorPower / 1000;
		int iMW = iReactorPower % 1000;

		if (iMW == 0)
			return strPatternSubst(CONSTLIT("%dGW"), iGW);
		else
			return strPatternSubst(CONSTLIT("%d.%dGW"),	iGW, (iMW + 50) / 100);
		}
	}

void ReportCrashObj (CString *retsMessage, CSpaceObject *pCrashObj)

//	ReportCrashObj
//
//	Include information about the given object

	{
	CString sName = CONSTLIT("unknown");
	CString sType = CONSTLIT("unknown");
	CString sData;

	if (pCrashObj == NULL)
		pCrashObj = g_pProgramObj;

	if (pCrashObj)
		{
		try
			{
			sName = pCrashObj->GetName();
			sType = pCrashObj->GetObjClassName();
			}
		catch (...)
			{
			}

		try
			{
			sData = pCrashObj->DebugCrashInfo();
			}
		catch (...)
			{
			sData = CONSTLIT("error obtaining crash info\r\n");
			}
		}

	retsMessage->Append(strPatternSubst(CONSTLIT("obj class: %s\r\n"), sType.GetASCIIZPointer()));
	retsMessage->Append(strPatternSubst(CONSTLIT("obj name: %s\r\n"), sName.GetASCIIZPointer()));
	retsMessage->Append(strPatternSubst(CONSTLIT("obj pointer: %x\r\n"), pCrashObj));
	retsMessage->Append(sData);
	}
