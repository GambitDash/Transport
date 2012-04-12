#ifndef INCL_CGBUTTONAREA
#define INCL_CGBUTTONAREA

class CGButtonArea : public AGArea
	{
	public:
		CGButtonArea (void);

      inline CString GetLabelAccelerator (void) { return (m_iAccelerator != -1 ? CString(m_sLabel.GetASCIIZPointer() + m_iAccelerator, 1) : NULL_STR); }
		inline bool IsDisabled (void) { return m_bDisabled; }
		inline void SetDisabled (bool bDisabled = true) { m_bDisabled = bDisabled; }
		inline void SetLabel (const CString &sText) { m_sLabel = sText; m_iAccelerator = -1; }
		void SetLabelAccelerator (const CString &sKey);
		inline void SetLabelColor (WORD wColor) { m_wLabelColor = wColor; }
		inline void SetLabelFont (const CG16bitFont *pFont) { m_pLabelFont = pFont; }

		//	AGArea virtuals
		virtual void LButtonUp (int x, int y);
		virtual void MouseEnter (void);
		virtual void MouseLeave (void);
		virtual void Paint (CG16bitImage &Dest, const RECT &rcRect);

   protected:
      // AGArea virtuals
      virtual void OnSetRect (void);

	private:
		CString m_sLabel;
		WORD m_wLabelColor;
		const CG16bitFont *m_pLabelFont;
		int m_iAccelerator;

		bool m_bMouseOver;
		bool m_bDisabled;
	};

#endif

