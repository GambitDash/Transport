//	Kernel.h
//
//	Kernel definitions.

#ifndef INCL_KERNEL
#define INCL_KERNEL

//	API flags

#define API_FLAG_MASKBLT				0x00000001	//	MaskBlt is available
#define API_FLAG_WINNT					0x00000002	//	Running on Windows NT

class CTextFileLog;
struct SDL_Thread;

//	Initialization functions (Kernel.cpp)

typedef int (*ThreadProc)(void *);

void kernelCleanUp (void);
void kernelDebugLogMessage (const char *pszLine, ...);
void kernelDebugLogFlush (void);
BOOL kernelInit (void);
ALERROR kernelSetDebugLog (CTextFileLog *pLog, BOOL bAppend);
struct SDL_Thread *kernelCreateThread (ThreadProc pfStart, void *pData);

//	System functions

DWORD sysGetAPIFlags (void);
CString sysGetUserName (void);

#endif

