//	CTranscendenceWnd.cpp
//
//	CTranscendence class

#include "SDL.h"

#include "PreComp.h"
#include "Transcendence.h"
#include "XMLUtil.h"

#include "CRegistry.h"
#include "CFileDirectory.h"
#include "CVersion.h"

extern CGImage *g_screen;

#define MAIN_SCREEN_WIDTH					1024
#define MAIN_SCREEN_HEIGHT					768

#define TEXT_CRAWL_X						512
#define TEXT_CRAWL_HEIGHT					320
#define TEXT_CRAWL_WIDTH					384

#define TICKS_BEFORE_GATE					34
#define TICKS_AFTER_GATE					30
#define TICKS_AFTER_DESTROYED				120

#define DEBUG_CONSOLE_WIDTH					512
#define DEBUG_CONSOLE_HEIGHT				600

#define BAR_COLOR							CG16bitImage::RGBValue(0, 2, 10)

#define REGISTRY_COMPANY_NAME				CONSTLIT("Neurohack")
#define REGISTRY_PRODUCT_NAME				CONSTLIT("Transcendence")
#define REGISTRY_MUSIC_OPTION				CONSTLIT("Music")
#define REGISTRY_SOUND_VOLUME_OPTION		CONSTLIT("SoundVolume")

#define STR_G_TRANS							CONSTLIT("gTrans")
#define STR_G_PLAYER						CONSTLIT("gPlayer")
#define STR_G_PLAYER_SHIP					CONSTLIT("gPlayerShip")

#define HIGH_SCORES_FILENAME				CONSTLIT("HighScores.xml")

int g_iBackBuffers = 2;	//	Triple-buffering

int g_cxScreen = 0;
int g_cyScreen = 0;

const int DEFAULT_SOUND_VOLUME =			7;

#define STR_SMALL_TYPEFACE					CONSTLIT("tahoma.ttf")
#define STR_MEDIUM_TYPEFACE					CONSTLIT("lucidasans.ttf")
#define STR_LARGE_TYPEFACE					CONSTLIT("trebuchet.ttf")
#define STR_FIXED_TYPEFACE					CONSTLIT("lucidaconsole.ttf")

const CItem g_DummyItem;
CItemList g_DummyItemList;
CItemListManipulator g_DummyItemListManipulator(g_DummyItemList);

CTranscendenceWnd::CTranscendenceWnd () :
		m_dwPlayerShip(0),
		m_dwAdventure(0),
		m_State(gsNone),
		m_bShowingMap(false),
		m_bAutopilot(false),
		m_bDebugConsole(false),
		m_chKeyDown('\0'),
		m_bNextWeaponKey(false),
		m_bNextMissileKey(false),
		m_pPlayer(NULL),
		m_DockScreens(TRUE, FALSE),
		m_pCurrentScreen(NULL),
		m_CurrentMenu(menuNone),
		m_CurrentPicker(pickNone),
		m_pMenuObj(NULL),
		m_bRedirectDisplayMessage(false),
		m_hWorkAvailableEvent(0),
		m_pHighScoreList(NULL),
		m_iLastHighScore(-1),
		m_dwIntroShipClass(0),
		m_pIntroSystem(NULL),
		m_pCrawlImage(NULL),
		m_Screen(*g_screen),
		m_pLargeHUD(NULL),
		m_iDamageFlash(0)
#ifdef DEBUGLOG
		, m_Log(CONSTLIT("Debug.log"))
#endif

//	CTranscendence constructor

	{
	ClearDebugLines();
	}

void CTranscendenceWnd::Animate (void)

//	Animate
//
//	Called on each frame

	{
	bool bFailed = false;

	try
		{
		SetProgramState(psAnimating);

		//	If minimized, bail out

		if (m_bMinimized)
			return;

		//	Do the appropriate thing

		switch (m_State)
			{
			case gsNone:
				break;
			case gsLoading:
				AnimateLoading();
				break;

			case gsIntro:
				AnimateIntro();
				break;

			case gsSelectAdventure:
				AnimateSelectAdventure();
				break;

			case gsSelectShip:
				AnimateSelectShip();
				break;

			case gsProlog:
				{
				AnimateCrawlScreen();
				BltScreen();

				//	If we're done creating the screen and the user wants to continue, then
				//	we go forward

				if (m_bContinue && IsBackgroundDone())
					{
					//	Check for error

					if (!m_sBackgroundError.IsBlank())
						{
						CString sError = strPatternSubst(CONSTLIT("Unable to begin new game: %s"), m_sBackgroundError.GetPointer());
						kernelDebugLogMessage(sError.GetASCIIZPointer());

						StartIntro();
						break;
						}

					StopProlog();
					StartGame();
					}
				break;
				}

			case gsInGame:
			case gsDestroyed:
				{
				DWORD dwStartTimer;
				DWORD dwBaseTimer;
				int iFrameIdx = m_iFrameCount % FRAME_RATE_COUNT;
				if (m_Options.bDebugVideo)
					dwBaseTimer = dwStartTimer = SDL_GetTicks();

				//	Figure out some stats

				bool bBlind = false;
				bool bSRSEnhanced = false;
				bool bShowMapHUD = false;
				CShip *pShip = NULL;
				if (m_pPlayer)
					{
					pShip = m_pPlayer->GetShip();
					bBlind = pShip->IsBlind();
					bSRSEnhanced = pShip->IsSRSEnhanced();
					bShowMapHUD = m_pPlayer->IsMapHUDActive();
					}

				//	Update some displays

				if ((m_iTick % 10) == 0)
					{
					SetProgramState(psUpdatingReactorDisplay);
					m_ReactorDisplay.Update();
					SetProgramState(psAnimating);
					}

				//	If we're showing damage flash, fill the screen

				if (m_iDamageFlash > 0 && (m_iDamageFlash % 2) == 0)
					{
					m_Screen.Fill(0, 0, g_cxScreen, g_cyScreen, CG16bitImage::RGBValue(128,0,0));
					if (pShip && pShip->GetSystem())
						{
						if (m_bShowingMap)
							m_Universe.PaintObjectMap(m_Screen, m_rcMainScreen, pShip);
						else
							m_Universe.PaintObject(m_Screen, m_rcMainScreen, pShip);
						}
					}

				//	Otherwise, if we're in map mode, paint the map

				else if (m_bShowingMap)
					{
					SetProgramState(psPaintingMap);
					PaintMap();
					SetProgramState(psAnimating);
					}

				//	Otherwise, if we're blind, paint scramble

				else if (bBlind 
						&& (m_iTick % (20 + (((m_iTick / 100) * pShip->GetDestiny()) % 100))) > 15)
					PaintSRSSnow();

				//	Otherwise, paint the normal SRS screen

				else
					{
					m_iTicks[iFrameIdx][0] = (dwBaseTimer = SDL_GetTicks()) - dwStartTimer;
					SetProgramState(psPaintingSRS);
					m_Universe.PaintPOV(m_Screen, m_rcMainScreen, bSRSEnhanced);
					SetProgramState(psAnimating);

					PaintMainScreenBorder();
					m_iTicks[iFrameIdx][1] = SDL_GetTicks() - dwBaseTimer;
					dwBaseTimer = SDL_GetTicks();
					}

				if (m_iDamageFlash > 0)
					m_iDamageFlash--;

				//	Paint various displays

				SetProgramState(psPaintingLRS);
				PaintLRS();
				m_iTicks[iFrameIdx][2] = SDL_GetTicks() - dwBaseTimer;
				dwBaseTimer = SDL_GetTicks();

				if (!m_bShowingMap || bShowMapHUD)
					{
					SetProgramState(psPaintingArmorDisplay);
					m_ArmorDisplay.Paint(m_Screen);

					SetProgramState(psPaintingReactorDisplay);
					m_ReactorDisplay.Paint(m_Screen);

					SetProgramState(psPaintingTargetDisplay);
					m_TargetDisplay.Paint(m_Screen);

					SetProgramState(psPaintingDeviceDisplay);
					m_DeviceDisplay.Paint(m_Screen);
					}
				m_iTicks[iFrameIdx][3] = SDL_GetTicks() - dwBaseTimer;
				dwBaseTimer = SDL_GetTicks();

				SetProgramState(psPaintingMessageDisplay);
				m_MessageDisplay.Paint(m_Screen);

				SetProgramState(psAnimating);

				if (m_CurrentMenu != menuNone)
					m_MenuDisplay.Paint(m_Screen);
				if (m_CurrentPicker != pickNone)
					m_PickerDisplay.Paint(m_Screen);
				if (m_bDebugConsole)
					m_DebugConsole.Paint(m_Screen);

#ifdef DEBUG_LINE_OF_FIRE
				if (m_pPlayer)
					{
					if (!m_pPlayer->GetShip()->IsLineOfFireClear(m_pPlayer->GetShip()->GetPos(),
							NULL,
							m_pPlayer->GetShip()->GetRotation()))
						g_pUniverse->DebugOutput("line of fire blocked");
					}
#endif
#ifdef DEBUG
				PaintDebugLines();
#endif

				//	Figure out how long it took to paint

				if (m_Options.bDebugVideo)
					{
					DWORD dwNow = SDL_GetTicks();
					m_iPaintTime[iFrameIdx] = dwNow - dwStartTimer;
					dwStartTimer = dwNow;
					}

				//	Some debug information
				if (m_Options.bDebugVideo)
					PaintFrameRate();

				m_iTicks[iFrameIdx][4] = SDL_GetTicks() - dwBaseTimer;
				dwBaseTimer = SDL_GetTicks();

				//	Update the screen
				BltScreen();

				m_iTicks[iFrameIdx][5] = SDL_GetTicks() - dwBaseTimer;
				
				//	Figure out how long it took to blt
				if (m_Options.bDebugVideo)
					{
					DWORD dwNow = SDL_GetTicks();
					m_iBltTime[iFrameIdx] = dwNow - dwStartTimer;
					dwStartTimer = dwNow;
					}
				dwBaseTimer = SDL_GetTicks();
				//	Update the universe

				if (!m_bPaused || m_bPausedStep)
					{
					SetProgramState(psUpdating);
					m_Universe.Update(g_SecondsPerUpdate);
					if (m_bAutopilot)
						{
						m_Universe.Update(g_SecondsPerUpdate);
						m_Universe.Update(g_SecondsPerUpdate);
						m_Universe.Update(g_SecondsPerUpdate);
						m_Universe.Update(g_SecondsPerUpdate);
						}
					SetProgramState(psAnimating);

					if (m_pPlayer)
						m_pPlayer->Update(m_iTick);
					if (m_pPlayer && m_pPlayer->GetSelectedTarget())
						m_TargetDisplay.Invalidate();
					m_iTick++;

					m_bPausedStep = false;
					}

				m_MessageDisplay.Update();

				//	Figure out how long it took to update

				if (m_Options.bDebugVideo)
					{
					DWORD dwNow = SDL_GetTicks();
					m_iUpdateTime[iFrameIdx] = dwNow - dwStartTimer;
					dwStartTimer = dwNow;
					}
				m_iTicks[iFrameIdx][6] = SDL_GetTicks() - dwBaseTimer;
				dwBaseTimer = SDL_GetTicks();

				//	Destroyed?

				if (m_State == gsDestroyed)
					{
					if (!m_bPaused || m_bPausedStep)
						{
						if (--m_iCountdown == 0)
							EndDestroyed();
						m_bPausedStep = false;
						}
					}
				break;
				}

			case gsHelp:
				{
				PaintHelpScreen();
				BltScreen();
				break;
				}

			case gsDocked:
				{
				//	Paint the screen

				m_pCurrentScreen->Paint(m_Screen);
				m_pCurrentScreen->Update();
				PaintMainScreenBorder();
				m_ArmorDisplay.Paint(m_Screen);
				m_TargetDisplay.Paint(m_Screen);

				//	We don't paint the LRS because the player doesn't need it and
				//	because it overwrites the credits/cargo space display
				//PaintLRS();

				//	Update the screen

				BltScreen();

				//	Update the universe (at 1/4 rate)

				if ((m_iTick % 4) == 0)
					m_Universe.Update(g_SecondsPerUpdate);
				m_MessageDisplay.Update();
				m_iTick++;

				//	Invalidate areas of the screen that are overlapped by
				//	the displays. Note that we need to convert to main screen
				//	coordinates.

				if (m_pCurrentScreen)
					{
					RECT rcRect = m_ArmorDisplay.GetRect();
					::OffsetRect(&rcRect, -m_rcMainScreen.left, -m_rcMainScreen.top);
					m_pCurrentScreen->Invalidate(rcRect);

					rcRect = m_TargetDisplay.GetRect();
					::OffsetRect(&rcRect, -m_rcMainScreen.left, -m_rcMainScreen.top);
					m_pCurrentScreen->Invalidate(rcRect);

					rcRect = m_rcLRS;
					::OffsetRect(&rcRect, -m_rcMainScreen.left, -m_rcMainScreen.top);
					m_pCurrentScreen->Invalidate(rcRect);
					}
				break;
				}

			case gsEnteringStargate:
				{
				//	Update some displays

				if ((m_iTick % 10) == 0)
					m_ReactorDisplay.Update();

				//	Tell the universe to paint

				m_Universe.PaintPOV(m_Screen, m_rcMainScreen, false);
				PaintMainScreenBorder();
				PaintLRS();
				m_ArmorDisplay.Paint(m_Screen);
				m_MessageDisplay.Paint(m_Screen);
				m_ReactorDisplay.Paint(m_Screen);
				m_TargetDisplay.Paint(m_Screen);
				m_DeviceDisplay.Paint(m_Screen);

				//	Debug information

				if (m_Options.bDebugVideo)
					PaintFrameRate();

#ifdef DEBUG
				PaintDebugLines();
#endif

				//	Update the screen

				BltScreen();

				//	Update the universe

				m_Universe.Update(g_SecondsPerUpdate);
				m_MessageDisplay.Update();
				m_iTick++;

				if (--m_iCountdown == 0)
					EnterStargate();
				break;
				}

			case gsLeavingStargate:
				{
				//	Update some displays

				if ((m_iTick % 10) == 0)
					m_ReactorDisplay.Update();

				//	Tell the universe to paint

				m_Universe.PaintPOV(m_Screen, m_rcMainScreen, false);
				PaintMainScreenBorder();
				PaintLRS();
				m_ArmorDisplay.Paint(m_Screen);
				m_MessageDisplay.Paint(m_Screen);
				m_ReactorDisplay.Paint(m_Screen);
				m_TargetDisplay.Paint(m_Screen);
				m_DeviceDisplay.Paint(m_Screen);

				//	Debug information

				if (m_Options.bDebugVideo)
					PaintFrameRate();

#ifdef DEBUG
				PaintDebugLines();
#endif

				//	Update the screen

				BltScreen();

				//	Update the universe

				m_Universe.Update(g_SecondsPerUpdate);
				m_MessageDisplay.Update();
				m_iTick++;

				if (--m_iCountdown == 0)
					LeaveStargate();
				break;
				}

			case gsEpilog:
				{
				AnimateCrawlScreen();
				BltScreen();
				break;
				}
			}

		SetProgramState(psUnknown);
		}
	catch (...)
		{
		bFailed = true;
		}

	//	Deal with errors/crashes

	if (bFailed)
		{
		CleanUpDirectDraw();
		ReportCrash();
		}
	}

