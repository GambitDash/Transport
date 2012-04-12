//	IntroScreen.cpp
//
//	Show intro screen

#include "PreComp.h"
#include "Transcendence.h"
#include "XMLUtil.h"

#include "SDL.h"

#define INTRO_DISPLAY_WIDTH					1024
#define INTRO_DISPLAY_HEIGHT				512

#define HIGHSCORE_DISPLAY_X					0
#define HIGHSCORE_DISPLAY_Y					200
#define HIGHSCORE_DISPLAY_WIDTH				500
#define HIGHSCORE_DISPLAY_HEIGHT			250

#define STR_TYPEFACE						CONSTLIT("Tahoma")
#define RGB_VERSION_COLOR					CG16bitImage::RGBValue(128,128,128)
#define RGB_COPYRIGHT_COLOR					CG16bitImage::RGBValue(80,80,80)

#define MAX_TIME_WITH_ONE_SHIP				(g_TicksPerSecond * 120)

#define DIALOG_WIDTH						500
#define DIALOG_HEIGHT						250
#define DIALOG_SPACING_X					8
#define DIALOG_SPACING_Y					8
#define DIALOG_BUTTON_HEIGHT				25
#define DIALOG_BUTTON_WIDTH					100
#define RGB_DIALOG_BACKGROUND				CG16bitImage::RGBValue(48,48,64)
#define RGB_DIALOG_TEXT						CG16bitImage::RGBValue(144,144,192)
#define RGB_DIALOG_BUTTON_BACKGROUND		CG16bitImage::RGBValue(144,144,192)
#define RGB_DIALOG_BUTTON_TEXT				CG16bitImage::RGBValue(0, 0, 0)

#define STR_OVERWRITE						CONSTLIT("Overwrite")
#define STR_CANCEL							CONSTLIT("Cancel")
#define STR_OVERWRITE_GAME					CONSTLIT("Overwrite Saved Game?")
#define STR_TEXT1							CONSTLIT("You currently have a saved game in progress.")
#define STR_TEXT2							CONSTLIT("If you start a new game your will overwrite your saved game.")

void CTranscendenceWnd::AnimateIntro (void)

//	AnimateIntro
//
//	Paint intro screen

	{
	DWORD dwStartTimer;
	if (m_Options.bDebugVideo)
		dwStartTimer = SDL_GetTicks();

	//	Tell the universe to paint

	m_Universe.PaintPOV(m_Screen, m_rcIntroMain, false);

	//	Paint displays

	m_HighScoreDisplay.Paint(m_Screen);

	m_PlayerDisplay.Update();
	m_PlayerDisplay.Paint(m_Screen);

	m_ButtonBarDisplay.Update();
	m_ButtonBarDisplay.Paint(m_Screen);

	//	Paint version

	int cy;
	int cx = m_Fonts.MediumHeavyBold.MeasureText(m_sVersion, &cy);
	int cxRightMargin = cy;
	int x = m_rcIntroMain.right - (cx + cxRightMargin);
	int y = m_rcIntroMain.bottom - (3 * cy);
	m_Screen.DrawText(x,
			y,
			m_Fonts.MediumHeavyBold,
			RGB_VERSION_COLOR,
			m_sVersion);
	y += cy;

	cx = m_Fonts.Medium.MeasureText(m_sCopyright, &cy);
	x = m_rcIntroMain.right - (cx + cxRightMargin);
	m_Screen.DrawText(x,
			y,
			m_Fonts.Medium,
			RGB_COPYRIGHT_COLOR,
			m_sCopyright);

	//	If we've got a dialog box up, paint it

	if (m_bOverwriteGameDlg)
		PaintOverwriteGameDlg();

	//	Figure out how long it took to paint

	if (m_Options.bDebugVideo)
		{
		DWORD dwNow = SDL_GetTicks();
		m_iPaintTime[m_iFrameCount % FRAME_RATE_COUNT] = dwNow - dwStartTimer;
		dwStartTimer = dwNow;
		}

	//	Debug information

	if (m_Options.bDebugVideo)
		PaintFrameRate();

#ifdef DEBUG
	PaintDebugLines();
#endif

	//	Update the screen

	BltScreen();

	//	Figure out how long it took to blt

	if (m_Options.bDebugVideo)
		{
		DWORD dwNow = SDL_GetTicks();
		m_iBltTime[m_iFrameCount % FRAME_RATE_COUNT] = dwNow - dwStartTimer;
		dwStartTimer = dwNow;
		}

	//	If the same ship has been here for a while, then create a new ship

	if (m_iTick - m_iLastShipCreated > MAX_TIME_WITH_ONE_SHIP)
		{
		CShip *pShip = m_Universe.GetPOV()->AsShip();
		if (pShip)
			pShip->Destroy(removedFromSystem, NULL);
		}

	//	Update the universe

	m_Universe.Update(g_SecondsPerUpdate);
	m_iTick++;

	//	Figure out how long it took to update

	if (m_Options.bDebugVideo)
		{
		DWORD dwNow = SDL_GetTicks();
		m_iUpdateTime[m_iFrameCount % FRAME_RATE_COUNT] = dwNow - dwStartTimer;
		dwStartTimer = dwNow;
		}
	}

