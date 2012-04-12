//	GameScreen.cpp
//
//	Handles CTranscendenceWnd methods related to playing the game

#include "PreComp.h"
#include "Transcendence.h"
#include "XMLUtil.h"

#define BEGIN_EXCEPTION_HANDLER		try
#define END_EXCEPTION_HANDLER		catch (...) { CleanUpDirectDraw(); throw; }

#define ARMOR_DISPLAY_WIDTH					360
#define ARMOR_DISPLAY_HEIGHT				136
#define ARMOR_DISPLAY_MARGIN_X				8
#define ARMOR_DISPLAY_MARGIN_Y				8

#define REACTOR_DISPLAY_HEIGHT				64
#define REACTOR_DISPLAY_WIDTH				256

#define TARGET_DISPLAY_WIDTH				360
#define TARGET_DISPLAY_HEIGHT				120

#define INVOKE_DISPLAY_WIDTH				200
#define INVOKE_DISPLAY_HEIGHT				300

#define MENU_DISPLAY_WIDTH					200
#define MENU_DISPLAY_HEIGHT					600

#define PICKER_DISPLAY_WIDTH				1024
#define PICKER_DISPLAY_HEIGHT				160

#define DEVICE_DISPLAY_WIDTH				1024
#define DEVICE_DISPLAY_HEIGHT				160

#define SRSSNOW_IMAGE_WIDTH					256
#define SRSSNOW_IMAGE_HEIGHT				256

#define CMD_PAUSE							100
#define CMD_SAVE							101
#define CMD_SELF_DESTRUCT					103

#define CMD_CONFIRM							110
#define CMD_CANCEL							111

#define MAX_COMMS_OBJECTS					25

#define STR_INSTALLED						CONSTLIT(" (installed)")
#define STR_DISABLED						CONSTLIT(" (disabled)")

#define BAR_COLOR							CG16bitImage::RGBValue(0, 2, 10)

#define STR_MAP_HELP						CONSTLIT("[+] and [-] to zoom map")
#define STR_MAP_HELP2						CONSTLIT("[H] to toggle HUD on/off")

const int LRS_UPDATE_DELAY = 5;
const int g_LRSWidth = 200;
const int g_LRSHeight = 200;

const int g_WeaponStatusWidth = 350;
const int g_WeaponStatusHeight = 32;

const int g_MessageDisplayWidth = 400;
const int g_MessageDisplayHeight = 32;

const int g_iScaleCount = 2;
const int g_iStellarScale = 0;
const int g_iPlanetaryScale = 1;

const int MAP_ZOOM_SPEED =					16;

void CTranscendenceWnd::Autopilot (bool bTurnOn)

//	Autopilot
//
//	Turn autopilot on/off

	{
	if (bTurnOn != m_bAutopilot)
		{
		if (bTurnOn)
			{
			if (m_pPlayer->GetShip()->HasAutopilot())
				{
				DisplayMessage(CONSTLIT("Autopilot engaged"));
				m_bAutopilot = true;
				}
			else
				DisplayMessage(CONSTLIT("No autopilot installed"));
			}
		else
			{
			DisplayMessage(CONSTLIT("Autopilot disengaged"));
			m_bAutopilot = false;
			}

		}
	}

void CTranscendenceWnd::DisplayMessage (CString sMessage)

//	DisplayMessage
//
//	Display a message for the player

	{
	if (m_bRedirectDisplayMessage)
		m_sRedirectMessage.Append(sMessage);
	else
		m_MessageDisplay.DisplayMessage(sMessage, m_Fonts.wTitleColor);
	}

void CTranscendenceWnd::DoCommsMenu (int iIndex)

//	DoCommsMenu
//
//	Send message to the object

	{
	if (m_pPlayer)
		m_pMenuObj->CommsMessageFrom(m_pPlayer->GetShip(), iIndex);

	m_CurrentMenu = menuNone;
	m_pMenuObj = NULL;
	}

void CTranscendenceWnd::DoEnableDisableItemCommand (DWORD dwData)

