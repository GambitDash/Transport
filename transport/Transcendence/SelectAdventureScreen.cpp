//	SelectAdventureScreen.cpp
//
//	Show select adventure screen

#include "PreComp.h"
#include "Transcendence.h"
#include "XMLUtil.h"

#include "SDL.h"

const COLORREF RGB_TOP_BAR =					CG16bitImage::RGBValue(0, 2, 10);

const int INTRO_DISPLAY_WIDTH =				1024;
const int INTRO_DISPLAY_HEIGHT =			512;

void CTranscendenceWnd::AnimateSelectAdventure (void)

//	AnimateSelectAdventure
//
//	Paint the select adventure screen

	{
	//	Paint displays

	m_AdventureDescDisplay.Update();
	m_AdventureDescDisplay.Paint(m_Screen);

	m_ButtonBarDisplay.Update();
	m_ButtonBarDisplay.Paint(m_Screen);

	//	Update the screen

	BltScreen();
	}

void CTranscendenceWnd::OnCharSelectAdventure (char chChar, DWORD dwKeyData)

//	OnCharSelectShip
//
//	Handle OnChar

	{
	if (m_ButtonBarDisplay.OnChar(chChar))
		return;
	}

void CTranscendenceWnd::OnDblClickSelectAdventure (int x, int y, DWORD dwFlags)

//	OnDblClickSelectShip
//
//	Handle double-click

	{
	if (m_ButtonBarDisplay.OnLButtonDoubleClick(x, y))
		return;
	}

void CTranscendenceWnd::OnKeyDownSelectAdventure (int iVirtKey, DWORD dwKeyData)

//	OnKeyDownSelectShip
//
//	Handle OnKeyDown

	{
	if (m_ButtonBarDisplay.OnKeyDown(iVirtKey))
		return;

	if (m_AdventureDescDisplay.OnKeyDown(iVirtKey))
		return;

	switch (iVirtKey)
		{
		case SDLK_ESCAPE:
			DoCommand(CMD_SELECT_ADVENTURE_CANCEL);
			break;

		case SDLK_RETURN:
			DoCommand(CMD_SELECT_ADVENTURE);
			break;
		}
	}

void CTranscendenceWnd::OnLButtonDownSelectAdventure (int x, int y, DWORD dwFlags)

//	OnLButtonDownSelectShip
//
//	Handle OnLButtonDown

	{
	if (m_ButtonBarDisplay.OnLButtonDown(x, y))
		return;
	}

void CTranscendenceWnd::OnMouseMoveSelectAdventure (int x, int y, DWORD dwFlags)

//	OnMouseMoveSelectShip
//
//	Handle OnMouseMove

	{
	m_ButtonBarDisplay.OnMouseMove(x, y);
	}

ALERROR CTranscendenceWnd::StartSelectAdventure (void)

//	StartSelectShip
//
//	Show the select ship screen

	{
	RECT rcRect;

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

	// Clear the screen
	
	m_Screen.DrawRectFilled(0, 0, m_Screen.GetWidth(), m_Screen.GetHeight(), CGImage::RGBColor(0, 0, 0));

	//	Create the buttons

	m_ButtonBar.Init();

	m_ButtonBar.AddButton(CMD_PREV_ADVENTURE,
			NULL_STR,
			NULL_STR,
			NULL_STR,
			8,
			CButtonBarData::alignCenter,
			CButtonBarData::styleMedium);

	m_ButtonBar.AddButton(CMD_SELECT_ADVENTURE,
			CONSTLIT("Start Adventure"),
			CONSTLIT("Start this adventure"),
			CONSTLIT("S"),
			3,
			CButtonBarData::alignCenter);

	m_ButtonBar.AddButton(CMD_NEXT_ADVENTURE,
			NULL_STR,
			NULL_STR,
			NULL_STR,
			9,
			CButtonBarData::alignCenter,
			CButtonBarData::styleMedium);

	m_ButtonBar.AddButton(CMD_SELECT_ADVENTURE_CANCEL,
			CONSTLIT("Cancel"),
			CONSTLIT("Cancel New Game"),
			CONSTLIT("C"),
			5,
			CButtonBarData::alignRight);

	m_ButtonBarDisplay.SetFontTable(&m_Fonts);
	m_ButtonBarDisplay.Init(this, &m_ButtonBar, m_rcIntroBottom);

	//	Create the adventure desc display

	rcRect = m_rcIntroTop;
	rcRect.bottom = m_rcIntroBottom.top;
	m_AdventureDescDisplay.Init(this, rcRect);

	//	Done

	m_State = gsSelectAdventure;
	ShowCursor(true);

	return NOERROR;
	}

void CTranscendenceWnd::StopSelectAdventure (void)

//	StopSelectAdventure
//
//	Stop select ship screen

	{
	ASSERT(m_State == gsSelectAdventure);

//	m_ShipClassDisplay.CleanUp();
	m_ButtonBarDisplay.CleanUp();
	m_ButtonBar.CleanUp();

	//	Hide cursor

	ShowCursor(false);
	}

