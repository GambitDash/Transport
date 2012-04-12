//	16bitDrawCircle.cpp
//
//	Drawing routines for circles

#include <math.h>
#include <stdio.h>

#include "portage.h"

#include "CG16bitImage.h"

//	DrawAlphaGradientCircle ---------------------------------------------------

struct SAlphaGradientCircleLineCtx
	{
	CG16bitImage *pDest;
	int xDest;
	int yDest;
	int iRadius;
	WORD wColor;

	DWORD dwRed;
	DWORD dwGreen;
	DWORD dwBlue;
	};

void DrawAlphaGradientCircleLine (const SAlphaGradientCircleLineCtx &Ctx, int x, int y)
	{
	int xStart = Ctx.xDest - x;
	int xEnd = Ctx.xDest + x + 1;
	const RECT &rcClip = Ctx.pDest->GetClipRect();

	if (xEnd <= rcClip.left || xStart >= rcClip.right)
		return;

	//	See which lines we need to paint

	int yLine = Ctx.yDest - y;
	bool bPaintTop = (yLine >= rcClip.top && yLine < rcClip.bottom);
	WORD *pCenterTop = Ctx.pDest->GetRowStart(yLine) + Ctx.xDest;

	yLine = Ctx.yDest + y;
	bool bPaintBottom = ((y > 0) && (yLine >= rcClip.top && yLine < rcClip.bottom));
	WORD *pCenterBottom = Ctx.pDest->GetRowStart(yLine) + Ctx.xDest;

	//	Compute radius increment

	int iRadius = y;
	int d = -y;
	int deltaE = 3;
	int deltaSE = -2 * y + 1;

	//	Loop

	int xPos = 0;

	//	This will skip the center pixel in the circle (avoids a divide by
	//	zero in the inner loop).

	if (y == 0)
		{
		xPos = 1;
		d += deltaSE;
		deltaE += 2;
		iRadius++;
		}

	//	Blt the line 

	while (xPos <= x)
		{
		//	Figure out the radius of the pixel at this location

		if (d < 0)
			{
			d += deltaE;
			deltaE += 2;
			deltaSE += 2;
			}
		else
			{
			d += deltaSE;
			deltaE += 2;
			//  deltaSE += 0;
			iRadius++;
			}

		//	Compute the transparency based on the radius

		DWORD dwTrans = 255 - (255 * iRadius / Ctx.iRadius);
		if (dwTrans > 255)
			{
			xPos++;
			continue;
			}

#define DRAW_PIXEL(pos)	\
			{ \
			DWORD dwDest = (DWORD)*(pos);	\
			DWORD dwRedDest = (dwDest >> 11) & 0x1f;	\
			DWORD dwGreenDest = (dwDest >> 5) & 0x3f;	\
			DWORD dwBlueDest = dwDest & 0x1f;	\
\
			*(pos) = (WORD)((dwTrans * (Ctx.dwBlue - dwBlueDest) >> 8) + dwBlueDest |	\
					((dwTrans * (Ctx.dwGreen - dwGreenDest) >> 8) + dwGreenDest) << 5 |	\
					((dwTrans * (Ctx.dwRed - dwRedDest) >> 8) + dwRedDest) << 11);	\
			}

		//	Paint

		if (Ctx.xDest - xPos < rcClip.right && Ctx.xDest - xPos >= rcClip.left)
			{
			if (bPaintTop)
				DRAW_PIXEL(pCenterTop - xPos);

			if (bPaintBottom)
				DRAW_PIXEL(pCenterBottom - xPos);
			}

		if (xPos > 0 && Ctx.xDest + xPos < rcClip.right && Ctx.xDest + xPos >= rcClip.left)
			{
			if (bPaintTop)
				DRAW_PIXEL(pCenterTop + xPos);

			if (bPaintBottom)
				DRAW_PIXEL(pCenterBottom + xPos);
			}
#undef DRAW_PIXEL

		xPos++;
		}
	}

void DrawAlphaGradientCircle (CG16bitImage &Dest, 
							  int xDest, 
							  int yDest, 
							  int iRadius,
							  WORD wColor)

