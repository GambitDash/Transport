//	CBaseShipAI.cpp
//
//	CBaseShipAI class

#include "PreComp.h"


const Metric MAX_AREA_WEAPON_CHECK =	(15.0 * LIGHT_SECOND);
const Metric MAX_AREA_WEAPON_CHECK2 =	(MAX_AREA_WEAPON_CHECK * MAX_AREA_WEAPON_CHECK);
const Metric MIN_TARGET_DIST =			(5.0 * LIGHT_SECOND);
const Metric MIN_TARGET_DIST2 =			(MIN_TARGET_DIST * MIN_TARGET_DIST);
const Metric MIN_STATION_TARGET_DIST =	(10.0 * LIGHT_SECOND);
const Metric MIN_STATION_TARGET_DIST2 =	(MIN_STATION_TARGET_DIST * MIN_STATION_TARGET_DIST);
const Metric HIT_NAV_POINT_DIST =		(8.0 * LIGHT_SECOND);
const Metric HIT_NAV_POINT_DIST2 =		(HIT_NAV_POINT_DIST * HIT_NAV_POINT_DIST);
const Metric MAX_TARGET_OF_OPPORTUNITY_RANGE = (20.0 * LIGHT_SECOND);
const Metric ESCORT_DISTANCE =			(6.0 * LIGHT_SECOND);
const Metric MAX_ESCORT_DISTANCE =		(12.0 * LIGHT_SECOND);
const Metric ATTACK_RANGE =				(20.0 * LIGHT_SECOND);
const Metric CLOSE_RANGE =				(50.0 * LIGHT_SECOND);
const Metric CLOSE_RANGE2 =				(CLOSE_RANGE * CLOSE_RANGE);

#define MAX_TARGETS						10
#define MAX_DOCK_DISTANCE				(15.0 * LIGHT_SECOND)
#define MAX_GATE_DISTANCE				(g_KlicksPerPixel * 32)
#define DOCKING_APPROACH_DISTANCE		(g_KlicksPerPixel * 200)
#define DEFAULT_DIST_CHECK				(700.0 * g_KlicksPerPixel)
#define MIN_FLYBY_SPEED					(2.0 * g_KlicksPerPixel)
#define MIN_FLYBY_SPEED2				(MIN_FLYBY_SPEED * MIN_FLYBY_SPEED)

#define MAX_DELTA						(2.0 * g_KlicksPerPixel)
#define MAX_DELTA2						(MAX_DELTA * MAX_DELTA)
#define MAX_DELTA_VEL					(g_KlicksPerPixel / 2.0)
#define MAX_DELTA_VEL2					(MAX_DELTA_VEL * MAX_DELTA_VEL)
#define MAX_DISTANCE					(400 * g_KlicksPerPixel)
#define MAX_IN_FORMATION_DELTA			(2.0 * g_KlicksPerPixel)
#define MAX_IN_FORMATION_DELTA2			(MAX_IN_FORMATION_DELTA * MAX_IN_FORMATION_DELTA)

#ifdef DEBUG_COMBAT
#define DEBUG_COMBAT_OUTPUT(x)			if (m_pShip->IsSelected()) g_pUniverse->DebugOutput("%d> %s", g_iDebugLine++, x)
#else
#define DEBUG_COMBAT_OUTPUT(x)
#endif

extern int g_iDebugLine;

CBaseShipAI::CBaseShipAI (IObjectClass *pClass) : CObject(pClass),
		m_pShip(NULL),
		m_Orders(sizeof(OrderEntry), 1),
		m_iManeuver(NoRotation),
		m_iThrustDir(constNeverThrust),
		m_dwFireWeapon(0),
		m_iBestWeapon(devNone),
		m_iLastTurn(NoRotation),
		m_iLastTurnCount(0),
		m_iManeuverCounter(0),
		m_pNavPath(NULL),
		m_iNavPathPos(-1),
		m_pShields(NULL),
		m_fRecalcBestWeapon(true),
		m_fDeviceActivate(false),
		m_fDockingRequested(false),
		m_fInOnOrderChanged(false),
		m_fInOnOrdersCompleted(false),
		m_fWaitForShieldsToRegen(false)

//	CBaseShipAI constructor

	{
	}

CBaseShipAI::~CBaseShipAI (void)

//	CBaseShipAI destructor

	{
	}

void CBaseShipAI::AddOrder(OrderTypes Order, CSpaceObject *pTarget, DWORD dwData, bool bAddBefore)

//	AddOrder
//
//	Add an order to the list

	{
	ASSERT(pTarget == NULL || pTarget->NotifyOthersWhenDestroyed());

	//	Make sure we have a target

	switch (Order)
		{
		case orderGuard:
		case orderDock:
		case orderDestroyTarget:
		case orderPatrol:
		case orderEscort:
		case orderLoot:
		case orderMine:
		case orderFollow:
		case orderGoTo:
		case orderWaitForTarget:
		case orderBombard:
		case orderApproach:
		case orderAimAtTarget:
			{
			if (pTarget == NULL)
				{
				ASSERT(false);
				return;
				}
			break;
			}
		default:
			break;
		}

	OrderEntry Entry;
	Entry.Order = Order;
	Entry.pTarget = pTarget;
	Entry.dwData = dwData;

	if (bAddBefore)
		{
		m_Orders.InsertStruct(&Entry, 0, NULL);
		FireOnOrderChanged();
		}
	else
		{
		m_Orders.AppendStruct(&Entry, NULL);
		if (m_Orders.GetCount() == 1)
			FireOnOrderChanged();
		}
	}

void CBaseShipAI::CalcBestWeapon (CSpaceObject *pTarget, Metric rTargetDist2)

//	CalcBestWeapon
//
//	Initializes:
//
//	m_iBestWeapon
//	m_pBestWeapon
//	m_rBestWeaponRange

	{
	int i;
	Metric rBestRange = g_InfiniteDistance;

	if (m_fRecalcBestWeapon)
		{
		ASSERT(m_pShip);

		int iBestWeapon = -1;
		int iBestWeaponVariant = 0;
		int iBestScore = 0;

		//	Loop over all devices to find the best weapon

		for (i = 0; i < m_pShip->GetDeviceCount(); i++)
			{
			CInstalledDevice *pWeapon = m_pShip->GetDevice(i);

			if (pWeapon->IsEmpty())
				continue;
			else if (pWeapon->IsSecondaryWeapon())
				{
				//	Remember the range in case we end up with no good weapons and we need to set 
				//	a course towards the target.

				Metric rRange = pWeapon->GetClass()->GetMaxEffectiveRange(m_pShip, pWeapon, pTarget);
				if (rRange < rBestRange)
					rBestRange = rRange;
				continue;
				}

			//	Compute score

			switch (pWeapon->GetCategory())
				{
				case itemcatWeapon:
					{
					int iScore = CalcWeaponScore(pTarget, pWeapon, rTargetDist2);
					if (iScore > iBestScore)
						{
						iBestWeapon = i;
						iBestWeaponVariant = 0;
						iBestScore = iScore;
						}

					break;
					}

				case itemcatLauncher:
					{
					int iCount = m_pShip->GetMissileCount();
					if (iCount > 0)
						{
						m_pShip->ReadyFirstMissile();

						for (int j = 0; j < iCount; j++)
							{
							int iScore = CalcWeaponScore(pTarget, pWeapon, rTargetDist2);

							//	If we only score 1 and we've got secondary weapons, then don't
							//	bother with this missile (we don't want to waste it)

							if (iScore == 1 && m_pShip->HasSecondaryWeapons())
								{
								iScore = 0;

								//	Remember the range in case we end up with no good weapons and we need to set 
								//	a course towards the target.

								Metric rRange = pWeapon->GetClass()->GetMaxEffectiveRange(m_pShip, pWeapon, pTarget);
								if (rRange < rBestRange)
									rBestRange = rRange;
								}

							if (iScore > iBestScore)
								{
								iBestWeapon = i;
								iBestWeaponVariant = j;
								iBestScore = iScore;
								}

							m_pShip->ReadyNextMissile();
							}
						}
					break;
					}
				default:
					/* Do nothing. */
					break;
				}
			}

		//	Given the best weapon, select the named device

		if (iBestWeapon != -1)
			{
			m_iBestWeapon = m_pShip->SelectWeapon(iBestWeapon, iBestWeaponVariant);
			m_pBestWeapon = m_pShip->GetNamedDevice(m_iBestWeapon);
			m_rBestWeaponRange = m_pBestWeapon->GetClass()->GetMaxEffectiveRange(m_pShip, m_pBestWeapon, pTarget);

			//	Optimum range varies by ship (destiny)

			int iAdj = 100 + ((m_pShip->GetDestiny() % 60) - 30);
			m_rBestWeaponRange = m_rBestWeaponRange * (iAdj * 0.01);
			}
		else
			{
			m_iBestWeapon = devNone;

			//	If we can't find a good weapon, at least set the weapon range so that we close
			//	to secondary weapon range.

			m_rBestWeaponRange = rBestRange;
			}

		//	Done

		m_fRecalcBestWeapon = false;
		}
	}

CSpaceObject *CBaseShipAI::CalcEnemyShipInRange (CSpaceObject *pCenter, Metric rRange, CSpaceObject *pExcludeObj)

//	CalcEnemyShipInRange
//
//	Returns the first enemy ship that it finds in range of pCenter. Returns NULL if none
//	are found.

	{
	int i;
	Metric rMaxRange2 = rRange * rRange;

	//	Compute this object's perception and perception range

	int iPerception = m_pShip->GetPerception();
	Metric rRange2[RANGE_INDEX_COUNT];
	for (i = 0; i < RANGE_INDEX_COUNT; i++)
		{
		rRange2[i] = RangeIndex2Range(i);
		rRange2[i] = rRange2[i] * rRange2[i];
		}

	//	Get the sovereign

	CSovereign *pSovereign = m_pShip->GetSovereignToDefend();
	if (pSovereign == NULL || m_pShip->GetSystem() == NULL)
		return NULL;

	//	Loop

	const CSpaceObjectList &ObjList = pSovereign->GetEnemyObjectList(m_pShip->GetSystem());
	int iCount = ObjList.GetCount();
	for (i = 0; i < iCount; i++)
		{
		CSpaceObject *pObj = ObjList.GetObj(i);

		if (pObj->GetCategory() == CSpaceObject::catShip
				&& pObj->CanAttack())
			{
			CVector vRange = pObj->GetPos() - pCenter->GetPos();
			Metric rDistance2 = vRange.Dot(vRange);

			if (rDistance2 < rMaxRange2
					&& rDistance2 < rRange2[pObj->GetDetectionRangeIndex(iPerception)]
					&& pObj != pExcludeObj
					&& !pObj->IsEscortingFriendOf(m_pShip))
				return pObj;
			}
		}

	return NULL;
	}

