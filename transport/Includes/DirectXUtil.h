//	DirectXUtil.h
//
//	DirectX Utilities
//
//	We require:
//
//		windows.h
//		ddraw.h
//		Alchemy.h

#ifndef INCL_DIRECTXUTIL
#define INCL_DIRECTXUTIL

class CG16bitFont;
class CG16bitImage;

struct SPoint
	{
	int x;
	int y;
	};

#include "CG16bitSprite.h"
#include "CG16bitImage.h"
#include "CG16bitFont.h"
#include "CG16bitRegion.h"
#include "AGScreen.h"
#include "CGFrameArea.h"
#include "CGButtonArea.h"
#include "CGImageArea.h"
#include "CGTextArea.h"
#include "CSoundMgr.h"

inline void CG16bitImage::DrawText (int x, int y, const CG16bitFont &Font, WORD wColor, CString sText, DWORD dwFlags, int *retx)
	{
	Font.DrawText(*this, x, y, wColor, sText, dwFlags, retx);
	}
//	Animation Framework

#ifdef LATER
class ISceneSink
	{
	public:
		virtual void OnStart (CGDirector *pDirector) { }
		virtual void OnStop (void) { }

		virtual void OnAnimate (void) { }
		virtual void OnKeyDown (int iVirtKey, DWORD dwKeyData) { }
		virtual void OnKeyUp (int iVirtKey, DWORD dwKeyData) { }
		virtual void OnLButtonDblClick (int x, int y, DWORD dwFlags) { }
		virtual void OnLButtonDown (int x, int y, DWORD dwFlags) { }
		virtual void OnLButtonUp (int x, int y, DWORD dwFlags) { }
		virtual void OnMouseMove (int x, int y, DWORD dwFlags) { }
	};

class CGDirector
	{
	public:
		CDirector (void);
		~CDirector (void);

		void SetScene (IScene *pScene);

		void StartShow (void);
		void EndShow (void);

		void Animate (void);
		void KeyDown (int iVirtKey, DWORD dwKeyData);
		void KeyUp (int iVirtKey, DWORD dwKeyData);
		void LButtonDblClick (int x, int y, DWORD dwFlags);
		void LButtonDown (int x, int y, DWORD dwFlags);
		void LButtonUp (int x, int y, DWORD dwFlags);
		void MouseMove (int x, int y, DWORD dwFlags);

	private:
		IScene *m_pScene;
	};
#endif

extern AlphaArray5 g_Alpha5 [256];
extern AlphaArray6 g_Alpha6 [256];

#endif

