//	CIntroShipController.cpp
//
//	Implements class to control ships on intro screen

#include "PreComp.h"
#include "Transcendence.h"

CIntroShipController::CIntroShipController (void) : CObject(NULL)

//	CIntroShipController constructor

	{
	}

CIntroShipController::~CIntroShipController (void)

//	CIntroShipController destructor

	{
	if (m_pDelegate)
		delete dynamic_cast<CObject *>(m_pDelegate);
	}

CIntroShipController::CIntroShipController (CTranscendenceWnd *pTrans, IShipController *pDelegate) : CObject(NULL),
		m_pTrans(pTrans),
		m_pDelegate(pDelegate)

//	CIntroShipController constructor

	{
	}

void CIntroShipController::OnDestroyed (DestructionTypes iCause, 
										CSpaceObject *pCause,
										bool *ioResurrectPending)

//	OnDestroyed
//
//	Handle on destroyed

	{
	int i;
	CSystem *pSystem = m_pShip->GetSystem();

	//	Make sure each sovereign has a ship

	CSovereign *pSovereign1 = g_pUniverse->FindSovereign(g_PlayerSovereignUNID);
	CSovereign *pSovereign2 = g_pUniverse->FindSovereign(g_PirateSovereignUNID);
	CShip *pShip1 = NULL;
	CShip *pShip2 = NULL;

	//	Look for the surviving ships

	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);

		if (pObj
				&& pObj->GetCategory() == CSpaceObject::catShip
				&& pObj != m_pShip
				&& !pObj->GetData(CONSTLIT("IntroController")).IsBlank())
			{
			if (pObj->GetSovereign() == pSovereign1)
				{
				if (pShip1 == NULL)
					pShip1 = pObj->AsShip();
				}
			else if (pObj->GetSovereign() == pSovereign2)
				{
				if (pShip2 == NULL)
					pShip2 = pObj->AsShip();
				}
			}
		}

	//	Create ships if necessary

	if (pShip1 == NULL)
		m_pTrans->CreateRandomShip(pSystem, pSovereign1, &pShip1);

	if (pShip2 == NULL)
		m_pTrans->CreateRandomShip(pSystem, pSovereign2, &pShip2);

	//	Make sure every ship has an order to attack someone

	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);

		if (pObj
				&& pObj->GetCategory() == CSpaceObject::catShip
				&& pObj != m_pShip)
			{
			CShip *pShip = pObj->AsShip();
			if (pShip)
				{
				IShipController *pController = pShip->GetController();

				CSpaceObject *pTarget;
				IShipController::OrderTypes iOrder = pController->GetCurrentOrderEx(&pTarget);
				if (pTarget == m_pShip || iOrder == IShipController::orderNone)
					{
					pController->CancelAllOrders();
					if (pShip->GetSovereign() == pSovereign1)
						pController->AddOrder(IShipController::orderDestroyTarget, pShip2, 0);
					else
						pController->AddOrder(IShipController::orderDestroyTarget, pShip1, 0);
					}
				}
			}
		}

	//	If this ship used to be the POV, set a new POV

	if (m_pTrans->m_Universe.GetPOV() == m_pShip)
		{
		m_pTrans->m_Universe.SetPOV(pShip1);
#ifdef DEBUG_COMBAT
		pShip2->SetSelection();
#endif
		}

	m_pTrans->m_iLastShipCreated = m_pTrans->m_iTick;
	}