CString CTranscendenceWnd::ComputeLoadGameLabel (bool *retbSavedGame)

//	ComputeLoadGameLabel
//
//	Computes the label for the Load Game button. If we return
//	an empty string then there is no valid Load Game.

	{
	ALERROR error;
	CString sLabel;

	if (retbSavedGame)
		*retbSavedGame = false;

	//	If we can't load the file (or no file exists) then
	//	we have no valid saved game.

	CString sFilename = m_GameFile.GenerateFilename(m_sPlayerName);
	if ((error = m_GameFile.Open(sFilename)))
		return sLabel;

	//	If the universe is not valid, then we don't have a valid
	//	saved game (this can happen in the first system).

	if (m_GameFile.IsUniverseValid())
		{
		//	If this is a resurrect game (meaning that we've died before),
		//	then compose the label appropriately

		if (m_GameFile.IsGameResurrect())
			sLabel = strPatternSubst(CONSTLIT("Resurrect in %s System"), m_GameFile.GetSystemName().GetASCIIZPointer());

		//	Otherwise, this is a normal saved game

		else
			{
			sLabel = strPatternSubst(CONSTLIT("Continue Game in %s System"), m_GameFile.GetSystemName().GetASCIIZPointer());
			if (retbSavedGame)
				*retbSavedGame = true;
			}
		}

	//	Done

	m_GameFile.Close();
	return sLabel;
	}

ALERROR CTranscendenceWnd::CreateRandomShip (CSystem *pSystem, CSovereign *pSovereign, CShip **retpShip)

//	CreateRandomShip
//
//	Creates a random ship

	{
	ALERROR error;
	int i;

	//	Figure out the class

	CShipClass *pShipClass;
	if (m_dwIntroShipClass == 0)
		{
		do
			pShipClass = m_Universe.GetShipClass(mathRandom(0, m_Universe.GetShipClassCount()-1));
		while (pShipClass->GetScore() > 1000 || pShipClass->IsPlayerShip());
		}
	else
		{
		int i;
		int iIndex = -1;
		for (i = 0; i < m_Universe.GetShipClassCount(); i++)
			if (m_Universe.GetShipClass(i)->GetUNID() == m_dwIntroShipClass)
				{
				iIndex = i;
				break;
				}

		if (iIndex == -1 || (iIndex + 1) == m_Universe.GetShipClassCount())
			pShipClass = m_Universe.GetShipClass(0);
		else
			pShipClass = m_Universe.GetShipClass(iIndex + 1);

		m_dwIntroShipClass = 0;
		}

	//	Normally we create a single ship, but sometimes we create lots

	int iCount;
	int iRoll = mathRandom(1, 100);

	//	Adjust the roll for capital ships

	if (pShipClass->GetHullMass() >= 10000)
		iRoll -= 9;
	else if (pShipClass->GetHullMass() >= 1000)
		iRoll -= 6;

	if (iRoll == 100)
		iCount = mathRandom(30, 60);
	else if (iRoll >= 98)
		iCount = mathRandom(10, 20);
	else if (iRoll >= 95)
		iCount = mathRandom(5, 10);
	else if (iRoll >= 90)
		iCount = mathRandom(2, 5);
	else
		iCount = 1;

	//	Create the ships

	for (i = 0; i < iCount; i++)
		{
		CShip *pShip;
		if ((error = pSystem->CreateShip(pShipClass->GetUNID(),
				NULL,
				pSovereign,
				PolarToVector(mathRandom(0, 359), mathRandom(250, 2500) * g_KlicksPerPixel),
				NullVector,
				mathRandom(0, 359),
				NULL,
				&pShip)))
			return error;

		//	Override the controller

		CIntroShipController *pNewController = new CIntroShipController(this, pShip->GetController());
		pShip->SetController(pNewController, false);
		pNewController->SetShip(pShip);
		pShip->SetData(CONSTLIT("IntroController"), CONSTLIT("True"));

		*retpShip = pShip;
		}

	return NOERROR;
	}

