//	CPickerDisplay.cpp
//
//	CPickerDisplay class

#include "PreComp.h"
#include "Transcendence.h"

#define TOP_BORDER									4
#define BOTTOM_BORDER								4

#define TILE_WIDTH									96
#define TILE_HEIGHT									96
#define TILE_SPACING_X								4
#define TILE_SPACING_Y								4

#define MAX_ITEMS_VISIBLE							3

#define RGB_SELECTION								(CG16bitImage::RGBValue(150,255,180))
#define RGB_TEXT									(CG16bitImage::RGBValue(150,255,180))
#define RGB_EXTRA									(CG16bitImage::RGBValue(255,255,255))
#define RGB_HELP									(CG16bitImage::RGBValue(96,96,96))
#define RGB_HOTKEY_BACKGROUND						(CG16bitImage::RGBValue(150,255,180))
#define RGB_BACKGROUND								(CG16bitImage::RGBValue(0,0,16))

CPickerDisplay::CPickerDisplay (void)

//	CPickerDisplay constructor

	{
	}

CPickerDisplay::~CPickerDisplay (void)

//	CPickerDisplay destructor

	{
	}

void CPickerDisplay::CleanUp (void)

//	CleanUp
//
//	Cleanup display
	
	{
	m_pMenu = NULL;
	}

int CPickerDisplay::GetSelection (void)

//	GetSelection
//
//	Returns the currently selected menu item

	{
	return m_iSelection;
	}

ALERROR CPickerDisplay::Init (CMenuData *pMenu, const RECT &rcRect)

//	Init
//
//	Initializes picker

	{
	ALERROR error;

	CleanUp();

	m_pMenu = pMenu;
	m_rcRect = rcRect;
	m_bInvalid = true;

	//	Create the off-screen buffer

	error = m_Buffer.CreateBlank(RectWidth(m_rcRect), RectHeight(m_rcRect), false);
	if (error)
		return error;

	m_Buffer.SetAlphaChannel(255);

	//	Initialize

	m_iSelection = 0;

	return NOERROR;
	}

void CPickerDisplay::Paint (CG16bitImage &Dest)

//	Paint
//
//	Paint the display

	{
	Update();

	Dest.ColorTransBlt(0,
			0,
			RectWidth(m_rcRect),
			RectHeight(m_rcRect),
			255,
			m_Buffer,
			m_rcRect.left,
			m_rcRect.top);
	}

void CPickerDisplay::PaintSelection (CG16bitImage &Dest, int x, int y)

//	PaintSelection
//
//	Paints the selection at the coordinate

	{
	COLORREF wSelectColor = CG16bitImage::DarkenPixel(m_pFonts->wSelectBackground, 192);

	Dest.Fill(x - TILE_SPACING_X, 
			y - TILE_SPACING_Y,
			TILE_SPACING_X,
			TILE_HEIGHT + 2 * TILE_SPACING_Y,
			wSelectColor);

	Dest.Fill(x,
			y - TILE_SPACING_Y,
			TILE_SPACING_X,
			TILE_SPACING_Y,
			wSelectColor);

	Dest.Fill(x,
			y + TILE_HEIGHT,
			TILE_SPACING_X,
			TILE_SPACING_Y,
			wSelectColor);

	Dest.Fill(x + TILE_WIDTH,
			y - TILE_SPACING_Y,
			TILE_SPACING_X,
			TILE_HEIGHT + 2 * TILE_SPACING_Y,
			wSelectColor);

	Dest.Fill(x + TILE_WIDTH - TILE_SPACING_X,
			y - TILE_SPACING_Y,
			TILE_SPACING_X,
			TILE_SPACING_Y,
			wSelectColor);

	Dest.Fill(x + TILE_WIDTH - TILE_SPACING_X,
			y + TILE_HEIGHT,
			TILE_SPACING_X,
			TILE_SPACING_Y,
			wSelectColor);
	}

void CPickerDisplay::SelectNext (void)

//	SelectNext
//
//	Select the next item

	{
	if (m_iSelection + 1 < m_pMenu->GetCount())
		{
		m_iSelection++;
		m_bInvalid = true;
		}
	}

void CPickerDisplay::SelectPrev (void)

//	SelectPrev
//
//	Select the previous item

	{
	if (m_iSelection > 0)
		{
		m_iSelection--;
		m_bInvalid = true;
		}
	}

void CPickerDisplay::Update (void)

