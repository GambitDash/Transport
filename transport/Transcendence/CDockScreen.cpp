//	CDockScreen.cpp
//
//	CDockScreen class

#include "PreComp.h"
#include "Transcendence.h"

#include "CGButtonArea.h"
#include "CGImageArea.h"

const int PICKER_ROW_HEIGHT	=	96;
const int PICKER_ROW_COUNT =	4;

const int g_cxDockScreen = 800;
const int g_cyDockScreen = 400;
const int g_cxBackground = 1024;
const int g_cyBackground = 400;
const int DESC_PANE_X =			600;
const int BACKGROUND_FOCUS_X =	(DESC_PANE_X / 2);
const int BACKGROUND_FOCUS_Y =	(g_cyBackground / 2);

const int STATUS_BAR_HEIGHT	=	20;
const int g_cyTitle =			72;
const int g_cxActionsRegion =	400;
const int g_cyAction =			22;
const int g_cyActionSpacing =	4;
const int g_iMaxActions =		8;
const int g_cyActionsRegion =	(g_cyAction * g_iMaxActions) + (g_cyActionSpacing * (g_iMaxActions - 1));

const int g_cyItemTitle =		32;
const int g_cxItemMargin =		132;
const int g_cxItemImage =		96;
const int g_cyItemImage =		96;

const int g_cxStats =			400;
const int g_cyStats =			30;
const int g_cxStatsLabel =		60;
const int g_cxCargoStats =		200;
const int g_cxCargoStatsLabel =	100;

const int g_cxCounter =			128;
const int g_cyCounter =			40;

const int TEXT_INPUT_WIDTH =	380;
const int TEXT_INPUT_HEIGHT	=	40;

const DWORD g_FirstActionID =		100;
const DWORD g_LastActionID =		199;

const DWORD g_PrevActionID =		200;
const DWORD g_NextActionID =		201;
const DWORD g_ItemTitleID =		202;
const DWORD g_ItemDescID =			203;
const DWORD g_CounterID =			204;
const DWORD g_ItemImageID =		205;
const DWORD PICKER_ID	=			206;
const DWORD TEXT_INPUT_ID =		207;
const DWORD IMAGE_AREA_ID =		208;

#define LIST_TAG					CONSTLIT("List")
#define INITIAL_PANE_TAG			CONSTLIT("InitialPane")
#define ON_INIT_TAG					CONSTLIT("OnInit")
#define DISPLAY_TAG					CONSTLIT("Display")
#define TEXT_TAG					CONSTLIT("Text")
#define IMAGE_TAG					CONSTLIT("Image")

#define BACKGROUND_ID_ATTRIB		CONSTLIT("backgroundID")
#define LIST_ATTRIB					CONSTLIT("list")
#define KEY_ATTRIB					CONSTLIT("key")
#define INITIAL_ITEM_ATTRIB			CONSTLIT("initialItem")
#define PANE_ATTRIB					CONSTLIT("pane")
#define NO_LIST_NAVIGATION_ATTRIB	CONSTLIT("noListNavigation")
#define SHOW_TEXT_INPUT_ATTRIB		CONSTLIT("showTextInput")
#define LEFT_ATTRIB					CONSTLIT("left")
#define TOP_ATTRIB					CONSTLIT("top")
#define RIGHT_ATTRIB				CONSTLIT("right")
#define BOTTOM_ATTRIB				CONSTLIT("bottom")
#define WIDTH_ATTRIB				CONSTLIT("width")
#define HEIGHT_ATTRIB				CONSTLIT("height")
#define FONT_ATTRIB					CONSTLIT("font")
#define COLOR_ATTRIB				CONSTLIT("color")
#define TRANSPARENT_ATTRIB			CONSTLIT("transparent")
#define ALIGN_ATTRIB				CONSTLIT("align")
#define VALIGN_ATTRIB				CONSTLIT("valign")
#define ID_ATTRIB					CONSTLIT("id")

#define SCREEN_TYPE_ITEM_PICKER		CONSTLIT("itemPicker")
#define SCREEN_TYPE_CUSTOM_PICKER	CONSTLIT("customPicker")

#define ALIGN_CENTER				CONSTLIT("center")
#define ALIGN_RIGHT					CONSTLIT("right")
#define ALIGN_LEFT					CONSTLIT("left")
#define ALIGN_BOTTOM				CONSTLIT("bottom")
#define ALIGN_TOP					CONSTLIT("top")
#define ALIGN_MIDDLE				CONSTLIT("middle")

static char g_ActionsTag[] = "Actions";
static char g_InitializeTag[] = "Initialize";
static char g_PanesTag[] = "Panes";
static char g_ListOptionsTag[] = "ListOptions";

static char g_NavigateTag[] = "Navigate";
static char g_NavigateScreenAttrib[] = "screen";

static char g_ExitTag[] = "Exit";

static char g_ShowPaneTag[] = "ShowPane";

static char g_NameAttrib[] = "name";
static char g_ActionNameAttrib[] = "name";
static char g_ScreenTypeAttrib[] = "type";
static char g_DescAttrib[] = "desc";
static char g_DataFromAttrib[] = "dataFrom";
static char g_CancelAttrib[] = "cancel";
static char g_DefaultAttrib[] = "default";
static char g_ShowCounterAttrib[] = "showCounter";

static char g_DataFromPlayer[] = "player";
// static char g_DataFromStation[] = "station";

CDockScreen::CDockScreen (void) : CObject(NULL),
		m_pUniv(NULL),
		m_pLocation(NULL),
		m_pDesc(NULL),
		m_pScreen(NULL),
		m_bFirstOnInit(true),
		m_pBackgroundImage(NULL),
		m_bFreeBackgroundImage(false),
		m_pItemListControl(NULL),
		m_iControlCount(0),
		m_pControls(NULL),
		m_pPanes(NULL),
		m_pCurrentPane(NULL),
		m_pCurrentActions(NULL),
		m_pCurrentFrame(NULL),
		m_bInShowPane(false)

//	CDockScreen constructor

	{
	}

CDockScreen::~CDockScreen (void)

//	CDockScreen destructor

	{
	CleanUpScreen();
	}

void CDockScreen::Action (DWORD dwTag, DWORD dwData)

