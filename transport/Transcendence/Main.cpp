//	Main.cpp
//
//	Main Windows program entry-point

#include "PreComp.h"
#include "Transcendence.h"

#include <mmsystem.h>

#define TIMER_RESOLUTION						1
#define FRAME_DELAY								(1000 / g_TicksPerSecond)		//	Delay in mSecs

//	Globals

static const char *g_pszClassName = "transcendence_class";
HINSTANCE g_hInst = NULL;

//	Forwards

ALERROR CreateMainWindow (HINSTANCE hInstance, int nCmdShow, LPSTR lpCmdLine, HWND *rethWnd);
int MainLoop (HWND hWnd, int iFrameDelay);
void ParseCommandLine (char *pszCmdLine, SCommandLineOptions *retOptions);
inline CTranscendenceWnd *GetCtx (HWND hWnd) { return (CTranscendenceWnd *)GetWindowLong(hWnd, GWL_USERDATA); }

LONG APIENTRY MainWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam);

int WINAPI WinMain (HINSTANCE hInstance, 
					HINSTANCE hPrevInstance, 
					LPSTR lpCmdLine, 
                    int nCmdShow)

//	WinMain
//
//	Main Windows entry-point

	{
	ALERROR error;
	HWND hWnd;
	int iResult;

	g_hInst = hInstance;

	//	Initialize alchemy

	if (!kernelInit())
		return 0;

	//	Create the main window

	if (error = CreateMainWindow(hInstance, nCmdShow, lpCmdLine, &hWnd))
		{
		kernelCleanUp();
		return 0;
		}

	//	Event loop

	iResult = MainLoop(hWnd, FRAME_DELAY);

	//	Done

	kernelCleanUp();

	return iResult;
	}

ALERROR CreateMainWindow (HINSTANCE hInstance, int nCmdShow, LPSTR lpCmdLine, HWND *rethWnd)

//	CreateMainWindow
//
//	Registers and creates the main window

	{
    WNDCLASSEX wc;

	//	Parse command line

	SCommandLineOptions Options;
	ParseCommandLine(lpCmdLine, &Options);

	//	Register window

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
	wc.hIcon = ::LoadIcon(hInstance, "AppIcon");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = g_pszClassName;
	if (!RegisterClassEx(&wc))
		return ERR_FAIL;

	//	Create the window

    *rethWnd = CreateWindowEx(0,	// WS_EX_TOPMOST,
			g_pszClassName, 
			"Transcendence",
			(Options.bWindowedMode ? WS_OVERLAPPEDWINDOW : WS_POPUP),
			0,
			0, 
			GetSystemMetrics(SM_CXSCREEN),
			GetSystemMetrics(SM_CYSCREEN), 
			NULL,
			NULL,
			hInstance,
			&Options);

	if (*rethWnd == NULL)
		return ERR_FAIL;

	//	Show the window

	if (Options.bWindowedMode)
		ShowWindow(*rethWnd, nCmdShow);
	else
		ShowWindow(*rethWnd, SW_SHOWMAXIMIZED);
    UpdateWindow(*rethWnd);

	return NOERROR;
	}

int MainLoop (HWND hWnd, int iFrameDelay)

//	MainLoop
//
//	Runs until the program terminates

	{
	MSG msg;
	CTranscendenceWnd *pTrans = GetCtx(hWnd);

	timeBeginPeriod(TIMER_RESOLUTION);

	//	Start main loop

	DWORD dwStartTime = timeGetTime();

	while (TRUE)
		{
		//	Tell the main window that we're animating

		pTrans->Animate();

		//	Process all events

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE|PM_NOYIELD))
			{
			BOOL bHandled = FALSE;

			//	Check to see if we're quitting

			if (msg.message == WM_QUIT)
				return (int)msg.wParam;

			//	If we haven't handled it yet, handle it now

			if (!bHandled)
				{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				}
			}

		//	Figure out how long until our next animation

		DWORD dwNow = timeGetTime();
		DWORD dwNextFrame = dwStartTime + iFrameDelay;
		if (dwNextFrame > dwNow)
			{
			::Sleep(dwNextFrame - dwNow);
			dwStartTime = dwNextFrame;
			}
		else
			dwStartTime = dwNow;
		}

	timeEndPeriod(TIMER_RESOLUTION);

	return 0;
	}

void ParseCommandLine (char *pszCmdLine, SCommandLineOptions *retOptions)

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

	::ZeroMemory(retOptions, sizeof(SCommandLineOptions));

	//	Parse

	char *argv[2];
	argv[0] = "Transcendence";
	argv[1] = pszCmdLine;
	CXMLElement *pCmdLine;
	if (error = CreateXMLElementFromCommandLine(2, argv, &pCmdLine))
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

