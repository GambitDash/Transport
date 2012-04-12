//	MainWndProc.cpp
//
//	Implementation of main window proc

#include "PreComp.h"
#include "Transcendence.h"
#include "XMLUtil.h"

CTranscendenceWnd *g_pTrans = NULL;
inline CTranscendenceWnd *GetCtx (HWND hWnd) { return (CTranscendenceWnd *)GetWindowLong(hWnd, GWL_USERDATA); }
LONG APIENTRY MainWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam);

LONG APIENTRY MainWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam)

//	MainWndProc
//
//	WndProc for main window

	{
	switch (message)
		{
		case WM_ACTIVATEAPP:
			return GetCtx(hWnd)->WMActivateApp(wParam ? true : false);

		case WM_CHAR:
			return GetCtx(hWnd)->WMChar((char)wParam, (DWORD)lParam);

		case WM_CLOSE:
			return GetCtx(hWnd)->WMClose();

		case WM_CREATE:
			{
			long iResult;
			g_pTrans = new CTranscendenceWnd(hWnd);

			if (g_pTrans == NULL)
				return -1;

			::SetWindowLong(hWnd, GWL_USERDATA, (LONG)g_pTrans);

			CString sError;
			iResult = g_pTrans->WMCreate((LPCREATESTRUCT)lParam, &sError);
			if (iResult != 0)
				{
				::MessageBox(NULL, sError.GetASCIIZPointer(), "Transcendence", MB_OK);

				delete g_pTrans;
				g_pTrans = NULL;
				::SetWindowLong(hWnd, GWL_USERDATA, 0);
				return iResult;
				}

			return 0;
			}

		case WM_DESTROY:
			{
			long iResult = 0;

			if (g_pTrans)
				{
				iResult = g_pTrans->WMDestroy();
				delete g_pTrans;
				g_pTrans = NULL;
				SetWindowLong(hWnd, GWL_USERDATA, (LONG)g_pTrans);
				}

			//	Quit the app

			PostQuitMessage(0);

			return iResult;
			}

		case WM_DISPLAYCHANGE:
			return GetCtx(hWnd)->WMDisplayChange((int)wParam, (int)LOWORD(lParam), (int)HIWORD(lParam));

		case WM_KEYDOWN:
			return GetCtx(hWnd)->WMKeyDown((int)wParam, lParam);

		case WM_KEYUP:
			return GetCtx(hWnd)->WMKeyUp((int)wParam, lParam);

		case WM_LBUTTONDBLCLK:
			return GetCtx(hWnd)->WMLButtonDblClick((int)LOWORD(lParam), (int)HIWORD(lParam), wParam);

		case WM_LBUTTONDOWN:
			return GetCtx(hWnd)->WMLButtonDown((int)LOWORD(lParam), (int)HIWORD(lParam), wParam);

		case WM_LBUTTONUP:
			return GetCtx(hWnd)->WMLButtonUp((int)LOWORD(lParam), (int)HIWORD(lParam), wParam);

		case WM_MOUSEMOVE:
			return GetCtx(hWnd)->WMMouseMove((int)LOWORD(lParam), (int)HIWORD(lParam), wParam);

		case WM_MOVE:
			return GetCtx(hWnd)->WMMove((int)LOWORD(lParam), (int)HIWORD(lParam));

		case WM_PAINT:
			{
			PAINTSTRUCT ps;
			HDC hDC = ::BeginPaint(hWnd, &ps);

			::EndPaint(hWnd, &ps);
			return 0;
			}

		case WM_SIZE:
			return GetCtx(hWnd)->WMSize((int)LOWORD(lParam), (int)HIWORD(lParam), (int)wParam);

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
