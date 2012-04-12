//	PaintUtilities.cpp
//
//	Paint Utility classes

#include "PreComp.h"


void ComputeLightningPoints (int iCount, Metric *pxPoint, Metric *pyPoint, Metric rChaos)

//	ComputeLightningPoints
//
//	Computes points for lightning using a simple fractal algorithm. We assume
//	that pxPoint[0],pyPoint[0] is the starting point and pxPoint[iCount-1],pyPoint[iCount-1]
//	is the ending point.

	{
	ASSERT(iCount > 2);

	//	Half the delta

	Metric dx2 = (pxPoint[iCount-1] - pxPoint[0]) / 2;
	Metric dy2 = (pyPoint[iCount-1] - pyPoint[0]) / 2;

	//	Center point

	Metric xCenter = pxPoint[0] + dx2;
	Metric yCenter = pyPoint[0] + dy2;

	//	Fractal offset

	Metric rOffset = (mathRandom(-100, 100) / 100.0) * rChaos;

	//	Index of middle of array

	int iMiddle = iCount / 2;

	pxPoint[iMiddle] = xCenter + dy2 * rOffset;
	pyPoint[iMiddle] = yCenter - dx2 * rOffset;

	//	Recurse

	if (iMiddle > 1)
		ComputeLightningPoints(iMiddle+1, pxPoint, pyPoint, rChaos);

	if (iCount - iMiddle > 2)
		ComputeLightningPoints(iCount - iMiddle, pxPoint + iMiddle, pyPoint + iMiddle, rChaos);
	}

void DrawItemTypeIcon (CG16bitImage &Dest, int x, int y, CItemType *pType)

//	DrawItemTypeIcon
//
//	Draws the item type icon at the given position

	{
	const CObjectImageArray &Image = pType->GetImage();
	if (!Image.IsEmpty())
		{
		RECT rcImage = Image.GetImageRect();

		Dest.ColorTransBlt(rcImage.left,
				rcImage.top,
				RectWidth(rcImage),
				RectHeight(rcImage),
				255,
				Image.GetImage(),
				x,
				y);
		}
	}

void DrawLightning (CG16bitImage &Dest,
					int xFrom, int yFrom,
					int xTo, int yTo,
					COLORREF wColor,
					int iPoints,
					Metric rChaos)

//	DrawLightning
//
//	Draw a lightning line

	{
	ASSERT(iPoints >= 0);

	Metric *pxPos = new Metric [iPoints];
	Metric *pyPos = new Metric [iPoints];

	pxPos[0] = xFrom;
	pyPos[0] = yFrom;
	pxPos[iPoints-1] = xTo;
	pyPos[iPoints-1] = yTo;

	ComputeLightningPoints(iPoints, pxPos, pyPos, rChaos);

	//	Draw lightning

	for (int i = 0; i < iPoints-1; i++)
		{
		Dest.DrawLine((int)pxPos[i], (int)pyPos[i],
				(int)pxPos[i+1], (int)pyPos[i+1],
				1,
				wColor);
		}

	//	Done

	delete [] pxPos;
	delete [] pyPos;
	}

void DrawParticle (CG16bitImage &Dest,
				   int x, int y,
				   COLORREF wColor,
				   int iSize,
				   DWORD byOpacity)

//	DrawParticle
//
//	Draws a single particle

	{
	DWORD byOpacity2 = byOpacity / 2;

	switch (iSize)
		{
		case 0:
			Dest.DrawPixelTrans(x, y, wColor, (BYTE)byOpacity);
			break;

		case 1:
			Dest.DrawPixelTrans(x, y, wColor, (BYTE)byOpacity);
			Dest.DrawPixelTrans(x + 1, y, wColor, (BYTE)byOpacity2);
			Dest.DrawPixelTrans(x, y + 1, wColor, (BYTE)byOpacity2);
			break;

		case 2:
			Dest.DrawPixelTrans(x, y, wColor, (BYTE)byOpacity);
			Dest.DrawPixelTrans(x + 1, y, wColor, (BYTE)byOpacity2);
			Dest.DrawPixelTrans(x, y + 1, wColor, (BYTE)byOpacity2);
			Dest.DrawPixelTrans(x - 1, y, wColor, (BYTE)byOpacity2);
			Dest.DrawPixelTrans(x, y - 1, wColor, (BYTE)byOpacity2);
			break;

		case 3:
			Dest.DrawPixelTrans(x, y, wColor, (BYTE)byOpacity);
			Dest.DrawPixelTrans(x + 1, y, wColor, (BYTE)byOpacity);
			Dest.DrawPixelTrans(x, y + 1, wColor, (BYTE)byOpacity);
			Dest.DrawPixelTrans(x - 1, y, wColor, (BYTE)byOpacity);
			Dest.DrawPixelTrans(x, y - 1, wColor, (BYTE)byOpacity);
			Dest.DrawPixelTrans(x + 1, y + 1, wColor, (BYTE)byOpacity2);
			Dest.DrawPixelTrans(x + 1, y - 1, wColor, (BYTE)byOpacity2);
			Dest.DrawPixelTrans(x - 1, y + 1, wColor, (BYTE)byOpacity2);
			Dest.DrawPixelTrans(x - 1, y - 1, wColor, (BYTE)byOpacity2);
			break;

		case 4:
		default:
			{
			Dest.FillTrans(x - 1,
					y - 1,
					3,
					3,
					wColor,
					(BYTE)byOpacity);

			Dest.DrawPixelTrans(x + 2, y, wColor, (BYTE)byOpacity2);
			Dest.DrawPixelTrans(x, y + 2, wColor, (BYTE)byOpacity2);
			Dest.DrawPixelTrans(x - 2, y, wColor, (BYTE)byOpacity2);
			Dest.DrawPixelTrans(x, y - 2, wColor, (BYTE)byOpacity2);
			}
		}
	}