//	Action
//
//	Button pressed

	{
	//	Get the data for the particular button

	switch (dwTag)
		{
		case g_PrevActionID:
			if (!m_bNoListNavigation)
				{
				SelectPrevItem();
				ShowItem();
				ShowPane(EvalInitialPane());
				}
			break;

		case g_NextActionID:
			if (!m_bNoListNavigation)
				{
				SelectNextItem();
				ShowItem();
				ShowPane(EvalInitialPane());
				}
			break;

		case PICKER_ID:
			{
			if (!m_bNoListNavigation)
				{
				if (dwData == ITEM_LIST_AREA_PAGE_UP_ACTION)
					{
					SelectPrevItem();
					SelectPrevItem();
					SelectPrevItem();
					m_pItemListControl->Invalidate();
					ShowPane(EvalInitialPane());
					}
				else if (dwData == ITEM_LIST_AREA_PAGE_DOWN_ACTION)
					{
					SelectNextItem();
					SelectNextItem();
					SelectNextItem();
					m_pItemListControl->Invalidate();
					ShowPane(EvalInitialPane());
					}
				else
					{
					m_pItemListControl->SetCursor(dwData);
					ShowPane(EvalInitialPane());
					}
				}
			break;
			}

		default:
			{
			if (dwTag >= g_FirstActionID && dwTag <= g_LastActionID)
				{
				//	If the action is disabled, then ignore it

				CGButtonArea *pAction = (CGButtonArea *)m_pCurrentFrame->FindArea(dwTag);
				if (pAction->IsDisabled() || !pAction->IsVisible())
					return;

				//	Execute the action

				CXMLElement *pDesc = m_pCurrentActions->GetContentElement(dwTag - g_FirstActionID);
				if (pDesc->GetContentElementCount() > 0)
					{
					CXMLElement *pAction = pDesc->GetContentElement(0);
					ExecuteAction(pAction);
					}
				else
					{
					CString sCode = pDesc->GetContentText(0);
					EvalString(sCode, true);
					}

				break;
				}
			}
		}
	}

void CDockScreen::CleanUpScreen (void)

//	CleanUpScreen
//
//	Called to bring the screen down after InitScreen

	{
	int i;

	if (m_pScreen)
		{
		delete m_pScreen;
		m_pScreen = NULL;

		//	Note: No need to free any of the controls because deleting
		//	the screen will free them.
		m_pCurrentFrame = NULL;
		m_pFrameDesc = NULL;
		m_pCounter = NULL;
		m_pTextInput = NULL;
		m_pItemListControl = NULL;
		}

	if (m_pBackgroundImage)
		{
		if (m_bFreeBackgroundImage)
			delete m_pBackgroundImage;

		m_pBackgroundImage = NULL;
		}

	if (m_pControls)
		{
		CCodeChain &CC = m_pUniv->GetCC();
		for (i = 0; i < m_iControlCount; i++)
			{
			if (m_pControls[i].pCode)
				m_pControls[i].pCode->Discard(&CC);
			}

		delete [] m_pControls;
		m_pControls = NULL;
		m_iControlCount = 0;
		}

	m_bInShowPane = false;
	}

ALERROR CDockScreen::CreateBackgroundImage (CXMLElement *pDesc)

//	CreateBackgroundImage
//
//	Creates the background image to use for the dock screen. Initializes
//	m_pBackgroundImage and m_bFreeBackgroundImage

	{
	enum BackgroundTypes { backgroundNone, backgroundImage, backgroundObj };

	ASSERT(m_pLocation);
	ASSERT(m_pBackgroundImage == NULL);

	BackgroundTypes iType = backgroundNone;
	DWORD dwBackgroundID = 0;

	//	Figure out which background to use

	CString sBackgroundID;
	if (pDesc->FindAttribute(BACKGROUND_ID_ATTRIB, &sBackgroundID))
		{
		//	If the attribute exists, but is empty (or equals "none") then
		//	we don't have a background

		if (sBackgroundID.IsBlank() || strEquals(sBackgroundID, CONSTLIT("none")))
			iType = backgroundNone;

		//	If the ID is "object" then we should the object

		else if (strEquals(sBackgroundID, CONSTLIT("object")))
			iType = backgroundObj;

		//	Otherwise, we expect an integer

		else
			{
			iType = backgroundImage;
			dwBackgroundID = strToInt(sBackgroundID, 0);
			}
		}

	//	If no attribute specified, ask the location

	else
		{
		dwBackgroundID = m_pLocation->GetDefaultBkgnd();
		iType = (dwBackgroundID ? backgroundImage : backgroundObj);
		}

	//	Load the image

	CG16bitImage *pImage = NULL;
	if (dwBackgroundID)
		pImage = m_pUniv->GetLibraryBitmap(dwBackgroundID);

	//	Sometimes (like in the case of item lists) the image is larger than normal

	int cyExtra = 0;
	if (pImage)
		cyExtra = Max(pImage->GetHeight() - g_cyBackground, 0);

	//	Create a new image for the background

	m_pBackgroundImage = new CG16bitImage;
	m_bFreeBackgroundImage = true;
	m_pBackgroundImage->CreateBlank(g_cxBackground, g_cyBackground + cyExtra, false);

	if (cyExtra)
		m_pBackgroundImage->Fill(0, g_cyBackground, g_cxBackground, cyExtra, 0);

	//	Load the dock screen background based on the ship class

	DWORD dwScreenUNID = DEFAULT_DOCK_SCREEN_IMAGE_UNID;
	DWORD dwScreenMaskUNID = DEFAULT_DOCK_SCREEN_MASK_UNID;
	CG16bitImage *pScreenImage = m_pUniv->GetLibraryBitmap(dwScreenUNID);

	//	Blt to background

	if (pScreenImage)
		m_pBackgroundImage->Blt(0, 0, g_cxBackground, g_cyBackground, *pScreenImage, 0, 0);

	//	If not image, then we're done

	if (iType == backgroundNone)
		;

	//	Paint the object as the background

	else if (iType == backgroundObj)
		{
		SViewportPaintCtx Ctx;
		Ctx.fNoSelection = true;
		Ctx.pObj = m_pLocation;
		m_pLocation->Paint(*m_pBackgroundImage,
				BACKGROUND_FOCUS_X,
				BACKGROUND_FOCUS_Y,
				Ctx);
		}

	//	If we have an image with a mask, just blt the masked image

	else if (pImage && pImage->HasMask())
		m_pBackgroundImage->ColorTransBlt(0, 0, pImage->GetWidth(), pImage->GetHeight(), 255, *pImage, 0, 0);

	//	If we have an image with no mask, then we need to create our own mask

	else if (pImage)
		{
		CG16bitImage *pScreenMask = m_pUniv->GetLibraryBitmap(dwScreenMaskUNID);
		if (pScreenMask)
			m_pBackgroundImage->BltWithMask(0, 0, pImage->GetWidth(), pImage->GetHeight(), *pScreenMask, *pImage, 0, 0);
		else
			m_pBackgroundImage->Blt(0, 0, pImage->GetWidth(), pImage->GetHeight(), *pImage, 0, 0);
		}

	return NOERROR;
	}

ALERROR CDockScreen::CreateItemPickerControl (CXMLElement *pDesc, AGScreen *pScreen, const RECT &rcRect)

//	CreateItemPickerControl
//
//	Creates control for item picker screen

	{
	//	Calculate some basic metrics

	int xScreen = rcRect.left;
	int yScreen = (RectHeight(rcRect) - g_cyDockScreen) / 2;

	//	Create the picker control

	m_pItemListControl = new CGItemListArea;
	if (m_pItemListControl == NULL)
		return ERR_MEMORY;

	m_pItemListControl->SetFontTable(m_pFonts);

	RECT rcPicker;
	rcPicker.left = xScreen + 12;
	rcPicker.top = yScreen + 12;
	rcPicker.right = xScreen + DESC_PANE_X - 44;
	rcPicker.bottom = rcPicker.top + (PICKER_ROW_COUNT * PICKER_ROW_HEIGHT);
	pScreen->AddArea(m_pItemListControl, rcPicker, PICKER_ID);

	return NOERROR;
	}

