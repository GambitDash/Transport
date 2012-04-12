//	CGItemListArea.cpp
//
//	CGItemListArea class

#include "PreComp.h"
#include "Transcendence.h"

const int ROW_HEIGHT =						96;
const int ICON_WIDTH =						96;
const int ICON_HEIGHT =						96;

const int ITEM_TEXT_MARGIN_Y =				4;
const int ITEM_TEXT_MARGIN_X =				4;

const int MODIFIER_SPACING_X =				4;

const COLORREF RGB_DISABLED_TEXT =				CGImage::RGBColor(128,128,128);
const COLORREF RGB_TITLE_TEXT =					CGImage::RGBColor(255,255,255);
const COLORREF RGB_DESCRIPTION_TEXT =			CGImage::RGBColor(128,128,128);
const COLORREF RGB_SELECTED_DESCRIPTION_TEXT =	CGImage::RGBColor(200,200,200);

const COLORREF RGB_MILITARY_BACKGROUND =	CGImage::RGBColor(0,23,167);
const COLORREF RGB_ILLEGAL_BACKGROUND =		CGImage::RGBColor(167,23,0);
const COLORREF RGB_MODIFIER_TEXT =				CGImage::RGBColor(150,180,255);

#define STR_NO_ITEMS						CONSTLIT("There are no items here")

CGItemListArea::CGItemListArea (void) :
		m_pListData(NULL),
		m_iType(listNone),
		m_pFonts(NULL),
		m_iOldCursor(-1),
		m_yOffset(0),
		m_yFirst(0)

//	CGItemListArea constructor

	{
	}

CGItemListArea::~CGItemListArea (void)

//	CGItemListArea destructor

	{
	if (m_pListData)
		delete m_pListData;
	}

void CGItemListArea::CleanUp (void)

//	CleanUp
//
//	Free list data

	{
	if (m_pListData)
		{
		delete m_pListData;
		m_pListData = NULL;
		}
	}

ICCItem *CGItemListArea::GetEntryAtCursor (void)

//	GetEntryAtCursor
//
//	Returns the current entry

	{
	CCodeChain &CC = g_pUniverse->GetCC();

	if (m_pListData == NULL)
		return CC.CreateNil();

	return m_pListData->GetEntryAtCursor(CC);
	}

bool CGItemListArea::LButtonDown (int x, int y)

//	LButtonDown
//
//	Handle button down

	{
	if (m_iOldCursor != -1 && m_pListData->GetCount())
		{
		//	Figure out the cursor position that the user clicked on

		int iPos = ((y - GetRect().top) - m_yFirst) / ROW_HEIGHT;
		if (iPos >= 0 && iPos < m_pListData->GetCount())
			SignalAction(iPos);

		return true;
		}

	return false;
	}

bool CGItemListArea::MoveCursorBack (void)

//	MoveCursorBack
//
//	Move cursor back

	{
	bool bOK = (m_pListData ? m_pListData->MoveCursorBack() : false);
	if (bOK)
		Invalidate();
	return bOK;
	}

bool CGItemListArea::MoveCursorForward (void)

//	MoveCursorForward
//
//	Move cursor forward

	{
	bool bOK = (m_pListData ? m_pListData->MoveCursorForward() : false);
	if (bOK)
		Invalidate();
	return bOK;
	}

void CGItemListArea::Paint (CG16bitImage &Dest, const RECT &rcRect)

