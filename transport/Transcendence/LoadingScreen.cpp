//	LoadingScreen.cpp
//
//	Show loading screen

#include "SDL.h"
#include "SDL_image.h"

#include "PreComp.h"
#include "Transcendence.h"
#include "XMLUtil.h"

#define STARGATE_WIDTH						128
#define STARGATE_HEIGHT						128

#define RGB_BAR								CG16bitImage::RGBValue(0, 2, 10)
#define RGB_LOADING_TEXT					CG16bitImage::RGBValue(128, 128, 128)
#define RGB_COPYRIGHT_TEXT					CG16bitImage::RGBValue(56, 63, 90)
#define STR_TYPEFACE						CONSTLIT("Tahoma")

#define Y_COPYRIGHT_TEXT					392

#define TIME_BLT_LOCK						15
#define TIME_BLT_NO_LOCK					15

extern bool g_Running;

void CTranscendenceWnd::AnimateLoading (void)

//	AnimateLoading
//
//	Animate loading screen

	{
	PaintLoadingScreen();

	//	Time our blts so that we can determine what method to use

	DWORD dwStartTimer;
	if (m_iCountdown <= (TIME_BLT_LOCK + TIME_BLT_NO_LOCK))
		dwStartTimer = SDL_GetTicks();

	BltScreen();

	if (m_iCountdown <= (TIME_BLT_LOCK + TIME_BLT_NO_LOCK))
		m_iBltTime[0] += (SDL_GetTicks() - dwStartTimer);

	m_iCountdown++;

	//	After we've got enough blts, try with no lock

	if (m_iCountdown == TIME_BLT_LOCK)
		{
		m_iBltTime[1] = m_iBltTime[0] / TIME_BLT_LOCK;
		m_iBltTime[0] = 0;
		}
	//	After blts with no lock, see which is faster

	else if ((m_iCountdown - TIME_BLT_LOCK) == TIME_BLT_NO_LOCK)
		{
		m_iBltTime[2] = m_iBltTime[0] / TIME_BLT_NO_LOCK;

		if (m_Options.bDebugVideo)
			{
			kernelDebugLogMessage("Blt with Lock: %d ms", m_iBltTime[1]);
			kernelDebugLogMessage("Blt without Lock: %d ms", m_iBltTime[2]);
			}

		//	If using a lock is faster, discard the surface

		if (m_Options.bWindowedMode || m_Options.bDebugManualBlt || m_iBltTime[1] < m_iBltTime[2])
			{
			if (m_Options.bDebugVideo)
				kernelDebugLogMessage("Blt with Lock enabled");
			}

		//	We're done with timing; start loading the universe

		SetBackgroundState(bsLoadUniverse);
		}

	//	See if we're done

	if (m_iCountdown > TIME_BLT_LOCK + TIME_BLT_NO_LOCK
			&& IsBackgroundDone())
		{
		if (!m_sBackgroundError.IsBlank())
			{
			kernelDebugLogMessage(m_sBackgroundError.GetASCIIZPointer());

			m_State = gsNone;
			g_Running = false;
			return;
			}

		StopLoading();

		//	Some debugging

#ifdef DEBUG_NEBULA_PAINTING
		{
		int i;
		int iCount = 1000;

		CSpaceEnvironmentType *pNebula = g_pUniverse->FindSpaceEnvironment(0x00009201);

		DWORD dwStart = SDL_GetTicks();
		for (i = 0; i < iCount; i++)
			{
			pNebula->Paint(m_Screen, 320, 320);
			}

		char szBuffer[1024];
		wsprintf(szBuffer, "Paint %d in %d ms\n", iCount, SDL_GetTicks() - dwStart);
		::OutputDebugString(szBuffer);
		}
#endif

		//	Start the intro screen

		StartIntro();
		}
	}

ALERROR CTranscendenceWnd::LoadUniverseDefinition (void)

//	LoadUniverseDefinition
//
//	Load the universe definition file

	{
	ALERROR error;

	//	Make sure the universe know about our various managers

	m_Universe.SetDebugMode(m_Options.bDebugGame);
	m_Universe.SetSoundMgr(&m_SoundMgr);
#ifdef DEBUG
	m_Universe.SetDebugOutput(this);
#endif

	//	Figure out what to load. If no extension is supplied, we check for an XML
	//	file first. Otherwise, we load the .tdb

	CString sGameFile;
	if (m_Options.bUseTDB)
		sGameFile = CONSTLIT("Transcendence.tdb");
	else
		sGameFile = CONSTLIT("Transcendence");

	//	Load the Transcendence Data Definition file that describes the universe.

	if ((error = m_Universe.Init(TransPath(sGameFile), &m_sBackgroundError)))
		return error;

	//	Initialize Transcendence extensions

	CCodeChain &CC = m_Universe.GetCC();
	if ((error = InitCodeChainExtensions(CC)))
		return error;

	return NOERROR;
	}

void CTranscendenceWnd::PaintLoadingScreen (void)

