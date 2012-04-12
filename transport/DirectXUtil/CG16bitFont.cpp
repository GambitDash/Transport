//	CG16bitFont.cpp
//
//	Implementation of raw 16-bit image font object

#include "SDL_ttf.h"
#include <string>

#include "portage.h"

#include "CStringArray.h"
#include "CRect.h"

#include "CG16bitImage.h"
#include "CG16bitFont.h"

#define STR_ELLIPSIS							CONSTLIT("...")

const int g_iStartChar = ' ';
const int g_iCharCount = 0xff - g_iStartChar + 1;

static void FormatLine (char *pPos, int iLen, bool *ioInSmartQuotes, CStringArray *pLines);

struct CharMetrics
	{
	int minx;
	int maxx;
	int miny;
	int maxy;
	int advance;
	};

CG16bitFont::CG16bitFont (void) : CObject(NULL), m_Font(NULL)

//	CG16bitFont constructor

	{
	}

CG16bitFont::~CG16bitFont()
	{
	Destroy();
	}

void CG16bitFont::BreakText (const CString &sText, int cxWidth, CStringArray *pLines, DWORD dwFlags) const

//	BreakText
//
//	Splits the given string into multiple lines so that they fit
//	in the given width

	{
	char *pPos = sText.GetASCIIZPointer();
	char *pStartLine = pPos;
	int iCharsInLine = 0;
	char *pStartWord = pPos;
	int iCharsInWord = 0;
	int cxWordWidth = 0;
	int cxRemainingWidth = cxWidth;

	//	Can't handle 0 widths

	if (cxWidth == 0)
		return;

	//	Smart quotes

	bool bInSmartQuotes = false;
	bool *ioInSmartQuotes;
	if (dwFlags & SmartQuotes)
		ioInSmartQuotes = &bInSmartQuotes;
	else
		ioInSmartQuotes = NULL;

	//	If we need to truncate, then we need to adjust the width

	if (dwFlags & TruncateLine)
		{
		cxWidth -= MeasureText(STR_ELLIPSIS);
		if (cxWidth < 0)
			return;
		}

	//	Do it

	bool bTruncate = false;
	while (*pPos != '\0')
		{
		//	If we've got a carriage return then we immediately end
		//	the line.

		if (*pPos == '\n')
			{
			//	Add the current word to the line

			iCharsInLine += iCharsInWord;

			//	Add the line to the array

			FormatLine(pStartLine, iCharsInLine, ioInSmartQuotes, pLines);

			//	Reset the line and word

			pStartLine = pPos + 1;
			pStartWord = pStartLine;
			iCharsInLine = 0;
			iCharsInWord = 0;
			cxWordWidth = 0;
			cxRemainingWidth = cxWidth;

			//	Reset smartquotes (sometimes we end a paragraph without closing
			//	a quote. In that case, we need to start with an open quote).

			bInSmartQuotes = false;

			//	If we're truncating, we're out of here (we don't add an ellipsis)

			if (dwFlags & TruncateLine)
				break;
			}

		//	Otherwise, continue by trying to add the character to the word

		else
			{
			//	Get the metrics for the character

			int iIndex = (int)(BYTE)(*pPos) - g_iStartChar;

			CharMetrics metric;

			iIndex = std::max(0, iIndex);

			if (TTF_GlyphMetrics(m_Font, *pPos, &metric.minx, &metric.maxx, &metric.miny,
						&metric.maxy, &metric.advance) != 0)
				{
				/* XXX Fail. */
				return;
				}
			//	Does the character fit in the line?

			bool bCharFits = ((cxRemainingWidth - cxWordWidth) >= metric.maxx);

			//	If the character doesn't fit, then we've reached the end
			//	of the line.

			if (!bCharFits)
				{
				//	If the character is a space then the entire word should
				//	fit on the line

				if (*pPos == ' ')
					{
					iCharsInLine += iCharsInWord;

					//	Reset the word

					pStartWord = pPos + 1;
					iCharsInWord = 0;
					cxWordWidth = 0;
					}

				//	If this is the first word in the line then we need to break
				//	up the word across lines.

				if (iCharsInLine == 0)
					{
					//	Add what we've got to the array

					FormatLine(pStartWord, iCharsInWord, ioInSmartQuotes, pLines);

					//	Reset the word

					pStartWord = pPos;
					iCharsInWord = 1;
					cxWordWidth = metric.advance;
					}

				//	Otherwise, add the line to the array

				else
					{
					FormatLine(pStartLine, iCharsInLine, ioInSmartQuotes, pLines);

					//	Reset the line

					pStartLine = pStartWord;
					iCharsInLine = 0;
					cxRemainingWidth = cxWidth;

					//	Add the character that didn't fit to the word

					if (*pPos != ' ')
						{
						iCharsInWord++;
						cxWordWidth += metric.advance;
						}
					}

				//	Done if we're truncating

				if (dwFlags & TruncateLine)
					{
					iCharsInLine = 0;
					iCharsInWord = 0;
					bTruncate = true;
					break;
					}
				}

			//	Otherwise, if it does fit, add it to the end of the word

			else
				{
				iCharsInWord++;
				cxWordWidth += metric.advance;

				//	If this character is a space or a hyphen, add it to the
				//	end of the line

				if (*pPos == ' ' || *pPos == '-')
					{
					iCharsInLine += iCharsInWord;
					cxRemainingWidth -= cxWordWidth;

					//	Reset the word

					pStartWord = pPos + 1;
					iCharsInWord = 0;
					cxWordWidth = 0;
					}
				}
			}

		//	Next character
		
		pPos++;
		}

	//	Add the remainder

	iCharsInLine += iCharsInWord;
	if (iCharsInLine)
		FormatLine(pStartLine, iCharsInLine, ioInSmartQuotes, pLines);

	//	Add ellipsis, if necessary

	if (bTruncate)
		pLines->GetStringRef(0).Append(STR_ELLIPSIS);
	}