ALERROR CDockScreen::CreateTitleAndBackground (CXMLElement *pDesc, AGScreen *pScreen, const RECT &rcRect)

//	CreateTitleAndBackground
//
//	Creates a standard screen based on the screen descriptor element

	{
	//	Generate a background image

	CreateBackgroundImage(pDesc);
	int cyBackgroundImage = (m_pBackgroundImage ? m_pBackgroundImage->GetHeight() : 512);

	//	Add the background

	RECT rcBackArea;
	CGImageArea *pImage = NULL;

	if (m_pBackgroundImage)
		{
		pImage = new CGImageArea;
		RECT rcImage;
		rcImage.left = 0;
		rcImage.top = 0;
		rcImage.right = g_cxBackground;
		rcImage.bottom = cyBackgroundImage;
		pImage->SetImage(m_pBackgroundImage, rcImage);
		}

	rcBackArea.left = 0;
	rcBackArea.top = (RectHeight(rcRect) - g_cyDockScreen) / 2;
	rcBackArea.right = rcBackArea.left + g_cxBackground;
	rcBackArea.bottom = rcBackArea.top + cyBackgroundImage;

	if (pImage)
		pScreen->AddArea(pImage, rcBackArea, IMAGE_AREA_ID);

	//	Add a background bar to the title part

	pImage = new CGImageArea;
	pImage->SetBackColor(m_pFonts->wAltBlueBackground);
	RECT rcArea;
	rcArea.left = 0;
	rcArea.top = rcBackArea.top - g_cyTitle;
	rcArea.right = rcRect.right;
	rcArea.bottom = rcBackArea.top - STATUS_BAR_HEIGHT;
	pScreen->AddArea(pImage, rcArea, 0);

	pImage = new CGImageArea;
	pImage->SetBackColor(CG16bitImage::DarkenPixel(m_pFonts->wAltBlueBackground, 200));
	rcArea.left = 0;
	rcArea.top = rcBackArea.top - STATUS_BAR_HEIGHT;
	rcArea.right = rcRect.right;
	rcArea.bottom = rcBackArea.top;
	pScreen->AddArea(pImage, rcArea, 0);

	//	Get the name of this location

	CString sName = EvalString(pDesc->GetAttribute(CONSTLIT(g_NameAttrib)));

	//	Add the name as a title to the screen

	CGTextArea *pText = new CGTextArea;
	pText->SetText(sName);
	pText->SetFont(&m_pFonts->Title);
	pText->SetColor(m_pFonts->wTitleColor);
	pText->AddShadowEffect();
	rcArea.left = 8;
	rcArea.top = rcBackArea.top - g_cyTitle;
	rcArea.right = rcRect.right;
	rcArea.bottom = rcBackArea.top;
	pScreen->AddArea(pText, rcArea, 0);

	//	Add the credits label

	int cyOffset = (STATUS_BAR_HEIGHT - m_pFonts->MediumHeavyBold.GetHeight()) / 2;

	pText = new CGTextArea;
	pText->SetText(CONSTLIT("Credits:"));
	pText->SetFont(&m_pFonts->MediumHeavyBold);
	pText->SetColor(m_pFonts->wTitleColor);

	rcArea.left = rcRect.right - g_cxStats;
	rcArea.top = rcBackArea.top - STATUS_BAR_HEIGHT + cyOffset;
	rcArea.right = rcRect.right;
	rcArea.bottom = rcArea.top + g_cyStats;
	pScreen->AddArea(pText, rcArea, 0);

	//	Add credits area

	m_pCredits = new CGTextArea;
	m_pCredits->SetFont(&m_pFonts->MediumHeavyBold);
	m_pCredits->SetColor(m_pFonts->wTitleColor);

	rcArea.left = rcRect.right - g_cxStats + g_cxStatsLabel;
	rcArea.top = rcBackArea.top - STATUS_BAR_HEIGHT + cyOffset;
	rcArea.right = rcRect.right;
	rcArea.bottom = rcArea.top + g_cyStats;
	pScreen->AddArea(m_pCredits, rcArea, 0);

	//	Add the cargo space label

	pText = new CGTextArea;
	pText->SetText(CONSTLIT("Cargo Space:"));
	pText->SetFont(&m_pFonts->MediumHeavyBold);
	pText->SetColor(m_pFonts->wTitleColor);

	rcArea.left = rcRect.right - g_cxCargoStats;
	rcArea.top = rcBackArea.top - STATUS_BAR_HEIGHT + cyOffset;
	rcArea.right = rcRect.right;
	rcArea.bottom = rcArea.top + g_cyStats;
	pScreen->AddArea(pText, rcArea, 0);

	//	Add the cargo space area

	m_pCargoSpace = new CGTextArea;
	m_pCargoSpace->SetFont(&m_pFonts->MediumHeavyBold);
	m_pCargoSpace->SetColor(m_pFonts->wTitleColor);

	rcArea.left = rcRect.right - g_cxCargoStats + g_cxCargoStatsLabel;
	rcArea.top = rcBackArea.top - STATUS_BAR_HEIGHT + cyOffset;
	rcArea.right = rcRect.right;
	rcArea.bottom = rcArea.top + g_cyStats;
	pScreen->AddArea(m_pCargoSpace, rcArea, 0);

	UpdateCredits();

	return NOERROR;
	}

void CDockScreen::DeleteCurrentItem (int iCount)

//	DeleteCurrentItem
//
//	Deletes the given number of items at the cursor

	{
	m_pItemListControl->DeleteAtCursor(iCount);
	ShowItem();
	}

int CDockScreen::GetCounter (void)

//	GetCounter
//
//	Returns the value of the counter field

	{
	if (m_pCounter)
		return strToInt(m_pCounter->GetText(), 0, NULL);
	else
		return 0;
	}

const CItem &CDockScreen::GetCurrentItem (void)

//	GetCurrentItem
//
//	Returns the current item at the cursor

	{
	return m_pItemListControl->GetItemAtCursor();
	}

ICCItem *CDockScreen::GetCurrentListEntry (void)

//	GetCurrentListEntry
//
//	Returns the current list entry

	{
	return m_pItemListControl->GetEntryAtCursor();
	}

CString CDockScreen::GetTextInput (void)

//	GetTextInput
//
//	Returns the value of the input field

	{
	if (m_pTextInput)
		return m_pTextInput->GetText();
	else
		return NULL_STR;
	}

bool CDockScreen::IsCurrentItemValid (void)

//	IsCurrentItemValid
//
//	Returns TRUE if current item is valid

	{
	return m_pItemListControl->IsCursorValid();
	}

bool CDockScreen::EvalBool (const CString &sCode)

//	EvalBool
//
//	Evaluates the given CodeChain code.

	{
	CCodeChain &CC = m_pUniv->GetCC();
	CodeChainCtx Ctx;

	Ctx.pUniv = m_pUniv;
	Ctx.pTrans = m_pTrans;
	Ctx.pScreen = this;

	ICCItem *pExp = CC.Link(sCode, 1, NULL);

	ICCItem *pResult = CC.TopLevel(pExp, &Ctx);
	pExp->Discard(&CC);

	if (pResult->IsError())
		{
		SetDescription(pResult->GetStringValue());
		}

	bool bResult = !pResult->IsNil();
	pResult->Discard(&CC);

	return bResult;
	}