//	DrawAlphaGradientCircle
//
//	Draws a filled circle of the given color. The circle has an alpha gradient
//	that ranges from fully opaque in the center to fully transparent at the edges.

	{
	//	Deal with edge-conditions

	if (iRadius <= 0)
		{
		Dest.DrawPixel(xDest, yDest, wColor);
		return;
		}

	//	Initialize some stuff

	int x = 0;
	int y = iRadius;
	int d = 1 - iRadius;
	int deltaE = 3;
	int deltaSE = -2 * iRadius + 5;

	//	Prepare struct

	SAlphaGradientCircleLineCtx Ctx;
	Ctx.pDest = &Dest;
	Ctx.xDest = xDest;
	Ctx.yDest = yDest;
	Ctx.iRadius = iRadius;
	Ctx.wColor = wColor;

	//	Pre-compute some color info

	DWORD dwColor = wColor;
	Ctx.dwRed = (dwColor >> 11) & 0x1f;
	Ctx.dwGreen = (dwColor >> 5) & 0x3f;
	Ctx.dwBlue = dwColor & 0x1f;

	//	Draw central line

	DrawAlphaGradientCircleLine(Ctx, iRadius, 0);

	//	Draw lines above and below the center

	int iLastDraw = -1;
	while (y > x)
		{
		if (d < 0)
			{
			d += deltaE;
			deltaE += 2;
			deltaSE += 2;
			}
		else
			{
			d += deltaSE;
			deltaE += 2;
			deltaSE += 4;

			//	Draw lines

			DrawAlphaGradientCircleLine(Ctx, x, y);
			iLastDraw = y;

			//	Next

			y--;
			}

		x++;

		//	Draw lines

		if (x != iLastDraw)
			DrawAlphaGradientCircleLine(Ctx, y, x);
		}
	}

//	DrawBltCircle -------------------------------------------------------------

struct SBltCircleLineCtx
	{
	//	Dest
	CG16bitImage *pDest;
	int xDest;
	int yDest;
	int iRadius;

	//	Source
	const CG16bitImage *pSrc;
	int xSrc;
	int ySrc;
	int cxSrc;
	int cySrc;
	DWORD byOpacity;
	};

void DrawBltCircleLine (const SBltCircleLineCtx &Ctx, int x, int y)

