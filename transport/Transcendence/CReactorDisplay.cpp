//	CReactorDisplay.cpp
//
//	CReactorDisplay class

#include "PreComp.h"
#include "Transcendence.h"

#define NORMAL_COLOR				RGB(0, 128, 0)
#define WARNING_COLOR				RGB(255, 255, 0)
#define CRITICAL_COLOR				RGB(255, 0, 0)

#define FUEL_MARKER_COUNT			20
#define FUEL_MARKER_HEIGHT			5
#define FUEL_MARKER_WIDTH			12
#define FUEL_MARKER_GAP				1

#define TEXT_COLOR					CG16bitImage::RGBValue(150,180,255)
#define DAMAGED_TEXT_COLOR			CG16bitImage::RGBValue(128,0,0)

CReactorDisplay::CReactorDisplay (void) :
		m_pPlayer(NULL),
		m_pFonts(NULL),
		m_iTickCount(0),
		m_iOverloading(0)

//	CReactorDisplay contructor

	{
	}

CReactorDisplay::~CReactorDisplay (void)

//	CReactorDisplay destructor

	{
	CleanUp();
	}

void CReactorDisplay::CleanUp (void)

//	CleanUp
//
//	Delete relevant stuff

	{
	m_pPlayer = NULL;
	}

ALERROR CReactorDisplay::Init (CPlayerShipController *pPlayer, const RECT &rcRect)

//	Init
//
//	Initialize

	{
	ALERROR error;

	CleanUp();

	m_pPlayer = pPlayer;
	m_rcRect = rcRect;

	//	Create the off-screen buffer

	if ((error = m_Buffer.CreateBlank(RectWidth(rcRect), RectHeight(rcRect), false)))
		return error;

	m_Buffer.SetTransparentColor(CG16bitImage::RGBValue(0,0,0));

	return NOERROR;
	}

void CReactorDisplay::Paint (CG16bitImage &Dest)

//	Paint
//
//	Paints the display

	{
	Dest.ColorTransBlt(0,
			0,
			RectWidth(m_rcRect),
			RectHeight(m_rcRect),
			255,
			m_Buffer,
			m_rcRect.left,
			m_rcRect.top);
	}

void CReactorDisplay::Update (void)

