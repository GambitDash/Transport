//	AGScreen.cpp
//
//	Implementation of AGScreen class

#include "SDL.h"

#include "portage.h"

#include "CRect.h"
#include "CG16bitImage.h"
#include "CG16bitFont.h"
#include "AGScreen.h"

AGScreen::AGScreen (void) : CObject(NULL)

//	AGScreen constructor

	{
	}

AGScreen::~AGScreen (void)
	{
	if (m_pMouseCapture)
		SDL_WM_GrabInput(SDL_GRAB_OFF);
	}

AGScreen::AGScreen (const RECT &rcRect) : CObject(NULL),
		m_rcRect(rcRect),
		m_pController(NULL),
		m_Areas(TRUE),
		m_pMouseCapture(NULL),
		m_pMouseOver(NULL)

//	AGScreen constructor

	{
	m_rcInvalid.left = 0;
	m_rcInvalid.top = 0;
	m_rcInvalid.right = RectWidth(rcRect);
	m_rcInvalid.bottom = RectHeight(rcRect);
	}

ALERROR AGScreen::AddArea (AGArea *pArea, const RECT &rcRect, DWORD dwTag)

//	AddArea
//
//	Add an area to the screen

	{
	ALERROR error;

	//	Initialize the area

	if ((error = pArea->Init(this, this, rcRect, dwTag)))
		return error;

	//	Add the area

	error = m_Areas.AppendObject(pArea, NULL);
	if (error)
		return error;

	//	Get the mouse position and fire mouse move, if appropriate

	POINT pt;
	GetMousePos(&pt);

	if (HitTest(pt) == pArea)
		SetMouseOver(pArea);

	return NOERROR;
	}

void AGScreen::DestroyArea (AGArea *pArea)

//	DestroyArea
//
//	Destroys an area

	{
	//	Clean up

	if (pArea == m_pMouseOver)
		m_pMouseOver = NULL;
	if (pArea == m_pMouseCapture)
		{
		SDL_WM_GrabInput(SDL_GRAB_OFF);
		m_pMouseCapture = NULL;
		}

	//	Destroy and invalidate

	RECT rcRect = pArea->GetRect();
	m_Areas.RemoveObject(GetAreaIndex(pArea));
	Invalidate(rcRect);
	}

AGArea *AGScreen::FindArea (DWORD dwTag)

//	FindArea
//
//	Finds area by tag. Returns NULL if do not find the area

	{
	for (int i = 0; i < GetAreaCount(); i++)
		{
		AGArea *pArea = GetArea(i);
		if (pArea->GetTag() == dwTag)
			return pArea;
		}

	return NULL;
	}

void AGScreen::FireMouseMove (const POINT &pt)

//	FireMouseMove
//
//	Fires MouseEnter, MouseMove, and MouseLeave events

	{
	if (m_pMouseCapture)
		{
		//	Check to see if we've entered the area

		if (::PtInRect(&m_pMouseCapture->GetRect(), pt))
			SetMouseOver(m_pMouseCapture);
		else
			SetMouseOver(NULL);

		m_pMouseCapture->MouseMove(pt.x, pt.y);
		}
	else
		{
		//	Are we still over the same area?

		if (m_pMouseOver && ::PtInRect(&m_pMouseOver->GetRect(), pt))
			m_pMouseOver->MouseMove(pt.x, pt.y);

		//	If not, find out what area we're over

		else
			{
			AGArea *pNewMouseOver = HitTest(pt);
			SetMouseOver(pNewMouseOver);

			if (m_pMouseOver)
				m_pMouseOver->MouseMove(pt.x, pt.y);
			}
		}
	}

void AGScreen::GetMousePos (POINT *retpt)

//	GetMousePos
//
//	Returns the current position of the mouse relative to the AGScreen

	{
	int x;
	int y;
	//	Get the current position of the mouse in display coordinates

	SDL_GetMouseState(&x, &y);
	retpt->x = x;
	retpt->y = y;

	//	Convert to local screen coordinates

	retpt->x -= m_rcRect.left;
	retpt->y -= m_rcRect.top;
	}

AGArea *AGScreen::HitTest (const POINT &pt)

//	HitTest
//
//	Returns the area at the given point (in local screen coords)

	{
	int i;

	for (i = 0; i < GetAreaCount(); i++)
		{
		AGArea *pArea = GetArea(i);
		RECT rcArea = pArea->GetRect();

		if (pArea->IsVisible() 
				&& pArea->WantsMouseOver() 
				&& ::PtInRect(&rcArea, pt))
			return pArea;
		}

	return NULL;
	}

void AGScreen::LButtonDoubleClick (int x, int y)