//	DrawBltCircleLine
//
//	Draws a single horizontal line across the circle. For each point on the line
//	we compute the radius and angle so that we can map a bitmap around the circle
//	(effectively, a polar coordinates map).
//
//	The calculation of the radius uses a modified version of the algorithm
//	described in page 84 of Foley and van Dam. But because we are computing the
//	radius as we advance x, the function that we are computing is:
//
//	F(x,r) = x^2 - r^2 + Y^2
//
//	In which Y^2 is constant (the y coordinate of this line).
//
//	The solution leads to the two decision functions:
//
//	deltaE = 2xp + 3
//	deltaSE = 2xp - 2r + 1
//
//	The calculation of the angle relies on a fast arctangent approximation
//	on page 389 of Graphic Gems II.

	{
	int xStart = Ctx.xDest - x;
	int xEnd = Ctx.xDest + x + 1;
	const RECT &rcClip = Ctx.pDest->GetClipRect();

	if (xEnd <= rcClip.left || xStart >= rcClip.right)
		return;

	//	See which lines we need to paint

	int yLine = Ctx.yDest - y;
	bool bPaintTop = (yLine >= rcClip.top && yLine < rcClip.bottom);
	WORD *pCenterTop = Ctx.pDest->GetRowStart(yLine) + Ctx.xDest;

	yLine = Ctx.yDest + y;
	bool bPaintBottom = ((y > 0) && (yLine >= rcClip.top && yLine < rcClip.bottom));
	WORD *pCenterBottom = Ctx.pDest->GetRowStart(yLine) + Ctx.xDest;

	//	Compute radius increment

	int iRadius = y;
	int d = -y;
	int deltaE = 3;
	int deltaSE = -2 * y + 1;

	//	Compute angle increment

	const int iFixedPoint = 8192;
	int iAngle = 2 * iFixedPoint;
	int angle1 = (y == 0 ? (2 * iFixedPoint) : (iFixedPoint / y));
	int num1 = iFixedPoint * y;

	int cxSrcQuadrant = Ctx.cxSrc / 4;
	int cxSrcHalf = Ctx.cxSrc / 2;

	//	Loop

	int xPos = 0;

	//	This will skip the center pixel in the circle (avoids a divide by
	//	zero in the inner loop).

	if (y == 0)
		{
		xPos = 1;
		d += deltaSE;
		deltaE += 2;
		iRadius++;
		}

	//	Blt the line based on the type of source image

	if (Ctx.pSrc->HasAlpha() && Ctx.byOpacity != 255)
		{
		BYTE *pAlpha5 = g_Alpha5[Ctx.byOpacity];
		BYTE *pAlpha6 = g_Alpha6[Ctx.byOpacity];

		while (xPos <= x)
			{
			//	Figure out the radius of the pixel at this location

			if (d < 0)
				{
				d += deltaE;
				deltaE += 2;
				deltaSE += 2;
				}
			else
				{
				d += deltaSE;
				deltaE += 2;
			//  deltaSE += 0;
				iRadius++;
				}

			//	If we're beyond the size of the source image then continue

			int yOffset = Ctx.iRadius - iRadius;
			if (yOffset >= Ctx.cySrc || yOffset < 0)
				{
				xPos++;
				continue;
				}

			yOffset += Ctx.ySrc;

			//	Figure out the angle of the pixel at this location

			if (xPos < y)
				iAngle -= angle1;
			else
				iAngle = num1 / xPos;

			int xOffset = Ctx.xSrc + (iAngle * cxSrcQuadrant / (2 * iFixedPoint));

			//	Figure out the row of the source image

			WORD *pSrcRow = Ctx.pSrc->GetRowStart(yOffset);
			BYTE *pSrcAlphaRow = Ctx.pSrc->GetAlphaRow(yOffset);

			//	Paint

#define DRAW_PIXEL									\
				{									\
				DWORD dwDestAlpha = 255 - ((*pAlpha) * Ctx.byOpacity / 255); \
				BYTE *pAlpha5Inv = g_Alpha5[dwDestAlpha]; \
				BYTE *pAlpha6Inv = g_Alpha6[dwDestAlpha]; \
\
				DWORD dTemp = *pDest; \
				DWORD sTemp = *(pSrcRow + xSrc); \
\
				DWORD dwRedResult = pAlpha5Inv[(dTemp & 0xf800) >> 11] + pAlpha5[(sTemp & 0xf800) >> 11]; \
				DWORD dwGreenResult = pAlpha6Inv[(dTemp & 0x7e0) >> 5] + pAlpha6[(sTemp & 0x7e0) >> 5]; \
				DWORD dwBlueResult = pAlpha5Inv[(dTemp & 0x1f)] + pAlpha5[(sTemp & 0x1f)]; \
\
				*pDest = (WORD)((dwRedResult << 11) | (dwGreenResult << 5) | (dwBlueResult)); \
				}

			if (Ctx.xDest - xPos < rcClip.right && Ctx.xDest - xPos >= rcClip.left)
				{
				if (bPaintTop)
					{
					int xSrc = cxSrcHalf - xOffset;
					BYTE *pAlpha = pSrcAlphaRow + xSrc;

					if (*pAlpha)
						{
						WORD *pDest = pCenterTop - xPos;
						DRAW_PIXEL;
						}
					}

				if (bPaintBottom)
					{
					int xSrc = cxSrcHalf + xOffset;
					BYTE *pAlpha = pSrcAlphaRow + xSrc;

					if (*pAlpha)
						{
						WORD *pDest = pCenterBottom - xPos;
						DRAW_PIXEL;
						}
					}
				}

			if (xPos > 0 && Ctx.xDest + xPos < rcClip.right && Ctx.xDest + xPos >= rcClip.left)
				{
				if (bPaintTop)
					{
					int xSrc = xOffset;
					BYTE *pAlpha = pSrcAlphaRow + xSrc;

					if (*pAlpha)
						{
						WORD *pDest = pCenterTop + xPos;
						DRAW_PIXEL;
						}
					}

				if (bPaintBottom)
					{
					int xSrc = (Ctx.cxSrc - 1) - xOffset;
					BYTE *pAlpha = pSrcAlphaRow + xSrc;

					if (*pAlpha)
						{
						WORD *pDest = pCenterBottom + xPos;
						DRAW_PIXEL;
						}
					}
				}
#undef DRAW_PIXEL

			xPos++;
			}
		}
	else if (Ctx.byOpacity != 255)
		{
		BYTE *pAlpha5 = g_Alpha5[Ctx.byOpacity];
		BYTE *pAlpha6 = g_Alpha6[Ctx.byOpacity];
		BYTE *pAlpha5Inv = g_Alpha5[255 - Ctx.byOpacity];
		BYTE *pAlpha6Inv = g_Alpha6[255 - Ctx.byOpacity];

		while (xPos <= x)
			{
			//	Figure out the radius of the pixel at this location

			if (d < 0)
				{
				d += deltaE;
				deltaE += 2;
				deltaSE += 2;
				}
			else
				{
				d += deltaSE;
				deltaE += 2;
			//  deltaSE += 0;
				iRadius++;
				}

			//	If we're beyond the size of the source image then continue

			int yOffset = Ctx.iRadius - iRadius;
			if (yOffset >= Ctx.cySrc || yOffset < 0)
				{
				xPos++;
				continue;
				}

			yOffset += Ctx.ySrc;

			//	Figure out the angle of the pixel at this location

			if (xPos < y)
				iAngle -= angle1;
			else
				iAngle = num1 / xPos;

			int xOffset = Ctx.xSrc + (iAngle * cxSrcQuadrant / (2 * iFixedPoint));

			//	Figure out the row of the source image

			WORD *pSrcRow = Ctx.pSrc->GetRowStart(yOffset);
			BYTE *pSrcAlphaRow = Ctx.pSrc->GetAlphaRow(yOffset);

			//	Paint

#define DRAW_PIXEL									\
				{									\
				DWORD dTemp = *pDest; \
				DWORD sTemp = *(pSrcRow + xSrc); \
\
				DWORD dwRedResult = pAlpha5Inv[(dTemp & 0xf800) >> 11] + pAlpha5[(sTemp & 0xf800) >> 11]; \
				DWORD dwGreenResult = pAlpha6Inv[(dTemp & 0x7e0) >> 5] + pAlpha6[(sTemp & 0x7e0) >> 5]; \
				DWORD dwBlueResult = pAlpha5Inv[(dTemp & 0x1f)] + pAlpha5[(sTemp & 0x1f)]; \
\
				*pDest = (WORD)((dwRedResult << 11) | (dwGreenResult << 5) | (dwBlueResult)); \
				}

			if (Ctx.xDest - xPos < rcClip.right && Ctx.xDest - xPos >= rcClip.left)
				{
				if (bPaintTop)
					{
					int xSrc = cxSrcHalf - xOffset;
					BYTE *pAlpha = pSrcAlphaRow + xSrc;

					if (*pAlpha)
						{
						WORD *pDest = pCenterTop - xPos;
						DRAW_PIXEL;
						}
					}

				if (bPaintBottom)
					{
					int xSrc = cxSrcHalf + xOffset;
					BYTE *pAlpha = pSrcAlphaRow + xSrc;

					if (*pAlpha)
						{
						WORD *pDest = pCenterBottom - xPos;
						DRAW_PIXEL;
						}
					}
				}

			if (xPos > 0 && Ctx.xDest + xPos < rcClip.right && Ctx.xDest + xPos >= rcClip.left)
				{
				if (bPaintTop)
					{
					int xSrc = xOffset;
					BYTE *pAlpha = pSrcAlphaRow + xSrc;

					if (*pAlpha)
						{
						WORD *pDest = pCenterTop + xPos;
						DRAW_PIXEL;
						}
					}

				if (bPaintBottom)
					{
					int xSrc = (Ctx.cxSrc - 1) - xOffset;
					BYTE *pAlpha = pSrcAlphaRow + xSrc;

					if (*pAlpha)
						{
						WORD *pDest = pCenterBottom + xPos;
						DRAW_PIXEL;
						}
					}
				}
#undef DRAW_PIXEL

			xPos++;
			}
		}
	else if (Ctx.pSrc->HasAlpha())
		{
		while (xPos <= x)
			{
			//	Figure out the radius of the pixel at this location

			if (d < 0)
				{
				d += deltaE;
				deltaE += 2;
				deltaSE += 2;
				}
			else
				{
				d += deltaSE;
				deltaE += 2;
			//  deltaSE += 0;
				iRadius++;
				}

			//	If we're beyond the size of the source image then continue

			int yOffset = Ctx.iRadius - iRadius;
			if (yOffset >= Ctx.cySrc || yOffset < 0)
				{
				xPos++;
				continue;
				}

			yOffset += Ctx.ySrc;

			//	Figure out the angle of the pixel at this location

			if (xPos < y)
				iAngle -= angle1;
			else
				iAngle = num1 / xPos;

			int xOffset = Ctx.xSrc + (iAngle * cxSrcQuadrant / (2 * iFixedPoint));

			//	Figure out the row of the source image

			WORD *pSrcRow = Ctx.pSrc->GetRowStart(yOffset);
			BYTE *pSrcAlphaRow = Ctx.pSrc->GetAlphaRow(yOffset);

			//	Paint

#define DRAW_PIXEL									\
				{									\
				if (*pAlpha == 255)					\
					*pDest = *(pSrcRow + xSrc);		\
				else								\
					{								\
					DWORD dwInvTrans = ((*pAlpha) ^ 0xff);	\
					DWORD dwSource = *(pSrcRow + xSrc);	\
					DWORD dwDest = *pDest;	\
\
					DWORD dwRedGreenS = ((dwSource << 8) & 0x00f80000) | (dwSource & 0x000007e0);	\
					DWORD dwRedGreen = (((((dwDest << 8) & 0x00f80000) | (dwDest & 0x000007e0)) * dwInvTrans) >> 8) + dwRedGreenS;	\
					DWORD dwBlue = (((dwDest & 0x1f) * dwInvTrans) >> 8) + (dwSource & 0x1f);	\
\
					*pDest = (WORD)(((dwRedGreen & 0x00f80000) >> 8) | (dwRedGreen & 0x000007e0) | dwBlue);	\
					}	\
				}

			if (Ctx.xDest - xPos < rcClip.right && Ctx.xDest - xPos >= rcClip.left)
				{
				if (bPaintTop)
					{
					int xSrc = cxSrcHalf - xOffset;
					BYTE *pAlpha = pSrcAlphaRow + xSrc;

					if (*pAlpha)
						{
						WORD *pDest = pCenterTop - xPos;
						DRAW_PIXEL;
						}
					}

				if (bPaintBottom)
					{
					int xSrc = cxSrcHalf + xOffset;
					BYTE *pAlpha = pSrcAlphaRow + xSrc;

					if (*pAlpha)
						{
						WORD *pDest = pCenterBottom - xPos;
						DRAW_PIXEL;
						}
					}
				}

			if (xPos > 0 && Ctx.xDest + xPos < rcClip.right && Ctx.xDest + xPos >= rcClip.left)
				{
				if (bPaintTop)
					{
					int xSrc = xOffset;
					BYTE *pAlpha = pSrcAlphaRow + xSrc;

					if (*pAlpha)
						{
						WORD *pDest = pCenterTop + xPos;
						DRAW_PIXEL;
						}
					}

				if (bPaintBottom)
					{
					int xSrc = (Ctx.cxSrc - 1) - xOffset;
					BYTE *pAlpha = pSrcAlphaRow + xSrc;

					if (*pAlpha)
						{
						WORD *pDest = pCenterBottom + xPos;
						DRAW_PIXEL;
						}
					}
				}
#undef DRAW_PIXEL

			xPos++;
			}
		}
	else
		{
		while (xPos <= x)
			{
			//	Figure out the radius of the pixel at this location

			if (d < 0)
				{
				d += deltaE;
				deltaE += 2;
				deltaSE += 2;
				}
			else
				{
				d += deltaSE;
				deltaE += 2;
			//  deltaSE += 0;
				iRadius++;
				}

			//	If we're beyond the size of the source image then continue

			int yOffset = Ctx.iRadius - iRadius;
			if (yOffset >= Ctx.cySrc || yOffset < 0)
				{
				xPos++;
				continue;
				}

			yOffset += Ctx.ySrc;

			//	Figure out the angle of the pixel at this location

			if (xPos < y)
				iAngle -= angle1;
			else
				iAngle = num1 / xPos;

			int xOffset = iAngle * cxSrcQuadrant / (2 * iFixedPoint);

			//	Figure out the row of the source image

			WORD *pSrcRow = Ctx.pSrc->GetRowStart(yOffset);

			//	Paint

			if (Ctx.xDest - xPos < rcClip.right && Ctx.xDest - xPos >= rcClip.left)
				{
				if (bPaintTop)
					*(pCenterTop - xPos) = *(pSrcRow + cxSrcHalf - xOffset);

				if (bPaintBottom)
					*(pCenterBottom - xPos) = *(pSrcRow + cxSrcHalf + xOffset);
				}

			if (xPos > 0 && Ctx.xDest + xPos < rcClip.right && Ctx.xDest + xPos >= rcClip.left)
				{
				if (bPaintTop)
					*(pCenterTop + xPos) = *(pSrcRow + xOffset);

				if (bPaintBottom)
					*(pCenterBottom + xPos) = *(pSrcRow + (Ctx.cxSrc - 1) - xOffset);
				}

			xPos++;
			}
		}
	}