//	DoEnableDisableItemCommand
//
//	Enable/disable an item

	{
	CShip *pShip = m_pPlayer->GetShip();
	CItemList &ItemList = pShip->GetItemList();
	CItem &Item = ItemList.GetItem(dwData);

	if (Item.IsInstalled() && Item.GetType()->IsDevice())
		{
		int iDev = Item.GetInstalled();
		CInstalledDevice *pDevice = pShip->GetDevice(iDev);
		pShip->EnableDevice(iDev, !pDevice->IsEnabled());
		}

	//	Dismiss picker

	m_CurrentPicker = pickNone;
	}

void CTranscendenceWnd::DoGameMenuCommand (DWORD dwCmd)

//	DoGameMenuCommand
//
//	Do game menu

	{
	switch (dwCmd)
		{
		case CMD_PAUSE:
			m_CurrentMenu = menuNone;
			StartHelp();
			break;

		case CMD_SAVE:
			m_CurrentMenu = menuNone;
			if (SaveGame(false) != NOERROR)
				{
				DisplayMessage(CONSTLIT("Chaos: Unable to save the game"));
				break;
				}

			//	Back to Intro screen

			ASSERT(m_State == gsInGame);
			RestartGame();
			break;

		case CMD_SELF_DESTRUCT:
			DisplayMessage(CONSTLIT("Warning: Self-Destruct Activated"));

			m_MenuData.SetTitle(CONSTLIT("Self-Destruct"));
			m_MenuData.RemoveAll();
			m_MenuData.AddMenuItem(CONSTLIT("1"), CONSTLIT("Confirm"), CMD_CONFIRM);
			m_MenuData.AddMenuItem(CONSTLIT("2"), CONSTLIT("Cancel"), CMD_CANCEL);
			m_MenuDisplay.Invalidate();
			m_CurrentMenu = menuSelfDestructConfirm;
			break;
		}
	}

void CTranscendenceWnd::DoInvocation (CPower *pPower)

//	DoInvocation
//
//	Invoke power

	{
	if (m_pPlayer)
		{
		CString sError;
		pPower->InvokeByPlayer(m_pPlayer->GetShip(), m_pPlayer->GetTarget(), &sError);
		if (!sError.IsBlank())
			DisplayMessage(sError);
		}

	m_CurrentMenu = menuNone;
	}

void CTranscendenceWnd::DoOrdersCommand (MessageTypes iOrder, DWORD dwData2)

//	DoOrdersCommand
//
//	Do orders menu command

	{
	if (m_pPlayer)
		m_pPlayer->FleetOrder(iOrder, dwData2);

	m_CurrentMenu = menuNone;
	}

void CTranscendenceWnd::DoSelfDestructConfirmCommand (DWORD dwCmd)

//	DoSelfDestructConfirmCommand
//
//	Confirm self destruct

	{
	switch (dwCmd)
		{
		case CMD_CONFIRM:
			m_pPlayer->GetShip()->Destroy(killedBySelf, NULL);
			m_CurrentMenu = menuNone;
			break;

		case CMD_CANCEL:
			m_CurrentMenu = menuNone;
			break;
		}
	}

void CTranscendenceWnd::DoUseItemCommand (DWORD dwData)

//	DoUseItemCommand
//
//	Use an item

	{
	CShip *pShip = m_pPlayer->GetShip();
	CItemList &ItemList = pShip->GetItemList();
	CItem &Item = ItemList.GetItem(dwData);
	CItemType *pType = Item.GetType();

	//	Use in cockpit

	if (pType->IsUsableInCockpit())
		{
		CString sError;

		pShip->UseItem(Item, &sError);

		if (!sError.IsBlank())
			DisplayMessage(sError);

		pShip->OnComponentChanged(comCargo);
		}

	//	Use screen

	else if (pType->GetUseScreen())
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		ICCItem *pItem = CreateListFromItem(CC, Item);
		CC.DefineGlobal(CONSTLIT("gItem"), pItem);
		pItem->Discard(&CC);

		ShowDockScreen(pShip,
				pType->GetUseScreen(),
				NULL_STR);

		pShip->OnComponentChanged(comCargo);
		}

	//	Dismiss picker

	m_CurrentPicker = pickNone;
	}

void CTranscendenceWnd::HideDockScreen (void)

//	HideDockScreen
//
//	Returns to space

	{
	if (m_State == gsDocked)
		{
		//	Hide the cursor

		SDL_ShowCursor(false);

		//	New state

		m_State = gsInGame;

		//	Clean up

		m_pCurrentScreen = NULL;
		m_CurrentDock.CleanUpScreen();
		}
	}

