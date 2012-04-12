//	CMessageDisplay.cpp
//
//	CMessageDisplay class

#include "PreComp.h"
#include "Transcendence.h"

CMessageDisplay::CMessageDisplay (void) : CObject(NULL),
		m_pFont(NULL),
		m_iBlinkTime(0),
		m_iSteadyTime(0),
		m_iFadeTime(0),
		m_iFirstMessage(0),
		m_iNextMessage(0),
		m_cySmoothScroll(0)

//	CMessageDisplay constructor

	{
	memset(&m_rcRect, 0, sizeof(m_rcRect));
	}

void CMessageDisplay::ClearAll (void)

//	ClearAll
//
//	Clear all messages

	{
	m_iFirstMessage = 0;
	m_iNextMessage = 0;
	m_cySmoothScroll = 0;
	}

void CMessageDisplay::DisplayMessage (CString sMessage, COLORREF wColor)

//	DisplayMessage
//
//	Display a new message in the given color

	{
	//	Don't accept duplicate messages

	if (m_iNextMessage != m_iFirstMessage)
		{
		int iMsg = Prev(m_iNextMessage);
		if (m_Messages[iMsg].iState != stateClear
				&& strEquals(m_Messages[iMsg].sMessage, sMessage))
			return;
		}

	//	If we're at the end, delete the last message

	if (Next(m_iNextMessage) == m_iFirstMessage)
		m_iFirstMessage = Next(m_iFirstMessage);

	//	If there are already messages here, then smooth scroll

	if (m_pFont)
		{
		int iCount = 0;
		int iMsg = Prev(m_iNextMessage);
		while (Next(iMsg) != m_iFirstMessage)
			{
			if (m_Messages[iMsg].iState != stateClear)
				iCount++;
			iMsg = Prev(iMsg);
			}

		if (iCount > 0)
			m_cySmoothScroll = -m_pFont->GetHeight();
		}

	//	Remember the message

	m_Messages[m_iNextMessage].sMessage = sMessage;
	m_Messages[m_iNextMessage].wColor = wColor;
	m_Messages[m_iNextMessage].x = -1;

	//	Start at the appropriate state

	if (m_iBlinkTime > 0)
		{
		m_Messages[m_iNextMessage].iState = stateBlinking;
		m_Messages[m_iNextMessage].iTick = m_iBlinkTime;
		}
	else if (m_iSteadyTime > 0)
		{
		m_Messages[m_iNextMessage].iState = stateSteady;
		m_Messages[m_iNextMessage].iTick = m_iSteadyTime;
		}
	else if (m_iFadeTime > 0)
		{
		m_Messages[m_iNextMessage].iState = stateFading;
		m_Messages[m_iNextMessage].iTick = m_iFadeTime;
		}
	else
		{
		m_Messages[m_iNextMessage].iState = stateNormal;
		m_Messages[m_iNextMessage].iTick = 0;
		}

	//	Point to next message

	m_iNextMessage = Next(m_iNextMessage);
	}

void CMessageDisplay::Paint (CG16bitImage &Dest)

//	Paint
//
//	Paint the message

	{
	//	Can't do anything without a fond

	if (m_pFont == NULL)
		return;

	bool bSkip = false;
	if (m_cySmoothScroll != 0)
		bSkip = true;

	//	Loop over all messages in the queue. We go backwards because
	//	the messages are shown in reverse chronological order

	int y = m_rcRect.top;
	int iMsg = Prev(m_iNextMessage);
	while (Next(iMsg) != m_iFirstMessage)
		{
		if (m_Messages[iMsg].iState != stateClear)
			{
			//	Figure out what color to use

			COLORREF wColor = CGImage::RGBValue(255, 255, 255);

			switch (m_Messages[iMsg].iState)
				{
				case stateNormal:
				case stateSteady:
					wColor = m_Messages[iMsg].wColor;
					break;

				case stateBlinking:
					{
					if ((m_Messages[iMsg].iTick % 2) == 0)
						wColor = CG16bitImage::RGBValue(255,255,255);
					else
						wColor = CG16bitImage::RGBValue(0,0,0);
					break;
					}

				case stateFading:
					{
					int iFade = 1000 * m_Messages[iMsg].iTick / m_iFadeTime;
					wColor = CG16bitImage::RGBValue(
							iFade * CG16bitImage::RedValue(m_Messages[iMsg].wColor) / 1000,
							iFade * CG16bitImage::GreenValue(m_Messages[iMsg].wColor) / 1000,
							iFade * CG16bitImage::BlueValue(m_Messages[iMsg].wColor) / 1000);
					break;
					}
				case stateClear:
					{
					assert(0);
					}
				}

			//	If we haven't yet figured out the position of the message
			//	calculate it now

			if (m_Messages[iMsg].x == -1)
				{
				int cxWidth = m_pFont->MeasureText(m_Messages[iMsg].sMessage, NULL);
				m_Messages[iMsg].x = m_rcRect.left + (RectWidth(m_rcRect) - cxWidth) / 2;
				}

			y += m_pFont->GetHeight();

			//	Draw the message centered

			if (!bSkip)
				{
				Dest.DrawText(m_Messages[iMsg].x,
						y + m_cySmoothScroll, 
						*m_pFont, 
						wColor, 
						m_Messages[iMsg].sMessage);

				if (m_cySmoothScroll != 0)
					m_cySmoothScroll = Min(0, m_cySmoothScroll + 4);
				}
			else
				bSkip = false;
			}

		iMsg = Prev(iMsg);
		}
	}

void CMessageDisplay::Update (void)

//	Update
//
//	Update the display

	{
	int iMsg = Prev(m_iNextMessage);
	while (Next(iMsg) != m_iFirstMessage)
		{
		if (m_Messages[iMsg].iTick > 0)
			{
			if (--(m_Messages[iMsg].iTick) == 0)
				{
				switch (m_Messages[iMsg].iState)
					{
					case stateBlinking:
						if (m_iSteadyTime > 0)
							{
							m_Messages[iMsg].iState = stateSteady;
							m_Messages[iMsg].iTick = m_iSteadyTime;
							}
						else
							m_Messages[iMsg].iState = stateClear;
						break;

					case stateSteady:
						if (m_iFadeTime > 0)
							{
							m_Messages[iMsg].iState = stateFading;
							m_Messages[iMsg].iTick = m_iFadeTime;
							}
						else
							m_Messages[iMsg].iState = stateClear;
						break;

					case stateFading:
						m_Messages[iMsg].iState = stateClear;
						break;

					case stateNormal:
					case stateClear:
						assert(0);
						break;
					}
				}
			}

		iMsg = Prev(iMsg);
		}
	}
