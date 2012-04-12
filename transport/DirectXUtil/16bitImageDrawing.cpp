//	16bitImageDrawing.cpp
//
//	Drawing routines such as lines and text

#include <math.h>
#include <stdio.h>

#include "portage.h"
#include "CMath.h"

#include "CG16bitImage.h"

#define SMALL_SQUARE_SIZE					2
#define MEDIUM_SQUARE_SIZE					4

#define WU_ERROR_BITS						16
#define WU_INTENSITY_SHIFT					(WU_ERROR_BITS - ALPHA_BITS)
#define WU_WEIGHTING_COMPLEMENT_MASK		(ALPHA_LEVELS - 1)

const BYTE g_wSmallRoundMask[9] = 
	{
	100, 255, 100,
	255, 255, 255,
	100, 255, 100,
	};

void CG16bitImage::BresenhamLine (int x1, int y1, 
								  int x2, int y2,
								  WORD wColor)

//	BresenhamLine
//
//	Draws a line using Bresenham's algorithm

	{
	int dx = x2 - x1;
	int ax = Absolute(dx) * 2;
	int sx = (dx > 0 ? 1 : -1);

	int dy = y2 - y1;
	int ay = Absolute(dy) * 2;
	int sy = (dy > 0 ? 1 : -1);

	int x = x1;
	int y = y1;
	int d;

	if (ax > ay)		//	x dominant
		{
		d = ay - ax / 2;
		while (true)
			{
			DrawPixel(x, y, wColor);

			if (x == x2)
				return;
			else if (d >= 0)
				{
				y = y + sy;
				d = d - ax;
				}

			x = x + sx;
			d = d + ay;
			}
		}
	else				//	y dominant
		{
		d = ax - ay / 2;
		while (true)
			{
			DrawPixel(x, y, wColor);

			if (y == y2)
				return;
			else if (d >= 0)
				{
				x = x + sx;
				d = d - ay;
				}

			y = y + sy;
			d = d + ax;
			}
		}
	}

void CG16bitImage::BresenhamLineAA (int x1, int y1, 
									int x2, int y2,
									int iWidth,
									WORD wColor)

