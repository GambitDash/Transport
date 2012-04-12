#ifndef INCL_AGSCREEN
#define INCL_AGSCREEN

#include "CObjectArray.h"

//	Screen Elements

class AGArea;
class AGScreen;
class SEffectDesc;

class IScreenController
	{
	public:
		virtual void Action (DWORD dwTag, DWORD dwData = 0) = 0;
		virtual ~IScreenController() { }
	};

class IAreaContainer
	{
	public:
		virtual ~IAreaContainer() { }
		virtual void GetMousePos (POINT *retpt) { }
		virtual void OnAreaSetRect (void) { }
	};

class AGArea : public CObject
	{
	public:
		AGArea (void);
		virtual ~AGArea (void);

		void AddEffect (const SEffectDesc &Effect);
		void AddShadowEffect (void);
		inline IAreaContainer *GetParent (void) { return m_pParent; }
		inline RECT &GetRect (void) { return m_rcRect; }
		inline AGScreen *GetScreen (void) { return m_pScreen; }
		inline DWORD GetTag (void) { return m_dwTag; }
		inline void Hide (void) { ShowHide(false); }
		ALERROR Init (AGScreen *pScreen, IAreaContainer *pParent, const RECT &rcRect, DWORD dwTag);
		inline void Invalidate (void) { Invalidate(m_rcRect); }
		inline void Invalidate (const RECT &rcInvalid);
		inline bool IsVisible (void) { return m_bVisible; }
		inline void SetController (IScreenController *pController) { m_pController = pController; }
		void SetRect (const RECT &rcRect);
		void SignalAction (DWORD dwData = 0);
		inline void Show (void) { ShowHide(true); }
		void ShowHide (bool bShow);

		//	AGArea virtuals
		virtual int Justify (const RECT &rcRect) { return 0; }
		virtual bool LButtonDoubleClick (int x, int y) { return false; }
		virtual bool LButtonDown (int x, int y) { return false; }
		virtual void LButtonUp (int x, int y) { }
		virtual void MouseEnter (void) { }
		virtual void MouseLeave (void) { }
		virtual void MouseMove (int x, int y) { }
		virtual void Paint (CG16bitImage &Dest, const RECT &rcRect) { }
		virtual void Update (void) { }
		virtual bool WantsMouseOver (void) { return true; }

	protected:
		inline const SEffectDesc &GetEffect (int iIndex) { return m_pEffects[iIndex]; }
		inline int GetEffectCount (void) { return m_iEffectCount; }
		inline const SEffectDesc *GetEffects (void) { return m_pEffects; }
		inline bool HasEffects (void) { return (m_pEffects != NULL); }
		virtual void OnSetRect (void) { }
		virtual void OnShowHide (bool bShow) { }

	private:
		IAreaContainer *m_pParent;
		AGScreen *m_pScreen;
		RECT m_rcRect;
		DWORD m_dwTag;
		IScreenController *m_pController;

		int m_iEffectCount;
		SEffectDesc *m_pEffects;

		bool m_bVisible;						//	TRUE if visible

	friend class CObjectClass<AGArea>;
	};

class AGScreen : public CObject, public IAreaContainer
	{
	public:
		AGScreen (const RECT &rcRect);
		virtual ~AGScreen (void);

		ALERROR AddArea (AGArea *pArea, const RECT &rcRect, DWORD dwTag);
		void DestroyArea (AGArea *pArea);
		AGArea *FindArea (DWORD dwTag);
		inline AGArea *GetArea (int iIndex) { return (AGArea *)m_Areas.GetObject(iIndex); }
		inline int GetAreaCount (void) { return m_Areas.GetCount(); }
		inline const RECT &GetRect (void) { return m_rcRect; }
		inline IScreenController *GetController (void) { return m_pController; }
		inline void Invalidate (const RECT &rcInvalid) { UnionRect(&m_rcInvalid, &rcInvalid, &m_rcInvalid); }
		inline void SetController (IScreenController *pController) { m_pController = pController; }

		//	IAreaContainer virtuals
		virtual void GetMousePos (POINT *retpt);
		virtual void OnAreaSetRect (void);

		//	These methods are called by the HWND
		void LButtonDoubleClick (int x, int y);
		void LButtonDown (int x, int y);
		void LButtonUp (int x, int y);
		void MouseMove (int x, int y);
		void Paint (CG16bitImage &Dest);
		void Update (void);

	private:
		AGScreen (void);
		void FireMouseMove (const POINT &pt);
		inline int GetAreaIndex (AGArea *pArea) { return m_Areas.FindObject(pArea); }
      AGArea *HitTest (const POINT &pt);
		void SetMouseOver (AGArea *pArea);

		RECT m_rcRect;							//	Rect of screen relative to Window
		RECT m_rcInvalid;						//	Invalid rect relative to m_rcRect
		IScreenController *m_pController;		//	Screen controller

		CObjectArray m_Areas;

		AGArea *m_pMouseCapture;				//	Area that has captured the mouse
		AGArea *m_pMouseOver;					//	Area that the mouse is currently over

	friend class CObjectClass<AGScreen>;
	};

inline void AGArea::Invalidate (const RECT &rcInvalid) { if (m_pScreen) m_pScreen->Invalidate(rcInvalid); }

#endif

