//	CSystem.cpp
//
//	CSystem class

#include "PreComp.h"
#include "Kernel.h"
#include "math.h"

#ifdef DEBUG
//#define DEBUG_STRESS_TEST
//#define DEBUG_STATION_TABLES
//#define DEBUG_STATION_PLACEMENT
#endif

#define GROUP_TAG						CONSTLIT("Group")
#define LABEL_TAG						CONSTLIT("Label")
#define VARIANTS_TAG					CONSTLIT("Variants")
#define SIBLINGS_TAG					CONSTLIT("Siblings")
#define PRIMARY_TAG						CONSTLIT("Primary")
#define ORBITALS_TAG					CONSTLIT("Orbitals")
#define TROJAN_TAG						CONSTLIT("Trojan")
#define ANTI_TROJAN_TAG					CONSTLIT("AntiTrojan")
#define SYSTEM_GROUP_TAG				CONSTLIT("SystemGroup")
#define MARKER_TAG						CONSTLIT("Marker")
#define PARTICLES_TAG					CONSTLIT("Particles")
#define SHIP_TAG						CONSTLIT("Ship")
#define ITEM_TAG						CONSTLIT("Item")
#define ITEMS_TAG						CONSTLIT("Items")
#define ESCORTS_TAG						CONSTLIT("Escorts")
#define RANDOM_LOCATION_TAG				CONSTLIT("RandomLocation")
#define PLACE_RANDOM_STATION_TAG		CONSTLIT("PlaceRandomStation")
#define FILL_RANDOM_LOCATION_TAG		CONSTLIT("FillRandomLocation")
#define FILL_LOCATIONS_TAG				CONSTLIT("FillLocations")
#define RANDOM_STATION_TAG				CONSTLIT("RandomStation")
#define SHIPS_TAG						CONSTLIT("Ships")
#define SPACE_ENVIRONMENT_TAG			CONSTLIT("SpaceEnvironment")
#define INITIAL_DATA_TAG				CONSTLIT("InitialData")
#define EVENTS_TAG						CONSTLIT("Events")
#define ON_CREATE_TAG					CONSTLIT("OnCreate")
#define STARGATE_TAG					CONSTLIT("Stargate")
#define STATION_TAG						CONSTLIT("Station")
#define LOOKUP_TAG						CONSTLIT("Lookup")
#define TABLES_TAG						CONSTLIT("Tables")
#define TABLE_TAG						CONSTLIT("Table")
#define NULL_TAG						CONSTLIT("Null")
#define SATELLITES_TAG					CONSTLIT("Satellites")

#define TYPE_ATTRIB						CONSTLIT("type")
#define STARGATE_ATTRIB					CONSTLIT("stargate")
#define DISTANCE_ATTRIB					CONSTLIT("distance")
#define BODE_DISTANCE_START_ATTRIB		CONSTLIT("BodeDistanceStart")
#define BODE_DISTANCE_END_ATTRIB		CONSTLIT("BodeDistanceEnd")
#define NAME_ATTRIB						CONSTLIT("name")
#define ATTRIBUTES_ATTRIB				CONSTLIT("attributes")
#define PROBABILITY_ATTRIB				CONSTLIT("probability")
#define CHANCE_ATTRIB					CONSTLIT("chance")
#define VARIANT_ATTRIB					CONSTLIT("variant")
#define MIN_ATTRIB						CONSTLIT("min")
#define MAX_ATTRIB						CONSTLIT("max")
#define INTERVAL_ATTRIB					CONSTLIT("interval")
#define MAX_RADIUS_ATTRIB				CONSTLIT("maxRadius")
#define DISTRIBUTION_ATTRIB				CONSTLIT("distribution")
#define NO_MAP_LABEL_ATTRIB				CONSTLIT("noMapLabel")
#define MATCH_ATTRIB					CONSTLIT("match")
#define WRECK_TYPE_ATTRIB				CONSTLIT("wreckType")
#define SOVEREIGN_ATTRIB				CONSTLIT("sovereign")
#define NO_SATELLITES_ATTRIB			CONSTLIT("noSatellites")
#define ECCENTRICITY_ATTRIB				CONSTLIT("eccentricity")
#define ROTATION_ATTRIB					CONSTLIT("rotation")
#define TABLE_ATTRIB					CONSTLIT("table")
#define CLASS_ATTRIB					CONSTLIT("class")
#define ORDERS_ATTRIB					CONSTLIT("orders")
#define PATROL_DIST_ATTRIB				CONSTLIT("patrolDist")
#define COUNT_ATTRIB					CONSTLIT("count")
#define X_OFFSET_ATTRIB					CONSTLIT("xOffset")
#define Y_OFFSET_ATTRIB					CONSTLIT("yOffset")
#define IMAGE_VARIANT_ATTRIB			CONSTLIT("imageVariant")
#define OBJ_NAME_ATTRIB					CONSTLIT("objName")
#define LOCATION_CRITERIA_ATTRIB		CONSTLIT("locationCriteria")
#define STATION_CRITERIA_ATTRIB			CONSTLIT("stationCriteria")
#define LOCATION_ATTRIBS_ATTRIB			CONSTLIT("locationAttribs")
#define NO_RANDOM_ENCOUNTERS_ATTRIB		CONSTLIT("noRandomEncounters")
#define SEPARATE_ENEMIES_ATTRIB			CONSTLIT("separateEnemies")
#define DEBUG_ONLY_ATTRIB				CONSTLIT("debugOnly")
#define PERCENT_ENEMIES_ATTRIB			CONSTLIT("percentEnemies")
#define PERCENT_FULL_ATTRIB				CONSTLIT("percentFull")
#define NO_REINFORCEMENTS_ATTRIB		CONSTLIT("noReinforcements")
#define SHAPE_ATTRIB					CONSTLIT("shape")
#define WIDTH_ATTRIB					CONSTLIT("width")
#define WIDTH_VARIATION_ATTRIB			CONSTLIT("widthVariation")
#define SPAN_ATTRIB						CONSTLIT("span")
#define PATCHES_ATTRIB					CONSTLIT("patchType")
#define PATCH_FREQUENCY_ATTRIB			CONSTLIT("patchFrequency")
#define ENCOUNTERS_ATTRIB				CONSTLIT("encountersCount")
#define CONTROLLER_ATTRIB				CONSTLIT("controller")
#define OFFSET_ATTRIB					CONSTLIT("offset")
#define MAX_SHIPS_ATTRIB				CONSTLIT("maxShips")
#define RADIUS_INC_ATTRIB				CONSTLIT("radiusInc")
#define RADIUS_DEC_ATTRIB				CONSTLIT("radiusDec")
#define ANGLE_ADJ_ATTRIB				CONSTLIT("angleAdj")
#define ANGLE_INC_ATTRIB				CONSTLIT("angleInc")
#define ARC_INC_ATTRIB					CONSTLIT("arcInc")
#define NO_OVERLAP_ATTRIB				CONSTLIT("noOverlap")
#define SPACE_SCALE_ATTRIB				CONSTLIT("spaceScale")
#define TIME_SCALE_ATTRIB				CONSTLIT("timeScale")
#define NO_ARTICLE_ATTRIB				CONSTLIT("noArticle")
#define ANGLE_ATTRIB					CONSTLIT("angle")

#define SPECIAL_ATTRIB_INNER_SYSTEM		CONSTLIT("InnerSystem")
#define SPECIAL_ATTRIB_OUTER_SYSTEM		CONSTLIT("OuterSystem")
#define SPECIAL_ATTRIB_LIFE_ZONE		CONSTLIT("LifeZone")
#define SPECIAL_ATTRIB_NEAR_FRIENDS		CONSTLIT("NearFriends")
#define SPECIAL_ATTRIB_NEAR_ENEMIES		CONSTLIT("NearEnemies")
#define SPECIAL_ATTRIB_NEAR_STATIONS	CONSTLIT("NearStations")

#define RANDOM_ANGLE					CONSTLIT("random")
#define EQUIDISTANT_ANGLE				CONSTLIT("equidistant")
#define INCREMENTING_ANGLE				CONSTLIT("incrementing")

#define ORDER_TYPE_ATTACK				CONSTLIT("attack")
#define ORDER_TYPE_GUARD				CONSTLIT("guard")
#define ORDER_TYPE_GATE					CONSTLIT("gate")
#define ORDER_TYPE_PATROL				CONSTLIT("patrol")
#define ORDER_TYPE_ESCORT				CONSTLIT("escort")
#define ORDER_TYPE_SCAVENGE				CONSTLIT("scavenge")
#define ORDER_TYPE_TRADE_ROUTE			CONSTLIT("trade route")
#define ORDER_TYPE_WANDER				CONSTLIT("wander")
#define ORDER_TYPE_HOLD					CONSTLIT("hold")
#define ORDER_TYPE_MINE					CONSTLIT("mine")
#define ORDER_TYPE_FOLLOW				CONSTLIT("follow")

#define SHAPE_CIRCULAR					CONSTLIT("circular")
#define SHAPE_ARC						CONSTLIT("arc")

#define TYPE_NEBULA						CONSTLIT("nebula")

#define MATCH_ALL						CONSTLIT("*")
#define ENEMY_ATTRIBUTE					CONSTLIT("enemy")
#define FRIEND_ATTRIBUTE				CONSTLIT("friendly")
#define REQUIRE_ENEMY					CONSTLIT("*enemy")
#define REQUIRE_FRIEND					CONSTLIT("*friendly")

//	Minimum distance that two enemy stations can be (in light-seconds)

#define MIN_ENEMY_DIST					30
#define MAX_NEBULAE						1000
#define OVERLAP_DIST					(g_KlicksPerPixel * 400.0)

static char g_ProbabilitiesAttrib[] = "probabilities";
static char g_ScaleAttrib[] = "scale";
static char g_ShowOrbitAttrib[] = "showOrbit";

static char g_AUScale[] = "AU";
static char g_LightMinuteScale[] = "light-minute";
static char g_LightSecondScale[] = "light-second";

//	Debugging Support

#ifdef DEBUG_STRESS_TEST

#define STRESS_ITERATIONS		50

#ifdef DEBUG_STRING_LEAKS
#define START_STRING_LEAK_TEST		CString::DebugMark();
#define STOP_STRING_LEAK_TEST		CString::DebugOutputLeakedStrings();	\
									char szBuffer[1024];					\
									wsprintf(szBuffer, "Total Strings: %d\n", CString::DebugGetStringCount());	\
									::OutputDebugString(szBuffer);
#else
#define START_STRING_LEAK_TEST
#define STOP_STRING_LEAK_TEST
#endif

#define START_STRESS_TEST					\
	for (int k = 0; k < STRESS_ITERATIONS; k++)	\
		{									\
		START_STRING_LEAK_TEST

#define STOP_STRESS_TEST					\
		if (k < (STRESS_ITERATIONS-1))		\
			{								\
			delete pSystem;					\
			STOP_STRING_LEAK_TEST			\
			}								\
		}

#else

#define START_STRESS_TEST
#define STOP_STRESS_TEST

#endif

#ifdef DEBUG_STATION_PLACEMENT
#define STATION_PLACEMENT_OUTPUT(x)		::OutputDebugString(x)
#else
#define STATION_PLACEMENT_OUTPUT(x)
#endif

//	Classes and structures

class CLabelDesc : public CObject
	{
	public:
		CLabelDesc (void) : CObject(NULL) { }

		CSystem::Orbit OrbitDesc;
		CString sAttributes;
	};

bool CheckForOverlap (SSystemCreateCtx *pCtx, const CVector &vPos);
int ComputeLocationWeight (SSystemCreateCtx *pCtx, 
						   const CString &sLocationAttribs,
						   const CVector &vPos,
						   const CString &sAttrib, 
						   int iMatchStrength);
int ComputeStationWeight (SSystemCreateCtx *pCtx, CStationType *pType, const CString &sAttrib, int iMatchStrength);
int ComputeWeightAdjFromMatchStrength (bool bHasAttrib, int iMatchStrength);
ALERROR CreateAppropriateStationAtRandomLocation (SSystemCreateCtx *pCtx, 
												  CXMLElement *pDesc, 
												  const CString &sAdditionCriteria,
												  bool *retbEnemy = NULL);
ALERROR CreateLabel (SSystemCreateCtx *pCtx,
					 CXMLElement *pObj,
					 const CSystem::Orbit &OrbitDesc);