//	BresenhamLineAA
//
//	Anti-aliased version of Bresenham's algorithm

	{
	//	Calculate the slope

	int dx = x2 - x1;
	int ax = Absolute(dx) * 2;
	int sx = (dx > 0 ? 1 : -1);

	int dy = y2 - y1;
	int ay = Absolute(dy) * 2;
	int sy = (dy > 0 ? 1 : -1);

	//	Handle special cases

	if (dx == 0 && dy == 0)
		{
		DrawPixel(x1, y1, wColor);
		return;
		}

	//	Do it

	int i, x, y;

	if (ax > ay)		//	x-axis dominant
		{
		//	Compute the distance from the mid-point of the line to
		//	the top edge along the y-axis.

		double rHalfHeight = (double)iWidth * sqrt((double)(dx*dx + dy*dy)) / (double)(2 * abs(dx));
		int cyHalfCount = (int)(((2 * rHalfHeight) - 1) / 2);
		int cyCount = 2 * cyHalfCount + 1;

		int d = ay - ax / 2;

		//	Compute information for anti-aliasing. rDist is the distance
		//	from the top solid point to the edge of the line. We update
		//	the value as we proceed along the x-axis and use it for
		//	anti-aliasing the edge of the line. A negative value means
		//	that the top point is outside the edge.

		double rSlope = (double)dy / (double)dx;
		double rDistTop = rHalfHeight - (double)cyHalfCount - 0.25;
		double rDistBottom = rDistTop;

		double rDistTopInc = sy;
		double rDistTopDec = rSlope * sx;
		double rDistBottomInc = rSlope * sx;
		double rDistBottomDec = sy;

		//	Draw!

		for (x = x1, y = y1; x != x2; x += sx)
			{
			int yTop = y - cyHalfCount;

			//	Draw anti-aliasing above the line

			if (rDistTop > 0)
				DrawPixelTrans(x, yTop - 1, wColor, (BYTE)(255.0 * (rDistTop > 1.0 ? 1.0 : rDistTop)));

			//	Draw the solid part of the line

			if (x >= 0 && x < m_cxWidth)
				{
				WORD *pPos = GetPixel(GetRowStart(yTop), x);

				for (i = yTop; i < yTop + cyCount; i++)
					{
					if (i >= 0 && i < m_cyHeight)
						*pPos = wColor;

					pPos = NextRow(pPos);
					}
				}

			//	Draw anti-aliasing below the line

			if (rDistBottom > 0)
				DrawPixelTrans(x, yTop + cyCount, wColor, (BYTE)(255.0 * (rDistBottom > 1.0 ? 1.0 : rDistBottom)));

			//	Next point

			if (d >= 0)
				{
				y = y + sy;
				d = d - ax;
				rDistTop += rDistTopInc;
				rDistBottom -= rDistBottomDec;
				}

			d = d + ay;
			rDistTop -= rDistTopDec;
			rDistBottom += rDistBottomInc;
			}
		}
	else				//	y-axis dominant
		{
		//	Compute the distance from the mid-point of the line to
		//	the left-edge of the x-axis

		double rHalfWidth = (double)iWidth * sqrt((double)(dx*dx + dy*dy)) / (double)(2 * abs(dy));
		int cxHalfCount = (int)(((2 * rHalfWidth) - 1) / 2);
		int cxCount = 2 * cxHalfCount + 1;

		int d = ax - ay / 2;

		//	Compute information for anti-aliasing. rDist is the distance
		//	from the top solid point to the edge of the line. We update
		//	the value as we proceed along the x-axis and use it for
		//	anti-aliasing the edge of the line. A negative value means
		//	that the top point is outside the edge.

		double rSlope = (double)dx / (double)dy;
		double rDistLeft = rHalfWidth - (double)cxHalfCount - 0.25;
		double rDistRight = rDistLeft;

		double rDistLeftInc = sx;
		double rDistRightDec = sx;
		double rDistLeftDec = rSlope * sy;
		double rDistRightInc = rSlope * sy;

		//	Draw!

		for (y = y1, x = x1; y != y2; y += sy)
			{
			int xTop = x - cxHalfCount;

			//	Draw anti-aliasing to the left

			if (rDistLeft > 0)
				DrawPixelTrans(xTop - 1, y, wColor, (BYTE)(255.0 * (rDistLeft > 1.0 ? 1.0 : rDistLeft)));

			//	Draw the solid part of the line

			if (y >= 0 && y < m_cyHeight)
				{
				WORD *pPos = GetPixel(GetRowStart(y), xTop);

				for (i = xTop; i < xTop + cxCount; i++)
					if (i >= 0 && i < m_cxWidth)
						*pPos++ = wColor;
					else
						pPos++;
				}

			//	Draw anti-aliasing to the right of the line

			if (rDistRight > 0)
				DrawPixelTrans(xTop + cxCount, y, wColor, (BYTE)(255.0 * (rDistRight > 1.0 ? 1.0 : rDistRight)));

			//	Next point

			if (d >= 0)
				{
				x = x + sx;
				d = d - ay;
				rDistLeft += rDistLeftInc;
				rDistRight -= rDistRightDec;
				}

			d = d + ax;
			rDistLeft -= rDistLeftDec;
			rDistRight += rDistRightInc;
			}
		}
	}

void CG16bitImage::BresenhamLineAAFade (int x1, int y1, 
									int x2, int y2,
									int iWidth,
									WORD wColor1,
									WORD wColor2)