void CTranscendenceWnd::OnDblClickIntro (int x, int y, DWORD dwFlags)

//	OnDblClickIntro
//
//	Handle WM_LBUTTONDBLCLK

	{
	if (m_bOverwriteGameDlg)
		return;

	if (m_PlayerDisplay.OnLButtonDoubleClick(x, y))
		return;

	if (m_ButtonBarDisplay.OnLButtonDoubleClick(x, y))
		return;
	}

void CTranscendenceWnd::OnCharIntro (char chChar, DWORD dwKeyData)

//	OnCharIntro
//
//	Handle WM_CHAR

	{
	if (m_bOverwriteGameDlg)
		{
		return;
		}

	if (m_PlayerDisplay.OnChar(chChar))
		return;

	if (m_ButtonBarDisplay.OnChar(chChar))
		return;

	switch (chChar)
		{
		case 'N':
		case 'n':
			{
			CShip *pShip = m_Universe.GetPOV()->AsShip();
			if (pShip)
				{
				m_dwIntroShipClass = pShip->GetClass()->GetUNID();

				pShip->Destroy(removedFromSystem, NULL);
				}
			break;
			}
		}
	}

void CTranscendenceWnd::OnKeyDownIntro (int iVirtKey, DWORD dwKeyData)

//	OnKeyDownIntro
//
//	Handle WM_KEYDOWN

	{
	if (m_bOverwriteGameDlg)
		return;

	if (m_PlayerDisplay.OnKeyDown(iVirtKey))
		return;

	if (m_ButtonBarDisplay.OnKeyDown(iVirtKey))
		return;

	switch (iVirtKey)
		{
		case SDLK_RETURN:
			if (m_bSavedGame)
				DoCommand(CMD_CONTINUE_OLD_GAME);
			else
				DoCommand(CMD_START_NEW_GAME);
			break;

		case SDLK_UP:
			m_HighScoreDisplay.SelectPrevious();
			break;

		case SDLK_DOWN:
			m_HighScoreDisplay.SelectNext();
			break;

		case SDLK_F1:
			StartHelp();
			break;
		}
	}

void CTranscendenceWnd::OnLButtonDownIntro (int x, int y, DWORD dwFlags)

//	OnLButtonDownIntro
//
//	Handle WM_LBUTTONDOWN

	{
	if (m_bOverwriteGameDlg)
		{
		POINT pt;
		pt.x = x;
		pt.y = y;

		if (::PtInRect(&m_rcOverwriteGameOK, pt))
			{
			m_bSavedGame = false;
			m_bOverwriteGameDlg = false;
			DoCommand(CMD_START_NEW_GAME);
			}
		else if (::PtInRect(&m_rcOverwriteGameCancel, pt))
			m_bOverwriteGameDlg = false;

		return;
		}

	if (m_PlayerDisplay.OnLButtonDown(x, y))
		return;

	if (m_ButtonBarDisplay.OnLButtonDown(x, y))
		return;
	}

void CTranscendenceWnd::OnMouseMoveIntro (int x, int y, DWORD dwFlags)

//	OnMouseMoveIntro
//
//	Handle WM_MOUSEMOVE

	{
	if (m_bOverwriteGameDlg)
		return;

	m_ButtonBarDisplay.OnMouseMove(x, y);
	}

void CTranscendenceWnd::PaintDlgButton (const RECT &rcRect, const CString &sText)

//	PaintDlgButton
//
//	Paint button

	{
	m_Screen.Fill(rcRect.left, 
			rcRect.top,
			RectWidth(rcRect),
			RectHeight(rcRect),
			RGB_DIALOG_BUTTON_BACKGROUND);

	int cy;
	int cx = m_Fonts.MediumHeavyBold.MeasureText(sText, &cy);

	m_Screen.DrawText(rcRect.left + (RectWidth(rcRect) - cx) / 2,
			rcRect.top + (RectHeight(rcRect) - cy) / 2,
			m_Fonts.MediumHeavyBold,
			RGB_DIALOG_BUTTON_TEXT,
			sText);
	}

void CTranscendenceWnd::PaintOverwriteGameDlg (void)