void CTranscendenceWnd::BltScreen (void)

//	BltScreen
//
//	Blt the off-screen bitmap to the screen

	{
	m_Screen.BltToDC(m_rcWindowScreen.left, m_rcWindowScreen.top);
	}

void CTranscendenceWnd::CleanUpDirectDraw (void)

//	CleanUpDirectDraw
//
//	Clean up

	{
	}

void CTranscendenceWnd::CleanUpPlayerShip (void)

//	CleanUpPlayerShip
//
//	Clean up player ship structures when ship is destroyed

	{
	m_pPlayer = NULL;
	m_ReactorDisplay.CleanUp();
	m_DeviceDisplay.CleanUp();
	m_TargetDisplay.CleanUp();
	m_MenuDisplay.CleanUp();
	m_PickerDisplay.CleanUp();

	m_bDebugConsole = false;
	}

void CTranscendenceWnd::CleanUpPlayerVariables (void)

//	CleanUpPlayerVariables
//
//	Clean up player ship variables

	{
	CCodeChain &CC = m_Universe.GetCC();
	CC.DefineGlobal(STR_G_PLAYER, CC.CreateNil());
	CC.DefineGlobal(STR_G_PLAYER_SHIP, CC.CreateNil());
	}

void CTranscendenceWnd::ClearDebugLines (void)

//	ClearDebugLines
//
//	Clear debug lines

	{
#ifdef DEBUG
	m_iDebugLinesStart = 0;
	m_iDebugLinesEnd = 0;
#endif
	}

void CTranscendenceWnd::ComputeScreenSize (void)

//	ComputeScreenSize
//
//	Computes g_cxScreen and g_cyScreen

	{
	//	By default we use the current resolution (unless in windowed mode)

	if (m_Options.bWindowedMode)
		{
		g_cxScreen = 1024;
		g_cyScreen = 768;
		}
	else
		{
		g_cxScreen = 1024;
		g_cyScreen = 768;
		}

	//	If the current resolution is smaller than 1024x768, then switch to
	//	1024x768.

	if (m_Options.bDirectX)
		{
		if (g_cxScreen < 1024 || g_cyScreen < 768)
			{
			g_cxScreen = 1024;
			g_cyScreen = 768;
			}

		//	Otherwise, if the aspect ratio of the display is the same as
		//	1024x768, then switch to 1024x768.

		else
			{
			int iRatio = g_cxScreen * 100 / g_cyScreen;
			if (iRatio == (102400 / 768))
				{
				g_cxScreen = 1024;
				g_cyScreen = 768;
				}
			}
		}
	}

void CTranscendenceWnd::DebugConsoleOutput (const CString &sOutput)

//	DebugConsoleOutput
//
//	Output to debug console

	{
	m_DebugConsole.Output(sOutput);
	}

void CTranscendenceWnd::DefineGameGlobals (void)

//	DefineGameGlobals
//
//	Define the basic game globals.
//	Note: It is OK to call this multiple times as new variables become available.

	{
	ASSERT(m_pPlayer);

	CCodeChain &CC = m_Universe.GetCC();
	CC.DefineGlobalInteger(STR_G_TRANS, (int)this);
	CC.DefineGlobalInteger(STR_G_PLAYER, (int)m_pPlayer);
	if (m_pPlayer->GetShip())
		CC.DefineGlobalInteger(STR_G_PLAYER_SHIP, (int)m_pPlayer->GetShip());
	else
		CC.DefineGlobal(STR_G_PLAYER_SHIP, CC.CreateNil());
	}

/* YYY Should be moved to a header, or member. */
extern BOOL g_Running;

void CTranscendenceWnd::DoCommand (DWORD dwCmd)

//	DoCommand
//
//	Do command

	{
	ALERROR error;

	switch (dwCmd)
		{
		case CMD_CONTINUE_OLD_GAME:
			{
			CString sError;
			StopIntro();
			if ((error = LoadOldGame(&sError)))
				{
				StartIntro();
				ShowErrorMessage(sError);
				break;
				}
			StartGame();
			break;
			}

		case CMD_START_NEW_GAME:
			{
			//	If we have a saved game and the player tries to create a new
			//	one, warn first that they will lose the other one.

			if (m_bSavedGame)
				{
				m_bOverwriteGameDlg = true;
				break;
				}

			//	Start select a ship to use

			StopIntro();

			//	Re-initialize the universe in case the intro did anything

			if ((error = m_Universe.Reinit()))
				{
				StartIntro();
				ShowErrorMessage(CONSTLIT("Unable to reinitialize the universe"));
				break;
				}

			//	If we have more than one adventure, then select an adventure

			ASSERT(g_pUniverse->GetAdventureDescCount() > 0);
			SetAdventure(g_pUniverse->GetAdventureDesc(0)->GetUNID());

			if (g_pUniverse->GetAdventureDescCount() > 1)
				StartSelectAdventure();
			else
				DoCommand(CMD_LOAD_ADVENTURE);
			break;
			}

		case CMD_SELECT_ADVENTURE:
			{
			StopSelectAdventure();
			DoCommand(CMD_LOAD_ADVENTURE);
			break;
			}

		case CMD_SELECT_ADVENTURE_CANCEL:
			{
			StopSelectAdventure();
			StartIntro();
			break;
			}

		case CMD_NEXT_ADVENTURE:
			m_AdventureDescDisplay.SelectNext();
			break;

		case CMD_PREV_ADVENTURE:
			m_AdventureDescDisplay.SelectPrev();
			break;

		case CMD_LOAD_ADVENTURE:
			{
			CString sError;

			error = m_Universe.InitAdventure(m_dwAdventure, &sError);
			if (error)
				{
				StartIntro();
				ShowErrorMessage(sError);
				break;
				}

			StartSelectShip();
			break;
			}

		case CMD_SELECT_SHIP:
			{
			StopSelectShip();
			StartProlog();
			break;
			}

		case CMD_SELECT_SHIP_CANCEL:
			{
			StopSelectShip();
			StartIntro();
			break;
			}

		case CMD_NEXT_SHIP:
			m_ShipClassDisplay.SelectNext();
			break;

		case CMD_PREV_SHIP:
			m_ShipClassDisplay.SelectPrev();
			break;

		case CMD_QUIT_GAME:
			g_Running = false;
			break;
		}
	}

void CTranscendenceWnd::EndDestroyed (void)

