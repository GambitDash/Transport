#ifndef INCL_CGTEXTAREA
#define INCL_CGTEXTAREA

class CGTextArea : public AGArea
	{
	public:
		CGTextArea (void);

		inline void SetColor (WORD wColor) { m_Color = wColor; }
		inline void SetEditable (bool bEditable = true) { m_bEditable = bEditable; }
		inline void SetFont (const CG16bitFont *pFont) { m_pFont = pFont; m_cxJustifyWidth = 0; }
		inline void SetLineSpacing (int cySpacing) { m_cyLineSpacing = cySpacing; m_cxJustifyWidth = 0; }
		inline void SetStyles (DWORD dwStyles) { m_dwStyles = dwStyles; m_cxJustifyWidth = 0; }
		inline void SetText (const CString &sText) { m_sText = sText; m_cxJustifyWidth = 0; Invalidate(); }
		inline const CString &GetText (void) { return m_sText; }

		//	AGArea virtuals
		virtual int Justify (const RECT &rcRect);
		virtual void Paint (CG16bitImage &Dest, const RECT &rcRect);
		virtual void Update (void) { m_iTick++; }

	private:
		CString m_sText;						//	Text text to draw

		bool m_bEditable;						//	TRUE if editable
		DWORD m_dwStyles;						//	AlignmentStyles
		int m_cyLineSpacing;					//	Extra spacing between lines
		const CG16bitFont *m_pFont;
		WORD m_Color;

		CStringArray m_Lines;					//	Justified lines of text
		int m_cxJustifyWidth;					//	Width (in pixels) for which m_Lines
												//		was justified.

		int m_iTick;							//	Cursor tick
		int m_iCursorPos;						//	Cursor position (-1 = no cursor)
	};

#endif