void DrawBltCircle (CG16bitImage &Dest, 
					int xDest, 
					int yDest, 
					int iRadius,
					const CG16bitImage &Src,
					int xSrc,
					int ySrc,
					int cxSrc,
					int cySrc,
					DWORD byOpacity)

//	DrawBltCircle
//
//	Takes the source image and blts it as a circle

	{
	//	Deal with edge-conditions

	if (iRadius <= 0)
		return;

	//	Initialize some stuff

	int x = 0;
	int y = iRadius;
	int d = 1 - iRadius;
	int deltaE = 3;
	int deltaSE = -2 * iRadius + 5;

	//	Prepare struct

	SBltCircleLineCtx Ctx;
	Ctx.pDest = &Dest;
	Ctx.xDest = xDest;
	Ctx.yDest = yDest;
	Ctx.iRadius = iRadius;
	Ctx.pSrc = &Src;
	Ctx.xSrc = xSrc;
	Ctx.ySrc = ySrc;
	Ctx.cxSrc = cxSrc;
	Ctx.cySrc = cySrc;
	Ctx.byOpacity = byOpacity;

	//	Draw central line

	DrawBltCircleLine(Ctx, iRadius, 0);

	//	Draw lines above and below the center

	int iLastDraw = -1;
	while (y > x)
		{
		if (d < 0)
			{
			d += deltaE;
			deltaE += 2;
			deltaSE += 2;
			}
		else
			{
			d += deltaSE;
			deltaE += 2;
			deltaSE += 4;

			//	Draw lines

			DrawBltCircleLine(Ctx, x, y);
			iLastDraw = y;

			//	Next

			y--;
			}

		x++;

		//	Draw lines

		if (x != iLastDraw)
			DrawBltCircleLine(Ctx, y, x);
		}
	}

