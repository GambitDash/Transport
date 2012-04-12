#ifndef INCL_PORTAGE
#define INCL_PORTAGE

#include <string.h>

// For min/max
#include <algorithm>

typedef unsigned long       DWORD;
typedef unsigned long       DWORD_PTR;
typedef unsigned short      WORD;
typedef long                LONG;
typedef DWORD 					 ALERROR;
typedef DWORD               BOOL;
typedef unsigned char       BYTE;
typedef void *              LPVOID;
typedef unsigned long long  ULONG64;
typedef void *              LPTSTR;
typedef char                CHAR;
typedef unsigned int *		 PUINT;
typedef DWORD               COLORREF;
typedef BYTE                AlphaArray5 [32];
typedef BYTE                AlphaArray6 [64];

struct RECT {
	RECT() : top(0), bottom(0), right(0), left(0) { }
	RECT(LONG t, LONG b, LONG r, LONG l) : top(t), bottom(b), right(r), left(l) { }
	LONG top;
	LONG bottom;
	LONG right;
	LONG left;
};

struct POINT
{
	LONG  x;
	LONG  y;
};

/* Placeholder defines. */
#define TRUE                true
#define FALSE               false

#define Max                 std::max
#define Min                 std::min

#define STRSIG(a, b, c, d)  ( ((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

#define SDL_MutexTake(m)												\
	if (SDL_mutexP((m)) == -1)											\
		{																		\
		ASSERT(0 && "failed mutex take");							\
		}

#define SDL_MutexGive(m)												\
	if (SDL_mutexV((m)) == -1)											\
		{																		\
		ASSERT(0 && "failed mutex give");							\
		}

template <class VALUE> void Swap (VALUE &a, VALUE &b)
	{
	VALUE temp = a;
	a = b;
	b = temp;
	}

#define OutputDebugString(x) printf("%s", (x));

#define MAKEWORD(a, b)      ((WORD)(((BYTE)((DWORD_PTR)(a) & 0xff)) | ((WORD)((BYTE)((DWORD_PTR)(b) & 0xff))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)((DWORD_PTR)(a) & 0xffff)) | ((DWORD)((WORD)((DWORD_PTR)(b) & 0xffff))) << 16))
#define LOWORD(l)           ((WORD)((DWORD_PTR)(l) & 0xffff))
#define HIWORD(l)           ((WORD)((DWORD_PTR)(l) >> 16))
#define LOBYTE(w)           ((BYTE)((DWORD_PTR)(w) & 0xff))
#define HIBYTE(w)           ((BYTE)((DWORD_PTR)(w) >> 8))

//#define GetRValue(rgb)      (LOBYTE(rgb))
//#define GetGValue(rgb)      (LOBYTE(((WORD)(rgb)) >> 8))
//#define GetBValue(rgb)      (LOBYTE((rgb)>>16))
//#define GetAValue(rgb)      (HIBYTE((rgb)>>16))
//#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

#include "CRect.h"

// Mouse capture routines
void SetCapture();
void ReleaseCapture();

// Standard includes that it looks like everyone needs.
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"