//	PaintOverwriteGameDlg
//
//	Paint dialog box

	{
	//	Fade the background

	m_Screen.FillTrans(0, 0, m_Screen.GetWidth(), m_Screen.GetHeight(), 0, 128);

	//	Paint the dialog box frame

	m_Screen.Fill(m_rcOverwriteGameDlg.left, m_rcOverwriteGameDlg.top, DIALOG_WIDTH, DIALOG_HEIGHT, RGB_DIALOG_BACKGROUND);

	//	Paint the text

	int y = m_rcOverwriteGameDlg.top + DIALOG_SPACING_Y;
	int cy;
	int cx = m_Fonts.SubTitle.MeasureText(STR_OVERWRITE_GAME, &cy);
	m_Screen.DrawText(m_rcOverwriteGameDlg.left + (DIALOG_WIDTH - cx) / 2,
			y,
			m_Fonts.SubTitle,
			RGB_DIALOG_TEXT,
			STR_OVERWRITE_GAME);
	y += cy + DIALOG_SPACING_Y;

	cx = m_Fonts.Medium.MeasureText(STR_TEXT1, &cy);
	m_Screen.DrawText(m_rcOverwriteGameDlg.left + (DIALOG_WIDTH - cx) / 2,
			y,
			m_Fonts.Medium,
			RGB_DIALOG_TEXT,
			STR_TEXT1);
	y += cy;

	cx = m_Fonts.Medium.MeasureText(STR_TEXT2, &cy);
	m_Screen.DrawText(m_rcOverwriteGameDlg.left + (DIALOG_WIDTH - cx) / 2,
			y,
			m_Fonts.Medium,
			RGB_DIALOG_TEXT,
			STR_TEXT2);

	//	Paint the buttons

	PaintDlgButton(m_rcOverwriteGameOK, STR_OVERWRITE);
	PaintDlgButton(m_rcOverwriteGameCancel, STR_CANCEL);
	}

ALERROR CTranscendenceWnd::StartIntro (void)

