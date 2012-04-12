//	Kernel.cpp
//
//	Kernel boot
#include "SDL.h"
#include "SDL_thread.h"

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"

#include "Kernel.h"
#include "CLog.h"

static DWORD g_dwAPIFlags = 0;
static SDL_mutex *g_csKernel;
static CTextFileLog *g_pDebugLog = NULL;

struct THREADCTX
	{
	ThreadProc pfStart;
	LPVOID pData;
	};

//	Forwards

static void InitAPIFlags (void);
static int kernelThreadProc (LPVOID pData);

BOOL kernelInit (void)

//	kernelInit
//
//	Must be called before any other routines.

	{
	//	Since this routine must be called at the
	//	beginning for each thread, but globals are shared
	//	between threads, we check to make sure that
	//	we haven't already initialized them
	
	static bool inited = FALSE;

	if (inited)
		{
		return TRUE;
		}
	//	Initialize the strings subsystem

	if (CString::INTStringInit() != NOERROR)
		return FALSE;

	//	Initialize API flags

	InitAPIFlags();

	//	Initialize the mutex

	g_csKernel = SDL_CreateMutex();
	if (!g_csKernel)
		{
		return FALSE;
		}

	inited = TRUE;

	return TRUE;
	}

BOOL threadInit ()

	//	Initialize random number generator. This is
	//	done for each thread because we link with the multi-threaded
	//	runtime library, which keeps separate context for
	//	each thread.

	{
	/* XXX Not a very good random seed. */
	srand(0);

	return TRUE;
	}

void kernelCleanUp (void)

//	KernelCleanUp
//
//	Must be called for each thread to clean up

	{
	CString::INTStringCleanUp();

	//	Done logging

	kernelSetDebugLog(NULL, FALSE);

	//	Clean up critical section

	SDL_DestroyMutex(g_csKernel);

	}

DWORD sysGetAPIFlags (void)

//	sysGetAPIFlags
//
//	Returns API flags

	{
	return g_dwAPIFlags;
	}

void InitAPIFlags (void)

//	InitAPIFlags
//
//	Initialize API flags

	{

	}

ALERROR kernelSetDebugLog (CTextFileLog *pLog, BOOL bAppend)

//	kernelSetDebugLog
//
//	Sets the debug log file

	{
	ALERROR error;

	SDL_MutexTake(g_csKernel);

	//	Close previous log

	if (g_pDebugLog)
		{
		g_pDebugLog->LogOutput(ILOG_FLAG_TIMEDATE, "End logging session");
		g_pDebugLog->Close();
		}

	g_pDebugLog = pLog;

	//	Start new one

	if (pLog)
		{
		if ((error = pLog->Create(bAppend)))
			{
			SDL_MutexGive(g_csKernel);
			return error;
			}

		//	Output start of logging

		pLog->LogOutput(ILOG_FLAG_TIMEDATE, "--------------------------------------------------------------------------------");
		pLog->LogOutput(ILOG_FLAG_TIMEDATE, "Start logging session");
		}

	SDL_MutexGive(g_csKernel);

	return NOERROR;
	}

void kernelDebugLogMessage (const char *pszLine, ...)

//	kernelDebugLogMessage
//
//	Log debug output

	{
	va_list ap;
	char szBuffer[4096];

	va_start(ap, pszLine);

	vsprintf(szBuffer, pszLine, ap);

	SDL_MutexTake(g_csKernel);

	if (g_pDebugLog)
		{
		g_pDebugLog->LogOutput(ILOG_FLAG_TIMEDATE, szBuffer);
		}
	else
		{
		printf(szBuffer);
		}

	SDL_MutexGive(g_csKernel);
	}

void kernelDebugLogFlush ()

//	kernelDebugLogFlush
//
//	Close and reopen log file to force flush all output.

	{
	SDL_MutexTake(g_csKernel);

	if (g_pDebugLog)
		{
		g_pDebugLog->Flush();
		}

	SDL_MutexGive(g_csKernel);
	}


SDL_Thread *kernelCreateThread (ThreadProc pfStart, LPVOID pData)

//	kernelCreateThread
//
//	Creates a new thread

	{
	SDL_Thread *thr;
	THREADCTX *pCtx;

	pCtx = new THREADCTX;
	if (pCtx == NULL)
		return NULL;

	pCtx->pfStart = pfStart;
	pCtx->pData = pData;

	thr = SDL_CreateThread(kernelThreadProc, pCtx);

	return thr;
	}

int kernelThreadProc (LPVOID pData)
	{
	THREADCTX *pCtx = (THREADCTX *)pData;
	DWORD dwResult;
	ThreadProc pfStart = pCtx->pfStart;
	LPVOID pUserData = pCtx->pData;
	delete pCtx;

	threadInit();

	dwResult = pfStart(pUserData);

	return dwResult;
	}