//	EndDestroyed
//
//	Done with destroyed screen. We proceed either to the Epilog or we
//	fullfill an insurance claim

	{
	if (m_pPlayer && m_pPlayer->IsInsured())
		{
		CSpaceObject *pSource = g_pUniverse->GetPOV();
		CSystem *pSystem = pSource->GetSystem();
		CShip *pShip = m_pPlayer->GetShip();

		//	Repair the player's ship

		m_pPlayer->InsuranceClaim();

		//	If necessary, remove the ship from the system first. This can happen if
		//	we are imprisoned by a station (instead of destroyed)

		if (pShip->GetIndex() != -1)
			pShip->Remove(removedFromSystem, NULL);

		//	Place the ship in the system

		pShip->AddToSystem(pSystem);

		//	Find nearest friendly station in the system

		int i;
		Metric rBestDist2 = g_InfiniteDistance * g_InfiniteDistance;
		CSpaceObject *pBestObj = NULL;
		for (i = 0; i < pSystem->GetObjectCount(); i++)
			{
			CSpaceObject *pObj = pSystem->GetObject(i);

			if (pObj 
					&& pObj->GetScale() == scaleStructure
					&& pObj->GetCategory() == CSpaceObject::catStation
					&& !pObj->IsAngryAt(m_pPlayer->GetShip())
					&& pObj->HasAttribute(CONSTLIT("populated"))
					&& pObj != pSource)
				{
				CVector vDist = pSource->GetPos() - pObj->GetPos();
				Metric rDist2 = vDist.Length2();

				if (rDist2 < rBestDist2)
					{
					rBestDist2 = rDist2;
					pBestObj = pObj;
					}
				}
			}

		//	If we didn't find a good object, then pick the central object (this
		//	shouldn't happen)

		if (pBestObj == NULL)
			pBestObj = pSystem->GetObject(0);

		//	Pick a random position and make sure it is not in a barrier

		CVector vPos;
		for (i = 0; i < 100; i++)
			{
			vPos = pBestObj->GetPos() + PolarToVector(mathRandom(0, 359), g_KlicksPerPixel * (200 + (i * 50)));
			if (!pShip->InBarrier(vPos))
				break;
			}

		pShip->Place(vPos);

		//	Place the insurance ship nearby

		CShip *pInsuranceShip;
		pSystem->CreateShip(g_InsuranceShipUNID,
				NULL,
				g_pUniverse->FindSovereign(g_CommonwealthSovereignUNID),
				vPos + PolarToVector(mathRandom(0, 259), g_KlicksPerPixel * 100),
				NullVector,
				mathRandom(0, 359),
				NULL,
				&pInsuranceShip);

		pInsuranceShip->GetController()->AddOrder(IShipController::orderWait, NULL, 5);
		pInsuranceShip->GetController()->AddOrder(IShipController::orderGate, NULL, 0);
		pShip->SendMessage(pInsuranceShip, CONSTLIT("Insurance claim processed. Have a better one!"));

		//	POV

		m_Universe.SetPOV(pShip);
		m_State = gsInGame;

		//	Save the game as a checkpoint (so that we can restore the
		//	game later if we die in this system)

		SaveGame(true);
		}
	else
		StartEpilog();
	}

void CTranscendenceWnd::EnterStargate (void)

//	EnterStargate
//
//	Place the player in the new system

	{
	SetProgramState(psStargateEnter);

	CShip *pShip = m_pPlayer->GetShip();

	//	Get the destination topology node

	CString sEntryPoint = m_sDestEntryPoint;
	CTopologyNode *pTopology = m_pDestNode;
	ASSERT(pTopology);

	//	If we've reached the end game node then the game is over

	if (pTopology->GetSystemDescUNID() == END_GAME_SYSTEM_UNID)
		{
		SetProgramState(psStargateEndGame);

		//	Bonus for escaping Human Space

		m_pPlayer->IncScore(10000);

		//	Final score

		RecordFinalScore(pTopology->GetEpitaph(), pTopology->GetEndGameReason(), true);

		//	Done with ship controller

		CleanUpPlayerShip();
		CleanUpPlayerVariables();

		StartEpilog();
		return;
		}

	//	Load or create the system

	CSystem *pNewSystem = pTopology->GetSystem();
	if (pNewSystem == NULL)
		{
		SetProgramState(psStargateLoadingSystem);

		//	See if we need to create the system

		DWORD dwSystemID = pTopology->GetSystemID();
		if (dwSystemID == 0xffffffff)
			{
			SetProgramState(psStargateCreatingSystem);

			//	If we failed to load, then we need to create a new system

			if (g_pUniverse->CreateStarSystem(pTopology, &pNewSystem) != NOERROR)
				{
				DisplayMessage(CONSTLIT("ERROR: Unable to create system"));
				kernelDebugLogMessage("Error creating system: %s", pTopology->GetSystemName().GetASCIIZPointer());
				throw;
				}

			m_pPlayer->IncSystemsVisited();
			}

		//	Otherwise, load the system

		else
			{
			if (m_GameFile.LoadSystem(dwSystemID, &pNewSystem) != NOERROR)
				{
				DisplayMessage(CONSTLIT("ERROR: Unable to load system"));
				kernelDebugLogMessage("Error loading system: %s (%x)", pTopology->GetSystemName().GetASCIIZPointer(), dwSystemID);
				throw;
				}
			}
		}

	//	Get the entry point. Note: We set the POV right away because Update
	//	cannot be called with a POV in a different system.

	CSpaceObject *pStart = pNewSystem->GetNamedObject(sEntryPoint);
	m_Universe.SetPOV(pStart);

	//	Time passes

	SetProgramState(psStargateUpdateExtended);
	pNewSystem->UpdateExtended();

	//	Gate effect

	pStart->OnObjLeaveGate(pShip);

	//	State

	m_State = gsLeavingStargate;
	m_iCountdown = TICKS_AFTER_GATE;

	//	Clear the POVLRS flag for all objects (so that we don't get the
	//	"Enemy Ships Detected" message when entering a system

	pNewSystem->SetPOVLRS(pStart);

	//	Move any henchmen through the stargate (note: we do this here because
	//	we need to remove the henchmen out of the old system before we save).

	SetProgramState(psStargateTransferringGateFollowers);
	m_pPlayer->TransferGateFollowers(pNewSystem, pStart);

	//	Save the old system

	SetProgramState(psStargateSavingSystem);
	if (m_GameFile.SaveSystem(m_pSystem->GetID(), m_pSystem) != NOERROR)
		kernelDebugLogMessage("Error saving system '%s' to game file", m_pSystem->GetName().GetASCIIZPointer());

	//	Remove the old system

	SetProgramState(psStargateFlushingSystem);
	g_pUniverse->FlushStarSystem(m_pSystem->GetTopology());

	//	Garbage-collect images and load those for the new system

	SetProgramState(psStargateGarbageCollecting);
	g_pUniverse->GarbageCollectLibraryBitmaps();

	SetProgramState(psStargateLoadingBitmaps);
	g_pUniverse->LoadLibraryBitmaps();

	SetProgramState(psStargateEnterDone);
	}

DWORD CTranscendenceWnd::GetDefaultPlayerShip (void)

//	GetDefaultPlayerShip
//
//	Returns the default player ship

	{
	//	Return the freighter (if it exists)

	if (g_pUniverse->FindShipClass(PLAYER_EI500_FREIGHTER_UNID))
		return PLAYER_EI500_FREIGHTER_UNID;

	//	Otherwise, return the first ship available to the player

	else
		{
		int i;

		//	Returns the first ship available to the player

		for (i = 0; i < g_pUniverse->GetShipClassCount(); i++)
			{
			CShipClass *pClass = g_pUniverse->GetShipClass(i);
			if (pClass->IsShownAtNewGame())
				return pClass->GetUNID();
			}

		ASSERT(false);
		return 0;
		}
	}

void CTranscendenceWnd::GetMousePos (POINT *retpt)

//	GetMousePos
//
//	Returns the mouse position relative to the Transcendence window

	{
	int x, y;
	SDL_GetMouseState(&x, &y);
	retpt->x = x;
	retpt->y = y;
	}

ALERROR CTranscendenceWnd::InitDirectDraw (int iColorDepth)

//	InitDirectDraw
//
//	Initialize DirectDraw subsystem (and initializes g_cxScreen and g_cyScreen)

	{
	return NOERROR;
	}

void CTranscendenceWnd::LeaveStargate (void)

//	LeaveStargate
//
//	Place the player in the new system

	{
	CShip *pShip = m_pPlayer->GetShip();
	CSpaceObject *pStargate = m_Universe.GetPOV();
	CSystem *pNewSystem = pStargate->GetSystem();

	pShip->Place(pStargate->GetPos());
	pShip->AddToSystem(pNewSystem);

	//	Set globals

	CCodeChain &CC = m_Universe.GetCC();
	CC.DefineGlobalInteger(STR_G_PLAYER_SHIP, (int)pShip);

	//	Welcome message

	DisplayMessage(strPatternSubst(CONSTLIT("Welcome to the %s system!"), pNewSystem->GetName().GetPointer()));

	//	POV

	m_Universe.SetPOV(pShip);
	m_State = gsInGame;

	//	Tell all objects that the player has entered the system

	pNewSystem->PlayerEntered(pShip);

	//	Save the game as a checkpoint (so that we can restore the
	//	game later if we die in this system)

	SaveGame(true);
	}

ALERROR CTranscendenceWnd::LoadOldGame (CString *retsError)

//	LoadOldGame
//
//	Load a previously saved game

	{
	ALERROR error;

	//	Old game

	CString sFilename = m_GameFile.GenerateFilename(m_sPlayerName);
	
	error = m_GameFile.Open(sFilename);
	if (error)
		{
		*retsError = strPatternSubst(CONSTLIT("Unable to open save file: %s"), sFilename.GetPointer());
		return error;
		}

	//	Load the universe

	DWORD dwSystemID, dwPlayerID;
	if ((error = m_GameFile.LoadUniverse(m_Universe, &dwSystemID, &dwPlayerID, retsError)))
		{
		m_GameFile.Close();
		return error;
		}

	//	Set debug mode

	m_Options.bDebugGame = m_Universe.InDebugMode();

	//	Load the POV system

	CSystem *pSystem;
	CShip *pPlayerShip;

	error = m_GameFile.LoadSystem(dwSystemID, 
			&pSystem, 
			dwPlayerID, 
			(CSpaceObject **)&pPlayerShip);
	if (error)
		{
		*retsError = strPatternSubst(CONSTLIT("Unable to load system"));
		m_GameFile.Close();
		return error;
		}

	//	Set the player ship

	m_pPlayer = dynamic_cast<CPlayerShipController *>(pPlayerShip->GetController());
	m_pPlayer->SetTrans(this);
	m_iPlayerGenome = m_pPlayer->GetGenome();

	//	We only need to do this for backwards compatibility (pre 0.97 this flag
	//	was not set)

	pPlayerShip->TrackMass();

	//	Set the resurrect flag (this will be cleared if we save the game
	//	properly later)

	error = m_GameFile.SetGameResurrect();
	if (error)
		{
		*retsError = strPatternSubst(CONSTLIT("Unable to set resurrect flag"));
		m_pPlayer = NULL;
		m_GameFile.Close();
		return error;
		}

	//	Define globals

	DefineGameGlobals();

	return NOERROR;
	}

void CTranscendenceWnd::LoadPreferences (void)

