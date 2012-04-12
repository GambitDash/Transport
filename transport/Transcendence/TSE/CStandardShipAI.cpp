//	CStandardShipAI.cpp
//
//	CStandardShipAI class
//
//	Depending on the current order, the implementation follows 
//	a specific state machine. For example:
//
//	orderGuard
//
//		stateNone:
//			IF docked -> stateWaitingForThreat
//			ELSE -> stateReturningFromThreat
//
//		stateReturningFromThreat:
//			IF enemies appear -> stateAttackingThreat
//			IF docked -> stateNone
//
//		stateWaitingForThreat:
//			IF enemies appear -> stateAttackingThreat
//
//		stateAttackingThreat:
//			IF enemy destroyed -> stateNone
//			IF too far -> stateReturningFromThreat
//
//	ORDERS AND STATES
//
//		orderNone
//			stateNone
//
//		orderGuard
//			stateWaitingForThreat
//			stateReturningFromThreat		oT
//			stateAttackingThreat			oT
//
//		orderDock
//			stateOnCourseForDocking
//
//		orderDestroyTarget
//			stateAttackingTarget
//
//		orderWait
//			stateWaiting
//
//		orderGate
//			stateOnCourseForStargate
//
//		orderGateOnThreat
//			stateWaitingUnarmed
//
//		orderGateOnStationDestroyed
//			stateNone
//
//		orderPatrol
//			stateOnPatrolOrbit				oT
//			stateAttackingOnPatrol			oT
//
//		orderEscort
//			stateEscorting					oT
//			stateAttackingThreat			oT
//
//		orderScavenge
//			stateLookingForLoot
//			stateLooting
//			stateDeterTarget
//
//		orderFollowPlayerThroughGate
//			stateOnCourseForStargate
//
//		orderAttackNearestEnemy
//
//		orderTradeRoute
//			stateOnCourseForDocking
//			stateOnCourseForStargate

#include "PreComp.h"


#ifdef DEBUG
//#define DEBUG_SHIP
//#define DEBUG_FIRE_ON_OPPORTUNITY
#endif

#ifdef DEBUG_COMBAT
#define DEBUG_COMBAT_OUTPUT(x)			if (bDebug) g_pUniverse->DebugOutput("%d> %s", g_iDebugLine++, x)
#else
#define DEBUG_COMBAT_OUTPUT(x)
#endif

const int ATTACK_TIME_THRESHOLD =		150;

const Metric ATTACK_RANGE =				(20.0 * LIGHT_SECOND);
const Metric PATROL_SENSOR_RANGE =		(30.0 * LIGHT_SECOND);
const Metric SCAVENGE_SENSOR_RANGE =	(10.0 * LIGHT_MINUTE);
const Metric THREAT_SENSOR_RANGE =		(10.0 * LIGHT_SECOND);
const Metric THREAT_SENSOR_RANGE2 =		(THREAT_SENSOR_RANGE * THREAT_SENSOR_RANGE);
const Metric WANDER_SAFETY_RANGE =		(20.0 * LIGHT_SECOND);
const Metric WANDER_SAFETY_RANGE2 =		(WANDER_SAFETY_RANGE * WANDER_SAFETY_RANGE);
const Metric NAV_PATH_THRESHOLD =		(2.0 * PATROL_SENSOR_RANGE);
const Metric NAV_PATH_THRESHOLD2 =		(NAV_PATH_THRESHOLD * NAV_PATH_THRESHOLD);
const Metric GOTO_THRESHOLD =			(LIGHT_SECOND / 10.0);
const Metric GOTO_THRESHOLD2 =			(GOTO_THRESHOLD * GOTO_THRESHOLD);
const Metric GOTO_SLOW_RANGE =			(5 * LIGHT_SECOND);
const Metric GOTO_SLOW_RANGE2 =			(GOTO_SLOW_RANGE * GOTO_SLOW_RANGE);
const Metric SAFE_ORBIT_RANGE =			(30.0 * LIGHT_SECOND);

#define SPACE_OBJ_SCAVENGE_DATA			CONSTLIT("$scavenge")
#define STR_G_SOURCE					CONSTLIT("gSource")

static CObjectClass<CStandardShipAI>g_Class(OBJID_CSTANDARDSHIPAI, NULL);

static CShip *g_pDebugShip = NULL;
int g_iDebugLine = 0;

bool EnemyStationsAtBearing (CSpaceObject *pShip, int iBearing, Metric rRange);

CStandardShipAI::CStandardShipAI (void) : CBaseShipAI(&g_Class),
		m_State(stateNone),
		m_pTarget(NULL),
		m_pDest(NULL),
		m_iLastAttack(0),
		m_bHasEscorts(false),
		m_pCommandCode(NULL)

//	CStandardShipAI constructor

	{
	}

CStandardShipAI::~CStandardShipAI (void)

//	CStandardShipAI destructor

	{
	if (m_pCommandCode)
		m_pCommandCode->Discard(&(g_pUniverse->GetCC()));
	}

void CStandardShipAI::Behavior (void)