//	BresenhamLineAA
//
//	Anti-aliased version of Bresenham's algorithm

	{
	//	Calculate the slope

	int dx = x2 - x1;
	int ax = Absolute(dx) * 2;
	int sx = (dx > 0 ? 1 : -1);

	int dy = y2 - y1;
	int ay = Absolute(dy) * 2;
	int sy = (dy > 0 ? 1 : -1);

	//	Handle special cases

	if (dx == 0 && dy == 0)
		{
		DrawPixel(x1, y1, wColor1);
		return;
		}

	//	Compute color fading scale

	int iRedStart = RedValue(wColor1);
	int iRedScale = RedValue(wColor2) - iRedStart;
	int iGreenStart = GreenValue(wColor1);
	int iGreenScale = GreenValue(wColor2) - iGreenStart;
	int iBlueStart = BlueValue(wColor1);
	int iBlueScale = BlueValue(wColor2) - iBlueStart;

	//	Do it

	int i, x, y;

	if (ax > ay)		//	x-axis dominant
		{
		//	Compute the distance from the mid-point of the line to
		//	the top edge along the y-axis.

		double rHalfHeight = (double)iWidth * sqrt((double)(dx*dx + dy*dy)) / (double)(2 * abs(dx));
		int cyHalfCount = (int)(((2 * rHalfHeight) - 1) / 2);
		int cyCount = 2 * cyHalfCount + 1;

		int d = ay - ax / 2;

		//	Compute information for anti-aliasing. rDist is the distance
		//	from the top solid point to the edge of the line. We update
		//	the value as we proceed along the x-axis and use it for
		//	anti-aliasing the edge of the line. A negative value means
		//	that the top point is outside the edge.

		double rSlope = (double)dy / (double)dx;
		double rDistTop = rHalfHeight - (double)cyHalfCount - 0.25;
		double rDistBottom = rDistTop;

		//	Compute the information for fading the color from wColor1
		//	to wColor2.

		int iSteps = dx * sx;

		//	Draw!

		for (x = x1, y = y1; x != x2; x += sx)
			{
			int iStep = (x - x1) * sx;
			WORD wColor = RGBValue(iRedStart + (iStep * iRedScale / iSteps),
					iGreenStart + (iStep * iGreenScale / iSteps),
					iBlueStart + (iStep * iBlueScale / iSteps));

			int yTop = y - cyHalfCount;

			//	Draw anti-aliasing above the line

			if (rDistTop > 0)
				DrawPixelTrans(x, yTop - 1, wColor, (BYTE)(255.0 * (rDistTop > 1.0 ? 1.0 : rDistTop)));

			//	Draw the solid part of the line

			if (x >= 0 && x < m_cxWidth)
				{
				WORD *pPos = GetPixel(GetRowStart(yTop), x);

				for (i = yTop; i < yTop + cyCount; i++)
					{
					if (i >= 0 && i < m_cyHeight)
						*pPos = wColor;

					pPos = NextRow(pPos);
					}
				}

			//	Draw anti-aliasing below the line

			if (rDistBottom > 0)
				DrawPixelTrans(x, yTop + cyCount, wColor, (BYTE)(255.0 * (rDistBottom > 1.0 ? 1.0 : rDistBottom)));

			//	Next point

			if (d >= 0)
				{
				y = y + sy;
				d = d - ax;
				rDistTop += sy;
				rDistBottom -= sy;
				}

			d = d + ay;
			rDistTop -= rSlope * sx;
			rDistBottom += rSlope * sx;
			}
		}
	else				//	y-axis dominant
		{
		//	Compute the distance from the mid-point of the line to
		//	the left-edge of the x-axis

		double rHalfWidth = (double)iWidth * sqrt((double)(dx*dx + dy*dy)) / (double)(2 * abs(dy));
		int cxHalfCount = (int)(((2 * rHalfWidth) - 1) / 2);
		int cxCount = 2 * cxHalfCount + 1;

		int d = ax - ay / 2;

		//	Compute information for anti-aliasing. rDist is the distance
		//	from the top solid point to the edge of the line. We update
		//	the value as we proceed along the x-axis and use it for
		//	anti-aliasing the edge of the line. A negative value means
		//	that the top point is outside the edge.

		double rSlope = (double)dx / (double)dy;
		double rDistLeft = rHalfWidth - (double)cxHalfCount - 0.25;
		double rDistRight = rDistLeft;

		//	Compute the information for fading the color from wColor1
		//	to wColor2.

		int iSteps = dy * sy;

		//	Draw!

		for (y = y1, x = x1; y != y2; y += sy)
			{
			int iStep = (y - y1) * sy;
			WORD wColor = RGBValue(iRedStart + (iStep * iRedScale / iSteps),
					iGreenStart + (iStep * iGreenScale / iSteps),
					iBlueStart + (iStep * iBlueScale / iSteps));

			int xTop = x - cxHalfCount;

			//	Draw anti-aliasing to the left

			if (rDistLeft > 0)
				DrawPixelTrans(xTop - 1, y, wColor, (BYTE)(255.0 * (rDistLeft > 1.0 ? 1.0 : rDistLeft)));

			//	Draw the solid part of the line

			if (y >= 0 && y < m_cyHeight)
				{
				WORD *pPos = GetPixel(GetRowStart(y), xTop);

				for (i = xTop; i < xTop + cxCount; i++)
					if (i >= 0 && i < m_cxWidth)
						*pPos++ = wColor;
					else
						pPos++;
				}

			//	Draw anti-aliasing to the right of the line

			if (rDistRight > 0)
				DrawPixelTrans(xTop + cxCount, y, wColor, (BYTE)(255.0 * (rDistRight > 1.0 ? 1.0 : rDistRight)));

			//	Next point

			if (d >= 0)
				{
				x = x + sx;
				d = d - ay;
				rDistLeft += sx;
				rDistRight -= sx;
				}

			d = d + ax;
			rDistLeft -= rSlope * sy;
			rDistRight += rSlope * sy;
			}
		}
	}

