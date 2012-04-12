//	MainWndProc.cpp
//
//	Implementation of main window proc

#include "PreComp.h"
#include "Transcendence.h"
#include "XMLUtil.h"

#include "SDL.h"

static int ConvertKey(SDLKey k)
	{
	int key = SDLK_UNKNOWN;

	/* Convert numpad characters. */
	if (k >= SDLK_KP0 && k <= SDLK_KP9)
		{
		key = (k - SDLK_KP0 + SDLK_0);
		}
	else if (k >= SDLK_KP_PERIOD && k <= SDLK_KP_EQUALS)
		{
		switch(k) {
			case SDLK_KP_PERIOD:		key = SDLK_PERIOD; break;
			case SDLK_KP_DIVIDE:		key = SDLK_SLASH; break;
			case SDLK_KP_MULTIPLY:	key = SDLK_ASTERISK; break;
			case SDLK_KP_MINUS:		key = SDLK_MINUS; break;
			case SDLK_KP_PLUS:		key = SDLK_PLUS; break;
			case SDLK_KP_EQUALS:		key = SDLK_EQUALS; break;
			case SDLK_KP_ENTER:		key = SDLK_RETURN; break;
			default:						key = (k); break;
			}
		}
	else if (k < 255)
		{
		key = k;
		//key = toupper(k);
		}

	return key;
	}

bool *g_kbmap;

ALERROR MainWndProc(CTranscendenceWnd *wnd, void *e)

//	MainWndProc
//
//	WndProc for main window

	{
	SDL_Event *evt = (SDL_Event *)e;
	switch (evt->type)
		{
		case SDL_ACTIVEEVENT:
			return wnd->WMActivateApp(evt->active.gain ? true : false);

		case SDL_QUIT:
			return wnd->WMClose();

		case SDL_VIDEORESIZE:
			return wnd->WMSize(evt->resize.w, evt->resize.h, 0);

		case SDL_VIDEOEXPOSE:
			// Possibly a repaint event.
			break;

		case SDL_KEYDOWN:
			g_kbmap[evt->key.keysym.sym] = true;
			if (((evt->key.keysym.mod & KMOD_SHIFT) || (evt->key.keysym.mod & KMOD_CAPS))
					&& isascii(evt->key.keysym.sym))
				{
				wnd->WMKeyDown(toupper(evt->key.keysym.sym), 0);
				if (evt->key.keysym.sym < 255) wnd->WMChar(toupper(evt->key.keysym.sym), 0);
				}
			else
				{
				int key = ConvertKey(evt->key.keysym.sym);
				if (key != SDLK_UNKNOWN)
					{
					wnd->WMKeyDown(key, 0);
					wnd->WMChar(key, 0);
					}
				else
					wnd->WMKeyDown(evt->key.keysym.sym, 0);
				}
			break;

		case SDL_KEYUP:
			g_kbmap[evt->key.keysym.sym] = false;
			return wnd->WMKeyUp(evt->key.keysym.sym, 0);

		case SDL_MOUSEBUTTONDOWN:
			if (evt->button.button == SDL_BUTTON_LEFT)
				return wnd->WMLButtonDown(evt->button.x, evt->button.y, 0);
			return 0;

		case SDL_MOUSEBUTTONUP:
			if (evt->button.button == SDL_BUTTON_LEFT)
				return wnd->WMLButtonUp(evt->button.x, evt->button.y, 0);
			return 0;

		case SDL_MOUSEMOTION:
			return wnd->WMMouseMove(evt->motion.x, evt->motion.y, 0);

		default:
			;
		}
	return 0;
	}
