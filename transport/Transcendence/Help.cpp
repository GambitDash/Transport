//	Help.cpp
//
//	Show help screen

#include "PreComp.h"
#include "Transcendence.h"
#include "XMLUtil.h"

struct SHelpLine
	{
	char *szKey;
	char *szDesc;
	};

static SHelpLine g_szHelp[] =
	{
		{ "Arrows",		"Rotate and thrust" },
		{ "[Ctrl]",		"Fire primary weapon system" },
		{ "[Shift]",	"Fire missile launcher (if installed)" },
		{ "[S]",		"Ship's status (refuel, view cargo, jettison)" },
		{ "[M]",		"Show map of entire system" },
		{ "[A]",		"Engage autopilot" },
		{ "[D]",		"Dock with nearest station" },
		{ "[G]",		"Enter stargate" },
		{ "[U]",		"Use item" },
		{ "[.]",		"Stop the ship" },
		{ "[W]",		"Select primary weapon to use (if multiple)" },
		{ "[Tab]",		"Select missile to launch (if installed)" },
		{ "[T]",		"Select next target (if targeting installed)" },
		{ "[F]",		"Select next friend (if targeting installed)" },
		{ "[R]",		"Clear target (if targeting installed)" },
		{ "[I]",		"Invoke the powers of Domina" },
		{ "[Q]",		"Squadron orders (if leading a squadron)" },
		{ "[C]",		"Communications" },
		{ "[B]",		"Enable/disable devices" },
		{ "[P]",		"Pauses the game" },
		{ "[Esc]",		"Game menu (save, self-destruct)" }
	};

#define RGB_HELP_TEXT				CG16bitImage::RGBValue(231,255,227)
#define RGB_BAR_COLOR				CG16bitImage::RGBValue(0, 2, 10)
#define HELP_LINE_COUNT				(sizeof(g_szHelp) / sizeof(g_szHelp[0]))

void CTranscendenceWnd::PaintHelpScreen (void)

//	PaintHelpScreen
//
//	Paints the help screen

	{
	RECT rcImage;
	rcImage.left = (g_cxScreen - m_HelpImage.GetWidth()) / 2;
	rcImage.top = (g_cyScreen - m_HelpImage.GetHeight()) / 2;
	rcImage.right = rcImage.left + m_HelpImage.GetWidth();
	rcImage.bottom = rcImage.right + m_HelpImage.GetHeight();

	//	Paint the parts that don't change

	if (m_bHelpInvalid)
		{
		//	Paint bars across top and bottom

		m_Screen.Fill(0, 
				0, 
				g_cxScreen, 
				g_cyScreen,
				RGB_BAR_COLOR);

		//	Paint image

		m_Screen.Blt(0,
				0,
				m_HelpImage.GetWidth(),
				m_HelpImage.GetHeight(),
				m_HelpImage,
				rcImage.left,
				rcImage.top);

		//	Paint help

		int x = rcImage.left + (4 * RectWidth(rcImage) / 9);
		int y = rcImage.top + 2 * m_Fonts.LargeBold.GetHeight();

		for (unsigned int i = 0; i < HELP_LINE_COUNT; i++)
			{
			m_Screen.DrawText(x, y, m_Fonts.LargeBold, RGB_HELP_TEXT, CString(g_szHelp[i].szKey));
			m_Screen.DrawText(x + 4 * m_Fonts.LargeBold.GetHeight(), y,
					m_Fonts.LargeBold, RGB_HELP_TEXT,
					CString(g_szHelp[i].szDesc));

			y += m_Fonts.LargeBold.GetHeight();
			}

		m_bHelpInvalid = false;
		}
	}

ALERROR CTranscendenceWnd::StartHelp (void)

//	StartHelp
//
//	Show the help screen

	{
	//	Load a JPEG of the epilog screen
/* XXX Need to implement this help screen loading!
	ALERROR error;

	HBITMAP hDIB;
	if (error = JPEGLoadFromResource(g_hInst,
			MAKEINTRESOURCE(IDR_HELP_BACKGROUND),
			JPEG_LFR_DIB, 
			NULL, 
			&hDIB))
		return error;

	error = m_HelpImage.CreateFromBitmap(hDIB, NULL);
	::DeleteObject(hDIB);
	if (error)
		return error;

	//	Done

	m_OldState = m_State;
	m_State = gsHelp;
	m_bHelpInvalid = true;
*/
	return NOERROR;
	}

void CTranscendenceWnd::StopHelp (void)

//	StopHelp
//
//	Clean up the help screen

	{
	ASSERT(m_State == gsHelp);

	m_HelpImage.Destroy();
	m_State = m_OldState;
	}

