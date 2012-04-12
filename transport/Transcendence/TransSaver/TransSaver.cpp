//	TransSaver.cpp
//
//	Transcendence Screen Saver
//	Copyright (c) 2007 by George Moromisato

#include "PreComp.h"

const int FRAME_DELAY = (1000 / g_TicksPerSecond);

LRESULT WINAPI ScreenSaverProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
	static int iTimerID;

	switch (message)
		{
		case WM_CREATE:
			iTimerID = ::SetTimer(hwnd, 1, FRAME_DELAY, NULL);
			break;

		case WM_DESTROY:
			if (iTimerID)
				::KillTimer(hwnd, iTimerID);
			break;

		case WM_ERASEBKGND:
			{
			HDC hDC = ::GetDC(hwnd);
			RECT rcRect;
			::GetClientRect(hwnd, &rcRect);
			gdiFillRect(hDC, &rcRect, RGB(0, 100, 0));
			::ReleaseDC(hwnd, hDC);
			break;
			}

		case WM_TIMER:
			break;

        default:
			return (::DefScreenSaverProc(hwnd, message, wParam, lParam));
		}

	return 0;
	}

BOOL WINAPI ScreenSaverConfigureDialog (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
	return TRUE;
	}

BOOL WINAPI RegisterDialogClasses (HANDLE hInst)
	{
	return TRUE;
	}
