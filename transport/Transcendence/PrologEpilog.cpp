//	PrologEpilog.cpp
//
//	Code to show prolog and epilog screens

#include "PreComp.h"
#include "Transcendence.h"
#include "XMLUtil.h"

#define TEXT_CRAWL_X						512
#define TEXT_CRAWL_HEIGHT					512
#define TEXT_CRAWL_WIDTH					384

#define BAR_COLOR							CG16bitImage::RGBValue(0, 2, 10)

void CTranscendenceWnd::AnimateCrawlScreen (void)

//	AnimateCrawlScreen
//
//	Paints the crawl screen

	{
	if (m_pCrawlImage)
		{
		RECT rcImage;
		rcImage.left = (g_cxScreen - m_pCrawlImage->GetWidth()) / 2;
		rcImage.top = (g_cyScreen - m_pCrawlImage->GetHeight()) / 2;
		rcImage.right = rcImage.left + m_pCrawlImage->GetWidth();
		rcImage.bottom = rcImage.right + m_pCrawlImage->GetHeight();

		//	Paint the parts that don't change

		if (m_bCrawlInvalid)
			{
			//	Paint bars across top and bottom

			m_Screen.Fill(0, 
					0, 
					g_cxScreen, 
					g_cyScreen,
					BAR_COLOR);

			//	Paint image

			m_Screen.Blt(0,
					0,
					m_pCrawlImage->GetWidth(),
					m_pCrawlImage->GetHeight(),
					*m_pCrawlImage,
					rcImage.left,
					rcImage.top);

			m_bCrawlInvalid = false;
			}

		//	Paint the background that the crawl is over

		RECT rcRect = m_CrawlText.GetRect();
		m_Screen.Fill(rcRect.left,
				rcRect.top,
				RectWidth(rcRect),
				RectHeight(rcRect),
				BAR_COLOR);

		m_Screen.Blt(rcRect.left - rcImage.left,
				rcRect.top - rcImage.top,
				RectWidth(rcRect),
				RectHeight(rcRect),
				*m_pCrawlImage,
				rcRect.left,
				rcRect.top);
		}
	else
		{
		if (m_bCrawlInvalid)
			{
			//	Erase the whole background

			m_Screen.Fill(0, 
					0, 
					g_cxScreen, 
					g_cyScreen,
					BAR_COLOR);

			m_bCrawlInvalid = false;
			}

		//	Paint the background that the crawl is over

		RECT rcRect = m_CrawlText.GetRect();
		m_Screen.Fill(rcRect.left,
				rcRect.top,
				RectWidth(rcRect),
				RectHeight(rcRect),
				BAR_COLOR);
		}

	//	Paint the crawl

	m_CrawlText.Paint(m_Screen);
	m_CrawlText.Update();
	}

ALERROR CTranscendenceWnd::InitCrawlScreen (void)

//	InitCrawlScreen
//
//	Initializes the crawl screen. Assumes that m_dwCrawlImage and
//	m_sCrawlText have been initialized.

	{
	ALERROR error;

	//	Set the image

	if (m_dwCrawlImage)
		m_pCrawlImage = g_pUniverse->GetLibraryBitmap(m_dwCrawlImage);
	else
		m_pCrawlImage = NULL;

	//	Set the text

	RECT rcRect;
	rcRect.top = (g_cyScreen - TEXT_CRAWL_HEIGHT) / 2;
	rcRect.left = m_rcMainScreen.left + TEXT_CRAWL_X;
	rcRect.bottom = rcRect.top + TEXT_CRAWL_HEIGHT;
	rcRect.right = rcRect.left + TEXT_CRAWL_WIDTH;
	m_CrawlText.SetFont(&m_Fonts.SubTitle);
	if ((error = m_CrawlText.Init(rcRect, m_sCrawlText)))
		return error;

	m_bCrawlInvalid = true;

	return NOERROR;
	}

ALERROR CTranscendenceWnd::StartEpilog (void)

//	StartEpilog
//
//	Start epilog after game ends. This call assumes that OnGameEnd has already been called
//	for the adventure (and that the crawl text and image are already set up)

	{
	ALERROR error;

	//	Play End March

	if (GetMusicOption())
		m_SoundMgr.PlayMusic(CONSTLIT("TranscendenceMarch.mp3"));

	//	Clean up current game

	m_GameFile.Close();
	if ((error = m_Universe.Reinit()))
		return error;

	//	If we don't have an epilog screen, then we go back to the intro

	if (m_dwCrawlImage == 0)
		{
		if ((error = StartIntro()))
			return error;
		return NOERROR;
		}

	//	Init the crawl screen

	if ((error = InitCrawlScreen()))
		return error;

	//	Done

	m_State = gsEpilog;
	SDL_ShowCursor(true);

	return NOERROR;
	}

ALERROR CTranscendenceWnd::StartProlog (void)

//	StartProlog
//
//	Starts the prolog screen

	{
	ALERROR error;

	SNewGameSettings NewGame;
	NewGame.sPlayerName = m_sPlayerName;
	NewGame.iPlayerGenome = (GenomeTypes)m_iPlayerGenome;
	NewGame.dwAdventure = m_dwAdventure;
	NewGame.dwPlayerShip = m_dwPlayerShip;

	CString sError;
	if ((error = StartNewGame(NewGame, &sError)))
		{
		sError = strPatternSubst(CONSTLIT("Unable to begin new game: %s"), sError.GetASCIIZPointer());
		kernelDebugLogMessage(sError.GetASCIIZPointer());

		StartIntro();
		return error;
		}

	//	Init the crawl screen

	if ((error = InitCrawlScreen()))
		return error;

	//	If we've got a prolog screen then we wait for the player to
	//	click before we continue. Otherwise, we continue automatically

	m_bContinue = (m_dwCrawlImage == 0);

	//	Kick-off background thread to finish up creating the game

	SetBackgroundState(bsCreateGame);

	//	Done

	m_State = gsProlog;
	SDL_ShowCursor(true);

	return NOERROR;
	}

void CTranscendenceWnd::StopEpilog (void)

//	StopEpilog
//
//	Clean up epilog screen

	{
	ASSERT(m_State == gsEpilog);

	m_CrawlText.CleanUp();
	SDL_ShowCursor(false);
	}

void CTranscendenceWnd::StopProlog (void)

//	StopProlog
//
//	Clean up the prolog screen

	{
	ASSERT(m_State == gsProlog);

	m_CrawlText.CleanUp();
	SDL_ShowCursor(false);
	}