bool CBaseShipAI::CalcFlockingFormation (CSpaceObject *pLeader,
										 Metric rFOVRange,
										 Metric rSeparationRange,
										 CVector *retvPos, 
										 CVector *retvVel, 
										 int *retiFacing)

//	CalcFlockingFormation
//
//	Calculates the position that this ship should take relative to the rest of the flock. Returns FALSE
//	if the current ship is a leader in the flock.

	{
	int i;
	CVector vFlockPos;
	CVector vFlockVel;
	CVector vFlockHeading;
	CVector vAvoid;
	Metric rFOVRange2 = rFOVRange * rFOVRange;
	Metric rSeparationRange2 = rSeparationRange * rSeparationRange;
	Metric rFlockCount = 0.0;
	Metric rAvoidCount = 0.0;

	for (i = 0; i < m_pShip->GetSystem()->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = m_pShip->GetSystem()->GetObject(i);

		if (pObj 
				&& pObj->GetSovereign() == m_pShip->GetSovereign()
				&& pObj->GetCategory() == CSpaceObject::catShip
				&& pObj != m_pShip
				&& pObj != pLeader)
			{
			CVector vTarget = pObj->GetPos() - m_pShip->GetPos();
			Metric rTargetDist2 = vTarget.Dot(vTarget);

			//	Only consider ships within a certain range

			if (rTargetDist2 < rFOVRange2)
				{
				CVector vTargetRot = vTarget.Rotate(360 - m_pShip->GetRotation());

				//	Only consider ships in front of us

				if (vTargetRot.GetX() > 0.0)
					{
					//	Only ships of a certain destiny

					if (pObj->GetDestiny() > m_pShip->GetDestiny())
						{
						vFlockPos = vFlockPos + vTarget;
						vFlockVel = vFlockVel + (pObj->GetVel() - m_pShip->GetVel());
						vFlockHeading = vFlockHeading + PolarToVector(pObj->GetRotation(), 1.0);
						rFlockCount = rFlockCount + 1.0;
						}

					//	Avoid ships that are too close

					if (rTargetDist2 < rSeparationRange2)
						{
						vAvoid = vAvoid + vTarget;
						rAvoidCount = rAvoidCount + 1.0;
						}
					}
				}
			}
		}

	//	If we've got a leader, add separately

	if (pLeader)
		{
		CVector vTarget = pLeader->GetPos() - m_pShip->GetPos();
		Metric rTargetDist2 = vTarget.Dot(vTarget);

		vFlockPos = vFlockPos + vTarget;
		vFlockVel = vFlockVel + (pLeader->GetVel() - m_pShip->GetVel());
		vFlockHeading = vFlockHeading + PolarToVector(pLeader->GetRotation(), 1.0);
		rFlockCount = rFlockCount + 1.0;

		//	Avoid ships that are too close

		if (rTargetDist2 < rSeparationRange2)
			{
			vAvoid = vAvoid + vTarget;
			rAvoidCount = rAvoidCount + 1.0;
			}
		}

	//	Compute the averages

	if (rFlockCount > 0.0)
		{
		CVector vAimPos = (vFlockPos / rFlockCount);
		if (rAvoidCount > 0.0)
			{
			int iAimAngle = VectorToPolar(vAimPos);

			CVector vAvoidAverage = (vAvoid / rAvoidCount);
			CVector vAvoidRot = vAvoidAverage.Rotate(360 - iAimAngle);

			CVector vAimPerp = vAimPos.Normal().Perpendicular();

			Metric rAvoidAverage = vAvoidAverage.Length();
			Metric rAvoidMag = 2.0 * (rSeparationRange - rAvoidAverage);

			CVector vAvoidAdj;
			if (rAvoidMag > 0.0)
				{
				if (vAvoidRot.GetY() > 0.0)
					vAvoidAdj = -rAvoidMag * vAimPerp;
				else
					vAvoidAdj = rAvoidMag * vAimPerp;
				}

			vAimPos = vAimPos + vAvoidAdj;
			}

		*retvPos = m_pShip->GetPos() + vAimPos;
		*retvVel = m_pShip->GetVel() + (vFlockVel / rFlockCount);
		*retiFacing = VectorToPolar(vFlockHeading);

		return true;
		}
	else
		{
		return false;
		}
	}

void CBaseShipAI::CalcInvariants (void)

//	CalcInvariants
//
//	Calculate some metrics that are invariant for the ship

	{
	int i;

	//	Initialize

	m_fSuperconductingShields = false;
	m_pShields = NULL;

	//	Primary aim range

	Metric rPrimaryRange = m_pShip->GetWeaponRange(devPrimaryWeapon);
	Metric rAimRange = (m_pAISettings->iFireRangeAdj * rPrimaryRange) / (100.0 * ((m_pShip->GetDestiny() % 8) + 4));
	if (rAimRange < 1.5 * MIN_TARGET_DIST)
		rAimRange = 1.5 * MIN_TARGET_DIST;
	m_rPrimaryAimRange2 = 4.0 * rAimRange * rAimRange;

	//	Flank distance

	int iAdj = 2 * std::max(0, (8 - m_pShip->GetClass()->GetManeuverability()));
	m_rFlankDist = (iAdj * ((m_pShip->GetDestiny() % 61) - 30) * g_KlicksPerPixel);

	//	Max turn count

	m_iMaxTurnCount =  (10 + (m_pShip->GetDestiny() % 20)) * m_pShip->GetClass()->GetManeuverability();

	//	Range of longest range weapons

	m_rMaxWeaponRange = 0.0;
	m_iBestNonLauncherWeaponLevel = 0;
	int iPrimaryCount = 0;
	for (i = 0; i < m_pShip->GetDeviceCount(); i++)
		{
		CInstalledDevice *pDevice = m_pShip->GetDevice(i);

		if (pDevice->IsEmpty())
			continue;

		switch (pDevice->GetCategory())
			{
			case itemcatWeapon:
			case itemcatLauncher:
				{
				//	Figure out the best non-launcher level

				if (pDevice->GetCategory() != itemcatLauncher
						&& pDevice->GetClass()->GetLevel() > m_iBestNonLauncherWeaponLevel)
					{
					m_iBestNonLauncherWeaponLevel = pDevice->GetClass()->GetLevel();
					}

				//	Get the max range that any (primary) weapon has

				if (!pDevice->IsSecondaryWeapon())
					{
					Metric rMaxRange = pDevice->GetMaxEffectiveRange(m_pShip);
					if (rMaxRange > m_rMaxWeaponRange)
						m_rMaxWeaponRange = rMaxRange;
					}

				//	Count the number of weapons that we can switch to (including
				//	launchers)

				if (!pDevice->IsSecondaryWeapon())
					iPrimaryCount++;
				break;
				}

			case itemcatShields:
				m_pShields = pDevice;
				if (pDevice->GetClass()->GetUNID() == g_SuperconductingShieldsUNID)
					m_fSuperconductingShields = true;
				break;
			default:
				/* Do nothing. */
				break;
			}
		}

	//	Flags

	m_fHasMultipleWeapons = (iPrimaryCount > 1);
	m_fThrustThroughTurn = ((m_pShip->GetDestiny() % 100) < 50);
	m_fAvoidExplodingStations = (rAimRange > MIN_STATION_TARGET_DIST);
	m_fImmobile = (m_pShip->GetMaxSpeed() == 0.0);
	m_fRecalcBestWeapon = true;
	}

bool CBaseShipAI::CalcNavPath (CSpaceObject *pTo)

//	CalcNavPath
//
//	Initializes m_pNavPath and m_iNavPathPos. Call this function
//	before using ImplementFollowNavPath

	{
	int i;
	CSystem *pSystem = m_pShip->GetSystem();

	ASSERT(pTo);

	//	Figure out an appropriate starting point

	CSpaceObject *pBestObj = NULL;
	Metric rBestDist2 = g_InfiniteDistance * g_InfiniteDistance;
	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);
		if (pObj
				&& pObj->GetCategory() == CSpaceObject::catStation
				&& pObj->GetScale() == scaleStructure
				&& (pObj->IsStargate()
					|| (pObj->HasAttribute(CONSTLIT("populated"))
						&& pObj->IsFriend(m_pShip))))
			{
			Metric rDist2 = (pObj->GetPos() - m_pShip->GetPos()).Length2();
			if (rDist2 < rBestDist2)
				{
				pBestObj = pObj;
				rBestDist2 = rDist2;
				}
			}
		}

	if (pBestObj == NULL)
		return false;

	CSpaceObject *pFrom = pBestObj;

	//	Get the appropriate nav path from the system

	CNavigationPath *pPath = pSystem->GetNavPath(m_pShip->GetSovereign(), pFrom, pTo);

	//	Done

	CalcNavPath(pPath);
	return true;
	}

void CBaseShipAI::CalcNavPath (CSpaceObject *pFrom, CSpaceObject *pTo)

//	CalcNavPath
//
//	Initializes m_pNavPath and m_iNavPathPos. Call this function
//	before using ImplementFollowNavPath

	{
	ASSERT(pFrom);
	ASSERT(pTo);

	//	Get the appropriate nav path from the system

	CSystem *pSystem = m_pShip->GetSystem();
	CNavigationPath *pPath = pSystem->GetNavPath(m_pShip->GetSovereign(), pFrom, pTo);

	//	Done

	CalcNavPath(pPath);
	}

void CBaseShipAI::CalcNavPath (CNavigationPath *pPath)

//	CalcNavPath
//
//	Initializes ship state to follow the given path

	{
	int i;

	ASSERT(pPath);

	//	Set the path

	m_pNavPath = pPath;

	//	Figure out which nav position we are closest to

	const Metric CLOSE_ENOUGH_DIST = (LIGHT_SECOND * 10.0);
	const Metric CLOSE_ENOUGH_DIST2 = CLOSE_ENOUGH_DIST * CLOSE_ENOUGH_DIST;
	Metric rBestDist2 = (g_InfiniteDistance * g_InfiniteDistance);
	int iBestPoint = -1;

	for (i = 0; i < m_pNavPath->GetNavPointCount(); i++)
		{
		CVector vDist = m_pNavPath->GetNavPoint(i) - m_pShip->GetPos();
		Metric rDist2 = vDist.Length2();

		if (rDist2 < rBestDist2)
			{
			rBestDist2 = rDist2;
			iBestPoint = i;

			if (rDist2 < CLOSE_ENOUGH_DIST2)
				break;
			}
		}

	//	Done

	ASSERT(iBestPoint != -1);
	if (iBestPoint == -1)
		iBestPoint = 0;

	m_iNavPathPos = iBestPoint;
	}

void CBaseShipAI::CalcShieldState (void)