//	LoadPreferences
//
//	Load game preferences

	{
	CRegKey Key;

	if (CRegKey::OpenUserAppKey(REGISTRY_COMPANY_NAME,
			REGISTRY_PRODUCT_NAME,
			&Key) != NOERROR)
		{
		m_Prefs.bMusicOn = true;
		m_Prefs.bModified = false;
		m_Prefs.iSoundVolume = DEFAULT_SOUND_VOLUME;
		return;
		}

	CString sMusic;
	if (Key.FindStringValue(REGISTRY_MUSIC_OPTION, &sMusic))
		m_Prefs.bMusicOn = strEquals(sMusic, CONSTLIT("on"));
	else
		m_Prefs.bMusicOn = true;

	CString sSoundVolume;
	if (Key.FindStringValue(REGISTRY_SOUND_VOLUME_OPTION, &sSoundVolume))
		m_Prefs.iSoundVolume = Min(10, Max(0, strToInt(sSoundVolume, 10, NULL)));
	else
		m_Prefs.iSoundVolume = DEFAULT_SOUND_VOLUME;

	m_Prefs.bModified = false;
	}

void CTranscendenceWnd::PaintDebugLines (void)

//	PaintDebugLines
//
//	Paint debug output

	{
#ifdef DEBUG
	int iLine = m_iDebugLinesStart;
	int iPos = 0;
	while (iLine != m_iDebugLinesEnd)
		{
		m_Screen.DrawText(0,
				iPos++ * 12,
				m_Fonts.Medium,
				CG16bitImage::RGBValue(128,0,0),
				m_DebugLines[iLine]);

		iLine = (iLine + 1) % DEBUG_LINES_COUNT;
		}
#endif
	}

void CTranscendenceWnd::PaintFrameRate (void)

//	PaintFrameRate
//
//	Paints the frame rate

	{
	int i, j;

	if (m_iStartAnimation == 0)
		m_iStartAnimation = SDL_GetTicks();
	else
		{
		//	Figure out how much time has elapsed since the last frame

		int iNow = SDL_GetTicks();
		m_iFrameTime[m_iFrameCount % FRAME_RATE_COUNT] = iNow - m_iStartAnimation;

		//	Add up all the times

		int iTotalFrameTime = 0;
		int iTotalPaintTime = 0;
		int iTotalBltTime = 0;
		int iTotalUpdateTime = 0;
		for (i = 0; i < FRAME_RATE_COUNT; i++)
			{
			iTotalFrameTime += m_iFrameTime[i];
			iTotalPaintTime += m_iPaintTime[i];
			iTotalBltTime += m_iBltTime[i];
			iTotalUpdateTime += m_iUpdateTime[i];
			}

		//	Compute the frame rate (in 10ths of frames)

		int rFrameRate = 0;
		if (iTotalFrameTime > 0)
			rFrameRate = 10000 * FRAME_RATE_COUNT / iTotalFrameTime;

		char szBuffer[256];
		int iLen = sprintf(szBuffer, "Frames: %d   Paint: %d  Blt: %d  Update: %d",
				rFrameRate / 10,
				iTotalPaintTime / FRAME_RATE_COUNT,
				iTotalBltTime / FRAME_RATE_COUNT,
				iTotalUpdateTime / FRAME_RATE_COUNT);

		m_Screen.DrawText(300, 0, m_Fonts.Header, CG16bitImage::RGBValue(80,80,80), CString(szBuffer, iLen));

		//	Every once in a while, output to log file

		if (m_iFrameCount > FRAME_RATE_COUNT
				&& ((m_iFrameCount % 300) == 0))
			{
			kernelDebugLogMessage(szBuffer);
			int iTotalTicks[10] = {0};
			for (i = 0; i < FRAME_RATE_COUNT; i++)
				{
				for (j = 0; j < 10; j++)
					{
					iTotalTicks[j] += m_iTicks[i][j];
					}
				}
			kernelDebugLogMessage("0: %d  SRS: %d  LRS: %d  HUD: %d  DBG: %d  BLT:  %d  DONE:  %d\n", iTotalTicks[0] / FRAME_RATE_COUNT, iTotalTicks[1] / FRAME_RATE_COUNT, iTotalTicks[2] / FRAME_RATE_COUNT, iTotalTicks[3] / FRAME_RATE_COUNT, iTotalTicks[4] / FRAME_RATE_COUNT, iTotalTicks[5] / FRAME_RATE_COUNT, iTotalTicks[6] / FRAME_RATE_COUNT);
			kernelDebugLogFlush();
//			if (SaveImg(&m_Screen, "screen.png") != NOERROR || SaveImg(g_screen, "screen2.png") != NOERROR)
//				{
//				kernelDebugLogMessage("Failed to write screen.png");
//				}
			}

		//	Next

		m_iFrameCount++;
		m_iStartAnimation = iNow;
		}
	}

void CTranscendenceWnd::PlayerDestroyed (const CString &sText)

//	PlayerDestroyed
//
//	This method gets called when the player is destroyed

	{
	//	Clean up

	m_CurrentPicker = pickNone;
	m_CurrentMenu = menuNone;
	m_bAutopilot = false;
	m_bShowingMap = false;
	if (m_State == gsDocked)
		m_pPlayer->Undock();

	//	Update display

	CString sMsg = sText;
	sMsg.Capitalize(CString::capFirstLetter);
	DisplayMessage(sMsg);
	UpdateArmorDisplay();

	//	If we are insured, then set our state so that we come back to life
	
	if (m_pPlayer->IsInsured())
		{
		//	Prepare resurrect

		m_State = gsDestroyed;
		m_iCountdown = TICKS_AFTER_DESTROYED;
		}

	//	Otherwise, proceed with destruction

	else
		{
		RecordFinalScore(sText, CONSTLIT("destroyed"), false);

		//	Done with ship screens

		CleanUpPlayerShip();

		//	Note: gPlayerShip and gPlayer are clean up inside of CSystem::RemoveObject
		//	[This guarantees that we clean up the variables as soon as the objects
		//	are deleted--and no sooner. If we cleaned up the variables here, they would still
		//	be Nil inside some <OnObjDestroyed> events]

		//	Player destroyed

		m_State = gsDestroyed;
		m_iCountdown = TICKS_AFTER_DESTROYED;
		}
	}

void CTranscendenceWnd::PlayerEnteredGate (CSystem *pSystem, 
										   CTopologyNode *pDestNode,
										   const CString &sDestEntryPoint)

//	PlayerEnteredGate
//
//	This method gets called when the player enters a stargate

	{
	//	Remember the destination gate and set our state

	m_pSystem = pSystem;
	m_pDestNode = pDestNode;
	m_sDestEntryPoint = sDestEntryPoint;
	m_iCountdown = TICKS_BEFORE_GATE;
	m_State = gsEnteringStargate;
	m_bShowingMap = false;
	m_bAutopilot = false;

	//	Clear out these globals so that events don't try to send us
	//	orders (Otherwise, an event could set a target for the player. If the
	//	target is destroyed while we are out of the system, we will
	//	never get an OnObjDestroyed message).
	//	Note: We need gPlayer for OnGameEnd event

	CCodeChain &CC = m_Universe.GetCC();
	CC.DefineGlobal(STR_G_PLAYER_SHIP, CC.CreateNil());
	}

void CTranscendenceWnd::RedirectDisplayMessage (bool bRedirect)

//	RedirectDisplayMessage
//
//	Start/stop displaying messages

	{
	if (bRedirect)
		{
		m_bRedirectDisplayMessage = true;
		m_sRedirectMessage = NULL_STR;
		}
	else
		{
		m_bRedirectDisplayMessage = false;
		}
	}

void CTranscendenceWnd::RecordFinalScore (const CString &sEpitaph, const CString &sEndGameReason, bool bEscaped)

//	RecordFinalScore
//
//	Adds the score to the high-score list and fires OnGameEnd for the adventure.

	{
	ASSERT(m_pPlayer);

	CAdventureDesc *pAdventure = g_pUniverse->GetCurrentAdventureDesc();
	ASSERT(pAdventure);

	//	Add to high score list

	SGameStats FinalScore;
	FinalScore.dwAdventure = pAdventure->GetUNID();
	FinalScore.sAdventureName = pAdventure->GetName();
	FinalScore.sName = m_sPlayerName;
	FinalScore.iGenome = m_iPlayerGenome;
	FinalScore.sEndGameReason = sEndGameReason;
	FinalScore.sEpitaph = sEpitaph;
	FinalScore.sTime = m_Universe.StopGameTime().Format(CString());
	FinalScore.iSystemsVisited = m_pPlayer->GetSystemsVisited();
	FinalScore.iEnemiesDestroyed = m_pPlayer->GetEnemiesDestroyed();
	FinalScore.pBestEnemyDestroyed = m_pPlayer->GetBestEnemiesDestroyedClass();
	FinalScore.iBestEnemyDestroyedCount = m_pPlayer->GetBestEnemiesDestroyed();
	FinalScore.bEscapedHumanQuarantineZone = bEscaped;
	FinalScore.bDebugGame = m_Universe.InDebugMode();

	//	Set the ship class for the score

	CShipClass *pClass = g_pUniverse->FindShipClass(m_pPlayer->GetStartingShipClass());
	if (pClass)
		FinalScore.sShipClass = pClass->GetName();
	else
		FinalScore.sShipClass = CONSTLIT("Unknown");

	//	The score is adjusted based on the number of times that we
	//	resurrected.

	FinalScore.iResurrectCount = m_GameFile.GetResurrectCount();
	int iResurrectAdj = 1 + std::min(9, m_GameFile.GetResurrectCount());
	FinalScore.iScore = m_pPlayer->GetScore() / iResurrectAdj;

	//	Add to high score if this is the default adventure

	if (pAdventure->GetUNID() == DEFAULT_ADVENTURE_UNID)
		{
		m_iLastHighScore = m_pHighScoreList->AddEntry(FinalScore);
		m_pHighScoreList->Save(HIGH_SCORES_FILENAME);
		}
	else
		m_iLastHighScore = -1;

	//	Fire events

	m_dwCrawlImage = 0;
	m_sCrawlText = NULL_STR;
	pAdventure->FireOnGameEnd(FinalScore);
	}

void CTranscendenceWnd::ReportCrash (void)