void DrawBrokenLine (CG16bitImage &Dest, int xSrc, int ySrc, int xDest, int yDest, int xyBreak, WORD wColor)

//	DrawBrokenLine
//
//	Draws a connection between two points using only horizontal and vertical lines

	{
	if (xyBreak < 0)
		xyBreak = -xyBreak;

	//	Different algorithm for horizontal vs. vertical orientation

	if (abs(xDest - xSrc) > abs(yDest - ySrc))
		{
		int xDelta = xDest - xSrc;
		if (xDelta < 0)
			{
			Swap(xSrc, xDest);
			xDelta = -xDelta;
			}

		int yDelta = yDest - ySrc;

		//	First segment

		int iDist;
		if (ySrc != yDest)
			iDist = std::min(xyBreak, xDelta);
		else
			iDist = xDelta;

		Dest.FillLine(xSrc, ySrc, iDist, wColor);

		//	Connector

		if (ySrc != yDest)
			{
			if (yDelta > 0)
				Dest.FillColumn(xSrc + iDist, ySrc, yDelta, wColor);
			else
				Dest.FillColumn(xSrc + iDist, yDest, -yDelta, wColor);

			//	Last segment

			if (iDist < xDelta)
				Dest.FillLine(xSrc + iDist, yDest, xDelta - iDist, wColor);
			}
		}
	else
		{
		int yDelta = yDest - ySrc;
		if (yDelta < 0)
			{
			Swap(ySrc, yDest);
			yDelta = -yDelta;
			}

		int xDelta = xDest - xSrc;

		//	First segment

		int iDist;
		if (xSrc != xDest)
			iDist = std::min(xyBreak, yDelta);
		else
			iDist = yDelta;

		Dest.FillColumn(xSrc, ySrc, iDist, wColor);

		//	Connector

		if (xSrc != xDest)
			{
			if (xDelta > 0)
				Dest.FillLine(xSrc, ySrc + iDist, xDelta, wColor);
			else
				Dest.FillLine(xDest, ySrc + iDist, -xDelta, wColor);

			//	Last segment

			if (iDist < yDelta)
				Dest.FillColumn(xDest, ySrc + iDist, yDelta - iDist, wColor);
			}
		}
	}

void CG16bitImage::DrawLineProc (SDrawLineCtx *pCtx, DRAWLINEPROC pfProc)