CString CDockScreen::EvalInitialPane (void)

//	EvalInitialPane
//
//	Invokes <InitialPane> code and returns the result (or "Default")

	{
	CXMLElement *pInitialPane = m_pDesc->GetContentElementByTag(INITIAL_PANE_TAG);
	if (pInitialPane)
		{
		CString sCode = pInitialPane->GetContentText(0);
		return EvalString(sCode, true);
		}
	else
		return CONSTLIT("Default");
	}

CString CDockScreen::EvalString (const CString &sString, bool bPlain)

//	EvalString
//
//	Evaluates a string using CodeChain. A string that begins
//	with an equals sign indicates a CodeChain expression.

	{
	CCodeChain &CC = m_pUniv->GetCC();
	char *pPos = sString.GetPointer();

	if (bPlain || *pPos == '=')
		{
		CodeChainCtx Ctx;

		Ctx.pUniv = m_pUniv;
		Ctx.pTrans = m_pTrans;
		Ctx.pScreen = this;

		ICCItem *pExp = CC.Link(sString, (bPlain ? 0 : 1), NULL);

		ICCItem *pResult = CC.TopLevel(pExp, &Ctx);
		pExp->Discard(&CC);

		if (pResult->IsError())
			{
			CString sError = pResult->GetStringValue();

			SetDescription(sError);
			kernelDebugLogMessage(sError.GetASCIIZPointer());
			}

		//	Note: We use GetStringValue instead of Unlink because we don't
		//	want to preserve CC semantics (e.g., we don't need strings to
		//	be quoted).

		CString sResult = pResult->GetStringValue();
		pResult->Discard(&CC);

		return sResult;
		}
	else
		return strCEscapeCodes(sString);
	}

void CDockScreen::EnableAction (int iAction, bool bEnabled)

//	EnableAction
//
//	Enable or disable the given action

	{
	if (m_pCurrentFrame)
		{
		CGButtonArea *pAction = (CGButtonArea *)m_pCurrentFrame->FindArea(g_FirstActionID + iAction);
		if (pAction)
			pAction->SetDisabled(!bEnabled);
		}
	}

void CDockScreen::ExecuteAction (CXMLElement *pAction)

//	ExecuteAction
//
//	Performs the given action

	{
	if (strEquals(pAction->GetTag(), CONSTLIT(g_NavigateTag)))
		{
		ShowScreen(pAction->GetAttribute(CONSTLIT(g_NavigateScreenAttrib)), 
				pAction->GetAttribute(PANE_ATTRIB));
		}
	else if (strEquals(pAction->GetTag(), CONSTLIT(g_ExitTag)))
		{
		Undock();
		}
	else if (strEquals(pAction->GetTag(), CONSTLIT(g_ShowPaneTag)))
		{
		ShowPane(pAction->GetAttribute(PANE_ATTRIB));
		}
	}

CDockScreen::SDisplayControl *CDockScreen::FindDisplayControl (const CString &sID)

//	FindDisplayControl
//
//	Returns the display control with the given ID (or NULL if not found)

	{
	int i;

	if (m_pControls)
		{
		for (i = 0; i < m_iControlCount; i++)
			{
			if (strEquals(sID, m_pControls[i].sID))
				return &m_pControls[i];
			}
		}

	return NULL;
	}

void CDockScreen::HandleChar (char chChar)

//	HandleChar
//
//	Handle char events

	{
	//	Deal with input fields

	if (m_pTextInput)
		{
		/* ??? Not really sure what is going on here; values chosen magically... */
		if (isprint(chChar))
			{
			CString sText = m_pTextInput->GetText();
			sText.Append(CString(&chChar, 1));
			m_pTextInput->SetText(sText);
			return;
			}
		}
	else if (m_pCounter && isdigit(chChar))
		{
		switch (chChar)
			{
			case SDLK_0:
			case SDLK_1:
			case SDLK_2:
			case SDLK_3:
			case SDLK_4:
			case SDLK_5:
			case SDLK_6:
			case SDLK_7:
			case SDLK_8:
			case SDLK_9:
				if (m_bReplaceCounter)
					{
					m_pCounter->SetText(strFromInt(chChar - SDLK_0, false));
					m_bReplaceCounter = false;
					}
				else
					{
					CString sCounter = m_pCounter->GetText();
					sCounter.Append(strFromInt(chChar - SDLK_0, false));
					m_pCounter->SetText(sCounter);
					}
				break;
			default:
				break;
			}
		return;
		}

	//	Deal with accelerators
	//	Check to see if one of the key matches one of the accelerators
	for (int i = 0; i < m_pCurrentActions->GetContentElementCount(); i++)
		{
		CGButtonArea *pAction = (CGButtonArea *)m_pCurrentFrame->FindArea(g_FirstActionID + i);
		if (pAction->IsVisible() && !pAction->IsDisabled())
			{
			CString sKey = strToUpper(pAction->GetLabelAccelerator());
			char chAccel = *sKey.GetPointer();
			if (chChar == chAccel
					|| chChar == (chAccel - 'A' + 'a'))
				{
				Action(g_FirstActionID + i);
				break;
				}
			}
		}
	}

void CDockScreen::HandleKeyDown (int iVirtKey)

//	HandleKeyDown
//
//	Handle key down events

	{
	int i;

	switch (iVirtKey)
		{
		case SDLK_UP:
		case SDLK_LEFT:
			if (m_pItemListControl)
				Action(g_PrevActionID);
			break;

		case SDLK_DOWN:
		case SDLK_RIGHT:
			if (m_pItemListControl)
				Action(g_NextActionID);
			break;

		case SDLK_PAGEUP:
			if (m_pItemListControl)
				Action(PICKER_ID, ITEM_LIST_AREA_PAGE_UP_ACTION);
			break;

		case SDLK_PAGEDOWN:
			if (m_pItemListControl)
				Action(PICKER_ID, ITEM_LIST_AREA_PAGE_DOWN_ACTION);
			break;

		case SDLK_BACKSPACE:
			{
			if (m_pTextInput)
				{
				CString sText = m_pTextInput->GetText();
				if (sText.GetLength() > 1)
					m_pTextInput->SetText(strSubString(sText, 0, sText.GetLength() - 1));
				else
					m_pTextInput->SetText(NULL_STR);
				}
			else if (m_pCounter)
				{
				CString sCounter = m_pCounter->GetText();
				if (sCounter.GetLength() > 1)
					{
					m_pCounter->SetText(strSubString(sCounter, 0, sCounter.GetLength() - 1));
					m_bReplaceCounter = false;
					}
				else
					{
					m_pCounter->SetText(CONSTLIT("0"));
					m_bReplaceCounter = true;
					}
				}
			break;
			}

		case SDLK_ESCAPE:
			{
			//	Find the action that is marked as cancel
			for (i = 0; i < m_pCurrentActions->GetContentElementCount(); i++)
				{
				CXMLElement *pActionDesc = m_pCurrentActions->GetContentElement(i);
				if (pActionDesc->GetAttributeInteger(CONSTLIT(g_CancelAttrib)) > 0)
					{
					CGButtonArea *pAction = (CGButtonArea *)m_pCurrentFrame->FindArea(g_FirstActionID + i);
					if (pAction->IsVisible() && !pAction->IsDisabled())
						{
						Action(g_FirstActionID + i);
						break;
						}
					}
				}

			break;
			}

		case SDLK_RETURN:
			{
			//	Find the action that is marked as default
			for (i = 0; i < m_pCurrentActions->GetContentElementCount(); i++)
				{
				CXMLElement *pActionDesc = m_pCurrentActions->GetContentElement(i);
				if (pActionDesc->GetAttributeInteger(CONSTLIT(g_DefaultAttrib)) > 0)
					{
					CGButtonArea *pAction = (CGButtonArea *)m_pCurrentFrame->FindArea(g_FirstActionID + i);
					if (pAction->IsVisible() && !pAction->IsDisabled())
						{
						Action(g_FirstActionID + i);
						break;
						}
					}
				}

			break;
			}
		default:
			;
		}
	}