//	StartIntro
//
//	Start introduction

	{
	ALERROR error;
	int i;

	ClearDebugLines();

	//	Use widescreen topology

	int cyBarHeight = (g_cyScreen - INTRO_DISPLAY_HEIGHT) / 2;
	m_rcIntroTop.top = 0;
	m_rcIntroTop.left = 0;
	m_rcIntroTop.bottom = cyBarHeight;
	m_rcIntroTop.right = g_cxScreen;

	m_rcIntroMain.top = cyBarHeight;
	m_rcIntroMain.left = 0;
	m_rcIntroMain.bottom = g_cyScreen - cyBarHeight;
	m_rcIntroMain.right = g_cxScreen;

	m_rcIntroBottom.top = g_cyScreen - cyBarHeight;
	m_rcIntroBottom.left = 0;
	m_rcIntroBottom.bottom = g_cyScreen;
	m_rcIntroBottom.right = g_cxScreen;

	m_Screen.DrawRectFilled(m_rcIntroTop.left, m_rcIntroTop.top,m_rcIntroTop.right, m_rcIntroTop.bottom, CGImage::RGBColor(0, 0, 0));
	m_Screen.DrawRectFilled(m_rcIntroBottom.left, m_rcIntroBottom.top,m_rcIntroBottom.right, m_rcIntroBottom.bottom, CGImage::RGBColor(0, 0, 0));
	//	Create the High-Score display

	RECT rcRect;
	rcRect.left = m_rcMainScreen.left + HIGHSCORE_DISPLAY_X;
	rcRect.top = m_rcMainScreen.top + HIGHSCORE_DISPLAY_Y;
	rcRect.right = rcRect.left + HIGHSCORE_DISPLAY_WIDTH;
	rcRect.bottom = rcRect.top + HIGHSCORE_DISPLAY_HEIGHT;
	m_HighScoreDisplay.Init(rcRect, m_pHighScoreList, m_iLastHighScore);
	m_HighScoreDisplay.SetFontTable(&m_Fonts);
	m_HighScoreDisplay.Update();

	//	Create the player display

	m_PlayerDisplay.Init(this, m_rcIntroTop, m_Options.bDebugMode);

	//	Create the buttons

	m_ButtonBar.Init();

	CString sContinueLabel = ComputeLoadGameLabel(&m_bSavedGame);
	m_ButtonBar.AddButton(CMD_CONTINUE_OLD_GAME,
			CONSTLIT("Continue Game"),
			sContinueLabel,
			CONSTLIT("C"),
			0,
			CButtonBarData::alignLeft);
	if (sContinueLabel.IsBlank())
		m_ButtonBar.SetVisible(0, false);

	m_ButtonBar.AddButton(CMD_START_NEW_GAME,
			CONSTLIT("New Game"),
			CONSTLIT("Begin a New Game"),
			CONSTLIT("N"),
			1,
			CButtonBarData::alignCenter);

	m_ButtonBar.AddButton(CMD_QUIT_GAME,
			CONSTLIT("Quit"),
			CONSTLIT("Exit Transcendence"),
			CONSTLIT("Q"),
			2,
			CButtonBarData::alignRight);

	m_ButtonBarDisplay.SetFontTable(&m_Fonts);
	m_ButtonBarDisplay.Init(this, &m_ButtonBar, m_rcIntroBottom);

	//	Create an empty system

	if (error = m_Universe.CreateEmptyStarSystem(&m_pIntroSystem))
		{
		ASSERT(false);
		return error;
		}

	m_Universe.SetCurrentSystem(m_pIntroSystem);

	CSovereign *pSovereign1 = m_Universe.FindSovereign(g_PlayerSovereignUNID);
	CSovereign *pSovereign2 = m_Universe.FindSovereign(g_PirateSovereignUNID);

	//	Create a couple of random enemy ships

	CShip *pShip1;
	CShip *pShip2;
	if ((error = CreateRandomShip(m_pIntroSystem, pSovereign1, &pShip1)))
		{
		ASSERT(false);
		return error;
		}

	if ((error = CreateRandomShip(m_pIntroSystem, pSovereign2, &pShip2)))
		{
		ASSERT(false);
		return error;
		}

	//	Make the ships attack each other

	for (i = 0; i < m_pIntroSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = m_pIntroSystem->GetObject(i);

		if (pObj
				&& pObj->GetCategory() == CSpaceObject::catShip
				&& !pObj->GetData(CONSTLIT("IntroController")).IsBlank())
			{
			CShip *pShip = pObj->AsShip();
			if (pShip)
				{
				IShipController *pController = pShip->GetController();
				if (pShip->GetSovereign() == pSovereign1)
					pController->AddOrder(IShipController::orderDestroyTarget, pShip2, 0);
				else
					pController->AddOrder(IShipController::orderDestroyTarget, pShip1, 0);
				}
			}
		}

	//	Other initialization

	m_bOverwriteGameDlg = false;
	m_rcOverwriteGameDlg.left = m_rcIntroMain.left + (RectWidth(m_rcIntroMain) - DIALOG_WIDTH) / 2;
	m_rcOverwriteGameDlg.top = m_rcIntroMain.top + (RectHeight(m_rcIntroMain) - DIALOG_HEIGHT) / 2;
	m_rcOverwriteGameDlg.right = m_rcOverwriteGameDlg.left + DIALOG_WIDTH;
	m_rcOverwriteGameDlg.bottom = m_rcOverwriteGameDlg.top + DIALOG_HEIGHT;

	int cxMid = m_rcOverwriteGameDlg.left + (DIALOG_WIDTH / 2);
	m_rcOverwriteGameOK.left = cxMid - DIALOG_SPACING_X - DIALOG_BUTTON_WIDTH;
	m_rcOverwriteGameOK.top = m_rcOverwriteGameDlg.bottom - DIALOG_SPACING_Y - DIALOG_BUTTON_HEIGHT;
	m_rcOverwriteGameOK.right = m_rcOverwriteGameOK.left + DIALOG_BUTTON_WIDTH;
	m_rcOverwriteGameOK.bottom = m_rcOverwriteGameOK.top + DIALOG_BUTTON_HEIGHT;

	m_rcOverwriteGameCancel = m_rcOverwriteGameOK;
	m_rcOverwriteGameCancel.left = cxMid + DIALOG_SPACING_X;
	m_rcOverwriteGameCancel.right = m_rcOverwriteGameCancel.left + DIALOG_BUTTON_WIDTH;

	//	No sound

	m_Universe.SetSound(false);

	//	Set the POV to one of them

	m_Universe.SetPOV(pShip1);
	m_iTick = 0;
	m_iLastShipCreated = m_iTick;

	m_State = gsIntro;

	//	Show the cursor

	SDL_ShowCursor(true);

	return NOERROR;
	}

void CTranscendenceWnd::StopIntro (void)

//	StopIntro
//
//	Stop introduction screen

	{
	ASSERT(m_State == gsIntro);

	m_HighScoreDisplay.CleanUp();
	m_PlayerDisplay.CleanUp();
	m_ButtonBarDisplay.CleanUp();
	m_ButtonBar.CleanUp();

	//	Destroy system

	m_Universe.DestroySystem(m_pIntroSystem);
	m_pIntroSystem = NULL;

	//	Enable sound

	m_Universe.SetSound(true);

	//	Hide cursor

	SDL_ShowCursor(false);
	}