//	Behavior
//
//	Fly, fight, die

	{
#ifdef DEBUG
	bool bDebug = m_pShip->IsSelected();
#endif

	//	Reset

	ResetBehavior();

	//	Use basic items

	UseItemsBehavior();

	//	Behave according to our state

	switch (m_State)
		{
		case stateNone:
			BehaviorStart();
			break;

		case stateApproaching:
			{
			ASSERT(m_pDest);
			CVector vTarget = m_pDest->GetPos() - m_pShip->GetPos();
			Metric rTargetDist2 = vTarget.Dot(vTarget);

			ImplementCloseOnTarget(m_pDest, vTarget, rTargetDist2);
			ImplementAttackNearestTarget(m_rBestWeaponRange, &m_pTarget);
			ImplementFireOnTargetsOfOpportunity();

			if (vTarget.Length() < m_rDistance)
				CancelCurrentOrder();

			break;
			}

		case stateAttackingOnPatrol:
			{
			ASSERT(m_pTarget);
			ImplementAttackTarget(m_pTarget);
			ImplementFireOnTargetsOfOpportunity(m_pTarget);

			//	Check to see if we've wandered outside our patrol zone

			if (m_pShip->IsDestinyTime(20))
				{
				CSpaceObject *pCenter = GetCurrentOrderTarget();
				int iDistance = GetCurrentOrderData();
				Metric rMaxRange2 = (LIGHT_SECOND * iDistance) + PATROL_SENSOR_RANGE;
				rMaxRange2 = rMaxRange2 * rMaxRange2;
				Metric rMinRange2 = std::max(0.0, (LIGHT_SECOND * iDistance) - PATROL_SENSOR_RANGE);
				rMinRange2 = rMinRange2 * rMinRange2;

				int iTick = m_pShip->GetSystem()->GetTick();
				CVector vRange = pCenter->GetPos() - m_pShip->GetPos();
				Metric rDistance2 = vRange.Dot(vRange);

				//	If we're outside of our patrol range and if we haven't
				//	been hit in a while then stop the attack

				if ((rDistance2 > rMaxRange2 || rDistance2 < rMinRange2)
						&& (iTick - m_iLastAttack) > ATTACK_TIME_THRESHOLD)
					{
					SetState(stateNone);
					DEBUG_COMBAT_OUTPUT("Patrol: End attack");
					}
				}

			break;
			}

		case stateAimingAtTarget:
			{
			ASSERT(m_pTarget);
			ImplementAttackTarget(m_pTarget, false, true);
			break;
			}

		case stateAttackingTarget:
			{
			ASSERT(m_pTarget);
			ImplementAttackTarget(m_pTarget);
			ImplementFireOnTargetsOfOpportunity(m_pTarget);
			break;
			}

		case stateAttackingTargetAndAvoiding:
			{
			ASSERT(m_pTarget);
			ImplementAttackTarget(m_pTarget);
			ImplementFireOnTargetsOfOpportunity(m_pTarget);

			//	Every once in a while check to see if we've wandered near
			//	an enemy station

			if (m_pShip->IsDestinyTime(41) && !IsImmobile() && m_pTarget->CanMove())
				{
				CSpaceObject *pEnemy = m_pShip->GetNearestEnemyStation(WANDER_SAFETY_RANGE);
				if (pEnemy 
						&& pEnemy != m_pTarget
						&& m_pTarget->GetDistance2(pEnemy) < WANDER_SAFETY_RANGE2)
					{
					SetState(stateAvoidingEnemyStation);
					m_pDest = pEnemy;
					m_pTarget = GetCurrentOrderTarget();
					}
				}

			break;
			}

		case stateAvoidingEnemyStation:
			{
			ASSERT(m_pDest);
			ASSERT(m_pTarget);

			int iTick = m_pShip->GetSystem()->GetTick();
			CVector vTarget = m_pTarget->GetPos() - m_pShip->GetPos();
			Metric rTargetDist2 = vTarget.Length2();
			CVector vDest = m_pDest->GetPos() - m_pShip->GetPos();

			//	We only spiral in/out part of the time (we leave ourselves some time to fight)

			bool bAvoid = (rTargetDist2 > THREAT_SENSOR_RANGE2) || ((iTick + m_pShip->GetDestiny()) % 91) > 55;
			
			if (!bAvoid)
				{
				//	Attack target

				ImplementAttackTarget(m_pTarget, true);
				ImplementFireOnTargetsOfOpportunity(m_pTarget);
				}
			else
				{
				//	Orbit around the enemy station

				Metric rDestDist2 = vDest.Length2();

				const Metric rMaxDist = SAFE_ORBIT_RANGE * 1.2;
				const Metric rMinDist = SAFE_ORBIT_RANGE * 0.9;

				if (rDestDist2 > (rMaxDist * rMaxDist))
					ImplementSpiralIn(vDest);
				else if (rDestDist2 < (rMinDist * rMinDist))
					ImplementSpiralOut(vDest);
				else
					{
					ImplementAttackTarget(m_pTarget, true);
					ImplementFireOnTargetsOfOpportunity(m_pTarget);
					}
				}

			//	If the target has left the safety of the station, then pursue again

			if (m_pShip->IsDestinyTime(23))
				{

				if (rTargetDist2 > WANDER_SAFETY_RANGE2)
					{
					//	If the station is still between the ship and the target,
					//	then we keep orbiting

					if (Absolute(AngleBearing(VectorToPolar(vTarget), VectorToPolar(vDest))) > 45)
						{
						SetState(stateAttackingTargetAndAvoiding);
						m_pTarget = GetCurrentOrderTarget();
						}
					}
				}

			break;
			}

		case stateAttackingThreat:
			{
			ASSERT(m_pTarget);
			ImplementAttackTarget(m_pTarget);
			ImplementFireOnTargetsOfOpportunity(m_pTarget);

			//	Every once in a while check to see if we've wandered too far from
			//	our base.

			if (m_pShip->IsDestinyTime(20))
				{
				CSpaceObject *pPrincipal = GetCurrentOrderTarget();
				int iTick = m_pShip->GetSystem()->GetTick();
				CVector vRange = pPrincipal->GetPos() - m_pShip->GetPos();
				Metric rDistance2 = vRange.Dot(vRange);

				//	If we're outside of our patrol range and if we haven't
				//	been hit in a while then stop the attack.

				if (rDistance2 > (PATROL_SENSOR_RANGE * PATROL_SENSOR_RANGE)
						&& (iTick - m_iLastAttack) > ATTACK_TIME_THRESHOLD)
					SetState(stateNone);
				}

			break;
			}

		case stateBombarding:
			{
			bool bInPlace;
			ImplementHold(&bInPlace);
			if (bInPlace)
				ImplementAttackTarget(m_pTarget);

			ImplementFireOnTargetsOfOpportunity(m_pTarget);

			//	See if we're done bombarding

			if (m_iCountdown != -1 && m_iCountdown-- == 0)
				{
				if (GetCurrentOrder() == IShipController::orderBombard)
					CancelCurrentOrder();

				SetState(stateNone);
				}
			break;
			}

		case stateDeterTarget:
			{
			ASSERT(m_pTarget);
			ImplementAttackTarget(m_pTarget);
			ImplementFireOnTargetsOfOpportunity(m_pTarget);

			//	Check to see if target has hit back. If not, stop the attack

			if (m_pShip->IsDestinyTime(20)
					&& (m_pShip->GetSystem()->GetTick() - m_iLastAttack) > 6 * ATTACK_TIME_THRESHOLD)
				SetState(stateNone);

			break;
			}

		case stateDeterTargetNoChase:
			{
			ASSERT(m_pTarget);
			ImplementAttackTarget(m_pTarget, true);
			ImplementFireOnTargetsOfOpportunity(m_pTarget);

			//	Check to see if target has hit back. If not, stop the attack

			if (m_pShip->IsDestinyTime(20)
					&& (m_pShip->GetSystem()->GetTick() - m_iLastAttack) > 3 * ATTACK_TIME_THRESHOLD)
				SetState(stateNone);

			break;
			}

		case stateDeterTargetWhileLootDocking:
			{
			ASSERT(m_pTarget);
			ImplementDocking(m_pDest);
			ImplementAttackTarget(m_pTarget, true);
			ImplementFireOnTargetsOfOpportunity(m_pTarget);

			//	Check to see if target has hit back. If not, stop the attack

			if (m_pShip->IsDestinyTime(20)
					&& (m_pShip->GetSystem()->GetTick() - m_iLastAttack) > 3 * ATTACK_TIME_THRESHOLD)
				{
				CSpaceObject *pDest = m_pDest;
				SetState(stateOnCourseForLootDocking);
				m_pDest = pDest;
				}

			break;
			}

		case stateEscorting:
			{
			ImplementEscort(m_pDest, &m_pTarget);

			//	Check to see if there are enemy ships that we need to attack

			if (m_pShip->IsDestinyTime(30))
				{
				CSpaceObject *pPrincipal = GetCurrentOrderTarget();
				CSpaceObject *pTarget = CalcEnemyShipInRange(pPrincipal, PATROL_SENSOR_RANGE);
				if (pTarget)
					{
					SetState(stateAttackingThreat);
					m_pTarget = pTarget;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					}
				}

			break;
			}

		case stateFollowing:
			{
			ImplementEscort(m_pDest, &m_pTarget);
			break;
			}

		case stateFollowNavPath:
			{
			ASSERT(m_iNavPathPos != -1);
			ImplementAttackNearestTarget(m_rBestWeaponRange, &m_pTarget);
			ImplementFireOnTargetsOfOpportunity();

			bool bAtDest;
			ImplementFollowNavPath(&bAtDest);
			if (bAtDest)
				{
				ClearNavPath();
				CancelCurrentOrder();
				}

			break;
			}

		case stateHolding:
			{
			ImplementHold();
			ImplementAttackNearestTarget(ATTACK_RANGE, &m_pTarget);
			ImplementFireOnTargetsOfOpportunity();

			//	Check to see if there are enemy ships that we need to attack

			if (m_pShip->IsDestinyTime(30))
				{
				CSpaceObject *pTarget = CalcEnemyShipInRange(m_pShip, PATROL_SENSOR_RANGE);
				if (pTarget)
					{
					SetState(stateDeterTargetNoChase);
					m_pTarget = pTarget;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					}
				}

			//	See if we're done holding

			if (m_iCountdown != -1 && m_iCountdown-- == 0)
				{
				if (GetCurrentOrder() == IShipController::orderHold)
					CancelCurrentOrder();

				SetState(stateNone);
				}
			break;
			}

		case stateLookingForLoot:
			{
			int i;

			//	Orbit around the center of the system

			CVector vTarget = -m_pShip->GetPos();
			Metric rTargetDist2 = vTarget.Dot(vTarget);

			Metric rMaxDist = m_rDistance * 1.1;
			Metric rMinDist = m_rDistance * 0.9;

			if (rTargetDist2 > (rMaxDist * rMaxDist))
				ImplementSpiralIn(vTarget);
			else if (rTargetDist2 < (rMinDist * rMinDist))
				ImplementSpiralOut(vTarget);

			ImplementAttackNearestTarget(ATTACK_RANGE, &m_pTarget);
			ImplementFireOnTargetsOfOpportunity();

			//	Look for loot

			if (m_pShip->IsDestinyTime(40))
				{
				Metric rBestDist2 = SCAVENGE_SENSOR_RANGE * SCAVENGE_SENSOR_RANGE;
				CSpaceObject *pBestScrap = NULL;
				for (i = 0; i < m_pShip->GetSystem()->GetObjectCount(); i++)
					{
					CSpaceObject *pObj = m_pShip->GetSystem()->GetObject(i);

					if (pObj 
							&& pObj->SupportsDocking()
							&& pObj->IsAbandoned()
							&& !pObj->IsRadioactive()
							&& pObj->GetData(SPACE_OBJ_SCAVENGE_DATA).IsBlank())
						{
						CVector vRange = pObj->GetPos() - m_pShip->GetPos();
						Metric rDistance2 = vRange.Dot(vRange);

						if (rDistance2 < rBestDist2)
							{
							rBestDist2 = rDistance2;
							pBestScrap = pObj;
							}
						}
					}

				//	If we've found some scrap then dock with it

				if (pBestScrap)
					{
					SetState(stateOnCourseForLootDocking);
					m_pDest = pBestScrap;
					}

				//	Otherwise, there is a small chance that we change orbits

				else if (mathRandom(1, 100) == 1)
					{
					m_rDistance = LIGHT_SECOND * 4 * (mathRandom(30, 120) + mathRandom(30, 120));
					}
				}

			break;
			}

		case stateLooting:
		case stateLootingOnce:
			{
			CSpaceObject *pDock = m_pShip->GetDockedObj();
			if (pDock)
				{
				//	See if it is time to take the loot and leave

				if (m_pShip->IsDestinyTime(300))
					{
					//	Take the loot

					bool bGotLoot = false;
					if (pDock->IsAbandoned())
						{
						CItemListManipulator Source(pDock->GetItemList());

						Source.MoveCursorForward();
						while (Source.IsCursorValid())
							{
							Source.TransferAtCursor(Source.GetItemAtCursor().GetCount(),
									m_pShip->GetItemList());
							}

						//	Tell the object that we've modified items in case
						//	the player ship is docked with the same station.

						pDock->ItemsModified();
						pDock->InvalidateItemListAddRemove();
						m_pShip->InvalidateItemListAddRemove();
						bGotLoot = true;
						}

					//	Mark this object as something that we've already looted

					if (m_State == stateLooting)
						pDock->SetData(SPACE_OBJ_SCAVENGE_DATA, CONSTLIT("l"));

					//	Undock

					m_pShip->Undock();

					//	If we got some loot, see if we can upgrade devices

					if (bGotLoot)
						{
						UpgradeWeaponBehavior();
						UpgradeShieldBehavior();
						}

					//	Continue looking for loot

					if (m_State == stateLootingOnce)
						{
						CancelCurrentOrder();
						}
					else
						{
						SetState(stateLookingForLoot);
						m_rDistance = LIGHT_SECOND * 4 * (mathRandom(30, 120) + mathRandom(30, 120));
						}
					}
				}
			else
				SetState(stateNone);

			break;
			}

		case stateMaintainBearing:
			{
			ImplementManeuver((int)m_rDistance, true);
			ImplementAttackNearestTarget(ATTACK_RANGE, &m_pTarget);
			ImplementFireOnTargetsOfOpportunity();

			//	Every so often, check to see if there are enemy stations ahead

			if (m_pShip->IsDestinyTime(10))
				{
				if (--m_iCountdown <= 0
						|| EnemyStationsAtBearing(m_pShip, (int)m_rDistance, WANDER_SAFETY_RANGE))
					SetState(stateNone);
				}

			break;
			}

		case stateOnCourseForDocking:
			{
			ASSERT(m_pDest);
			ImplementDocking(m_pDest);
			ImplementAttackNearestTarget(m_rBestWeaponRange, &m_pTarget, m_pDest);
			ImplementFireOnTargetsOfOpportunity(NULL, m_pDest);
			break;
			}

		case stateOnCourseForDockingViaNavPath:
			{
			ASSERT(m_iNavPathPos != -1);
			ImplementAttackNearestTarget(m_rBestWeaponRange, &m_pTarget);
			ImplementFireOnTargetsOfOpportunity();

			bool bAtDest;
			ImplementFollowNavPath(&bAtDest);
			if (bAtDest)
				{
				ClearNavPath();
				SetState(stateOnCourseForDocking);
				m_pDest = GetCurrentOrderTarget();
				}

			break;
			}

		case stateOnCourseForLootDocking:
			{
			ASSERT(m_pDest);
			ImplementDocking(m_pDest);
			ImplementAttackNearestTarget(m_rBestWeaponRange, &m_pTarget);
			ImplementFireOnTargetsOfOpportunity();
			break;
			}

		case stateOnCourseForPatrolViaNavPath:
			{
			ASSERT(m_iNavPathPos != -1);
			ImplementAttackNearestTarget(m_rBestWeaponRange, &m_pTarget);
			ImplementFireOnTargetsOfOpportunity();

			bool bAtDest;
			ImplementFollowNavPath(&bAtDest);
			if (bAtDest)
				{
				ClearNavPath();
				SetState(stateOnPatrolOrbit);
				m_pDest = GetCurrentOrderTarget();
				m_rDistance = LIGHT_SECOND * GetCurrentOrderData();
				}

			break;
			}

		case stateOnCourseForPoint:
			{
			ImplementAttackNearestTarget(m_rBestWeaponRange, &m_pTarget);
			ImplementFireOnTargetsOfOpportunity();

			bool bInPosition = false;
			ImplementFormationManeuver(m_pDest->GetPos(), m_pDest->GetVel(), m_iCountdown, &bInPosition);
			if (bInPosition)
				CancelCurrentOrder();

			break;
			}

		case stateOnCourseForPointViaNavPath:
			{
			ASSERT(m_iNavPathPos != -1);
			ImplementAttackNearestTarget(m_rBestWeaponRange, &m_pTarget);
			ImplementFireOnTargetsOfOpportunity();

			bool bAtDest;
			ImplementFollowNavPath(&bAtDest);
			if (bAtDest)
				{
				ClearNavPath();

				if (GetCurrentOrder() == IShipController::orderApproach)
					{
					SetState(stateApproaching);
					m_pDest = GetCurrentOrderTarget();
					m_rDistance = LIGHT_SECOND * GetCurrentOrderData();
					if (m_rDistance < LIGHT_SECOND)
						m_rDistance = LIGHT_SECOND;
					}
				else
					{
					SetState(stateOnCourseForPoint);
					m_pDest = GetCurrentOrderTarget();
					m_iCountdown = m_pShip->GetRotation();
					}
				}

			break;
			}

		case stateOnCourseForStargate:
			{
			ImplementGating(m_pDest);
			ImplementAttackNearestTarget(m_rBestWeaponRange, &m_pTarget);
			ImplementFireOnTargetsOfOpportunity();
			break;
			}

		case stateOnPatrolOrbit:
			{
			ASSERT(m_pDest);
			CVector vTarget = m_pDest->GetPos() - m_pShip->GetPos();
			Metric rTargetDist2 = vTarget.Dot(vTarget);

			Metric rMaxDist = m_rDistance * 1.1;
			Metric rMinDist = m_rDistance * 0.9;

			if (rTargetDist2 > (rMaxDist * rMaxDist))
				ImplementSpiralIn(vTarget);
			else if (rTargetDist2 < (rMinDist * rMinDist))
				ImplementSpiralOut(vTarget);

			ImplementAttackNearestTarget(m_rMaxWeaponRange, &m_pTarget, m_pDest);
			ImplementFireOnTargetsOfOpportunity(NULL, m_pDest);

			//	Check to see if any enemy ships appear

			if (m_pShip->IsDestinyTime(30))
				{
				CSpaceObject *pPrincipal = GetCurrentOrderTarget();
				CSpaceObject *pTarget = CalcEnemyShipInRange(pPrincipal, PATROL_SENSOR_RANGE, m_pDest);
				if (pTarget)
					{
					SetState(stateAttackingOnPatrol);
					m_pTarget = pTarget;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					DEBUG_COMBAT_OUTPUT("Patrol: Attack target");
					}
				}

			break;
			}

		case stateReturningFromThreat:
			{
			ASSERT(m_pDest);
			ImplementDocking(m_pDest);
			ImplementAttackNearestTarget(m_rMaxWeaponRange, &m_pTarget);
			ImplementFireOnTargetsOfOpportunity();

			//	Every once in a while check to see if any enemy ships show up

			if (m_pShip->IsDestinyTime(30))
				{
				CSpaceObject *pPrincipal = GetCurrentOrderTarget();
				CSpaceObject *pTarget = CalcEnemyShipInRange(pPrincipal, PATROL_SENSOR_RANGE);
				if (pTarget)
					{
					SetState(stateAttackingThreat);
					m_pTarget = pTarget;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					}
				}
			break;
			}

		case stateReturningViaNavPath:
			{
			ASSERT(m_iNavPathPos != -1);
			ImplementAttackNearestTarget(m_rBestWeaponRange, &m_pTarget);
			ImplementFireOnTargetsOfOpportunity();

			bool bAtDest;
			ImplementFollowNavPath(&bAtDest);
			if (bAtDest)
				{
				ClearNavPath();
				SetState(stateReturningFromThreat);
				m_pDest = GetCurrentOrderTarget();
				}

			break;
			}

		case stateWaitForPlayerAtGate:
			ImplementEscort(m_pDest, &m_pTarget);
			break;

		case stateWaiting:
			{
			if (m_iCountdown != -1 && m_iCountdown-- == 0)
				{
				if (GetCurrentOrder() == IShipController::orderWait)
					CancelCurrentOrder();

				SetState(stateNone);
				}
			break;
			}

		case stateWaitingForEnemy:
			{
			ImplementHold();

			//	Check to see if the target that we're looking for is in sight

			if (m_pShip->IsDestinyTime(17)
					&& m_pShip->GetNearestVisibleEnemy())
				CancelCurrentOrder();

			//	See if we're done waiting

			if (m_iCountdown != -1 && m_iCountdown-- == 0)
				CancelCurrentOrder();

			break;
			}

		case stateWaitingForTarget:
			{
			ImplementHold();
			ImplementAttackNearestTarget(ATTACK_RANGE, &m_pTarget);
			ImplementFireOnTargetsOfOpportunity();

			//	Check to see if the target that we're looking for is in sight

			if (m_pShip->IsDestinyTime(17))
				{
				Metric rDetectRange = m_pDest->GetDetectionRange(m_pShip->GetPerception());
				Metric rDetectRange2 = rDetectRange * rDetectRange;

				Metric rRange2 = (m_pDest->GetPos() - m_pShip->GetPos()).Length2();
				if (rRange2 < rDetectRange2)
					CancelCurrentOrder();
				}

			//	See if we're done waiting

			if (m_iCountdown != -1 && m_iCountdown-- == 0)
				CancelCurrentOrder();

			break;
			}

		case stateWaitingForThreat:
			{
			if (m_pShip->IsDestinyTime(30))
				{
				ASSERT(m_pDest);
				CSpaceObject *pTarget = CalcEnemyShipInRange(m_pDest, PATROL_SENSOR_RANGE);

				//	If there are enemy ships in range, then attack them.

				if (pTarget)
					{
					SetState(stateAttackingThreat);
					m_pTarget = pTarget;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					}
				}

			break;
			}

		case stateWaitingUnarmed:
			{
			//	Every once in a while check to see if there are enemy ships
			//	near-by. If so, then gate out.

			if (m_pShip->IsDestinyTime(30))
				{
				if (m_pShip->IsEnemyInRange(THREAT_SENSOR_RANGE))
					{
					CancelCurrentOrder();
					AddOrder(IShipController::orderGate, NULL, 0);
					}
				}

			break;
			}

		default:
			ASSERT(false);
		}
	}