#ifdef BUILD_DIRTY
/* Things to expunge all references of. */
#define SPI_GETWORKAREA 0
#define __stdcall
#define wvsprintf vsprintf
#define VER_PLATFORM_WIN32_NT 0
#define FW_BOLD 0
#define FW_NORMAL 0
#define ANSI_CHARSET 0
#define OUT_TT_ONLY_PRECIS 0
#define CLIP_DEFAULT_PRECIS 0
#define ANTIALIASED_QUALITY 0
#define FF_DONTCARE 0
#define MEM_DECOMMIT 0
#define MEM_COMMIT 0
#define MEM_RELEASE 0
#define MEM_RESERVE 0
#define PAGE_NOACCESS 0
#define PAGE_READWRITE 0
#define ERROR_SUCCESS 0
#define REG_SZ 0
#define HKEY_CURRENT_USER 0
#define KEY_READ 0
#define KEY_WRITE 0
#define REG_OPTION_NON_VOLATILE 0
#define HRSRC DWORD
#define HGLOBAL DWORD
#define RT_RCDATA 0
#define OPEN_ALWAYS 0
#define FILE_END 0
#define TRANSPARENT 0
#define SRCAND 0
#define SRCPAINT 0
#define SRCCOPY 0
#define OUT_DEFAULT_PRECIS 0
#define CLIP_EMBEDDED 0
#define DEFAULT_QUALITY 0
#define VARIABLE_PITCH 0
#define FF_SWISS 0
#define IMAGE_BITMAP 0
#define LR_CREATEDIBSECTION 0#define GENERIC_READ 0
#define GENERIC_WRITE 0
#define CREATE_NEW 0
#define CREATE_ALWAYS 0
#define FILE_ATTRIBUTE_NORMAL 0
#define FILE_BEGIN 0
#define FILE_SHARE_READ 0
#define OPEN_EXISTING 0
#define ERROR_FILE_NOT_FOUND 0
#define PAGE_READONLY 0
#define FILE_MAP_READ 0
#define WINAPI
#define INVALID_HANDLE_VALUE (0)
#define UnionRect(x, y, z) (x)
#define MB_OK 			1
#define MB_ICONSTOP  2
#define INFINITE 		0
#define HeapAlloc(x, y, n) (new BYTE[(n)])
#define HeapFree(x, y, p) (delete []((BYTE *)(p)))
#define MAKEINTRESOURCE(x) (NULL)
#define SM_CXSCREEN 0
#define SM_CYSCREEN 0
#define WM_CLOSE 0
#define SIZE_MINIMIZED 0
typedef DWORD               HANDLE;
typedef DWORD *             HINSTANCE;
typedef DWORD               HRESULT;
typedef DWORD					 CRITICAL_SECTION;
typedef DWORD               WIN32_FIND_DATA;
typedef DWORD               HKEY;
typedef DWORD               HDC;
typedef DWORD *             HBITMAP;
typedef DWORD *             HFONT;
typedef DWORD *             HPALETTE;
typedef DWORD               COLORREF;
typedef DWORD               HRGN;
typedef DWORD               HMODULE;
typedef DWORD             (*LPTHREAD_START_ROUTINE)(void*);
typedef DWORD *             LPDIRECTDRAW7;
typedef DWORD *             LPDIRECTDRAWSURFACE7;
typedef DWORD *             BITMAPINFO;
typedef DWORD               HWND;
typedef DWORD               DDSURFACEDESC2;
typedef DWORD               DDPIXELFORMAT;
typedef DWORD *             LPDIRECTSOUNDBUFFER;
typedef DWORD *             LPDIRECTSOUND;
typedef DWORD               HMMIO;
typedef DWORD *             LPDIRECTDRAWCLIPPER;

typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

