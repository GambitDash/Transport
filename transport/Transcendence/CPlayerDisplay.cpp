//	CPlayerDisplay.cpp
//
//	CPlayerDisplay class

#include "PreComp.h"
#include "Transcendence.h"

#define BAR_COLOR							CG16bitImage::RGBValue(0, 2, 10)

#define ICON_LEFT							16
#define ICON_HEIGHT							80
#define ICON_WIDTH							80

#define NAME_COLOR							CG16bitImage::RGBValue(128,128,128)
#define NAME_LEFT							(ICON_LEFT + ICON_WIDTH + 16)
#define NAME_WIDTH							256

#define EDIT_BACKGROUND_COLOR				CG16bitImage::RGBValue(128,128,128)
#define EDIT_TEXT_COLOR						CG16bitImage::RGBValue(0,0,0)

#define BOOLEAN_OPTIONS_WIDTH				80
#define BOOLEAN_OPTIONS_HEIGHT				80
#define BOOLEAN_OPTIONS_SPACING				8
#define BOOLEAN_OPTIONS_COUNT				2

#define BOOLEAN_OPTIONS_TOTAL_WIDTH			((BOOLEAN_OPTIONS_SPACING + BOOLEAN_OPTIONS_WIDTH) * BOOLEAN_OPTIONS_COUNT)

#define BOOLEAN_ICONS_X						0
#define BOOLEAN_ICONS_Y						80
#define BOOLEAN_ICONS_WIDTH					64
#define BOOLEAN_ICONS_HEIGHT				64

#define BOOLEAN_LABEL_COLOR					CG16bitImage::RGBValue(128,128,128)

#define MALE_ICON_X							(BOOLEAN_ICONS_X + 0)
#define MALE_ICON_Y							(BOOLEAN_ICONS_Y)
#define FEMALE_ICON_X						(BOOLEAN_ICONS_X + 64)
#define FEMALE_ICON_Y						(BOOLEAN_ICONS_Y)

#define MUSIC_ICON_X						(BOOLEAN_ICONS_X + 128)
#define MUSIC_ICON_Y						(BOOLEAN_ICONS_Y)
#define NO_MUSIC_ICON_X						(BOOLEAN_ICONS_X + 192)
#define NO_MUSIC_ICON_Y						(BOOLEAN_ICONS_Y)

#define NO_DEBUG_ICON_X						(BOOLEAN_ICONS_X + 256)
#define NO_DEBUG_ICON_Y						(BOOLEAN_ICONS_Y)
#define DEBUG_ICON_X						(BOOLEAN_ICONS_X + 320)
#define DEBUG_ICON_Y						(BOOLEAN_ICONS_Y)

CPlayerDisplay::CPlayerDisplay (void) : 
		m_bShowDebugOption(false),
		m_bEditing(false), 
		m_bInvalid(true)

//	CPlayerDisplay constructor

	{
	}

CPlayerDisplay::~CPlayerDisplay (void)

//	CPlayerDisplay destructor

	{
	CleanUp();
	}

void CPlayerDisplay::CleanUp (void)

//	CleanUp
//
//	Must be called to release resources

	{
	//	NOTE: We only do this to save memory. The destructor will
	//	automatically free the memory.

	m_Buffer.Destroy();
	m_IconImage.Destroy();
	}

ALERROR CPlayerDisplay::Init (CTranscendenceWnd *pTrans, const RECT &rcRect, bool bShowDebugOption)