void CStandardShipAI::BehaviorStart (void)

//	BehaviorStart
//
//	Figure out what we need to do to achieve our current orders

	{
	switch (GetCurrentOrder())
		{
		case IShipController::orderNone:
			{
			//	If we've got command code, execute it now

			if (m_pCommandCode)
				{
				CCodeChain &CC = g_pUniverse->GetCC();

				ICCItem *pOldSource = CC.LookupGlobal(STR_G_SOURCE, &g_pUniverse);
				DefineGlobalSpaceObject(CC, STR_G_SOURCE, m_pShip);

				ICCItem *pResult = CC.TopLevel(m_pCommandCode, &g_pUniverse);
				if (pResult->IsError())
					m_pShip->ReportEventError(CONSTLIT("CommandCode"), pResult);
				pResult->Discard(&CC);

				CC.DefineGlobal(STR_G_SOURCE, pOldSource);
				pOldSource->Discard(&CC);
				}

			//	If we still got no orders, let the ship class deal with this

			if (GetCurrentOrder() == IShipController::orderNone)
				FireOnOrdersCompleted();

			//	If we've still got no orders, we need to come up with something

			if (GetCurrentOrder() == IShipController::orderNone)
				{
				if (m_pShip->GetDockedObj() == NULL)
					AddOrder(IShipController::orderGate, NULL, 0);
				else
					AddOrder(IShipController::orderWait, NULL, 0);
				}
			break;
			}

		case IShipController::orderAimAtTarget:
			{
			SetState(stateAimingAtTarget);
			m_pTarget = GetCurrentOrderTarget();
			ASSERT(m_pTarget);
			ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
			break;
			}

		case IShipController::orderAttackNearestEnemy:
			{
			//	Look for the nearest target (if we can't find one, then
			//	just gate out).

			CSpaceObject *pTarget = m_pShip->GetNearestEnemy();
			if (pTarget)
				{
				SetState(stateAttackingTarget);
				m_pTarget = pTarget;
				ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
				}
			else
				{
				CancelCurrentOrder();
				}

			break;
			}

		case IShipController::orderDestroyPlayerOnReturn:
			{
			CSpaceObject *pGate = m_pShip->GetNearestStargate();
			if (pGate)
				{
				SetState(stateWaitForPlayerAtGate);
				m_pDest = pGate;
				}
			
			break;
			}

		case IShipController::orderDestroyTarget:
			{
			SetState(stateAttackingTargetAndAvoiding);
			m_pTarget = GetCurrentOrderTarget();
			ASSERT(m_pTarget);
			ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
			break;
			}

		case IShipController::orderDock:
			{
			CSpaceObject *pDest = GetCurrentOrderTarget();
			ASSERT(pDest);

			//	If we're docked with our destination then we're done.

			if (m_pShip->GetDockedObj() == pDest)
				{
				CancelCurrentOrder();
				}

			//	Otherwise, see if we should take a nav path

			else if (GetDistance2(pDest) > NAV_PATH_THRESHOLD2
					&& CalcNavPath(pDest))
				{
				SetState(stateOnCourseForDockingViaNavPath);
				}

			//	Otherwise, try to dock

			else
				{
				SetState(stateOnCourseForDocking);
				m_pDest = pDest;
				}

			break;
			}

		case IShipController::orderEscort:
			{
			SetState(stateEscorting);
			m_pDest = GetCurrentOrderTarget();
			ASSERT(m_pDest);

			m_pShip->Communicate(m_pDest, msgEscortReportingIn, m_pShip);
			break;
			}

		case IShipController::orderFollow:
			{
			SetState(stateFollowing);
			m_pDest = GetCurrentOrderTarget();
			ASSERT(m_pDest);

			m_pShip->Communicate(m_pDest, msgEscortReportingIn, m_pShip);
			break;
			}

		case IShipController::orderFollowPlayerThroughGate:
			{
			SetState(stateOnCourseForStargate);
			m_pDest = m_pShip->GetNearestStargate();
			break;
			}

		case IShipController::orderGate:
			{
			//	Look for the gate

			CSpaceObject *pGate = GetCurrentOrderTarget();
			if (pGate == NULL)
				pGate = m_pShip->GetNearestStargate(true);

			//	Head for the gate

			if (pGate)
				{
				SetState(stateOnCourseForStargate);
				m_pDest = pGate;
				}

			break;
			}

		case IShipController::orderGoTo:
			{
			CSpaceObject *pDest = GetCurrentOrderTarget();
			ASSERT(pDest);

			//	See if we should take a nav path

			if (GetDistance2(pDest) > NAV_PATH_THRESHOLD2
					&& CalcNavPath(pDest))
				{
				SetState(stateOnCourseForPointViaNavPath);
				}

			//	Otherwise, go there

			else
				{
				SetState(stateOnCourseForPoint);
				m_pDest = pDest;

				//	Use m_iCountdown to store the desired rotation

				CVector vBearing = m_pDest->GetPos() - m_pShip->GetPos();
				m_iCountdown = m_pShip->AlignToRotationAngle(VectorToPolar(vBearing));
				}

			break;
			}

		case IShipController::orderApproach:
			{
			CSpaceObject *pDest = GetCurrentOrderTarget();
			ASSERT(pDest);

			//	See if we should take a nav path

			if (GetDistance2(pDest) > NAV_PATH_THRESHOLD2
					&& CalcNavPath(pDest))
				{
				SetState(stateOnCourseForPointViaNavPath);
				}

			//	Otherwise, go there

			else
				{
				SetState(stateApproaching);
				m_pDest = GetCurrentOrderTarget();
				m_rDistance = LIGHT_SECOND * GetCurrentOrderData();
				if (m_rDistance < LIGHT_SECOND)
					m_rDistance = LIGHT_SECOND;
				}

			break;
			}

		case IShipController::orderGuard:
			{
			CSpaceObject *pPrincipal = GetCurrentOrderTarget();
			ASSERT(pPrincipal);

			//	If we're docked, wait for threat

			if (m_pShip->GetDockedObj())
				{
				SetState(stateWaitingForThreat);
				m_pDest = pPrincipal;
				}

			//	If we're very far from our principal and we can use a nav
			//	path, do it

			else if (GetDistance2(pPrincipal) > NAV_PATH_THRESHOLD2
					&& CalcNavPath(pPrincipal))
				{
				SetState(stateReturningViaNavPath);
				}

			//	Otherwise, return directly to base

			else
				{
				SetState(stateReturningFromThreat);
				m_pDest = pPrincipal;
				}

			break;
			}

		case IShipController::orderGateOnStationDestroyed:
			break;

		case IShipController::orderGateOnThreat:
			SetState(stateWaitingUnarmed);
			break;

		case IShipController::orderHold:
			SetState(stateHolding);
			if (GetCurrentOrderData())
				m_iCountdown = 1 + (g_TicksPerSecond * GetCurrentOrderData());
			else
				m_iCountdown = -1;
			break;

		case IShipController::orderBombard:
			SetState(stateBombarding);
			m_pTarget = GetCurrentOrderTarget();
			ASSERT(m_pTarget);
			ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());

			if (GetCurrentOrderData())
				m_iCountdown = 1 + (g_TicksPerSecond * GetCurrentOrderData());
			else
				m_iCountdown = -1;
			break;

		case IShipController::orderLoot:
			{
			CSpaceObject *pDest = GetCurrentOrderTarget();
			ASSERT(pDest);

			if (m_pShip->GetDockedObj() == pDest)
				SetState(stateLootingOnce);
			else
				{
				SetState(stateOnCourseForDocking);
				m_pDest = pDest;
				}
			break;
			}

		case IShipController::orderNavPath:
			{
			CNavigationPath *pPath = m_pShip->GetSystem()->GetNavPathByID(GetCurrentOrderData());
			if (pPath)
				{
				SetState(stateFollowNavPath);
				CalcNavPath(pPath);
				}
			else
				CancelCurrentOrder();

			break;
			}

		case IShipController::orderPatrol:
			{
			CSpaceObject *pBase = GetCurrentOrderTarget();
			Metric rDist = LIGHT_SECOND * GetCurrentOrderData();
			ASSERT(pBase);

			if (rDist < (NAV_PATH_THRESHOLD / 2.0)
					&& GetDistance2(pBase) > NAV_PATH_THRESHOLD2
					&& CalcNavPath(pBase))
				{
				SetState(stateOnCourseForPatrolViaNavPath);
				}
			else
				{
				SetState(stateOnPatrolOrbit);
				m_pDest = pBase;
				m_rDistance = rDist;
				}

			break;
			}

		case IShipController::orderScavenge:
			{
			if (m_pShip->GetDockedObj())
				SetState(stateLooting);
			else
				{
				SetState(stateLookingForLoot);
				m_rDistance = LIGHT_SECOND * 4 * (mathRandom(30, 120) + mathRandom(30, 120));
				}
			break;
			}

		case IShipController::orderTradeRoute:
			{
			int i;

			//	Look for a new station to go to.

			Metric rBestDist2 = 100.0 * LIGHT_SECOND * LIGHT_SECOND;
			CSpaceObject *pBestDest = NULL;
			for (i = 0; i < m_pShip->GetSystem()->GetObjectCount(); i++)
				{
				CSpaceObject *pObj = m_pShip->GetSystem()->GetObject(i);

				if (pObj
						&& pObj->SupportsDocking()
						&& !pObj->IsEnemy(m_pShip)
						&& !pObj->IsAbandoned())
					{
					CVector vRange = pObj->GetPos() - m_pShip->GetPos();
					Metric rDistance2 = vRange.Dot(vRange);

					if (rDistance2 > rBestDist2)
						{
						if (pBestDest == NULL || mathRandom(1, 2) == 1)
							{
							rBestDist2 = rDistance2;
							pBestDest = pObj;
							}
						}
					}
				}

			//	If we've found some place to go, then dock

			if (pBestDest)
				{
				AddOrder(IShipController::orderWait, NULL, mathRandom(4, 24), true);
				AddOrder(IShipController::orderDock, pBestDest, 0, true);
				}

			//	Otherwise, gate out of here

			else
				{
				CSpaceObject *pGate = m_pShip->GetNearestStargate(true);

				//	Head for the gate

				if (pGate)
					{
					SetState(stateOnCourseForStargate);
					m_pDest = pGate;
					}
				}

			break;
			}

		case IShipController::orderWait:
			{
			SetState(stateWaiting);
			if (GetCurrentOrderData())
				m_iCountdown = 1 + (g_TicksPerSecond * GetCurrentOrderData());
			else
				m_iCountdown = -1;
			break;
			}

		case IShipController::orderWaitForEnemy:
			{
			SetState(stateWaitingForEnemy);
			if (GetCurrentOrderData())
				m_iCountdown = 1 + (g_TicksPerSecond * GetCurrentOrderData());
			else
				m_iCountdown = -1;
			break;
			}

		case IShipController::orderWaitForTarget:
			{
			CSpaceObject *pTarget = GetCurrentOrderTarget();
			ASSERT(pTarget);

			SetState(stateWaitingForTarget);
			m_pDest = pTarget;
			if (GetCurrentOrderData())
				m_iCountdown = 1 + (g_TicksPerSecond * GetCurrentOrderData());
			else
				m_iCountdown = -1;
			break;
			}

		case IShipController::orderWander:
			{
			//	Figure out which bearing to take. We set m_rDistance with
			//	the direction that we want to move. We will continue moving
			//	in that direction until we are blocked by an enemy or until
			//	the countdown expires.
			//
			//	First we figure out where we are relative to the center
			//	of the system.

			Metric rSunRadius;
			int iSunBearing = VectorToPolar(m_pShip->GetPos(), &rSunRadius);

			//	Figure out where we want to be

			Metric rBestDist = LIGHT_SECOND * (100 + (900 * m_pShip->GetDestiny() / g_DestinyRange));
			int iSign = ((m_pShip->GetDestiny() % 2) == 1 ? 1 : -1);

			//	Move outwards if we're too close; inward if we're too far

			int iBearing;
			if (rSunRadius > rBestDist)
				iBearing = (iSunBearing + iSign * (90 + 30) + 360) % 360;
			else
				iBearing = (iSunBearing + iSign * (90 - 15) + 360) % 360;

			//	Adjust the bearing if it turns out that there are enemy
			//	stations in that direction.

			iBearing = m_pShip->AlignToRotationAngle(iBearing);
			int iBearingStart = iBearing;
			ASSERT((360 % m_pShip->GetRotationAngle()) == 0);
			while (true)
				{
				if (!EnemyStationsAtBearing(m_pShip, iBearing, WANDER_SAFETY_RANGE))
					break;

				//	Adjust bearing

				iBearing = (iBearing + m_pShip->GetRotationAngle()) % 360;
				if (iBearing == iBearingStart)
					break;
				}

			//	Set state

			SetState(stateMaintainBearing);
			m_rDistance = (Metric)iBearing;
			m_iCountdown = 150 + mathRandom(0, 50);
			break;
			}
		}
	}