//	CalcShieldState
//
//	Updates m_fWaitForShieldsToRegen

	{
	if (m_pShields
			&& !(m_pAISettings->dwFlags & aiflagNoShieldRetreat)
			&& m_pShip->IsDestinyTime(17) 
			&& !m_fSuperconductingShields)
		{
		int iHPLeft, iMaxHP;
		m_pShields->GetStatus(m_pShip, &iHPLeft, &iMaxHP);

		//	If iMaxHP is 0 then we treat the shields as up. This can happen
		//	if a ship with (e.g.) hull-plate ionizer gets its armor destroyed

		int iLevel = (iMaxHP > 0 ? (iHPLeft * 100 / iMaxHP) : 100);

		//	If we're waiting for shields to regenerate, see if
		//	they are back to 60% or better

		if (m_fWaitForShieldsToRegen)
			{
			if (iLevel >= 60)
				m_fWaitForShieldsToRegen = false;
			}

		//	Otherwise, if shields are down, then we need to wait
		//	until they regenerate

		else
			{
			if (iLevel <= 0)
				m_fWaitForShieldsToRegen = true;
			}
		}
	}

Metric CBaseShipAI::CalcShipIntercept (const CVector &vRelPos, const CVector &vAbsVel, Metric rMaxSpeed)

//	CalcShipIntercept
//
//	Calculates (heuristically) the time that it will take us
//	to intercept at target at vRelPos (relative to ship) and moving
//	at vAbsVel (in absolute terms). If we return < 0.0 then we
//	cannot intercept even at maximum velocity.

	{
	CVector vInterceptPoint;

	//	Compute the speed with which the target is closing
	//	the distance (this may be a negative number). Note
	//	that we care about the target's absolute velocity
	//	(not its relative velocity because we are trying to
	//	adjust our velocity).

	Metric rClosingSpeed = -vAbsVel.Dot(vRelPos.Normal());

	//	Figure out how long it will take to overtake the target's
	//	current position at maximum speed. (This is just a heuristic
	//	that gives us an approximation of the length of an intercept
	//	course.)

	rClosingSpeed = rClosingSpeed + rMaxSpeed;
	if (rClosingSpeed > 0.0)
		return vRelPos.Length() / (rClosingSpeed);
	else
		return -1.0;
	}

int CBaseShipAI::CalcWeaponScore (CSpaceObject *pTarget, CInstalledDevice *pWeapon, Metric rTargetDist2)

//	CalcWeaponScore
//
//	Calculates a score for this weapon

	{
	int iScore = 0;

	//	If this is an EMP weapon adjust the score based on the state of
	//	the target.

	int iEffectiveness = pWeapon->GetClass()->GetWeaponEffectiveness(m_pShip, pWeapon, pTarget);
	if (iEffectiveness < 0)
		return 0;

	//	Get the range of this weapon

	Metric rRange2 = pWeapon->GetClass()->GetMaxEffectiveRange(m_pShip, pWeapon, pTarget);
	rRange2 *= rRange2;

	//	If the weapon is out of range of the target then we score 1
	//	(meaning that it is better than nothing (0) but we would rather any
	//	other weapon)

	if (rRange2 < rTargetDist2)
		return 1;

	//	If this weapon will take a while to get ready, then 
	//	lower the score.

	if (pWeapon->GetTimeUntilReady() >= 15)
		return 1;

	//	Get the item for the selected variant (either the weapon
	//	or the ammo)

	CItemType *pType;
	pWeapon->GetClass()->GetSelectedVariantInfo(m_pShip,
			pWeapon,
			NULL,
			NULL,
			&pType);

	//	Base score is based on the level of the variant

	iScore += pType->GetLevel() * 10;

	//	Missiles count for more

	if (pWeapon->GetCategory() == itemcatLauncher)
		{
		//	Don't waste missiles on "lesser" targets

		if (pTarget 
				&& pTarget->GetCategory() == CSpaceObject::catShip
				&& pTarget->GetLevel() <= (m_iBestNonLauncherWeaponLevel + 2)
				&& pTarget->GetLevel() <= (pType->GetLevel() - 2)
				&& pTarget != g_pUniverse->GetPlayer())
			return 1;

		//	Otherwise, count for more

		else
			iScore += 20;
		}

	//	Adjust score based on effectiveness

	iScore += iEffectiveness;

	//	If this is an area weapon then make sure there aren't too many friendlies around

#if 0
	if (iScore > 0 && pWeapon->IsAreaWeapon(m_pShip))
		{
		int i;
		CSystem *pSystem = m_pShip->GetSystem();
		int iFireAngle = (m_pShip->GetRotation() + pWeapon->GetRotation()) % 360;

		for (i = 0; i < pSystem->GetObjectCount(); i++)
			{
			CSpaceObject *pObj = pSystem->GetObject(i);

			if (pObj && pObj != m_pShip
					&& pObj->CanAttack()
					&& !IsEnemy(pObj))
				{
				CVector vDist = GetPos() - pObj->GetPos();
				Metric rDist2 = vDist.Length2();

				if (rDist2 < MAX_AREA_WEAPON_CHECK2)
					{
					int iBearing = VectorToPolar(vDist);
					if (AreAnglesAligned(iFireAngle, iBearing, 170))
						return 1;
					}
				}
			}
		}
#endif

	return iScore;
	}

void CBaseShipAI::CancelAllOrders (void)

//	CancelAllOrders
//
//	Cancel orders

	{
	m_Orders.RemoveAll();
	FireOnOrderChanged();
	}

void CBaseShipAI::CancelCurrentOrder (void)

//	CancelCurrentOrder
//
//	Cancels the current order

	{
	if (m_Orders.GetCount() > 0)
		{
		m_Orders.Remove(0);
		FireOnOrderChanged();
		}
	}

void CBaseShipAI::CancelDocking (CSpaceObject *pTarget)

//	CancelDocking
//
//	Cancel docking with the given station

	{
	pTarget->Undock(m_pShip);
	m_fDockingRequested = false;
	}

bool CBaseShipAI::CheckForEnemiesInRange (CSpaceObject *pCenter, Metric rRange, int iInterval, CSpaceObject **retpTarget)

//	CheckForEnemiesInRange
//
//	Checks every given interval to see if there are enemy ships in range

	{
	if (m_pShip->IsDestinyTime(iInterval))
		{
		*retpTarget = CalcEnemyShipInRange(pCenter, rRange);

		if (*retpTarget)
			return true;
		else
			return false;
		}
	else
		return false;
	}

bool CBaseShipAI::CheckOutOfRange (CSpaceObject *pTarget, Metric rRange, int iInterval)

//	CheckOutOfRange
//
//	Checks to see if the ship is out of range. Returns TRUE if we are known to be
//	out of range.

	{
	if (m_pShip->IsDestinyTime(iInterval))
		{
		CVector vRange = pTarget->GetPos() - m_pShip->GetPos();
		Metric rDistance2 = vRange.Length2();

		return (rDistance2 > (rRange * rRange));
		}
	else
		return false;
	}

bool CBaseShipAI::CheckOutOfZone (CSpaceObject *pBase, Metric rInnerRadius, Metric rOuterRadius, int iInterval)

//	CheckOutOfZone
//
//	Checks to see if the ship is out of its zone.

	{
	if (m_pShip->IsDestinyTime(iInterval))
		{
		Metric rMaxRange2 = rOuterRadius * rOuterRadius;
		Metric rMinRange2 = rInnerRadius * rInnerRadius;

		CVector vRange = pBase->GetPos() - m_pShip->GetPos();
		Metric rDistance2 = vRange.Length2();

		return (rDistance2 > rMaxRange2 || rDistance2 < rMinRange2);
		}
	else
		return false;
	}

void CBaseShipAI::FireOnOrderChanged (void)

//	FireOnOrderChanged
//
//	Ship's current order has changed

	{
	if (m_pShip->GetClass()->HasOnOrderChangedEvent() && !m_fInOnOrderChanged)
		{
		m_fInOnOrderChanged = true;
 		m_pShip->FireOnOrderChanged();
		m_fInOnOrderChanged = false;
		}

	OnOrderChanged();
	}

void CBaseShipAI::FireOnOrdersCompleted (void)

//	FireOnOrdersCompleted
//
//	Ship has no more orders

	{
	if (m_pShip->GetClass()->HasOnOrdersCompletedEvent() && !m_fInOnOrdersCompleted)
		{
		m_fInOnOrdersCompleted = true;
 		m_pShip->FireOnOrdersCompleted();
		m_fInOnOrdersCompleted = false;
		}
	}

bool CBaseShipAI::FollowsObjThroughGate (CSpaceObject *pLeader)

//	FollowsObjThroughGate
//
//	Returns true if we will follow the leader through a gate

	{
	return (GetEscortPrincipal() == pLeader);
	}

int CBaseShipAI::GetCombatPower (void)

//	GetCombatPower
//
//	Returns the relative strength of this object in combat
//	Scale from 0 to 100 (Note: This works for ships up to
//	level XV).

	{
	int iPower = m_pShip->GetScore() / 100;
	if (iPower > 100)
		return 100;
	else if (iPower == 0)
		return 1;
	else
		return iPower;
	}

IShipController::OrderTypes CBaseShipAI::GetCurrentOrder (void) const

//	GetCurrentOrder
//
//	Returns current order

	{
	if (m_Orders.GetCount() == 0)
		return IShipController::orderNone;
	else
		return ((OrderEntry *)m_Orders.GetStruct(0))->Order;
	}

IShipController::OrderTypes CBaseShipAI::GetCurrentOrderEx (CSpaceObject **retpTarget, DWORD *retdwData)

//	GetCurrentOrderEx
//
//	Returns current order and related data

	{
	if (m_Orders.GetCount() == 0)
		{
		if (retpTarget)
			*retpTarget = NULL;
		if (retdwData)
			*retdwData = 0;
		return IShipController::orderNone;
		}
	else
		{
		if (retpTarget)
			*retpTarget = GetCurrentOrderTarget();
		if (retdwData)
			*retdwData = GetCurrentOrderData();
		return ((OrderEntry *)m_Orders.GetStruct(0))->Order;
		}
	}

CSpaceObject *CBaseShipAI::GetEscortPrincipal (void) const

//	GetEscortPrincipal
//
//	Returns the ship that is being escorted (or NULL)

	{
	switch (GetCurrentOrder())
		{
		case IShipController::orderEscort:
		case IShipController::orderFollow:
		case IShipController::orderFollowPlayerThroughGate:
			return GetCurrentOrderTarget();

		default:
			return NULL;
		}
	}

CSpaceObject *CBaseShipAI::GetWeaponTarget (int iDev)