//	DrawLineProc
//
//	Calls proc at each pixel of a Bresenham line draw algorithm. Must
//	call DrawLineProcInit first

	{
	if (pCtx->IsXDominant())		//	x dominant
		{
		pCtx->d = pCtx->ay - pCtx->ax / 2;
		while (true)
			{
			pfProc(pCtx);

			if (pCtx->x == pCtx->x2)
				return;
			else if (pCtx->d >= 0)
				{
				pCtx->y = pCtx->y + pCtx->sy;
				pCtx->d = pCtx->d - pCtx->ax;
				}

			pCtx->x = pCtx->x + pCtx->sx;
			pCtx->d = pCtx->d + pCtx->ay;
			}
		}
	else				//	y dominant
		{
		pCtx->d = pCtx->ax - pCtx->ay / 2;
		while (true)
			{
			pfProc(pCtx);

			if (pCtx->y == pCtx->y2)
				return;
			else if (pCtx->d >= 0)
				{
				pCtx->x = pCtx->x + pCtx->sx;
				pCtx->d = pCtx->d - pCtx->ay;
				}

			pCtx->y = pCtx->y + pCtx->sy;
			pCtx->d = pCtx->d + pCtx->ax;
			}
		}
	}

void CG16bitImage::DrawLineProcInit (int x1, int y1, int x2, int y2, SDrawLineCtx *pCtx)

//	DrawLineProcInit
//
//	Initializes DrawLineProc structure

	{
	pCtx->pDest = this;

	pCtx->x1 = x1;
	pCtx->y1 = y1;
	pCtx->x2 = x2;
	pCtx->y2 = y2;

	pCtx->dx = x2 - x1;
	pCtx->ax = Absolute(pCtx->dx) * 2;
	pCtx->sx = (pCtx->dx > 0 ? 1 : -1);

	pCtx->dy = y2 - y1;
	pCtx->ay = Absolute(pCtx->dy) * 2;
	pCtx->sy = (pCtx->dy > 0 ? 1 : -1);

	pCtx->x = x1;
	pCtx->y = y1;
	}

void CG16bitImage::WuLine (int x0, int y0, 
						   int x1, int y1,
						   int iWidth,
						   WORD wColor)