void CStandardShipAI::CommunicateWithEscorts (MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2)

//	CommunicateWithEscorts
//
//	Sends a message to the ship's escorts

	{
	if (m_bHasEscorts)
		{
		bool bEscortsFound = false;

		CSovereign *pSovereign = m_pShip->GetSovereign();
		for (int i = 0; i < m_pShip->GetSystem()->GetObjectCount(); i++)
			{
			CSpaceObject *pObj = m_pShip->GetSystem()->GetObject(i);

			if (pObj 
					&& pObj->GetCategory() == CSpaceObject::catShip
					&& pObj->GetSovereign() == pSovereign
					&& pObj != m_pShip
					&& pObj->GetEscortPrincipal() == m_pShip)
				{
				m_pShip->Communicate(pObj, iMessage, pParam1, dwParam2);
				bEscortsFound = true;
				}
			}

		if (!bEscortsFound)
			m_bHasEscorts = false;
		}
	}

CString CStandardShipAI::DebugCrashInfo (void)

//	DebugCrashInfo
//
//	Returns debug crash info

	{
	CString sResult;

	sResult.Append(CONSTLIT("CStandardShipAI\r\n"));
	sResult.Append(strPatternSubst(CONSTLIT("Order: %d\r\n"), (int)GetCurrentOrder()));
	sResult.Append(strPatternSubst(CONSTLIT("m_State: %d\r\n"), m_State));
	sResult.Append(strPatternSubst(CONSTLIT("m_pDest: %s\r\n"), CSpaceObject::DebugDescribe(m_pDest).GetASCIIZPointer()));
	sResult.Append(strPatternSubst(CONSTLIT("m_pTarget: %s\r\n"), CSpaceObject::DebugDescribe(m_pTarget).GetASCIIZPointer()));
	sResult.Append(strPatternSubst(CONSTLIT("m_pNavPath: %s\r\n"), CNavigationPath::DebugDescribe(m_pShip, m_pNavPath).GetASCIIZPointer()));

	return sResult;
	}