ALERROR CreateObjectAtRandomLocation (SSystemCreateCtx *pCtx, CXMLElement *pDesc);
ALERROR CreateOrbitals (SSystemCreateCtx *pCtx, 
						CXMLElement *pObj, 
						const CSystem::Orbit &OrbitDesc);
ALERROR CreateRandomStation (SSystemCreateCtx *pCtx, CXMLElement *pDesc, const CSystem::Orbit &OrbitDesc);
ALERROR CreateRandomStationAtAppropriateLocation (SSystemCreateCtx *pCtx, CXMLElement *pDesc);
ALERROR CreateShipsForStation (CSpaceObject *pStation, CXMLElement *pShips);
ALERROR CreateSiblings (SSystemCreateCtx *pCtx, 
						CXMLElement *pObj, 
						const CSystem::Orbit &OrbitDesc);
ALERROR CreateSpaceEnvironment (SSystemCreateCtx *pCtx, CXMLElement *pDesc, const CSystem::Orbit &OrbitDesc);
void CreateSpaceEnvironmentTile (SSystemCreateCtx *pCtx,
								 const CVector &vPos,
								 int xTile,
								 int yTile,
								 CSpaceEnvironmentType *pEnvironment,
								 CEffectCreator *pPatch,
								 int iPatchFrequency);
ALERROR CreateStargate (SSystemCreateCtx *pCtx, CXMLElement *pDesc, const CSystem::Orbit &OrbitDesc);
ALERROR CreateStation (SSystemCreateCtx *pCtx,
					   CXMLElement *pDesc,
					   const CSystem::Orbit &OrbitDesc,
					   CStation **retpStation = NULL);
ALERROR CreateSystemObject (SSystemCreateCtx *pCtx, 
							CXMLElement *pObj, 
							const CSystem::Orbit &OrbitDesc);
ALERROR GenerateAngles (SSystemCreateCtx *pCtx, const CString &sAngle, int iCount, Metric *pAngles);
Metric GetScale (CXMLElement *pObj);
int ParseSingleCriteria (const CString &sSingleCriteria, CString *retsAttrib);
void RemoveOverlappingLabels (SSystemCreateCtx *pCtx, Metric rMinDistance);

//	Helper functions

ALERROR ChooseRandomLocation (SSystemCreateCtx *pCtx, 
							  const CString &sCriteria, 
							  CSovereign *pSovereign,
							  CSystem::Orbit *retOrbitDesc, 
							  CString *retsAttribs,
							  int *retiLabelPos = NULL)

//	ChooseRandomLocation
//
//	Returns the orbital position for a random label that
//	matches the desired characteristics in sCriteria. If ERR_NOTFOUND
//	is returned then it means that a label of that characteristic could
//	not be found.
//
//	If pSovereign is specified then we make sure that we don't pick
//	a location near enemies of the sovereign.
//
//	If retiLabelPos is passed-in then we do not automatically remove the label
//	from the list.

	{
	STATION_PLACEMENT_OUTPUT("ChooseRandomLocation\n");

	//	Check labels for overlap

	RemoveOverlappingLabels(pCtx, OVERLAP_DIST);

	//	If there are no more entries in the list then return
	//	with NOTFOUND. This can happen if the design needs to
	//	fill all labels

	int iSize = pCtx->Labels.GetCount();
	if (iSize == 0)
		{
		STATION_PLACEMENT_OUTPUT("   no more labels\n");
		return ERR_NOTFOUND;
		}

	//	Build up a probability table to see which of the entries
	//	in the label list fit the match best.

	int i;
	int *pProbTable = new int[iSize];

	//	Initialize the table

	for (i = 0; i < iSize; i++)
		pProbTable[i] = 1000;

	//	Loop over each part of the criteria

	if (!strEquals(sCriteria, MATCH_ALL))
		{
		CStringArray AttribMatch;
		strDelimit(sCriteria, ',', 0, &AttribMatch);
		for (i = 0; i < AttribMatch.GetCount(); i++)
			{
			CString sAttrib;
			int iMatchStrength = ParseSingleCriteria(AttribMatch.GetStringValue(i), &sAttrib);

			for (int j = 0; j < iSize; j++)
				{
				CLabelDesc *pLabelDesc = (CLabelDesc *)pCtx->Labels.GetObject(j);

				//	Adjust probability based on location attributes

				if (pProbTable[j])
					{
					int iAdj = ComputeLocationWeight(pCtx, 
							pLabelDesc->sAttributes,
							pLabelDesc->OrbitDesc.GetObjectPos(),
							sAttrib, 
							iMatchStrength);
					pProbTable[j] = (pProbTable[j] * iAdj) / 1000;
					}
				}
			}
		}

	//	Add up the total probabilities in the table

	int iTotal = 0;
	for (i = 0; i < iSize; i++)
		iTotal += pProbTable[i];

	//	If no entries match, then we're done

	if (iTotal == 0)
		{
		delete [] pProbTable;

		STATION_PLACEMENT_OUTPUT("   no locations match criteria\n");
		return ERR_NOTFOUND;
		}

	//	Keep trying until we find something or until we run out of tries

	int iTries = 10;
	int iPos;
	while (iTries)
		{
		//	Pick a random entry in the table

		iPos = 0;
		int iRoll = mathRandom(0, iTotal - 1);

		//	Get the position

		while (pProbTable[iPos] <= iRoll)
			iRoll -= pProbTable[iPos++];

		//	If we're not done trying, see if this position is near enemies
		//	if it is, then pick again

		if (pSovereign && iTries > 1)
			{
			bool bRetry = false;
			CLabelDesc *pDesc = (CLabelDesc *)pCtx->Labels.GetObject(iPos);

			for (int j = 0; j < pCtx->pSystem->GetObjectCount(); j++)
				{
				CSpaceObject *pObj = pCtx->pSystem->GetObject(j);

				if (pObj 
						&& pObj->GetScale() == scaleStructure
						&& pObj->GetSovereign()
						&& pObj->GetSovereign()->IsEnemy(pSovereign))
					{
					CVector vDist = pDesc->OrbitDesc.GetObjectPos() - pObj->GetPos();
					Metric rDist2 = (vDist.Length2() / (LIGHT_SECOND * LIGHT_SECOND));

					//	If we're too close to an enemy then retry

					if (rDist2 < MIN_ENEMY_DIST * MIN_ENEMY_DIST)
						{
						STATION_PLACEMENT_OUTPUT("   location too close to enemies...retrying\n");
						bRetry = true;
						break;
						}
					}
				}

			if (bRetry)
				{
				iTries--;
				continue;
				}
			}

		//	We found something

		break;
		}

	//	Get the orbit

	CLabelDesc *pDesc = (CLabelDesc *)pCtx->Labels.GetObject(iPos);
	*retOrbitDesc = pDesc->OrbitDesc;
	*retsAttribs = pDesc->sAttributes;

	//	Delete the entry so that it isn't found again

	if (retiLabelPos == NULL)
		{
		pCtx->Labels.RemoveObject(iPos);
		STATION_PLACEMENT_OUTPUT("-remove label\n");
		}
	else
		*retiLabelPos = iPos;

	//	Done

	delete [] pProbTable;

#ifdef DEBUG_STATION_PLACEMENT
	{
	char szBuffer[1024];
	wsprintf(szBuffer, "   found random location (%s)\n", retsAttribs->GetASCIIZPointer());
	::OutputDebugString(szBuffer);
	}
#endif

	return NOERROR;
	}

ALERROR ChooseRandomStation (SSystemCreateCtx *pCtx, 
							 const CString &sCriteria, 
							 const CString &sLocationAttribs,
							 const CVector &vPos,
							 bool bSeparateEnemies,
							 CStationType **retpType)

//	ChooseRandomStation
//
//	Picks a random station to create. The station is appropriate to the level
//	of the system and to the given criteria.

	{
#ifdef DEBUG_STATION_PLACEMENT
	{
	char szBuffer[1024];
	wsprintf(szBuffer, "ChooseRandomStation (%s)\n", sCriteria.GetASCIIZPointer());
	::OutputDebugString(szBuffer);
	}
#endif

	CIDTable EnemiesAtLocation(FALSE, TRUE);
	int iLevel = pCtx->pSystem->GetLevel();

	//	Build up a probability table that gives the chance to create
	//	each station type

	int i;
	int iSize = g_pUniverse->GetStationTypeCount();
	int *pProbTable = new int[iSize];

	//	Initialize the table

	for (i = 0; i < iSize; i++)
		{
		CStationType *pType = g_pUniverse->GetStationType(i);
		pProbTable[i] = pType->GetFrequencyForSystem(pCtx->pSystem);
		}

	//	Loop over each station type and adjust for the location that
	//	we want to create the station at

	if (!strEquals(sLocationAttribs, MATCH_ALL))
		{
		for (i = 0; i < iSize; i++)
			{
			if (pProbTable[i])
				{
				CStationType *pType = g_pUniverse->GetStationType(i);
				if (!strEquals(pType->GetLocationCriteria(), MATCH_ALL))
					{
					CStringArray AttribMatch;
					strDelimit(pType->GetLocationCriteria(), ',', 0, &AttribMatch);
					for (int j = 0; j < AttribMatch.GetCount(); j++)
						{
						CString sAttrib;
						int iMatchStrength = ParseSingleCriteria(AttribMatch.GetStringValue(j), &sAttrib);

						int iAdj = ComputeLocationWeight(pCtx,
								sLocationAttribs,
								vPos,
								sAttrib,
								iMatchStrength);
						pProbTable[i] = (pProbTable[i] * iAdj) / 1000;
						}
					}

				//	If we want to separate enemies, then see if there are any
				//	enemies of this station type at this location.

				if (bSeparateEnemies)
					{
					int iEnemiesHere;
					if (EnemiesAtLocation.Lookup(pType->GetSovereign()->GetUNID(), 
							(CObject **)&iEnemiesHere) != NOERROR)
						{
						iEnemiesHere = FALSE;
						for (int j = 0; j < pCtx->pSystem->GetObjectCount(); j++)
							{
							CSpaceObject *pObj = pCtx->pSystem->GetObject(j);

							if (pObj 
									&& pObj->GetScale() == scaleStructure
									&& pObj->GetSovereign()
									&& pObj->GetSovereign()->IsEnemy(pType->GetControllingSovereign()))
								{
								CVector vDist = vPos - pObj->GetPos();
								Metric rDist2 = (vDist.Length2() / (LIGHT_SECOND * LIGHT_SECOND));

								//	If we're too close to an enemy then retry

								if (rDist2 < MIN_ENEMY_DIST * MIN_ENEMY_DIST)
									{
									iEnemiesHere = TRUE;

									//	If there are enemies at this location for this type, then
									//	remember it (so we don't have to do a search again). If there
									//	aren't enemies, then we don't remember it because we may
									//	subsequently add a new enemy.

									EnemiesAtLocation.AddEntry(pType->GetSovereign()->GetUNID(),
											(CObject *)iEnemiesHere);
									break;
									}
								}
							}

						}

					if (iEnemiesHere)
						pProbTable[i] = 0;
					}
				}
			}
		}

	//	Loop over each part of the criteria and refine the table

	if (!strEquals(sCriteria, MATCH_ALL))
		{
		CStringArray AttribMatch;
		strDelimit(sCriteria, ',', 0, &AttribMatch);
		for (i = 0; i < AttribMatch.GetCount(); i++)
			{
			CString sAttrib;
			int iMatchStrength = ParseSingleCriteria(AttribMatch.GetStringValue(i), &sAttrib);

			for (int j = 0; j < iSize; j++)
				{
				CStationType *pType = g_pUniverse->GetStationType(j);

				//	Adjust probability

				if (pProbTable[j])
					{
					int iAdj = ComputeStationWeight(pCtx, pType, sAttrib, iMatchStrength);
					pProbTable[j] = (pProbTable[j] * iAdj) / 1000;
					}
				}
			}
		}

	//	Add up the total probabilities in the table

	int iTotal = 0;
	for (i = 0; i < iSize; i++)
		iTotal += pProbTable[i];

	//	If no entries match, then we're done

	if (iTotal == 0)
		{
		delete [] pProbTable;

		STATION_PLACEMENT_OUTPUT("   no appropriate station found for this location\n");
		return ERR_NOTFOUND;
		}

#ifdef DEBUG_STATION_TABLES
	{
	int i;
	char szBuffer[1024];
	wsprintf(szBuffer, "ChooseRandomStation\nCriteria: %s\nLocation: %s\n\n",
			sCriteria.GetASCIIZPointer(),
			sLocationAttribs.GetASCIIZPointer());
	::OutputDebugString(szBuffer);

	//	First list all the stations that are in the list

	for (i = 0; i < iSize; i++)
		{
		if (pProbTable[i])
			{
			CStationType *pType = g_pUniverse->GetStationType(i);
			double rProb = 100.0 * (double)pProbTable[i] / (double)iTotal;
			wsprintf(szBuffer, "%3d.%02d %s\n", 
					(int)rProb,
					((int)(rProb * 100)) % 100,
					pType->GetName().GetASCIIZPointer());

			::OutputDebugString(szBuffer);
			}
		}

	::OutputDebugString("\n");

	//	Then include the stations that are not

#if 0
	for (i = 0; i < iSize; i++)
		{
		if (pProbTable[i] == 0)
			{
			CStationType *pType = g_pUniverse->GetStationType(i);
			wsprintf(szBuffer, "%s\n", pType->GetName().GetASCIIZPointer());
			::OutputDebugString(szBuffer);
			}
		}
#endif
	}
#endif

	//	Pick a random entry in the table

	int iRoll = mathRandom(0, iTotal - 1);
	int iPos = 0;

	//	Get the position

	while (pProbTable[iPos] <= iRoll)
		iRoll -= pProbTable[iPos++];

	//	Found it

	*retpType = g_pUniverse->GetStationType(iPos);
	delete [] pProbTable;

#ifdef DEBUG_STATION_PLACEMENT
	{
	char szBuffer[1024];
	wsprintf(szBuffer, "   chose station: %s\n", (*retpType)->GetName().GetASCIIZPointer());
	::OutputDebugString(szBuffer);
	}
#endif

	return NOERROR;
	}