//	PaintLoadingScreen
//
//	Paints the loading screen

	{
	int cyBarHeight = (g_cyScreen - m_TitleImage.GetHeight()) / 2;
	int xImage = (g_cxScreen - m_TitleImage.GetWidth()) / 2;
	int yImage = (g_cyScreen - m_TitleImage.GetHeight()) / 2;

	//	Paint the parts that don't change

	if (m_bTitleInvalid)
		{
		//	Paint bars across top and bottom

		m_Screen.Fill(0, 
				0, 
				g_cxScreen, 
				cyBarHeight,
				RGB_BAR);

		m_Screen.Fill(0, 
				g_cyScreen - cyBarHeight, 
				g_cxScreen, 
				cyBarHeight,
				RGB_BAR);

		//	Paint image

		m_Screen.Blt(0,
				0,
				m_TitleImage.GetWidth(),
				m_TitleImage.GetHeight(),
				m_TitleImage,
				xImage,
				yImage);

		//	Paint the copyright text

		int cxWidth = m_Fonts.MediumHeavyBold.MeasureText(m_sCopyright);
		m_Fonts.MediumHeavyBold.DrawText(m_Screen,
				(g_cxScreen - cxWidth) / 2,
				yImage + Y_COPYRIGHT_TEXT,
				RGB_COPYRIGHT_TEXT,
				m_sCopyright);

		//	Paint the loading title

		CString sLoading = CONSTLIT("Loading");
		cxWidth = m_Fonts.SubTitle.MeasureText(sLoading, NULL);
		m_Fonts.SubTitle.DrawText(m_Screen,
				(g_cxScreen - cxWidth) / 2,
				m_rcMainScreen.bottom - (m_Fonts.SubTitle.GetHeight() + 32),
				RGB_LOADING_TEXT,
				sLoading);

		m_bTitleInvalid = false;
		}

	//	Animate the stargate

	if ((m_iCountdown % 2) == 0)
		{
		RECT rcSG;
		rcSG.left = (g_cxScreen - STARGATE_WIDTH) / 2;
		rcSG.right = rcSG.left + STARGATE_WIDTH;
		rcSG.top = m_rcScreen.bottom - ((RectHeight(m_rcScreen) - m_TitleImage.GetHeight()) / 2) - STARGATE_HEIGHT / 2;
		rcSG.bottom = rcSG.top + STARGATE_HEIGHT;

		//	Erase the stargate

		m_Screen.Blt(rcSG.left - xImage,
				m_TitleImage.GetHeight() - STARGATE_HEIGHT / 2,
				STARGATE_WIDTH,
				STARGATE_HEIGHT / 2,
				m_TitleImage,
				rcSG.left,
				rcSG.top);

		m_Screen.Fill(rcSG.left, 
				rcSG.top + STARGATE_HEIGHT / 2, 
				STARGATE_WIDTH, 
				STARGATE_HEIGHT / 2,
				RGB_BAR);

		//	Paint stargate

		m_Screen.ColorTransBlt(STARGATE_WIDTH * ((m_iCountdown / 2) % 12),
				0,
				STARGATE_WIDTH,
				STARGATE_HEIGHT,
				255,
				m_StargateImage,
				rcSG.left,
				rcSG.top);
		}
	}

ALERROR CTranscendenceWnd::StartLoading (void)

//	StartLoading
//
//	Start loading universe (on a different thread)

	{
	ALERROR error;
	SDL_Surface *surface;
	SDL_Surface *mask;

	m_sBackgroundError = NULL_STR;
	m_State = gsLoading;
	m_iCountdown = 0;
	m_iBltTime[0] = 0;

	//	Start a background thread to load the universe

	CreateBackgroundThread();

	//	Load a JPEG of the loading screen
	surface = IMG_Load("Resources/Title.jpg");
	if (surface == NULL)
		{
		return ERR_NOTFOUND;
		}
	error = m_TitleImage.CreateFromSurface(surface, NULL, false);
	SDL_FreeSurface(surface);
	if (error)
		return error;

	//	Load stargate image
	surface = IMG_Load("Resources/Stargate.jpg");
	if (surface == NULL)
		{
		kernelDebugLogMessage("Unable to load Stargate.jpg");
		return ERR_NOTFOUND;
		}
	mask = IMG_Load("Resources/StargateMask.bmp");
	if (mask == NULL)
		{
		kernelDebugLogMessage("Unable to load StargateMask.bmp");
		return ERR_NOTFOUND;
		}
	error = m_StargateImage.CreateFromSurface(surface, mask, false);
	SDL_FreeSurface(surface);
	SDL_FreeSurface(mask);
	if (error)
		{
		kernelDebugLogMessage("Failed to create stargate image.");
		return error;
		}

	//	Fonts

	m_bTitleInvalid = true;

	return NOERROR;
	}

void CTranscendenceWnd::StopLoading (void)

//	StopLoading
//
//	Clean up loading screen

	{
	ASSERT(m_State == gsLoading);

	m_TitleImage.Destroy();
	m_StargateImage.Destroy();
	}