//	GetWeaponTarget
//
//	Returns target for turret weapon

	{
	if (iDev >= devTurretWeapon1 && iDev <= devTurretWeapon6)
		//	We always set the fire angle for turret weapons, so we return
		//	NULL (meaning that the weapon should just fire along the
		//	fire angle).
		//
		//	We did this because we are too lazy to keep a target for
		//	each weapon (and deal with clearing the target when the
		//	target is destroyed). The only bad thing about this is that
		//	we cannot have tracking weapons as turrets (i.e., secondary weapons).
		return NULL;
	else
		return GetTarget();
	}

void CBaseShipAI::HandleFriendlyFire (CSpaceObject *pOrderGiver)

//	HandleFriendlyFire
//
//	Ship has been hit by friendly fire

	{
	//	If the player hit us (and it seems to be on purpose) then raise an event

	if (pOrderGiver == g_pUniverse->GetPlayer() 
			&& m_Blacklist.Hit(m_pShip->GetSystem()->GetTick())
			&& m_pShip->GetClass()->HasOnAttackedByPlayerEvent())
		m_pShip->FireOnAttackedByPlayer();

	//	Otherwise, send the standard message

	else
		m_pShip->Communicate(pOrderGiver, msgWatchTargets);
	}

void CBaseShipAI::ImplementAttackNearestTarget (Metric rMaxRange, CSpaceObject **iopTarget, CSpaceObject *pExcludeObj)

//	ImplementAttackNearestTarget
//
//	Sets m_pTarget to be the nearest target and attacks it. This method
//	should only be used while in a state that does not need m_pTarget.

	{
	if (m_pShip->IsDestinyTime(19))
		(*iopTarget) = m_pShip->GetNearestVisibleEnemy(rMaxRange, false, pExcludeObj);

	if (*iopTarget)
		{
		CVector vTarget = (*iopTarget)->GetPos() - m_pShip->GetPos();
		Metric rTargetDist2 = vTarget.Dot(vTarget);

		//	Don't bother if the target is too far away

		if (rTargetDist2 > rMaxRange * rMaxRange)
			{
			(*iopTarget) = NULL;
			return;
			}

		ImplementFireWeaponOnTarget(-1, -1, *iopTarget, vTarget, rTargetDist2);

		//	If we don't have a good weapon, then don't go after this target

		if (m_iBestWeapon == devNone)
			(*iopTarget) = NULL;
		}
	}

void CBaseShipAI::ImplementAttackTarget (CSpaceObject *pTarget, bool bMaintainCourse, bool bDoNotShoot)

//	ImplementAttackTarget
//
//	Maneuvers towards and attacks target

	{
	CVector vTarget = pTarget->GetPos() - m_pShip->GetPos();
	Metric rTargetDist2 = vTarget.Dot(vTarget);

	//	Fire on the target as best we can

	int iFireDir;
	ImplementFireWeaponOnTarget(-1, -1, pTarget, vTarget, rTargetDist2, &iFireDir, bDoNotShoot);

	//	See if we need to alter course as part of combat.
	//	(unless we're supposed to maintain our course)

	if (bMaintainCourse || m_fImmobile)
		{
		}
	else
		{
		//	If the ship has shields then figure out their state

		CalcShieldState();

		//	Maneuver according to the ship's combat style

		switch (m_pAISettings->iCombatStyle)
			{
			case aicombatStandard:
				{
				//	If we're waiting for shields to regenerate, then
				//	spiral away

				if (m_fWaitForShieldsToRegen
						&& m_pShip->GetMaxSpeed() >= pTarget->GetMaxSpeed())
					{
					ImplementSpiralOut(vTarget, 75);
					iFireDir = -1;

					DEBUG_COMBAT_OUTPUT("Wait for shields");
					}

				//	If we're not well in range of our primary weapon then
				//	get closer to the target. (Or if we are not moving)

				else if (rTargetDist2 > m_rPrimaryAimRange2)
					{
					//	Try to flank our target, if we are faster

					bool bFlank = (m_pShip->GetMaxSpeed() > pTarget->GetMaxSpeed());
					ImplementCloseOnTarget(pTarget, vTarget, rTargetDist2, bFlank);

					//	Do not maneuver to aim

					iFireDir = -1;

					DEBUG_COMBAT_OUTPUT("Close on target");
					}

				//	If we're attacking a station, then keep our distance so that
				//	we don't get caught in the explosion

				else if (m_fAvoidExplodingStations
						&& rTargetDist2 < MIN_STATION_TARGET_DIST2 
						&& pTarget->GetMass() > 5000.0)
					{
					ImplementSpiralOut(vTarget);
					iFireDir = -1;

					DEBUG_COMBAT_OUTPUT("Spiral away to avoid explosion");
					}
				else if (rTargetDist2 < MIN_TARGET_DIST2)
					{
					ImplementSpiralOut(vTarget);
					iFireDir = -1;

					DEBUG_COMBAT_OUTPUT("Spiral away");
					}
				else if (pTarget->CanMove()
						&& (m_pShip->GetVel().Length2() < (0.01 * 0.01 * LIGHT_SPEED * LIGHT_SPEED)))
					{
					ImplementSpiralOut(vTarget);
					iFireDir = -1;

					DEBUG_COMBAT_OUTPUT("Speed away");
					}

				break;
				}

			case aicombatStandOff:
				{
				Metric rMaxRange2 = m_rBestWeaponRange * m_rBestWeaponRange;
				Metric rIdealRange2 = 0.45 * rMaxRange2;

				//	If we're beyond our weapon's max range, then close on target

				if (rTargetDist2 > rMaxRange2)
					{
					ImplementCloseOnTarget(pTarget, vTarget, rTargetDist2);
					iFireDir = -1;
					}

				//	If we're inside the ideal range, then move away from the target

				else if (rTargetDist2 < rIdealRange2)
					{
					ImplementSpiralOut(vTarget, 45);
					iFireDir = -1;
					}

				break;
				}

			case aicombatFlyby:
				{
				Metric rCloseRange2 = 0.25 * m_rPrimaryAimRange2;

				//	If we're beyond our weapon's effective range, then close on target

				if (rTargetDist2 > m_rPrimaryAimRange2)
					{
					ImplementCloseOnTarget(pTarget, vTarget, rTargetDist2);
					iFireDir = -1;
					}

				//	If we're too close to the target, move

				else if (rTargetDist2 < rCloseRange2)
					{
					//	Compute our bearing from the target's perspective

					int iBearing = VectorToPolar(-vTarget);

					//	If the target is facing us, then spiral away

					if (AreAnglesAligned(iBearing, pTarget->GetRotation(), 90))
						{
						ImplementSpiralOut(vTarget);
						iFireDir = -1;
						}
					}

				//	Otherwise, make sure we are moving wrt to the target

				else
					{
					CVector vVDelta = pTarget->GetVel() - m_pShip->GetVel();
					if (vVDelta.Length2() < MIN_FLYBY_SPEED2)
						{
						if (m_pShip->GetDestiny() > 180)
							ImplementSpiralIn(vTarget);
						else
							ImplementCloseOnTarget(pTarget, vTarget, rTargetDist2);

						iFireDir = -1;
						}
					}

				break;
				}

			case aicombatNoRetreat:
				{
				//	If we're not well in range of our primary weapon then
				//	get closer to the target. (Or if we are not moving)

				if (rTargetDist2 > m_rPrimaryAimRange2)
					{
					//	Try to flank our target, if we are faster

					bool bFlank = (m_pShip->GetMaxSpeed() > pTarget->GetMaxSpeed());
					ImplementCloseOnTarget(pTarget, vTarget, rTargetDist2, bFlank);

					//	Do not maneuver to aim

					iFireDir = -1;

					DEBUG_COMBAT_OUTPUT("Close on target");
					}
				else if (pTarget->CanMove()
						&& (m_pShip->GetVel().Length2() < (0.01 * 0.01 * LIGHT_SPEED * LIGHT_SPEED)))
					{
					ImplementSpiralOut(vTarget);
					iFireDir = -1;

					DEBUG_COMBAT_OUTPUT("Speed away");
					}

				break;
				}
			}
		}

	//	If necessary, turn to aim our weapon based on the fire solution

	if (iFireDir != -1
			&& !(m_pAISettings->dwFlags & aiflagNoDogfights))
		ImplementManeuver(iFireDir, false);
	}

void CBaseShipAI::ImplementCloseOnImmobileTarget (CSpaceObject *pTarget, 
												  const CVector &vTarget, 
												  Metric rTargetDist2)

//	ImplementCloseOnImmobileTarget
//
//	Head towards a target like a station or stargate

	{
	//	If we're close to the target, then we need to be more precise

	if (rTargetDist2 < CLOSE_RANGE2)
		{
		//	Compute our velocity relative to the target

		CVector vTargetVel;
		if (pTarget)
			vTargetVel = pTarget->GetVel() - m_pShip->GetVel();
		else
			vTargetVel = -m_pShip->GetVel();

		//	Decompose velocity into radial and tangental velocity

		Metric rTargetDist;
		CVector vRadial = vTarget.Normal(&rTargetDist);
		CVector vTangent = vRadial.Perpendicular();

		Metric rRadial = -vTargetVel.Dot(vRadial);
		Metric rTangent = vTargetVel.Dot(vTangent);

		//	If our tangential velocity is greater than our radial velocity, then
		//	we need to cancel out our tangential motion

		int iDirectionToFace;
		if (Absolute(rTangent) > rRadial)
			{
			iDirectionToFace = VectorToPolar(rTangent * vTangent, NULL);

			//	Continue correcting for a bit

			m_iManeuverCounter = 10 * m_pShip->GetManeuverDelay();
			}

		//	If we're correcting, continue thrusting

		else if (m_iManeuverCounter && Absolute(rTangent) > 0.25 * rRadial)
			{
			iDirectionToFace = VectorToPolar(rTangent * vTangent, NULL);
			m_iManeuverCounter--;
			}

		//	Otherwise, just head for the target

		else
			{
			CVector vInterceptPoint = vTarget + (vTargetVel * g_SecondsPerUpdate);

			iDirectionToFace = VectorToPolar(vInterceptPoint, NULL);
			m_iManeuverCounter = 0;
			}

		ImplementManeuver(iDirectionToFace, true);
		}

	//	Otherwise, just head for the target

	else
		{
		int iDirectionToFace = VectorToPolar(vTarget, NULL);
		m_iManeuverCounter = 0;

		ImplementManeuver(iDirectionToFace, true);
		}
	}

void CBaseShipAI::ImplementCloseOnTarget (CSpaceObject *pTarget,
										  const CVector &vTarget, 
										  Metric rTargetDist2, 
										  bool bFlank)