CSpaceObject *CStandardShipAI::GetBase (void) const

//	GetBase
//
//	Returns this ship's base

	{
	switch (GetCurrentOrder())
		{
		case IShipController::orderDock:
		case IShipController::orderGuard:
		case IShipController::orderPatrol:
			return GetCurrentOrderTarget();

		default:
			return NULL;
		}
	}

CSpaceObject *CStandardShipAI::GetTarget (void)

//	GetTarget
//
//	Returns the target that this ship is attacking
	
	{
	switch (m_State)
		{
		case stateNone:
		case stateLooting:
		case stateLootingOnce:
		case stateWaitingForEnemy:
		case stateWaitingForThreat:
		case stateWaitingUnarmed:
			return NULL;

		default:
			return m_pTarget;
		}
	}

CSpaceObject *CStandardShipAI::GetWeaponTarget (int iDev)

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
		return m_pTarget;
	}

void CStandardShipAI::OnAttacked (CSpaceObject *pAttacker, const DamageDesc &Damage)

//	OnAttacked
//
//	Our ship was attacked. Note that pAttacker can either be an enemy ship/station
//	or it can be the missile/beam that hit us.

	{
	CSpaceObject *pOrderGiver = (pAttacker ? pAttacker->GetOrderGiver(Damage.GetCause()) : NULL);

	if (pAttacker)
		{
		//	If we were attacked by a friend, then warn them off
		//	(Unless we're explicitly targeting the friend)

		if (m_pShip->IsFriend(pAttacker) && pAttacker != m_pTarget && pOrderGiver != m_pTarget)
			{
			//	We deal with the order giver instead of the attacker because we want to get
			//	at the root problem (the player instead of her autons)
			//
			//	Also, we ignore damage from automated weapons

			if (!Damage.IsAutomatedWeapon())
				HandleFriendlyFire(pOrderGiver);
			}

		//	Else if we were attacked by an enemy/neutral, see if we need
		//	to attack them (or flee). In this case, we take care of the immediate
		//	problem (attackers) instead of the order giver.

		else if (pAttacker->CanAttack())
			{
			//	Tell our escorts that we were attacked

			CommunicateWithEscorts(msgAttack, pAttacker);

			//	Tell others that we were attacked

			switch (m_State)
				{
				case stateEscorting:
					{
					m_pShip->Communicate(m_pDest, msgEscortAttacked, pAttacker);
					break;
					}
				}

			//	Change state to deal with the attack

			switch (m_State)
				{
				case stateEscorting:
				case stateReturningFromThreat:
				case stateWaitingForThreat:
					{
					SetState(stateAttackingThreat);
					m_pTarget = pAttacker;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					break;
					}

				case stateLookingForLoot:
				case stateMaintainBearing:
					SetState(stateDeterTarget);
					m_pTarget = pAttacker;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					break;

				case stateOnPatrolOrbit:
					SetState(stateAttackingOnPatrol);
					m_pTarget = pAttacker;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					break;

				case stateWaitingUnarmed:
					CancelCurrentOrder();
					AddOrder(IShipController::orderGate, NULL, 0);
					break;

				case stateOnCourseForLootDocking:
					{
					CSpaceObject *pDest = m_pDest;
					SetState(stateDeterTargetWhileLootDocking);
					m_pDest = pDest;
					m_pTarget = pAttacker;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					break;
					}

				case stateHolding:
					{
					if (m_pTarget == NULL && mathRandom(1, 3) == 1)
						{
						SetState(stateDeterTargetNoChase);
						m_pTarget = pAttacker;
						ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
						}
					break;
					}
				}
			}
		}

	//	Remember the last time we were attacked

	m_iLastAttack = m_pShip->GetSystem()->GetTick();
	}

