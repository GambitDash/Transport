//	PlayerController.cpp
//
//	Implements class to control player's ship

#include "PreComp.h"
#include "Transcendence.h"

#include "CUi.h"

#define INVOKE_REFRESH_INTERVAL					100		//	Ticks to gain 1 point of deity rel (if rel is negative)

#define STR_NO_TARGET_FOR_FLEET					CONSTLIT("No target selected")

static DATADESCSTRUCT g_DataDesc[] =
	{	{ DATADESC_OPCODE_VTABLE,		1,	0 },		//	IShipController virtuals
		{ DATADESC_OPCODE_REFERENCE,	1,	0 },		//	m_pTrans
		{ DATADESC_OPCODE_INT,			4,	0 },		//	ints
		{ DATADESC_OPCODE_STOP,	0,	0 } };
static CObjectClass<CPlayerShipController>g_Class(OBJID_CPLAYERSHIPCONTROLLER, g_DataDesc);

const Metric MAX_IN_COMBAT_RANGE =				(LIGHT_SECOND * 30.0);
const int UPDATE_HELP_TIME =					31;
const Metric MAX_AUTO_TARGET_DISTANCE =			(LIGHT_SECOND * 30.0);

#define MAX_DOCK_DISTANCE						(g_KlicksPerPixel * 256.0)
#define MAX_GATE_DISTANCE						(g_KlicksPerPixel * 64.0)
#define MAX_STARGATE_HELP_RANGE					(g_KlicksPerPixel * 256.0)

CPlayerShipController::CPlayerShipController (void) : CObject(&g_Class),
		m_pTrans(NULL),
		m_pStation(NULL),
		m_pTarget(NULL),
		m_pDestination(NULL),
		m_pWreck(NULL),
		m_GateFollowers(FALSE),
		m_iLastHelpTick(0),
		m_iLastHelpUseTick(0),
		m_bShowHelpDock(true),
		m_bShowHelpMap(true),
		m_bShowHelpAutopilot(true),
		m_bShowHelpGate(true),
		m_bShowHelpUse(true),
		m_bShowHelpRefuel(true),
		m_bShowHelpEnableDevice(true),
		m_iManeuver(IShipController::NoRotation),
		m_bThrust(false),
		m_bFireMain(false),
		m_bFireMissile(false),
		m_bActivate(false),
		m_bMapHUD(false),
		m_iCredits(0),
		m_iScore(0),
		m_iSystemsVisited(0),
		m_iEnemiesDestroyed(0),
		m_pBestEnemyDestroyed(NULL),
		m_iBestEnemyDestroyedCount(0),
		m_bShowHelpSwitchMissile(true),
		m_iLastHelpFireMissileTick(0),
		m_bShowHelpFireMissile(true),
		m_iInsuranceClaims(0),
		m_bInsured(false),
		m_dwFormationPlace(0)

//	CPlayerShipController constructor

	{
	}

void CPlayerShipController::AddOrder(OrderTypes Order, CSpaceObject *pTarget, DWORD dwData, bool bAddBefore)

//	AddOrder
//
//	We get an order

	{
	//	For now, we only deal with one order at a time

	switch (Order)
		{
		case orderGuard:
		case orderEscort:
		case orderDock:
			SetDestination(pTarget);
			break;

		case orderAimAtTarget:
		case orderDestroyTarget:
			if (m_pShip->HasTargetingComputer())
				SetTarget(pTarget);

			SetDestination(pTarget);
			break;
		default:
			assert(0);
			break;
		}
	}

void CPlayerShipController::CancelAllOrders (void)

//	CancelAllOrder
//
//	Cancel all orders

	{
	SetDestination(NULL);
	}

void CPlayerShipController::CancelCurrentOrder (void)

//	CancelCurrentOrder
//
//	Cancel current order

	{
	SetDestination(NULL);
	}

void CPlayerShipController::Cargo (void)

//	Cargo
//
//	Show the cargo screen

	{
	if (m_pShip == NULL)
		return;

	const SPlayerSettings *pSettings = m_pShip->GetClass()->GetPlayerSettings();
	if (pSettings->m_pShipScreen == NULL)
		return;

	m_pTrans->ShowDockScreen(m_pShip, 
			pSettings->m_pShipScreen->GetDesc(),
			CString());

	m_pShip->OnComponentChanged(comCargo);
	}

void CPlayerShipController::Communications (CSpaceObject *pObj, 
											MessageTypes iMsg, 
											DWORD dwData)

//	Communications
//
//	Send a message to the given object

	{
	switch (iMsg)
		{
		case msgFormUp:
			{
			DWORD dwFormation;
			if (dwData == 0xffffffff)
				dwFormation = dwData;
			else
				dwFormation = MAKELONG(m_dwFormationPlace, (DWORD)dwData);

			DWORD dwRes = m_pShip->Communicate(pObj, iMsg, m_pShip, dwFormation);
			if (dwRes == resAck)
				m_dwFormationPlace++;
			break;
			}

		case msgAttack:
			{
			if (m_pTarget)
				m_pShip->Communicate(pObj, msgAttack, m_pTarget);
			else
				m_pTrans->DisplayMessage(STR_NO_TARGET_FOR_FLEET);

			break;
			}

		default:
			m_pShip->Communicate(pObj, iMsg);
			break;
		}
	}

void CPlayerShipController::ClearFireAngle (void)

//	ClearFireAngle
//
//	Clears the fire angle of weapon and launcher

	{
	int i;

	for (i = 0; i < m_pShip->GetDeviceCount(); i++)
		{
		CInstalledDevice *pDevice = m_pShip->GetDevice(i);
		if (!pDevice->IsEmpty()
				&& (pDevice->GetCategory() == itemcatWeapon 
					|| pDevice->GetCategory() == itemcatLauncher))
			pDevice->SetFireAngle(-1);
		}
	}

CString CPlayerShipController::DebugCrashInfo (void)

//	DebugCrashInfo
//
//	Returns debug crash info

	{
	int i;
	CString sResult;

	sResult.Append(CONSTLIT("CPlayerShipController\r\n"));
	sResult.Append(strPatternSubst(CONSTLIT("m_pStation: %s\r\n"), CSpaceObject::DebugDescribe(m_pStation).GetASCIIZPointer()));
	sResult.Append(strPatternSubst(CONSTLIT("m_pTarget: %s\r\n"), CSpaceObject::DebugDescribe(m_pTarget).GetASCIIZPointer()));
	sResult.Append(strPatternSubst(CONSTLIT("m_pDestination: %s\r\n"), CSpaceObject::DebugDescribe(m_pDestination).GetASCIIZPointer()));

	for (i = 0; i < m_TargetList.GetCount(); i++)
		sResult.Append(strPatternSubst(CONSTLIT("m_TargetList[%d]: %s\r\n"), i, CSpaceObject::DebugDescribe(m_TargetList.Get(i)).GetASCIIZPointer()));

	return sResult;
	}

void CPlayerShipController::DestroyPlayer (const CString &sEpitaph)