//	ImplementCloseOnTarget
//
//	Head towards the target.

	{
	CVector vInterceptPoint;

#ifdef DEBUG_SHIP
	bool bDebug = m_pShip->IsSelected();
#endif

	//	If we don't have a target object, then go to the position

	if (pTarget == NULL)
		vInterceptPoint = vTarget;

	//	If we are very far (>10M klicks) from the target then 
	//	compensate for the target's motion.

	else if (rTargetDist2 > 1.0e14)
		{
		//	Compute the speed with which the target is closing
		//	the distance (this may be a negative number). Note
		//	that we care about the target's absolute velocity
		//	(not its relative velocity because we are trying to
		//	adjust our velocity).

		CVector vAbsVel = pTarget->GetVel();
		Metric rClosingSpeed = -vAbsVel.Dot(vTarget.Normal());

		//	Figure out how long it will take to overtake the target's
		//	current position at maximum speed. (This is just a heuristic
		//	that gives us an approximation of the length of an intercept
		//	course.)

		rClosingSpeed = rClosingSpeed + m_pShip->GetMaxSpeed();
		if (rClosingSpeed > 0.0)
			{
			Metric rTimeToIntercept = vTarget.Length() / (rClosingSpeed);
			vInterceptPoint = vTarget + vAbsVel * rTimeToIntercept;
			}
		else
			vInterceptPoint = vTarget;

#ifdef DEBUG_SHIP
		if (bDebug)
			g_pUniverse->DebugOutput("Adjust for distance: %d at %d%%c", 
					(int)(vTarget.Length() / LIGHT_SECOND),
					(int)(100.0 * rClosingSpeed / LIGHT_SPEED));
#endif
		}
	else
		{
		CVector vTargetVel = pTarget->GetVel() - m_pShip->GetVel();
		vInterceptPoint = vTarget + (vTargetVel * g_SecondsPerUpdate);

		//	Close on a flanking point

		if (bFlank)
			{
			CVector vFlankingLine = vInterceptPoint.Normal().Perpendicular();
			vFlankingLine = vFlankingLine * m_rFlankDist;

			vInterceptPoint = vInterceptPoint + vFlankingLine;

#ifdef DEBUG_SHIP
			if (bDebug)
				g_pUniverse->DebugOutput("Flank target");
#endif
			}
		}

	//	Face towards the interception point and thrust

	int iDirectionToFace = VectorToPolar(vInterceptPoint, NULL);
	ImplementManeuver(iDirectionToFace, true);
	}

void CBaseShipAI::ImplementDocking (CSpaceObject *pTarget)

//	ImplementDocking
//
//	Docks with target

	{
	CVector vTarget = pTarget->GetPos() - m_pShip->GetPos();
	Metric rTargetDist2 = vTarget.Dot(vTarget);

	if (rTargetDist2 > (MAX_DOCK_DISTANCE * MAX_DOCK_DISTANCE))
		ImplementCloseOnImmobileTarget(pTarget, vTarget, rTargetDist2);
	else if (!m_fDockingRequested && m_pShip->IsDestinyTime(20))
		{
		if (pTarget->RequestDock(m_pShip))
			m_fDockingRequested = true;
		}
	}

void CBaseShipAI::ImplementEscort (CSpaceObject *pBase, CSpaceObject **iopTarget)

//	ImplementEscort
//
//	Basic escort algorithm

	{
	ASSERT(pBase);
	CVector vTarget = pBase->GetPos() - m_pShip->GetPos();
	Metric rTargetDist2 = vTarget.Dot(vTarget);

	if (rTargetDist2 > (MAX_ESCORT_DISTANCE * MAX_ESCORT_DISTANCE))
		ImplementCloseOnTarget(pBase, vTarget, rTargetDist2);
	else
		{
		if (*iopTarget)
			ImplementAttackTarget(*iopTarget, true);
		else
			ImplementEscortManeuvers(pBase, vTarget);
		}

	ImplementAttackNearestTarget(ATTACK_RANGE, iopTarget, pBase);
	ImplementFireOnTargetsOfOpportunity(NULL, pBase);
	}

void CBaseShipAI::ImplementEscortManeuvers (CSpaceObject *pTarget, const CVector &vTarget)

//	ImplementEscortManeuvers
//
//	Maneuvers to escort the given target

	{
	//	Pick escort position relative to our current position
	//	at time = 0

	if ((m_pShip->GetSystem()->GetTick() % 3) <= (m_pShip->GetDestiny() % 3))
		{
		CVector vEscortPos = pTarget->GetPos() 
				+ PolarToVector((m_pShip->GetDestiny() * 30) % 360, ESCORT_DISTANCE * (0.75 + (m_pShip->GetDestiny() / 720.0)));

		//	Maneuver towards the position

		ImplementFormationManeuver(vEscortPos, pTarget->GetVel(), pTarget->GetRotation());
		}
	}

void CBaseShipAI::ImplementEvasiveManeuvers (CSpaceObject *pTarget)

//	ImplementEvasiveManeuvers
//
//	Move away from the given target

	{
	CVector vTarget = pTarget->GetPos() - m_pShip->GetPos();
	int iDirectionToFace = VectorToPolar(-vTarget, NULL);
	ImplementManeuver(iDirectionToFace, true);
	}

void CBaseShipAI::ImplementFireOnTarget (CSpaceObject *pTarget, bool *retbOutOfRange)

//	ImplementFireOnTarget
//
//	Fires on the given target, if aligned and in range

	{
	bool bOutOfRange;

	ASSERT(pTarget);

	CVector vTarget = pTarget->GetPos() - m_pShip->GetPos();
	Metric rTargetDist2 = vTarget.Dot(vTarget);

	//	Don't bother if the target is too far away

	if (rTargetDist2 > MAX_TARGET_OF_OPPORTUNITY_RANGE * MAX_TARGET_OF_OPPORTUNITY_RANGE)
		bOutOfRange = true;
	else
		{
		ImplementFireWeaponOnTarget(-1, -1, pTarget, vTarget, rTargetDist2);
		bOutOfRange = false;
		}

	//	Done

	if (retbOutOfRange)
		*retbOutOfRange = bOutOfRange;
	}

void CBaseShipAI::ImplementFireOnTargetsOfOpportunity (CSpaceObject *pTarget, CSpaceObject *pExcludeObj)

//	ImplementFireOnTargetsOfOpportunity
//
//	Attacks any targets in the area with secondary weapons

	{
	int i;

	//	If this ship has secondary weapons that also fire at will
	//	(note that we don't turn this on normally because it is relatively
	//	expensive)

	if (m_pShip->HasSecondaryWeapons())
		{
		//	Fire

		CIntArray Targets;
		int iCount = 0;

		//	First build a list of the nearest enemy ships within
		//	25 light-seconds of the ship.

		iCount += m_pShip->GetNearestVisibleEnemies(MAX_TARGETS,
				LIGHT_SECOND * 25.0,
				&Targets,
				pExcludeObj);

		//	If we've got a target, add it to the list. Sometimes this will be 
		//	a duplicate, but that's OK.

		if (pTarget)
			{
			Targets.AppendElement((int)pTarget, 0);
			iCount++;
			}

#ifdef DEBUG_FIRE_ON_OPPORTUNITY
		g_pUniverse->DebugOutput("Enemies: %d", iCount);
#endif

		//	If we've got enemy ships, see if any weapons are in range

		if (iCount > 0)
			{
			for (i = devTurretWeapon1; i <= devTurretWeapon6; i++)
				{
				CInstalledDevice *pWeapon = m_pShip->GetNamedDevice((DeviceNames)i);

				if (pWeapon && pWeapon->IsReady())
					{
					Metric rMaxRange = pWeapon->GetClass()->GetMaxEffectiveRange(m_pShip, pWeapon, NULL);
					Metric rMaxRange2 = rMaxRange * rMaxRange;

					for (int j = 0; j < iCount; j++)
						{
						int iFireAngle;
						CSpaceObject *pTarget = (CSpaceObject *)Targets.GetElement(j);
						Metric rDist2 = (pTarget->GetPos() - m_pShip->GetPos()).Length2();

						if (rDist2 < rMaxRange2 
								&& pWeapon->GetWeaponEffectiveness(m_pShip, pTarget) >=0
								&& m_pShip->IsWeaponAligned((DeviceNames)i, pTarget, NULL, &iFireAngle))
							{
							if (m_pShip->IsLineOfFireClear(m_pShip->GetPos(), pTarget, iFireAngle, rMaxRange))
								{
								SetFireWeaponFlag(i);
								pWeapon->SetFireAngle(iFireAngle);
								break;
								}
							}
						}
					}
				}
			}
		}
	}

void CBaseShipAI::ImplementFireWeapon (DeviceNames iDev)

//	ImplementFireWeapon
//
//	Fires the given weapon blindly

	{
	CInstalledDevice *pWeapon;

	if (iDev == devNone)
		{
		CalcBestWeapon(NULL, 0.0);

		if (m_iBestWeapon == devNone)
			return;

		iDev = m_iBestWeapon;
		pWeapon = m_pBestWeapon;
		}
	else
		pWeapon = m_pShip->GetNamedDevice(iDev);

	if (pWeapon && pWeapon->IsReady())
		{
		int iFireAngle = (m_pShip->GetRotation() + pWeapon->GetRotation()) % 360;
		if (m_pShip->IsLineOfFireClear(m_pShip->GetPos(), 
				NULL, 
				iFireAngle, 
				DEFAULT_DIST_CHECK, 
				pWeapon->IsAreaWeapon(m_pShip)))
			{
			SetFireWeaponFlag(iDev);
			pWeapon->SetFireAngle(iFireAngle);
			}
		}
	}

void CBaseShipAI::ImplementFireWeaponOnTarget (int iWeapon,
											   int iWeaponVariant,
											   CSpaceObject *pTarget,
											   const CVector &vTarget,
											   Metric rTargetDist2,
											   int *retiFireDir,
											   bool bDoNotShoot)