ALERROR CG16bitFont::Create (const CString &sTypeface, int iSize, bool bBold, bool bItalic, bool bUnderline)

//	Create
//
//	Creates a basic font

	{
	ALERROR error = NOERROR;

	m_Font = TTF_OpenFont(sTypeface.GetASCIIZPointer(), iSize);
	if (!m_Font)
		{
		error = ERR_FAIL;
		printf("TTF_OpenFont: %s\n", TTF_GetError());
		assert(0);
		}

	return error;
	}

void CG16bitFont::Destroy (void)
	{
	if (m_Font)
		TTF_CloseFont(m_Font);
	m_Font = NULL;
	}

void CG16bitFont::DrawText (CG16bitImage &Dest, 
							int x, 
							int y, 
							COLORREF wColor, 
							const CString &sText,
							DWORD dwFlags,
							int *retx) const

//	DrawText
//
//	Draws a line of text on the given image

	{
	SDL_Surface *render;
	CG16bitImage img;

	std::string str(sText.GetPointer(), sText.GetLength());

	//SDL_Color black = {0, 0, 0, 0};
	SDL_Color fg = {
		CG16bitImage::RedColor(wColor),
		CG16bitImage::GreenColor(wColor),
		CG16bitImage::BlueColor(wColor),
		0
	};

//	render = TTF_RenderText_Solid(m_Font, str.c_str(), fg);
	render = TTF_RenderText_Blended(m_Font, str.c_str(), fg);
	if (render == NULL)
		{
		return;
		}
	img.BindSurface(render);
	// img.SetTransparentColor(wColor); // CG16bitImage::RGBColor(0, 0, 0));

	Dest.Blt(x, y, img, 0, 0, img.GetWidth(), img.GetHeight());
	img.UnbindSurface();

	if (retx)
		{
		*retx = x + render->w;
		}
	SDL_FreeSurface(render);
	}