//	DestroyPlayer
//
//	Player ship destroyed

	{
	CString sText = sEpitaph;

	//	Check to see if the epitaph is of the pattern "abc for xyz"
	//	If it is, convert from "abc for xyz" to "abc in the blah system for xyz"

	int iFound = strFind(sText, CONSTLIT(" for "));
	if (iFound != -1)
		sText = strPatternSubst(CONSTLIT("%s in the %s System for %s"),
				strSubString(sEpitaph, 0, iFound).GetASCIIZPointer(),
				m_pShip->GetSystem()->GetName().GetASCIIZPointer(),
				strSubString(sEpitaph, iFound + 5, -1).GetASCIIZPointer());

	//	Otherwise, just add " in the blah system" at the end

	else
		sText.Append(strPatternSubst(CONSTLIT(" in the %s System"), m_pShip->GetSystem()->GetName().GetASCIIZPointer()));

	//	If we happen to be docked, then defer the destruction until we undock
	//	This can happen if we get destroyed from an event at the station

	if (m_pStation)
		{
		m_sDeferredDestruction = sText;
		}

	//	Otherwise, continue with destruction

	else
		{
		m_pTrans->PlayerDestroyed(sText);

		m_iManeuver = IShipController::NoRotation;
		m_bThrust = false;
		m_bFireMain = false;
		m_bFireMissile = false;
		m_bActivate = false;
		}
	}

CSpaceObject *CPlayerShipController::FindDockTarget (void)

//	FindDockTarget
//
//	Finds the closest dock target

	{
	int i;

	CSystem *pSystem = m_pShip->GetSystem();
	CSpaceObject *pStation = NULL;
	Metric rMaxDist2 = MAX_DOCK_DISTANCE * MAX_DOCK_DISTANCE;
	Metric rBestDist = rMaxDist2;

	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);

		if (pObj 
				&& pObj->SupportsDocking()
				&& pObj != m_pShip)
			{
			CVector vDist = pObj->GetPos() - m_pShip->GetPos();
			Metric rDist2 = vDist.Length2();

			if (rDist2 < rMaxDist2)
				{
				//	If the station is inside the dock distance, check
				//	to see how close we are to a docking position.

				CVector vDockPos = pObj->GetNearestDockVector(m_pShip);
				Metric rDockDist2 = vDockPos.Length2();

				if (rDockDist2 < rBestDist)
					{
					rBestDist = rDockDist2;
					pStation = pObj;
					}
				}
			}
		}

	return pStation;
	}

void CPlayerShipController::FleetOrder (MessageTypes iOrder, DWORD dwData2)

//	FleetOrder
//
//	Invoke fleet order

	{
	int i;
	CSystem *pSystem = m_pShip->GetSystem();

	//	Iterate over all ships in formation

	m_dwFormationPlace = 0;

	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);

		if (pObj 
				&& pObj->GetEscortPrincipal() == m_pShip
				&& pObj != m_pShip)
			Communications(pObj, iOrder, dwData2);
		}

	m_dwFormationPlace = 0;
	}

void CPlayerShipController::Gate (void)

//	Gate
//
//	Enter a stargate

	{
	CSystem *pSystem = m_pShip->GetSystem();

	//	Find the stargate closest to the ship

	int i;
	Metric rBestDist = MAX_GATE_DISTANCE * MAX_GATE_DISTANCE;
	CSpaceObject *pStation = NULL;
	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);

		if (pObj 
				&& pObj->SupportsGating()
				&& pObj != m_pShip)
			{
			CVector vDist = pObj->GetPos() - m_pShip->GetPos();
			Metric rDist = vDist.Length2();

			if (rDist < rBestDist)
				{
				rBestDist = rDist;
				pStation = pObj;
				}
			}
		}

	//	If we did not find a station then we're done

	if (pStation == NULL)
		{
		m_pTrans->DisplayMessage(CONSTLIT("No stargates in range"));
		return;
		}

	//	Otherwise, request gating

	pStation->RequestGate(m_pShip);
	}

int CPlayerShipController::GetCombatPower (void)

//	GetCombatPower
//
//	Computes the combat strength of this ship
//
//	CombatPower = ((reactor-power)^0.515) / 2.5

	{
	int iReactorPower = m_pShip->GetReactorDesc()->iMaxPower;
	if (iReactorPower <= 100)
		return 4;
	else if (iReactorPower <= 250)
		return 7;
	else if (iReactorPower <= 500)
		return 10;
	else if (iReactorPower <= 1000)
		return 14;
	else if (iReactorPower <= 2500)
		return 22;
	else if (iReactorPower <= 5000)
		return 32;
	else if (iReactorPower <= 10000)
		return 46;
	else if (iReactorPower <= 25000)
		return 74;
	else
		return 100;
	}

DWORD CPlayerShipController::GetCommsStatus (void)

//	GetCommsStatus
//
//	Returns the comms capabilities of all the ships that are escorting
//	the player.

	{
	int i;
	CSystem *pSystem = m_pShip->GetSystem();
	DWORD dwStatus = 0;

	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);

		if (pObj && pObj != m_pShip)
			dwStatus |= m_pShip->Communicate(pObj, msgQueryCommunications);
		}

	return dwStatus;
	}

bool CPlayerShipController::HasFleet (void)

//	HasFleet
//
//	Returns TRUE if the player has a fleet of ships to command

	{
	int i;
	CSystem *pSystem = m_pShip->GetSystem();

	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);

		if (pObj 
				&& (m_pShip->Communicate(pObj, msgQueryFleetStatus, m_pShip) == resAck)
				&& pObj != m_pShip)
			return true;
		}

	return false;
	}

void CPlayerShipController::InitTargetList (TargetTypes iTargetType, bool bUpdate)

//	InitTargetList
//
//	Initializes the target list

	{
	int i;

	//	Compute some invariants based on our perception

	int iPerception = m_pShip->GetPerception();

	//	Make a list of all targets

	if (!bUpdate)
		m_TargetList.RemoveAll();

	CSystem *pSystem = m_pShip->GetSystem();
	char szBuffer[1024];
	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);

		if (pObj 
				&& pObj->CanBeHit()
				&& pObj != m_pShip)
			{
			bool bInList = false;

			//	Figure out if we this object matches what we're looking for.
			//	If this is an enemy that can attack, then only show it if
			//	we're looking for enemy targets; otherwise, show it when
			//	we're looking for friendly targets

			int iMainKey = -1;
			if ((iTargetType == targetEnemies) == (m_pShip->IsEnemy(pObj) && pObj->CanAttack()))
				{
				if (iTargetType == targetEnemies)
					{
					if (pObj->GetScale() == scaleShip)
						iMainKey = 0;
					else if (pObj->GetScale() == scaleStructure)
						iMainKey = 1;
					}
				else
					{
					if (pObj->GetScale() == scaleShip || pObj->GetScale() == scaleStructure)
						iMainKey = 0;
					}
				}

			//	If not what we're looking for, skip

			if (iMainKey != -1)
				{
				//	Figure out the distance to the object

				CVector vDist = pObj->GetPos() - m_pShip->GetPos();
				Metric rDist2 = vDist.Length2();

				//	If we can detect the object then it belongs on the list

				if (rDist2 < pObj->GetDetectionRange2(iPerception))
					{
					//	Compute the distance in light-seconds

					int iDist = (int)((vDist.Length() / LIGHT_SECOND) + 0.5);

					//	Compute the sort order based on the main key and the distance
					sprintf(szBuffer, "%d%08d%8x", iMainKey, iDist, (int)pObj);
					bInList = true;
					}
				}

			//	Add or update the list

			if (bUpdate)
				{
				int iIndex;
				bool bFound = m_TargetList.Find(pObj, &iIndex);

				if (bInList)
					{
					if (!bFound)
						m_TargetList.Add(CString(szBuffer), pObj);
					}
				else
					{
					if (bFound)
						m_TargetList.Remove(iIndex);
					}
				}
			else if (bInList)
				m_TargetList.Add(CString(szBuffer), pObj);
			}
		}
	}