//	ImplementFireWeaponOnTarget
//
//	Fires the given weapon (if aligned) on target

	{
	int iFireDir = -1;
	int iTick = m_pShip->GetSystem()->GetTick();

#ifdef DEBUG
	bool bDebug = m_pShip->IsSelected();
#endif

	ASSERT(pTarget);

	//	Select the appropriate weapon. If we're not given a weapon, then choose the
	//	best one.

	DeviceNames iWeaponToFire;
	Metric rWeaponRange;
	if (iWeapon == -1)
		{
		if (((iTick % 30) == 0)
				&& (m_fHasMultipleWeapons || m_iBestWeapon == devNone))
			ClearBestWeapon();

		CalcBestWeapon(pTarget, rTargetDist2);
		if (m_iBestWeapon == devNone)
			{
			if (retiFireDir)
				*retiFireDir = -1;

			DEBUG_COMBAT_OUTPUT("Fire: No appropriate weapon found");
			return;
			}

		iWeaponToFire = m_iBestWeapon;
		rWeaponRange = m_rBestWeaponRange;
		}
	else
		{
		iWeaponToFire = m_pShip->SelectWeapon(iWeapon, iWeaponVariant);
		rWeaponRange = m_pShip->GetWeaponRange(iWeaponToFire);
		}

	//	See if the chosen weapon can hit the target

	int iAimAngle = m_pShip->GetRotation();
	int iFireAngle = -1;
	bool bAligned;
	bAligned = m_pShip->IsWeaponAligned(iWeaponToFire,
			pTarget, 
			&iAimAngle,
			&iFireAngle);

	//	There is a chance of missing

	if (m_pShip->GetWeaponIsReady(iWeaponToFire))
		{
		if (bAligned)
			{
			if (mathRandom(1, 100) > m_pAISettings->iFireAccuracy)
				{
				bAligned = false;

				//	In this case, we happen to be aligned, but because of inaccuracy
				//	reason we think we're not. We clear the aim angle because for
				//	omnidirectional weapons, we don't want to try to turn towards
				//	the new aim point.

				iAimAngle = -1;
				DEBUG_COMBAT_OUTPUT("Aim error: hold fire when aligned");
				}
			}
		else if (iAimAngle != -1)
			{
			int iPrematureFiring = (100 - m_pAISettings->iFireAccuracy) / 5;
			if (mathRandom(1, 100) <= iPrematureFiring)
				{
				int iFacingAngle = iFireAngle;
				if (iFacingAngle < iAimAngle)
					iFacingAngle += 360;

				int iAimOffset = 180 - abs((iFacingAngle - iAimAngle) - 180);
				if (iAimOffset < 20)
					{
					bAligned = true;
					DEBUG_COMBAT_OUTPUT("Aim error: fire when not aligned");
					}
				}
			}
		}

	//	Fire

	if (bAligned)
		{
#ifdef DEBUG
		{
		char szDebug[1024];
		wsprintf(szDebug, "Fire: Weapon aligned  iAim=%d  iFireAngle=%d", iAimAngle, iFireAngle);
		DEBUG_COMBAT_OUTPUT(szDebug);
		}
#endif

		//	If we're aligned and the weapon is ready, and we're
		//	in range of the target, then fire!

		if (m_pShip->GetWeaponIsReady(iWeaponToFire)
				&& rTargetDist2 < (rWeaponRange * rWeaponRange))
			{
			CInstalledDevice *pWeapon = m_pShip->GetNamedDevice(iWeaponToFire);

			if (iWeaponToFire == devPrimaryWeapon)
				{
				if (m_pShip->IsLineOfFireClear(m_pShip->GetPos(), 
						pTarget, 
						iFireAngle, 
						DEFAULT_DIST_CHECK, 
						pWeapon->IsAreaWeapon(m_pShip)))
					{
					if (!bDoNotShoot)
						SetFireWeaponFlag(iWeaponToFire);
					DEBUG_COMBAT_OUTPUT("FireOnTarget: Fire primary!");
					}
				else
					DEBUG_COMBAT_OUTPUT("FireOnTarget: Friendlies in line of fire");
				}
			else
				{
				if (m_pShip->IsLineOfFireClear(m_pShip->GetPos(),
						pTarget, 
						iFireAngle, 
						DEFAULT_DIST_CHECK, 
						pWeapon->IsAreaWeapon(m_pShip)))
					{
					if (!bDoNotShoot)
						SetFireWeaponFlag(iWeaponToFire);
					DEBUG_COMBAT_OUTPUT("FireOnTarget: Fire missile!");
					}
				else
					DEBUG_COMBAT_OUTPUT("FireOnTarget: Friendlies in line of fire");
				}
			}
		}
	else
		{
		DEBUG_COMBAT_OUTPUT("Fire: Weapon NOT aligned");

		//	If the weapon is not aligned, turn to aim

		if (iAimAngle != -1)
			iFireDir = iAimAngle;

#ifdef DEBUG_SHIP
		if (bDebug)
			g_pUniverse->DebugOutput("Face target at distance: %d moving at: %d%%c", 
					(int)(vTarget.Length() / LIGHT_SECOND),
					(int)(100.0 * 0 / LIGHT_SPEED));
#endif

		//	If the primary weapon happens to be aligned then
		//	fire it.

#if 0
		if (iWeaponToFire != devPrimaryWeapon)
			{
			int iDummy;
			int iFireAngle;
			if (m_pShip->IsWeaponAligned(devPrimaryWeapon,
					pTarget, 
					&iDummy,
					&iFireAngle))
				{
				if (m_pShip->GetWeaponIsReady(devPrimaryWeapon))
					{
					if (m_pShip->IsLineOfFireClear(m_pShip->GetPos(), pTarget, iFireAngle))
						SetFireWeaponFlag(devPrimaryWeapon);
					}
				}
			}
#endif
		}

	if (retiFireDir)
		*retiFireDir = iFireDir;
	}

void CBaseShipAI::ImplementFollowNavPath (bool *retbAtDestination)

//	ImplementFollowNavPath
//
//	Ship follows nav path to its destination

	{
	//	Figure out our next point along the path

	CVector vTarget = m_pNavPath->GetNavPoint(m_iNavPathPos) - m_pShip->GetPos();

	//	Are we at our target? If so, then we move on to
	//	the next nav point

	Metric rTargetDist2 = vTarget.Length2();
	if (rTargetDist2 < HIT_NAV_POINT_DIST2)
		{
		//	If we're at the last nav point, then we've reached our
		//	destination.

		if (m_iNavPathPos + 1 == m_pNavPath->GetNavPointCount())
			{
			if (retbAtDestination)
				*retbAtDestination = true;
			return;
			}

		//	Otherwise, we go to the next nav point

		m_iNavPathPos++;
		vTarget = m_pNavPath->GetNavPoint(m_iNavPathPos) - m_pShip->GetPos();
		rTargetDist2 = vTarget.Length2();
		}

	//	Navigate towards the next nav point

	ImplementCloseOnImmobileTarget(NULL, vTarget, rTargetDist2);

	//	Done

	if (retbAtDestination)
		*retbAtDestination = false;
	}

void CBaseShipAI::ImplementFormationManeuver (const CVector vDest, 
											  const CVector vDestVel, 
											  int iDestFacing,
											  bool *retbInFormation)

//	ImplementFormationManeuver
//
//	Moves the ship to the given formation point

	{
	//	Figure out how far we are from where we want to be

	CVector vDelta = vDest - m_pShip->GetPos();
	CVector vDeltaVel = vDestVel - m_pShip->GetVel();

	//	If our position and velocity are pretty close, then stay
	//	where we are (though we cheat a little by adjusting our velocity
	//	manually)

	Metric rDelta2 = vDelta.Length2();
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
		rDesiredVelX = (Min(MAX_DISTANCE, rDeltaX) / MAX_DISTANCE) * m_pShip->GetMaxSpeed();
	else
		rDesiredVelX = (Max(-MAX_DISTANCE, rDeltaX) / MAX_DISTANCE) * m_pShip->GetMaxSpeed();

	//	Same with our velocity along the axis

	Metric rDesiredVelY;
	if (bCloseEnough)
		rDesiredVelY = 0.0;
	else if (rDeltaY > 0.0)
		rDesiredVelY = (Min(MAX_DISTANCE, rDeltaY) / MAX_DISTANCE) * m_pShip->GetMaxSpeed();
	else
		rDesiredVelY = (Max(-MAX_DISTANCE, rDeltaY) / MAX_DISTANCE) * m_pShip->GetMaxSpeed();

	//	Recompose to our desired velocity

	CVector vDesiredVel = (rDesiredVelX * vAxisX) + (rDesiredVelY * vAxisY);
	vDesiredVel = vDesiredVel + vDestVel;

	//	Figure out the delta v that we need to achieve our desired velocity

	CVector vDiff = vDesiredVel - m_pShip->GetVel();
	Metric rDiff2 = vDiff.Length2();

	//	If we're close enough to the velocity, cheat a little by
	//	accelerating without using the main engine

	if (rDiff2 < MAX_DELTA_VEL2)
		{
		m_pShip->Accelerate(vDiff * m_pShip->GetMass() / 2000.0, g_SecondsPerUpdate);

		ImplementTurnTo(iDestFacing);
		}

	//	Otherwise, thrust with the main engines

	else
		{
		//	Figure out which direction we need to thrust in to achieve the delta v

		int iAngle = VectorToPolar(vDiff);

		//	Turn towards the angle

		ImplementTurnTo(iAngle);

		//	If we don't need to turn, engage thrust

		if (m_iManeuver == IShipController::NoRotation)
			m_iThrustDir = constAlwaysThrust;
		}

	//	See if we're in formation

	if (retbInFormation)
		*retbInFormation = ((m_pShip->GetRotation() == iDestFacing)
				&& (rDelta2 < MAX_IN_FORMATION_DELTA2)
				&& (rDiff2 < MAX_DELTA_VEL2));
	}

void CBaseShipAI::ImplementGating (CSpaceObject *pTarget)

//	ImplementGating
//
//	Implementing gating out

	{
	ASSERT(pTarget);
	CVector vTarget = pTarget->GetPos() - m_pShip->GetPos();
	Metric rTargetDist2 = vTarget.Dot(vTarget);

	if (rTargetDist2 < (MAX_GATE_DISTANCE * MAX_GATE_DISTANCE))
		pTarget->RequestGate(m_pShip);
	else if (rTargetDist2 < (DOCKING_APPROACH_DISTANCE * DOCKING_APPROACH_DISTANCE))
		{
		m_pShip->SetMaxSpeedHalf();
		ImplementCloseOnImmobileTarget(pTarget, vTarget, rTargetDist2);
		}
	else
		ImplementCloseOnImmobileTarget(pTarget, vTarget, rTargetDist2);
	}

void CBaseShipAI::ImplementHold (bool *retbInPlace)

//	ImplementHold
//
//	Stop by thrusting as soon as possible

	{
	bool bInPlace;

	//	Compute our velocity

	CVector vVel = m_pShip->GetVel();
	Metric rVel2 = vVel.Length2();

	//	If we're going slow enough, we cheat a bit

	if (rVel2 < MAX_DELTA_VEL2)
		{
		ImplementStop();
		bInPlace = true;
		}

	//	Otherwise, we thrust to cancel our velocity

	else
		{
		//	Figure out which direction we need to thrust in to achieve the delta v

		int iAngle = VectorToPolar(-vVel);

		//	Turn towards the angle

		ImplementTurnTo(iAngle);

		//	If we don't need to turn, engage thrust

		if (m_iManeuver == IShipController::NoRotation)
			m_iThrustDir = constAlwaysThrust;

		bInPlace = false;
		}

	if (retbInPlace)
		*retbInPlace = bInPlace;
	}