//	WuLine
//
//	Draws an antialiased line using Xiaolin Wu's algorithsm.
//	From Michael Abrash's Graphics Programming Black Book
//	p775 (Fast Antialiased Lines Using Wu's Algorithm)

	{
	//	Make sure the line runs top to bottom

	if (y0 > y1)
		{
		int temp;
		temp = y0; y0 = y1; y1 = temp;
		temp = x0; x0 = x1; x1 = temp;
		}

	WORD *pPos = NULL;

	//	Figure out whether the line is going to the left or
	//	to the right.

	int xDir, xDelta, yDelta;
	if ((xDelta = x1 - x0) >= 0)
		xDir = 1;
	else
		{
		xDir = -1;
		xDelta = -xDelta;
		}

	//	See if this is a horizontal line

	if ((yDelta = y1 - y0) == 0)
		{
		//	If offscreen, nothing to do

		if (!InClipY(y0))
			return;

		//	Increment until we are in range

		while (xDelta > 0 && !InClipX(x0))
			{
			xDelta--;
			x0 += xDir;
			}

		while (xDelta > 0 && !InClipX(x0 + xDir * xDelta))
			xDelta--;

		//	Draw

		pPos = GetPixel(GetRowStart(y0), x0);
		while (xDelta-- > 0)
			{
			*pPos = wColor;
			pPos += xDir;
			}

		return;
		}

	//	Draw vertical line

	if (xDelta == 0)
		{
		//	If offscreen, nothing to do

		if (!InClipX(x0))
			return;

		//	Increment until we are in range

		while (yDelta > 0 && !InClipY(y0))
			{
			yDelta--;
			y0++;
			}

		while (yDelta > 0 && !InClipY(y0 + yDelta))
			yDelta--;

		pPos = GetPixel(GetRowStart(y0), x0);
		while (yDelta-- > 0)
			{
			*pPos = wColor;
			pPos = NextRow(pPos);
			}

		return;
		}

	//	Draw diagonal line

	if (xDelta == yDelta)
		{
		//	Increment until we are in range

		while (xDelta > 0 && !InClipX(x0))
			{
			xDelta--;
			yDelta--;
			x0 += xDir;
			}

		while (xDelta > 0 && !InClipX(x0 + xDir * xDelta))
			{
			xDelta--;
			yDelta--;
			}

		//	Increment until we are in range

		while (yDelta > 0 && !InClipY(y0))
			{
			xDelta--;
			yDelta--;
			y0++;
			}

		while (yDelta > 0 && !InClipY(y0 + yDelta))
			{
			xDelta--;
			yDelta--;
			}

		pPos = GetPixel(GetRowStart(y0), x0);
		while (yDelta-- > 0)
			{
			*pPos = wColor;
			pPos = NextRow(pPos) + xDir;
			}

		return;
		}

	//	Draw the initial pixel, which is always exactly intersected
	//	by the line and so needs no weighing

	if (InClipX(x0) && InClipY(y0))
		{
		pPos = GetPixel(GetRowStart(y0), x0);
		*pPos = wColor;
		}

	//	Initialize the line error accumulator

	int iErrorAcc = 0;

	//	Is this an X-major or Y-major line?

	if (yDelta > xDelta)
		{
		//	Y-major line: Calculate 16-bit fixed-point fractional part of a
		//	pixel that x advances each time y advances 1 pixel, truncating
		//	the result so that we won't overrun the endpoint along
		//	the x axis.

		int iErrorAdj = (xDelta << WU_ERROR_BITS) / yDelta;

		//	Draw all pixels except the first and last

		while (--yDelta)
			{
			//	Add the error accumulator and see if we've run over

			iErrorAcc += iErrorAdj;
			if (iErrorAcc > 0xFFFF)
				{
				x0 += xDir;
				iErrorAcc &= 0xFFFF;

				if (pPos)
					{
					if (!InClipX(x0))
						return;

					pPos = pPos + xDir;
					}
				}

			y0++;	//	Y-major, so always advance
			if (pPos)
				{
				if (!InClipY(y0))
					return;

				pPos = NextRow(pPos);
				}

			//	Make sure we're in range

			if (pPos == NULL)
				{
				if (InClipX(x0) && InClipY(y0))
					pPos = GetPixel(GetRowStart(y0), x0);
				else
					continue;
				}

			//	The most significant bits of ErrorAcc give us the
			//	intensity weighting for this pixel, and the complement
			//	of the weighting for the paired pixel

			int iWeighting = iErrorAcc >> WU_INTENSITY_SHIFT;
			*pPos = BlendPixel(*pPos, wColor, (iWeighting ^ WU_WEIGHTING_COMPLEMENT_MASK));

			WORD *pComplementPos = pPos + xDir;
			*pComplementPos = BlendPixel(*pComplementPos, wColor, iWeighting);
			}
		}
	else
		{
		//	X-major line

		int iErrorAdj = (yDelta << WU_ERROR_BITS) / xDelta;

		//	Draw all pixels except the first and last

		while (--xDelta)
			{
			//	Add the error accumulator and see if we've run over

			iErrorAcc += iErrorAdj;
			if (iErrorAcc > 0xFFFF)
				{
				y0++;
				iErrorAcc &= 0xFFFF;

				if (pPos)
					{
					if (!InClipY(y0))
						return;
					pPos = NextRow(pPos);
					}
				}

			x0 += xDir;	//	X-major, so always advance
			if (pPos)
				{
				if (!InClipX(x0))
					return;
				pPos = pPos + xDir;
				}

			//	Make sure we're in range

			if (pPos == NULL)
				{
				if (InClipX(x0) && InClipY(y0))
					pPos = GetPixel(GetRowStart(y0), x0);
				else
					continue;
				}

			//	The most significant bits of ErrorAcc give us the
			//	intensity weighting for this pixel, and the complement
			//	of the weighting for the paired pixel

			int iWeighting = iErrorAcc >> WU_INTENSITY_SHIFT;
			*pPos = BlendPixel(*pPos, wColor, (iWeighting ^ WU_WEIGHTING_COMPLEMENT_MASK));

			WORD *pComplementPos = NextRow(pPos);
			*pComplementPos = BlendPixel(*pComplementPos, wColor, iWeighting);
			}
		}

	//	Now draw the final pixel

	if (InClipX(x1) && InClipY(y1))
		*GetPixel(GetRowStart(y1), x1) = wColor;
	}