DWORD CStandardShipAI::OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2)

//	Communicate
//
//	Handle communications from other objects

	{
	switch (iMessage)
		{
		case msgAttack:
			{
			switch (m_State)
				{
				case stateEscorting:
				case stateReturningFromThreat:
				case stateWaitingForThreat:
					SetState(stateAttackingThreat);
					m_pTarget = pParam1;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					return resAck;

				case stateLookingForLoot:
					SetState(stateDeterTarget);
					m_pTarget = pParam1;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					return resAck;

				case stateOnPatrolOrbit:
					SetState(stateAttackingOnPatrol);
					m_pTarget = pParam1;
					ASSERT(m_pTarget->DebugIsValid() && m_pTarget->NotifyOthersWhenDestroyed());
					return resAck;

				default:
					return resNoAnswer;
				}
			}

		case msgDestroyBroadcast:
			{
			switch (m_State)
				{
				case stateEscorting:
				case stateReturningFromThreat:
				case stateWaitingForThreat:
				case stateOnPatrolOrbit:
					if (pParam1)
						{
						AddOrder(IShipController::orderDestroyTarget,
								pParam1,
								0,
								true);
						return resAck;
						}
					else
						return resNoAnswer;

				default:
					return resNoAnswer;
				}
			}

		case msgEscortAttacked:
			{
			//	Treat this as an attack on ourselves

			DamageDesc Dummy;
			OnAttacked(pParam1, Dummy);
			return resAck;
			}

		case msgEscortReportingIn:
			m_bHasEscorts = true;
			return resAck;

		case msgQueryEscortStatus:
			{
			if (GetEscortPrincipal() == pParam1)
				return resAck;
			else
				return resNoAnswer;
			}

		default:
			return resNoAnswer;
		}
	}