//	Init
//
//	Must be called to initialize

	{
	ALERROR error;
	CString sResource = "Resources/IconDisplay.jpg";

	m_rcRect = rcRect;
	m_pTrans = pTrans;
	m_pFonts = &pTrans->GetFonts();
	m_sName = m_pTrans->GetPlayerName();
	m_bMale = (m_pTrans->GetPlayerGenome() == genomeHumanMale);
	m_bMusicOn = m_pTrans->GetMusicOption();
	m_bShowDebugOption = bShowDebugOption;

	//	Create the off-screen buffer

	error = m_Buffer.CreateBlank(RectWidth(rcRect), RectHeight(rcRect), false);
	if (error)
		return error;

	m_IconImage.CreateFromBitmap(sResource, false);

	//	Compute rectangles

	m_rcIcon.left = m_rcRect.left + ICON_LEFT;
	m_rcIcon.top = m_rcRect.top + ((RectHeight(m_rcRect) - BOOLEAN_ICONS_HEIGHT) / 2);
	m_rcIcon.right = m_rcIcon.left + BOOLEAN_ICONS_HEIGHT;
	m_rcIcon.bottom = m_rcIcon.top + BOOLEAN_ICONS_WIDTH;

	m_rcName.left = m_rcRect.left + NAME_LEFT;
	m_rcName.top = m_rcRect.top + ((RectHeight(m_rcRect) - m_pFonts->SubTitle.GetHeight()) / 2);
	m_rcName.right = m_rcName.left + NAME_WIDTH;
	m_rcName.bottom = m_rcName.top + m_pFonts->SubTitle.GetHeight();

	//	Figure out the number of buttons

	int iButtonCount = 2;
	if (m_bShowDebugOption)
		iButtonCount++;

	int cxOptions = iButtonCount * (BOOLEAN_OPTIONS_SPACING + BOOLEAN_OPTIONS_WIDTH);
	int xOptions = m_rcRect.right - cxOptions;

	//	Compute the button rects

	m_rcGenderOption.left = xOptions;
	m_rcGenderOption.top = m_rcRect.top + ((RectHeight(m_rcRect) - BOOLEAN_OPTIONS_HEIGHT) / 2);
	m_rcGenderOption.right = m_rcGenderOption.left + BOOLEAN_OPTIONS_WIDTH;
	m_rcGenderOption.bottom = m_rcGenderOption.top + BOOLEAN_OPTIONS_HEIGHT;
	xOptions += (BOOLEAN_OPTIONS_SPACING + BOOLEAN_OPTIONS_WIDTH);

	m_rcMusicOption.left = xOptions;
	m_rcMusicOption.top = m_rcGenderOption.top;
	m_rcMusicOption.right = m_rcMusicOption.left + BOOLEAN_OPTIONS_WIDTH;
	m_rcMusicOption.bottom = m_rcMusicOption.top + BOOLEAN_OPTIONS_HEIGHT;
	xOptions += (BOOLEAN_OPTIONS_SPACING + BOOLEAN_OPTIONS_WIDTH);

	if (m_bShowDebugOption)
		{
		m_rcDebugModeOption.left = xOptions;
		m_rcDebugModeOption.top = m_rcRect.top + ((RectHeight(m_rcRect) - BOOLEAN_OPTIONS_HEIGHT) / 2);
		m_rcDebugModeOption.right = m_rcDebugModeOption.left + BOOLEAN_OPTIONS_WIDTH;
		m_rcDebugModeOption.bottom = m_rcDebugModeOption.top + BOOLEAN_OPTIONS_HEIGHT;
		xOptions += (BOOLEAN_OPTIONS_SPACING + BOOLEAN_OPTIONS_WIDTH);
		}

	//	Init

	m_bInvalid = true;
	m_bEditing = false;

	return NOERROR;
	}

bool CPlayerDisplay::OnChar (char chChar)

//	OnChar
//
//	Handle character

	{
	if (!m_bEditing)
		return false;

	switch (chChar)
		{
		case SDLK_BACKSPACE:
			{
			if (m_bClearAll)
				m_sEditBuffer = CString();
			else
				{
				if (!m_sEditBuffer.IsBlank())
					m_sEditBuffer = strSubString(m_sEditBuffer, 0, m_sEditBuffer.GetLength() - 1);
				}

			m_bInvalid = true;
			m_bClearAll = false;
			break;
			}

		case SDLK_RETURN:
			{
			m_sEditBuffer = strTrimWhitespace(m_sEditBuffer);
			if (!m_sEditBuffer.IsBlank())
				{
				m_sName = m_sEditBuffer;
				m_pTrans->SetPlayerName(m_sName);
				}

			m_bEditing = false;
			m_bInvalid = true;
			break;
			}

		case SDLK_ESCAPE:
			{
			m_bEditing = false;
			m_bInvalid = true;
			break;
			}

		//	Characters

		default:
			{
			if (!isprint(chChar))
				break;

			if (m_bClearAll)
				m_sEditBuffer = CString(&chChar, 1);
			else
				m_sEditBuffer.Append(CString(&chChar, 1));

			m_bInvalid = true;
			m_bClearAll = false;
			}
		}

	return true;
	}

bool CPlayerDisplay::OnKeyDown (int iVirtKey)

//	OnKeyDown
//
//	Handle keydown

	{
	if (!m_bEditing)
		return false;

	return true;
	}

bool CPlayerDisplay::OnLButtonDoubleClick (int x, int y)

//	OnLButtonDoubleClick
//
//	Handle lbuttondoubleclick

	{
	return OnLButtonDown(x, y);
	}

bool CPlayerDisplay::OnLButtonDown (int x, int y)

//	OnLButtonDown
//
//	Handle lbuttondown

	{
	if (m_bEditing)
		{
		m_bEditing = false;
		m_bInvalid = true;
		return true;
		}

	POINT pt;
	pt.x = x;
	pt.y = y;

	if (::PtInRect(&m_rcName, pt))
		{
		//	Edit mode

		m_sEditBuffer = m_sName;
		m_bEditing = true;
		m_bInvalid = true;
		m_bClearAll = true;
		}
	else if (::PtInRect(&m_rcGenderOption, pt))
		{
		m_bMale = !m_bMale;
		m_bInvalid = true;
		m_pTrans->SetPlayerGenome(m_bMale ? genomeHumanMale : genomeHumanFemale);
		}
	else if (::PtInRect(&m_rcMusicOption, pt))
		{
		m_bMusicOn = !m_bMusicOn;
		m_bInvalid = true;
		m_pTrans->SetMusicOption(m_bMusicOn);
		}
	else if (m_bShowDebugOption && ::PtInRect(&m_rcDebugModeOption, pt))
		{
		m_bInvalid = true;
		m_pTrans->SetDebugGame(!m_pTrans->GetDebugGame());
		}
	else
		return false;

	return true;
	}