void CG16bitImage::DrawDot (int x, int y, WORD wColor, MarkerTypes iMarker)

//	DrawDot
//
//	Paints a dot in various styles

	{
	switch (iMarker)
		{
		case markerPixel:
			{
			DrawPixel(x, y, wColor);
			break;
			}

		case markerSmallRound:
			{
			int xStart, xEnd, yStart, yEnd;

			x = x - 1;
			y = y - 1;

			xStart = std::max(0, x);
			xEnd = std::min(m_cxWidth, x + 3);
			yStart = std::max(0, y);
			yEnd = std::min(m_cyHeight, y + 3);

			for (int yPos = yStart; yPos < yEnd; yPos++)
				{
				WORD *pPos = GetPixel(GetRowStart(yPos), xStart);

				for (int xPos = xStart; xPos < xEnd; xPos++)
					{
					BYTE byTrans = g_wSmallRoundMask[(yPos - y) * 3 + xPos - x];

					if (byTrans == 255)
						*pPos = wColor;
					else if (byTrans == 0)
						{
						}
					else
						*pPos = BlendPixel(*pPos, wColor, byTrans);

					pPos++;
					}

				pPos = NextRow(pPos);
				}

			break;
			}

		case markerSmallCross:
			{
			int yTop = y - SMALL_SQUARE_SIZE;
			int yBottom = y + SMALL_SQUARE_SIZE;
			int xLeft = x - SMALL_SQUARE_SIZE;
			int xRight = x + SMALL_SQUARE_SIZE;

			//	Draw the horizontal line

			if (y >= 0 && y < m_cyHeight)
				{
				WORD *pRowStart = GetRowStart(y);
				WORD *pPos = pRowStart + std::max(0, xLeft);
				WORD *pPosEnd = pRowStart + std::min(xRight+1, m_cxWidth);

				while (pPos < pPosEnd)
					*pPos++ = wColor;
				}

			//	Draw the vertical line

			if (x >= 0 && x < m_cxWidth)
				{
				WORD *pPos = GetRowStart(std::max(yTop+1, 0)) + x;
				WORD *pPosEnd = GetRowStart(std::min(yBottom, m_cyHeight)) + x;

				while (pPos < pPosEnd)
					{
					*pPos = wColor;
					pPos = NextRow(pPos);
					}
				}

			break;
			}

		case markerMediumCross:
			{
			int yTop = y - MEDIUM_SQUARE_SIZE;
			int yBottom = y + MEDIUM_SQUARE_SIZE;
			int xLeft = x - MEDIUM_SQUARE_SIZE;
			int xRight = x + MEDIUM_SQUARE_SIZE;

			//	Draw the horizontal line

			if (y >= 0 && y < m_cyHeight)
				{
				WORD *pRowStart = GetRowStart(y);
				WORD *pPos = pRowStart + std::max(0, xLeft);
				WORD *pPosEnd = pRowStart + std::min(xRight+1, m_cxWidth);

				while (pPos < pPosEnd)
					*pPos++ = wColor;
				}

			//	Draw the vertical line

			if (x >= 0 && x < m_cxWidth)
				{
				WORD *pPos = GetRowStart(std::max(yTop, 0)) + x;
				WORD *pPosEnd = GetRowStart(std::min(yBottom+1, m_cyHeight)) + x;

				while (pPos < pPosEnd)
					{
					*pPos = wColor;
					pPos = NextRow(pPos);
					}
				}

			break;
			}

		case markerSmallFilledSquare:
			{
			int xLeft = x - SMALL_SQUARE_SIZE;
			int yTop = y - SMALL_SQUARE_SIZE;
			WORD wInsideColor = BlendPixel(0, wColor, 192);

			DrawDot(x, y, wColor, markerSmallSquare);
			Fill(xLeft + 1, yTop + 1, 2 * SMALL_SQUARE_SIZE - 1, 2 * SMALL_SQUARE_SIZE - 1, wInsideColor);
			break;
			}

		case markerSmallSquare:
			{
			int yTop = y - SMALL_SQUARE_SIZE;
			int yBottom = y + SMALL_SQUARE_SIZE;
			int xLeft = x - SMALL_SQUARE_SIZE;
			int xRight = x + SMALL_SQUARE_SIZE;

			//	Draw the top line

			if (yTop >= 0 && yTop < m_cyHeight)
				{
				WORD *pRowStart = GetRowStart(yTop);
				WORD *pPos = pRowStart + std::max(0, xLeft);
				WORD *pPosEnd = pRowStart + std::min(xRight+1, m_cxWidth);

				while (pPos < pPosEnd)
					*pPos++ = wColor;
				}

			//	Draw the bottom line

			if (yBottom >= 0 && yBottom < m_cyHeight)
				{
				WORD *pRowStart = GetRowStart(yBottom);
				WORD *pPos = pRowStart + std::max(0, xLeft);
				WORD *pPosEnd = pRowStart + std::min(xRight+1, m_cxWidth);

				while (pPos < pPosEnd)
					*pPos++ = wColor;
				}

			//	Draw the left line

			if (xLeft >= 0 && xLeft < m_cxWidth)
				{
				WORD *pPos = GetRowStart(std::max(yTop+1, 0)) + xLeft;
				WORD *pPosEnd = GetRowStart(std::min(yBottom, m_cyHeight)) + xLeft;

				while (pPos < pPosEnd)
					{
					*pPos = wColor;
					pPos = NextRow(pPos);
					}
				}

			//	Draw the right line

			if (xRight >= 0 && xRight < m_cxWidth)
				{
				WORD *pPos = GetRowStart(std::max(yTop+1, 0)) + xRight;
				WORD *pPosEnd = GetRowStart(std::min(yBottom, m_cyHeight)) + xRight;

				while (pPos < pPosEnd)
					{
					*pPos = wColor;
					pPos = NextRow(pPos);
					}
				}

			break;
			}
		}
	}