void CStandardShipAI::OnDockedEvent (CSpaceObject *pObj)

//	OnDockedEvent
//
//	Event when the ship is docked

	{
	SetState(stateNone);
	}

void CStandardShipAI::OnObjDestroyedNotify (const SDestroyCtx &Ctx)

//	OnObjDestroyedNotify
//
//	Handle the case where another object is destroyed

	{
	//	Alter our goals

	if (Ctx.pObj == m_pDest)
		{
		switch (m_State)
			{
			case stateOnPatrolOrbit:
			case stateEscorting:
			case stateOnCourseForDocking:
			case stateOnCourseForDockingViaNavPath:
			case stateOnCourseForLootDocking:
			case stateOnCourseForPatrolViaNavPath:
			case stateDeterTargetWhileLootDocking:
			case stateReturningFromThreat:
			case stateReturningViaNavPath:
			case stateOnCourseForStargate:
			case stateWaitingForThreat:
			case stateFollowing:
			case stateWaitForPlayerAtGate:
			case stateOnCourseForPoint:
			case stateWaitingForTarget:
			case stateAvoidingEnemyStation:
			case stateApproaching:
				{
				//	If the object that we were heading for has been destroyed, then
				//	choose a new order.

				SetState(stateNone);
				break;
				}
			}

		m_pDest = NULL;
		}

	//	Alter our goals

	if (Ctx.pObj == m_pTarget)
		{
		switch (m_State)
			{
			case stateAimingAtTarget:
			case stateAttackingOnPatrol:
			case stateAttackingTarget:
			case stateAttackingTargetAndAvoiding:
			case stateAttackingThreat:
			case stateDeterTarget:
			case stateDeterTargetNoChase:
			case stateBombarding:
				{
				//	If a friend destroyed our target then thank them

				if (Ctx.pDestroyer && !m_pShip->IsEnemy(Ctx.pDestroyer))
					m_pShip->Communicate(Ctx.pDestroyer, msgNiceShooting);

				SetState(stateNone);
				break;
				}

			case stateDeterTargetWhileLootDocking:
				{
				CSpaceObject *pDest = m_pDest;
				SetState(stateOnCourseForLootDocking);
				m_pDest = pDest;
				break;
				}
			}

		m_pTarget = NULL;
		}

	//	Alter our orders

	if (GetOrderCount() > 0)
		{
		//	If our orders involve this target, then we've got to deal with
		//	that appropriately.

		if (Ctx.pObj == GetCurrentOrderTarget())
			{
			//	Deal with the consequences

			switch (GetCurrentOrder())
				{
				//	If we're looting then we don't remove the order. (If the
				//	object is really going away, the order will get deleted by
				//	our ancestor class).

				case IShipController::orderLoot:
					break;

				//	In these cases we avenge the target

				case IShipController::orderPatrol:
				case IShipController::orderGuard:
				case IShipController::orderEscort:
					{
					CancelCurrentOrder();

					if (Ctx.pDestroyer && Ctx.pDestroyer->CanAttack() && !m_pShip->IsFriend(Ctx.pDestroyer))
						AddOrder(IShipController::orderDestroyTarget, Ctx.pDestroyer, 0);
					else
						AddOrder(IShipController::orderAttackNearestEnemy, NULL, 0);

					break;
					}

				default:
					{
					CancelCurrentOrder();
					}
				}
			}
		}

	//	If we're docked with the object that got destroyed then react

	if (Ctx.pObj == m_pShip->GetDockedObj())
		{
		switch (GetCurrentOrder())
			{
			//	Gate out

			case IShipController::orderGateOnStationDestroyed:
			case IShipController::orderGateOnThreat:
				CancelCurrentOrder();
				AddOrder(IShipController::orderGate, NULL, 0);
				break;

			//	Avenge

			default:
				if (Ctx.pDestroyer && Ctx.pDestroyer->CanAttack() && m_pShip->IsEnemy(Ctx.pDestroyer))
					{
					CancelCurrentOrder();
					AddOrder(IShipController::orderDestroyTarget, Ctx.pDestroyer, 0);
					}
			}
		}

	//	Clean up debug ship

	if (Ctx.pObj == g_pDebugShip)
		g_pDebugShip = NULL;
	}