void CPlayerShipController::InsuranceClaim (void)

//	InsuranceClaim
//
//	Repair the ship and update insurance claim

	{
	ASSERT(m_bInsured);
	ASSERT(m_pShip);

	//	No longer insured

	m_iInsuranceClaims++;
	m_bInsured = false;

	//	Repair ship

	m_pShip->RepairAllArmor();
	m_pShip->Refuel(m_pShip->GetMaxFuel());
	m_pShip->ClearBlindness();
	m_pShip->ClearParalyzed();
	m_pShip->Decontaminate();

	//	Empty out the wreck

	if (m_pWreck)
		{
		m_pWreck->GetItemList().DeleteAll();
		m_pWreck->InvalidateItemListAddRemove();
		m_pWreck = NULL;
		}
	}

void CPlayerShipController::OnArmorRepaired (int iSection)

//	OnArmorRepaired
//
//	Armor repaired or replaced

	{
	m_pTrans->UpdateArmorDisplay();
	}

void CPlayerShipController::OnBlindnessChanged (bool bBlind)

//	OnBlindnessChanged
//
//	Player is blind or not blind

	{
	if (!bBlind)
		m_pTrans->DisplayMessage(CONSTLIT("Visual display repaired"));
	}

DWORD CPlayerShipController::OnCommunicate (CSpaceObject *pSender, MessageTypes iMessage, CSpaceObject *pParam1, DWORD dwParam2)

//	OnCommunicate
//
//	Message from another object

	{
	CSovereign *pSovereign = NULL;

	if (pSender)
		pSovereign = pSender->GetSovereign();

	//	Make sure we have a sovereign

	if (pSovereign == NULL)
		pSovereign = g_pUniverse->FindSovereign(g_PlayerSovereignUNID);

	//	Get the message based on the sovereign

	const CString &sMessage = pSovereign->GetText(iMessage);
	if (!sMessage.IsBlank())
		{
		m_pTrans->DisplayMessage(sMessage);
		if (pSender)
			pSender->Highlight(CG16bitImage::RGBValue(0, 255, 0));
		}

	return resNoAnswer;
	}

void CPlayerShipController::OnComponentChanged (ObjectComponentTypes iComponent)

//	OnComponentChanged
//
//	Component has changed

	{
	switch (iComponent)
		{
		case comDeviceCounter:
			m_pTrans->UpdateDeviceCounterDisplay();
			break;
		default:
			/* Do nothing. */
			break;
		}
	}

void CPlayerShipController::OnDamaged (CSpaceObject *pCause, int iSection, const DamageDesc &Damage)

//	OnDamaged
//
//	We get called here when the ship takes damage

	{
	m_pTrans->DamageFlash();
	m_pTrans->Autopilot(false);
	if (!m_pShip->IsArmorRepairable(iSection))
		m_pTrans->DisplayMessage(CONSTLIT("Hull breach imminent!"));

	m_pTrans->UpdateArmorDisplay();
	}

bool CPlayerShipController::OnDestroyCheck (DestructionTypes iCause, CSpaceObject *pCause)

//	OnDestroyCheck
//
//	Check to see if the player ship can be destroyed

	{
	int i;

	//	Loop over powers

	for (i = 0; i < g_pUniverse->GetPowerCount(); i++)
		{
		CPower *pPower = g_pUniverse->GetPower(i);
		if (!pPower->OnDestroyCheck(m_pShip, iCause, pCause))
			return false;
		}

	return true;
	}

void CPlayerShipController::OnDestroyed (DestructionTypes iCause, 
										 CSpaceObject *pCause,
										 bool *ioResurrectPending)

//	OnDestroyed
//
//	Ship has been destroyed

	{
	//	If we can be resurrected, do so now, then set the flag so that
	//	our caller doesn't delete the ship object

	if (IsInsured())
		*ioResurrectPending = true;

	//	Figure out who killed us

	CString sCause;
	if (pCause)
		{
		if (pCause->IsAngryAt(m_pShip) || pCause->GetSovereign() == NULL)
			sCause = strPatternSubst(CONSTLIT("by %s"), pCause->GetDamageCauseNounPhrase(nounArticle).GetPointer());
		else
			{
			if (pCause == m_pShip)
				sCause = m_pTrans->ComposePlayerNameString(CONSTLIT("accidentally by %his% own weapon"));
			else
				sCause = strPatternSubst(CONSTLIT("accidentally by %s"), pCause->GetDamageCauseNounPhrase(nounArticle).GetPointer());
			}
		}
	else
		sCause = CONSTLIT("by unknown forces");

	//	Otherwise, proceed with destruction

	CString sText;
	switch (iCause)
		{
		case killedByDamage:
			sText = strPatternSubst(CONSTLIT("destroyed %s"), sCause.GetASCIIZPointer());
			break;

		case killedByDisintegration:
			sText = strPatternSubst(CONSTLIT("disintegrated %s"), sCause.GetASCIIZPointer());
			break;

		case killedByRunningOutOfFuel:
			sText = CONSTLIT("ran out of fuel");
			break;

		case killedByRadiationPoisoning:
			sText = CONSTLIT("killed by radiation poisoning");
			break;

		case killedBySelf:
			sText = CONSTLIT("self-destructed");
			break;

		case killedByWeaponMalfunction:
			sText = CONSTLIT("destroyed by a malfunctioning weapon");
			break;

		case killedByEjecta:
			if (pCause)
				sText = strPatternSubst(CONSTLIT("destroyed by exploding debris from %s"), pCause->GetDamageCauseNounPhrase(nounArticle).GetPointer());
			else
				sText = CONSTLIT("destroyed by exploding debris");
			break;

		case killedByExplosion:
		case killedByPlayerCreatedExplosion:
			if (pCause)
				sText = strPatternSubst(CONSTLIT("destroyed by the explosion of %s"), pCause->GetDamageCauseNounPhrase(nounArticle).GetPointer());
			else
				sText = CONSTLIT("destroyed by exploding debris");
			break;

		case killedByShatter:
			sText = strPatternSubst(CONSTLIT("shattered %s"), sCause.GetASCIIZPointer());
			break;

		default:
			sText = CONSTLIT("destroyed by unknown forces");
			break;
		}

	//	Make sure we undock first

	Undock();

	//	Clear various variables

	if (m_pTarget)
		{
		SetTarget(NULL);
		m_pTrans->UpdateWeaponStatus();
		}

	if (m_pDestination)
		SetDestination(NULL);

	//	Add the name of the system

	DestroyPlayer(sText);
	}