ALERROR CDockScreen::InitCodeChain (CTranscendenceWnd *pTrans, CSpaceObject *pStation)

//	InitCodeChain
//
//	Initializes CodeChain language

	{
	CCodeChain &CC = m_pUniv->GetCC();

	//	Define some globals

	CC.DefineGlobalInteger(CONSTLIT("gSource"), (int)pStation);
	CC.DefineGlobalInteger(CONSTLIT("gScreen"), (int)this);

	return NOERROR;
	}

ALERROR CDockScreen::InitCustomList (void)

//	InitCustomList
//
//	Initializes the custom list for this screen

	{
	//	Get the list element

	CXMLElement *pListData = m_pDesc->GetContentElementByTag(LIST_TAG);
	if (pListData == NULL)
		return ERR_FAIL;

	//	Get the list to show

	CCodeChain &CC = m_pUniv->GetCC();
	ICCItem *pExp = CC.Link(pListData->GetContentText(0), 0, NULL);

	//	Evaluate the function

	CodeChainCtx Ctx;
	Ctx.pUniv = m_pUniv;
	Ctx.pTrans = m_pTrans;
	Ctx.pScreen = this;

	ICCItem *pResult = CC.TopLevel(pExp, &Ctx);
	pExp->Discard(&CC);

	if (pResult->IsError())
		{
		CString sError = pResult->GetStringValue();

		SetDescription(sError);
		kernelDebugLogMessage(sError.GetASCIIZPointer());

		return NOERROR;
		}

	//	Set this expression as the list

	m_pItemListControl->SetList(CC, pResult);
	pResult->Discard(&CC);

	//	Position the cursor on the next relevant item

	SelectNextItem();

	//	Give the screen a chance to start at a different item (other
	//	than the first)

	CString sInitialItemFunc = pListData->GetAttribute(INITIAL_ITEM_ATTRIB);
	if (!sInitialItemFunc.IsBlank())
		{
		bool bMore = IsCurrentItemValid();
		while (bMore && !EvalBool(sInitialItemFunc))
			SelectNextItem(&bMore);
		}

	return NOERROR;
	}

ALERROR CDockScreen::InitDisplay (CXMLElement *pDisplayDesc, AGScreen *pScreen, const RECT &rcScreen)

//	InitDisplay
//
//	Initializes display controls

	{
	int i;
	CCodeChain &CC = m_pUniv->GetCC();

	ASSERT(m_pControls == NULL);

	//	Allocate the controls

	m_iControlCount = pDisplayDesc->GetContentElementCount();
	if (m_iControlCount == 0)
		return NOERROR;

	m_pControls = new SDisplayControl [m_iControlCount];

	//	Compute the canvas rect for the controls (relative to pScreen)

	RECT rcCanvas;
	rcCanvas.left = 0;
	rcCanvas.top = (RectHeight(rcScreen) - g_cyBackground) / 2;
	rcCanvas.right = rcCanvas.left + DESC_PANE_X;
	rcCanvas.bottom = rcCanvas.top + g_cyBackground;

	//	Create each control

	for (i = 0; i < m_iControlCount; i++)
		{
		CXMLElement *pControlDesc = pDisplayDesc->GetContentElement(i);
		m_pControls[i].pArea = NULL;
		m_pControls[i].pCode = NULL;

		//	Set the ID

		m_pControls[i].sID = pControlDesc->GetAttribute(ID_ATTRIB);

		//	Control rect relative to canvas

		RECT rcRect;

		rcRect.left = pControlDesc->GetAttributeInteger(LEFT_ATTRIB);
		rcRect.top = pControlDesc->GetAttributeInteger(TOP_ATTRIB);
		rcRect.right = pControlDesc->GetAttributeInteger(RIGHT_ATTRIB);
		rcRect.bottom = pControlDesc->GetAttributeInteger(BOTTOM_ATTRIB);

		if (rcRect.right < 0)
			rcRect.right = RectWidth(rcCanvas) + rcRect.right;

		if (rcRect.bottom < 0)
			rcRect.bottom = RectHeight(rcCanvas) + rcRect.bottom;

		int cxWidth = pControlDesc->GetAttributeInteger(WIDTH_ATTRIB);
		if (cxWidth)
			{
			if (rcRect.right == 0)
				rcRect.right = rcRect.left + cxWidth;
			else if (rcRect.left == 0)
				rcRect.left = rcRect.right - cxWidth;
			}

		int cyHeight = pControlDesc->GetAttributeInteger(HEIGHT_ATTRIB);
		if (cyHeight)
			{
			if (rcRect.bottom == 0)
				rcRect.bottom = rcRect.top + cyHeight;
			else if (rcRect.top == 0)
				rcRect.top = rcRect.bottom - cyHeight;
			}

		//	Make sure we don't exceed the canvas

		rcRect.left = Min(RectWidth(rcCanvas), Max(0, (int)rcRect.left));
		rcRect.right = Min(RectWidth(rcCanvas), (int)Max(rcRect.left, rcRect.right));
		rcRect.top = Min(RectHeight(rcCanvas), Max(0, (int)rcRect.top));
		rcRect.bottom = Min(RectHeight(rcCanvas), (int)Max(rcRect.top, rcRect.bottom));

		rcRect.left += rcCanvas.left;
		rcRect.right += rcCanvas.left;
		rcRect.top += rcCanvas.top;
		rcRect.bottom += rcCanvas.top;

		//	Get the font

		const CG16bitFont *pControlFont = &m_pFonts->Large;
		CString sFontName;
		if (pControlDesc->FindAttribute(FONT_ATTRIB, &sFontName))
			pControlFont = &GetFontByName(*m_pFonts, sFontName);

		COLORREF wControlColor;
		CString sColorName;
		if (pControlDesc->FindAttribute(COLOR_ATTRIB, &sColorName))
			wControlColor = ::LoadRGBColor(sColorName);
		else
			wControlColor = CG16bitImage::RGBValue(255, 255, 255);

		//	Create the control based on the type

		if (strEquals(pControlDesc->GetTag(), TEXT_TAG))
			{
			m_pControls[i].iType = ctrlText;

			CGTextArea *pControl = new CGTextArea;
			pControl->SetFont(pControlFont);
			pControl->SetColor(wControlColor);

			CString sAlign = pControlDesc->GetAttribute(ALIGN_ATTRIB);
			if (strEquals(sAlign, ALIGN_CENTER))
				pControl->SetStyles(alignCentered);
			else if (strEquals(sAlign, ALIGN_RIGHT))
				pControl->SetStyles(alignRight);

			pScreen->AddArea(pControl, rcRect, 0);
			m_pControls[i].pArea = pControl;

			//	Load the text code

			m_pControls[i].pCode = CC.Link(pControlDesc->GetContentText(0), 0, NULL);
			}
		else if (strEquals(pControlDesc->GetTag(), IMAGE_TAG))
			{
			m_pControls[i].iType = ctrlImage;

			CGImageArea *pControl = new CGImageArea;
			pControl->SetTransBackground(pControlDesc->GetAttributeBool(TRANSPARENT_ATTRIB));

			DWORD dwStyles = 0;
			CString sAlign = pControlDesc->GetAttribute(ALIGN_ATTRIB);
			if (strEquals(sAlign, ALIGN_CENTER))
				dwStyles |= alignCentered;
			else if (strEquals(sAlign, ALIGN_RIGHT))
				dwStyles |= alignRight;
			else
				dwStyles |= alignLeft;

			sAlign = pControlDesc->GetAttribute(VALIGN_ATTRIB);
			if (strEquals(sAlign, ALIGN_CENTER))
				dwStyles |= alignMiddle;
			else if (strEquals(sAlign, ALIGN_BOTTOM))
				dwStyles |= alignBottom;
			else
				dwStyles |= alignTop;

			pControl->SetStyles(dwStyles);

			pScreen->AddArea(pControl, rcRect, 0);
			m_pControls[i].pArea = pControl;

			//	Load the code that returns the image

			m_pControls[i].pCode = CC.Link(pControlDesc->GetContentText(0), 0, NULL);
			}
		}

	return NOERROR;
	}