void CG16bitFont::DrawText (CG16bitImage &Dest, 
							const RECT &rcRect, 
							COLORREF wColor, 
							const CString &sText, 
							int iLineAdj, 
							DWORD dwFlags,
							int *retcyHeight) const

//	Draw
//
//	Draws wrapped text

	{
	int i;
	CStringArray Lines;

	BreakText(sText, RectWidth(rcRect), &Lines, dwFlags);
	int y = rcRect.top;
	for (i = 0; i < Lines.GetCount(); i++)
		{
		int x = rcRect.left;

		if (dwFlags & AlignCenter)
			{
			int cxWidth = MeasureText(Lines.GetStringValue(i));
			x = rcRect.left + (RectWidth(rcRect) - cxWidth) / 2;
			}
		else if (dwFlags & AlignRight)
			{
			int cxWidth = MeasureText(Lines.GetStringValue(i));
			x = rcRect.right - cxWidth;
			}

		if (!(dwFlags & MeasureOnly))
			DrawText(Dest, x, y, wColor, Lines.GetStringValue(i));

		y += GetHeight() + iLineAdj;
		}

	if (retcyHeight)
		*retcyHeight = y - rcRect.top;
	}

void CG16bitFont::DrawTextEffect (CG16bitImage &Dest,
								  int x,
								  int y,
								  COLORREF wColor,
								  const CString &sText,
								  int iEffectsCount,
								  const SEffectDesc *pEffects,
								  DWORD dwFlags,
								  int *retx) const

//	DrawTextEffect
//
//	Draw text with effect

	{
	int i;

	//	Paint background effects

	for (i = 0; i < iEffectsCount; i++)
		{
		switch (pEffects[i].iType)
			{
			case effectShadow:
				{
				int xOffset = GetHeight() / 16;
				int yOffset = GetHeight() / 16;

				DrawText(Dest,
						x + xOffset,
						y + yOffset, 
						CG16bitImage::RGBColor(0, 0, 0),
						sText,
						dwFlags);
				break;
				}
			}
		}

	//	Paint

	DrawText(Dest, x, y, wColor, sText, dwFlags, retx);
	}

int CG16bitFont::MeasureText (CString sText, int *retcyHeight) const

//	MeasureText
//
//	Returns the width of the text in pixels and optionally the height

	{
	int w, h;

	if (TTF_SizeText(m_Font, sText.GetASCIIZPointer(), &w, &h) == -1)
		return 0;

	if (retcyHeight)
		*retcyHeight = h;
	return w;
	}

int CG16bitFont::GetHeight() const
	{
	return TTF_FontHeight(m_Font);
	}

int CG16bitFont::GetAverageWidth (void) const
	{
	int w, h;
	TTF_SizeText(m_Font, "A", &w, &h);
	return w;
	}

//	Internals -----------------------------------------------------------------

static void FormatLine (char *pPos, int iLen, bool *ioInSmartQuotes, CStringArray *pLines)
	{
	if (ioInSmartQuotes)
		{
		//	Add a new empty line

		int iIndex;
		pLines->AppendString(NULL_STR, &iIndex);
		CString &sLine = pLines->GetStringRef(iIndex);

		//	Copy the characters to the new empty line
		//	(converting to smart quotes as appropriate)

		char *pEndPos = pPos + iLen;
		char *pStart = pPos;
		while (pPos < pEndPos)
			{
			if (*pPos == '\"')
				{
				if (pStart != pPos)
					sLine.Append(CString(pStart, pPos - pStart, true));

				if (*ioInSmartQuotes)
					sLine.Append(CString("”", 1, true));
				else
					sLine.Append(CString("“", 1, true));

				*ioInSmartQuotes = !(*ioInSmartQuotes);

				pPos++;
				pStart = pPos;
				}
			else
				pPos++;
			}

		if (pStart != pPos)
			sLine.Append(CString(pStart, pPos - pStart, true));
		}
	else
		pLines->AppendString(CString(pPos, iLen), NULL);
	}