int ComputeWeightAdjFromMatchStrength (bool bHasAttrib, int iMatchStrength)

//	ComputeWeightAdjFromMatchStrength
//
//	If iMatchStrength is positive, then it means we want a certain
//	attribute. If we have the attribute, then we increase our probability
//	but if we don't have the attribute, our probability is normal.
//
//	If iMatchStrength is negative, then it means we DO NOT want a
//	certain attribute. If we have the attribute, then decrease our
//	probability. Otherwise, our probability is normal.

	{
	switch (iMatchStrength)
		{
		case -4:
			return (bHasAttrib ? 0 : 1000);

		case -3:
			return (bHasAttrib ? 50 : 1000);

		case -2:
			return (bHasAttrib ? 200 : 1000);

		case -1:
			return (bHasAttrib ? 500 : 1000);

		case 1:
			return (bHasAttrib ? 2000 : 1000);

		case 2:
			return (bHasAttrib ? 5000 : 1000);

		case 3:
			return (bHasAttrib ? 5000 : 500);

		case 4:
			return (bHasAttrib ? 5000 : 0);

		default:
			return 1000;
		}

#if 0
	bool bWantsAttrib = (iMatchStrength > 0);
	if (bHasAttrib != bWantsAttrib)
		{
		switch (iMatchStrength)
			{
			case -1:
			case 1:
				return 500;

			case -2:
			case 2:
				return 200;

			case -3:
			case 3:
				return 50;

			case -4:
			case 4:
				return 0;

			default:
				return 1000;
			}
		}
	else
		return 1000;
#endif
	}

int ComputeLocationWeight (SSystemCreateCtx *pCtx, 
						   const CString &sLocationAttribs,
						   const CVector &vPos,
						   const CString &sAttrib, 
						   int iMatchStrength)

//	ComputeLocationWeight
//
//	Computes the weight of the given location if we're looking for
//	locations with the given criteria.

	{
	//	Figure out if the location has this attribute

	bool bHasAttrib;
	if (strEquals(sAttrib, SPECIAL_ATTRIB_INNER_SYSTEM))
		{
		CVector vDist = vPos;
		int iDist = (int)(vDist.Length() / LIGHT_SECOND);
		bHasAttrib = (iDist < 360);
		}
	else if (strEquals(sAttrib, SPECIAL_ATTRIB_LIFE_ZONE))
		{
		CVector vDist = vPos;
		int iDist = (int)(vDist.Length() / LIGHT_SECOND);
		bHasAttrib = (iDist >= 360 && iDist <= 620);
		}
	else if (strEquals(sAttrib, SPECIAL_ATTRIB_OUTER_SYSTEM))
		{
		CVector vDist = vPos;
		int iDist = (int)(vDist.Length() / LIGHT_SECOND);
		bHasAttrib = (iDist > 620);
		}
	else if (strEquals(sAttrib, SPECIAL_ATTRIB_NEAR_STATIONS))
		{
		bool bHasAttrib = false;
		for (int i = 0; i < pCtx->pSystem->GetObjectCount(); i++)
			{
			CSpaceObject *pObj = pCtx->pSystem->GetObject(i);

			if (pObj && pObj->GetScale() == scaleStructure)
				{
				CVector vDist = vPos - pObj->GetPos();
				Metric rDist = (vDist.Length2() / LIGHT_SECOND);

				if (rDist < 30 * 30)
					{
					bHasAttrib = true;
					break;
					}
				}
			}
		}
	else
		bHasAttrib = HasModifier(sLocationAttribs, sAttrib);

	//	Adjust probability based on the match strength

	return ComputeWeightAdjFromMatchStrength(bHasAttrib, iMatchStrength);
	}

int ComputeStationWeight (SSystemCreateCtx *pCtx, CStationType *pType, const CString &sAttrib, int iMatchStrength)

//	ComputeStationWeight
//
//	Returns the weight of this station type given the attribute and match weight

	{
	return ComputeWeightAdjFromMatchStrength(
			HasModifier(pType->GetAttributes(), sAttrib),
			iMatchStrength);
	}

ALERROR DistributeStationsAtRandomLocations (SSystemCreateCtx *pCtx, CXMLElement *pDesc, int iCount)

//	DistributeStationsAtRandomLocations
//
//	Fills several locations with random stations

	{
	ALERROR error;
	int i;

	STATION_PLACEMENT_OUTPUT("DistributeStationsAtRandomLocations\n");

	//	Figure out how many friends and enemies we need to create

	int iEnemies = 0;
	int iFriends = 0;
	int iPercentEnemies = pDesc->GetAttributeInteger(PERCENT_ENEMIES_ATTRIB);
	for (i = 0; i < iCount; i++)
		{
		if (mathRandom(1, 100) <= iPercentEnemies)
			iEnemies++;
		else
			iFriends++;
		}

	//	Create the stations

	for (i = 0; i < iCount; i++)
		{
		if (iEnemies && iFriends)
			{
			bool bEnemy;
			if (error = CreateAppropriateStationAtRandomLocation(pCtx, pDesc, NULL_STR, &bEnemy))
				return error;

			if (bEnemy)
				iEnemies--;
			else
				iFriends--;
			}
		else if (iEnemies)
			{
			if (error = CreateAppropriateStationAtRandomLocation(pCtx, pDesc, REQUIRE_ENEMY))
				return error;

			iEnemies--;
			}
		else if (iFriends)
			{
			if (error = CreateAppropriateStationAtRandomLocation(pCtx, pDesc, REQUIRE_FRIEND))
				return error;

			iFriends--;
			}
		}

	return NOERROR;
	}

ALERROR CreateAppropriateStationAtRandomLocation (SSystemCreateCtx *pCtx, 
												  CXMLElement *pDesc, 
												  const CString &sAdditionalCriteria,
												  bool *retbEnemy)

//	CreateAppropriateStationAtRandomLocation
//
//	Picks a random location and fills it with a randomly chosen station approriate
//	to the location.

	{
	ALERROR error;

	STATION_PLACEMENT_OUTPUT("CreateAppropriateStationAtRandomLocation\n");

	CString sLocationCriteria = pDesc->GetAttribute(LOCATION_CRITERIA_ATTRIB);
	CString sStationCriteria = pDesc->GetAttribute(STATION_CRITERIA_ATTRIB);
	bool bSeparateEnemies = pDesc->GetAttributeBool(SEPARATE_ENEMIES_ATTRIB);

	//	Add addition criteria

	if (!sAdditionalCriteria.IsBlank())
		{
		if (sStationCriteria.IsBlank())
			sStationCriteria = sAdditionalCriteria;
		else
			sStationCriteria = strPatternSubst(CONSTLIT("%s,%s"), sStationCriteria.GetASCIIZPointer(), sAdditionalCriteria.GetASCIIZPointer());
		}

	//	Keep trying for a while to make sure that we find something that fits

	int iTries = 10;
	while (iTries > 0)
		{
		//	Pick a random location that fits the criteria

		int iLabelPos;
		CSystem::Orbit OrbitDesc;
		CString sLocationAttribs;
		if (error = ChooseRandomLocation(pCtx,
				sLocationCriteria,
				NULL,
				&OrbitDesc,
				&sLocationAttribs,
				&iLabelPos))
			{
			if (error == ERR_NOTFOUND)
				return NOERROR;
			else
				return error;
			}

		//	Now look for the most appropriate station to place at the location

		CStationType *pType;
		if (error = ChooseRandomStation(pCtx, 
				sStationCriteria, 
				sLocationAttribs,
				OrbitDesc.GetObjectPos(),
				bSeparateEnemies,
				&pType))
			{
			//	If we couldn't find an appropriate location then try picking
			//	a different location.

			if (error == ERR_NOTFOUND)
				{
				iTries--;
				continue;
				}
			else
				return error;
			}

		//	Remove the label so it doesn't match again

		pCtx->Labels.RemoveObject(iLabelPos);

		//	Remember if this is friend or enemy

		if (retbEnemy)
			*retbEnemy = HasModifier(pType->GetAttributes(), ENEMY_ATTRIBUTE);

		//	Create the station at the location

		if (error = pCtx->pSystem->CreateStation(pCtx,
				pType,
				OrbitDesc.GetObjectPos(),
				OrbitDesc,
				true,
				NULL))
			return error;

		//	No more tries

		break;
		}

	return NOERROR;
	}

ALERROR CreateLabel (SSystemCreateCtx *pCtx,
					 CXMLElement *pObj,
					 const CSystem::Orbit &OrbitDesc)

//	CreateLabel
//
//	Creates a labeled point

	{
	//	Add the Orbit to the list

	CLabelDesc *pLabelDesc = new CLabelDesc;
	pLabelDesc->OrbitDesc = OrbitDesc;
	pLabelDesc->sAttributes = pObj->GetAttribute(ATTRIBUTES_ATTRIB);
	pCtx->Labels.AppendObject(pLabelDesc, NULL);

	STATION_PLACEMENT_OUTPUT("+create label\n");

	return NOERROR;
	}

ALERROR CreateObjectAtRandomLocation (SSystemCreateCtx *pCtx, CXMLElement *pDesc)

//	CreateObjectAtRandomLocation
//
//	Creates one or more objects at a random location

	{
	ALERROR error;

	CSystem::Orbit NewOrbit;
	CString sLocationAttribs;

	if (error = ChooseRandomLocation(pCtx, 
			pDesc->GetAttribute(LOCATION_CRITERIA_ATTRIB),
			NULL,
			&NewOrbit, 
			&pCtx->sLocationAttribs))
		{
		//	NOTFOUND means that an appropriate entry could
		//	not be found. This is not an error since sometimes
		//	we fill-up all labels.

		if (error == ERR_NOTFOUND)
			return NOERROR;
		else
			return error;
		}

	//	Create the object(s)

	for (int i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pDesc->GetContentElement(i);

		if (error = CreateSystemObject(pCtx, pItem, NewOrbit))
			return error;
		}

	//	Done

	pCtx->sLocationAttribs = CString();

	return NOERROR;
	}

ALERROR CreateOrbitals (SSystemCreateCtx *pCtx, 
						CXMLElement *pObj, 
						const CSystem::Orbit &OrbitDesc)