ALERROR CTranscendenceWnd::InitDisplays (void)

//	InitDisplays
//
//	Initializes display structures

	{
	ALERROR error;
	RECT rcRect;

	//	Set up some options

	m_bTransparencyEffects = true;

	//	Create a bitmap for the LRS

	if ((error = m_LRS.CreateBlank(g_LRSWidth, g_LRSHeight, false)))
		return error;

	/* XXX Need to set this only for pixels matching the mask from the HUD. */
	if (m_bTransparencyEffects)
		m_LRS.SetAlphaChannel(200);

	//m_rcLRS.left = m_rcScreen.left;
	m_rcLRS.left = m_rcScreen.right - g_LRSWidth;
	//m_rcLRS.top = g_cyScreen - g_LRSHeight;
	m_rcLRS.top = 0;
	m_rcLRS.right = m_rcLRS.left + g_LRSWidth;
	m_rcLRS.bottom = m_rcLRS.top + g_LRSHeight;

	//	Find some bitmaps that we need

	m_pLargeHUD = m_Universe.GetLibraryBitmap(g_LRSImageUNID);
	m_pSRSSnow = m_Universe.GetLibraryBitmap(g_SRSSnowImageUNID);
	m_pLRSBorder = m_Universe.GetLibraryBitmap(g_LRSBorderUNID);

	//	Create the message display

	rcRect.left = (RectWidth(m_rcScreen) - g_MessageDisplayWidth) / 2;
	rcRect.right = rcRect.left + g_MessageDisplayWidth;
	rcRect.top = m_rcScreen.bottom - (RectHeight(m_rcScreen) / 3);
	rcRect.bottom = rcRect.top + 4 * g_MessageDisplayHeight;
	m_MessageDisplay.SetRect(rcRect);
	m_MessageDisplay.SetFont(&m_Fonts.Header);
	m_MessageDisplay.SetBlinkTime(15);
	m_MessageDisplay.SetSteadyTime(150);
	m_MessageDisplay.SetFadeTime(30);

	//	Initialize some displays (these need to be done after we've
	//	created the universe).

	rcRect.left = m_rcScreen.right - (ARMOR_DISPLAY_WIDTH + ARMOR_DISPLAY_MARGIN_X);
	rcRect.top = g_cyScreen - (ARMOR_DISPLAY_HEIGHT + ARMOR_DISPLAY_MARGIN_Y);
	//rcRect.top = 0;
	rcRect.right = rcRect.left + ARMOR_DISPLAY_WIDTH;
	rcRect.bottom = rcRect.top + ARMOR_DISPLAY_HEIGHT;
	m_ArmorDisplay.SetFontTable(&m_Fonts);
	m_ArmorDisplay.Init(m_pPlayer, rcRect);
	m_ArmorDisplay.Update();

	rcRect.left = 12;
	rcRect.top = 12;
	rcRect.right = rcRect.left + REACTOR_DISPLAY_WIDTH;
	rcRect.bottom = rcRect.top + REACTOR_DISPLAY_HEIGHT;
	m_ReactorDisplay.SetFontTable(&m_Fonts);
	m_ReactorDisplay.Init(m_pPlayer, rcRect);
	m_ReactorDisplay.Update();

	//rcRect.left = (RectWidth(m_rcScreen) - TARGET_DISPLAY_WIDTH) / 2;
	rcRect.left = 0;
	rcRect.right = rcRect.left + TARGET_DISPLAY_WIDTH;
	rcRect.top = m_rcScreen.bottom - TARGET_DISPLAY_HEIGHT;
	rcRect.bottom = m_rcScreen.bottom;
	m_TargetDisplay.SetFontTable(&m_Fonts);
	m_TargetDisplay.Init(m_pPlayer, rcRect);

	rcRect.left = m_rcScreen.right - (MENU_DISPLAY_WIDTH + 4);
	rcRect.top = (RectHeight(m_rcScreen) - MENU_DISPLAY_HEIGHT) / 2;
	rcRect.right = rcRect.left + MENU_DISPLAY_WIDTH;
	rcRect.bottom = rcRect.top + MENU_DISPLAY_HEIGHT;
	m_MenuDisplay.SetFontTable(&m_Fonts);
	m_MenuDisplay.Init(&m_MenuData, rcRect);

	rcRect.left = m_rcScreen.left + (RectWidth(m_rcScreen) - PICKER_DISPLAY_WIDTH) / 2;
	rcRect.right = rcRect.left + PICKER_DISPLAY_WIDTH;
	rcRect.top = m_ArmorDisplay.GetRect().top - PICKER_DISPLAY_HEIGHT;
	rcRect.bottom = rcRect.top + PICKER_DISPLAY_HEIGHT;
	m_PickerDisplay.SetFontTable(&m_Fonts);
	m_PickerDisplay.Init(&m_MenuData, rcRect);

	rcRect.left = m_rcScreen.left + (RectWidth(m_rcScreen) - DEVICE_DISPLAY_WIDTH) / 2;
	rcRect.right = rcRect.left + DEVICE_DISPLAY_WIDTH;
	rcRect.top = m_rcScreen.bottom - DEVICE_DISPLAY_HEIGHT;
	rcRect.bottom = m_rcScreen.bottom;
	m_DeviceDisplay.SetFontTable(&m_Fonts);
	m_DeviceDisplay.Init(m_pPlayer, rcRect);

	return NOERROR;
	}