ALERROR CDockScreen::InitItemList (void)

//	InitItemList
//
//	Initializes the item list for this screen

	{
	CSpaceObject *pListSource;

	//	Get the list options element

	CXMLElement *pOptions = m_pDesc->GetContentElementByTag(CONSTLIT(g_ListOptionsTag));
	if (pOptions == NULL)
		return ERR_FAIL;

	//	Figure out where to get the data from: either the station
	//	or the player's ship.

	if (strEquals(pOptions->GetAttribute(CONSTLIT(g_DataFromAttrib)), CONSTLIT(g_DataFromPlayer)))
		pListSource = m_pPlayer->GetShip();
	else
		pListSource = m_pLocation;

	//	Set the list control

	m_pItemListControl->SetList(pListSource);

	//	Initialize flags that control what items we will show

	CItem::ParseCriteria(EvalString(pOptions->GetAttribute(LIST_ATTRIB)), &m_ItemCriteria);
	m_pItemListControl->SetFilter(m_ItemCriteria);

	//	If we have content, then eval the function (note that this might
	//	re-enter and set the filter)

	CString sCode = pOptions->GetContentText(0);
	if (!sCode.IsBlank())
		EvalString(sCode, true);

	//	Position the cursor on the next relevant item

	SelectNextItem();

	//	Give the screen a chance to start at a different item (other
	//	than the first)

	CString sInitialItemFunc = pOptions->GetAttribute(INITIAL_ITEM_ATTRIB);
	if (!sInitialItemFunc.IsBlank())
		{
		bool bMore = IsCurrentItemValid();
		while (bMore && !EvalBool(sInitialItemFunc))
			SelectNextItem(&bMore);
		}

	return NOERROR;
	}

ALERROR CDockScreen::InitScreen (CTranscendenceWnd *pTrans, 
								 RECT &rcRect, 
								 CSpaceObject *pLocation, 
								 CXMLElement *pDesc, 
								 const CString &sPane,
								 AGScreen **retpScreen)

//	InitScreen
//
//	Initializes the docking screen. Returns an AGScreen object
//	that has been initialized appropriately.

	{
	ALERROR error;

	//	Make sure we clean up first

	CleanUpScreen();
	m_pFonts = &pTrans->GetFonts();

	//	Init some variables

	m_pTrans = pTrans;
	m_pLocation = pLocation;
	m_pPlayer = m_pTrans->GetPlayer();
	m_pDesc = pDesc;
	m_pUniv = g_pUniverse;

	//	Initialize CodeChain processor

	error = InitCodeChain(m_pTrans, m_pLocation);
	if (error)
		return error;

	//	Give the screen a chance to re-direct

	CXMLElement *pOnInit = m_pDesc->GetContentElementByTag(ON_INIT_TAG);
	if (pOnInit)
		{
		CString sCode = pOnInit->GetContentText(0);
		EvalString(sCode, true);
		m_bFirstOnInit = false;

		//	If we've already got a screen set up then we don't need to
		//	continue (OnInit has navigated to a different screen).

		if (m_pScreen)
			return NOERROR;
		}

	m_bFirstOnInit = false;

	//	Create a new screen

	m_pScreen = new AGScreen(rcRect);
	m_pScreen->SetController(this);

	RECT rcScreen;
	rcScreen.left = 0;
	rcScreen.top = 0;
	rcScreen.right = RectWidth(rcRect);
	rcScreen.bottom = RectHeight(rcRect);

	//	Creates the title and background controls

	error = CreateTitleAndBackground(m_pDesc, m_pScreen, rcScreen);
	if (error)
		return error;

	//	Get the list of panes for this screen

	m_pPanes = m_pDesc->GetContentElementByTag(CONSTLIT(g_PanesTag));

	//	If this is an item list screen then add the appropriate
	//	controls.

	CString sType = m_pDesc->GetAttribute(CONSTLIT(g_ScreenTypeAttrib));
	if (strEquals(sType, SCREEN_TYPE_ITEM_PICKER))
		{
		error = CreateItemPickerControl(m_pDesc, m_pScreen, rcScreen);
		if (error)
			return error;

		//	Create an item list manipulator for the items of this object

		error = InitItemList();
		if (error)
			return error;
		}
	else if (strEquals(sType, SCREEN_TYPE_CUSTOM_PICKER))
		{
		error = CreateItemPickerControl(m_pDesc, m_pScreen, rcScreen);
		if (error)
			return error;

		//	Create an item list manipulator for the items of this object

		error = InitCustomList();
		if (error)
			return error;
		}

	//	If we have a display element, then load the display controls

	CXMLElement *pDisplay = m_pDesc->GetContentElementByTag(DISPLAY_TAG);
	if (pDisplay)
		{
		error = InitDisplay(pDisplay, m_pScreen, rcScreen);
		if (error)
			return error;
		}

	//	Show the pane

	m_rcPane.left = rcScreen.right - g_cxActionsRegion;
	m_rcPane.top = (RectHeight(rcScreen) - g_cyDockScreen) / 2;
	m_rcPane.right = rcScreen.right - 8;
	m_rcPane.bottom = m_rcPane.top + g_cyBackground;

	if (!sPane.IsBlank())
		ShowPane(sPane);
	else
		ShowPane(EvalInitialPane());

	//	Done

	*retpScreen = m_pScreen;

	return NOERROR;
	}