void CBaseShipAI::ImplementManeuver (int iDir, bool bThrust, bool bNoThrustThroughTurn)

//	ImplementManeuver
//
//	Turns the ship towards the given direction. This function sets the variables:
//
//	m_iManeuver
//	m_iThrustDir
//	m_iLastTurn
//	m_iLastTurnCount

	{
#ifdef DEBUG_SHIP
	bool bDebug = m_pShip->IsSelected();
#endif

	if (iDir != -1)
		{
		int iCurrentDir = m_pShip->GetRotation();

		//	If we're within a few degrees of where we want to be, then
		//	don't bother changing

		if (!AreAnglesAligned(iDir, iCurrentDir, 9))
			{
			int iTurn = (iDir + 360 - iCurrentDir) % 360;

			if (iTurn >= 180)
				m_iManeuver = IShipController::RotateRight;
			else
				m_iManeuver = IShipController::RotateLeft;

			//	If we're turning in a new direction now, then reset
			//	our counter

			if (m_iManeuver != m_iLastTurn)
				{
				m_iLastTurn = m_iManeuver;
				m_iLastTurnCount = 0;
				}
			else
				{
				m_iLastTurnCount++;

				//	If we've been turning in the same direction
				//	for a while, then arbitrarily turn in the opposite
				//	direction for a while.

				if (m_iLastTurnCount > m_iMaxTurnCount)
					{
					if (m_iManeuver == IShipController::RotateRight)
						m_iManeuver = IShipController::RotateLeft;
					else
						m_iManeuver = IShipController::RotateRight;
#ifdef DEBUG_SHIP
					if (bDebug)
						g_pUniverse->DebugOutput("Reverse direction");
#endif
					}
				}

#ifdef DEBUG_SHIP
			if (bDebug)
				g_pUniverse->DebugOutput("Turn: %s (%d -> %d)",
						(m_iManeuver == IShipController::RotateRight ? "right" : 
							(m_iManeuver == IShipController::RotateLeft ? "left" : "none")),
						iCurrentDir,
						iDir);
#endif

			//	Do not thrust (unless we want to thrust through the turn)

			m_iThrustDir = constNeverThrust;
			if (bThrust && m_fThrustThroughTurn && !bNoThrustThroughTurn)
				m_iThrustDir = constAlwaysThrust;
			}
		else
			{
			if (bThrust)
				m_iThrustDir = iCurrentDir;
			else
				m_iThrustDir = constNeverThrust;

#ifdef DEBUG_SHIP
			if (bDebug)
				g_pUniverse->DebugOutput("Turn: none (%d)", iDir);
#endif
			}
		}
	else
		{
		if (bThrust)
			m_iThrustDir = m_pShip->GetRotation();
		else
			m_iThrustDir = constNeverThrust;

#ifdef DEBUG_SHIP
		if (bDebug)
			g_pUniverse->DebugOutput("Turn: none");
#endif
		}
	}

void CBaseShipAI::ImplementOrbit (CSpaceObject *pBase, Metric rDistance)

//	ImplementOrbit
//
//	Orbit around object

	{
	CVector vTarget = pBase->GetPos() - m_pShip->GetPos();
	Metric rTargetDist2 = vTarget.Dot(vTarget);

	Metric rMaxDist = rDistance * 1.1;
	Metric rMinDist = rDistance * 0.9;

	if (rTargetDist2 > (rMaxDist * rMaxDist))
		ImplementSpiralIn(vTarget);
	else if (rTargetDist2 < (rMinDist * rMinDist))
		ImplementSpiralOut(vTarget);
	}

void CBaseShipAI::ImplementSpiralIn (const CVector &vTarget)

//	SpiralIn
//
//	Spiral in towards the target

	{
	CVector vTangent = (vTarget.Perpendicular()).Normal() * m_pShip->GetMaxSpeed() * g_SecondsPerUpdate * 8;

	Metric rRadius;
	int iAngle = VectorToPolar(vTangent, &rRadius);

	//	Curve inward

	CVector vInterceptPoint = PolarToVector(iAngle + 330, rRadius);

	//	Face towards the interception point

	int iDirectionToFace = VectorToPolar(vInterceptPoint, NULL);
	ImplementManeuver(iDirectionToFace, true);
	}

void CBaseShipAI::ImplementSpiralOut (const CVector &vTarget, int iTrajectory)

//	SpiralOut
//
//	Spiral out away from the target

	{
	CVector vTangent = (vTarget.Perpendicular()).Normal() * m_pShip->GetMaxSpeed() * g_SecondsPerUpdate * 8;

	Metric rRadius;
	int iAngle = VectorToPolar(vTangent, &rRadius);

	//	Curve out

	CVector vInterceptPoint = PolarToVector(iAngle + iTrajectory, rRadius);

	//	Face towards the interception point

	int iDirectionToFace = VectorToPolar(vInterceptPoint, NULL);
	ImplementManeuver(iDirectionToFace, true);
	}

void CBaseShipAI::ImplementStop (void)

//	ImplementStop
//
//	Stops in space

	{
	Metric rSpeed;
	int iAngle = VectorToPolar(m_pShip->GetVel(), &rSpeed);
	Metric rThrust = m_pShip->GetThrust();

	if (rSpeed > g_KlicksPerPixel)
		{
		CVector vAccel = PolarToVector(iAngle, -rThrust);

		m_pShip->Accelerate(vAccel, g_SecondsPerUpdate);
		}
	else
		m_pShip->Accelerate(-m_pShip->GetVel() * m_pShip->GetMass() / 2000.0, g_SecondsPerUpdate);
	}

void CBaseShipAI::ImplementTurnTo (int iRotation)

//	ImplementTurnTo
//
//	Turn towards the given angle

	{
	int iCurrentDir = m_pShip->GetRotation();
	int iTurn = (iRotation + 360 - iCurrentDir) % 360;

	if ((iTurn >= (360 - (m_pShip->GetRotationAngle() / 2)))
			|| (iTurn <= (m_pShip->GetRotationAngle() / 2)))
		m_iManeuver = IShipController::NoRotation;
	else
		{
		if (iTurn >= 180)
			m_iManeuver = IShipController::RotateRight;
		else
			m_iManeuver = IShipController::RotateLeft;
		}
	}

void CBaseShipAI::OnDocked (CSpaceObject *pObj)

//	OnDocked
//
//	Event when the ship is docked

	{
	//	NOTE: At SystemCreate time we dock ships to their station without them
	//	requesting it, thus we cannot assume that m_bDockingRequested is TRUE.
	//	(However, we set state to None no matter what).

	m_fDockingRequested = false;

	//	Call descendants

	OnDockedEvent(pObj);
	}

void CBaseShipAI::OnEnterGate (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate)

//	OnEnterGate
//
//	Ship enters a stargate
	
	{
	//	If our orders are to follow the player through a gate, then set the
	//	follow flag

	if (GetCurrentOrder() == IShipController::orderFollowPlayerThroughGate)
		m_pShip->SetFollowPlayerThroughGate();

	//	Otherwise, we destroy ourselves

	else
		m_pShip->SetDestroyInGate();
	}

void CBaseShipAI::OnObjEnteredGate (CSpaceObject *pObj, CSpaceObject *pStargate)

//	OnObjEnteredGate
//
//	Handle the case where another object entered a stargate

	{
	if (GetCurrentOrder() != IShipController::orderNone
			&& pObj == GetCurrentOrderTarget())
		{
		switch (GetCurrentOrder())
			{
			case IShipController::orderEscort:
			case IShipController::orderFollow:
				if (pObj == g_pUniverse->GetPlayer())
					{
					//	If the player just entered a stargate then see if we need to follow
					//	her or wait for her to return.

					if (m_pShip->Communicate(m_pShip, msgQueryWaitStatus) == resAck)
						AddOrder(IShipController::orderWaitForPlayer, NULL, 0, true);
					else
						AddOrder(IShipController::orderFollowPlayerThroughGate, pObj, 0, true);
					}
				else
					{
					//	Otherwise, we cancel our order and follow the ship through the gate

					CancelCurrentOrder();
					AddOrder(IShipController::orderGate, pStargate, 0, true);
					}
				break;

			//	If we're trying to destroy the player, then wait until she returns.

			case IShipController::orderDestroyTarget:
				if (pObj == g_pUniverse->GetPlayer())
					AddOrder(IShipController::orderDestroyPlayerOnReturn, NULL, 0, true);
				break;
			default:
				assert(0);
				break;
			}
		}
	}

void CBaseShipAI::OnObjDestroyed (const SDestroyCtx &Ctx)

//	OnObjDestroyed
//
//	Handle the case where an object has been removed from the system

	{
	//	If our current order is to follow the player through and we get
	//	a notification that the player has been removed, then do nothing
	//	(we will be updated when we enter the new system).

	if (GetCurrentOrder() == IShipController::orderFollowPlayerThroughGate
			&& Ctx.pObj == GetCurrentOrderTarget())
		return;

	//	Let our subclass deal with it...

	OnObjDestroyedNotify(Ctx);

	//	Loop over all our future orders and make sure that we
	//	delete any that refer to this object.

	for (int i = 0; i < m_Orders.GetCount(); i++)
		{
		OrderEntry *pOrder = (OrderEntry *)m_Orders.GetStruct(i);
		if (pOrder->pTarget == Ctx.pObj)
			{
			//	Remove the order

			m_Orders.Remove(i);
			if (i == 0)
				FireOnOrderChanged();
			i--;
			}
		}
	}

void CBaseShipAI::OnPlayerObj (CSpaceObject *pPlayer)

//	OnPlayerObj
//
//	Player has entered the system

	{
	if (GetCurrentOrder() == IShipController::orderDestroyPlayerOnReturn)
		{
		CancelCurrentOrder();
		AddOrder(IShipController::orderDestroyTarget, pPlayer, 0, true);
		}
	}

void CBaseShipAI::OnStationDestroyed (const SDestroyCtx &Ctx)

//	OnStationDestroyed
//
//	Station has been destroyed

	{
	//	Let our subclass deal with it...

	OnObjDestroyedNotify(Ctx);

	//	NOTE: We do not remove the object from all orders because otherwise
	//	we would remove orders to loot a station.
	}