//	CreateOrbitals
//
//	Create multiple objects in orbit around the given center

	{
	ALERROR error;

	//	If we have no elements, then there is nothing to do

	if (pObj->GetContentElementCount() == 0)
		return NOERROR;

	//	Figure out the number of objects to create, ...

	int iCount = GetDiceCountFromAttribute(pObj->GetAttribute(COUNT_ATTRIB));
	if (pObj->GetContentElementCount() > 1)
		iCount = Max(iCount, pObj->GetContentElementCount());

	//	...the distance range

	CString sDistance = pObj->GetAttribute(DISTANCE_ATTRIB);
	CString sBodeDistanceStart = pObj->GetAttribute(BODE_DISTANCE_START_ATTRIB);

	//	...the angle of the objects

	CString sAngle = pObj->GetAttribute(ANGLE_ATTRIB);

	//	...elliptical orbit parameters

	DiceRange EccentricityRange(0, 0, 0);
	CString sEccentricity = pObj->GetAttribute(ECCENTRICITY_ATTRIB);
	if (!sEccentricity.IsBlank())
		{
		if (error = EccentricityRange.LoadFromXML(sEccentricity))
			{
			pCtx->sError = CONSTLIT("Invalid eccentricity in orbitals");
			return error;
			}
		}

	CString sEllipseRotation = pObj->GetAttribute(ROTATION_ATTRIB);

	//	...other parameters

	bool bNoOverlap = pObj->GetAttributeBool(NO_OVERLAP_ATTRIB);

	//	Calculate the number of objects

	int i;

	//	Create an array of position vectors for the objects

	if (iCount > 0)
		{
		Metric *rDistance = new Metric[iCount];
		Metric *rAngle = new Metric[iCount];
		Metric *rEccentricity = new Metric[iCount];
		Metric *rRotation = new Metric[iCount];

		//	Calculate the distance for each object. If the distance is specified
		//	then use that.

		if (!sDistance.IsBlank())
			{
			DiceRange DistanceRange;

			if (error = DistanceRange.LoadFromXML(sDistance))
				{
				pCtx->sError = CONSTLIT("Invalid distance in orbitals");
				return error;
				}

			Metric rScale = GetScale(pObj);

			for (i = 0; i < iCount; i++)
				rDistance[i] = rScale * DistanceRange.Roll();
			}

		//	If Bode distance range specified then compute that

		else if (!sBodeDistanceStart.IsBlank())
			{
			DiceRange Range;

			Range.LoadFromXML(sBodeDistanceStart);
			Metric rDistanceStart = Range.Roll();

			Range.LoadFromXML(pObj->GetAttribute(BODE_DISTANCE_END_ATTRIB));
			Metric rDistanceEnd = Range.Roll();

			Metric rScale = GetScale(pObj);

			if (iCount == 1)
				rDistance[0] = rScale * (rDistanceStart + ((rDistanceEnd - rDistanceStart) / 2.0));
			else if (iCount == 2)
				{
				rDistance[0] = rScale * rDistanceStart;
				rDistance[1] = rScale * rDistanceEnd;
				}
			else
				{
				const Metric K = 1.35;		//	Orbital ratio (for Bode's Law, this
											//	is actually around 2, but we adjust
											//	it so that the planets aren't so far
											//	apart.

				Metric rDen = 1.0;
				Metric rKSum = K;
				for (i = 2; i < iCount; i++)
					{
					rDen += rKSum;
					rKSum *= K;
					}

				Metric rDist = rDistanceStart;
				Metric rDistInc = (rDistanceEnd - rDistanceStart) / rDen;

				for (i = 0; i < iCount; i++)
					{
					rDistance[i] = rScale * rDist;
					rDist += rDistInc;
					rDistInc *= K;
					}
				}
			}

		//	Otherwise, distance is specified in children

		else
			{
			int iSubCount = pObj->GetContentElementCount();

			for (i = 0; i < iCount; i++)
				{
				CXMLElement *pSub = pObj->GetContentElement(i % iSubCount);

				DiceRange DistanceRange;
				if (error = DistanceRange.LoadFromXML(pSub->GetAttribute(DISTANCE_ATTRIB)))
					{
					pCtx->sError = CONSTLIT("Invalid distance in orbitals");
					return error;
					}

				Metric rScale = GetScale(pSub);

				rDistance[i] = rScale * DistanceRange.Roll();
				}
			}

		//	Calculate eccentricity

		for (i = 0; i < iCount; i++)
			{
			rEccentricity[i] = EccentricityRange.Roll() / 100.0;
			if (rEccentricity[i] > 0.99)
				rEccentricity[i] = 0.99;
			}

		//	Calculate rotation angles for each object

		if (!sEllipseRotation.IsBlank())
			{
			if (error = GenerateAngles(pCtx, sEllipseRotation, iCount, rRotation))
				return error;
			}
		else
			{
			for (i = 0; i < iCount; i++)
				rRotation[i] = 0.0;
			}

		//	Calculate the angle for each object. If we want to check for overlap, we
		//	iterate several times to make sure that we have chosen non-overlapping
		//	points.

		bool bConfigurationOK = true;
		int iTries = 10;
		do
			{
			if (error = GenerateAngles(pCtx, sAngle, iCount, rAngle))
				return error;

			//	If any of the positions overlap, then the configuration is not OK

			if (bNoOverlap)
				{
				bConfigurationOK = true;
				for (i = 0; i < iCount; i++)
					{
					CSystem::Orbit NewOrbit(OrbitDesc.GetObjectPos(),
							rDistance[i],
							rEccentricity[i],
							rRotation[i],
							rAngle[i]);

					if (CheckForOverlap(pCtx, NewOrbit.GetObjectPos()))
						{
						bConfigurationOK = false;
						break;
						}
					}
				}
			}
		while (!bConfigurationOK && --iTries > 0);

		//	Create each object

		int iObj = 0;
		int iPos = 0;
		int iLoops = Max(iCount, pObj->GetContentElementCount());
		for (i = 0; i < iLoops; i++)
			{
			CSystem::Orbit NewOrbit(OrbitDesc.GetObjectPos(),
					rDistance[iPos],
					rEccentricity[iPos],
					rRotation[iPos],
					rAngle[iPos]);

			if (error = CreateSystemObject(pCtx, pObj->GetContentElement(iObj), NewOrbit))
				return error;

			iObj = (iObj + 1) % pObj->GetContentElementCount();
			iPos = (iPos + 1) % iCount;
			}

			delete []rDistance;
			delete []rAngle;
			delete []rEccentricity;
			delete []rRotation;
		}

	return NOERROR;
	}

ALERROR CreateRandomStation (SSystemCreateCtx *pCtx, 
							 CXMLElement *pDesc, 
							 const CSystem::Orbit &OrbitDesc)

//	CreateRandomStation
//
//	Creates a random station at the given location

	{
	ALERROR error;

	CString sStationCriteria = pDesc->GetAttribute(STATION_CRITERIA_ATTRIB);
	CString sLocationAttribs = pDesc->GetAttribute(LOCATION_ATTRIBS_ATTRIB);
	if (sLocationAttribs.IsBlank())
		sLocationAttribs = pCtx->sLocationAttribs;

	//	Pick a random station type that fits the criteria

	CStationType *pType;
	if (error = ChooseRandomStation(pCtx, 
			sStationCriteria, 
			sLocationAttribs,
			OrbitDesc.GetObjectPos(),
			false,
			&pType))
		{
		if (error == ERR_NOTFOUND)
			return NOERROR;
		else
			return error;
		}

	//	Create the station at the location

	if (error = pCtx->pSystem->CreateStation(pCtx,
			pType,
			OrbitDesc.GetObjectPos(),
			OrbitDesc,
			true,
			NULL))
		return error;

	return NOERROR;
	}

ALERROR CreateRandomStationAtAppropriateLocation (SSystemCreateCtx *pCtx, CXMLElement *pDesc)

//	CreateRandomStationAtAppropriateLocation
//
//	Picks a random station and then picks a random location appropriate
//	for the given station.

	{
	ALERROR error;

	STATION_PLACEMENT_OUTPUT("CreateRandomStationAtAppropriateLocation\n");

	CString sStationCriteria = pDesc->GetAttribute(STATION_CRITERIA_ATTRIB);
	bool bSeparateEnemies = pDesc->GetAttributeBool(SEPARATE_ENEMIES_ATTRIB);

	//	Keep trying for a while to make sure that we find something that fits

	int iTries = 10;
	while (iTries > 0)
		{
		//	Pick a random station type that fits the criteria

		CStationType *pType;
		if (error = ChooseRandomStation(pCtx, 
				sStationCriteria, 
				MATCH_ALL,
				NullVector,
				false,
				&pType))
			{
			if (error == ERR_NOTFOUND)
				return NOERROR;
			else
				return error;
			}

		//	Get the sovereign for this station (we need it is we want to separate
		//	stations that are enemies).

		CSovereign *pSovereign = NULL;
		if (bSeparateEnemies)
			pSovereign = pType->GetSovereign();

		//	Now look for the most appropriate location to place the station

		CSystem::Orbit OrbitDesc;
		CString sLocationAttribs;
		if (error = ChooseRandomLocation(pCtx, 
				pType->GetLocationCriteria(), 
				pSovereign,
				&OrbitDesc, 
				&sLocationAttribs))
			{
			//	If we couldn't find an appropriate location then try picking
			//	a different kind of station.

			if (error == ERR_NOTFOUND)
				{
				iTries--;
				continue;
				}
			else
				return error;
			}

		//	Create the station at the location

		if (error = pCtx->pSystem->CreateStation(pCtx,
				pType,
				OrbitDesc.GetObjectPos(),
				OrbitDesc,
				true,
				NULL))
			return error;

		//	No more tries

		break;
		}

	return NOERROR;
	}

ALERROR CreateSiblings (SSystemCreateCtx *pCtx, 
						CXMLElement *pObj, 
						const CSystem::Orbit &OrbitDesc)

//	CreateSiblings
//
//	Creates objects in similar orbits

	{
	ALERROR error;
	CString sDistribution;

	if (pObj->GetContentElementCount() == 0)
		return NOERROR;

	//	Get the number of objects to create

	int iCount = GetDiceCountFromAttribute(pObj->GetAttribute(COUNT_ATTRIB));

	//	Create a random distribution

	if (pObj->FindAttribute(DISTRIBUTION_ATTRIB, &sDistribution))
		{
		DiceRange Distribution;
		Distribution.LoadFromXML(sDistribution);
		Metric rScale = GetScale(pObj);

		for (int i = 0; i < iCount; i++)
			{
			CSystem::Orbit SiblingOrbit(OrbitDesc.GetFocus(),
					OrbitDesc.GetSemiMajorAxis() + (rScale * Distribution.Roll()),
					OrbitDesc.GetEccentricity(),
					OrbitDesc.GetRotation(),
					mathDegreesToRadians(mathRandom(0,3599) / 10.0));

			if (error = CreateSystemObject(pCtx, 
					pObj->GetContentElement(0), 
					SiblingOrbit))
				return error;
			}
		}

	//	Create objects with radial and angular offsets

	else
		{
		CString sAttrib;

		//	Load the radial increment

		Metric rScale = GetScale(pObj);
		DiceRange RadiusAdj;
		Metric rRadiusAdjScale;
		if (pObj->FindAttribute(RADIUS_INC_ATTRIB, &sAttrib))
			{
			RadiusAdj.LoadFromXML(sAttrib);
			rRadiusAdjScale = rScale;
			}
		else if (pObj->FindAttribute(RADIUS_DEC_ATTRIB, &sAttrib))
			{
			RadiusAdj.LoadFromXML(sAttrib);
			rRadiusAdjScale = -rScale;
			}
		else
			rRadiusAdjScale = 0.0;

		//	Load the angle or arc increment

		enum IncTypes { incNone, incAngle, incArc };
		IncTypes iAngleInc;
		DiceRange AngleAdj;
		if (pObj->FindAttribute(ARC_INC_ATTRIB, &sAttrib))
			{
			iAngleInc = incArc;
			AngleAdj.LoadFromXML(sAttrib);
			}
		else if (pObj->FindAttribute(ANGLE_ADJ_ATTRIB, &sAttrib))
			{
			iAngleInc = incAngle;
			AngleAdj.LoadFromXML(sAttrib);
			}
		else if (pObj->FindAttribute(ANGLE_INC_ATTRIB, &sAttrib))
			{
			iAngleInc = incAngle;
			AngleAdj.LoadFromXML(sAttrib);
			}
		else
			{
			iAngleInc = incNone;
			}

		//	Loop over count

		for (int i = 0; i < iCount; i++)
			{
			Metric rRadiusAdj = rRadiusAdjScale * (Metric)RadiusAdj.Roll();
			Metric rAngleAdj;
			switch (iAngleInc)
				{
				case incArc:
					{
					//	Convert from linear to angular
					Metric rCirc = (OrbitDesc.GetSemiMajorAxis() + rRadiusAdj);
					rAngleAdj = (rCirc > 0.0 ? ((Metric)AngleAdj.Roll() * rScale) / rCirc : 0.0);
					break;
					}

				case incAngle:
					rAngleAdj = mathDegreesToRadians((360 + AngleAdj.Roll()) % 360);
					break;

				default:
					rAngleAdj = 0.0;
				}

			CSystem::Orbit SiblingOrbit(OrbitDesc.GetFocus(),
					OrbitDesc.GetSemiMajorAxis() + rRadiusAdj,
					OrbitDesc.GetEccentricity(),
					OrbitDesc.GetRotation(),
					OrbitDesc.GetObjectAngle() + rAngleAdj);

			if (error = CreateSystemObject(pCtx,
					pObj->GetContentElement(0),
					SiblingOrbit))
				return error;
			}

#if 0

		//	!!!!!!!!!!!!!!TO DO: Need to deal with count!!!!!!!!!!!!
		Metric rRadiusAdj = 0.0;
		CString sRadiusAdj;
		if (pObj->FindAttribute(RADIUS_INC_ATTRIB, &sRadiusAdj))
			{
			DiceRange RadiusAdj;
			RadiusAdj.LoadFromXML(sRadiusAdj);
			rRadiusAdj = GetScale(pObj) * (Metric)RadiusAdj.Roll();
			}
		else if (pObj->FindAttribute(RADIUS_DEC_ATTRIB, &sRadiusAdj))
			{
			DiceRange RadiusAdj;
			RadiusAdj.LoadFromXML(sRadiusAdj);
			rRadiusAdj = -GetScale(pObj) * (Metric)RadiusAdj.Roll();
			}

		Metric rAngleAdj;
		CString sAngleAdj;
		if (pObj->FindAttribute(ARC_INC_ATTRIB, &sAngleAdj))
			{
			DiceRange ArcAdj;
			ArcAdj.LoadFromXML(sAngleAdj);

			//	Convert from linear to angular
			Metric rCirc = (OrbitDesc.GetSemiMajorAxis() + rRadiusAdj);
			rAngleAdj = (rCirc > 0.0 ? ((Metric)ArcAdj.Roll() * GetScale(pObj)) / rCirc : 0.0);
			}
		else if (pObj->FindAttribute(ANGLE_ADJ_ATTRIB, &sAngleAdj))
			{
			DiceRange AngleAdj;
			AngleAdj.LoadFromXML(sAngleAdj);
			int iAngleAdj = AngleAdj.Roll();
			rAngleAdj = mathDegreesToRadians((360 + iAngleAdj) % 360);
			}
		else
			rAngleAdj = 0.0;

		CSystem::Orbit SiblingOrbit(OrbitDesc.GetFocus(),
				OrbitDesc.GetSemiMajorAxis() + rRadiusAdj,
				OrbitDesc.GetEccentricity(),
				OrbitDesc.GetRotation(),
				OrbitDesc.GetObjectAngle() + rAngleAdj);

		if (error = CreateSystemObject(pCtx,
				pObj->GetContentElement(0),
				SiblingOrbit))
			return error;
#endif
		}

	return NOERROR;
	}

