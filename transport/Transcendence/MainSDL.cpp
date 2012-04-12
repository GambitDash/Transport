//	Main.cpp
//
//	Main Windows program entry-point

#include "PreComp.h"
#include "Transcendence.h"

#include "SDL.h"
#include "SDL_syswm.h"
#include "SDL_ttf.h"
#include "SDL_gfxPrimitives.h"
#include "assert.h"

// XXX Conflicts with SDL
// #define TIMER_RESOLUTION						1
#define FRAME_DELAY								(1000 / g_TicksPerSecond)		//	Frame delay in mSecs

//	Forwards
ALERROR CreateMainWindow(void);
static ALERROR MainLoop(void);
static void ParseCommandLine (char *pszCmdLine, SCommandLineOptions *retOptions);
void DestroyMainWindow(void);

ALERROR MainWndProc(CTranscendenceWnd *wnd, void *evt);

CTranscendenceWnd *g_pTrans = NULL;
extern bool *g_kbmap;

int main(int argc, char *argv[])
	// main
	//
	// Main SDL entry-point
	{
	if (!kernelInit())
		return 0;

	g_kbmap = new bool[SDLK_LAST];
	for (int i = 0; i < SDLK_LAST; i++)
		g_kbmap[i] = false;

	if (CreateMainWindow())
		{
		if (TTF_WasInit())
			TTF_Quit();
		kernelCleanUp();
		delete []g_kbmap;
		return 0;
		}

	atexit(SDL_Quit);

	MainLoop();

	DestroyMainWindow();

	delete []g_kbmap;

	kernelCleanUp();

	SDL_Quit();
	if (TTF_WasInit())
		TTF_Quit();

	return 0;
	}

void BlitTest(SDL_Surface *surface)
{
	int ticks;
	int frame_delay = 0;
	int blit_delay = 0;
	int draw_delay = 0;
	int frames = 0;
	while (1) {
		if (!(frames % 10)) {
			printf("%d - Frame: %d Blit: %d\n", frames, frame_delay / 10, blit_delay / 10);
			fflush(stdout);
			frame_delay = 0;
			blit_delay = 0;
			if (frames == 200) exit(0);
		}
		ticks = SDL_GetTicks();

//		int x;
//		for (x = 0; x < 500; x++) {
//			boxRGBA(surface, 0, 0, 1024, 768, 0xff, 0, 0, 0xff);
//		}
//		SDL_BlitSurface(input, NULL, screen, NULL);
		//SDL_BlitSurface(img, NULL, screen, NULL);

		blit_delay += SDL_GetTicks() - ticks;
		ticks = SDL_GetTicks();

		SDL_Flip(surface);
		//SDL_UpdateRect(screen, 0, 0, 0, 0);

		frame_delay += SDL_GetTicks() - ticks;
		frames++;
	}
}

CGImage *g_screen;

ALERROR CreateMainWindow(void)
	// CreateMainWindow
	//
	// Registers and creates the main window
	{
	SDL_Surface *surface;
	SDL_SysWMinfo	wmInfo;
	
	long iResult;
	CString sError;
	SCommandLineOptions cmdOpts;
	bool b_fullScreen = false;
	const struct SDL_VideoInfo *info;
	static struct SDL_PixelFormat fmt;

	// XXX No command line support atm.
	//	ParseCommandLine(lpCmdLine, &Options);

	if (TTF_Init() == -1)
		return ERR_FAIL;

	putenv("SDL_VIDEO_WINDOW_POS");
	putenv("SDL_VIDEO_CENTERED=1");

	if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0 || !SDL_GetVideoInfo())
	   return ERR_FAIL;

	const SDL_VideoInfo *vid_info = SDL_GetVideoInfo();
	if (vid_info->current_w <= 1024 || vid_info->current_h <= 768)
		{
		b_fullScreen = TRUE;
		}

	g_cxScreen = 1024;
	g_cyScreen = 768;

	printf("Display is %dx%d @ %d bpp\n", vid_info->current_w, vid_info->current_h, vid_info->vfmt->BitsPerPixel);
	surface = SDL_SetVideoMode( g_cxScreen, g_cyScreen, vid_info->vfmt->BitsPerPixel, SDL_HWSURFACE | SDL_DOUBLEBUF);
	if (surface == NULL) {
		assert(0 && "ohnoes");
		}
	printf("Created video surface %dx%d@%dbpp in %s mode.\n", surface->w, surface->h, surface->format->BitsPerPixel, surface->flags & SDL_HWSURFACE ? "hardware" : "software");