void CPlayerShipController::OnDeviceEnabledDisabled (int iDev, bool bEnable)

//	OnDeviceEnabledDisabled
//
//	Device has been disabled (generally by reactor overload)

	{
	CInstalledDevice *pDevice = m_pShip->GetDevice(iDev);
	if (pDevice)
		{
		if (!bEnable)
			{
			if (m_bShowHelpEnableDevice)
				m_pTrans->DisplayMessage(CONSTLIT("(press [B] to enable/disable devices)"));
			m_pTrans->DisplayMessage(strCapitalize(strPatternSubst(CONSTLIT("%s disabled"),
					pDevice->GetClass()->GetName().GetASCIIZPointer())));
			}
		else
			{
			m_pTrans->DisplayMessage(strCapitalize(strPatternSubst(CONSTLIT("%s enabled"),
					pDevice->GetClass()->GetName().GetASCIIZPointer())));
			}
		}
	}

void CPlayerShipController::OnDeviceFailure (CInstalledDevice *pDev, int iType)

//	OnDeviceFailure
//
//	Device has failed in some way

	{
	switch (iType)
		{
		case CDeviceClass::failWeaponJammed:
			m_pTrans->DisplayMessage(CONSTLIT("Weapon jammed!"));
			break;

		case CDeviceClass::failWeaponMisfire:
			m_pTrans->DisplayMessage(CONSTLIT("Weapon misfire!"));
			break;

		case CDeviceClass::failWeaponExplosion:
			m_pTrans->DisplayMessage(CONSTLIT("Weapon chamber explosion!"));
			break;

		case CDeviceClass::failShieldFailure:
			m_pTrans->DisplayMessage(CONSTLIT("Shield failure"));
			break;

		case CDeviceClass::failDeviceHitByDamage:
			{
			m_pTrans->DisplayMessage(strCapitalize(strPatternSubst(CONSTLIT("%s damaged"), pDev->GetClass()->GetName().GetASCIIZPointer())));
			m_pTrans->UpdateArmorDisplay();
			m_pTrans->UpdateWeaponStatus();
			break;
			}

		case CDeviceClass::failDeviceOverheat:
			{
			m_pTrans->DisplayMessage(strCapitalize(strPatternSubst(CONSTLIT("%s damaged by overheating"), pDev->GetClass()->GetName().GetASCIIZPointer())));
			m_pTrans->UpdateArmorDisplay();
			m_pTrans->UpdateWeaponStatus();
			break;
			}
		}
	}

void CPlayerShipController::OnFuelLowWarning (int iSeq)

//	OnFuelLowWarning
//
//	Warn of low fuel

	{
	//	If -1, we are out of fuel

	if (iSeq == -1)
		{
		m_pTrans->DisplayMessage(CONSTLIT("Out of fuel!"));

		//	Stop

		SetThrust(false);
		SetManeuver(IShipController::NoRotation);
		SetFireMain(false);
		SetFireMissile(false);
		}

	//	Don't warn the player every time

	else if ((iSeq % 15) == 0)
		{
		if (m_bShowHelpRefuel)
			m_pTrans->DisplayMessage(CONSTLIT("(press [S] to access refueling screen)"));
		m_pTrans->DisplayMessage(CONSTLIT("Fuel low!"));
		}
	}

void CPlayerShipController::OnEnterGate (CTopologyNode *pDestNode, const CString &sDestEntryPoint, CSpaceObject *pStargate)

//	OnEnterGate
//
//	Enter stargate

	{
	int i;

	CSystem *pSystem = m_pShip->GetSystem();

	//	Clear our targeting computer (otherwise, we crash since we archive ships
	//	in the old system)

	if (m_pTarget)
		{
		SetTarget(NULL);
		m_pTrans->UpdateWeaponStatus();
		}

	if (m_pDestination)
		SetDestination(NULL);

	//	Create a marker that will hold our place for the POV (the marker
	//	object will be destroyed when the POV changes)

	CPOVMarker *pMarker;
	CPOVMarker::Create(pSystem, m_pShip->GetPos(), NullVector, &pMarker);
	g_pUniverse->SetPOV(pMarker);

	//	Remove the ship from the system
	//	(Note: this will remove us from the system. There are cases
	//	where we will lose our escorts when we leave the system, so we don't
	//	ask about henchmen until after we gate).

	m_pShip->Remove(removedFromSystem, pStargate);

	//	Make a list of all our henchmen

	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);

		if (pObj 
				&& pObj->FollowsObjThroughGate(m_pShip)
				&& pObj != m_pShip)
			m_GateFollowers.AppendObject(pObj, NULL);
		}

	//	Reset our state

	m_pTrans->PlayerEnteredGate(pSystem, pDestNode, sDestEntryPoint);
	ClearShowHelpGate();
	}

void CPlayerShipController::OnLifeSupportWarning (int iSecondsLeft)

//	OnLifeSupportWarning
//
//	Handle life support warning

	{
	if (iSecondsLeft > 10 && ((iSecondsLeft % 5) != 0))
		{
		}
	else if (iSecondsLeft > 1)
		m_pTrans->DisplayMessage(strPatternSubst(CONSTLIT("Life support failure in %d seconds"), iSecondsLeft));
	else if (iSecondsLeft == 1)
		m_pTrans->DisplayMessage(CONSTLIT("Life support failure in 1 second"));
	}

void CPlayerShipController::OnRadiationWarning (int iSecondsLeft)

//	OnRadiationWarning
//
//	Handle radiation warning

	{
	if (iSecondsLeft > 10 && ((iSecondsLeft % 5) != 0))
		{
		}
	else if (iSecondsLeft > 1)
		m_pTrans->DisplayMessage(strPatternSubst(CONSTLIT("Radiation Warning: Fatal exposure in %d seconds"), iSecondsLeft));
	else if (iSecondsLeft == 1)
		m_pTrans->DisplayMessage(CONSTLIT("Radiation Warning: Fatal exposure in 1 second"));
	else
		m_pTrans->DisplayMessage(CONSTLIT("Radiation Warning: Fatal exposure received"));
	}

void CPlayerShipController::OnRadiationCleared (void)

//	OnRadiationCleared
//
//	Handler radiation cleared

	{
	m_pTrans->DisplayMessage(CONSTLIT("Decontamination complete"));
	}

void CPlayerShipController::OnReactorOverloadWarning (int iSeq)

//	OnReactorOverlordWarning
//
//	Handler reactor overload

	{
	//	Warn every 60 ticks

	if ((iSeq % 6) == 0)
		m_pTrans->DisplayMessage(CONSTLIT("Warning: Reactor overload"));
	}

