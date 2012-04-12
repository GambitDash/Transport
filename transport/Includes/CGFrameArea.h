#ifndef INCL_CGFRAMEAREA
#define INCL_CGFRAMEAREA

class CGFrameArea : public AGArea, public IAreaContainer
	{
	public:
		CGFrameArea (void);

		ALERROR AddArea (AGArea *pArea, const RECT &rcRect, DWORD dwTag);
		AGArea *FindArea (DWORD dwTag);
		inline AGArea *GetArea (int iIndex) { return (AGArea *)m_Areas.GetObject(iIndex); }
		inline int GetAreaCount (void) { return m_Areas.GetCount(); }

		//	AGArea virtuals
		virtual bool LButtonDoubleClick (int x, int y);
		virtual bool LButtonDown (int x, int y);
		virtual void LButtonUp (int x, int y);
		virtual void MouseMove (int x, int y);
		virtual void Paint (CG16bitImage &Dest, const RECT &rcRect);

      // IAreaContainer virtuals
      virtual void OnAreaSetRect (void);

	private:
		RECT m_rcInvalid;						//	Invalid rect relative to m_rcRect

		CObjectArray m_Areas;					//	Array of areas

		AGArea *m_pMouseCapture;				//	Area that has captured the mouse
		AGArea *m_pMouseOver;					//	Area that the mouse is currently over
	};


#endif