//	Paint
//
//	Paint the area

	{
	//	Can't paint if we are not properly initialized

	if (m_pFonts == NULL)
		return;

	//	If there are no items here, then say so

	if (m_pListData == NULL || !m_pListData->IsCursorValid())
		{
		int x = rcRect.left + (RectWidth(rcRect) - m_pFonts->LargeBold.MeasureText(STR_NO_ITEMS)) / 2;
		int y = rcRect.top + (RectHeight(rcRect) - m_pFonts->LargeBold.GetHeight()) / 2;

		Dest.DrawText(x, y,
				m_pFonts->LargeBold,
				RGB_DISABLED_TEXT,
				STR_NO_ITEMS);

		m_iOldCursor = -1;
		}

	//	Otherwise, paint the list of items

	else
		{
		int iCursor = m_pListData->GetCursor();
		int iCount = m_pListData->GetCount();

		//	If the cursor has changed, update the offset so that we
		//	have a smooth scroll.

		if (m_iOldCursor != -1 && m_iOldCursor != iCursor)
			{
			if (m_iOldCursor < iCursor)
				m_yOffset = ROW_HEIGHT;
			else
				m_yOffset = -ROW_HEIGHT;
			}

		m_iOldCursor = iCursor;

		//	Figure out the ideal position of the cursor (relative to the
		//	rect).

		int yIdeal = m_yOffset + ((RectHeight(rcRect) - ROW_HEIGHT) / 2);

		//	Figure out the actual position of the cursor row

		int yCursor;

		//	If the cursor is in the top part of the list
		if ((iCursor * ROW_HEIGHT) < yIdeal)
			yCursor = iCursor * ROW_HEIGHT;

		//	If the total number of lines is less than the whole rect
		else if ((iCount * ROW_HEIGHT) < RectHeight(rcRect))
			yCursor = iCursor * ROW_HEIGHT;

		//	If the cursor is in the bottom part of the list
		else if (((iCount - iCursor) * ROW_HEIGHT) < (RectHeight(rcRect) - yIdeal))
			yCursor = (RectHeight(rcRect) - ((iCount - iCursor) * ROW_HEIGHT));

		//	The cursor is in the middle of the list
		else
			yCursor = yIdeal;

		//	Figure out the item position at which we start painting

		int iStart = iCursor - ((yCursor + (ROW_HEIGHT - 1)) / ROW_HEIGHT);
		int yStart = yCursor - ((iCursor - iStart) * ROW_HEIGHT);

		//	Compute y offset of first row (so that we can handle clicks later)

		m_yFirst = yStart - (iStart * ROW_HEIGHT);

		//	Paint

		int y = rcRect.top + yStart;
		int iPos = iStart;
		bool bPaintSeparator = false;
		RECT rcItem;

		while (y < rcRect.bottom && iPos < iCount)
			{
			//	Paint previous separator

			if (bPaintSeparator)
				{
				Dest.Fill(rcItem.left,
						rcItem.bottom - 1,
						RectWidth(rcItem),
						1,
						CG16bitImage::RGBValue(80,80,80));
				}
			else
				bPaintSeparator = true;

			//	Paint only if we have a valid entry. Sometimes we can
			//	start at an invalid entry because we're scrolling.

			if (iPos >= 0)
				{
				m_pListData->SetCursor(iPos);

				rcItem.top = y;
				rcItem.left = rcRect.left;
				rcItem.bottom = y + ROW_HEIGHT;
				rcItem.right = rcRect.right;

				//	See if we need to paint the cursor

				bool bPaintCursor = (iPos == iCursor);

				//	Paint selection background (if selected)

				if (bPaintCursor)
					{
					Dest.FillTrans(rcItem.left,
							rcItem.top,
							RectWidth(rcItem),
							RectHeight(rcItem),
							m_pFonts->wSelectBackground,
							128);

					bPaintSeparator = false;
					}

				//	Paint item

				switch (m_iType)
					{
					case listCustom:
						PaintCustom(Dest, rcItem, bPaintCursor);
						break;

					case listItem:
						PaintItem(Dest, m_pListData->GetItemAtCursor(), rcItem, bPaintCursor);
						break;
					default:
						break;
					}
				}

			//	Next

			iPos++;
			y += ROW_HEIGHT;
			}

		//	Done

		m_pListData->SetCursor(iCursor);
		}
	}

void CGItemListArea::PaintCustom (CG16bitImage &Dest, const RECT &rcRect, bool bSelected)

//	PaintCustom
//
//	Paints a custom element

	{
	//	Paint the image

	m_pListData->PaintImageAtCursor(Dest, rcRect.left, rcRect.top);

	RECT rcDrawRect = rcRect;
	rcDrawRect.left += ICON_WIDTH + ITEM_TEXT_MARGIN_X;
	rcDrawRect.right -= ITEM_TEXT_MARGIN_X;
	rcDrawRect.top += ITEM_TEXT_MARGIN_Y;

	//	Paint the title

	int cyHeight;
	RECT rcTitle = rcDrawRect;
	m_pFonts->LargeBold.DrawText(Dest,
			rcTitle,
			RGB_TITLE_TEXT,
			m_pListData->GetTitleAtCursor(),
			0,
			CG16bitFont::SmartQuotes | CG16bitFont::TruncateLine,
			&cyHeight);

	rcDrawRect.top += cyHeight;

	//	Paint the description

	m_pFonts->Medium.DrawText(Dest, 
			rcDrawRect,
			(bSelected ? RGB_SELECTED_DESCRIPTION_TEXT : RGB_DESCRIPTION_TEXT),
			m_pListData->GetDescAtCursor(),
			0,
			CG16bitFont::SmartQuotes,
			&cyHeight);

	rcDrawRect.top += cyHeight;
	}

void CGItemListArea::PaintItem (CG16bitImage &Dest, const CItem &Item, const RECT &rcRect, bool bSelected)