//	LButtonDoubleClick
//
//	Handle left button double click

	{
	int i;

	//	Convert to AGScreen coordinates

	POINT pt;
	pt.x = x - m_rcRect.left;
	pt.y = y - m_rcRect.top;

	//	Give it to the area under the pointer

	for (i = 0; i < GetAreaCount(); i++)
		{
		AGArea *pArea = GetArea(i);
		RECT rcArea = pArea->GetRect();

		if (pArea->IsVisible() 
				&& pArea->WantsMouseOver() 
				&& ::PtInRect(&rcArea, pt))
			{
			m_pMouseCapture = pArea;
			SDL_WM_GrabInput(SDL_GRAB_ON);

			pArea->LButtonDoubleClick(pt.x, pt.y);
			break;
			}
		}
	}

void AGScreen::LButtonDown (int x, int y)

//	LButtonDown
//
//	Handle left button down

	{
	int i;

	//	Convert to AGScreen coordinates

	POINT pt;
	pt.x = x - m_rcRect.left;
	pt.y = y - m_rcRect.top;

	//	Give it to the area under the pointer

	for (i = 0; i < GetAreaCount(); i++)
		{
		AGArea *pArea = GetArea(i);
		RECT rcArea = pArea->GetRect();

		if (pArea->IsVisible() 
				&& pArea->WantsMouseOver() 
				&& ::PtInRect(&rcArea, pt))
			{
			m_pMouseCapture = pArea;
			SDL_WM_GrabInput(SDL_GRAB_ON);

			pArea->LButtonDown(pt.x, pt.y);
			break;
			}
		}
	}

void AGScreen::LButtonUp (int x, int y)

//	LButtonUp
//
//	Handle left button up

	{
	if (m_pMouseCapture)
		{
		//	Convert to screen coordinates

		POINT pt;
		pt.x = x - m_rcRect.left;
		pt.y = y - m_rcRect.top;

		//	Remember these values before we call LButtonUp because
		//	we may not have a valid screen after that.

		AGArea *pMouseCapture = m_pMouseCapture;
		SDL_WM_GrabInput(SDL_GRAB_OFF); 
		m_pMouseCapture = NULL;

		pMouseCapture->LButtonUp(pt.x, pt.y);
		}
	}

void AGScreen::MouseMove (int x, int y)

//	MouseMove
//
//	Handle mouse move

	{
	//	Convert to screen coordinates

	POINT pt;
	pt.x = x - m_rcRect.left;
	pt.y = y - m_rcRect.top;

	FireMouseMove(pt);
	}

void AGScreen::OnAreaSetRect (void)

//	OnAreaSetRect
//
//	One of our areas has moved

	{
	}

void AGScreen::Paint (CG16bitImage &Dest)

//	Paint
//
//	Paint the whole screen.
//
//	Dest is the entire display area; its origin is 0,0.

	{
	if (!IsRectEmpty(&m_rcInvalid))
		{
		int i;

		//	Convert to Window coordinates

		RECT rcUpdate = m_rcInvalid;
		OffsetRect(&rcUpdate, m_rcRect.left, m_rcRect.top);

		//	Clip appropriately. Note that Dest is always in
		//	window coordinates.

		Dest.SetClipRect(rcUpdate);

		//	Blank the screen

		Dest.Fill(rcUpdate.left, rcUpdate.top, RectWidth(rcUpdate), RectHeight(rcUpdate), CG16bitImage::RGBValue(0,0,0));

		//	Let each area paint

		for (i = 0; i < GetAreaCount(); i++)
			{
			AGArea *pArea = GetArea(i);

			//	m_rcInvalid is in Screen coordinates, and so is the rect
			//	for each area. The intersection is the portion of the
			//	area's rect that is invalid.

			RECT rcIntersect;
			if (pArea->IsVisible()
					&& ::IntersectRect(&rcIntersect, &m_rcInvalid, &pArea->GetRect()))
				{
				//	Calculate the rect of the area relative to the Window

				RECT rcArea = pArea->GetRect();
				OffsetRect(&rcArea, m_rcRect.left, m_rcRect.top);

				//	Clip appropriately

				OffsetRect(&rcIntersect, m_rcRect.left, m_rcRect.top);
				Dest.SetClipRect(rcIntersect);

				//	Paint

				pArea->Paint(Dest, rcArea);
				}
			}

		//	Reset the invalid rect

		memset(&m_rcInvalid, 0, sizeof(m_rcInvalid));
		Dest.ResetClipRect();
		}
	}

void AGScreen::SetMouseOver (AGArea *pArea)

//	SetMouseOver
//
//	Sets m_pMouseOver variable

	{
	if (m_pMouseOver != pArea)
		{
		if (m_pMouseOver)
			m_pMouseOver->MouseLeave();

		if (pArea)
			pArea->MouseEnter();

		m_pMouseOver = pArea;
		}
	}

void AGScreen::Update (void)

//	Update
//
//	Update the screen

	{
	int i;

	for (i = 0; i < GetAreaCount(); i++)
		{
		AGArea *pArea = GetArea(i);
		pArea->Update();
		}
	}