void CDockScreen::PositionActionButtons (int yPos)

//	PositionActionButtons
//
//	Positions action buttons appropriately

	{
	int i;

	int y = yPos;
	for (i = 0; i < m_pCurrentActions->GetContentElementCount(); i++)
		{
		AGArea *pAction = m_pCurrentFrame->FindArea(g_FirstActionID + i);

		if (pAction->IsVisible())
			{
			RECT rcRect;
			rcRect.left = m_rcPane.left;
			rcRect.top = y;
			rcRect.right = m_rcPane.right;
			rcRect.bottom = y + g_cyAction;
			pAction->SetRect(rcRect);

			y += g_cyAction + g_cyActionSpacing;
			}
		}
	}

void CDockScreen::ResetList (void)

//	ResetList
//
//	Resets the display list

	{
	if (m_pItemListControl)
		{
		m_pItemListControl->ResetCursor();
		ShowItem();
		ShowPane(EvalInitialPane());
		}
	}

void CDockScreen::ShowAction (int iAction, bool bShow)

//	ShowAction
//
//	Show or hide the given action

	{
	if (m_pCurrentFrame)
		{
		CGButtonArea *pAction = (CGButtonArea *)m_pCurrentFrame->FindArea(g_FirstActionID + iAction);
		if (pAction)
			pAction->ShowHide(bShow);
		}
	}

void CDockScreen::ShowDisplay (void)

//	ShowDisplay
//
//	Updates the controls on the display

	{
	int i;
	CCodeChain &CC = m_pUniv->GetCC();

	for (i = 0; i < m_iControlCount; i++)
		{
		switch (m_pControls[i].iType)
			{
			case ctrlText:
				{
				CGTextArea *pControl = (CGTextArea *)m_pControls[i].pArea;

				if (m_pControls[i].pCode)
					{
					CodeChainCtx Ctx;

					Ctx.pUniv = m_pUniv;
					Ctx.pTrans = m_pTrans;
					Ctx.pScreen = this;

					ICCItem *pResult = CC.TopLevel(m_pControls[i].pCode, &Ctx);

					//	The result is the text for the control

					pControl->SetText(pResult->GetStringValue());

					//	If we have an error, report it as well

					if (pResult->IsError())
						{
						kernelDebugLogMessage(pResult->GetStringValue().GetASCIIZPointer());
						}

					//	Done

					pResult->Discard(&CC);
					}

				break;
				}

			case ctrlImage:
				{
				CGImageArea *pControl = (CGImageArea *)m_pControls[i].pArea;

				if (m_pControls[i].pCode)
					{
					CodeChainCtx Ctx;

					Ctx.pUniv = m_pUniv;
					Ctx.pTrans = m_pTrans;
					Ctx.pScreen = this;

					ICCItem *pResult = CC.TopLevel(m_pControls[i].pCode, &Ctx);

					//	If we have an error, report it

					if (pResult->IsError())
						{
						kernelDebugLogMessage(pResult->GetStringValue().GetASCIIZPointer());
						pResult->Discard(&CC);
						break;
						}

					//	The result is the image descriptor

					CG16bitImage *pImage;
					RECT rcImage;
					GetImageDescFromList(CC, pResult, &pImage, &rcImage);
					if (pImage)
						pControl->SetImage(pImage, rcImage);

					//	Done

					pResult->Discard(&CC);
					}

				break;
				}
			}
		}
	}

void CDockScreen::ShowItem (void)

//	ShowItem
//
//	Sets the title and description for the currently selected item

	{
	m_pItemListControl->Invalidate();

	//	If we've got an installed armor segment selected, then highlight
	//	it on the armor display

	if (m_pItemListControl->IsCursorValid())
		{
		const CItem &Item = m_pItemListControl->GetItemAtCursor();
		if (Item.IsInstalled() && Item.GetType()->IsArmor())
			{
			int iSeg = Item.GetInstalled();
			m_pTrans->SelectArmor(iSeg);
			}
		else
			m_pTrans->SelectArmor(-1);
		}
	else
		m_pTrans->SelectArmor(-1);
	}

void CDockScreen::SetActionLabel (int iAction, const CString &sLabel, const CString &sKey)

//	SetActionLabel
//
//	Sets the label of the given action

	{
	if (m_pCurrentFrame)
		{
		CGButtonArea *pAction = (CGButtonArea *)m_pCurrentFrame->FindArea(g_FirstActionID + iAction);
		if (pAction)
			{
			pAction->SetLabel(sLabel);
			if (!sKey.IsBlank())
				pAction->SetLabelAccelerator(sKey);
			else
				{
				CXMLElement *pActionDesc = m_pCurrentActions->GetContentElement(iAction);
				pAction->SetLabelAccelerator(pActionDesc->GetAttribute(KEY_ATTRIB));
				}
			}
		}
	}

void CDockScreen::SetCounter (int iCount)

//	SetCounter
//
//	Sets the value of the counter field

	{
	if (m_pCounter)
		{
		m_pCounter->SetText(strFromInt(iCount, false));
		m_bReplaceCounter = true;
		}
	}

ALERROR CDockScreen::SetDisplayText (const CString &sID, const CString &sText)

//	SetDisplayText
//
//	Sets the text for a display control

	{
	SDisplayControl *pControl = FindDisplayControl(sID);
	if (pControl == NULL || pControl->pArea == NULL)
		return ERR_FAIL;

	if (pControl->iType != ctrlText)
		return ERR_FAIL;

	CGTextArea *pTextControl = (CGTextArea *)pControl->pArea;
	pTextControl->SetText(sText);

	return NOERROR;
	}

void CDockScreen::SetListFilter (const CItemCriteria &Filter)

//	SetListFilter
//
//	Filters the list given the criteria

	{
	if (m_pItemListControl)
		{
		m_pItemListControl->SetFilter(Filter);
		ShowItem();

		if (!m_bInShowPane)
			ShowPane(EvalInitialPane());
		}
	}

void CDockScreen::SetTextInput (const CString &sText)

//	SetTextInput
//
//	Sets the value of the text input field

	{
	if (m_pTextInput)
		m_pTextInput->SetText(sText);
	}

void CDockScreen::ShowPane (const CString &sName)

