
#ifndef INCL_CG16BITFONT
#define INCL_CG16BITFONT

#ifndef _SDL_TTF_H
struct _TTF_Font;
typedef struct _TTF_Font TTF_Font;
#endif

#include "CArray.h"
#include "CStructArray.h"

enum AlignmentStyles
	{
	alignLeft =				0x00000001,
	alignCentered =			0x00000002,
	alignRight =			0x00000004,
	alignTop =				0x00000008,
	alignMiddle =			0x00000010,
	alignBottom =			0x00000020,
	};

enum EffectTypes
	{
	effectShadow =			0x01,
	};

struct SEffectDesc
	{
	EffectTypes iType;
	COLORREF wColor1;
	COLORREF wColor2;
	};

class CG16bitFont : public CObject
	{
	public:
		enum Flags
			{
			SmartQuotes =	0x00000001,
			TruncateLine =	0x00000002,

			AlignCenter =	0x00000004,
			AlignRight =	0x00000008,

			MeasureOnly =	0x00000010,
			};

		CG16bitFont (void);
		~CG16bitFont();

		ALERROR Create (const CString &sTypeface, int iSize, bool bBold = false, bool bItalic = false, bool bUnderline = false);
		inline void Destroy (void);

		void BreakText (const CString &sText, int cxWidth, CStringArray *pLines, DWORD dwFlags = 0) const;
		void DrawText (CG16bitImage &Dest, int x, int y, COLORREF wColor, const CString &sText, DWORD dwFlags = 0, int *retx = NULL) const;
		void DrawText (CG16bitImage &Dest, 
					   const RECT &rcRect, 
					   COLORREF wColor, 
					   const CString &sText, 
					   int iLineAdj = 0, 
					   DWORD dwFlags = 0,
					   int *retcyHeight = NULL) const;
		void DrawTextEffect (CG16bitImage &Dest,
							 int x,
							 int y,
							 COLORREF wColor,
							 const CString &sText,
							 int iEffectsCount,
							 const SEffectDesc *pEffects,
							 DWORD dwFlags = 0,
							 int *retx = NULL) const;
		int GetAverageWidth (void) const;
		int GetHeight (void) const;
		int MeasureText (CString sText, int *retcyHeight = NULL) const;

	private:
		TTF_Font *m_Font;
	};


#endif