//	Update
//
//	Update the off-screen buffer

	{
	int i;

	if (!m_bInvalid || m_pMenu == NULL)
		return;

	//	Clear the area

	m_Buffer.Fill(0, 0, m_Buffer.GetWidth(), m_Buffer.GetHeight(), CG16bitImage::RGBValue(0, 0, 0));

	//	Figure out how many items we could show

	int iMaxCount = m_Buffer.GetWidth() / (TILE_WIDTH + TILE_SPACING_X);
	if ((iMaxCount % 2) == 0)
		iMaxCount--;

	//	Figure out which items we're showing

	int iLeft = Max(0, m_iSelection - (iMaxCount / 2));
	int iRight = Min(m_pMenu->GetCount() - 1, m_iSelection + (iMaxCount / 2));

	//	Figure out how many items we're showing

	int iCount = (iRight - iLeft) + 1;

	//	Adjust

	if ((iRight < m_pMenu->GetCount() - 1) && iCount < iMaxCount)
		{
		iRight = Min(m_pMenu->GetCount() - 1, iRight + (iMaxCount - iCount));
		iCount = (iRight - iLeft) + 1;
		}

	if (iLeft > 0 && iCount < iMaxCount)
		{
		iLeft = Max(0, iLeft - (iMaxCount - iCount));
		iCount = (iRight - iLeft) + 1;
		}

	//	Figure out the rect

	int cxView = (iCount * (TILE_WIDTH + TILE_SPACING_X)) - TILE_SPACING_X;
	RECT rcView;
	RECT rcText;
	rcView.left = (m_Buffer.GetWidth() - cxView) / 2;
	rcView.right = rcView.left + cxView;
	rcView.top = TOP_BORDER;
	rcView.bottom = rcView.top + TILE_HEIGHT;

	//	Clear out an area

	m_Buffer.Fill(rcView.left, 
			rcView.top, 
			RectWidth(rcView),
			RectHeight(rcView),
			m_pFonts->wBackground);

	//	Paint the items

	int x = rcView.left;
	for (i = iLeft; i <= iRight; i++)
		{
		m_pMenu->GetItemImage(i)->PaintImage(m_Buffer,
				x + (TILE_WIDTH / 2),
				rcView.top + (TILE_HEIGHT / 2),
				0,
				0);

		//	Paint the number of items

		CString sExtra = m_pMenu->GetItemExtra(i);
		if (!sExtra.IsBlank() && !m_sHelpText.IsBlank())
			{
			int cyExtra;
			int cxExtra = m_pFonts->LargeBold.MeasureText(sExtra, &cyExtra);

			m_pFonts->LargeBold.DrawText(m_Buffer,
					x + TILE_WIDTH - cxExtra - TILE_SPACING_X,
					rcView.top + TILE_HEIGHT - cyExtra,
					RGB_EXTRA,
					sExtra);
			}

		//	Paint the hotkey

		CString sKey = m_pMenu->GetItemKey(i);
		if (!sKey.IsBlank())
			{
			int cyKey;
			int cxKey = m_pFonts->LargeBold.MeasureText(sKey, &cyKey);

			int xKey = x + TILE_SPACING_X;
			int yKey = rcView.top + TILE_SPACING_Y;
			m_Buffer.Fill(xKey, yKey, cxKey + 4, cyKey, m_pFonts->wTitleColor);

			m_pFonts->LargeBold.DrawText(m_Buffer,
					xKey + 2,
					yKey,
					m_pFonts->wBackground,
					sKey);
			}

		//	If this item is selected, paint the selection

		if (i == m_iSelection)
			PaintSelection(m_Buffer, x, rcView.top);

		x += TILE_WIDTH + TILE_SPACING_X;
		}

	m_Buffer.SetAlphaChannel(0);
	m_Buffer.SetAlphaChannel(rcView.left - TILE_SPACING_X, rcView.top - TILE_SPACING_Y, rcView.right + TILE_SPACING_X, rcView.bottom + TILE_SPACING_Y, 200);

	//	Paint the name of the selected item

	CString sText = m_pMenu->GetItemLabel(m_iSelection);
	int cxText = m_pFonts->Header.MeasureText(sText);

	m_Buffer.Fill((m_Buffer.GetWidth() - cxText) / 2,
			rcView.bottom + TILE_SPACING_Y,
			cxText,
			m_pFonts->Header.GetHeight(),
			CG16bitImage::RGBValue(0, 0, 16));

	m_pFonts->Header.DrawText(m_Buffer,
			(m_Buffer.GetWidth() - cxText) / 2,
			rcView.bottom + TILE_SPACING_Y,
			m_pFonts->wTitleColor,
			sText);

	rcText.top = rcView.bottom + TILE_SPACING_Y;
	rcText.bottom = rcText.top + m_pFonts->Header.GetHeight();
	rcText.left = (m_Buffer.GetWidth() - cxText) / 2;
	rcText.right = rcText.left + cxText;

	m_Buffer.SetAlphaChannel(rcText.left, rcText.top, rcText.right, rcText.bottom, 200);

	//	Paint help text (if help text is blank, then use menu extra
	//	string for help).

	if (m_sHelpText.IsBlank())
		sText = m_pMenu->GetItemExtra(m_iSelection);
	else
		sText = m_sHelpText;
	cxText = m_pFonts->Medium.MeasureText(sText);
	m_pFonts->Medium.DrawText(m_Buffer,
			(m_Buffer.GetWidth() - cxText) / 2,
			rcView.bottom + TILE_SPACING_Y + m_pFonts->Header.GetHeight(),
			m_pFonts->wHelpColor,
			sText);

	rcText.top = rcView.bottom + TILE_SPACING_Y + m_pFonts->Header.GetHeight();
	rcText.bottom = rcText.top + m_pFonts->Header.GetHeight();
	rcText.left = (m_Buffer.GetWidth() - cxText) / 2;
	rcText.right = rcText.left + cxText;

	m_Buffer.SetAlphaChannel(rcText.left, rcText.top, rcText.right, rcText.bottom, 200);

	m_bInvalid = false;
	}