void CPlayerShipController::OnStationDestroyed (const SDestroyCtx &Ctx)

//	OnStationDestroyed
//
//	A station has been destroyed

	{
	//	If the station was targeted, then clear the target

	if (m_pTarget == Ctx.pObj)
		{
		SetTarget(NULL);
		m_pTrans->UpdateWeaponStatus();
		}
	}

void CPlayerShipController::OnWreckCreated (CSpaceObject *pWreck)

//	OnWreckCreated
//
//	Called when the ship has been destroyed and a wreck has been
//	created.

	{
	ASSERT(pWreck);

	//	Change our POV to the wreck

	g_pUniverse->SetPOV(pWreck);
	m_pWreck = pWreck;
	}

void CPlayerShipController::Dock (void)

//	Dock
//
//	Docks the ship with the nearest station

	{
	//	If we're already in the middle of docking, cancel...

	if (m_pStation)
		{
		m_pStation->Undock(m_pShip);
		m_pStation = NULL;
		m_pTrans->DisplayMessage(CONSTLIT("Docking cancelled"));
		return;
		}

	CSpaceObject *pStation = NULL;
	Metric rMaxDist2 = MAX_DOCK_DISTANCE * MAX_DOCK_DISTANCE;

	//	See if the targeted object supports docking

	if (m_pTarget 
			&& m_pTarget != m_pShip 
			&& m_pTarget->SupportsDocking()
			&& (!m_pShip->IsEnemy(m_pTarget) || m_pTarget->IsAbandoned()))
		{
		CVector vDist = m_pTarget->GetPos() - m_pShip->GetPos();
		Metric rDist2 = vDist.Length2();

		if (rDist2 < rMaxDist2)
			pStation = m_pTarget;
		}

	//	Find the station closest to the ship

	if (pStation == NULL)
		pStation = FindDockTarget();

	//	If we did not find a station then we're done

	if (pStation == NULL)
		{
		m_pTrans->DisplayMessage(CONSTLIT("No stations in range"));
		return;
		}

	//	Otherwise, request docking

	if (!pStation->RequestDock(m_pShip))
		return;

	//	Station has agreed to allow dock...

	m_pStation = pStation;

	SetActivate(false);
	SetFireMain(false);
	SetFireMissile(false);
	SetManeuver(IShipController::NoRotation);
	SetThrust(false);

	ClearShowHelpDock();
	}

IShipController::ManeuverTypes CPlayerShipController::GetManeuver (void)
	{
	return m_iManeuver;
	}

bool CPlayerShipController::GetThrust (void)
	{
	return m_bThrust;
	}

bool CPlayerShipController::GetReverseThrust (void)
	{
	return false;
	}

bool CPlayerShipController::GetStopThrust (void)
	{
	return uiIsKeyDown(SDLK_PERIOD);
	}

bool CPlayerShipController::GetMainFire (void)
	{
	return m_bFireMain;
	}

bool CPlayerShipController::GetMissileFire (void)
	{
	return m_bFireMissile;
	}

bool CPlayerShipController::GetDeviceActivate (void)
	{
	return m_bActivate;
	}

CSpaceObject *CPlayerShipController::GetTarget (void)

//	GetTarget
//
//	Returns the target for the player ship

	{
	if (m_pTarget)
		return m_pTarget;
	else
		{
		CSpaceObject *pTarget = m_pShip->GetNearestEnemy(MAX_AUTO_TARGET_DISTANCE, true);

		//	Make sure the fire angle is set to -1 if we don't have a target.
		//	Otherwise, we will keep firing at the wrong angle after we destroy
		//	a target.

		if (pTarget == NULL)
			{
			if (m_bFireMain)
				{
				CInstalledDevice *pDevice = m_pShip->GetNamedDevice(devPrimaryWeapon);
				if (pDevice)
					pDevice->SetFireAngle(-1);
				}

			if (m_bFireMissile)
				{
				CInstalledDevice *pDevice = m_pShip->GetNamedDevice(devMissileWeapon);
				if (pDevice)
					pDevice->SetFireAngle(-1);
				}
			}

		return pTarget;
		}
	}

bool CPlayerShipController::GetWeaponFire (int iDev)

//	GetWeaponFire
//
//	Returns TRUE if the player is firing weapons

	{
	if (iDev == devPrimaryWeapon)
		return m_bFireMain;
	else if (iDev == devMissileWeapon)
		return m_bFireMissile;
	else
		return false;
	}

void CPlayerShipController::OnDocked (CSpaceObject *pObj)
	{
	m_pTrans->ClearMessage();
	m_pTrans->ShowDockScreen(pObj, pObj->GetDockScreen(), CString());
	}

void CPlayerShipController::OnDockedObjChanged (void)
	{
	//	Reset the dock screen list because items on the station
	//	have changed.

	m_pTrans->ResetDockScreenList();
	}

void CPlayerShipController::OnMessage (CSpaceObject *pSender, const CString &sMsg)

//	OnMessage
//
//	Receive a message from some other object

	{
	if (pSender)
		pSender->Highlight(CG16bitImage::RGBValue(0, 255, 0));

	m_pTrans->DisplayMessage(sMsg);
	}

void CPlayerShipController::OnObjDestroyed (const SDestroyCtx &Ctx)

//	OnObjDestroyed
//
//	An object was destroyed

	{
	//	If we are the cause of the destruction and the ship is
	//	not a friend, then increase our score

	if (Ctx.pDestroyer 
			&& m_pShip == Ctx.pDestroyer->GetOrderGiver(Ctx.iCause)
			&& Ctx.pObj->IsEnemy(m_pShip))
		{
		int iScore = Ctx.pObj->GetScore();
		m_iScore += iScore;

		if (Ctx.pObj->GetCategory() == CSpaceObject::catShip)
			{
			CShip *pShip = Ctx.pObj->AsShip();
			if (pShip)
				{
				m_iEnemiesDestroyed++;

				if (m_pBestEnemyDestroyed == NULL
						|| iScore > m_pBestEnemyDestroyed->GetScore())
					{
					m_pBestEnemyDestroyed = pShip->GetClass();
					m_iBestEnemyDestroyedCount = 1;
					}
				else if (m_pBestEnemyDestroyed == pShip->GetClass())
					m_iBestEnemyDestroyedCount++;
				}
			}
		}

	//	If the object we're docked with got destroyed, then undock

	if (m_pStation == Ctx.pObj)
		{
		m_pStation = NULL;
		Undock();
		}

	//	Clear out some variables

	if (m_pTarget == Ctx.pObj)
		{
		m_pTarget = NULL;
		ClearFireAngle();
		m_pTrans->UpdateWeaponStatus();
		}

	if (m_pDestination == Ctx.pObj)
		m_pDestination = NULL;

	//	Clear out the gate followers array

	for (int i = 0; i < m_GateFollowers.GetCount(); i++)
		if ((CSpaceObject *)m_GateFollowers.GetObject(i) == Ctx.pObj)
			{
			m_GateFollowers.RemoveObject(i);
			i--;
			}

	//	Clear out the target list

	m_TargetList.Remove(Ctx.pObj);

	m_pTrans->OnObjDestroyed(Ctx.pObj, Ctx.pDestroyer);
	}