ALERROR CreateShipsForStation (CSpaceObject *pStation, CXMLElement *pShips)

//	CreateShipsForStation
//
//	Creates random ships for a station

	{
	ALERROR error;

	//	Load the generator

	SDesignLoadCtx Ctx;

	IShipGenerator *pGenerator;
	if (error = IShipGenerator::CreateFromXMLAsGroup(Ctx, pShips, &pGenerator))
		{
		ASSERT(false);
		kernelDebugLogMessage("Unable to load ship generator: %s", Ctx.sError.GetASCIIZPointer());
		return error;
		}

	if (error = pGenerator->OnDesignLoadComplete(Ctx))
		{
		ASSERT(false);
		kernelDebugLogMessage("Unable to load ship generator: %s", Ctx.sError.GetASCIIZPointer());
		return error;
		}

	//	Create the ships

	pStation->CreateRandomDockedShips(pGenerator);

	//	Done

	delete pGenerator;
	return NOERROR;
	}

void AdjustBounds (CVector *pUL, CVector *pLR, const CVector &vPos)
	{
	if (vPos.GetX() < pUL->GetX())
		pUL->SetX(vPos.GetX());
	else if (vPos.GetX() > pLR->GetX())
		pLR->SetX(vPos.GetX());

	if (vPos.GetY() < pLR->GetY())
		pLR->SetY(vPos.GetY());
	else if (vPos.GetY() > pUL->GetY())
		pUL->SetY(vPos.GetY());
	}

void ChangeVariation (Metric *pVariation, Metric rMaxVariation)
	{
	Metric rChange = (rMaxVariation / 15.0);

	int iChange = mathRandom(1, 3);
	if (iChange == 1)
		{
		if (*pVariation > 0.0)
			*pVariation -= rChange;
		else
			*pVariation += rChange;
		}
	else if (iChange == 3)
		{
		if (*pVariation < rMaxVariation)
			*pVariation += rChange;
		else
			*pVariation -= rChange;
		}
	}

ALERROR CreateSpaceEnvironment (SSystemCreateCtx *pCtx, CXMLElement *pDesc, const CSystem::Orbit &OrbitDesc)

//	CreateSpaceEnvironment
//
//	Creates a space environment, such as nebulae

	{
	ALERROR error;
	int i;

	//	Figure out what tile we need here

	CSpaceEnvironmentType *pEnvironment = g_pUniverse->FindSpaceEnvironment(pDesc->GetAttributeInteger(TYPE_ATTRIB));
	if (pEnvironment == NULL)
		{
		pCtx->sError = CONSTLIT("Invalid space environment type");
		return ERR_FAIL;
		}

	//	Patches

	CEffectCreator *pPatchType = g_pUniverse->FindEffectType(pDesc->GetAttributeInteger(PATCHES_ATTRIB));
	int iPatchFrequency = GetDiceCountFromAttribute(pDesc->GetAttribute(PATCH_FREQUENCY_ATTRIB));

	//	Keep track of all the places where we've added nebulae

	int xNebulae[MAX_NEBULAE];
	int yNebulae[MAX_NEBULAE];
	int iNebulaeCount = 0;

	//	Fill the appropriate shape

	CString sShape = pDesc->GetAttribute(SHAPE_ATTRIB);
	if (strEquals(sShape, SHAPE_CIRCULAR))
		{
		Metric rHalfWidth = GetDiceCountFromAttribute(pDesc->GetAttribute(WIDTH_ATTRIB)) * LIGHT_SECOND / 2.0;
		Metric rMaxVariation = GetDiceCountFromAttribute(pDesc->GetAttribute(WIDTH_VARIATION_ATTRIB)) * rHalfWidth / 100.0;
		Metric rHalfVariation = rMaxVariation / 2.0;

		CVector vCenter = OrbitDesc.GetFocus();

		//	Compute the inner and outer radius for each degree around the orbit
		//	Also, while we're at it, we compute the upper-left and lower-right
		//	bounds of the nebula

		Metric rOuterRadius[360];
		Metric rInnerRadius[360];
		CVector vUL = vCenter;
		CVector vLR = vCenter;

		Metric rOuterVariation = rHalfVariation;
		Metric rInnerVariation = rHalfVariation;
		for (i = 0; i < 360; i++)
			{
			Metric rAngle = i * g_Pi / 180.0;
			Metric rRadius;
			OrbitDesc.GetPointAndRadius(rAngle, &rRadius);

			rOuterRadius[i] = rRadius + rHalfWidth + rOuterVariation - rHalfVariation;
			rInnerRadius[i] = rRadius - rHalfWidth + rInnerVariation - rHalfVariation;

			//	Adjust inner and outer radius in a random walk

			ChangeVariation(&rOuterVariation, rMaxVariation);
			ChangeVariation(&rInnerVariation, rMaxVariation);

			//	Adjust bounds

			AdjustBounds(&vUL, &vLR, vCenter + PolarToVector(i, rOuterRadius[i]));
			}

		//	Now iterate over every tile in bounds and see if it is within
		//	the band that we have defined.

		int xTileStart, yTileStart, xTileEnd, yTileEnd;
		VectorToTile(vUL, &xTileStart, &yTileStart);
		VectorToTile(vLR, &xTileEnd, &yTileEnd);

		int x, y;
		for (x = xTileStart; x <= xTileEnd; x++)
			for (y = yTileStart; y <= yTileEnd; y++)
				{
				CVector vTile = TileToVector(x, y);
				CVector vRadius = vTile - vCenter;

				Metric rRadius;
				int iAngle = VectorToPolar(vRadius, &rRadius);

				if (rRadius > rInnerRadius[iAngle] && rRadius < rOuterRadius[iAngle])
					{
					CreateSpaceEnvironmentTile(pCtx, vTile, x, y, pEnvironment, pPatchType, iPatchFrequency);

					if (iNebulaeCount < MAX_NEBULAE - 1)
						{
						xNebulae[iNebulaeCount] = x;
						yNebulae[iNebulaeCount++] = y;
						}
					}
				}
		}
	else if (strEquals(sShape, SHAPE_ARC))
		{
		Metric rHalfWidth = GetDiceCountFromAttribute(pDesc->GetAttribute(WIDTH_ATTRIB)) * LIGHT_SECOND / 2.0;
		Metric rMaxVariation = GetDiceCountFromAttribute(pDesc->GetAttribute(WIDTH_VARIATION_ATTRIB)) * rHalfWidth / 100.0;
		Metric rHalfVariation = rMaxVariation / 2.0;
		int iHalfSpan = GetDiceCountFromAttribute(pDesc->GetAttribute(SPAN_ATTRIB)) / 2;

		CVector vCenter = OrbitDesc.GetFocus();
		int iArcCenterAngle = (((int)((180.0 * OrbitDesc.GetObjectAngle() / g_Pi) + 0.5)) % 360);

		//	Compute the inner and outer radius for each degree around the orbit
		//	Also, while we're at it, we compute the upper-left and lower-right
		//	bounds of the nebula

		Metric rOuterRadius[360];
		Metric rInnerRadius[360];
		CVector vUL = CVector(g_InfiniteDistance, -g_InfiniteDistance);
		CVector vLR = CVector(-g_InfiniteDistance, g_InfiniteDistance);

		Metric rOuterVariation = 0.0;
		Metric rInnerVariation = rMaxVariation;
		for (i = 0; i < 360; i++)
			{
			rOuterRadius[i] = 0.0;
			rInnerRadius[i] = 0.0;
			}

		for (i = -iHalfSpan; i <= iHalfSpan; i++)
			{
			int iAngle = ((iArcCenterAngle + i + 360) % 360);

			Metric rAngle = iAngle * g_Pi / 180.0;
			Metric rRadius;
			OrbitDesc.GetPointAndRadius(rAngle, &rRadius);

			rOuterRadius[iAngle] = rRadius + rHalfWidth + rOuterVariation - rHalfVariation;
			rInnerRadius[iAngle] = rRadius - rHalfWidth + rInnerVariation - rHalfVariation;

			//	Adjust inner and outer radius in a random walk

			ChangeVariation(&rOuterVariation, rMaxVariation);
			ChangeVariation(&rInnerVariation, rMaxVariation);

			//	Adjust bounds

			AdjustBounds(&vUL, &vLR, vCenter + PolarToVector(iAngle, rOuterRadius[iAngle]));
			AdjustBounds(&vUL, &vLR, vCenter + PolarToVector(iAngle, rInnerRadius[iAngle]));
			}

		//	Now iterate over every tile in bounds and see if it is within
		//	the band that we have defined.

		int xTileStart, yTileStart, xTileEnd, yTileEnd;
		VectorToTile(vUL, &xTileStart, &yTileStart);
		VectorToTile(vLR, &xTileEnd, &yTileEnd);

		int x, y;
		for (x = xTileStart; x <= xTileEnd; x++)
			for (y = yTileStart; y <= yTileEnd; y++)
				{
				CVector vTile = TileToVector(x, y);
				CVector vRadius = vTile - vCenter;

				Metric rRadius;
				int iAngle = VectorToPolar(vRadius, &rRadius);

				if (rRadius > rInnerRadius[iAngle] && rRadius < rOuterRadius[iAngle])
					{
					CreateSpaceEnvironmentTile(pCtx, vTile, x, y, pEnvironment, pPatchType, iPatchFrequency);

					if (iNebulaeCount < MAX_NEBULAE - 1)
						{
						xNebulae[iNebulaeCount] = x;
						yNebulae[iNebulaeCount++] = y;
						}
					}
				}
		}
	else
		{
		pCtx->sError = CONSTLIT("Invalid space environment shape");
		return ERR_FAIL;
		}

	//	Create random encounters

	int iEncounters = GetDiceCountFromAttribute(pDesc->GetAttribute(ENCOUNTERS_ATTRIB));

	CXMLElement *pEncounter = NULL;
	if (pDesc->GetContentElementCount())
		pEncounter = pDesc->GetContentElement(0);

	if (pEncounter && iNebulaeCount > 0)
		{
		if (iEncounters == 0)
			iEncounters = 1;

		for (i = 0; i < iEncounters; i++)
			{
			//	Pick a random tile

			int iTile = mathRandom(0, iNebulaeCount-1);
			CVector vCenter = TileToVector(xNebulae[iTile], yNebulae[iTile]);
			CVector vOffset(
					g_KlicksPerPixel * (mathRandom(0, seaTileSize) - (seaTileSize / 2)),
					g_KlicksPerPixel * (mathRandom(0, seaTileSize) - (seaTileSize / 2)));

			CSystem::Orbit NewOrbit(vCenter + vOffset, 0.0);

			//	Create the object

			if (error = CreateSystemObject(pCtx, pEncounter, NewOrbit))
				return error;
			}
		}

	return NOERROR;
	}

