//	CHighScoreDisplay.cpp
//
//	CHighScoreDisplay class

#include "PreComp.h"
#include "Transcendence.h"

#define SCORE_COLUMN_WIDTH			116

CHighScoreDisplay::CHighScoreDisplay (void) : m_pList(NULL)

//	CHighScoreDisplay constructor

	{
	}

CHighScoreDisplay::~CHighScoreDisplay (void)

//	CHighScoreDisplay destructor

	{
	CleanUp();
	}

void CHighScoreDisplay::CleanUp (void)

//	CleanUp
//
//	Must be called to release resources

	{
	m_pList = NULL;

	//	NOTE: We only do this to save memory. The destructor will
	//	automatically free the memory.

	m_Buffer.Destroy();
	}

int CHighScoreDisplay::ComputeFirstEntry (int iHighlight)

//	ComputeFirstEntry
//
//	Computes the first entry. iHighlight is the entry that we
//	should highlight (-1 if no highlight)

	{
	ASSERT(m_pList);

	if (iHighlight == -1)
		return 0;
	else
		{
		if (iHighlight >= 2)
			return iHighlight - 2;
		else
			return 0;
		}
	}

ALERROR CHighScoreDisplay::Init (const RECT &rcRect, CHighScoreList *pList, int iHighlight)

//	Init
//
//	Must be called to initialize

	{
	ALERROR error;

	ASSERT(m_pList == NULL);
	m_pList = pList;
	m_rcRect = rcRect;
	m_iSelectedEntry = iHighlight;
	m_iFirstEntry = ComputeFirstEntry(m_iSelectedEntry);

	//	Create the off-screen buffer

	error = m_Buffer.CreateBlank(RectWidth(rcRect), RectHeight(rcRect), false);
	if (error)
		return error;

	return NOERROR;
	}

void CHighScoreDisplay::Paint (CG16bitImage &Dest)

//	Paint
//
//	Paints the display

	{
	ASSERT(m_pList);

	//	If we have a selected entry, highlight the background

	if (m_iSelectedEntry != -1)
		{
		RECT rcHighlight = m_rcHighlight;
		::OffsetRect(&rcHighlight, m_rcRect.left, m_rcRect.top);

		Dest.FillTransRGB(rcHighlight.left,
				rcHighlight.top,
				RectWidth(rcHighlight),
				RectHeight(rcHighlight),
				CGImage::RGBColor(255, 255, 255),
				32);
		}

	//	Blt the scores

	Dest.ColorTransBlt(0,
			0,
			RectWidth(m_rcRect),
			RectHeight(m_rcRect),
			255,
			m_Buffer,
			m_rcRect.left,
			m_rcRect.top);
	}

void CHighScoreDisplay::SelectNext (void)

//	SelectNext
//
//	Select the next score

	{
	if (m_iSelectedEntry + 1 < m_pList->GetCount())
		{
		m_iSelectedEntry++;
		m_iFirstEntry = ComputeFirstEntry(m_iSelectedEntry);
		}
	}

void CHighScoreDisplay::SelectPrevious (void)

//	SelectPrevious
//
//	Select the previous score

	{
	if (m_iSelectedEntry - 1 >= 0)
		{
		m_iSelectedEntry--;
		m_iFirstEntry = ComputeFirstEntry(m_iSelectedEntry);
		}
	}

void CHighScoreDisplay::Update (void)

//	Update
//
//	Updates the display

	{
	ASSERT(m_pList);

	//	Don't bother drawing anything if we've got no scores

	if (m_pList->GetCount() == 0)
		return;

	m_Buffer.Fill(0, 0, RectWidth(m_rcRect), RectHeight(m_rcRect), CGImage::RGBAColor(0, 0, 0, 0xFF));

	//	Draw Title

	m_pFonts->SubTitle.DrawText(m_Buffer, 
			0, 
			0, 
			CGImage::RGBColor(128,128,128), 
			CONSTLIT("High Score List"));

	int y = m_pFonts->SubTitle.GetHeight();

	//	Draw each score

	CStringArray Lines;
	for (int i = m_iFirstEntry; 
			(i < m_pList->GetCount()) && (y + m_pFonts->SubTitle.GetHeight() < RectHeight(m_rcRect)); 
			i++)
		{
		const SGameStats &Entry = m_pList->GetEntry(i);

		if (i == m_iSelectedEntry)
			{
			m_rcHighlight.left = 0;
			m_rcHighlight.top = y;
			m_rcHighlight.right = RectWidth(m_rcRect);
			}

		CString sScore = strFromInt(Entry.iScore, false);
		int cxWidth = m_pFonts->SubTitle.MeasureText(sScore, NULL);
		m_pFonts->SubTitle.DrawText(m_Buffer,
				(SCORE_COLUMN_WIDTH - 10) - cxWidth,
				y,
				CGImage::RGBColor(128,128,128),
				sScore);

		CString sName;
		if (Entry.bDebugGame)
			sName = strPatternSubst(CONSTLIT("%d. %s [debug]"), i+1, Entry.sName.GetASCIIZPointer());
		else
			sName = strPatternSubst(CONSTLIT("%d. %s [%s]"), i+1, Entry.sName.GetASCIIZPointer(), Entry.sShipClass.GetASCIIZPointer());
		m_pFonts->Medium.DrawText(m_Buffer,
				SCORE_COLUMN_WIDTH,
				y,
				CGImage::RGBColor(255,255,255),
				sName);

		CString sText = Entry.sEpitaph;
		sText.Capitalize(CString::capFirstLetter);
		if (!Entry.sTime.IsBlank())
			sText.Append(strPatternSubst(CONSTLIT("\nPlayed for %s"), Entry.sTime.GetASCIIZPointer()));

		if (Entry.iResurrectCount == 1)
			sText.Append(CONSTLIT(" (resurrected once)"));
		else if (Entry.iResurrectCount > 1)
			sText.Append(strPatternSubst(CONSTLIT(" (resurrected %d times)"), Entry.iResurrectCount));

		y += m_pFonts->Medium.GetHeight();
		Lines.RemoveAll();
		m_pFonts->Medium.BreakText(sText, RectWidth(m_rcRect) - SCORE_COLUMN_WIDTH, &Lines);
		for (int j = 0; j < Lines.GetCount(); j++)
			{
			m_pFonts->Medium.DrawText(m_Buffer,
					SCORE_COLUMN_WIDTH,
					y,
					CGImage::RGBColor(128,128,128),
					Lines.GetStringValue(j));

			y += m_pFonts->Medium.GetHeight();
			}

		y += 2;

		if (i == m_iSelectedEntry)
			m_rcHighlight.bottom = y;
		}

	m_Buffer.SetTransparentColor(CGImage::RGBColor(0,0,0));
	}
