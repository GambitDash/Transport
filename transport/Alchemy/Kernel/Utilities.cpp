//	Utilities.cpp
//
//	Miscellaneous utility functions

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"

#include "Kernel.h"

CString sysGetUserName (void)

//	sysGetUserName
//
//	Returns the name of the currently logged-on user

	{
	return getenv("USERNAME");
	}

void uiGetCenteredWindowRect (int cxWidth, 
							  int cyHeight, 
							  RECT *retrcRect,
							  bool bClip)

//	uiGetCenteredWindowRect
//
//	Returns the RECT of a window of the given dimensions centered on the
//	workarea of the primary screen (i.e., area not hidden by taskbar, etc.)

	{
		retrcRect->left = 0;
		retrcRect->top = 0;
		retrcRect->right = 0;
		retrcRect->bottom = 0;
	}