void CTranscendenceWnd::OnObjDestroyed (CSpaceObject *pObjDestroyed, CSpaceObject *pDestroyer)

//	OnObjDestroyed
//
//	Object destroyed

	{
	//	If we're showing the comms menu, reload the list (in case
	//	any ships got destroyed)

	if (m_CurrentMenu == menuCommsTarget)
		ShowCommsTargetMenu();
	else if (m_CurrentMenu == menuComms)
		{
		if (pObjDestroyed == m_pMenuObj)
			{
			m_CurrentMenu = menuNone;
			m_pMenuObj = NULL;
			}
		}
	}

void CTranscendenceWnd::PaintLRS (void)

//	PaintLRS
//
//	Paint the long-range scanner

	{
	//	Update the LRS every 10 ticks

	if ((m_iTick % LRS_UPDATE_DELAY) == 0)
		{
		bool bNewEnemies;

		m_LRS.Blt(0, 0, *m_pLargeHUD, 0, 0, g_LRSWidth, g_LRSHeight, false);

		//	If we're not blind, paint the LRS

		if (m_pPlayer == NULL 
				|| !m_pPlayer->GetShip()->IsLRSBlind())
			{
			RECT rcView;
			rcView.left = 0;
			rcView.top = 0;
			rcView.right = g_LRSWidth;
			rcView.bottom = g_LRSHeight;

			m_Universe.PaintPOVLRS(m_LRS, rcView, &bNewEnemies);

			//	Notify player of enemies

			if (bNewEnemies)
				DisplayMessage(CONSTLIT("Enemy ships detected"));
			}

		//	If we're blind, paint snow

		else
			{
			PaintSnow(m_LRS, 0, 0, g_LRSWidth, g_LRSHeight);

			int iCount = mathRandom(1, 8);
			for (int i = 0; i < iCount; i++)
				{
				m_LRS.Fill(0, mathRandom(0, g_LRSHeight),
						g_LRSWidth,
						mathRandom(1, 20),
						CG16bitImage::RGBValue(108, 252, 128));
				}
			}

		//	Mask out the border

		m_LRS.BltToAlpha(m_pLRSBorder);
		}

	//	Blt the LRS

	m_Screen.ColorTransBlt(0,
			0,
			RectWidth(m_rcLRS),
			RectHeight(m_rcLRS),
			255,
			m_LRS,
			m_rcLRS.left,
			m_rcLRS.top);
	}

void CTranscendenceWnd::PaintMainScreenBorder (void)