//BlitTest(surface);

	if (b_fullScreen)
		{
		SDL_WM_ToggleFullScreen(surface);
		} 

	g_screen = new CGImage();
	g_screen->BindSurface(surface);
	
	info = SDL_GetVideoInfo();
	memcpy(&fmt, info->vfmt, sizeof(struct SDL_PixelFormat));
	fmt.Aloss = 0;
	fmt.Ashift = 24;
	fmt.Amask = 0xFF000000;
	CGImage::DispPixFormat = &fmt;

	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	SDL_VERSION(&wmInfo.version);
	SDL_GetWMInfo(&wmInfo);

	g_pTrans = new CTranscendenceWnd();

	if (g_pTrans == NULL)
		return ERR_FAIL;

	memset(&cmdOpts, 0, sizeof(SCommandLineOptions));
	cmdOpts.bWindowedMode = !b_fullScreen;
	cmdOpts.bDirectX = false;
	cmdOpts.bDebugVideo = true;
	cmdOpts.bNoSound = false;
	g_pTrans->m_bMinimized = false;
	iResult = g_pTrans->WMCreate(&cmdOpts, &sError);

	if (iResult != 0)
		{
		delete g_pTrans;
		g_pTrans = NULL;
		return iResult;
		}

	iResult = g_pTrans->WMSize(g_cxScreen, g_cyScreen, 0);

	if (iResult != 0)
		{
		delete g_pTrans;
		g_pTrans = NULL;
		return iResult;
		}
	return NOERROR;
	}

BOOL g_Running = true;

static ALERROR MainLoop(void)
	// MainLoop
	//
	// Manage SDL events.
	{
	SDL_Event	evt;
	DWORD			dwNow;
	DWORD			dwNextFrame;
	DWORD			dwStartTime = SDL_GetTicks();

	while(g_Running)
		{
		dwNextFrame = dwStartTime + FRAME_DELAY;

		// Render the screen.
		g_pTrans->Animate();

		while (SDL_PollEvent(&evt))
			{
			if (MainWndProc(g_pTrans, &evt))
				{
				return NOERROR;
				}
			}

		// Stall for VSYNC.
		dwNow = SDL_GetTicks();
		if (dwNextFrame > dwNow)
			{
			SDL_Delay(dwNextFrame - dwNow);
			dwStartTime = dwNextFrame;
			}
		else
			{
			dwStartTime = dwNow;
			}
		}

	return NOERROR;
	}

void DestroyMainWindow(void)
	{
	long iResult = 0;

	if (g_pTrans)
		{
		iResult = g_pTrans->WMDestroy();
		delete g_pTrans;
		g_pTrans = NULL;
		delete g_screen;
		}
	}

static void ParseCommandLine (char *pszCmdLine, SCommandLineOptions *retOptions)

//	ParseCommandLine
//
//	Parses the command line
//
//	/debug				Use debug ship and starting system
//	/windowed			Run in windowed mode (as opposed to full screen)
//	/nosound			No music or sound
//	/nolog				Do not output Debug.log
//	/useTDB				Use Transcendence.tdb instead of .xml
//	/debugVideo			Output debug info on video system
//	/debugNonExclusive	No exclusive mode in DirectX
//	/debugManualBlt		Do not use DirectX to blt to screen buffer

	{
	ALERROR error;

	//	Initialize defaults (in case of error parsing, we still end up
	//	with valid values).

	memset(retOptions, 0, sizeof(SCommandLineOptions));

	//	Parse

	char *argv[2];
	argv[0] = "Transcendence";
	argv[1] = pszCmdLine;
	CXMLElement *pCmdLine;
	if ((error = CreateXMLElementFromCommandLine(2, argv, &pCmdLine)))
		return;

	//	If we're in debug mode then there are various game changes

	if (pCmdLine->GetAttributeBool(CONSTLIT("debug")))
		{
		retOptions->bDebugMode = true;
		retOptions->bDebugGame = true;
		}

	//	Turn debug log off

	if (pCmdLine->GetAttributeBool(CONSTLIT("nolog")))
		retOptions->bNoDebugLog = true;

	//	No sound

	if (pCmdLine->GetAttributeBool(CONSTLIT("nosound")))
		retOptions->bNoSound = true;

	//	Debug video problems

	if (pCmdLine->GetAttributeBool(CONSTLIT("debugVideo")))
		retOptions->bDebugVideo = true;

	if (pCmdLine->GetAttributeBool(CONSTLIT("debugNonExclusive")))
		retOptions->bDebugNonExclusive = true;

	if (pCmdLine->GetAttributeBool(CONSTLIT("debugManualBlt")))
		retOptions->bDebugManualBlt = true;

	if (pCmdLine->GetAttributeBool(CONSTLIT("dx")))
		retOptions->bDirectX = true;

	if (pCmdLine->GetAttributeBool(CONSTLIT("windowed")))
		retOptions->bWindowedMode = true;

	if (pCmdLine->GetAttributeBool(CONSTLIT("useTDB")))
		retOptions->bUseTDB = true;

	delete pCmdLine;
	}