void CPlayerDisplay::OnMouseMove (int x, int y)

//	OnMouseMove
//
//	Handle mouse move

	{
	}

void CPlayerDisplay::Paint (CG16bitImage &Dest)

//	Paint
//
//	Paints the display

	{
	if (m_bInvalid)
		{
		PaintBuffer();
		m_bInvalid = false;
		}

	Dest.Blt(0,
			0,
			RectWidth(m_rcRect),
			RectHeight(m_rcRect),
			m_Buffer,
			m_rcRect.left,
			m_rcRect.top);
	}

void CPlayerDisplay::PaintBuffer (void)

//	PaintBuffer
//
//	Paint off-screen buffer

	{
	//	Paint the background

	m_Buffer.Fill(0, 
			0, 
			RectWidth(m_rcRect), 
			RectHeight(m_rcRect),
			BAR_COLOR);

	//	Paint the ship icon

	m_Buffer.Blt(NO_DEBUG_ICON_X,
			NO_DEBUG_ICON_Y,
			BOOLEAN_ICONS_WIDTH,
			BOOLEAN_ICONS_HEIGHT,
			m_IconImage,
			m_rcIcon.left,
			m_rcIcon.top);

	//	Paint the name

	if (m_bEditing)
		{
		m_Buffer.Fill(m_rcName.left,
				m_rcName.top,
				RectWidth(m_rcName),
				RectHeight(m_rcName),
				EDIT_BACKGROUND_COLOR);

		m_pFonts->SubTitle.DrawText(m_Buffer,
				m_rcName.left,
				m_rcName.top,
				EDIT_TEXT_COLOR,
				m_sEditBuffer);
		}
	else
		{
		m_pFonts->SubTitle.DrawText(m_Buffer,
				m_rcName.left,
				m_rcName.top,
				NAME_COLOR,
				m_sName);
		}

	//	Paint gender option

	CString sText;
	RECT rcIcon;
	if (m_bMale)
		{
		sText = CONSTLIT("Human Male");
		rcIcon.left = MALE_ICON_X;
		rcIcon.top = MALE_ICON_Y;
		}
	else
		{
		sText = CONSTLIT("Human Female");
		rcIcon.left = FEMALE_ICON_X;
		rcIcon.top = FEMALE_ICON_Y;
		}

	PaintOption(0, rcIcon, sText);

	//	Paint the music option

	if (m_bMusicOn)
		{
		sText = CONSTLIT("Music On");
		rcIcon.left = MUSIC_ICON_X;
		rcIcon.top = MUSIC_ICON_Y;
		}
	else
		{
		sText = CONSTLIT("Music Off");
		rcIcon.left = NO_MUSIC_ICON_X;
		rcIcon.top = NO_MUSIC_ICON_Y;
		}

	PaintOption(1, rcIcon, sText);

	//	Paint debug mode

	if (m_bShowDebugOption)
		{
		if (!m_pTrans->GetDebugGame())
			{
			sText = CONSTLIT("Play");
			rcIcon.left = NO_DEBUG_ICON_X;
			rcIcon.top = NO_DEBUG_ICON_Y;
			}
		else
			{
			sText = CONSTLIT("Debug Mode");
			rcIcon.left = DEBUG_ICON_X;
			rcIcon.top = DEBUG_ICON_Y;
			}

		PaintOption(2, rcIcon, sText);
		}
	}

void CPlayerDisplay::PaintOption (int iPos, const RECT &rcIcon, const CString &sLabel)

//	PaintOption
//
//	Paints an option icon

	{
	int x = m_rcGenderOption.left + iPos * (BOOLEAN_OPTIONS_WIDTH + BOOLEAN_OPTIONS_SPACING);
	int y = m_rcGenderOption.top;

	m_Buffer.Blt(rcIcon.left,
			rcIcon.top,
			BOOLEAN_ICONS_WIDTH,
			BOOLEAN_ICONS_HEIGHT,
			m_IconImage,
			x + 8,
			y);

	int cxWidth = m_pFonts->Medium.MeasureText(sLabel, NULL);
	m_pFonts->Medium.DrawText(m_Buffer,
			x + (BOOLEAN_OPTIONS_WIDTH - cxWidth) / 2,
			y + BOOLEAN_ICONS_HEIGHT,
			BOOLEAN_LABEL_COLOR,
			sLabel);
	}

void CPlayerDisplay::Update (void)

//	Update
//
//	Updates the display

	{
	}