void CBaseShipAI::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Reads controller data from stream
//
//	DWORD		Controller ObjID
//	DWORD		ship class UNID (needed to set AISettings)
//	DWORD		m_pShip (CSpaceObject ref)
//	DWORD		m_iManeuver
//	DWORD		m_iThrustDir
//	DWORD		m_iLastTurn
//	DWORD		m_iLastTurnCount
//	DWORD		m_Blacklist
//	DWORD		m_iManeuverCounter
//
//	DWORD		m_pNavPath (ID)
//	DWORD		m_iNavPathPos
//
//	DWORD		No of orders
//	DWORD		order: Order
//	DWORD		order: pTarget
//	DWORD		order: dwData
//
//	DWORD		flags

	{
	int i;
	DWORD dwLoad;

	//	Read stuff

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	CShipClass *pClass = g_pUniverse->FindShipClass(dwLoad);
	m_pAISettings = &pClass->GetAISettings();
	Ctx.pSystem->ReadObjRefFromStream(Ctx, (CSpaceObject **)&m_pShip);
	Ctx.pStream->Read((char *)&m_iManeuver, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iThrustDir, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iLastTurn, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iLastTurnCount, sizeof(DWORD));

	//	Read blacklist

	if (Ctx.dwVersion >= 6)
		m_Blacklist.ReadFromStream(Ctx);

	//	Read manuever counter

	if (Ctx.dwVersion >= 33)
		Ctx.pStream->Read((char *)&m_iManeuverCounter, sizeof(DWORD));

	//	Read navpath info

	if (Ctx.dwVersion >= 11)
		{
		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		if (dwLoad)
			m_pNavPath = Ctx.pSystem->GetNavPathByID(dwLoad);
		else
			m_pNavPath = NULL;

		Ctx.pStream->Read((char *)&m_iNavPathPos, sizeof(DWORD));

		if (m_pNavPath == NULL)
			m_iNavPathPos = -1;
		}

	//	Read orders

	DWORD dwCount;
	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		{
		OrderEntry Entry;
		m_Orders.AppendStruct(&Entry, NULL);
		}

	for (i = 0; i < (int)dwCount; i++)
		{
		OrderEntry *pEntry = (OrderEntry *)m_Orders.GetStruct(i);
		Ctx.pStream->Read((char *)&pEntry->Order, sizeof(DWORD));
		Ctx.pSystem->ReadObjRefFromStream(Ctx, &pEntry->pTarget);
		Ctx.pStream->Read((char *)&pEntry->dwData, sizeof(DWORD));
		}

	//	Flags

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_fDeviceActivate =		((dwLoad & 0x00000001) ? true : false);
	m_fDockingRequested =	((dwLoad & 0x00000002) ? true : false);
	m_fWaitForShieldsToRegen = ((dwLoad & 0x00000004) ? true : false);
	m_fRecalcBestWeapon = true;

	//	Subclasses

	OnReadFromStream(Ctx);
	}

void CBaseShipAI::ResetBehavior (void)

//	ResetBehavior
//
//	Resets fire and motion

	{
	if (!IsDockingRequested())
		{
		m_iManeuver = IShipController::NoRotation;
		m_iThrustDir = constNeverThrust;
		}

	ClearFireWeaponFlags();
	m_Blacklist.Update(g_pUniverse->GetTicks());
	}

void CBaseShipAI::SetCurrentOrderData (DWORD dwData)

//	SetCurrentOrderData
//
//	Set the data for current order

	{
	((OrderEntry *)m_Orders.GetStruct(0))->dwData = dwData;
	}

void CBaseShipAI::SetShipToControl (CShip *pShip)

//	SetShipToControl
//
//	Set the ship that this controller will be controlling

	{
	ASSERT(m_pShip == NULL);

	m_pShip = pShip;
	m_pAISettings = &pShip->GetClass()->GetAISettings();

	CalcInvariants();
	}

void CBaseShipAI::UpgradeShieldBehavior (void)

//	UpgradeShieldBehavior
//
//	Upgrade the ship's shields with a better one in cargo

	{
	CItem BestItem;
	int iBestLevel = 0;
	bool bReplace = false;

	//	Figure out the currently installed shields

	CInstalledDevice *pDevice = m_pShip->GetNamedDevice(devShields);
	if (pDevice)
		{
		iBestLevel = pDevice->GetClass()->GetLevel();
		bReplace = true;
		}

	//	Look for better shields

	CItemListManipulator ItemList(m_pShip->GetItemList());
	while (ItemList.MoveCursorForward())
		{
		const CItem &Item = ItemList.GetItemAtCursor();
		if (Item.GetType()->GetCategory() == itemcatShields
				&& !Item.IsInstalled()
				&& !Item.IsDamaged()
				&& Item.GetType()->GetLevel() >= iBestLevel)
			{
			//	If this device requires some item, then skip it for now
			//	(later we can add code to figure out if we've got enough
			//	of the item)

			if (Item.GetType()->GetDeviceClass()->RequiresItems())
				;

			//	If we cannot install this item, skip it

			else if (m_pShip->CanInstallDevice(Item, bReplace) != CShip::insOK)
				;

			//	Otherwise, remember this item

			else
				{
				BestItem = Item;
				iBestLevel = Item.GetType()->GetLevel();
				}
			}
		}

	//	If we found a better device, upgrade

	if (BestItem.GetType())
		{
		//	Uninstall the device weapon

		if (pDevice)
			{
			m_pShip->SetCursorAtNamedDevice(ItemList, devShields);
			m_pShip->RemoveItemAsDevice(ItemList);
			}

		//	Install the new item

		ItemList.SetCursorAtItem(BestItem);
		m_pShip->InstallItemAsDevice(ItemList);

		//	Recalc stuff

		CalcInvariants();
		}
	}

void CBaseShipAI::UpgradeWeaponBehavior (void)

//	UpgradeWeaponBehavior
//
//	Upgrade the ship's weapon with a better one in cargo

	{
	int i;

	//	Loop over all currently installed weapons

	bool bWeaponsInstalled = false;
	for (i = 0; i < m_pShip->GetDeviceCount(); i++)
		{
		CInstalledDevice *pDevice = m_pShip->GetDevice(i);
		if (!pDevice->IsEmpty() 
				&& pDevice->GetCategory() == itemcatWeapon)
			{
			//	Loop over all uninstalled weapons and see if we can
			//	find something better than this one.

			CItem BestItem;
			int iBestLevel = pDevice->GetClass()->GetLevel();

			CItemListManipulator ItemList(m_pShip->GetItemList());
			while (ItemList.MoveCursorForward())
				{
				const CItem &Item = ItemList.GetItemAtCursor();
				if (Item.GetType()->GetCategory() == itemcatWeapon
						&& !Item.IsInstalled()
						&& !Item.IsDamaged()
						&& Item.GetType()->GetLevel() >= iBestLevel)
					{
					//	If this weapon is an ammo weapon, then skip it for now
					//	(later we can add code to figure out if we've got enough
					//	ammo for the weapon)

					if (Item.GetType()->GetDeviceClass()->RequiresItems())
						;

					//	If we cannot install this item, skip it

					else if (m_pShip->CanInstallDevice(Item, true) != CShip::insOK)
						;

					//	Otherwise, remember this item

					else
						{
						BestItem = Item;
						iBestLevel = Item.GetType()->GetLevel();
						}
					}
				}

			//	If we found a better weapon, upgrade

			if (BestItem.GetType())
				{
				//	Uninstall the previous weapon

				m_pShip->SetCursorAtDevice(ItemList, i);
				m_pShip->RemoveItemAsDevice(ItemList);

				//	Install the new item

				ItemList.SetCursorAtItem(BestItem);
				m_pShip->InstallItemAsDevice(ItemList, i);

				bWeaponsInstalled = true;
				}
			}
		}

	//	If we installed weapons, recalc invariants

	if (bWeaponsInstalled)
		CalcInvariants();
	}

void CBaseShipAI::UseItemsBehavior (void)

//	UseItemsBehavior
//
//	Use various items appropriately

	{
	if (m_pShip->IsDestinyTime(29))
		m_pShip->FireOnItemAIUpdate();

	if (m_fSuperconductingShields 
			&& m_pShip->IsDestinyTime(61)
			&& m_pShip->GetShieldLevel() < 40)
		{
		//	Look for superconducting coils

		CItemType *pType = g_pUniverse->FindItemType(g_SuperconductingCoilUNID);
		if (pType)
			{
			CItem Coils(pType, 1);
			CItemListManipulator ItemList(m_pShip->GetItemList());
			
			if (ItemList.SetCursorAtItem(Coils))
				{
				m_pShip->UseItem(Coils);
				m_pShip->OnComponentChanged(comCargo);
				}
			}
		}
	}

void CBaseShipAI::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Save the AI data to a stream
//
//	DWORD		Controller ObjID
//	DWORD		ship class UNID (needed to set AISettings)
//	DWORD		m_pShip (CSpaceObject ref)
//	DWORD		m_iManeuver
//	DWORD		m_iThrustDir
//	DWORD		m_iLastTurn
//	DWORD		m_iLastTurnCount
//	DWORD		m_Blacklist
//	DWORD		m_iManeuverCounter
//
//	DWORD		m_pNavPath (ID)
//	DWORD		m_iNavPathPos
//
//	DWORD		No of orders
//	DWORD		order: Order
//	DWORD		order: pTarget
//	DWORD		order: dwData
//
//	DWORD		flags
//
//	Subclasses...

	{
	int i;
	DWORD dwSave;

	dwSave = (DWORD)GetClass()->GetObjID();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	dwSave = m_pShip->GetClass()->GetUNID();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	m_pShip->GetSystem()->WriteObjRefToStream(m_pShip, pStream);
	pStream->Write((char *)&m_iManeuver, sizeof(DWORD));
	pStream->Write((char *)&m_iThrustDir, sizeof(DWORD));
	pStream->Write((char *)&m_iLastTurn, sizeof(DWORD));
	pStream->Write((char *)&m_iLastTurnCount, sizeof(DWORD));
	m_Blacklist.WriteToStream(pStream);
	pStream->Write((char *)&m_iManeuverCounter, sizeof(DWORD));

	//	Nav path

	dwSave = (m_pNavPath ? m_pNavPath->GetID() : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	pStream->Write((char *)&m_iNavPathPos, sizeof(DWORD));

	//	Orders

	dwSave = m_Orders.GetCount();
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	for (i = 0; i < m_Orders.GetCount(); i++)
		{
		OrderEntry *pOrder = (OrderEntry *)m_Orders.GetStruct(i);
		pStream->Write((char *)&pOrder->Order, sizeof(DWORD));
		m_pShip->GetSystem()->WriteObjRefToStream(pOrder->pTarget, pStream);
		pStream->Write((char *)&pOrder->dwData, sizeof(DWORD));
		}

	//	Flags

	dwSave = 0;
	dwSave |= (m_fDeviceActivate ?		0x00000001 : 0);
	dwSave |= (m_fDockingRequested ?	0x00000002 : 0);
	dwSave |= (m_fWaitForShieldsToRegen ? 0x00000004 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	//	Subclasses

	OnWriteToStream(pStream);
	}