//	ReportCrash
//
//	Report a crash

	{
#ifdef DEBUG_PROGRAMSTATE
	CString sMessage = CONSTLIT("Unable to continue due to program error\r\n\r\n");

	//	Program state

	switch (g_iProgramState)
		{
		case psAnimating:
			sMessage.Append(CONSTLIT("program state: animating\r\n"));
			break;

		case psPaintingSRS:
			sMessage.Append(CONSTLIT("program state: painting SRS\r\n"));
			break;

		case psUpdating:
			sMessage.Append(CONSTLIT("program state: updating universe\r\n"));
			break;

		case psUpdatingEvents:
			sMessage.Append(CONSTLIT("program state: updating events\r\n"));
			ReportCrashEvent(&sMessage);
			break;

		case psUpdatingBehavior:
			sMessage.Append(CONSTLIT("program state: updating object behavior\r\n"));
			ReportCrashObj(&sMessage);
			break;

		case psUpdatingObj:
			sMessage.Append(CONSTLIT("program state: updating object\r\n"));
			ReportCrashObj(&sMessage);
			break;

		case psUpdatingMove:
			sMessage.Append(CONSTLIT("program state: updating object motion\r\n"));
			ReportCrashObj(&sMessage);
			break;

		case psUpdatingEncounters:
			sMessage.Append(CONSTLIT("program state: updating random encounters\r\n"));
			break;

		case psDestroyNotify:
			sMessage.Append(CONSTLIT("program state: handling OnObjDestroy\r\n"));
			ReportCrashObj(&sMessage);
			break;

		case psUpdatingReactorDisplay:
			sMessage.Append(CONSTLIT("program state: updating reactor display\r\n"));
			break;

		case psPaintingMap:
			sMessage.Append(CONSTLIT("program state: painting map\r\n"));
			break;

		case psPaintingLRS:
			sMessage.Append(CONSTLIT("program state: painting LRS\r\n"));
			break;

		case psPaintingArmorDisplay:
			sMessage.Append(CONSTLIT("program state: painting armor display\r\n"));
			break;

		case psPaintingMessageDisplay:
			sMessage.Append(CONSTLIT("program state: painting message display\r\n"));
			break;

		case psPaintingReactorDisplay:
			sMessage.Append(CONSTLIT("program state: painting reactor display\r\n"));
			break;

		case psPaintingTargetDisplay:
			sMessage.Append(CONSTLIT("program state: painting target display\r\n"));
			ReportCrashObj(&sMessage, (m_pPlayer ? m_pPlayer->GetShip() : NULL));
			break;

		case psPaintingDeviceDisplay:
			sMessage.Append(CONSTLIT("program state: painting device display\r\n"));
			break;

		case psStargateEnter:
			sMessage.Append(CONSTLIT("program state: entering stargate"));
			break;

		case psStargateEndGame:
			sMessage.Append(CONSTLIT("program state: entering end-game stargate"));
			break;

		case psStargateLoadingSystem:
			sMessage.Append(CONSTLIT("program state: enter stargate, loading destination system"));
			break;

		case psStargateCreatingSystem:
			sMessage.Append(CONSTLIT("program state: enter stargate, creating destination system"));
			break;

		case psStargateUpdateExtended:
			sMessage.Append(CONSTLIT("program state: enter stargate, updating extended"));
			break;

		case psStargateTransferringGateFollowers:
			sMessage.Append(CONSTLIT("program state: enter stargate, transferring gate followers"));
			break;

		case psStargateSavingSystem:
			sMessage.Append(CONSTLIT("program state: enter stargate, saving current system"));
			break;

		case psStargateFlushingSystem:
			sMessage.Append(CONSTLIT("program state: enter stargate, flushing current system"));
			break;

		case psStargateGarbageCollecting:
			sMessage.Append(CONSTLIT("program state: enter stargate, garbage-collecting images"));
			break;

		case psStargateLoadingBitmaps:
			sMessage.Append(CONSTLIT("program state: enter stargate, loading new images"));
			break;

		case psStargateEnterDone:
			sMessage.Append(CONSTLIT("program state: enter stargate done"));
			break;

		default:
			sMessage.Append(CONSTLIT("program state: unknown\r\n"));
		}

	//	Game state

	switch (m_State)
		{
		case gsInGame:
			sMessage.Append(CONSTLIT("game state: in game\r\n"));
			break;

		case gsDocked:
			sMessage.Append(CONSTLIT("game state: docked\r\n"));
			break;

		case gsDestroyed:
			sMessage.Append(CONSTLIT("game state: destroyed\r\n"));
			break;

		case gsEnteringStargate:
			sMessage.Append(CONSTLIT("game state: entering stargate\r\n"));
			break;

		case gsLeavingStargate:
			sMessage.Append(CONSTLIT("game state: leaving stargate\r\n"));
			break;

		case gsIntro:
			sMessage.Append(CONSTLIT("game state: intro\r\n"));
			break;

		case gsSelectAdventure:
			sMessage.Append(CONSTLIT("game state: select adventure\r\n"));
			break;

		case gsSelectShip:
			sMessage.Append(CONSTLIT("game state: select ship\r\n"));
			break;

		case gsProlog:
			sMessage.Append(CONSTLIT("game state: prolog\r\n"));
			break;

		case gsHelp:
			sMessage.Append(CONSTLIT("game state: in help\r\n"));
			sMessage.Append(strPatternSubst(CONSTLIT("old state: %d\r\n"), m_OldState));
			break;

		case gsEpilog:
			sMessage.Append(CONSTLIT("game state: epilog\r\n"));
			break;

		case gsLoading:
			sMessage.Append(CONSTLIT("game state: loading\r\n"));
			break;

		default:
			sMessage.Append(CONSTLIT("game state: unknown\r\n"));
		}

#endif

	throw;
	}

void CTranscendenceWnd::ReportCrashEvent (CString *retsMessage)

//	ReportCrashEvent
//
//	Include information about the given event

	{
	try
		{
		retsMessage->Append(g_pProgramEvent->DebugCrashInfo());
		}
	catch (...)
		{
		retsMessage->Append(strPatternSubst(CONSTLIT("error obtaining crash info from event: %x\r\n"), g_pProgramEvent));
		}
	}

ALERROR CTranscendenceWnd::RestartGame (void)

//	RestartGame
//
//	Restarts the game

	{
	ALERROR error;

	m_GameFile.Close();

	error = m_Universe.Reinit();
	if (error)
		return error;

	StartIntro();

	return NOERROR;
	}

ALERROR CTranscendenceWnd::SaveGame (bool bCheckpoint)

//	SaveGame
//
//	Saves the game to a file

	{
	ALERROR error;

	//	Save the current system

	CSystem *pSystem = m_pPlayer->GetShip()->GetSystem();
	error = m_GameFile.SaveSystem(pSystem->GetID(), pSystem);
	if (error)
		{
		kernelDebugLogMessage("Error saving system '%s' to game file", m_pSystem->GetName().GetASCIIZPointer());
		return error;
		}

	//	Save the universe
	error = m_GameFile.SaveUniverse(m_Universe, bCheckpoint);
	if (error)
		{
		kernelDebugLogMessage("Error saving universe to game file");
		return error;
		}

	return NOERROR;
	}

void CTranscendenceWnd::SavePreferences (void)

//	SavePreferences
//
//	Save preferences to the registry

	{
	if (m_Prefs.bModified)
		{
		CRegKey Key;

		if (CRegKey::OpenUserAppKey(REGISTRY_COMPANY_NAME,
				REGISTRY_PRODUCT_NAME,
				&Key) != NOERROR)
			return;

		Key.SetStringValue(REGISTRY_MUSIC_OPTION,
				(m_Prefs.bMusicOn ? CONSTLIT("on") : CONSTLIT("off")));

		Key.SetStringValue(REGISTRY_SOUND_VOLUME_OPTION,
				strFromInt(m_Prefs.iSoundVolume, FALSE));

		m_Prefs.bModified = false;
		}
	}

void CTranscendenceWnd::SetDebugGame (bool bDebugMode)

//	SetDebugGame
//
//	Sets DebugGame on and off

	{
	m_Options.bDebugGame = bDebugMode;
	m_Universe.SetDebugMode(bDebugMode);
	}

void CTranscendenceWnd::SetMusicOption (bool bMusicOn)

//	SetMusicOption
//
//	Turns music on and off

	{
	if (bMusicOn != m_Prefs.bMusicOn)
		{
		//	Turn music on or off

		if (bMusicOn)
			m_SoundMgr.PlayMusic(CONSTLIT("TranscendenceMarch.mp3"));
		else
			m_SoundMgr.StopMusic();

		m_Prefs.bMusicOn = bMusicOn;
		m_Prefs.bModified = true;
		}
	}

void CTranscendenceWnd::SetPlayerGenome (int iGenome)

//	SetPlayerGenome
//
//	Sets the player genome

	{
	m_iPlayerGenome = iGenome;
	}

void CTranscendenceWnd::SetPlayerName (const CString &sName)

//	SetPlayerName
//
//	Sets the name of the player

	{
	m_sPlayerName = sName;

	//	If we're in the intro screen then we need to recompute the buttons
	//	because the load game button depends on the name of the player

	if (m_State == gsIntro)
		{
		CString sContinueLabel = ComputeLoadGameLabel(&m_bSavedGame);
		if (sContinueLabel.IsBlank())
			m_ButtonBar.SetVisible(0, false);
		else
			{
			m_ButtonBar.SetVisible(0, true);
			m_ButtonBar.SetDescription(0, sContinueLabel);
			}
		m_ButtonBarDisplay.Invalidate();
		}
	}

void CTranscendenceWnd::SetPlayerShip (DWORD dwUNID)

//	SetPlayerShip
//
//	Sets the ship that the player will use

	{
	m_dwPlayerShip = dwUNID;
	}

void CTranscendenceWnd::ShowErrorMessage (const CString &sError)

//	ShowErrorMessage
//
//	Shows an error message box

	{
	::kernelDebugLogMessage(sError.GetASCIIZPointer());
	}

ALERROR CTranscendenceWnd::StartGame (void)

//	StartGame
//
//	Start the game (either a new game or a previously saved game)

	{
	ALERROR error;
	int i;

	//	Tell the universe to focus on the ship

	m_Universe.SetPOV(m_pPlayer->GetShip());

	//	Load images necessary for the system

	g_pUniverse->LoadLibraryBitmaps();

	//	Set sound

	if (m_Options.bNoSound)
		m_Universe.SetSound(false);

	//	Initialize sub-displays

	error = InitDisplays();
	if (error)
		return error;

	//	Set map state

	m_bShowingMap = false;
	m_iMapScale = 1;
	m_iMapZoomEffect = 0;
	for (i = 0; i < MAP_SCALE_COUNT; i++)
		if (i == 0)
			m_rMapScale[i] = g_AU / 800.0;
		else
			m_rMapScale[i] = m_rMapScale[i - 1] * 2.0;

	//	Set the state appropriately

	m_State = gsInGame;
	m_bPaused = false;
	m_bPausedStep = false;
	m_bDebugConsole = false;
	m_bAutopilot = false;
	m_CurrentPicker = pickNone;
	m_CurrentMenu = menuNone;
	m_iTick = 0;
	m_MessageDisplay.ClearAll();
	m_MessageDisplay.DisplayMessage(CONSTLIT("Welcome to Transcendence!"), m_Fonts.wTitleColor);

	//	Clear the POVLRS flag for all objects (so that we don't get the
	//	"Enemy Ships Detected" message when entering a system

	m_pPlayer->GetShip()->GetSystem()->SetPOVLRS(m_pPlayer->GetShip());

	return NOERROR;
	}

ALERROR CTranscendenceWnd::StartNewGame (const SNewGameSettings &NewGame, CString *retsError)