//	Update
//
//	Updates the buffer from data

	{
	if (m_pPlayer == NULL)
		return;

	CShip *pShip = m_pPlayer->GetShip();
	const SPlayerSettings *pSettings = pShip->GetClass()->GetPlayerSettings();

	int yOffset = (RectHeight(m_rcRect) - RectHeight(pSettings->m_ReactorImage.GetImageRect())) / 2;

	//	Paint the background

	pSettings->m_ReactorImage.PaintImageUL(m_Buffer, 0, yOffset, 0, 0, false);

	//	Calculate fuel values

	int iFuelLeft = pShip->GetFuelLeft();
	int iMaxFuel = pShip->GetMaxFuel();
	int iFuelLevel = (iMaxFuel > 0 ? (Min((iFuelLeft * 100 / iMaxFuel) + 1, 100)) : 0);

	//	Paint the fuel level

	CG16bitImage *pFuelImage = NULL;
	RECT rcSrcRect;
	bool bBlink;
	if (iFuelLevel < 15)
		{
		pFuelImage = &pSettings->m_FuelLowLevelImage.GetImage();
		rcSrcRect = pSettings->m_FuelLowLevelImage.GetImageRect();
		bBlink = true;
		}
	else
		{
		pFuelImage = &pSettings->m_FuelLevelImage.GetImage();
		rcSrcRect = pSettings->m_FuelLevelImage.GetImageRect();
		bBlink = false;
		}

	if (!bBlink || ((m_iTickCount % 2) == 0))
		{
		int cxFuelLevel = RectWidth(rcSrcRect) * iFuelLevel / 100;
		m_Buffer.ColorTransBlt(rcSrcRect.left,
				rcSrcRect.top,
				cxFuelLevel,
				RectHeight(rcSrcRect),
				255,
				*pFuelImage,
				pSettings->m_xFuelLevelImage,
				yOffset + pSettings->m_yFuelLevelImage);
		}

	//	Paint fuel level text

	m_Buffer.Fill(pSettings->m_rcFuelLevelText.left,
			yOffset + pSettings->m_rcFuelLevelText.top,
			RectWidth(pSettings->m_rcFuelLevelText),
			RectHeight(pSettings->m_rcFuelLevelText),
			CG16bitImage::RGBValue(0, 0, 0));

	CString sFuelLevel = strPatternSubst(CONSTLIT("fuel"));
	int cxWidth = m_pFonts->Small.MeasureText(sFuelLevel);
	m_pFonts->Small.DrawText(m_Buffer,
			pSettings->m_rcFuelLevelText.left,
			yOffset + pSettings->m_rcFuelLevelText.top,
			m_pFonts->wHelpColor,
			sFuelLevel,
			0);

	//	Calculate the power level

	int iPowerUsage = pShip->GetPowerConsumption();
	int iMaxPower = pShip->GetMaxPower();
	int iPowerLevel;
	if (iMaxPower)
		iPowerLevel = Min((iPowerUsage * 100 / iMaxPower) + 1, 120);
	else
		iPowerLevel = 0;

	if (iPowerLevel >= 100)
		m_iOverloading++;
	else
		m_iOverloading = 0;

	bBlink = (m_iOverloading > 0);

	//	Paint the power level

	if (!bBlink || ((m_iOverloading % 2) == 1))
		{
		CG16bitImage *pPowerImage = &pSettings->m_PowerLevelImage.GetImage();
		rcSrcRect = pSettings->m_PowerLevelImage.GetImageRect();
		int cxPowerLevel = RectWidth(rcSrcRect) * iPowerLevel / 120;
		m_Buffer.ColorTransBlt(rcSrcRect.left,
				rcSrcRect.top,
				cxPowerLevel,
				RectHeight(rcSrcRect),
				255,
				*pPowerImage,
				pSettings->m_xPowerLevelImage,
				yOffset + pSettings->m_yPowerLevelImage);
		}

	//	Paint power level text

	m_Buffer.Fill(pSettings->m_rcPowerLevelText.left,
			yOffset + pSettings->m_rcPowerLevelText.top,
			RectWidth(pSettings->m_rcPowerLevelText),
			RectHeight(pSettings->m_rcPowerLevelText),
			CG16bitImage::RGBValue(0, 0, 0));

	CString sPowerLevel = strPatternSubst(CONSTLIT("power usage"));
	int cyHeight;
	cxWidth = m_pFonts->Small.MeasureText(sPowerLevel, &cyHeight);
	m_pFonts->Small.DrawText(m_Buffer,
			pSettings->m_rcPowerLevelText.left,
			yOffset + pSettings->m_rcPowerLevelText.bottom - cyHeight,
			m_pFonts->wHelpColor,
			sPowerLevel,
			0);

	//	Paint the reactor name (we paint on top of the levels)

	COLORREF wColor;
	if (pShip->GetReactorDesc()->fDamaged)
		wColor = DAMAGED_TEXT_COLOR;
	else
		wColor = m_pFonts->wTitleColor;

	CString sReactorName = strPatternSubst(CONSTLIT("%s (%s)"), 
			pShip->GetReactorName().GetASCIIZPointer(),
			ReactorPower2String(iMaxPower).GetASCIIZPointer());

	m_pFonts->Medium.DrawText(m_Buffer,
			pSettings->m_rcReactorText.left,
			yOffset + pSettings->m_rcReactorText.top,
			wColor,
			sReactorName,
			0);

	m_Buffer.SetTransparentColor(0);

	m_iTickCount++;
	}