void CreateSpaceEnvironmentTile (SSystemCreateCtx *pCtx,
								 const CVector &vPos,
								 int xTile,
								 int yTile,
								 CSpaceEnvironmentType *pEnvironment,
								 CEffectCreator *pPatch,
								 int iPatchFrequency)
	{
	pCtx->pSystem->SetSpaceEnvironment(xTile, yTile, pEnvironment);

	//	Create some patches

	if (pPatch && mathRandom(1, 100) <= iPatchFrequency)
		{
		CVector vOffset(
				g_KlicksPerPixel * (mathRandom(0, seaTileSize) - (seaTileSize / 2)),
				g_KlicksPerPixel * (mathRandom(0, seaTileSize) - (seaTileSize / 2)));

		CStaticEffect::Create(pPatch,
				pCtx->pSystem,
				vPos + vOffset,
				NULL);
		}
	}

ALERROR CreateStargate (SSystemCreateCtx *pCtx, CXMLElement *pDesc, const CSystem::Orbit &OrbitDesc)

//	CreateStargate
//
//	Creates a stargate (and sub-objects) based on the topology

	{
	ALERROR error;

	//	Get the name of this stargate

	CString sStargate = pDesc->GetAttribute(OBJ_NAME_ATTRIB);
	if (sStargate.IsBlank())
		{
		pCtx->sError = CONSTLIT("<Stargate> must specify objName");
		return ERR_FAIL;
		}

	//	Figure out the destination node for the stargate

	CString sDestEntryPoint;
	CTopologyNode *pDestNode = pCtx->pSystem->GetStargateDestination(sStargate, &sDestEntryPoint);

	//	If we don't have a destination node, then it means that this is an optional stargate
	//	so we exit with no error.

	if (pDestNode == NULL)
		return NOERROR;

	//	We create the stargate (as if the element were a <Station> element)

	CStation *pStation;
	if (error = CreateStation(pCtx, pDesc, OrbitDesc, &pStation))
		return error;

	if (pStation == NULL)
		{
		pCtx->sError = CONSTLIT("<Stargate> must be a station");
		return ERR_FAIL;
		}

	//	Set stargate properties (note: CreateStation also looks at objName and adds the name
	//	to the named-objects system table.)

	pStation->SetStargate(pDestNode->GetID(), sDestEntryPoint);

	//	If we haven't already set the name, set the name of the stargate
	//	to include the name of the destination system

	if (pStation->GetName(NULL).IsBlank())
		pStation->SetName(strPatternSubst(CONSTLIT("%s Stargate"), pDestNode->GetSystemName().GetASCIIZPointer()));

	return NOERROR;
	}

ALERROR CreateSystemObject (SSystemCreateCtx *pCtx, 
							CXMLElement *pObj, 
							const CSystem::Orbit &OrbitDesc)

//	CreateSystemObject
//
//	Create an object in the system. The pObj element may be one of
//	the following tags:
//
//		<AntiTrojan ...>
//		<Encounter ...>
//		<Group ...>
//		<Label ...>
//		<LabelStation ...>
//		<Lookup ...>
//		<Null/>
//		<Orbitals ...>
//		<Particles ...>
//		<Primary ...>
//		<Siblings ...>
//		<SpaceEnvironment ...>
//		<Station ...>
//		<Table ...>
//		<Trojan ...>
//		<Variants ...>

	{
	ALERROR error;
	CString sTag = pObj->GetTag();

	//	Nothing to do if this is a debug-only object and we are not in
	//	debug mode.

	if (pObj->GetAttributeBool(DEBUG_ONLY_ATTRIB) && !g_pUniverse->InDebugMode())
		return NOERROR;

	//	See if we've got a probability

	int iProb = pObj->GetAttributeInteger(PROBABILITY_ATTRIB);
	if (iProb > 0 && mathRandom(1, 100) > iProb)
		return NOERROR;

	//	Act based on the tag

	if (strEquals(sTag, STATION_TAG))
		{
		if (error = CreateStation(pCtx, pObj, OrbitDesc))
			return error;
		}
	else if (strEquals(sTag, TABLE_TAG))
		{
		CRandomEntryResults Results;

		if (error = CRandomEntryGenerator::Generate(pObj, Results))
			{
			pCtx->sError = CONSTLIT("<Table> error");
			return error;
			}

		for (int i = 0; i < Results.GetCount(); i++)
			{
			CXMLElement *pResult = Results.GetResult(i);

			if (error = CreateSystemObject(pCtx, pResult, OrbitDesc))
				return error;
			}
		}
	else if (strEquals(sTag, GROUP_TAG) || strEquals(sTag, SYSTEM_GROUP_TAG))
		{
		for (int i = 0; i < pObj->GetContentElementCount(); i++)
			{
			CXMLElement *pItem = pObj->GetContentElement(i);

			if (error = CreateSystemObject(pCtx, pItem, OrbitDesc))
				return error;
			}
		}
	else if (strEquals(sTag, LOOKUP_TAG))
		{
		//	Find the appropriate table

		CXMLElement *pTable = NULL;
		if (pCtx->pLocalTables)
			pTable = pCtx->pLocalTables->GetContentElementByTag(pObj->GetAttribute(TABLE_ATTRIB));
		if (pTable == NULL)
			{
			if (pCtx->pGlobalTables)
				{
				pTable = pCtx->pGlobalTables->GetContentElementByTag(pObj->GetAttribute(TABLE_ATTRIB));
				if (pTable == NULL)
					{
					pCtx->sError = CONSTLIT("Unable to find table in <Lookup>");
					return ERR_FAIL;
					}
				}
			else
				{
				ASSERT(false);
				return ERR_FAIL;
				}
			}

		//	Lookup the object

		CXMLElement *pResult = pTable->GetContentElement(0);

		//	Recurse

		if (error = CreateSystemObject(pCtx, pResult, OrbitDesc))
			return error;
		}
	else if (strEquals(sTag, ORBITALS_TAG))
		{
		if (error = CreateOrbitals(pCtx, pObj, OrbitDesc))
			return error;
		}
	else if (strEquals(sTag, TROJAN_TAG))
		{
		Metric rOffset;
		CString sOffset;
		if (pObj->FindAttribute(OFFSET_ATTRIB, &sOffset))
			{
			DiceRange Offset;
			if (error = Offset.LoadFromXML(sOffset))
				return error;

			rOffset = mathDegreesToRadians(Offset.Roll());
			}
		else
			rOffset = (g_Pi / 3.0);

		CSystem::Orbit TrojanOrbit(OrbitDesc.GetFocus(),
				OrbitDesc.GetSemiMajorAxis(),
				OrbitDesc.GetEccentricity(),
				OrbitDesc.GetRotation(),
				OrbitDesc.GetObjectAngle() + rOffset);

		for (int i = 0; i < pObj->GetContentElementCount(); i++)
			{
			CXMLElement *pItem = pObj->GetContentElement(i);

			if (error = CreateSystemObject(pCtx, pItem, TrojanOrbit))
				return error;
			}
		}
	else if (strEquals(sTag, ANTI_TROJAN_TAG))
		{
		Metric rOffset;
		CString sOffset;
		if (pObj->FindAttribute(OFFSET_ATTRIB, &sOffset))
			{
			DiceRange Offset;
			if (error = Offset.LoadFromXML(sOffset))
				return error;

			rOffset = mathDegreesToRadians(Offset.Roll());
			}
		else
			rOffset = (g_Pi / 3.0);

		CSystem::Orbit TrojanOrbit(OrbitDesc.GetFocus(),
				OrbitDesc.GetSemiMajorAxis(),
				OrbitDesc.GetEccentricity(),
				OrbitDesc.GetRotation(),
				OrbitDesc.GetObjectAngle() - rOffset);

		for (int i = 0; i < pObj->GetContentElementCount(); i++)
			{
			CXMLElement *pItem = pObj->GetContentElement(i);

			if (error = CreateSystemObject(pCtx, pItem, TrojanOrbit))
				return error;
			}
		}
	else if (strEquals(sTag, RANDOM_LOCATION_TAG))
		{
		int iCount = GetDiceCountFromAttribute(pObj->GetAttribute(COUNT_ATTRIB));
		for (int i = 0; i < iCount; i++)
			if (error = CreateObjectAtRandomLocation(pCtx, pObj))
				return error;
		}
	else if (strEquals(sTag, PLACE_RANDOM_STATION_TAG))
		{
		STATION_PLACEMENT_OUTPUT("<PlaceRandomStation>\n");

		int iCount = GetDiceCountFromAttribute(pObj->GetAttribute(COUNT_ATTRIB));
		for (int i = 0; i < iCount; i++)
			if (error = CreateRandomStationAtAppropriateLocation(pCtx, pObj))
				return error;

		STATION_PLACEMENT_OUTPUT("</PlaceRandomStation>\n");
		}
	else if (strEquals(sTag, FILL_LOCATIONS_TAG))
		{
		STATION_PLACEMENT_OUTPUT("<FillLocations>\n");

		int iCount = pCtx->Labels.GetCount();
		int iPercent = pObj->GetAttributeInteger(PERCENT_FULL_ATTRIB);
		iCount = iCount * iPercent / 100;

		if (error = DistributeStationsAtRandomLocations(pCtx, pObj, iCount))
			return error;

		STATION_PLACEMENT_OUTPUT("</FillLocations>\n");
		}
	else if (strEquals(sTag, FILL_RANDOM_LOCATION_TAG))
		{
		STATION_PLACEMENT_OUTPUT("<FillRandomLocation>\n");

		int iCount = GetDiceCountFromAttribute(pObj->GetAttribute(COUNT_ATTRIB));

		if (error = DistributeStationsAtRandomLocations(pCtx, pObj, iCount))
			return error;

		STATION_PLACEMENT_OUTPUT("</FillRandomLocation>\n");
		}
	else if (strEquals(sTag, RANDOM_STATION_TAG))
		{
		if (error = CreateRandomStation(pCtx, pObj, OrbitDesc))
			return error;
		}
	else if (strEquals(sTag, SIBLINGS_TAG))
		{
		if (error = CreateSiblings(pCtx, pObj, OrbitDesc))
			return error;
		}
	else if (strEquals(sTag, LABEL_TAG))
		{
		if (error = CreateLabel(pCtx, pObj, OrbitDesc))
			return error;
		}
	else if (strEquals(sTag, VARIANTS_TAG))
		{
		//	Loop over all elements and create all objects that
		//	match the current variants.

		for (int i = 0; i < pObj->GetContentElementCount(); i++)
			{
			CXMLElement *pVariant = pObj->GetContentElement(i);
			CString sVariant = pVariant->GetAttribute(VARIANT_ATTRIB);
			int iMaxRadius = pVariant->GetAttributeInteger(MAX_RADIUS_ATTRIB);

			if (iMaxRadius != 0)
				{
				Metric rMaxRadius = LIGHT_SECOND * iMaxRadius;
				if (OrbitDesc.GetSemiMajorAxis() < rMaxRadius)
					{
					if (error = CreateSystemObject(pCtx, pVariant, OrbitDesc))
						return error;
					break;
					}
				}
			else
				{
				if (pCtx->pTopologyNode->HasVariantLabel(sVariant))
					{
					if (error = CreateSystemObject(pCtx, pVariant, OrbitDesc))
						return error;
					}
				}
			}
		}
	else if (strEquals(sTag, PRIMARY_TAG))
		{
		for (int i = 0; i < pObj->GetContentElementCount(); i++)
			{
			CXMLElement *pItem = pObj->GetContentElement(i);

			if (error = CreateSystemObject(pCtx, pItem, OrbitDesc))
				return error;
			}
		}
	else if (strEquals(sTag, STARGATE_TAG))
		{
		if (error = CreateStargate(pCtx, pObj, OrbitDesc))
			return error;
		}
	else if (strEquals(sTag, SHIP_TAG))
		{
		int iCount = GetDiceCountFromAttribute(pObj->GetAttribute(COUNT_ATTRIB));

		//	Load generator

		SDesignLoadCtx Ctx;

		IShipGenerator *pGenerator;
		if (error = IShipGenerator::CreateFromXML(Ctx, pObj, &pGenerator))
			{
			pCtx->sError = strPatternSubst(CONSTLIT("Unable to create ship: %s"), Ctx.sError.GetASCIIZPointer());
			return error;
			}

		if (error = pGenerator->OnDesignLoadComplete(Ctx))
			{
			pCtx->sError = strPatternSubst(CONSTLIT("Unable to create ship: %s"), Ctx.sError.GetASCIIZPointer());
			return error;
			}

		SShipCreateCtx CreateCtx;
		CreateCtx.pSystem = pCtx->pSystem;
		CreateCtx.vPos = OrbitDesc.GetObjectPos();
		if (iCount > 0)
			CreateCtx.PosSpread = DiceRange(6, 2, 1);

		if (error = pGenerator->ValidateShipTable(Ctx))
			{
			pCtx->sError = CONSTLIT("Unable to create ship: sovereign must be specified");
			return error;
			}

		//	Create the ships

		for (int i = 0; i < iCount; i++)
			pGenerator->CreateShips(CreateCtx);
		}
	else if (strEquals(sTag, PARTICLES_TAG))
		{
		if (error = pCtx->pSystem->CreateParticles(pObj, OrbitDesc))
			{
			pCtx->sError = CONSTLIT("Unable to create particle field");
			return error;
			}
		}
	else if (strEquals(sTag, MARKER_TAG))
		{
		if (error = pCtx->pSystem->CreateMarker(pObj, OrbitDesc))
			{
			pCtx->sError = CONSTLIT("Unable to create marker");
			return error;
			}
		}
	else if (strEquals(sTag, SPACE_ENVIRONMENT_TAG))
		{
		if (error = CreateSpaceEnvironment(pCtx, pObj, OrbitDesc))
			return error;
		}
	else if (strEquals(sTag, NULL_TAG))
		NULL;
	else
		{
		pCtx->sError = strPatternSubst(CONSTLIT("Unknown object tag: %s"), sTag.GetASCIIZPointer());
		return ERR_FAIL;
		}

	return NOERROR;
	}

