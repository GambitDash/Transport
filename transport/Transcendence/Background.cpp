//	Background.cpp
//
//	Background thread functions

#include "PreComp.h"
#include "Transcendence.h"
#include "XMLUtil.h"

int CTranscendenceWnd::BackgroundThread (LPVOID pData)

//	BackgroundThread
//
//	Background thread

	{
	CTranscendenceWnd *pThis = (CTranscendenceWnd *)pData;

	while (true)
		{
		//	Wait for work to do

		SDL_SemWait(pThis->m_hWorkAvailableEvent);

		//	Do the work

		switch (pThis->m_iBackgroundState)
			{
			case bsQuit:
				return 0;

			case bsLoadUniverse:
				pThis->BackgroundLoadUniverse();
				break;

			case bsCreateGame:
				pThis->BackgroundNewGame();
				break;

			default:
				return 0;
			}

		//	Reset

		pThis->m_iBackgroundState = bsNone;
		}
	}

void CTranscendenceWnd::BackgroundLoadUniverse (void)

//	BackgroundLoadUniverse
//
//	Load the universe

	{
	//	Load the universe definition

	if (LoadUniverseDefinition() != NOERROR)
		return;

	//	Load high-score list

	ASSERT(m_pHighScoreList == NULL);
	m_pHighScoreList = new CHighScoreList;
	m_pHighScoreList->Load(CONSTLIT("HighScores.xml"));

	//	Set the default name of the player

	m_sPlayerName = m_pHighScoreList->GetMostRecentPlayerName();
	if (m_sPlayerName.IsBlank())
		m_sPlayerName = sysGetUserName();

	m_iPlayerGenome = m_pHighScoreList->GetMostRecentPlayerGenome();
	if (m_iPlayerGenome == genomeUnknown)
		m_iPlayerGenome = (mathRandom(1, 2) == 2 ? genomeHumanMale : genomeHumanFemale);

	//	Set the default adventure

	m_dwAdventure = 0;

	//	Set the default player ship

	m_dwPlayerShip = GetDefaultPlayerShip();
	if (m_dwPlayerShip == 0)
		m_sBackgroundError = CONSTLIT("No valid player ship class");
	}

void CTranscendenceWnd::BackgroundNewGame (void)

//	BackgroundNewGame
//
//	Complete creating a new game

	{
	if (StartNewGameBackground(&m_sBackgroundError) != NOERROR)
		{
		m_Universe.Reinit();
		m_GameFile.Close();
		return;
		}
	}

void CTranscendenceWnd::CreateBackgroundThread (void)

//	CreateBackgroundThread
//
//	Creates the background thread

	{
	m_hWorkAvailableEvent = SDL_CreateSemaphore(0);
	m_hBackgroundThread = ::kernelCreateThread(BackgroundThread, this);
	}

void CTranscendenceWnd::DestroyBackgroundThread (void)

//	DestroyBackgroundThread
//
//	Destroy the background thread

	{
	SetBackgroundState(bsQuit);

	SDL_WaitThread(m_hBackgroundThread, NULL);
	SDL_DestroySemaphore(m_hWorkAvailableEvent);
	}

void CTranscendenceWnd::SetBackgroundState (BackgroundState iState)

//	SetBackgroundState
//
//	Called by foreground thread when we want to do work

	{
	m_iBackgroundState = iState;
	SDL_SemPost(m_hWorkAvailableEvent);
	}

