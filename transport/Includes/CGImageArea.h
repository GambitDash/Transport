#ifndef INCL_CGIMAGEAREA
#define INCL_CGIMAGEAREA

class CGImageArea : public AGArea
	{
	public:
		CGImageArea (void);

		inline void SetBackColor (COLORREF wColor) { m_rgbBackColor = wColor; }
		inline void SetImage (CG16bitImage *pImage, const RECT &rcImage) { m_pImage = pImage; m_rcImage = rcImage; Invalidate(); }
		inline void SetStyles (DWORD dwStyles) { m_dwStyles = dwStyles; Invalidate(); }
		inline void SetTransBackground (bool bTrans = true) { m_bTransBackground = bTrans; }

		//	AGArea virtuals
		virtual void Paint (CG16bitImage &Dest, const RECT &rcRect);
		virtual bool WantsMouseOver (void) { return false; }

	private:
		CG16bitImage *m_pImage;
		RECT m_rcImage;

		COLORREF m_rgbBackColor;
		bool m_bTransBackground;
		DWORD m_dwStyles;
	};

#endif

