//	CGTextArea.cpp
//
//	Implementation of CGTextArea class

#include "portage.h"

#include "CMath.h"
#include "CStringArray.h"
#include "CRect.h"

#include "CG16bitImage.h"
#include "CG16bitFont.h"
#include "AGScreen.h"
#include "CGTextArea.h"

CGTextArea::CGTextArea (void) :
		m_bEditable(false),
		m_dwStyles(alignLeft),
		m_cyLineSpacing(0),
		m_pFont(NULL),
		m_cxJustifyWidth(0),
		m_iTick(0)

//	CGTextArea constructor

	{
	m_Color = CG16bitImage::RGBValue(255,255,255);
	}

int CGTextArea::Justify (const RECT &rcRect)

//	Justify
//
//	Justify the text and return the height (in pixels)

	{
	if (m_pFont == NULL)
		return 0;

	if (m_cxJustifyWidth != RectWidth(rcRect))
		{
		m_cxJustifyWidth = RectWidth(rcRect);
		m_Lines.RemoveAll();
		m_pFont->BreakText(m_sText, m_cxJustifyWidth, &m_Lines, CG16bitFont::SmartQuotes);
		}

	return m_Lines.GetCount() * m_pFont->GetHeight() + (m_Lines.GetCount() - 1) * m_cyLineSpacing;
	}

void CGTextArea::Paint (CG16bitImage &Dest, const RECT &rcRect)

//	Paint
//
//	Handle paint

	{
	//	Paint the editable box

	if (m_bEditable)
		{
		WORD wBorderColor = CG16bitImage::BlendPixel(CG16bitImage::RGBValue(0, 0, 0), m_Color, 128);
		DrawRectDotted(Dest, rcRect.left, rcRect.top, RectWidth(rcRect), RectHeight(rcRect), wBorderColor);
		}

	//	Paint the text

	if (m_pFont)
		{
		//	If we haven't justified the text for this size, do it now

		if (m_cxJustifyWidth != RectWidth(rcRect))
			{
			m_cxJustifyWidth = RectWidth(rcRect);
			m_Lines.RemoveAll();
			m_pFont->BreakText(m_sText, m_cxJustifyWidth, &m_Lines, CG16bitFont::SmartQuotes);
			}

		//	Compute the rect within which we draw the text

		RECT rcText = rcRect;
		if (m_bEditable)
			{
			rcText.left += 2;
			rcText.right -= 2;
			rcText.top += 2;
			rcText.bottom -= 2;
			}

		//	Clip to text rect

		RECT rcOldClip = Dest.GetClipRect();
		Dest.SetClipRect(rcText);

		//	Figure out how many lines fit in the rect

		int iMaxLineCount = RectHeight(rcText) / m_pFont->GetHeight();

		//	If there are too many lines, and we're editable, start at the end

		int iStart = 0;
		if (m_bEditable && iMaxLineCount < m_Lines.GetCount())
			iStart = m_Lines.GetCount() - iMaxLineCount;

		//	Paint each line

		int x = rcText.left;
		int y = rcText.top;
		for (int i = iStart; i < m_Lines.GetCount(); i++)
			{
			CString sLine = m_Lines.GetStringValue(i);

			//	Trim the last space in the line, if necessary

			char *pPos = sLine.GetASCIIZPointer();
			if (sLine.GetLength() > 0 && pPos[sLine.GetLength() - 1] == ' ')
				sLine = strTrimWhitespace(sLine);

			//	Alignment

			int xLine;
			if (m_dwStyles & alignCentered)
				{
				int cxWidth = m_pFont->MeasureText(sLine);
				xLine = x + (RectWidth(rcText) - cxWidth) / 2;
				}
			else if (m_dwStyles & alignRight)
				{
				int cxWidth = m_pFont->MeasureText(sLine);
				xLine = x + (RectWidth(rcRect) - cxWidth);
				}
			else
				xLine = x;

			//	Paint

			if (HasEffects())
				m_pFont->DrawTextEffect(Dest, xLine, y, m_Color, sLine, GetEffectCount(), GetEffects());
			else
				Dest.DrawText(xLine, y, *m_pFont, m_Color, sLine);

			//	Next

			y += m_pFont->GetHeight() + m_cyLineSpacing;
			if (y >= rcText.bottom)
				break;
			}

		//	Restore clip

		Dest.SetClipRect(rcOldClip);
		}
	}