//	ShowPane
//
//	Shows the pane of the given name

	{
	int i;

#ifdef DEBUG_STRING_LEAKS
	CString::DebugMark();
#endif

	//	Find the pane named

	CXMLElement *pNewPane = m_pPanes->GetContentElementByTag(sName);
	if (pNewPane == NULL)
		{
		SetDescription(CONSTLIT("ERROR: Unable to find pane."));
		return;
		}

	//	Make sure we don't recurse

	m_bInShowPane = true;

	//	Destroy the previous pane

	if (m_pCurrentFrame)
		m_pScreen->DestroyArea(m_pCurrentFrame);

	//	Create a new pane

	CGFrameArea *pFrame = new CGFrameArea;
	m_pScreen->AddArea(pFrame, m_rcPane, 0);

	//	Find the pane element

	m_pCurrentPane = pNewPane;

	//	Find the actions element

	m_pCurrentActions = m_pCurrentPane->GetContentElementByTag(g_ActionsTag);

	//	Are we showing the counter?

	RECT rcInput;
	bool bShowCounter = m_pCurrentPane->GetAttributeBool(CONSTLIT(g_ShowCounterAttrib));
	if (bShowCounter)
		{
		m_pCounter = new CGTextArea;
		m_pCounter->SetEditable();
		m_pCounter->SetText(CONSTLIT("0"));
		m_pCounter->SetFont(&m_pFonts->SubTitleHeavyBold);
		m_pCounter->SetColor(CG16bitImage::RGBValue(255,255,255));
		m_pCounter->SetStyles(alignCentered);

		rcInput.left = m_rcPane.left + (RectWidth(m_rcPane) - g_cxCounter) / 2;
		rcInput.right = rcInput.left + g_cxCounter;
		rcInput.top = m_rcPane.bottom - g_cyActionsRegion - g_cyCounter - 24;
		rcInput.bottom = rcInput.top + g_cyCounter;

		pFrame->AddArea(m_pCounter, rcInput, g_CounterID);

		m_bReplaceCounter = true;
		}
	else
		m_pCounter = NULL;

	//	Are we showing an input field

	bool bShowTextInput = m_pCurrentPane->GetAttributeBool(SHOW_TEXT_INPUT_ATTRIB);
	if (bShowTextInput && !bShowCounter)
		{
		m_pTextInput = new CGTextArea;
		m_pTextInput->SetEditable();
		m_pTextInput->SetFont(&m_pFonts->SubTitleHeavyBold);
		m_pTextInput->SetColor(CG16bitImage::RGBValue(255,255,255));

		rcInput.left = m_rcPane.left + (RectWidth(m_rcPane) - TEXT_INPUT_WIDTH) / 2;
		rcInput.right = rcInput.left + TEXT_INPUT_WIDTH;
		rcInput.top = m_rcPane.bottom - g_cyActionsRegion - TEXT_INPUT_HEIGHT - 24;
		rcInput.bottom = rcInput.top + TEXT_INPUT_HEIGHT;

		pFrame->AddArea(m_pTextInput, rcInput, TEXT_INPUT_ID);
		}
	else
		m_pTextInput = NULL;

	//	Create the description

	CString sDesc = EvalString(m_pCurrentPane->GetAttribute(CONSTLIT(g_DescAttrib)));
	m_pFrameDesc = new CGTextArea;
	m_pFrameDesc->SetText(sDesc);
	m_pFrameDesc->SetFont(&m_pFonts->Large);
	m_pFrameDesc->SetColor(m_pFonts->wTextColor);
	m_pFrameDesc->SetLineSpacing(3);

	//	Justify the text

	RECT rcDesc;
	rcDesc.left = m_rcPane.left;
	rcDesc.top = m_rcPane.top + 16;
	rcDesc.right = m_rcPane.right;
	if (bShowCounter || bShowTextInput)
		rcDesc.bottom = rcInput.top;
	else
		rcDesc.bottom = m_rcPane.bottom - g_cyActionsRegion;

	pFrame->AddArea(m_pFrameDesc, rcDesc, 0);

	//	Show the currently selected item

	if (m_pItemListControl)
		{
		//	Update armor items to match the current state (the damaged flag)

		CSpaceObject *pLocation = m_pItemListControl->GetSource();
		if (pLocation)
			pLocation->UpdateArmorItems();

		//	Update the item list

		ShowItem();

		//	If this is set, don't allow the list selection to change

		m_bNoListNavigation = m_pCurrentPane->GetAttributeBool(NO_LIST_NAVIGATION_ATTRIB);
		}
	else
		{
		m_pTrans->SelectArmor(-1);
		m_bNoListNavigation = false;
		}

	//	Update the display

	if (m_pControls)
		ShowDisplay();

	//	Create the actions

	int y = m_rcPane.bottom - g_cyActionsRegion;

	for (i = 0; i < m_pCurrentActions->GetContentElementCount(); i++)
		{
		RECT rcArea;

		CXMLElement *pActionDesc = m_pCurrentActions->GetContentElement(i);
		CGButtonArea *pAction = new CGButtonArea;
		pAction->SetLabel(pActionDesc->GetAttribute(CONSTLIT(g_ActionNameAttrib)));
		pAction->SetLabelFont(&m_pFonts->MediumHeavyBold);
		pAction->SetLabelAccelerator(pActionDesc->GetAttribute(KEY_ATTRIB));

		rcArea.left = m_rcPane.left;
		rcArea.top = y;
		rcArea.right = m_rcPane.right;
		rcArea.bottom = y + g_cyAction;
		pFrame->AddArea(pAction, rcArea, g_FirstActionID + i);

		y += g_cyAction + g_cyActionSpacing;
		}

	//	Done

	m_pCurrentFrame = pFrame;

	//	Evaluate the initialize element

	CXMLElement *pInit = m_pCurrentPane->GetContentElementByTag(g_InitializeTag);
	if (pInit)
		{
		CString sCode = pInit->GetContentText(0);
		EvalString(sCode, true);
		}

	//	Check to see if the description is too large for the area. If so, then
	//	we shift everything down.

	int cyDesc = m_pFrameDesc->Justify(rcDesc);
	int cyExtraSpace = 0;
	if (cyDesc > (RectHeight(rcDesc) - 5 * g_cyActionSpacing)
			&& m_pCurrentActions->GetContentElementCount() <= (g_iMaxActions - 3))
		{
		cyExtraSpace = 3 * (g_cyAction + g_cyActionSpacing);

		rcDesc.bottom += cyExtraSpace;
		m_pFrameDesc->SetRect(rcDesc);
		}

	//	Position the buttons (deals with extra space above and show/hide)

	PositionActionButtons(m_rcPane.bottom - g_cyActionsRegion + cyExtraSpace);

	//	Update screen

	UpdateCredits();
	m_bInShowPane = false;

#ifdef DEBUG_STRING_LEAKS
	CString::DebugOutputLeakedStrings();
#endif
	}

void CDockScreen::SelectNextItem (bool *retbMore)

//	SelectNextItem
//
//	Selects the next item in the list

	{
	bool bMore = m_pItemListControl->MoveCursorForward();

	if (retbMore)
		*retbMore = bMore;
	}

void CDockScreen::ShowScreen (const CString &sName, const CString &sPane)

//	ShowScreen
//
//	Shows a different screen

	{
	CXMLElement *pScreen = m_pLocation->GetScreen(sName);
	ASSERT(pScreen);
	m_pTrans->ShowDockScreen(m_pLocation, pScreen, sPane);
	}

void CDockScreen::SelectPrevItem (void)

//	SelectPrevItem
//
//	Selects the previous item in the list

	{
	m_pItemListControl->MoveCursorBack();
	}

void CDockScreen::SetDescription (const CString &sDesc)

//	SetDescription
//
//	Sets the description of the current pane

	{
	if (m_pFrameDesc)
		m_pFrameDesc->SetText(sDesc);
	}

void CDockScreen::UpdateCredits (void)

//	UpdateCredits
//
//	Updates the display of credits

	{
	m_pCredits->SetText(strFromInt(m_pPlayer->GetCredits(), TRUE));
	m_pCargoSpace->SetText(strPatternSubst("%d ton%p", m_pPlayer->GetCargoSpace()));
	}