//	PaintMainScreenBorder
//
//	Paints the borders of the main screen in case the display is larger
//	than the main screen (larger than 1024x768)

	{
	if (m_rcMainScreen.left != m_rcScreen.left)
		{
		int cxMainScreen = RectWidth(m_rcMainScreen);
		int cyMainScreen = RectHeight(m_rcMainScreen);
		int cxLeftRight = (g_cxScreen - cxMainScreen) / 2;
		int cyTopBottom = (g_cyScreen - cyMainScreen) / 2;

		m_Screen.Fill(0, 0, g_cxScreen, cyTopBottom, BAR_COLOR);
		m_Screen.Fill(0, g_cyScreen - cyTopBottom, g_cxScreen, cyTopBottom, BAR_COLOR);
		m_Screen.Fill(0, cyTopBottom, cxLeftRight, cyMainScreen, BAR_COLOR);
		m_Screen.Fill(g_cxScreen - cxLeftRight, cyTopBottom, cxLeftRight, cyMainScreen, BAR_COLOR);
		}
	}

void CTranscendenceWnd::PaintMap (void)

//	PaintMap
//
//	Paints the system map

	{
	//	Paint the map

	Metric rScale = m_rMapScale[m_iMapScale];
	if (m_iMapZoomEffect != 0)
		{
		if (m_iMapZoomEffect > 0)
			{
			rScale = rScale * (1.0 - (m_iMapZoomEffect / 200.0));
			m_iMapZoomEffect = Max(0, m_iMapZoomEffect - MAP_ZOOM_SPEED);
			}
		else
			{
			rScale = rScale * (1.0 - (m_iMapZoomEffect / 100.0));
			m_iMapZoomEffect = Min(0, m_iMapZoomEffect + MAP_ZOOM_SPEED);
			}
		}

	m_Universe.PaintPOVMap(m_Screen, m_rcMainScreen, rScale);

	//	Paint the edges

	PaintMainScreenBorder();

	//	Paint some text, including the system name

	CSpaceObject *pPOV = m_Universe.GetPOV();
	if (pPOV)
		{
		int x = m_rcMainScreen.left + 2 * m_Fonts.LargeBold.GetAverageWidth();
		int y;
		if (m_pPlayer->IsMapHUDActive())
			y = m_TargetDisplay.GetRect().top - 3 * m_Fonts.Header.GetHeight();
		else
			y = m_TargetDisplay.GetRect().bottom - 3 * m_Fonts.Header.GetHeight();

		m_Screen.DrawText(x,
				y,
				m_Fonts.Header,
				m_Fonts.wTitleColor,
				strPatternSubst("%s System", pPOV->GetSystem()->GetName().GetASCIIZPointer()));
		y += m_Fonts.Header.GetHeight();

		//	Paint some helper text

		m_Screen.DrawText(x,
				y,
				m_Fonts.Medium,
				m_Fonts.wHelpColor,
				STR_MAP_HELP);
		y += m_Fonts.Medium.GetHeight();

		m_Screen.DrawText(x,
				y,
				m_Fonts.Medium,
				m_Fonts.wHelpColor,
				STR_MAP_HELP2);
		}
	}

void CTranscendenceWnd::PaintSnow (CG16bitImage &Dest, int x, int y, int cxWidth, int cyHeight)

//	PaintSnow
//
//	Fills the rect with snow

	{
	int y1 = y;
	int ySrc = mathRandom(0, SRSSNOW_IMAGE_HEIGHT-1);
	int cySrc = SRSSNOW_IMAGE_HEIGHT - ySrc;
	while (y1 < cyHeight)
		{
		int x1 = x;
		int xSrc = mathRandom(0, SRSSNOW_IMAGE_WIDTH-1);
		int cxSrc = SRSSNOW_IMAGE_WIDTH - xSrc;
		while (x1 < cxWidth)
			{
			Dest.Blt(xSrc, ySrc, cxSrc, cySrc, *m_pSRSSnow, x1, y1);
			x1 += cxSrc;
			xSrc = 0;
			cxSrc = std::min(SRSSNOW_IMAGE_WIDTH, (g_cxScreen - x1));
			}

		y1 += cySrc;
		ySrc = 0;
		cySrc = std::min(SRSSNOW_IMAGE_HEIGHT, (g_cyScreen - y1));
		}
	}

void CTranscendenceWnd::PaintSRSSnow (void)

//	PaintSRSSnow
//
//	Fills the screen with snow

	{
	PaintSnow(m_Screen, 0, 0, g_cxScreen, g_cyScreen);

	CShip *pShip = m_pPlayer->GetShip();
	if (pShip && pShip->GetSystem())
		m_Universe.PaintObject(m_Screen, m_rcMainScreen, pShip);
	}