//	StartNewGame
//
//	Initializes:
//		m_Game
//		m_Universe
//		m_pPlayer

	{
	//	New game

	CString sFilename = m_GameFile.GenerateFilename(NewGame.sPlayerName);
	m_GameFile.Create(sFilename);

	//	Set the adventure

	CAdventureDesc *pAdventure = g_pUniverse->FindAdventureDesc(NewGame.dwAdventure);
	if (pAdventure == NULL)
		{
		*retsError = CONSTLIT("Unable to find adventure desc");
		m_Universe.Reinit();
		m_GameFile.Close();
		return ERR_FAIL;
		}

	g_pUniverse->SetCurrentAdventureDesc(pAdventure);

	//	Create a controller for the player's ship (this is owned
	//	by the ship once we pass it to CreateShip)

	m_pPlayer = new CPlayerShipController;
	if (m_pPlayer == NULL)
		{
		*retsError = CONSTLIT("Unable to create CPlayerShipController");
		m_Universe.Reinit();
		m_GameFile.Close();
		return ERR_MEMORY;
		}

	m_pPlayer->SetTrans(this);
	m_pPlayer->SetGenome(NewGame.iPlayerGenome);
	m_pPlayer->SetStartingShipClass(NewGame.dwPlayerShip);
	m_pPlayer->IncSystemsVisited();

	//	Define globals for OnGameStart (only gPlayer is defined)

	DefineGameGlobals();

	//	Invoke Adventure OnGameStart

	m_dwCrawlImage = 0;
	m_sCrawlText = NULL_STR;
	pAdventure->FireOnGameStart();

	//	The remainder of new game start happens in the background thread
	//	in StartNewGamebackground

	return NOERROR;
	}

ALERROR CTranscendenceWnd::StartNewGameBackground (CString *retsError)

//	StartNewGameBackground
//
//	Completes the work started in StartNewGame

	{
	ALERROR error;

	ASSERT(m_pPlayer);

	//	Figure out the ship class that we want

	CShipClass *pStartingShip = g_pUniverse->FindShipClass(m_pPlayer->GetStartingShipClass());
	const SPlayerSettings *pPlayerSettings = pStartingShip->GetPlayerSettings();
	if (pPlayerSettings == NULL)
		{
		*retsError = CONSTLIT("Missing <PlayerSettings> for ship class");
		return ERR_FAIL;
		}

	CString sStartNode = pPlayerSettings->m_sStartNode;
	CString sStartPos = pPlayerSettings->m_sStartPos;

	//	Get the starting system

	CSystem *pSystem;

	error = m_Universe.CreateFirstStarSystem(sStartNode, &pSystem, retsError);
	if (error)
		return error;

	//	Figure out where in the system we want to start

	CSpaceObject *pStart = pSystem->GetNamedObject(sStartPos);
	ASSERT(pStart);

	//	Set some credits

	m_pPlayer->Payment(pStartingShip->GetPlayerSettings()->m_StartingCredits.Roll());

	//	Create the player's ship

	CShip *pPlayerShip;

	error = pSystem->CreateShip(m_pPlayer->GetStartingShipClass(),
			m_pPlayer,
			m_Universe.FindSovereign(g_PlayerSovereignUNID),
			pStart->GetPos(),
			NullVector,
			90,
			NULL,
			&pPlayerShip);
	if (error)
		{
		*retsError = CONSTLIT("Unable to create player ship");
		m_pPlayer = NULL;
		return error;
		}

	//	Ship automatically gets autopilot

	pPlayerShip->TrackFuel();
	pPlayerShip->TrackMass();
	pPlayerShip->InstallAutopilot();
#ifdef DEBUG_SHIP
	pPlayerShip->InstallTargetingComputer();
	pPlayerShip->SetSRSEnhanced();
#endif

	//	Associate with the controller

	m_pPlayer->SetShip(pPlayerShip);

	//	All items on the ship are automatically 
	//	known to the player

	CItemListManipulator ItemList(pPlayerShip->GetItemList());
	while (ItemList.MoveCursorForward())
		{
		CItemType *pType = ItemList.GetItemAtCursor().GetType();
		pType->SetKnown();
		pType->SetShowReference();
		}

#ifdef DEBUG_ALL_ITEMS
	if (m_Options.bDebugGame)
		{
		for (int i = 0; i < g_pUniverse->GetItemTypeCount(); i++)
			{
			CItemType *pType = g_pUniverse->GetItemType(i);

			//	Do not count unknown items

			if (pType->HasModifier(CONSTLIT("unknown")))
				continue;

			//	Do not count virtual items

			if (pType->IsVirtual())
				continue;

			//	Add item

			CItem Item(pType, 1);
			ItemList.AddItem(Item);

			pType->SetKnown();
			pType->SetShowReference();
			}
		}
#endif

#ifdef DEBUG_HENCHMAN
	if (m_Options.bDebugGame)
		{
		CShip *pHenchman;

		for (int i = 0; i < 14; i++)
			{
			CFleetShipAI *pController = new CFleetShipAI;
			if (error = pSystem->CreateShip(g_DebugHenchmenShipUNID,
						pController,
						m_Universe.FindSovereign(g_PlayerSovereignUNID),
						pStart->GetPos() + PolarToVector(0, g_KlicksPerPixel * 100.0),
						NullVector,
						0,
						NULL,
						&pHenchman))
				{
				*retsError = CONSTLIT("Unable to create henchmen");
				return error;
				}

			pHenchman->GetController()->AddOrder(IShipController::orderEscort, pPlayerShip, i);
			}
		}
#endif

	//	Define globals (now with gPlayerShip defined)

	DefineGameGlobals();

	//	Clock is running

	m_Universe.StartGameTime();

	return NOERROR;
	}

LONG CTranscendenceWnd::WMActivateApp (bool bActivate)

//	WMActivateApp
//
//	Handle WM_ACTIVATEAPP

	{
	return 0;
	}

LONG CTranscendenceWnd::WMChar (char chChar, DWORD dwKeyData)

//	WMChar
//
//	Handle WM_CHAR

	{
	//	If we already processed the keydown, then skip it

	if (m_chKeyDown 
			&& ((chChar == m_chKeyDown) || (chChar == (m_chKeyDown + ('a' - 'A')))))
		{
		m_chKeyDown = '\0';
		return 0;
		}
	else
		m_chKeyDown = '\0';

	//	Handle key

	switch (m_State)
		{
		case gsInGame:
			{
			//	Handle debug console

			if (m_bDebugConsole)
				{
				if (chChar >= SDLK_SPACE)
					{
					CString sKey = CString(&chChar, 1);
					m_DebugConsole.Input(sKey);
					}
				return 0;
				}

			//	If we're paused, then check for unpause key

			if (m_bPaused)
				{
				if (chChar == SDLK_SPACE)
					m_bPausedStep = true;
				else
					{
					m_bPaused = false;
					DisplayMessage(CONSTLIT("Game continues"));
					}
				return 0;
				}

			//	If we're in a menu, handle it

			if (m_CurrentMenu != menuNone)
				{
				CString sKey = CString(&chChar, 1);
				DWORD dwData, dwData2;
				if (m_MenuData.FindItemData(sKey, &dwData, &dwData2))
					{
					switch (m_CurrentMenu)
						{
						case menuGame:
							DoGameMenuCommand(dwData);
							break;

						case menuSelfDestructConfirm:
							DoSelfDestructConfirmCommand(dwData);
							break;

						case menuOrders:
							DoOrdersCommand((MessageTypes)dwData, dwData2);
							break;

						case menuComms:
							DoCommsMenu(dwData);
							break;

						case menuCommsTarget:
							ShowCommsMenu((CSpaceObject *)dwData);
							break;

						case menuInvoke:
							DoInvocation((CPower *)dwData);
							break;
						default:
							break;
						}
					}
				return 0;
				}

			//	If we're in a picker, handle it

			if (m_CurrentPicker != pickNone)
				{
				CString sKey = CString(&chChar, 1);
				DWORD dwData;
				bool bHotKey = m_MenuData.FindItemData(sKey, &dwData);

				if (chChar == SDLK_RIGHTBRACKET)
					m_PickerDisplay.SelectNext();
				else if (chChar == SDLK_LEFTBRACKET)
					m_PickerDisplay.SelectPrev();
				else if (bHotKey)
					{
					switch (m_CurrentPicker)
						{
						case pickUsableItem:
							DoUseItemCommand(dwData);
							break;
						default:
							break;
						}
					}

				return 0;
				}

			break;
			}

		case gsIntro:
			OnCharIntro(chChar, dwKeyData);
			break;

		case gsSelectAdventure:
			OnCharSelectAdventure(chChar, dwKeyData);
			break;

		case gsSelectShip:
			OnCharSelectShip(chChar, dwKeyData);
			break;

		case gsDestroyed:
			{
			switch (chChar)
				{
				case SDLK_SPACE:
					EndDestroyed();
					break;
				}
			break;
			}

		case gsDocked:
			m_CurrentDock.HandleChar(chChar);
			break;
		default:
			break;
		}

	return 0;
	}

LONG CTranscendenceWnd::WMClose (void)

//	WMClose
//
//	Handle WM_CLOSE

	{
	//	Do not allow the user to close if we're entering or leaving
	//	a stargate (because we are too lazy to handle saving a game in the
	//	middle of a gate).

	if (m_State == gsEnteringStargate || m_State == gsLeavingStargate)
		return 0;

	//	If we have a loading error, display it now

	if (!m_sBackgroundError.IsBlank())
		{
		CleanUpDirectDraw();
		}

	//	Destroy the window
	return 1;
	}

LONG CTranscendenceWnd::WMCreate (SCommandLineOptions *cmdOpts, CString *retsError)