//	DrawFilledCircle ----------------------------------------------------------

void DrawFilledCircle (CG16bitImage &Dest, int xDest, int yDest, int iRadius, WORD wColor)

//	DrawFilledCircle
//
//	Draws a filled circle centered on the given coordinates.
//
//	Foley and vanDam. Computer Graphics: Principles and Practice.
//	p.87

	{
	//	Deal with edge-conditions

	if (iRadius <= 0)
		{
		Dest.DrawPixel(xDest, yDest, wColor);
		return;
		}

	//	Initialize some stuff

	int x = 0;
	int y = iRadius;
	int d = 1 - iRadius;
	int deltaE = 3;
	int deltaSE = -2 * iRadius + 5;

	Dest.FillLine(xDest - iRadius, yDest, 1 + 2 * iRadius, wColor);

	//	Loop

	while (y > x)
		{
		if (d < 0)
			{
			d += deltaE;
			deltaE += 2;
			deltaSE += 2;
			}
		else
			{
			d += deltaSE;
			deltaE += 2;
			deltaSE += 4;

			Dest.FillLine(xDest - x, yDest - y, 1 + 2 * x, wColor);
			Dest.FillLine(xDest - x, yDest + y, 1 + 2 * x, wColor);

			y--;
			}

		x++;

		if (y >= x)
			{
			Dest.FillLine(xDest - y, yDest - x, 1 + 2 * y, wColor);
			Dest.FillLine(xDest - y, yDest + x, 1 + 2 * y, wColor);
			}
		}
	}