bool CheckForOverlap (SSystemCreateCtx *pCtx, const CVector &vPos)

//	CheckForOverlap
//
//	Returns TRUE if the given position overlaps an existing object

	{
	int i;

	CVector vUR(vPos.GetX() + OVERLAP_DIST, vPos.GetY() + OVERLAP_DIST);
	CVector vLL(vPos.GetX() - OVERLAP_DIST, vPos.GetY() - OVERLAP_DIST);

	for (i = 0; i < pCtx->pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pCtx->pSystem->GetObject(i);

		if (pObj && pObj->InBox(vUR, vLL))
			return true;
		}

	return false;
	}

ALERROR GenerateAngles (SSystemCreateCtx *pCtx, const CString &sAngle, int iCount, Metric *pAngles)

//	GenerateAngles
//
//	Generates random angles based on the angle type

	{
	ALERROR error;
	int i;

	//	Separate value after colon

	char *pString = sAngle.GetASCIIZPointer();
	char *pColon = pString;
	while (pColon && *pColon != ':' && *pColon != '\0')
		pColon++;

	CString sKeyword;
	CString sValue;
	if (pColon && *pColon == ':')
		{
		sKeyword = CString(pString, pColon - pString);
		sValue = CString(pColon+1);
		}
	else
		sKeyword = sAngle;

	//	Generate list of angles

	if (strEquals(sKeyword, RANDOM_ANGLE))
		{
		for (i = 0; i < iCount; i++)
			pAngles[i] = mathDegreesToRadians(mathRandom(0,3599) / 10.0);
		}
	else if (strEquals(sKeyword, EQUIDISTANT_ANGLE))
		{
		int iStart = mathRandom(0, 3599);
		int iSeparation = 3600 / iCount;

		for (i = 0; i < iCount; i++)
			pAngles[i] = mathDegreesToRadians(((iStart + iSeparation * i) % 3600) / 10.0);
		}
	else if (strEquals(sKeyword, INCREMENTING_ANGLE))
		{
		DiceRange IncRange;
		if (error = IncRange.LoadFromXML(sValue))
			{
			pCtx->sError = CONSTLIT("Invalid increment range in orbitals");
			return error;
			}

		int iAngle = mathRandom(0, 359);

		for (i = 0; i < iCount; i++)
			{
			pAngles[i] = mathDegreesToRadians(iAngle % 360);
			iAngle += IncRange.Roll();
			}
		}
	else
		{
		ASSERT(iCount == 1);

		DiceRange AngleRange;
		if (error = AngleRange.LoadFromXML(sAngle))
			{
			pCtx->sError = CONSTLIT("Invalid angle in orbitals");
			return error;
			}

		pAngles[0] = mathDegreesToRadians(AngleRange.Roll());
		}

	return NOERROR;
	}

Metric GetScale (CXMLElement *pObj)

//	GetScale
//
//	Returns the scale for this element

	{
	CString sScale = pObj->GetAttribute(CONSTLIT(g_ScaleAttrib));
	if (sScale.IsBlank())
		return LIGHT_SECOND;
	else if (strEquals(sScale, CONSTLIT(g_AUScale)))
		return g_AU;
	else if (strEquals(sScale, CONSTLIT(g_LightMinuteScale)))
		return LIGHT_MINUTE;
	else
		return LIGHT_SECOND;
	}

void RemoveOverlappingLabels (SSystemCreateCtx *pCtx, Metric rMinDistance)

//	RemoveOverlappingLabels
//
//	Removes labels that are too close together

	{
	int i, j;

	if (pCtx->bLabelsChecked)
		return;

	Metric rMinDist2 = rMinDistance * rMinDistance;

	//	Find overlapping labels

	for (i = 0; i < pCtx->Labels.GetCount(); i++)
		{
		CLabelDesc *pLabelDesc = (CLabelDesc *)pCtx->Labels.GetObject(i);

		for (j = i + 1; j < pCtx->Labels.GetCount(); j++)
			{
			CLabelDesc *pCompare = (CLabelDesc *)pCtx->Labels.GetObject(j);

			CVector vDist = pCompare->OrbitDesc.GetObjectPos() - pLabelDesc->OrbitDesc.GetObjectPos();
			if (vDist.Length2() < rMinDist2)
				{
				pCtx->Labels.RemoveObject(j);
				break;
				}
			}
		}

	//	Compute label stats now (after this, labels might get removed)

	if (pCtx->pStats)
		{
		for (i = 0; i < pCtx->Labels.GetCount(); i++)
			{
			CLabelDesc *pLabelDesc = (CLabelDesc *)pCtx->Labels.GetObject(i);
			pCtx->pStats->AddLabel(pLabelDesc->sAttributes);
			}
		}

	pCtx->bLabelsChecked = true;
	}

//	CSystem methods

ALERROR CSystem::CreateEmpty (CUniverse *pUniv, CTopologyNode *pTopology, CSystem **retpSystem)

//	CreateEmpty
//
//	Creates an empty system

	{
	ALERROR error;
	CSystem *pSystem;

	pSystem = new CSystem(pUniv, pTopology);
	if (pSystem == NULL)
		return ERR_MEMORY;

	//	Initialize

	pSystem->m_iTick = pUniv->GetTicks();

	//	Create the background star field

	if (error = pSystem->CreateStarField(STARFIELD_COUNT))
		return error;

	//	Set the name

	if (pTopology)
		pSystem->m_sName = pTopology->GetSystemName();
	else
		pSystem->m_sName = CONSTLIT("George's Star");

	//	Done

	*retpSystem = pSystem;

	return NOERROR;
	}

ALERROR CSystem::CreateFromXML (CUniverse *pUniv, 
								CSystemType *pType, 
								CTopologyNode *pTopology, 
								CXMLElement *pGlobalTables,
								CSystem **retpSystem,
								CSystemCreateStats *pStats)

//	CreateFromXML
//
//	Creates a new system from description

	{
	ALERROR error;
	int i;

	CXMLElement *pDesc = pType->GetDesc();
	if (pDesc == NULL)
		return ERR_FAIL;

#ifdef DEBUG_STATION_PLACEMENT
	{
	char szBuffer[1024];
	wsprintf(szBuffer, "CSystem::CreateFromXML: %s\n", pTopology->GetSystemName().GetASCIIZPointer());
	::OutputDebugString(szBuffer);
	}
#endif

	START_STRESS_TEST;

	CSystem *pSystem;
	if (error = CreateEmpty(pUniv, pTopology, &pSystem))
		return error;

	//	System is being created

	pSystem->m_fInCreate = true;

	//	Load some data

	pSystem->m_pType = pType;
	pSystem->m_fNoRandomEncounters = pDesc->GetAttributeBool(NO_RANDOM_ENCOUNTERS_ATTRIB);

	//	Set scales

	CString sAttrib;
	if (pDesc->FindAttribute(SPACE_SCALE_ATTRIB, &sAttrib))
		{
		int iScale = strToInt(sAttrib, 0, NULL);
		if (iScale > 0)
			pSystem->m_rKlicksPerPixel = (Metric)iScale;
		}
	if (pDesc->FindAttribute(TIME_SCALE_ATTRIB, &sAttrib))
		{
		int iScale = strToInt(sAttrib, 0, NULL);
		if (iScale > 0)
			pSystem->m_rTimeScale = (Metric)iScale;
		}

	//	Get a pointer to the tables element (may be NULL)

	CXMLElement *pTables = pType->GetLocalSystemTables();

	//	Look for the outer-most group tag

	CXMLElement *pPrimary = pDesc->GetContentElementByTag(SYSTEM_GROUP_TAG);
	if (pPrimary == NULL)
		return ERR_FAIL;

	//	Store the current system. We need this so that any OnCreate code can
	//	get the right system.

	pUniv->SetCurrentSystem(pSystem);

	//	Create the group

	SSystemCreateCtx Ctx;
	Ctx.pTopologyNode = pTopology;
	Ctx.pSystem = pSystem;
	Ctx.pLocalTables = pTables;
	Ctx.pGlobalTables = pGlobalTables;
	Ctx.pStats = pStats;

	if (error = CreateSystemObject(&Ctx,
			pPrimary,
			Orbit()))
		{
		kernelDebugLogMessage("Unable to create system: %s", Ctx.sError.GetASCIIZPointer());
#ifdef DEBUG
		::OutputDebugString(Ctx.sError.GetASCIIZPointer());
		::OutputDebugString("\n");
		::DebugBreak();
#endif
		return error;
		}

	//	Invoke OnCreate event

	if (error = pType->FireOnCreate(&Ctx.sError))
		kernelDebugLogMessage("%s", Ctx.sError.GetASCIIZPointer());

	//	Now invoke OnGlobalSystemCreated

	for (i = 0; i < g_pUniverse->GetDesignTypeCount(); i++)
		{
		if (error = g_pUniverse->GetDesignType(i)->FireOnGlobalSystemCreated(&Ctx.sError))
			kernelDebugLogMessage("%s", Ctx.sError.GetASCIIZPointer());
		}

	STOP_STRESS_TEST;

	//	Done creating

	pSystem->m_fInCreate = false;

	//	Arrange all map labels so that they don't overlap

	pSystem->ComputeMapLabels();
	pSystem->ComputeStars();

	//	Call each object and tell it that the system has been
	//	created.

	int iEncounterCount = 0;
	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);

		if (pObj)
			{
			pObj->OnSystemCreated();

			//	Ask the object if it has encounters

			if (pObj->GetRandomEncounterTable())
				iEncounterCount++;
			}
		}

	pSystem->ComputeRandomEncounters(iEncounterCount);

	//	Done

	*retpSystem = pSystem;

	return NOERROR;
	}

ALERROR CSystem::CreateMarker (CXMLElement *pDesc, const CSystem::Orbit &oOrbit)