//	WMCreate
//
//	Handle WM_CREATE

	{
	ALERROR error;
	int i;
	RECT rcRect;

	//	Set our default directory

	CString sCurDir = pathGetExecutablePath();

	//	Process command line options

	m_Options = *cmdOpts;

	//	Initialize log file

#ifdef DEBUGLOG
	if (!m_Options.bNoDebugLog)
		{
		if ((error = kernelSetDebugLog(&m_Log, TRUE)))
			{
			*retsError = CONSTLIT("Unable to initialize debug log file.");
			goto Fail;
			}
		}
#endif

	//	Get the version information

	{
	SFileVersionInfo VerInfo;
	fileGetVersionInfo(NULL_STR, &VerInfo);
	m_sVersion = strPatternSubst(CONSTLIT("%s %s"), "transport", "1.1");
	m_sCopyright = strPatternSubst(CONSTLIT("%s"), "Copyright Forever");
	kernelDebugLogMessage(m_sVersion.GetASCIIZPointer());
	}

	//	Load preferences

	LoadPreferences();

	//	Initialize DirectSound (note that we need to initialize before we start
	//	loading the universe).
	if ((error = m_SoundMgr.Init()))
		{
		//	Report error, but don't abort

		kernelDebugLogMessage("Unable to initialize sound manager.");
		}

	m_SoundMgr.SetWaveVolume(m_Prefs.iSoundVolume);
	//	Compute screen size

	ComputeScreenSize();

	//	Initialize DirectDraw

	if (m_Options.bDirectX)
		{
		if ((error = InitDirectDraw(g_iColorDepth)))
			{
			*retsError = CONSTLIT("Unable to initialize DirectDraw. Make sure you have DirectX 7.0 or later installed properly.");
			goto Fail;
			}
		}

	//	Initialize frame rate information

	m_iStartAnimation = 0;
	m_iFrameCount = 0;
	for (i = 0; i < FRAME_RATE_COUNT; i++)
		{
		m_iFrameTime[i] = 0;
		m_iPaintTime[i] = 0;
		m_iUpdateTime[i] = 0;
		m_iBltTime[i] = 0;
		}

	//	Initialize

	m_rcScreen.left = 0;
	m_rcScreen.top = 0;
	m_rcScreen.right = g_cxScreen;
	m_rcScreen.bottom = g_cyScreen;

	m_rcMainScreen.left = (g_cxScreen - MAIN_SCREEN_WIDTH) / 2;
	m_rcMainScreen.top = (g_cyScreen - MAIN_SCREEN_HEIGHT) / 2;
	m_rcMainScreen.right = m_rcMainScreen.left + MAIN_SCREEN_WIDTH;
	m_rcMainScreen.bottom = m_rcMainScreen.top + MAIN_SCREEN_HEIGHT;

	//	Initialize the font table

	m_Fonts.Small.Create(STR_SMALL_TYPEFACE, 10);
	m_Fonts.Medium.Create(STR_SMALL_TYPEFACE, 13);
	m_Fonts.MediumBold.Create(STR_SMALL_TYPEFACE, 13, true);
	m_Fonts.MediumHeavyBold.Create(STR_MEDIUM_TYPEFACE, 14, true);
	m_Fonts.Large.Create(STR_MEDIUM_TYPEFACE, 16);
	m_Fonts.LargeBold.Create(STR_MEDIUM_TYPEFACE, 16, true);
	m_Fonts.Header.Create(STR_LARGE_TYPEFACE, 19);
	m_Fonts.HeaderBold.Create(STR_LARGE_TYPEFACE, 19, true);
	m_Fonts.SubTitle.Create(STR_LARGE_TYPEFACE, 26);
	m_Fonts.SubTitleBold.Create(STR_LARGE_TYPEFACE, 26, true);
	m_Fonts.SubTitleHeavyBold.Create(STR_LARGE_TYPEFACE, 28, true);
	m_Fonts.Title.Create(STR_LARGE_TYPEFACE, 48);
	m_Fonts.Console.Create(STR_FIXED_TYPEFACE, 14);

	//	Set colors

	m_Fonts.wTextColor = CG16bitImage::RGBValue(191,196,201);
	m_Fonts.wTitleColor = CG16bitImage::RGBValue(218,235,255);
	m_Fonts.wHelpColor = CG16bitImage::RGBValue(103,114,128);
	m_Fonts.wBackground = CG16bitImage::RGBValue(15,17,18);
	m_Fonts.wSectionBackground = CG16bitImage::RGBValue(86,82,73);
	m_Fonts.wSelectBackground = CG16bitImage::RGBValue(115,230,115);
	//m_Fonts.wSelectBackground = CG16bitImage::RGBValue(255,225,103);

	m_Fonts.wAltGreenColor = CG16bitImage::RGBValue(5,211,5);
	m_Fonts.wAltGreenBackground = CG16bitImage::RGBValue(23,77,23);
	m_Fonts.wAltYellowColor = CG16bitImage::RGBValue(255,225,103);
	m_Fonts.wAltYellowBackground = CG16bitImage::RGBValue(65,57,24);
	m_Fonts.wAltRedColor = CG16bitImage::RGBValue(4,179,4);
	m_Fonts.wAltRedBackground = CG16bitImage::RGBValue(76,0,0);
	m_Fonts.wAltBlueColor = CG16bitImage::RGBValue(87,111,205);
	m_Fonts.wAltBlueBackground = CG16bitImage::RGBValue(52,57,64);

	//	Play Intro Music
	if (GetMusicOption())
		m_SoundMgr.PlayMusic(CONSTLIT("TranscendenceMarch.mp3"));
	//	Initialize debug console

	rcRect.left = m_rcScreen.right - (DEBUG_CONSOLE_WIDTH + 4);
	rcRect.top = (RectHeight(m_rcScreen) - DEBUG_CONSOLE_HEIGHT) / 2;
	rcRect.right = rcRect.left + DEBUG_CONSOLE_WIDTH;
	rcRect.bottom = rcRect.top + DEBUG_CONSOLE_HEIGHT;
	m_DebugConsole.SetFontTable(&m_Fonts);
	m_DebugConsole.Init(this, rcRect);

	m_DebugConsole.Output(CONSTLIT("Transcendence Debug Console"));
	m_DebugConsole.Output(m_sVersion);
	m_DebugConsole.Output(m_sCopyright);
	m_DebugConsole.Output(NULL_STR);

	//	Start loading

	if ((error = StartLoading()))
		{
		*retsError = CONSTLIT("Unable to create loading thread.");
		goto Fail;
		}

	//	Hide the cursor

	SDL_ShowCursor(false);

	return 0;

Fail:

	CleanUpDirectDraw();
	m_SoundMgr.CleanUp();
	if (retsError)
		kernelDebugLogMessage("Startup error %d: %s", error,
				(*retsError).GetPointer());
	else
		kernelDebugLogMessage("Startup error: %d", error);

#ifdef DEBUGLOG
	kernelSetDebugLog(NULL, FALSE);
#endif

	return -1;
	}

LONG CTranscendenceWnd::WMDestroy (void)

//	WMDestroy
//
//	Handle WM_DESTROY

	{
	m_SoundMgr.StopMusic();
	//	Cancel help state

	if (m_State == gsHelp)
		StopHelp();

	//	If we're docked, undock first

	if (m_State == gsDocked)
		m_pPlayer->Undock();

	//	Clean up state

	switch (m_State)
		{
		case gsInGame:
			{
			SaveGame(false);
			m_GameFile.Close();
			break;
			}

		case gsEnteringStargate:
		case gsLeavingStargate:
		case gsDestroyed:
			m_GameFile.Close();
			break;
		default:
			break;
		}

	//	Clean up the rest
	m_SoundMgr.CleanUp();
	CleanUpDirectDraw();

	//	Clean up other stuff

	if (m_pHighScoreList)
		{
		//	Save the last name and genome

		m_pHighScoreList->SetMostRecentPlayerName(m_sPlayerName);
		m_pHighScoreList->SetMostRecentPlayerGenome(m_iPlayerGenome);
		m_pHighScoreList->Save(HIGH_SCORES_FILENAME);

		delete m_pHighScoreList;
		m_pHighScoreList = NULL;
		}

	//	Save preferences

	SavePreferences();

	//	Kill the background thread

	DestroyBackgroundThread();

	//	Clean up debug console

	m_DebugConsole.CleanUp();

	//	Close the debug log

#ifdef DEBUGLOG
	kernelSetDebugLog(NULL, FALSE);
#endif

	return 0;
	}

LONG CTranscendenceWnd::WMDisplayChange (int iBitDepth, int cxWidth, int cyHeight)

//	WMDisplayChange
//
//	Handle WM_DISPLAYCHANGE

	{
	return 0;
	}

LONG CTranscendenceWnd::WMKeyDown (int iVirtKey, DWORD dwKeyData)

