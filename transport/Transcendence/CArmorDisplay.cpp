//	CArmorDisplay.cpp
//
//	CArmorDisplay class

#include "PreComp.h"
#include "Transcendence.h"

#define ARMOR_SECTION_COUNT				4

#define DISPLAY_WIDTH					360
#define DISPLAY_HEIGHT					136

#define SHIELD_IMAGE_WIDTH				136
#define SHIELD_IMAGE_HEIGHT				136

#define DESCRIPTION_WIDTH				(DISPLAY_WIDTH - SHIELD_IMAGE_WIDTH)

#define HP_DISPLAY_WIDTH				26
#define HP_DISPLAY_HEIGHT				14

#define HP_DISPLAY_BACK_COLOR			CG16bitImage::RGBValue(0,23,167)
#define HP_DISPLAY_TEXT_COLOR			CG16bitImage::RGBValue(150,180,255)

#define SHIELD_HP_DISPLAY_X				DESCRIPTION_WIDTH
#define SHIELD_HP_DISPLAY_Y				(DISPLAY_HEIGHT - 16)
#define SHIELD_HP_DISPLAY_WIDTH			26
#define SHIELD_HP_DISPLAY_HEIGHT		14
#define SHIELD_HP_DISPLAY_BACK_COLOR	CG16bitImage::RGBValue(0,117,16)
#define SHIELD_HP_DISPLAY_TEXT_COLOR	CG16bitImage::RGBValue(150,255,180)
#define SHIELD_HP_DISPLAY_LINE_COLOR	CG16bitImage::RGBValue(0,117,16)
#define DISABLED_TEXT_COLOR				CG16bitImage::RGBValue(128,0,0)

#define ARMOR_NAME_COLOR				CG16bitImage::RGBValue(150,180,255)
#define ARMOR_LINE_COLOR				CG16bitImage::RGBValue(0,23,167)
#define ARMOR_ENHANCE_X					224
#define ARMOR_ENHANCE_BACK_COLOR		CG16bitImage::RGBValue(0,23,167)
#define ARMOR_ENHANCE_TEXT_COLOR		CG16bitImage::RGBValue(150,180,255)
#define ARMOR_DAMAGED_BACK_COLOR		CG16bitImage::RGBValue(167,23,0)
#define ARMOR_DAMAGED_TEXT_COLOR		CG16bitImage::RGBValue(150,180,255)

CArmorDisplay::CArmorDisplay (void) : m_pUniverse(NULL),
		m_pPlayer(NULL),
		m_iSelection(-1)

//	CArmorDisplay constructor

	{
	}

CArmorDisplay::~CArmorDisplay (void)

//	CArmorDisplay destructor

	{
	CleanUp();
	}

void CArmorDisplay::CleanUp (void)

//	CleanUp
//
//	Delete relevant stuff

	{
	}

ALERROR CArmorDisplay::Init (CPlayerShipController *pPlayer, const RECT &rcRect)

//	Init
//
//	Initialize

	{
	ALERROR error;

	CleanUp();

	m_pUniverse = pPlayer->GetShip()->GetUniverse();
	m_pPlayer = pPlayer;
	m_rcRect = rcRect;

	//	Create the off-screen buffer

	error = m_Buffer.CreateBlank(DISPLAY_WIDTH, DISPLAY_HEIGHT, false);
	if (error)
		return error;

	return NOERROR;
	}

void CArmorDisplay::Paint (CG16bitImage &Dest)

//	Paint
//
//	Paints to the destination

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

void CArmorDisplay::SetSelection (int iSelection)

//	SetSelection
//
//	Selects an armor segment

	{
	if (iSelection != m_iSelection)
		{
		m_iSelection = iSelection;
		Update();
		}
	}

void CArmorDisplay::Update (void)