void DrawDottedLine (CG16bitImage &Dest, int x1, int y1, int x2, int y2, WORD wColor)

//	DrawDottedLine
//
//	Draw a dotted line

	{
	int dx = x2 - x1;
	int ax = Absolute(dx) * 2;
	int sx = (dx > 0 ? 1 : -1);

	int dy = y2 - y1;
	int ay = Absolute(dy) * 2;
	int sy = (dy > 0 ? 1 : -1);

	int x = x1;
	int y = y1;
	int d;

	if (ax > ay)		//	x dominant
		{
		d = ay - ax / 2;
		while (true)
			{
			if ((x % 2) == 0)
				Dest.DrawPixel(x, y, wColor);

			if (x == x2)
				return;
			else if (d >= 0)
				{
				y = y + sy;
				d = d - ax;
				}

			x = x + sx;
			d = d + ay;
			}
		}
	else				//	y dominant
		{
		d = ax - ay / 2;
		while (true)
			{
			if ((y % 2) == 0)
				Dest.DrawPixel(x, y, wColor);

			if (y == y2)
				return;
			else if (d >= 0)
				{
				x = x + sx;
				d = d - ay;
				}

			y = y + sy;
			d = d + ax;
			}
		}
	}

void DrawRectDotted (CG16bitImage &Dest, int x, int y, int cxWidth, int cyHeight, WORD wColor)

//	DrawRectDotted
//
//	Draws a dotted rect outline

	{
	DrawDottedLine(Dest, x, y, x + cxWidth, y, wColor);
	DrawDottedLine(Dest, x, y + cyHeight, x + cxWidth, y + cyHeight, wColor);
	DrawDottedLine(Dest, x, y, x, y + cyHeight, wColor);
	DrawDottedLine(Dest, x + cxWidth, y, x + cxWidth, y + cyHeight, wColor);
	}
