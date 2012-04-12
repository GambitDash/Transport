#ifndef INCL_CUI
#define INCL_CUI

//	UI functions
extern bool *g_kbmap;

void uiGetCenteredWindowRect (int cxWidth, 
							  int cyHeight, 
							  RECT *retrcRect,
							  bool bClip = true);

inline bool uiIsKeyDown (int iVirtKey)	{ return g_kbmap[iVirtKey]; }
inline BOOL IsShiftDown(void) { return uiIsKeyDown(SDLK_LSHIFT) || uiIsKeyDown(SDLK_RSHIFT); }
inline BOOL IsControlDown(void) { return uiIsKeyDown(SDLK_LCTRL) || uiIsKeyDown(SDLK_RCTRL); }

#endif