//	Update
//
//	Updates buffer from data

	{
	int i;
	CShip *pShip = m_pPlayer->GetShip();
	const SPlayerSettings *pSettings = pShip->GetClass()->GetPlayerSettings();
	CItemListManipulator ItemList(pShip->GetItemList());
	const CG16bitFont &SmallFont = m_pPlayer->GetTrans()->GetFonts().Small;

	//	Figure out the status of the shields

	int iHP = 0;
	int iMaxHP = 10;
	CInstalledDevice *pShield = pShip->GetNamedDevice(devShields);
	if (pShield)
		pShield->GetStatus(pShip, &iHP, &iMaxHP);

	//	Draw shields

	int iWhole = (iHP * 100) / iMaxHP;
	int iIndex = (100 - iWhole) / 20;

	m_Buffer.DrawRectFilled(0, 0, m_Buffer.GetWidth(), m_Buffer.GetHeight(), CGImage::RGBColor(0,0,0));

	const RECT &rcShield = pSettings->m_ShieldImage.GetImageRect();
	m_Buffer.Blt(rcShield.left, 
			rcShield.top + (RectHeight(rcShield) * iIndex), 
			RectWidth(rcShield), 
			RectHeight(rcShield), 
			pSettings->m_ShieldImage.GetImage(), 
			DESCRIPTION_WIDTH + ((SHIELD_IMAGE_WIDTH - RectWidth(rcShield)) / 2),
			(SHIELD_IMAGE_HEIGHT - RectHeight(rcShield)) / 2);

	if (pShield)
		{
		m_Buffer.Fill(SHIELD_HP_DISPLAY_X,
				SHIELD_HP_DISPLAY_Y, 
				SHIELD_HP_DISPLAY_WIDTH, 
				SHIELD_HP_DISPLAY_HEIGHT,
				SHIELD_HP_DISPLAY_BACK_COLOR);
		
		CString sHP = strFromInt(iHP, true);
		int cxWidth = m_pFonts->Medium.MeasureText(sHP, NULL);
		m_pFonts->Medium.DrawText(m_Buffer,
				SHIELD_HP_DISPLAY_X + (SHIELD_HP_DISPLAY_WIDTH - cxWidth) / 2,
				SHIELD_HP_DISPLAY_Y - 1,
				CG16bitImage::LightenPixel(m_pFonts->wAltGreenColor, 60),
				sHP);

		DrawBrokenLine(m_Buffer,
				0,
				SHIELD_HP_DISPLAY_Y,
				SHIELD_HP_DISPLAY_X,
				SHIELD_HP_DISPLAY_Y,
				0,
				SHIELD_HP_DISPLAY_LINE_COLOR);

		COLORREF wColor;
		if (pShield->IsEnabled() && !pShield->IsDamaged())
			wColor = m_pFonts->wAltGreenColor;
		else
			wColor = DISABLED_TEXT_COLOR;

		m_pFonts->Medium.DrawText(m_Buffer,
				0,
				SHIELD_HP_DISPLAY_Y,
				wColor,
				pShield->GetClass()->GetName());

		//	Paint the modifiers

		if (pShield->GetMods() != 0)
			{
			pShip->SetCursorAtNamedDevice(ItemList, devShields);
			CString sMods = ItemList.GetItemAtCursor().GetModsDesc(pShip);
			if (!sMods.IsBlank())
				{
				int cx = SmallFont.MeasureText(sMods);
				m_Buffer.Fill(SHIELD_HP_DISPLAY_X - cx - 8,
						SHIELD_HP_DISPLAY_Y,
						cx + 8,
						SHIELD_HP_DISPLAY_HEIGHT,
						ARMOR_ENHANCE_BACK_COLOR);

				SmallFont.DrawText(m_Buffer,
						SHIELD_HP_DISPLAY_X - cx - 4,
						SHIELD_HP_DISPLAY_Y + (SHIELD_HP_DISPLAY_HEIGHT - SmallFont.GetHeight()) / 2,
						ARMOR_ENHANCE_TEXT_COLOR,
						sMods);
				}
			}
		}

	//	Draw armor

	int iArmorCount = Min(pShip->GetArmorSectionCount(), pSettings->m_iArmorDescCount);
	for (i = 0; i < iArmorCount; i++)
		{
		SArmorImageDesc *pImage = &pSettings->m_pArmorDesc[i];

		CInstalledArmor *pArmor = pShip->GetArmorSection(i);
		int iMaxHP = pArmor->pArmorClass->GetHitPoints(pArmor);
		int iWhole = (pArmor->iHitPoints * 100) / iMaxHP;
		int iIndex = (100 - iWhole) / 20;
		
		if (iIndex < 5)
			{
			const RECT &rcImage = pImage->Image.GetImageRect();
			m_Buffer.ColorTransBlt(rcImage.left,
					rcImage.top + iIndex * RectHeight(rcImage),
					RectWidth(rcImage),
					RectHeight(rcImage),
					255,
					pImage->Image.GetImage(),
					DESCRIPTION_WIDTH + pImage->xDest,
					pImage->yDest);
			}

		//	Paint the HPs

		if (i == m_iSelection)
			{
			m_Buffer.Fill(DESCRIPTION_WIDTH + pImage->xHP - 1, 
					pImage->yHP - 1, 
					HP_DISPLAY_WIDTH + 2, 
					HP_DISPLAY_HEIGHT + 2,
					CG16bitImage::DarkenPixel(m_pFonts->wSelectBackground, 128));
			}
		else
			{
			m_Buffer.Fill(DESCRIPTION_WIDTH + pImage->xHP, 
					pImage->yHP, 
					HP_DISPLAY_WIDTH, 
					HP_DISPLAY_HEIGHT,
					HP_DISPLAY_BACK_COLOR);
			}

		CString sHP = strFromInt(pArmor->iHitPoints, true);
		int cxWidth = m_pFonts->Medium.MeasureText(sHP, NULL);
		m_pFonts->Medium.DrawText(m_Buffer,
				DESCRIPTION_WIDTH + pImage->xHP + (HP_DISPLAY_WIDTH - cxWidth) / 2,
				pImage->yHP - 1,
				m_pFonts->wTitleColor,
				sHP);
		}

	//	Draw armor names

	for (i = 0; i < iArmorCount; i++)
		{
		SArmorImageDesc *pImage = &pSettings->m_pArmorDesc[i];
		CInstalledArmor *pArmor = pShip->GetArmorSection(i);

		//	Paint the armor name line

		DrawBrokenLine(m_Buffer,
				0,
				pImage->yName + m_pFonts->Medium.GetHeight(),
				DESCRIPTION_WIDTH + pImage->xHP + pImage->xNameDestOffset,
				pImage->yHP + pImage->yNameDestOffset,
				pImage->cxNameBreak,
				(i == m_iSelection ? CG16bitImage::DarkenPixel(m_pFonts->wSelectBackground, 128) : ARMOR_LINE_COLOR));

		//	Paint the armor names

		CString sName = pArmor->pArmorClass->GetShortName();
		if (i == m_iSelection)
			{
			int cy;
			int cx = m_pFonts->Medium.MeasureText(sName, &cy) + 4;
			m_Buffer.Fill(0, 
					pImage->yName, 
					cx, 
					cy,
					CG16bitImage::DarkenPixel(m_pFonts->wSelectBackground, 128));
			}

		m_pFonts->Medium.DrawText(m_Buffer,
				2,
				pImage->yName,
				m_pFonts->wTitleColor,
				sName);

		//	Paint the modifiers

		if (pArmor->m_Mods)
			{
			pShip->SetCursorAtArmor(ItemList, i);
			CString sMods = pArmor->pArmorClass->GetShortModifierDesc(ItemList.GetItemAtCursor(), pShip);
			if (!sMods.IsBlank())
				{
				int cx = SmallFont.MeasureText(sMods);
				m_Buffer.Fill(ARMOR_ENHANCE_X - cx - 4,
						pImage->yName + m_pFonts->Medium.GetHeight() - HP_DISPLAY_HEIGHT,
						cx + 8,
						HP_DISPLAY_HEIGHT,
						ARMOR_ENHANCE_BACK_COLOR);

				SmallFont.DrawText(m_Buffer,
						ARMOR_ENHANCE_X - cx,
						pImage->yName + 3,
						ARMOR_ENHANCE_TEXT_COLOR,
						sMods);
				}
			}
		}

	m_Buffer.SetTransparentColor(CG16bitImage::RGBValue(0,0,0));
	}