void CStandardShipAI::OnOrderChanged (void)

//	OnOrderChanged
//
//	The current order has changed

	{
	SetState(stateNone);
	}

void CStandardShipAI::OnReadFromStream (SLoadCtx &Ctx)

//	OnReadFromStream
//
//	Reads controller data from stream
//
//	DWORD		m_State
//	DWORD		m_pDest (CSpaceObject ref)
//	DWORD		m_pTarget (CSpaceObject ref)
//	Metric		m_rDistance
//
//	CString		m_pCommandCode (unlinked)
//
//	DWORD		m_iLastAttack
//	DWORD		m_iCountdown
//	DWORD		flags

	{
	DWORD dwLoad;

	//	Read stuff

	Ctx.pStream->Read((char *)&m_State, sizeof(DWORD));
	Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_pDest);
	Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_pTarget);
	Ctx.pStream->Read((char *)&m_rDistance, sizeof(Metric));

	//	Read code

	if (Ctx.dwVersion >= 13)
		{
		CString sCode;
		sCode.ReadFromStream(Ctx.pStream);
		if (!sCode.IsBlank())
			m_pCommandCode = g_pUniverse->GetCC().Link(sCode, 0, NULL);
		else
			m_pCommandCode = NULL;
		}
	else
		m_pCommandCode = NULL;

	//	Read more stuff

	Ctx.pStream->Read((char *)&m_iLastAttack, sizeof(DWORD));
	if (Ctx.dwVersion >= 4)
		Ctx.pStream->Read((char *)&m_iCountdown, sizeof(DWORD));
	else
		m_iCountdown = 1;

	//	Flags

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_bHasEscorts =			((dwLoad & 0x00000001) ? true : false);
	}

void CStandardShipAI::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Save the AI data to a stream
//
//	DWORD		m_State
//	DWORD		m_pDest (CSpaceObject ref)
//	DWORD		m_pTarget (CSpaceObject ref)
//	Metric		m_rDistance
//
//	CString		m_pCommandCode (unlinked)
//
//	DWORD		m_iLastAttack
//	DWORD		m_iCountdown
//	DWORD		flags

	{
	DWORD dwSave;

	pStream->Write((char *)&m_State, sizeof(DWORD));
	m_pShip->GetSystem()->WriteObjRefToStream(m_pDest, pStream);
	m_pShip->GetSystem()->WriteObjRefToStream(m_pTarget, pStream);
	pStream->Write((char *)&m_rDistance, sizeof(Metric));

	CString sCode;
	if (m_pCommandCode)
		sCode = g_pUniverse->GetCC().Unlink(m_pCommandCode);
	sCode.WriteToStream(pStream);

	pStream->Write((char *)&m_iLastAttack, sizeof(DWORD));
	pStream->Write((char *)&m_iCountdown, sizeof(DWORD));

	dwSave = 0;
	dwSave |= (m_bHasEscorts ?			0x00000001 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

void CStandardShipAI::SetCommandCode (ICCItem *pCode)

//	SetCommandCode
//
//	Sets the command code for the ship

	{
	if (m_pCommandCode)
		{
		m_pCommandCode->Discard(&(g_pUniverse->GetCC()));
		m_pCommandCode = NULL;
		}

	if (pCode)
		m_pCommandCode = pCode->Reference();
	}

void CStandardShipAI::SetDebugShip (CShip *pShip)
	{
	g_pDebugShip = pShip;
	}

void CStandardShipAI::SetState (StateTypes State)

//	SetState
//
//	Sets the current state

	{
	//	If docking has been requested and we're trying to change state
	//	then cancel docking.

	if (IsDockingRequested())
		CancelDocking(m_pDest);

	//	Set state (NOTE: We do this before we undock because the Undock
	//	call may destroy the station and cause us to recurse into SetState.
	//	This happens when a ship is guarding an empty cargo crate).

	m_State = State;
	m_pTarget = NULL;
	m_pDest = NULL;

	//	If we're currently docked and we're changing to a state that
	//	does not support docking, then we undock first.

	if (m_pShip->GetDockedObj()
			&& State != stateNone 
			&& State != stateWaiting
			&& State != stateWaitingForThreat
			&& State != stateWaitingUnarmed
			&& State != stateLooting
			&& State != stateLootingOnce)
		m_pShip->Undock();
	}

//	Helpers

bool EnemyStationsAtBearing (CSpaceObject *pShip, int iBearing, Metric rRange)

//	EnemyStationsAtBearing
//
//	Examines a point at 2*rRange from the the ship, bearing iBearing. If there are
//	any enemies within an rRange radius of that point, we return TRUE.

	{
	int i;

	//	Get the sovereign

	CSovereign *pSovereign = pShip->GetSovereign();
	if (pSovereign == NULL || pShip->GetSystem() == NULL)
		return false;

	//	Get the list of enemy objects

	const CSpaceObjectList &ObjList = pSovereign->GetEnemyObjectList(pShip->GetSystem());

	Metric rRange2 = rRange * rRange;
	CVector vCenter = pShip->GetPos() + PolarToVector(iBearing, 2.0 * rRange);

	int iObjCount = ObjList.GetCount();
	for (i = 0; i < iObjCount; i++)
		{
		CSpaceObject *pObj = ObjList.GetObj(i);

		if ((pObj->GetScale() == scaleStructure)
				&& pObj->CanAttack())
			{
			CVector vRange = pObj->GetPos() - vCenter;
			Metric rDistance2 = vRange.Dot(vRange);

			if (rDistance2 < rRange2)
				return true;
			}
		}

	return false;
	}