void DrawFilledCircleGray (CG16bitImage &Dest, int xDest, int yDest, int iRadius, WORD wColor, DWORD byOpacity)

//	DrawFilledCircleGray
//
//	Draws an transparent filled circle

	{
	//	Deal with edge-conditions

	if (iRadius <= 0)
		{
		Dest.DrawPixelTrans(xDest, yDest, wColor, (BYTE)byOpacity);
		return;
		}
	else if (byOpacity == 255)
		{
		DrawFilledCircle(Dest, xDest, yDest, iRadius, wColor);
		return;
		}

	//	Initialize some stuff

	int x = 0;
	int y = iRadius;
	int d = 1 - iRadius;
	int deltaE = 3;
	int deltaSE = -2 * iRadius + 5;

	Dest.FillLineGray(xDest - iRadius, yDest, 1 + 2 * iRadius, wColor, byOpacity);

	//	Loop

	while (y > x)
		{
		if (d < 0)
			{
			d += deltaE;
			deltaE += 2;
			deltaSE += 2;
			}
		else
			{
			d += deltaSE;
			deltaE += 2;
			deltaSE += 4;

			Dest.FillLineGray(xDest - x, yDest - y, 1 + 2 * x, wColor, byOpacity);
			Dest.FillLineGray(xDest - x, yDest + y, 1 + 2 * x, wColor, byOpacity);

			y--;
			}

		x++;

		if (y >= x)
			{
			Dest.FillLineGray(xDest - y, yDest - x, 1 + 2 * y, wColor, byOpacity);
			Dest.FillLineGray(xDest - y, yDest + x, 1 + 2 * y, wColor, byOpacity);
			}
		}
	}