void CTranscendenceWnd::SelectArmor (int iSeg)

//	SelectArmor
//
//	Select an armor segment in the armor display

	{
	if (m_pPlayer)
		m_ArmorDisplay.SetSelection(iSeg);
	}

void CTranscendenceWnd::SetSoundVolumeOption (int iVolume)

//	SetSoundVolumeOption
//
//	Sets the volume level

	{
	ASSERT(iVolume >= 0 && iVolume <= 10);

	if (iVolume != m_Prefs.iSoundVolume)
		{
		m_SoundMgr.SetWaveVolume(iVolume);
		m_Prefs.iSoundVolume = iVolume;
		m_Prefs.bModified = true;
		}
	}

void CTranscendenceWnd::ShowCommsMenu (CSpaceObject *pObj)

//	ShowCommsMenu
//
//	Shows the comms menu for this object

	{
	int i;

	if (m_pPlayer)
		{
		CSpaceObject *pShip = m_pPlayer->GetShip();
		m_pMenuObj = pObj;

		m_MenuData.SetTitle(CONSTLIT("Communications"));
		m_MenuData.RemoveAll();

		int iMsgCount = m_pMenuObj->GetCommsMessageCount();
		for (i = 0; i < iMsgCount; i++)
			{
			CString sName;
			CString sKey;

			if (m_pMenuObj->IsCommsMessageValidFrom(pShip, i, &sName, &sKey))
				m_MenuData.AddMenuItem(sKey,
						sName,
						i);
			}

		m_MenuDisplay.Invalidate();
		m_CurrentMenu = menuComms;
		}
	}

void CTranscendenceWnd::ShowCommsTargetMenu (void)

//	ShowCommsTargetMenu
//
//	Shows list of objects to communicate with

	{
	int i;

	if (m_pPlayer)
		{
		CShip *pShip = m_pPlayer->GetShip();
		m_MenuData.SetTitle(CONSTLIT("Communications"));
		m_MenuData.RemoveAll();

		//	Iterate over all objects in communications range and add them
		//	to the menu.

		CSystem *pSystem = pShip->GetSystem();
		for (i = 0; i < pSystem->GetObjectCount(); i++)
			{
			CSpaceObject *pObj = pSystem->GetObject(i);

			if (pObj 
					&& pObj->CanCommunicateWith(pShip)
					&& pObj != pShip)
				{
				if (m_MenuData.GetCount() < MAX_COMMS_OBJECTS)
					{
					char chKey = 'A' + m_MenuData.GetCount();
					m_MenuData.AddMenuItem(CString(&chKey, 1),
							pObj->GetName(),
							(DWORD)pObj);
					}
				}
			}

		if (m_MenuData.GetCount())
			{
			m_MenuDisplay.Invalidate();
			m_CurrentMenu = menuCommsTarget;
			}
		else
			{
			DisplayMessage(CONSTLIT("No carrier signal"));
			m_CurrentMenu = menuNone;
			}
		}
	}

void CTranscendenceWnd::ShowInvokeMenu (void)

//	ShowInvokeMenu
//
//	Shows menu of powers to invoke

	{
	int i;

	if (m_pPlayer)
		{
		m_MenuData.SetTitle(CONSTLIT("Invoke Powers"));
		m_MenuData.RemoveAll();

		//	Add the powers

		for (i = 0; i < g_pUniverse->GetPowerCount(); i++)
			{
			CPower *pPower = g_pUniverse->GetPower(i);

			CString sError;
			if (pPower->OnShow(m_pPlayer->GetShip(), NULL, &sError))
				{
				m_MenuData.AddMenuItem(
						pPower->GetInvokeKey(),
						pPower->GetName(),
						(DWORD)pPower);
				}

			if (!sError.IsBlank())
				{
				DisplayMessage(sError);
				return;
				}
			}

		//	If no powers are available, say so

		if (m_MenuData.GetCount() == 0)
			{
			DisplayMessage(CONSTLIT("No Powers available"));
			return;
			}

		//	Show menu

		m_MenuDisplay.Invalidate();
		m_CurrentMenu = menuInvoke;
		}
	}

void CTranscendenceWnd::ShowOrdersMenu (void)