//	PaintItem
//
//	Paints the item

	{
	//	Paint the image

	DrawItemTypeIcon(Dest, rcRect.left, rcRect.top, Item.GetType());

	RECT rcDrawRect = rcRect;
	rcDrawRect.left += ICON_WIDTH + ITEM_TEXT_MARGIN_X;
	rcDrawRect.right -= ITEM_TEXT_MARGIN_X;
	rcDrawRect.top += ITEM_TEXT_MARGIN_Y;

	//	Paint the attribute blocks

	RECT rcAttrib;
	rcAttrib = rcDrawRect;
	rcAttrib.bottom = rcAttrib.top + m_pFonts->MediumHeavyBold.GetHeight();

	if (Item.IsDamaged())
		PaintItemModifier(Dest,
				CONSTLIT("Damaged"),
				RGB_ILLEGAL_BACKGROUND,
				&rcAttrib);

	if (Item.GetType()->IsKnown()
			&& Item.GetType()->HasModifier(CONSTLIT("Military")))
		PaintItemModifier(Dest, 
				CONSTLIT("Military"),
				RGB_MILITARY_BACKGROUND,
				&rcAttrib);

	if (Item.GetType()->IsKnown()
			&& Item.GetType()->HasModifier(CONSTLIT("Illegal")))
		PaintItemModifier(Dest, 
				CONSTLIT("Illegal"),
				RGB_ILLEGAL_BACKGROUND,
				&rcAttrib);

	CString sEnhanced = Item.GetEnhancedDesc(m_pListData->GetSource());
	if (!sEnhanced.IsBlank())
		{
		bool bDisadvantage = (*(sEnhanced.GetASCIIZPointer()) == '-');
		PaintItemModifier(Dest,
				sEnhanced,
				(bDisadvantage ? RGB_ILLEGAL_BACKGROUND : RGB_MILITARY_BACKGROUND),
				&rcAttrib);
		}

	//	Paint the item name

	int cyHeight;
	RECT rcTitle = rcDrawRect;
	rcTitle.right = rcAttrib.right;
	m_pFonts->LargeBold.DrawText(Dest,
			rcTitle,
			RGB_TITLE_TEXT,
			Item.GetNounPhrase(nounCount | nounNoModifiers),
			0,
			CG16bitFont::SmartQuotes | CG16bitFont::TruncateLine,
			&cyHeight);

	rcDrawRect.top += cyHeight;

	//	Stats

	CString sReference = Item.GetReference(m_pListData->GetSource());
	CString sStat;
	
	if (sReference.IsBlank())
		sStat = strPatternSubst("Level: %s", strRomanNumeral(Item.GetType()->GetApparentLevel()).GetPointer());
	else
		sStat = strPatternSubst("Level: %s — %s", 
				strRomanNumeral(Item.GetType()->GetApparentLevel()).GetPointer(),
				sReference.GetPointer());

	m_pFonts->Medium.DrawText(Dest, 
			rcDrawRect,
			RGB_TITLE_TEXT,//(bSelected ? RGB_SELECTED_DESCRIPTION_TEXT : RGB_DESCRIPTION_TEXT),
			sStat,
			0,
			0,
			&cyHeight);

	rcDrawRect.top += cyHeight;

	//	Description

	CString sDesc = Item.GetDesc();
#if 0
	if (Item.IsInstalled())
		{
		if (m_pListData->GetSource())
			sDesc = strPatternSubst("%s. %s", m_pListData->GetSource()->GetInstallationPhrase(Item), sDesc);
		else
			sDesc = strPatternSubst("Installed. %s", sDesc);
		}
#endif

	m_pFonts->Medium.DrawText(Dest,
			rcDrawRect,
			(bSelected ? RGB_SELECTED_DESCRIPTION_TEXT : RGB_DESCRIPTION_TEXT),
			sDesc,
			0,
			CG16bitFont::SmartQuotes,
			&cyHeight);
	rcDrawRect.top += cyHeight;
	}

void CGItemListArea::PaintItemModifier (CG16bitImage &Dest, 
										const CString &sLabel,
										COLORREF rgbBackground,
										RECT *ioRect)

//	PaintItemModifier
//
//	Paints the item modifier and adjusts the rect

	{
	int cx = m_pFonts->Medium.MeasureText(sLabel);

	Dest.FillRGB(ioRect->right - (cx + 2 * MODIFIER_SPACING_X),
			ioRect->top,
			cx + 2 * MODIFIER_SPACING_X,
			m_pFonts->Medium.GetHeight(),
			rgbBackground);

	Dest.DrawText(ioRect->right - (cx + MODIFIER_SPACING_X),
			ioRect->top,
			m_pFonts->Medium,
			RGB_MODIFIER_TEXT,
			sLabel);

	ioRect->right -= (cx + 3 * MODIFIER_SPACING_X);
	}

void CGItemListArea::SetList (CSpaceObject *pSource)