void CPlayerShipController::OnProgramDamage (CSpaceObject *pHacker, const ProgramDesc &Program)

//	OnProgramDamage
//
//	Hit by a program

	{
	if (pHacker)
		pHacker->Highlight(CG16bitImage::RGBValue(0, 255, 0));

	m_pTrans->DisplayMessage(strPatternSubst(CONSTLIT("Cyberattack detected: %s"), Program.sProgramName.GetASCIIZPointer()));
	}

void CPlayerShipController::OnWeaponStatusChanged (void)

//	OnWeaponStatusChanged
//
//	Weapon status has changed

	{
	m_pTrans->UpdateWeaponStatus();
	}

void CPlayerShipController::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Reads data from stream
//
//	DWORD		m_iGenome
//	DWORD		m_dwStartingShipClass
//	DWORD		m_pShip (CSpaceObject ref)
//	DWORD		m_pStation (CSpaceObject ref)
//	DWORD		m_pTarget (CSpaceObject ref)
//	DWORD		m_pDestination (CSpaceObject ref)
//	DWORD		m_iManeuver
//	DWORD		m_iCredits
//	DWORD		m_iScore
//	DWORD		m_iSystemsVisited
//	DWORD		m_iEnemiesDestroyed
//	DWORD		m_pBestEnemyDestroyed (UNID)
//	DWORD		m_iBestEnemyDestroyedCount
//	DWORD		m_iInsuranceClaims
//	DWORD		flags
//
//	CString		m_sDeferredDestruction

	{
	int i;
	DWORD dwLoad;

	Ctx.pStream->Read((char *)&m_iGenome, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_dwStartingShipClass, sizeof(DWORD));
	Ctx.pSystem->ReadObjRefFromStream(Ctx, (CSpaceObject **)&m_pShip);
	Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_pStation);
	Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_pTarget);
	Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_pDestination);
	Ctx.pStream->Read((char *)&m_iManeuver, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iCredits, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iScore, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iSystemsVisited, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iEnemiesDestroyed, sizeof(DWORD));
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad)
		m_pBestEnemyDestroyed = g_pUniverse->FindShipClass(dwLoad);
	Ctx.pStream->Read((char *)&m_iBestEnemyDestroyedCount, sizeof(DWORD));
	Ctx.pStream->Read((char *)&m_iInsuranceClaims, sizeof(DWORD));

	//	Flags

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	m_bThrust = false;
	m_bFireMain = false;
	m_bFireMissile = false;
	m_bActivate = false;
	m_bInsured =				((dwLoad & 0x00000010) ? true : false);
	m_bShowHelpDock =			((dwLoad & 0x00000020) ? true : false);
	m_bShowHelpMap =			((dwLoad & 0x00000040) ? true : false);
	m_bShowHelpAutopilot =		((dwLoad & 0x00000080) ? true : false);
	m_bShowHelpGate =			((dwLoad & 0x00000100) ? true : false);
	m_bShowHelpUse =			((dwLoad & 0x00000200) ? true : false);
	m_bShowHelpRefuel =			((dwLoad & 0x00000400) ? true : false);
	m_bShowHelpEnableDevice =	((dwLoad & 0x00000800) ? true : false);
	m_bMapHUD =					((dwLoad & 0x00001000) ? true : false);
	m_bShowHelpSwitchMissile =	((dwLoad & 0x00002000) ? true : false);
	m_bShowHelpFireMissile =	((dwLoad & 0x00004000) ? true : false);
	m_iLastHelpTick = 0;
	m_iLastHelpUseTick = 0;
	m_iLastHelpFireMissileTick = 0;

	//	Deities

	if (Ctx.dwVersion < 25)
		{
		int iDummy;

		for (i = 0; i < 2; i++)
			{
			Ctx.pStream->Read((char *)&iDummy, sizeof(DWORD));
			Ctx.pStream->Read((char *)&iDummy, sizeof(DWORD));

			DWORD dwCount;
			Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
			for (int j = 0; j < (int)dwCount; j++)
				Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
			}
		}

	//	Deferred destruction string

	if (Ctx.dwVersion >= 3)
		m_sDeferredDestruction.ReadFromStream(Ctx.pStream);
	}

void CPlayerShipController::ReadyNextMissile (void)

//	ReadyNextMissile
//
//	Select the next missile
	
	{
	m_pShip->ReadyNextMissile();

	CInstalledDevice *pLauncher = m_pShip->GetNamedDevice(devMissileWeapon);
	if (pLauncher)
		{
		if (pLauncher->GetValidVariantCount(m_pShip) == 0)
			m_pTrans->DisplayMessage(CONSTLIT("No missiles on board"));
		else
			{
			CString sVariant;
			int iAmmoLeft;
			pLauncher->GetSelectedVariantInfo(m_pShip, &sVariant, &iAmmoLeft);
			if (sVariant.IsBlank())
				sVariant = pLauncher->GetName();
			m_pTrans->DisplayMessage(strPatternSubst(CONSTLIT("%s ready"), sVariant.GetPointer()));
			}
		}
	else
		m_pTrans->DisplayMessage(CONSTLIT("No launcher installed"));
	}

void CPlayerShipController::ReadyNextWeapon (void)

//	ReadyNextWeapon
//
//	Select the next primary weapon

	{
	m_pShip->ReadyNextWeapon();

	CInstalledDevice *pWeapon = m_pShip->GetNamedDevice(devPrimaryWeapon);
	if (pWeapon)
		{
		//	There is a delay in activation

		m_pShip->SetFireDelay(pWeapon);

		//	Feedback to player

		m_pTrans->DisplayMessage(strPatternSubst(CONSTLIT("%s ready"), strCapitalize(pWeapon->GetName()).GetPointer()));
		}
	}

void CPlayerShipController::SelectNearestTarget (void)

//	SelectNearestTarget
//
//	Selects the nearest enemy target

	{
	//	Must have a targeting computer

	if (!m_pShip->HasTargetingComputer())
		return;

	//	Initialize target list

	InitTargetList(targetEnemies);
	if (m_TargetList.GetCount() > 0)
		SetTarget(m_TargetList.Get(0));
	else
		SetTarget(NULL);
	}

void CPlayerShipController::SetDestination (CSpaceObject *pTarget)

//	SetDestination
//
//	Sets the destination for the player

	{
	//	Select

	if (m_pDestination)
		m_pDestination->ClearPlayerTarget();

	m_pDestination = pTarget;

	if (m_pDestination)
		m_pDestination->SetPlayerTarget();
	}

void CPlayerShipController::SetFireMain (bool bFire)

//	SetFireMain
//
//	Fire main weapon

	{
	m_bFireMain = bFire;

	//	If we don't have a target, set our fire angle to fire straight ahead
	//	we need this for omni-directional weapons

	if (bFire && m_pTarget == NULL)
		{
		CInstalledDevice *pDevice = m_pShip->GetNamedDevice(devPrimaryWeapon);
		if (pDevice)
			pDevice->SetFireAngle(-1);
		}
	}