//	ShowOrdersMenu
//
//	Shows menu of orders to fleet

	{
	if (m_pPlayer)
		{
		DWORD dwStatus = m_pPlayer->GetCommsStatus();

		if (dwStatus == 0)
			{
			DisplayMessage(CONSTLIT("No carrier signal"));
			return;
			}

		m_MenuData.SetTitle(CONSTLIT("Squadron Orders"));
		m_MenuData.RemoveAll();

		if (dwStatus & resCanAttackInFormation)
			m_MenuData.AddMenuItem(CONSTLIT("A"), CONSTLIT("Attack in formation"), msgAttackInFormation);

		if (dwStatus & resCanBreakAndAttack)
			m_MenuData.AddMenuItem(CONSTLIT("B"), CONSTLIT("Break & attack"), msgBreakAndAttack);

		if ((dwStatus & resCanFormUp)
				&& !(dwStatus & resCanBeInFormation))
			m_MenuData.AddMenuItem(CONSTLIT("F"), CONSTLIT("Form up"), msgFormUp, 0xffffffff);

		if (dwStatus & resCanAttack)
			m_MenuData.AddMenuItem(CONSTLIT("T"), CONSTLIT("Attack target"), msgAttack);

		if (dwStatus & resCanWait)
			m_MenuData.AddMenuItem(CONSTLIT("W"), CONSTLIT("Wait"), msgWait);

		if (dwStatus & resCanAbortAttack)
			m_MenuData.AddMenuItem(CONSTLIT("X"), CONSTLIT("Cancel attack"), msgAbort);

		if (dwStatus & resCanBeInFormation)
			{
			m_MenuData.AddMenuItem(CONSTLIT("1"), CONSTLIT("Alpha formation"), msgFormUp, 0);
			m_MenuData.AddMenuItem(CONSTLIT("2"), CONSTLIT("Beta formation"), msgFormUp, 1);
			m_MenuData.AddMenuItem(CONSTLIT("3"), CONSTLIT("Gamma formation"), msgFormUp, 2);
			}

		m_MenuDisplay.Invalidate();
		m_CurrentMenu = menuOrders;
		}
	}

ALERROR CTranscendenceWnd::ShowDockScreen (CSpaceObject *pLocation, 
										   CXMLElement *pScreenDesc, 
										   const CString &sPane)

//	ShowDockScreen
//
//	Shows the given screen

	{
	BEGIN_EXCEPTION_HANDLER
		{
		ALERROR error;

		//	If we're not yet docked, then this is the first OnInit

		if (m_State != gsDocked)
			m_CurrentDock.ResetFirstOnInit();

		//	Initialize the current screen object

		if ((error = m_CurrentDock.InitScreen(this, m_rcMainScreen, pLocation, pScreenDesc, sPane, &m_pCurrentScreen)))
			return error;

		//	Change the state, if necessary

		if (m_State != gsDocked)
			{
			//	Show the cursor, if it was previously hidden

			if (m_State == gsInGame)
				SDL_ShowCursor(true);

			//	New state

			m_State = gsDocked;
			}
		}
	END_EXCEPTION_HANDLER

	return NOERROR;
	}

void CTranscendenceWnd::ShowEnableDisablePicker (void)

//	ShowEnableDisablePicker
//
//	Show the picker to select devices to enable/disable

	{
	int i;

	if (m_pPlayer)
		{
		CShip *pShip = m_pPlayer->GetShip();

		//	Fill the menu with all usable items

		m_MenuData.RemoveAll();

		CItemList &List = pShip->GetItemList();
		List.SortItems();

		for (i = 0; i < List.GetCount(); i++)
			{
			CItem &Item = List.GetItem(i);
			CItemType *pType = Item.GetType();
			CInstalledDevice *pDevice = pShip->FindDevice(Item);

			if (pDevice && pDevice->CanBeDisabled())
				{
				//	Name of item

				CString sName;
				if (pDevice->IsEnabled())
					sName = strPatternSubst(CONSTLIT("Disable %s"), Item.GetNounPhrase(0).GetASCIIZPointer());
				else
					sName = strPatternSubst(CONSTLIT("Enable %s"), Item.GetNounPhrase(0).GetASCIIZPointer());

				//	Extra

				CString sExtra;
				if (pDevice->IsEnabled())
					sExtra = CONSTLIT("[Enter] to disable; [Arrows] to select");
				else
					sExtra = CONSTLIT("[Enter] to enable; [Arrows] to select");

				//	Add the item

				m_MenuData.AddMenuItem(NULL_STR,
						sName,
						&pType->GetImage(),
						sExtra,
						i);
				}
			}

		//	If we've got items, then show the picker...

		if (m_MenuData.GetCount() > 0)
			{
			m_PickerDisplay.ResetSelection();
			m_PickerDisplay.Invalidate();
			m_PickerDisplay.SetHelpText(NULL_STR);
			m_CurrentPicker = pickEnableDisableItem;
			}

		//	Otherwise, message

		else
			DisplayMessage(CONSTLIT("No installed devices"));
		}
	}