//	WMKeyDown
//
//	Handle WM_KEYDOWN

	{
	switch (m_State)
		{
		case gsInGame:
			{
			if (m_pPlayer == NULL)
				return 0;

			//	Deal with console

			if (m_bDebugConsole)
				{
				switch (iVirtKey)
					{
					case SDLK_BACKSPACE:
						m_DebugConsole.InputBackspace();
						break;

					case SDLK_ESCAPE:
						m_bDebugConsole = false;
						break;

					case SDLK_RETURN:
						{
						CString sInput = m_DebugConsole.GetInput();
						if (!sInput.IsBlank())
							{
							m_DebugConsole.InputEnter();

							CCodeChain &CC = g_pUniverse->GetCC();
							ICCItem *pCode = CC.Link(sInput, 0, NULL);
							ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
							CString sOutput = CC.Unlink(pResult);
							pResult->Discard(&CC);
							pCode->Discard(&CC);

							m_DebugConsole.Output(sOutput);
							}
						break;
						}
					}

				return 0;
				}

			//	If we're paused, then check for unpause key

			if (m_bPaused)
				{
				/* ??? Capitals and lower cases need clarification. */
				if ((iVirtKey < SDLK_a || iVirtKey > SDLK_z) && iVirtKey != SDLK_SPACE && iVirtKey != SDLK_F9)
					{
					m_bPaused = false;
					DisplayMessage(CONSTLIT("Game continues"));
					return 0;
					}
				else if (iVirtKey == SDLK_F9)
					//	Passthrough so that we can invoke the debug console
					;
				else
					return 0;
				}

			//	Handle menu

			if (m_CurrentMenu != menuNone
					&& iVirtKey >= SDLK_a && iVirtKey <= SDLK_z)
				return 0;

			//	Handle picker

			if (m_CurrentPicker != pickNone)
				{
				if (iVirtKey == SDLK_RETURN)
					{
					switch (m_CurrentPicker)
						{
						case pickUsableItem:
							DoUseItemCommand(m_MenuData.GetItemData(m_PickerDisplay.GetSelection()));
							break;

						case pickEnableDisableItem:
							m_pPlayer->ClearShowHelpEnableDevice();
							DoEnableDisableItemCommand(m_MenuData.GetItemData(m_PickerDisplay.GetSelection()));
							break;
						default:
							break;
						}

					return 0;
					}
				else if (iVirtKey == SDLK_LEFT)
					{
					m_PickerDisplay.SelectPrev();
					return 0;
					}
				else if (iVirtKey == SDLK_RIGHT)
					{
					m_PickerDisplay.SelectNext();
					return 0;
					}
				else if (iVirtKey >= SDLK_a && iVirtKey <= SDLK_z)
					return 0;
				}

			//	Other commands

			switch (iVirtKey)
				{
				case SDLK_a:
					Autopilot(!m_bAutopilot);
					m_pPlayer->ClearShowHelpAutopilot();
					m_chKeyDown = iVirtKey;
					break;

				case SDLK_b:
					if (!m_pPlayer->DockingInProgress() 
							&& !m_pPlayer->GetShip()->IsOutOfFuel())
						{
						Autopilot(false);
						ShowEnableDisablePicker();
						}
					m_chKeyDown = iVirtKey;
					break;

				case SDLK_c:
					if (!m_pPlayer->DockingInProgress())
						{
						Autopilot(false);
						ShowCommsTargetMenu();
						}
					m_chKeyDown = iVirtKey;
					break;

				case SDLK_d:
					if (!m_pPlayer->GetShip()->IsOutOfFuel())
						{
						Autopilot(false);
						m_pPlayer->Dock();
						}
					m_chKeyDown = iVirtKey;
					break;

				case SDLK_f:
					m_pPlayer->SelectNextFriendly();
					m_chKeyDown = iVirtKey;
					break;

				case SDLK_g:
					if (!m_pPlayer->DockingInProgress()
							&& !m_pPlayer->GetShip()->IsOutOfFuel())
						{
						Autopilot(false);
						m_pPlayer->Gate();
						}
					m_chKeyDown = iVirtKey;
					break;

				case SDLK_h:
					if (m_bShowingMap)
						m_pPlayer->SetMapHUD(!m_pPlayer->IsMapHUDActive());
					break;

				case SDLK_i:
					if (!m_pPlayer->DockingInProgress())
						{
						Autopilot(false);
						ShowInvokeMenu();
						}
					m_chKeyDown = iVirtKey;
					break;

				case SDLK_m:
					if (m_bShowingMap)
						Autopilot(false);
					m_bShowingMap = !m_bShowingMap;
					m_pPlayer->ClearShowHelpMap();
					m_chKeyDown = iVirtKey;
					break;

				case SDLK_p:
					{
					m_bPaused = true;;
					if (m_pPlayer)
						{
						m_pPlayer->SetThrust(false);
						m_pPlayer->SetManeuver(IShipController::NoRotation);
						m_pPlayer->SetFireMain(false);
						m_pPlayer->SetFireMissile(false);
						}
					DisplayMessage(CONSTLIT("Game paused"));
					m_chKeyDown = iVirtKey;
					break;
					}

				case SDLK_q:
					if (!m_pPlayer->DockingInProgress())
						{
						Autopilot(false);
						ShowOrdersMenu();
						}
					m_chKeyDown = iVirtKey;
					break;

				case SDLK_r:
					m_pPlayer->SetTarget(NULL);
					m_chKeyDown = iVirtKey;
					break;

				case SDLK_s:
					if (!m_pPlayer->DockingInProgress())
						m_pPlayer->Cargo();
					m_chKeyDown = iVirtKey;
					break;

				case SDLK_t:
					m_pPlayer->SelectNextTarget();
					m_chKeyDown = iVirtKey;
					break;

				case SDLK_u:
					if (!m_pPlayer->DockingInProgress())
						{
						Autopilot(false);
						ShowUsePicker();
						}
					m_chKeyDown = iVirtKey;
					break;

				case SDLK_w:
					if (!m_bNextWeaponKey)
						{
						Autopilot(false);
						m_pPlayer->ReadyNextWeapon();
						UpdateWeaponStatus();
						m_chKeyDown = iVirtKey;
						m_bNextWeaponKey = true;
						}
					break;

				case SDLK_MINUS:
					if (m_bShowingMap && m_iMapScale < (MAP_SCALE_COUNT - 1))
						{
						m_iMapScale++;
						m_iMapZoomEffect = 100;
						}
					break;

				case SDLK_PLUS:
				case SDLK_EQUALS:
					if (m_bShowingMap && m_iMapScale > 0)
						{
						m_iMapScale--;
						m_iMapZoomEffect = -100;
						}
					break;

				case SDLK_ESCAPE:
					if (m_CurrentPicker != pickNone)
						m_CurrentPicker = pickNone;
					else if (m_CurrentMenu != menuNone)
						m_CurrentMenu = menuNone;
					else if (m_bShowingMap)
						m_bShowingMap = false;
					else if (m_bAutopilot)
						Autopilot(false);
					else if (m_CurrentMenu == menuNone)
						ShowGameMenu();
					else if (m_bDebugConsole)
						m_bDebugConsole = false;
					break;

				case SDLK_DOWN:
				case SDLK_UP:
				case SDLK_k:
					if (!m_pPlayer->GetShip()->IsOutOfFuel())
						{
						Autopilot(false);
						m_pPlayer->SetThrust(true);
						}
					break;

				case SDLK_LEFT:
				case SDLK_j:
					if (!m_pPlayer->GetShip()->IsOutOfFuel())
						{
						Autopilot(false);
						m_pPlayer->SetManeuver(IShipController::RotateLeft);
						}
					break;

				case SDLK_RIGHT:
				case SDLK_l:
					if (!m_pPlayer->GetShip()->IsOutOfFuel())
						{
						Autopilot(false);
						m_pPlayer->SetManeuver(IShipController::RotateRight);
						}
					break;

				case SDLK_SPACE:
				case SDLK_LCTRL:
				case SDLK_RCTRL:
					if (!m_pPlayer->GetShip()->IsOutOfFuel())
						{
						Autopilot(false);
						m_pPlayer->SetFireMain(true);
						}
					break;

				case SDLK_RETURN:
				case SDLK_LSHIFT:
				case SDLK_RSHIFT:
					if (!m_pPlayer->GetShip()->IsOutOfFuel())
						{
						Autopilot(false);
						m_pPlayer->SetFireMissile(true);
						m_pPlayer->ClearShowHelpFireMissile();
						}
					break;

				case SDLK_TAB:
					if (!m_bNextMissileKey)
						{
						Autopilot(false);
						m_pPlayer->ReadyNextMissile();
						UpdateWeaponStatus();
						m_pPlayer->ClearShowHelpSwitchMissile();
						m_bNextMissileKey = true;
						}
					break;

				case SDLK_F1:
					StartHelp();
					break;

				case SDLK_F7:
					{
					int iVolume = GetSoundVolumeOption();
					if (--iVolume >= 0)
						{
						SetSoundVolumeOption(iVolume);
						DisplayMessage(strPatternSubst(CONSTLIT("Volume %d"), iVolume));
						}
					break;
					}

				case SDLK_F8:
					{
					int iVolume = GetSoundVolumeOption();
					if (++iVolume <= 10)
						{
						SetSoundVolumeOption(iVolume);
						DisplayMessage(strPatternSubst(CONSTLIT("Volume %d"), iVolume));
						}
					break;
					}

				case SDLK_F9:
					{
					if (m_Options.bDebugMode)
						m_bDebugConsole = !m_bDebugConsole;
					break;
					}

				//	SDLK_F12
				//	NOTE: Do not use F12 as it causes a break with running with a debugger
				}

			break;
			}

		case gsIntro:
			OnKeyDownIntro(iVirtKey, dwKeyData);
			break;

		case gsSelectAdventure:
			OnKeyDownSelectAdventure(iVirtKey, dwKeyData);
			break;

		case gsSelectShip:
			OnKeyDownSelectShip(iVirtKey, dwKeyData);
			break;

		case gsProlog:
			m_bContinue = true;
			break;

		case gsDocked:
			m_CurrentDock.HandleKeyDown(iVirtKey);
			break;

		case gsEpilog:
			StopEpilog();
			StartIntro();
			break;

		case gsHelp:
			StopHelp();
			break;
		default:
			break;
		}

	return 0;
	}

LONG CTranscendenceWnd::WMKeyUp (int iVirtKey, DWORD dwKeyData)

//	WMKeyUp
//
//	Handle WM_KEYUP

	{
	switch (m_State)
		{
		case gsInGame:
		case gsDocked:
		case gsEnteringStargate:
			{
			if (m_CurrentMenu != menuNone
					&& iVirtKey >= 'a' && iVirtKey < 'z')
				return 0;

			if (m_bDebugConsole)
				return 0;

			switch (iVirtKey)
				{
				case SDLK_DOWN:
				case SDLK_UP:
				case SDLK_k:
					if (m_pPlayer)
						m_pPlayer->SetThrust(false);
					break;

				case SDLK_LEFT:
				case SDLK_j:
					if (m_pPlayer && m_pPlayer->GetManeuver() == IShipController::RotateLeft)
						m_pPlayer->SetManeuver(IShipController::NoRotation);
					break;

				case SDLK_RIGHT:
				case SDLK_l:
					if (m_pPlayer && m_pPlayer->GetManeuver() == IShipController::RotateRight)
						m_pPlayer->SetManeuver(IShipController::NoRotation);
					break;

				case SDLK_SPACE:
				case SDLK_LCTRL:
				case SDLK_RCTRL:
					if (m_pPlayer)
						m_pPlayer->SetFireMain(false);
					break;

				case SDLK_RETURN:
				case SDLK_LSHIFT:
				case SDLK_RSHIFT:
					if (m_pPlayer)
						m_pPlayer->SetFireMissile(false);
					break;

				case SDLK_w:
					m_bNextWeaponKey = false;
					break;

				case SDLK_TAB:
					m_bNextMissileKey = false;
					break;
				}

			break;
			}
		default:
			break;
		}

	return 0;
	}

LONG CTranscendenceWnd::WMLButtonDblClick (int x, int y, DWORD dwFlags)

//	WMLButtonDblClick
//
//	Handle WM_LBUTTONDBLCLICK

	{
	if (m_Options.bWindowedMode)
		{
		x -= m_rcWindowScreen.left;
		y -= m_rcWindowScreen.top;
		}

	switch (m_State)
		{
		case gsIntro:
			OnDblClickIntro(x, y, dwFlags);
			break;

		case gsSelectAdventure:
			OnDblClickSelectAdventure(x, y, dwFlags);
			break;

		case gsSelectShip:
			OnDblClickSelectShip(x, y, dwFlags);
			break;

		case gsDocked:
			m_pCurrentScreen->LButtonDown(x, y);
			break;

		default:
			break;
		}

	return 0;
	}

LONG CTranscendenceWnd::WMLButtonDown (int x, int y, DWORD dwFlags)

//	WMLButtonDown
//
//	Handle WM_LBUTTONDOWN

	{
	if (m_Options.bWindowedMode)
		{
		x -= m_rcWindowScreen.left;
		y -= m_rcWindowScreen.top;
		}

	switch (m_State)
		{
		case gsIntro:
			OnLButtonDownIntro(x, y, dwFlags);
			break;

		case gsSelectAdventure:
			OnLButtonDownSelectAdventure(x, y, dwFlags);
			break;

		case gsSelectShip:
			OnLButtonDownSelectShip(x, y, dwFlags);
			break;

		case gsProlog:
			m_bContinue = true;
			break;

		case gsDocked:
			m_pCurrentScreen->LButtonDown(x, y);
			break;

		case gsEpilog:
			StopEpilog();
			StartIntro();
			break;

		case gsHelp:
			StopHelp();
			break;

		default:
			break;
		}

	return 0;
	}

LONG CTranscendenceWnd::WMLButtonUp (int x, int y, DWORD dwFlags)

//	WMLButtonUp
//
//	Handle WM_LBUTTONUP

	{
	if (m_Options.bWindowedMode)
		{
		x -= m_rcWindowScreen.left;
		y -= m_rcWindowScreen.top;
		}

	switch (m_State)
		{
		case gsDocked:
			m_pCurrentScreen->LButtonUp(x, y);
			break;
		default:
			/* Do nothing. */
			break;
		}

	return 0;
	}

LONG CTranscendenceWnd::WMMouseMove (int x, int y, DWORD dwFlags)

//	WMMouseMove
//
//	Handle WM_MOUSEMOVE

	{
	if (m_Options.bWindowedMode)
		{
		x -= m_rcWindowScreen.left;
		y -= m_rcWindowScreen.top;
		}

	switch (m_State)
		{
		case gsIntro:
			OnMouseMoveIntro(x, y, dwFlags);
			break;

		case gsSelectAdventure:
			OnMouseMoveSelectAdventure(x, y, dwFlags);
			break;

		case gsSelectShip:
			OnMouseMoveSelectShip(x, y, dwFlags);
			break;

		case gsDocked:
			m_pCurrentScreen->MouseMove(x, y);
			break;
		default:
			break;
		}

	return 0;
	}

LONG CTranscendenceWnd::WMMove (int x, int y)

//	WMMove
//
//	Handle WM_MOVE

	{
	return 0;
	}

LONG CTranscendenceWnd::WMSize (int cxWidth, int cyHeight, int iSize)

//	WMSize
//
//	Handle WM_SIZE

	{
	return 0;
	}