void DrawFilledCircleTrans (CG16bitImage &Dest, int xDest, int yDest, int iRadius, WORD wColor, DWORD byOpacity)

//	DrawFilledCircleTrans
//
//	Draws an transparent filled circle

	{
	//	Deal with edge-conditions

	if (iRadius <= 0)
		{
		Dest.DrawPixelTrans(xDest, yDest, wColor, (BYTE)byOpacity);
		return;
		}
	else if (byOpacity == 255)
		{
		DrawFilledCircle(Dest, xDest, yDest, iRadius, wColor);
		return;
		}

	//	Initialize some stuff

	int x = 0;
	int y = iRadius;
	int d = 1 - iRadius;
	int deltaE = 3;
	int deltaSE = -2 * iRadius + 5;

	Dest.FillLineTrans(xDest - iRadius, yDest, 1 + 2 * iRadius, wColor, byOpacity);

	//	Loop

	while (y > x)
		{
		if (d < 0)
			{
			d += deltaE;
			deltaE += 2;
			deltaSE += 2;
			}
		else
			{
			d += deltaSE;
			deltaE += 2;
			deltaSE += 4;

			Dest.FillLineTrans(xDest - x, yDest - y, 1 + 2 * x, wColor, byOpacity);
			Dest.FillLineTrans(xDest - x, yDest + y, 1 + 2 * x, wColor, byOpacity);

			y--;
			}

		x++;

		if (y >= x)
			{
			Dest.FillLineTrans(xDest - y, yDest - x, 1 + 2 * y, wColor, byOpacity);
			Dest.FillLineTrans(xDest - y, yDest + x, 1 + 2 * y, wColor, byOpacity);
			}
		}
	}

//	DrawGlowRing --------------------------------------------------------------

struct SGlowRingLineCtx
	{
	CG16bitImage *pDest;
	int xDest;
	int yDest;
	int iRadius;
	int iRingThickness;
	WORD *wColor;
	DWORD *byOpacity;

	DWORD *dwRed;
	DWORD *dwGreen;
	DWORD *dwBlue;
	};

void DrawGlowRingLine (const SGlowRingLineCtx &Ctx, int x, int y)
	{
	int xStart = Ctx.xDest - x;
	int xEnd = Ctx.xDest + x + 1;
	const RECT &rcClip = Ctx.pDest->GetClipRect();

	if (xEnd <= rcClip.left || xStart >= rcClip.right)
		return;

	//	See which lines we need to paint

	int yLine = Ctx.yDest - y;
	bool bPaintTop = (yLine >= rcClip.top && yLine < rcClip.bottom);
	WORD *pCenterTop = Ctx.pDest->GetRowStart(yLine) + Ctx.xDest;

	yLine = Ctx.yDest + y;
	bool bPaintBottom = ((y > 0) && (yLine >= rcClip.top && yLine < rcClip.bottom));
	WORD *pCenterBottom = Ctx.pDest->GetRowStart(yLine) + Ctx.xDest;

	//	Compute radius increment

	int iRadius = y;
	int d = -y;
	int deltaE = 3;
	int deltaSE = -2 * y + 1;

	//	Loop

	int xPos = 0;

	//	This will skip the center pixel in the circle (avoids a divide by
	//	zero in the inner loop).

	if (y == 0)
		{
		xPos = 1;
		d += deltaSE;
		deltaE += 2;
		iRadius++;
		}

	//	Blt the line 

	while (xPos <= x)
		{
		//	Figure out the radius of the pixel at this location

		if (d < 0)
			{
			d += deltaE;
			deltaE += 2;
			deltaSE += 2;
			}
		else
			{
			d += deltaSE;
			deltaE += 2;
			//  deltaSE += 0;
			iRadius++;
			}

		//	Compute the index into the ramp based on radius and ring thickness
		//	(If we're outside the ramp, then continue)

		int iIndex = Ctx.iRadius - iRadius;
		if (iIndex >= Ctx.iRingThickness || iIndex < 0)
			{
			xPos++;
			continue;
			}

		//	Compute the transparency

		DWORD dwOpacity = Ctx.byOpacity[iIndex];

		//	Optimize opaque painting

		if (dwOpacity >= 255)
			{
			WORD wColor = Ctx.wColor[iIndex];

			if (Ctx.xDest - xPos < rcClip.right && Ctx.xDest - xPos >= rcClip.left)
				{
				if (bPaintTop)
					*(pCenterTop - xPos) = wColor;

				if (bPaintBottom)
					*(pCenterBottom - xPos) = wColor;
				}

			if (xPos > 0 && Ctx.xDest + xPos < rcClip.right && Ctx.xDest + xPos >= rcClip.left)
				{
				if (bPaintTop)
					*(pCenterTop + xPos) = wColor;

				if (bPaintBottom)
					*(pCenterBottom + xPos) = wColor;
				}
			}
		else if (dwOpacity == 0)
			;
		else
			{
			DWORD dwRed = Ctx.dwRed[iIndex];
			DWORD dwGreen = Ctx.dwGreen[iIndex];
			DWORD dwBlue = Ctx.dwBlue[iIndex];

			//	Draw transparent

#define DRAW_PIXEL(pos)	\
				{ \
				DWORD dwDest = (DWORD)*(pos);	\
				DWORD dwRedDest = (dwDest >> 11) & 0x1f;	\
				DWORD dwGreenDest = (dwDest >> 5) & 0x3f;	\
				DWORD dwBlueDest = dwDest & 0x1f;	\
				\
				*(pos) = (WORD)((dwOpacity * (dwBlue - dwBlueDest) >> 8) + dwBlueDest |	\
						((dwOpacity * (dwGreen - dwGreenDest) >> 8) + dwGreenDest) << 5 |	\
						((dwOpacity * (dwRed - dwRedDest) >> 8) + dwRedDest) << 11);	\
				}

			//	Paint

			if (Ctx.xDest - xPos < rcClip.right && Ctx.xDest - xPos >= rcClip.left)
				{
				if (bPaintTop)
					DRAW_PIXEL(pCenterTop - xPos);

				if (bPaintBottom)
					DRAW_PIXEL(pCenterBottom - xPos);
				}

			if (xPos > 0 && Ctx.xDest + xPos < rcClip.right && Ctx.xDest + xPos >= rcClip.left)
				{
				if (bPaintTop)
					DRAW_PIXEL(pCenterTop + xPos);

				if (bPaintBottom)
					DRAW_PIXEL(pCenterBottom + xPos);
				}
#undef DRAW_PIXEL
			}

		xPos++;
		}
	}