void CPlayerShipController::SetFireMissile (bool bFire)

//	SetFireMissile
//
//	Fire launcher weapon

	{
	m_bFireMissile = bFire;

	//	If we don't have a target, set our fire angle to fire straight ahead
	//	we need this for omni-directional weapons

	if (bFire && m_pTarget == NULL)
		{
		CInstalledDevice *pDevice = m_pShip->GetNamedDevice(devMissileWeapon);
		if (pDevice)
			pDevice->SetFireAngle(-1);
		}
	}

void CPlayerShipController::SetTarget (CSpaceObject *pTarget)

//	SetTarget
//
//	Sets the target

	{
	//	Select

	if (m_pTarget)
		{
		m_pTarget->ClearSelection();
		m_pTarget->ClearPlayerTarget();
		}

	m_pTarget = pTarget;

	if (m_pTarget)
		{
		m_pTarget->SetSelection();
		m_pTarget->SetPlayerTarget();
		}
	else
		{
		m_TargetList.RemoveAll();
		ClearFireAngle();
		}
	}

void CPlayerShipController::SelectNextFriendly (void)

//	SelectNextFriendly
//
//	Sets the target to the next nearest friendly object

	{
	//	Must have a targeting computer

	if (!m_pShip->HasTargetingComputer())
		return;

	//	If a friendly is already selected, then cycle
	//	to the next friendly.

	if (m_pTarget && !(m_pShip->IsEnemy(m_pTarget) && m_pTarget->CanAttack()))
		{
		InitTargetList(targetFriendlies, true);
		if (m_TargetList.GetCount() > 0)
			{
			if (m_pTarget)
				{
				//	Look for the current target

				int iIndex;
				if (m_TargetList.Find(m_pTarget, &iIndex))
					{
					iIndex++;
					if (iIndex < m_TargetList.GetCount())
						SetTarget(m_TargetList.Get(iIndex));
					else
						SetTarget(m_TargetList.Get(0));
					}
				else
					SetTarget(m_TargetList.Get(0));
				}
			else
				SetTarget(m_TargetList.Get(0));
			}
		else
			SetTarget(NULL);
		}

	//	Otherwise, select the nearest friendly

	else
		{
		InitTargetList(targetFriendlies);

		if (m_TargetList.GetCount() > 0)
			SetTarget(m_TargetList.Get(0));
		else
			SetTarget(NULL);
		}
	}

void CPlayerShipController::SelectNextTarget (void)

//	SelectNextTarget
//
//	Selects the next target

	{
	//	Must have a targeting computer

	if (!m_pShip->HasTargetingComputer())
		return;

	//	If an enemy target is already selected, then cycle
	//	to the next enemy.

	if (m_pTarget && m_pShip->IsEnemy(m_pTarget) && m_pTarget->CanAttack())
		{
		InitTargetList(targetEnemies, true);
		if (m_TargetList.GetCount() > 0)
			{
			if (m_pTarget)
				{
				//	Look for the current target

				int iIndex;
				if (m_TargetList.Find(m_pTarget, &iIndex))
					{
					iIndex++;
					if (iIndex < m_TargetList.GetCount())
						SetTarget(m_TargetList.Get(iIndex));
					else
						SetTarget(m_TargetList.Get(0));
					}
				else
					SetTarget(m_TargetList.Get(0));
				}
			else
				SetTarget(m_TargetList.Get(0));
			}
		else
			SetTarget(NULL);
		}

	//	Otherwise, select the nearest enemy target

	else
		{
		InitTargetList(targetEnemies);

		if (m_TargetList.GetCount() > 0)
			SetTarget(m_TargetList.Get(0));
		else
			SetTarget(NULL);
		}
	}

void CPlayerShipController::TransferGateFollowers (CSystem *pSystem, CSpaceObject *pStargate)

//	TransferGateFollowers
//
//	Move henchmen to the new system

	{
	int iTimer = 30 + mathRandom(5, 15);
	for (int i = 0; i < m_GateFollowers.GetCount(); i++)
		{
		CSpaceObject *pFollower = (CSpaceObject *)m_GateFollowers.GetObject(i);

		//	Randomize the position a bit so that ships don't end up being on top
		//	of each other (this can happen to zoanthropes who have very deterministic
		//	motion).

		CVector vPos = pStargate->GetPos() + PolarToVector(mathRandom(0, 359), g_KlicksPerPixel);

		//	Place the ship at the gate in the new system

		pFollower->Remove(removedFromSystem, NULL);
		pFollower->Place(vPos);
		pFollower->AddToSystem(pSystem);
		
		CShip *pShip = pFollower->AsShip();
		if (pShip)
			{
			//	Cancel the follow through gate order
			pShip->GetController()->CancelCurrentOrder();

			//	Ship appears in gate
			pShip->SetInGate(pStargate, iTimer);
			iTimer += mathRandom(20, 40);

			//	Misc clean up
			pShip->ResetMaxSpeed();
			}
		}

	//	Done

	m_GateFollowers.RemoveAll();
	}

void CPlayerShipController::Undock (void)
	{
	if (m_pStation)
		{
		m_pShip->Undock();
		m_pStation = NULL;
		}

	m_pTrans->HideDockScreen();
	m_pTrans->SelectArmor(-1);
	}

void CPlayerShipController::Update (int iTick)

//	Update
//
//	Updates each tick

	{
	//	Destroy the ship, if we deferered destruction (and we are 
	//	no longer docked)

	if (!m_sDeferredDestruction.IsBlank() && m_pStation == NULL)
		{
		CString sText = m_sDeferredDestruction;
		m_sDeferredDestruction = NULL_STR;

		m_iManeuver = IShipController::NoRotation;
		m_bThrust = false;
		m_bFireMain = false;
		m_bFireMissile = false;
		m_bActivate = false;

		m_pTrans->PlayerDestroyed(sText);
		return;
		}

	//	Update help

	if (iTick % UPDATE_HELP_TIME)
		UpdateHelp(iTick);
	}

void CPlayerShipController::UpdateHelp (int iTick)