void CTranscendenceWnd::ShowGameMenu (void)

//	ShowGameMenu
//
//	Show the game menu

	{
	m_MenuData.SetTitle(CONSTLIT("Transcendence"));
	m_MenuData.RemoveAll();
	m_MenuData.AddMenuItem(CONSTLIT("1"), CONSTLIT("Help [F1]"), CMD_PAUSE);
	m_MenuData.AddMenuItem(CONSTLIT("2"), CONSTLIT("Save & Quit"), CMD_SAVE);
	m_MenuData.AddMenuItem(CONSTLIT("3"), CONSTLIT("Self-Destruct"), CMD_SELF_DESTRUCT);
	m_MenuDisplay.Invalidate();
	m_CurrentMenu = menuGame;
	}

void CTranscendenceWnd::ShowUsePicker (void)

//	ShowUsePicker
//
//	Show the picker to select an item to use

	{
	int i;

	if (m_pPlayer)
		{
		//	Fill the menu with all usable items

		m_MenuData.RemoveAll();

		CItemList &List = m_pPlayer->GetShip()->GetItemList();
		List.SortItems();

		for (i = 0; i < List.GetCount(); i++)
			{
			CItem &Item = List.GetItem(i);

			CItemType *pType = Item.GetType();
			if (pType->IsUsableInCockpit()
					&& (!pType->IsUsableOnlyIfInstalled() || Item.IsInstalled())
					&& (!pType->IsUsableOnlyIfUninstalled() || !Item.IsInstalled()))
				{
				CString sCount;
				if (Item.GetCount() > 1)
					sCount = strFromInt(Item.GetCount(), false);

				//	Show the key only if the item is identified

				CString sKey;
				if (pType->IsKnown())
					sKey = pType->GetUseKey();

				//	Name of item

				CString sName = Item.GetNounPhrase(0);
				if (Item.IsInstalled())
					sName.Append(STR_INSTALLED);
				sName = strPatternSubst(CONSTLIT("Use %s"), sName.GetASCIIZPointer());

				//	Add the item

				m_MenuData.AddMenuItem(sKey,
						sName,
						&pType->GetImage(),
						sCount,
						i);
				}
			}

		//	Add items that have a use screen

		for (i = 0; i < List.GetCount(); i++)
			{
			CItem &Item = List.GetItem(i);

			CItemType *pType = Item.GetType();
			if (pType->GetUseScreen())
				{
				CString sCount;
				if (Item.GetCount() > 1)
					sCount = strFromInt(Item.GetCount(), false);

				//	Name of item

				CString sName = Item.GetNounPhrase(0);
				if (Item.IsInstalled())
					sName.Append(STR_INSTALLED);
				sName = strPatternSubst(CONSTLIT("Use %s"), sName.GetASCIIZPointer());

				//	Add the item

				m_MenuData.AddMenuItem(NULL_STR,
						sName,
						&pType->GetImage(),
						sCount,
						i);
				}
			}

		//	If we've got items, then show the picker...

		if (m_MenuData.GetCount() > 0)
			{
			m_pPlayer->ClearShowHelpUse();
			m_PickerDisplay.ResetSelection();
			m_PickerDisplay.SetHelpText(CONSTLIT("[Enter] to use; [Arrows] to select"));
			m_PickerDisplay.Invalidate();
			m_CurrentPicker = pickUsableItem;
			}

		//	Otherwise, message

		else
			DisplayMessage(CONSTLIT("No usable items"));
		}
	}