//	CreateMarker
//
//	Creates a marker from an XML description

	{
	ALERROR error;

	//	Create the marker

	CMarker *pMarker;
	if (error = CMarker::Create(this,
			NULL,
			oOrbit.GetObjectPos(),
			NullVector,
			NULL_STR,
			&pMarker))
		return error;

	//	If this station is a gate entry-point, then add it to
	//	the table in the system.

	CString sEntryPoint = pDesc->GetAttribute(OBJ_NAME_ATTRIB);
	if (!sEntryPoint.IsBlank())
		NameObject(sEntryPoint, pMarker);

	return NOERROR;
	}

ALERROR CSystem::CreateParticles (CXMLElement *pDesc, const CSystem::Orbit &oOrbit)

//	CreateParticles
//
//	Create a particle field from an XML description

	{
	ALERROR error;

	//	Create the field

	CParticleEffect *pParticles;
	if (error = CParticleEffect::Create(this,
			pDesc,
			oOrbit.GetObjectPos(),
			NullVector,
			&pParticles))
		return error;

	//	Done

	return NOERROR;
	}

ALERROR CSystem::CreateRandomEncounter (IShipGenerator *pTable, 
										CSpaceObject *pBase,
										CSovereign *pBaseSovereign,
										CSpaceObject *pTarget,
										CSpaceObject *pGate)

//	CreateRandomEncounter
//
//	Creates a random ship encounter

	{
	SShipCreateCtx Ctx;
	Ctx.pSystem = this;
	Ctx.pBase = pBase;
	Ctx.pBaseSovereign = pBaseSovereign;
	Ctx.pTarget = pTarget;

	//	Figure out where the encounter will come from

	if (pGate && pGate->IsStargate())
		Ctx.pGate = pGate;
	else if (pGate)
		{
		Ctx.vPos = pGate->GetPos();
		Ctx.PosSpread = DiceRange(2, 1, 2);
		}
	else if (pTarget)
		//	Exclude uncharted stargates
		Ctx.pGate = pTarget->GetNearestStargate(true);

	//	Generate ship

	pTable->CreateShips(Ctx);

	return NOERROR;
	}

ALERROR CSystem::CreateStation (CStationType *pType,
								const CVector &vPos,
								const CVector &vVel,
								CXMLElement *pExtraData,
								CSpaceObject **retpStation,
								CString *retsError)

//	CreateStation
//
//	Creates a station from a type

	{
	ALERROR error;
	CSpaceObject *pStation = NULL;

	//	If this is a ship encounter, then just create the ship

	if (pType->IsShipEncounter())
		{
		CXMLElement *pShipRegistry = pType->GetDesc()->GetContentElementByTag(SHIPS_TAG);
		if (pShipRegistry == NULL)
			{
			ASSERT(false);
			kernelDebugLogMessage("No ship for ship encounter: %s", pType->GetName().GetASCIIZPointer());
			if (retpStation)
				*retpStation = NULL;
			return NOERROR;
			}

		//	Load the generator

		SDesignLoadCtx Ctx;

		IShipGenerator *pGenerator;
		if (error = IShipGenerator::CreateFromXMLAsGroup(Ctx, pShipRegistry, &pGenerator))
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("Unable to create ship for %s: %s"), pType->GetName().GetASCIIZPointer(), Ctx.sError.GetASCIIZPointer());
			return error;
			}

		if (error = pGenerator->OnDesignLoadComplete(Ctx))
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("Unable to create ship for %s: %s"), pType->GetName().GetASCIIZPointer(), Ctx.sError.GetASCIIZPointer());
			return error;
			}

		//	Create the ships

		SShipCreateCtx CreateCtx;
		CreateCtx.pSystem = this;
		CreateCtx.vPos = vPos;
		CreateCtx.pBaseSovereign = pType->GetSovereign();
		CreateCtx.pEncounterInfo = pType;
		CreateCtx.dwFlags = SShipCreateCtx::RETURN_RESULT;

		pGenerator->CreateShips(CreateCtx);

		//	If no ships are created we return

		if (CreateCtx.Result.GetCount() == 0)
			{
			if (retpStation)
				*retpStation = NULL;
			return NOERROR;
			}

		//	Return the first ship created

		pStation = CreateCtx.Result.GetObj(0);

		//	This type has now been encountered

		pType->SetEncountered();
		}

	//	If this is static, create a static object

	else if (pType->IsStatic())
		{
		//	Create the station

		if (error = CStation::CreateFromType(this,
				pType,
				vPos,
				vVel,
				pExtraData,
				(CStation **)&pStation))
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("Unable to create station from type: %s"), pType->GetName().GetASCIIZPointer());
			return error;
			}
		}

	//	Otherwise, create the station

	else
		{
		//	Create the station

		if (error = CStation::CreateFromType(this,
				pType,
				vPos,
				vVel,
				pExtraData,
				(CStation **)&pStation))
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("Unable to create station from type: %s"), pType->GetName().GetASCIIZPointer());
			return error;
			}
		}

	//	Done

	if (retpStation)
		*retpStation = pStation;

	return NOERROR;
	}

ALERROR CSystem::CreateStation (SSystemCreateCtx *pCtx, 
								CStationType *pType, 
								const CVector &vPos,
								const CSystem::Orbit &OrbitDesc,
								bool bCreateSatellites,
								CXMLElement *pExtraData,
								CSpaceObject **retpStation)

//	CreateStation
//
//	Creates a station of the given type

	{
	ALERROR error;
	CSpaceObject *pStation = NULL;

	//	Create the station (or ship encounter). Note that pStation may come back NULL

	if (error = CreateStation(pType,
			vPos,
			NullVector,
			pExtraData,
			&pStation,
			&pCtx->sError))
		return ERR_FAIL;

	//	If this is a satellite, then add it as a subordinate

	if (pStation && pCtx->pStation && pStation->CanAttack())
		pCtx->pStation->AddSubordinate(pStation);

	//	Create any satellites of the station

	CSpaceObject *pSavedStation = pCtx->pStation;
	pCtx->pStation = pStation;

	CXMLElement *pSatellites = pType->GetSatellitesDesc();
	if (pSatellites && bCreateSatellites)
		{
		for (int i = 0; i < pSatellites->GetContentElementCount(); i++)
			{
			CXMLElement *pSatDesc = pSatellites->GetContentElement(i);
			if (error = CreateSystemObject(pCtx, pSatDesc, OrbitDesc))
				return error;
			}
		}

	pCtx->pStation = pSavedStation;

#ifdef DEBUG_STATION_PLACEMENT2
	if (pStation && pStation->GetScale() == scaleStructure)
		{
		char szBuffer[1024];
		wsprintf(szBuffer, "CreateStation: %s\n", pType->GetName().GetASCIIZPointer());
		::OutputDebugString(szBuffer);
		}
#endif

	//	Done

	if (retpStation)
		*retpStation = pStation;

	return NOERROR;
	}

ALERROR CreateStation (SSystemCreateCtx *pCtx, CXMLElement *pDesc, const CSystem::Orbit &OrbitDesc, CStation **retpStation)

//	CreateStation
//
//	Creates a station from an XML description

	{
	ALERROR error;
	int i;
	CStationType *pStationType;

	//	Get the type of the station

	pStationType = g_pUniverse->FindStationType((DWORD)pDesc->GetAttributeInteger(TYPE_ATTRIB));
	if (pStationType == NULL)
		{
		pCtx->sError = strPatternSubst(CONSTLIT("Unknown station type: %s"), pDesc->GetAttribute(TYPE_ATTRIB).GetASCIIZPointer());
		return ERR_FAIL;
		}

	//	If this is a unique station that we've already encountered, then bail out (this is not
	//	an error because sometimes we explicitly place a station even though there is a chance
	//	that it might have been encountered previously).

	if (!pStationType->CanBeEncountered(pCtx->pSystem))
		return NOERROR;

	//	Get offsets

	int x = pDesc->GetAttributeInteger(X_OFFSET_ATTRIB);
	int y = pDesc->GetAttributeInteger(Y_OFFSET_ATTRIB);

	//	Compute position of station

	CVector vPos(OrbitDesc.GetObjectPos());
	if (x != 0 || y != 0)
		vPos = vPos + CVector(x * g_KlicksPerPixel, y * g_KlicksPerPixel);

	//	Create the station

	CStation *pStation = NULL;
	CSpaceObject *pObj;
	if (error = pCtx->pSystem->CreateStation(pCtx,
			pStationType,
			vPos,
			OrbitDesc,
			!pDesc->GetAttributeBool(NO_SATELLITES_ATTRIB),
			pDesc->GetContentElementByTag(INITIAL_DATA_TAG),
			&pObj))
		return error;

	if (pObj)
		pStation = pObj->AsStation();

#ifdef DEBUG_STATION_PLACEMENT2
	if (pStation->GetScale() == scaleStructure)
		{
		char szBuffer[1024];
		wsprintf(szBuffer, "CreateStation: %s\n", pStationType->GetName().GetASCIIZPointer());
		::OutputDebugString(szBuffer);
		}
#endif

	//	Done if this is a ship encounter

	if (pStationType->IsShipEncounter())
		{
		if (retpStation)
			*retpStation = NULL;
		return NOERROR;
		}

	//	Set the name of the station, if specified by the system

	CString sName = pDesc->GetAttribute(NAME_ATTRIB);
	if (!sName.IsBlank())
		{
		DWORD dwFlags = 0;
		if (pDesc->GetAttributeBool(NO_ARTICLE_ATTRIB))
			dwFlags |= nounNoArticle;

		pObj->SetName(sName, dwFlags);
		}

	//	If we want to show the orbit for this station, set the orbit desc

	if (pDesc->GetAttributeBool(CONSTLIT(g_ShowOrbitAttrib)))
		pStation->SetMapOrbit(OrbitDesc);

	//	Set the image variant

	int iVariant;
	if (pDesc->FindAttributeInteger(IMAGE_VARIANT_ATTRIB, &iVariant))
		pStation->SetImageVariant(iVariant);

	//	If this station is a gate entry-point, then add it to
	//	the table in the system.

	CString sEntryPoint = pDesc->GetAttribute(OBJ_NAME_ATTRIB);
	if (!sEntryPoint.IsBlank())
		pCtx->pSystem->NameObject(sEntryPoint, pStation);

	//	If we don't want to show a map label

	if (pDesc->GetAttributeBool(NO_MAP_LABEL_ATTRIB))
		pStation->SetNoMapLabel();

	//	No reinforcements

	if (pDesc->GetAttributeBool(NO_REINFORCEMENTS_ATTRIB))
		pStation->SetNoReinforcements();

	//	Create additional satellites

	CXMLElement *pSatellites = pDesc->GetContentElementByTag(SATELLITES_TAG);
	if (pSatellites)
		{
		for (i = 0; i < pSatellites->GetContentElementCount(); i++)
			{
			CXMLElement *pSatDesc = pSatellites->GetContentElement(i);
			if (error = CreateSystemObject(pCtx, pSatDesc, OrbitDesc))
				return error;
			}
		}

	//	See if we need to create additional ships

	CXMLElement *pShips = pDesc->GetContentElementByTag(SHIPS_TAG);
	if (pShips)
		{
		if (error = CreateShipsForStation(pStation, pShips))
			{
			pCtx->sError = CONSTLIT("Unable to create ships for station");
			return error;
			}
		}

	//	Done

	if (retpStation)
		*retpStation = pStation;

	return NOERROR;
	}

int ParseSingleCriteria (const CString &sSingleCriteria, CString *retsAttrib)

//	ParseSingleCriteria
//
//	Parse the criteria into two parts: the match strength and
//	the attribute. The match strength is expressed as an integer.
//
//	4	*	attribute required
//	3	+++	weight/20 if lacks attribute
//	2	++	weight/5 if lacks attribute
//	1	+	weight/2 if lacks attribute
//	-1	-	weight/2 if has attribute
//	-2	--	weight/5 if has attribute
//	-3	---	weight/20 if has attribute
//	-4	!	can't have attribute

	{
	char *pPos = sSingleCriteria.GetPointer();
	int iMatchStrength = 0;
	while (*pPos != '\0')
		{
		if (*pPos == '*')
			{
			iMatchStrength = 4;
			pPos++;
			break;
			}
		else if (*pPos == '!')
			{
			iMatchStrength = -4;
			pPos++;
			break;
			}
		else if (*pPos == '+')
			iMatchStrength++;
		else if (*pPos == '-')
			iMatchStrength--;
		else
			break;
		pPos++;
		}

	*retsAttrib = CString(pPos);
	return iMatchStrength;
	}