//	UpdateHelp
//
//	Checks to see if we should show a help message to the player

	{
	//	See if we need to show help and if we can show help at this point

	if (!m_bShowHelpDock 
			&& !m_bShowHelpMap 
			&& !m_bShowHelpAutopilot 
			&& !m_bShowHelpGate
			&& !m_bShowHelpUse
			&& !m_bShowHelpSwitchMissile
			&& !m_bShowHelpFireMissile)
		return;

	//	If we just showed help, then skip

	if ((iTick - m_iLastHelpTick) < 240)
		return;

	//	Don't bother if we're docking, or gating, etc.

	if (!m_pTrans->InGameState() 
			|| DockingInProgress() 
			|| m_pTrans->InAutopilot()
			|| m_pTrans->InMenu())
		return;

	//	See if there are enemies in the area

	bool bEnemiesInRange = m_pShip->IsEnemyInRange(MAX_IN_COMBAT_RANGE, true);

	//	If we've never docked and we're near a dockable station, then tell
	//	the player about docking.

	if (m_bShowHelpDock)
		{
		if (!bEnemiesInRange
				&& !m_pTrans->InMap()
				&& FindDockTarget())
			{
			m_pTrans->DisplayMessage(CONSTLIT("(press [D] to dock with stations and wrecks)"));
			m_iLastHelpTick = iTick;
			return;
			}
		}

	//	If we've never used an item, and we've got a usable item in cargo
	//	and we're not in the middle of anything, then tell the player.

	if (m_bShowHelpUse && !bEnemiesInRange)
		{
		CItemListManipulator ItemList(m_pShip->GetItemList());
		CItemCriteria UsableItems;
		CItem::ParseCriteria(CONSTLIT("u"), &UsableItems);
		ItemList.SetFilter(UsableItems);
		bool bHasUsableItems = ItemList.MoveCursorForward();

		if (!m_pTrans->InMap() 
				&& bHasUsableItems
				&& (m_iLastHelpUseTick == 0 || (iTick - m_iLastHelpUseTick) > 9000))
			{
			m_pTrans->DisplayMessage(CONSTLIT("(press [U] to use items in your cargo hold)"));
			m_iLastHelpTick = iTick;
			m_iLastHelpUseTick = iTick;
			return;
			}
		}

	//	If we've never used the map, and then tell the player about the map

	if (m_bShowHelpMap && !bEnemiesInRange)
		{
		m_pTrans->DisplayMessage(CONSTLIT("(press [M] to see a map of the system)"));
		m_iLastHelpTick = iTick;
		return;
		}

	//	If we've never used autopilot, and we're not in the middle of
	//	anything, then tell the player about autopilot

	if (m_bShowHelpAutopilot && !bEnemiesInRange)
		{
		Metric rSpeed = m_pShip->GetVel().Length();

		if (rSpeed > 0.9 * m_pShip->GetMaxSpeed())
			{
			m_pTrans->DisplayMessage(CONSTLIT("(press [A] to engage autopilot and travel faster)"));
			m_iLastHelpTick = iTick;
			return;
			}
		}

	//	If we've never entered a gate, and there is a gate nearby
	//	and we're not in the middle of anything, then tell the player.

	if (m_bShowHelpGate)
		{
		if (!bEnemiesInRange
				&& !m_pTrans->InMap() 
				&& m_pShip->IsStargateInRange(MAX_STARGATE_HELP_RANGE))
			{
			m_pTrans->DisplayMessage(CONSTLIT("(press [G] over stargate to travel to next system)"));
			m_iLastHelpTick = iTick;
			return;
			}
		}

	//	If we've never switched missiles and we have more than one

	if (m_bShowHelpSwitchMissile)
		{
		if (!bEnemiesInRange &&
				m_pShip->GetMissileCount() > 1)
			{
			m_pTrans->DisplayMessage(CONSTLIT("(press [Tab] to switch missiles)"));
			m_iLastHelpTick = iTick;
			return;
			}
		}

	//	If we've never fired a missile and we have one and we're under attack

	if (m_bShowHelpFireMissile)
		{
		if (bEnemiesInRange
				&& m_pShip->GetMissileCount() > 0 
				&& (m_iLastHelpFireMissileTick == 0 || (iTick - m_iLastHelpFireMissileTick) > 9000))
			{
			m_pTrans->DisplayMessage(CONSTLIT("(press [Shift] to fire missiles)"));
			m_iLastHelpTick = iTick;
			m_iLastHelpFireMissileTick = iTick;
			return;
			}
		}
	}

void CPlayerShipController::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Write data to stream
//
//	DWORD		m_iGenome
//	DWORD		m_dwStartingShipClass
//	DWORD		m_pShip (CSpaceObject ref)
//	DWORD		m_pStation (CSpaceObject ref)
//	DWORD		m_pTarget (CSpaceObject ref)
//	DWORD		m_pDestination (CSpaceObject ref)
//	DWORD		m_iManeuver
//	DWORD		m_iCredits
//	DWORD		m_iScore
//	DWORD		m_iSystemsVisited
//	DWORD		m_iEnemiesDestroyed
//	DWORD		m_pBestEnemyDestroyed (UNID)
//	DWORD		m_iBestEnemyDestroyedCount
//	DWORD		m_iInsuranceClaims
//	DWORD		flags
//
//	CString		m_sDeferredDestruction

	{
	DWORD dwSave;

	dwSave = (DWORD)GetClass()->GetObjID();
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	pStream->Write((char *)&m_iGenome, sizeof(DWORD));
	pStream->Write((char *)&m_dwStartingShipClass, sizeof(DWORD));
	m_pShip->GetSystem()->WriteObjRefToStream(m_pShip, pStream);
	m_pShip->GetSystem()->WriteObjRefToStream(m_pStation, pStream);
	m_pShip->GetSystem()->WriteObjRefToStream(m_pTarget, pStream);
	m_pShip->GetSystem()->WriteObjRefToStream(m_pDestination, pStream);
	pStream->Write((char *)&m_iManeuver, sizeof(DWORD));
	pStream->Write((char *)&m_iCredits, sizeof(DWORD));
	pStream->Write((char *)&m_iScore, sizeof(DWORD));
	pStream->Write((char *)&m_iSystemsVisited, sizeof(DWORD));
	pStream->Write((char *)&m_iEnemiesDestroyed, sizeof(DWORD));

	dwSave = 0;
	if (m_pBestEnemyDestroyed)
		dwSave = m_pBestEnemyDestroyed->GetUNID();
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	pStream->Write((char *)&m_iBestEnemyDestroyedCount, sizeof(DWORD));

	pStream->Write((char *)&m_iInsuranceClaims, sizeof(DWORD));

	dwSave = 0;
	dwSave |= (m_bThrust ?				0x00000001 : 0);
	dwSave |= (m_bFireMain ?			0x00000002 : 0);
	dwSave |= (m_bFireMissile ?			0x00000004 : 0);
	dwSave |= (m_bActivate ?			0x00000008 : 0);
	dwSave |= (m_bInsured ?				0x00000010 : 0);
	dwSave |= (m_bShowHelpDock ?		0x00000020 : 0);
	dwSave |= (m_bShowHelpMap ?			0x00000040 : 0);
	dwSave |= (m_bShowHelpAutopilot ?	0x00000080 : 0);
	dwSave |= (m_bShowHelpGate ?		0x00000100 : 0);
	dwSave |= (m_bShowHelpUse ?			0x00000200 : 0);
	dwSave |= (m_bShowHelpRefuel ?		0x00000400 : 0);
	dwSave |= (m_bShowHelpEnableDevice ? 0x00000800 : 0);
	dwSave |= (m_bMapHUD ?				0x00001000 : 0);
	dwSave |= (m_bShowHelpSwitchMissile ? 0x00002000 : 0);
	dwSave |= (m_bShowHelpFireMissile ?	0x00004000 : 0);
	pStream->Write((char *)&dwSave, sizeof(DWORD));

	m_sDeferredDestruction.WriteToStream(pStream);
	}