//	SetList
//
//	Sets the item list that this control will display

	{
	CleanUp();

	//	Make sure the items in the source are sorted

	pSource->GetItemList().SortItems();

	//	Create a new data source

	m_pListData = new CItemListWrapper(pSource);
	m_iType = listItem;

	//	Done

	Invalidate();
	}

void CGItemListArea::SetList (CCodeChain &CC, ICCItem *pList)

//	SetList
//
//	Sets the list from a CC list

	{
	CleanUp();

	//	Create the new data source

	m_pListData = new CListWrapper(&CC, pList);
	m_iType = listCustom;

	//	Done

	Invalidate();
	}

void CGItemListArea::Update (void)

//	Update
//
//	Update state

	{
	if (m_yOffset)
		{
		int iDelta;
		if (m_yOffset > 0)
			{
			iDelta = Max(12, m_yOffset / 4);
			m_yOffset = Max(0, m_yOffset - iDelta);
			}
		else
			{
			iDelta = Min(-12, m_yOffset / 4);
			m_yOffset = Min(0, m_yOffset - iDelta);
			}

		Invalidate();
		}
	}

//	CItemListWrapper -----------------------------------------------------------

CItemListWrapper::CItemListWrapper (CSpaceObject *pSource) :
		m_pSource(pSource),
		m_ItemList(pSource->GetItemList())

//	CItemListWrapper constructor

	{
	}

//	CListWrapper ---------------------------------------------------------------

const int TITLE_INDEX =			0;
const int ICON_INDEX =			1;
const int DESC_INDEX =			2;

const int IMAGE_UNID_INDEX =	0;
const int IMAGE_X_INDEX =		1;
const int IMAGE_Y_INDEX =		2;
const int IMAGE_ELEMENTS =		3;

CListWrapper::CListWrapper (CCodeChain *pCC, ICCItem *pList) :
		m_pCC(pCC),
		m_pList(pList->Reference()),
		m_iCursor(-1)

//	CListWrapper constructor

	{
	}

CString CListWrapper::GetDescAtCursor (void)

//	GetDescAtCursor
//
//	Returns the description of the list element

	{
	if (IsCursorValid())
		{
		ICCItem *pItem = m_pList->GetElement(m_iCursor);
		if (DESC_INDEX < pItem->GetCount())
			return pItem->GetElement(DESC_INDEX)->GetStringValue();
		}

	return NULL_STR;
	}

ICCItem *CListWrapper::GetEntryAtCursor (CCodeChain &CC)

//	GetEntryAtCursor
//
//	Returns the entry at the cursor

	{
	if (!IsCursorValid())
		return CC.CreateNil();

	ICCItem *pItem = m_pList->GetElement(m_iCursor);
	return pItem->Reference();
	}

CString CListWrapper::GetTitleAtCursor (void)

//	GetTitleAtCursor
//
//	Returns the title of the list element

	{
	if (IsCursorValid())
		{
		ICCItem *pItem = m_pList->GetElement(m_iCursor);
		if (TITLE_INDEX < pItem->GetCount())
			return pItem->GetElement(TITLE_INDEX)->GetStringValue();
		}

	return NULL_STR;
	}

bool CListWrapper::MoveCursorBack (void)

//	MoveCursorBack
//
//	Move cursor back

	{
	if (m_iCursor <= 0)
		return false;
	else
		{
		m_iCursor--;
		return true;
		}
	}

bool CListWrapper::MoveCursorForward (void)

//	MoveCursorForward
//
//	Moves the cursor forward

	{
	if (m_iCursor + 1 == GetCount())
		return false;
	else
		{
		m_iCursor++;
		return true;
		}
	}

void CListWrapper::PaintImageAtCursor (CG16bitImage &Dest, int x, int y)

//	PaintImageAtCursor
//
//	Paints the image for the current element

	{
	if (!IsCursorValid())
		return;

	ICCItem *pItem = m_pList->GetElement(m_iCursor);
	if (pItem->GetCount() <= ICON_INDEX)
		return;

	ICCItem *pIcon = pItem->GetElement(ICON_INDEX);
	if (pIcon->GetCount() < IMAGE_ELEMENTS)
		return;

	CG16bitImage *pImage = g_pUniverse->GetLibraryBitmap(pIcon->GetElement(IMAGE_UNID_INDEX)->GetIntegerValue());
	if (pImage == NULL)
		return;

	Dest.ColorTransBlt(pIcon->GetElement(IMAGE_X_INDEX)->GetIntegerValue(),
			pIcon->GetElement(IMAGE_Y_INDEX)->GetIntegerValue(),
			ICON_WIDTH,
			ICON_HEIGHT,
			255,
			*pImage,
			x,
			y);
	}