void DrawGlowRing (CG16bitImage &Dest,
				   int xDest,
				   int yDest,
				   int iRadius,
				   int iRingThickness,
				   WORD *wColorRamp,
				   DWORD *byOpacityRamp)

//	DrawGlowRing
//
//	Draws a glowing ring

	{
	int i;

	//	Deal with edge-conditions

	if (iRingThickness <= 0 || wColorRamp == NULL || byOpacityRamp == NULL || iRadius <= 0)
		return;

	//	Initialize some stuff

	int x = 0;
	int y = iRadius;
	int d = 1 - iRadius;
	int deltaE = 3;
	int deltaSE = -2 * iRadius + 5;

	//	Prepare struct

	SGlowRingLineCtx Ctx;
	Ctx.pDest = &Dest;
	Ctx.xDest = xDest;
	Ctx.yDest = yDest;
	Ctx.iRadius = iRadius;
	Ctx.iRingThickness = iRingThickness;
	Ctx.wColor = wColorRamp;
	Ctx.byOpacity = byOpacityRamp;

	//	Decompose the color ramp into RGB

	Ctx.dwRed = new DWORD [iRingThickness];
	Ctx.dwGreen = new DWORD [iRingThickness];
	Ctx.dwBlue = new DWORD [iRingThickness];
	for (i = 0; i < iRingThickness; i++)
		{
		Ctx.dwRed[i] = (wColorRamp[i] >> 11) & 0x1f;
		Ctx.dwGreen[i] = (wColorRamp[i] >> 5) & 0x3f;
		Ctx.dwBlue[i] = (wColorRamp[i]) & 0x1f;
		}

	//	Draw central line

	DrawGlowRingLine(Ctx, iRadius, 0);

	//	Draw lines above and below the center

	int iLastDraw = -1;
	while (y > x)
		{
		if (d < 0)
			{
			d += deltaE;
			deltaE += 2;
			deltaSE += 2;
			}
		else
			{
			d += deltaSE;
			deltaE += 2;
			deltaSE += 4;

			//	Draw lines

			DrawGlowRingLine(Ctx, x, y);
			iLastDraw = y;

			//	Next

			y--;
			}

		x++;

		//	Draw lines

		if (x != iLastDraw)
			DrawGlowRingLine(Ctx, y, x);
		}

	//	Done

	delete [] Ctx.dwRed;
	delete [] Ctx.dwGreen;
	delete [] Ctx.dwBlue;
	}