typedef struct _MEMORY_BASIC_INFORMATION {
    void *BaseAddress;
    void *AllocationBase;
    DWORD AllocationProtect;
    DWORD RegionSize;
    DWORD State;
    DWORD Protect;
    DWORD Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;
typedef struct _OSVERSIONINFO {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    CHAR   szCSDVersion[ 128 ];     // Maintenance string for PSS usage
} OSVERSIONINFO, *POSVERSIONINFO, *LPOSVERSIONINFO;

typedef struct tagVS_FIXEDFILEINFO
{
    DWORD   dwSignature;            /* e.g. 0xfeef04bd */
    DWORD   dwStrucVersion;         /* e.g. 0x00000042 = "0.42" */
    DWORD   dwFileVersionMS;        /* e.g. 0x00030075 = "3.75" */
    DWORD   dwFileVersionLS;        /* e.g. 0x00000031 = "0.31" */
    DWORD   dwProductVersionMS;     /* e.g. 0x00030010 = "3.10" */
    DWORD   dwProductVersionLS;     /* e.g. 0x00000031 = "0.31" */
    DWORD   dwFileFlagsMask;        /* = 0x3F for version "0.42" */
    DWORD   dwFileFlags;            /* e.g. VFF_DEBUG | VFF_PRERELEASE */
    DWORD   dwFileOS;               /* e.g. VOS_DOS_WINDOWS16 */
    DWORD   dwFileType;             /* e.g. VFT_DRIVER */
    DWORD   dwFileSubtype;          /* e.g. VFT2_DRV_KEYBOARD */
    DWORD   dwFileDateMS;           /* e.g. 0 */
    DWORD   dwFileDateLS;           /* e.g. 0 */
} VS_FIXEDFILEINFO;

typedef struct tagTEXTMETRICA
{
    LONG        tmHeight;
    LONG        tmAscent;
    LONG        tmDescent;
    LONG        tmInternalLeading;
    LONG        tmExternalLeading;
    LONG        tmAveCharWidth;
    LONG        tmMaxCharWidth;
    LONG        tmWeight;
    LONG        tmOverhang;
    LONG        tmDigitizedAspectX;
    LONG        tmDigitizedAspectY;
    BYTE        tmFirstChar;
    BYTE        tmLastChar;
    BYTE        tmDefaultChar;
    BYTE        tmBreakChar;
    BYTE        tmItalic;
    BYTE        tmUnderlined;
    BYTE        tmStruckOut;
    BYTE        tmPitchAndFamily;
    BYTE        tmCharSet;
} TEXTMETRIC, *PTEXTMETRIC, *NPTEXTMETRIC, *LPTEXTMETRIC;

typedef struct _ABC {
    int     abcA;
    unsigned int abcB;
    int     abcC;
} ABC;

typedef struct tagBITMAP
{
    LONG        bmType;
    LONG        bmWidth;
    LONG        bmHeight;
    LONG        bmWidthBytes;
    WORD        bmPlanes;
    WORD        bmBitsPixel;
    LPVOID      bmBits;
} BITMAP;

void MessageBox(HWND, const void *, const void *, DWORD) { }
void ZeroMemory(void *p, DWORD len) { memset(p, 0, len); }
void WaitForSingleObject(DWORD, DWORD) { }
void ResetEvent(DWORD) { }
HANDLE CreateEvent(void *, bool, bool, void *) { }
void CloseHandle(DWORD) { }
void SetEvent(DWORD) { }
HANDLE GetProcessHeap() { return 0; }
BOOL IntersectRect(RECT *, const RECT *, const RECT *) {  return FALSE; }
BOOL OffsetRect(RECT *, DWORD, DWORD) {  return FALSE; }
BOOL PtInRect(RECT *, POINT) {  return FALSE; }
void DeleteObject(void *) { }
void MessageBeep(DWORD) { }
DWORD GetTickCount() { return 0; }
DWORD GetSystemMetrics(DWORD) { return 0; }
void SendMessage(HWND, DWORD, DWORD, DWORD) { }
void PostMessage(HWND, DWORD, DWORD, DWORD) { }
void SetCurrentDirectory(const char *) { }
void ShowCursor(bool) { }
void ClientToScreen(HWND, LPPOINT) { }
void InvalidateRect(HWND, void *, bool) { }
void GetClientRect(HWND, RECT *) { }
HANDLE CreateFile(void *, DWORD, DWORD, void *, DWORD, DWORD, void *) { return 0; }
DWORD ReadFile(HANDLE, void *, DWORD, void *, void *) { return 0; }
DWORD WriteFile(HANDLE, void *, DWORD, void *, void *) { return false; }
void DeleteFile(void *) { }
DWORD SetFilePointer(HANDLE, DWORD, void *, DWORD) { return 0; }
DWORD FindFirstFile(void *, void *) { }
void FindClose(HANDLE) { }
DWORD FindNextFile(HANDLE, void *) { return 0; }
DWORD GetLastError() { return 0; }
void UnmapViewOfFile(void *) { }
HANDLE MapViewOfFile(HANDLE, DWORD, DWORD, DWORD, DWORD) { return 0; }
DWORD GetFileSize(HANDLE, void *) { return 0; }
HANDLE CreateFileMapping(HANDLE, void *, DWORD, DWORD, DWORD, void *) { return 0; }
void VirtualFree(void *, DWORD, DWORD);
void *VirtualAlloc(void *, DWORD, DWORD, DWORD);
void VirtualQuery(void *, PMEMORY_BASIC_INFORMATION, DWORD) { }
void RegCloseKey(HKEY) { }
DWORD RegOpenKeyEx(DWORD, const void *, DWORD, DWORD, void *) { return 0; }
DWORD RegQueryValueEx(HKEY, void *, void *, void *, void *, void *) { return 0; }
void RegSetValueEx(HKEY, void *, DWORD, DWORD, void *, DWORD) { }
DWORD RegCreateKeyEx(HKEY, const void *, DWORD, void *, DWORD, DWORD, void *, void *, void *) { return 0; }
HRSRC FindResource(DWORD, const void *, DWORD) { return 0; }
DWORD SizeofResource(DWORD, HRSRC) { return 0; }
DWORD LoadResource(DWORD, HRSRC) { return 0; }
DWORD LockResource(DWORD) { return 0; }
void InitializeCriticalSection(void *) { }
void DeleteCriticalSection(void *) { }
void EnterCriticalSection(void *) { }
void LeaveCriticalSection(void *) { }
char CharUpper(void *) { return ' '; }
void *CharLower(void *) { return NULL; }
void CharUpperBuff(void *, DWORD) { }
DWORD LoadString(void *, DWORD, void *, DWORD) { return 0; }
void SetEndOfFile(HANDLE) { }
void GetLocalTime(void *) { }
void FlushFileBuffers(HANDLE) { }
DWORD InterlockedIncrement(void *) { return 0; }
DWORD InterlockedDecrement(void *) { return 0; }
DWORD GetVersionEx(void *) { return 0; }
HDC CreateCompatibleDC(void *) { return 0; }
HBITMAP CreateCompatibleBitmap(HDC, DWORD, DWORD) { }
void *SelectObject(HDC, HBITMAP) { return 0; }
DWORD MaskBlt(HDC, DWORD, DWORD, DWORD, DWORD, HDC, DWORD, DWORD, HBITMAP, DWORD, DWORD, DWORD) { return 0; }
void DeleteDC(HDC) { }
HANDLE _beginthreadex(void *, DWORD, unsigned int (*)(void *), void *, DWORD, void *) { return 0; }
DWORD GetModuleFileName(void *, void *, DWORD) { return 0; }
DWORD GetFileVersionInfoSize(void *, void *) { return 0; }
DWORD GetFileVersionInfo(void *, DWORD, DWORD, void *) { return 0; }
DWORD VerQueryValue(void *, const void *, void *, void *) { return 0; }
DWORD CreateDirectory(void *, void *) { return 0; }
DWORD GetFileAttributes(void *) { return 0; }
void GetSystemTime(void *) { }
void GetUserName(void *, void *) { }
void SystemParametersInfo(DWORD, DWORD, void *, DWORD) { }
void ReleaseCapture() { }
void SetCapture(HANDLE) { }
bool IsRectEmpty(void *) { }
DWORD GetObject(DWORD *, DWORD, void *) { return 0; }

HFONT CreateFont(DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, void *) { return 0; }
void GetTextMetrics(HDC, TEXTMETRIC *) { }
COLORREF SetTextColor(HDC, DWORD) { return 0; }
COLORREF SetBkColor(HDC, DWORD) { return 0; }
void SetBkMode(HDC, DWORD) { }
void TextOut(HDC, DWORD, DWORD, void *, DWORD) { }
void GetCharABCWidths(HDC, DWORD, DWORD, void *) {}
void GetCharWidth(HDC, DWORD, DWORD, void *) { }
void BitBlt(HDC, DWORD, DWORD, DWORD, DWORD, HDC, DWORD, DWORD, DWORD) { }
HBITMAP LoadImage(HINSTANCE, DWORD, DWORD, DWORD, DWORD, DWORD);
#endif
#endif

