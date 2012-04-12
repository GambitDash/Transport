#ifndef INCL_CRECT
#define INCL_CRECT

inline int RectHeight(const RECT *pRect) { return pRect->bottom - pRect->top; }
inline int RectHeight(const RECT &Rect) { return Rect.bottom - Rect.top; }
inline int RectWidth(const RECT *pRect) { return pRect->right - pRect->left; }
inline int RectWidth(const RECT &Rect) { return Rect.right - Rect.left; }
inline BOOL RectsIntersect(const RECT *R1, const RECT *R2)
	{
	return (R1->right > R2->left)
			&& (R1->left < R2->right)
			&& (R1->bottom > R2->top)
			&& (R1->top < R2->bottom);
	}
inline BOOL RectEncloses (const RECT *pR1, const RECT *pR2)
	{
	return (pR1->left <= pR2->left)
			&& (pR1->right >= pR2->right)
			&& (pR1->top <= pR2->top)
			&& (pR1->bottom >= pR2->bottom);
	}
inline bool RectEquals (const RECT &rc1, const RECT &rc2)
	{
	return (rc1.left == rc2.left
			&& rc1.top == rc2.top
			&& rc1.right == rc2.right
			&& rc1.bottom == rc2.bottom);
	}
inline void RectInit (RECT *pRect)
	{
	pRect->left = 0;
	pRect->top = 0;
	pRect->right = 0;
	pRect->bottom = 0;
	}
inline bool CheckRect(const RECT *r)
	{
	return (r->left <= r->right && r->top <= r->bottom);
	}
inline bool UnionRect(RECT *dest, const RECT *r1, const RECT *r2)
	{
	if (!CheckRect(r1))
		{
		*dest = *r2;
		return true;
		}
	if (!CheckRect(r2))
		{
		*dest = *r1;
		return true;
		}
	dest->top = std::min(r1->top, r2->top);
	dest->bottom = std::max(r1->bottom, r2->bottom);
	dest->left = std::min(r1->left, r2->left);
	dest->right = std::max(r1->right, r2->right);
	return true;
	}
inline bool PtInRect(const RECT *r, const POINT &pt)
	{
	if (!CheckRect(r))
		return false;

	return pt.x >= r->left &&
			 pt.x <= r->right &&
			 pt.y >= r->top &&
			 pt.y <= r->bottom;
	}
inline bool IsRectEmpty(const RECT *r) { return r == NULL || (r->top == r->bottom && r->right == r->left); }
inline void OffsetRect(RECT *r, DWORD dx, DWORD dy)
	{
	r->left += dx;
	r->right += dx;
	r->top += dy;
	r->bottom += dy;
	}

inline bool IntersectRect(RECT *dest, const RECT *r1, const RECT *r2)
	{
	if (!CheckRect(r1)) {
		*dest = *r2;
		return true;
	}
	if (!CheckRect(r2)) {
		*dest = *r1;
		return true;
	}

	if(RectsIntersect(r1, r2))
		{
      dest->left = std::max(r1->left, r2->left),
      dest->top = std::max(r1->top, r2->top),
      dest->right = std::min( r1->right, r2->right),
      dest->bottom = std::min(r1->bottom, r2->bottom);
		return true;
		}
	else
		{
		return false;
		}
	}

#endif

