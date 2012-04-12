//	CParticleArray.cpp
//
//	CParticleArray object

#include "PreComp.h"

const int FIXED_POINT =							256;
const COLORREF FLAME_CORE_COLOR =					CG16bitImage::RGBValue(255,241,230);
const COLORREF FLAME_MIDDLE_COLOR =					CG16bitImage::RGBValue(248,200,12);
const COLORREF FLAME_OUTER_COLOR =					CG16bitImage::RGBValue(189,30,0);

#define PAINT_GASEOUS_PARTICLE(Dest,x,y,iWidth,wColor,iFade,iFade2)	\
	{	\
	switch (iWidth)	\
		{	\
		case 0:	\
			Dest.DrawPixelTrans((x), (y), (wColor), (iFade));	\
			break;	\
\
		case 1:	\
			Dest.DrawPixelTrans((x), (y), (wColor), (iFade));	\
			Dest.DrawPixelTrans((x) + 1, (y), (wColor), (iFade2));	\
			Dest.DrawPixelTrans((x), (y) + 1, (wColor), (iFade2));	\
			break;	\
\
		case 2:	\
			Dest.DrawPixelTrans((x), (y), (wColor), (iFade));	\
			Dest.DrawPixelTrans((x) + 1, (y), (wColor), (iFade2));	\
			Dest.DrawPixelTrans((x), (y) + 1, (wColor), (iFade2));	\
			Dest.DrawPixelTrans((x) - 1, (y), (wColor), (iFade2));	\
			Dest.DrawPixelTrans((x), (y) - 1, (wColor), (iFade2));	\
			break;	\
\
		case 3:	\
			Dest.DrawPixelTrans((x), (y), (wColor), (iFade));	\
			Dest.DrawPixelTrans((x) + 1, (y), (wColor), (iFade));	\
			Dest.DrawPixelTrans((x), (y) + 1, (wColor), (iFade));	\
			Dest.DrawPixelTrans((x) - 1, (y), (wColor), (iFade));	\
			Dest.DrawPixelTrans((x), (y) - 1, (wColor), (iFade));	\
			Dest.DrawPixelTrans((x) + 1, (y) + 1, (wColor), (iFade2));	\
			Dest.DrawPixelTrans((x) + 1, (y) - 1, (wColor), (iFade2));	\
			Dest.DrawPixelTrans((x) - 1, (y) + 1, (wColor), (iFade2));	\
			Dest.DrawPixelTrans((x) - 1, (y) - 1, (wColor), (iFade2));	\
			break;	\
\
		default:	\
			if (CG16bitImage::IsGrayscaleValue(wColor))	\
				DrawFilledCircleGray(Dest, (x), (y), ((iWidth) + 1) / 2, (wColor), (iFade));	\
			else	\
				DrawFilledCircleTrans(Dest, (x), (y), ((iWidth) + 1) / 2, (wColor), (iFade));	\
			break;	\
		}	\
	}

CParticleArray::CParticleArray (void) :
		m_iCount(0),
		m_pArray(NULL),
		m_iLastAdded(-1)

//	CParticleArray constructor

	{
	m_rcBounds.left = 0;
	m_rcBounds.top = 0;
	m_rcBounds.right = 0;
	m_rcBounds.bottom = 0;
	}

CParticleArray::~CParticleArray (void)

//	CParticleArray destructor

	{
	if (m_pArray)
		delete [] m_pArray;
	}

void CParticleArray::AddParticle (const CVector &vPos, const CVector &vVel, int iLifeLeft, int iRotation, DWORD dwData)

//	AddParticle
//
//	Adds a new particle to the array

	{
	if (m_iCount == 0)
		return;

	//	Look for an open slot

	int iSlot = (m_iLastAdded + 1) % m_iCount;
	while (iSlot != m_iLastAdded && m_pArray[iSlot].fAlive)
		iSlot = (iSlot + 1) % m_iCount;

	//	If we're out of room, can't add

	if (iSlot == m_iLastAdded)
		return;

	//	Add the particle at the slot

	SParticle *pParticle = &m_pArray[iSlot];
	PosToXY(vPos, &pParticle->x, &pParticle->y);
	PosToXY(vVel, &pParticle->xVel, &pParticle->yVel);
	pParticle->iLifeLeft = iLifeLeft;
	pParticle->iDestiny = mathRandom(1, 360);
	pParticle->iRotation = iRotation;
	pParticle->dwData = dwData;

	pParticle->fAlive = true;

	m_iLastAdded = iSlot;
	}

void CParticleArray::CleanUp (void)

//	CleanUp
//
//	Deletes the array

	{
	if (m_pArray)
		{
		delete [] m_pArray;
		m_pArray = NULL;
		}

	m_iCount = 0;
	}

int CParticleArray::HitTest (CSpaceObject *pObj,
							 const CVector &vOldPos,
							 const CVector &vNewPos,
							 CVector *retvHitPos)

//	HitTest
//
//	Intersects the particles with the given object. Any particle
//	that hits the object is destroyed.
//
//	Returns the number of particles that hit

	{
	int iHitCount = 0;
	CVector vHitPos;

	//	Compute the half-way pos

	CVector vDiff = vNewPos - vOldPos;
	CVector vHalfPos = vOldPos + (0.5 * vDiff);

	//	Loop over all particles

	SParticle *pParticle = m_pArray;
	SParticle *pEnd = pParticle + m_iCount;

	while (pParticle < pEnd)
		{
		if (pParticle->fAlive)
			{
			//	Compute the position of the particle relative to center

			CVector vOffset = XYToPos(pParticle->x, pParticle->y);

			//	First check to see if the new position hit the object

			if (pObj->PointInObject(vNewPos + vOffset))
				{
				iHitCount++;
				vHitPos = vHitPos + vOffset;
				pParticle->fAlive = false;
				}

			//	Otherwise, check the half-way point

			else if (pObj->PointInObject(vHalfPos + vOffset))
				{
				iHitCount++;
				vHitPos = vHitPos + vOffset - (0.5 * vDiff);
				pParticle->fAlive = false;
				}
			}

		//	Next

		pParticle++;
		}

	//	The hit pos is the average of all particles that hit

	if (retvHitPos && iHitCount > 0)
		*retvHitPos = vNewPos + (vHitPos / iHitCount);

	//	Done

	return iHitCount;
	}

void CParticleArray::Init (int iMaxCount)

//	Init
//
//	Initializes the particle array

	{
	CleanUp();

	if (iMaxCount > 0)
		{
		m_pArray = new SParticle [iMaxCount];
		memset(m_pArray, 0, sizeof(SParticle) * iMaxCount);

		m_iCount = iMaxCount;
		}
	}


void CParticleArray::Paint (CG16bitImage &Dest,
							int xPos,
							int yPos,
							SViewportPaintCtx &Ctx,
							SParticlePaintDesc &Desc)

//	Paint
//
//	Paint all particles

	{
	//	Paint based on style

	switch (Desc.iStyle)
		{
		case paintFlame:
			{
			int iCore = 1;
			int iFlame = Desc.iMaxLifetime / 2;
			int iSmoke = 3 * Desc.iMaxLifetime / 4;
			int iSmokeBrightness = 80;

			PaintFireAndSmoke(Dest, 
					xPos, yPos, 
					Ctx, 
					Desc.iMaxLifetime,
					Desc.iMinWidth,
					Desc.iMaxWidth, 
					iCore, 
					iFlame, 
					iSmoke, 
					iSmokeBrightness);
			break;
			}

		case paintImage:
			PaintImage(Dest, xPos, yPos, Ctx, Desc);
			break;

		case paintLine:
			PaintLine(Dest, xPos, yPos, Ctx, Desc.wPrimaryColor);
			break;

		case paintSmoke:
			{
			int iCore = 1;
			int iFlame = Desc.iMaxLifetime / 6;
			int iSmoke = Desc.iMaxLifetime / 4;
			int iSmokeBrightness = 120;

			PaintFireAndSmoke(Dest, 
					xPos, yPos, 
					Ctx, 
					Desc.iMaxLifetime,
					Desc.iMinWidth,
					Desc.iMaxWidth, 
					iCore, 
					iFlame, 
					iSmoke, 
					iSmokeBrightness);
			break;
			}

		default:
			PaintGaseous(Dest,
					xPos, yPos,
					Ctx,
					Desc.iMaxLifetime,
					Desc.iMinWidth,
					Desc.iMaxWidth,
					Desc.wPrimaryColor,
					Desc.wSecondaryColor);
			break;
		}
	}

void CParticleArray::Paint (CG16bitImage &Dest,
							int xPos,
							int yPos,
							SViewportPaintCtx &Ctx,
							IEffectPainter *pPainter)

//	Paint
//
//	Paint using a painter for each particle

	{
	int iSavedDestiny = Ctx.iDestiny;
	int iSavedRotation = Ctx.iRotation;

	SParticle *pParticle = m_pArray;
	SParticle *pEnd = pParticle + m_iCount;

	while (pParticle < pEnd)
		{
		if (pParticle->fAlive)
			{
			//	Compute the position of the particle

			int x = xPos + pParticle->x / FIXED_POINT;
			int y = yPos + pParticle->y / FIXED_POINT;

			//	Paint the particle

			Ctx.iDestiny = pParticle->iDestiny;
			Ctx.iRotation = pParticle->iRotation;
			pPainter->Paint(Dest, x, y, Ctx);
			}

		//	Next

		pParticle++;
		}

	Ctx.iDestiny = iSavedDestiny;
	Ctx.iRotation = iSavedRotation;
	}

void CParticleArray::PaintFireAndSmoke (CG16bitImage &Dest, 
										int xPos, 
										int yPos, 
										SViewportPaintCtx &Ctx, 
										int iLifetime,
										int iMinWidth,
										int iMaxWidth,
										int iCore,
										int iFlame,
										int iSmoke,
										int iSmokeBrightness)

//	PaintFireAndSmoke
//
//	Paints each particle as fire/smoke particle. 
//
//	iLifetime is the point at which the particle fades completely
//	iWidth is the maximum width of the particle

	{
	iCore = Max(iCore, 1);
	iFlame = Max(iFlame, iCore + 1);
	iSmoke = Max(iSmoke, iFlame + 1);

	iMinWidth = Max(1, iMinWidth);
	iMaxWidth = Max(iMinWidth, iMaxWidth);
	int iWidthRange = (iMaxWidth - iMinWidth) + 1;

	SParticle *pParticle = m_pArray;
	SParticle *pEnd = pParticle + m_iCount;

	while (pParticle < pEnd)
		{
		if (pParticle->fAlive)
			{
			int iLifeLeft = Min(pParticle->iLifeLeft, iLifetime);
			int iAge = iLifetime - iLifeLeft;

			//	Compute properties of the particle based on its life

			COLORREF wColor = 0;
			int iFade = 0;
			int iFade2 = 0;
			int iWidth = 0;

			if (iLifetime > 0)
				{
				//	Particle fades out over time

				iFade = Max(20, Min(255, (255 * iLifeLeft / iLifetime)));
				iFade2 = iFade / 2;

				//	Particle grows over time

				iWidth = iMinWidth + (iWidthRange * iAge / iLifetime);

				//	Smoke color

				int iDarkness = Min(255, iSmokeBrightness + (2 * (pParticle->iDestiny % 25)));
				COLORREF wSmokeColor = CG16bitImage::GrayscaleValue(iDarkness);

				//	Some particles are gray

				COLORREF wFadeColor;
				if ((pParticle->iDestiny % 4) != 0)
					wFadeColor = FLAME_OUTER_COLOR;
				else
					wFadeColor = wSmokeColor;

				//	Particle color changes over time

				if (iAge <= iCore)
					wColor = CG16bitImage::FadeColor(FLAME_CORE_COLOR,
							FLAME_MIDDLE_COLOR,
							100 * iAge / iCore);
				else if (iAge <= iFlame)
					wColor = CG16bitImage::FadeColor(FLAME_MIDDLE_COLOR,
							wFadeColor,
							100 * (iAge - iCore) / (iFlame - iCore));
				else if (iAge <= iSmoke)
					wColor = CG16bitImage::FadeColor(wFadeColor,
							wSmokeColor,
							100 * (iAge - iFlame) / (iSmoke - iFlame));
				else
					wColor = wSmokeColor;
				}

			//	Compute the position of the particle

			int x = xPos + pParticle->x / FIXED_POINT;
			int y = yPos + pParticle->y / FIXED_POINT;

			//	Paint the particle

			PAINT_GASEOUS_PARTICLE(Dest, x, y, iWidth, wColor, iFade, iFade2);
			}

		//	Next

		pParticle++;
		}
	}

void CParticleArray::PaintGaseous (CG16bitImage &Dest,
								   int xPos,
								   int yPos,
								   SViewportPaintCtx &Ctx,
								   int iMaxLifetime,
								   int iMinWidth,
								   int iMaxWidth,
								   COLORREF wPrimaryColor,
								   COLORREF wSecondaryColor)

//	PaintGaseous
//
//	Paints gaseous particles that fade from primary color to secondary color

	{
	iMinWidth = Max(1, iMinWidth);
	iMaxWidth = Max(iMinWidth, iMaxWidth);
	int iWidthRange = (iMaxWidth - iMinWidth) + 1;

	SParticle *pParticle = m_pArray;
	SParticle *pEnd = pParticle + m_iCount;

	while (pParticle < pEnd)
		{
		if (pParticle->fAlive)
			{
			int iLifeLeft = Min(pParticle->iLifeLeft, iMaxLifetime);
			int iAge = iMaxLifetime - iLifeLeft;

			//	Compute properties of the particle based on its life

			COLORREF wColor = 0;
			int iFade = 0;
			int iFade2 = 0;
			int iWidth = 0;

			if (iMaxLifetime > 0)
				{
				//	Particle fades out over time

				iFade = Max(20, Min(255, (255 * iLifeLeft / iMaxLifetime)));
				iFade2 = iFade / 2;

				//	Particle grows over time

				iWidth = iMinWidth + (iWidthRange * iAge / iMaxLifetime);

				//	Particle color fades from primary to secondary

				wColor = CG16bitImage::FadeColor(wPrimaryColor, wSecondaryColor, 100 * iAge / iMaxLifetime);
				}

			//	Compute the position of the particle

			int x = xPos + pParticle->x / FIXED_POINT;
			int y = yPos + pParticle->y / FIXED_POINT;

			//	Paint the particle

			PAINT_GASEOUS_PARTICLE(Dest, x, y, iWidth, wColor, iFade, iFade2);
			}

		//	Next

		pParticle++;
		}
	}

void CParticleArray::PaintImage (CG16bitImage &Dest, int xPos, int yPos, SViewportPaintCtx &Ctx, SParticlePaintDesc &Desc)

//	Paint
//
//	Paints particle as an image

	{
	int iRotationFrame = 0;
	if (Desc.bDirectional)
		iRotationFrame = Angle2Direction(Ctx.iRotation, Desc.iVariants);

	SParticle *pParticle = m_pArray;
	SParticle *pEnd = pParticle + m_iCount;

	while (pParticle < pEnd)
		{
		if (pParticle->fAlive)
			{
			//	Figure out the animation frame to paint

			int iTick;
			if (Desc.bRandomStartFrame)
				iTick = Ctx.iTick + pParticle->iDestiny;
			else
				iTick = Ctx.iTick;

			//	Figure out the rotation or variant to paint

			int iFrame = (Desc.bDirectional ? iRotationFrame : (pParticle->iDestiny % Desc.iVariants));

			//	Compute the position of the particle

			int x = xPos + pParticle->x / FIXED_POINT;
			int y = yPos + pParticle->y / FIXED_POINT;

			//	Paint the particle

			Desc.pImage->PaintImage(Dest, x, y, iTick, iFrame);
			}

		//	Next

		pParticle++;
		}

#if 0
			for (i = 0; i < m_iParticleCount; i++)
				if (!m_pParticles[i].IsDestroyed())
					{
					m_pDesc->m_Image.PaintImage(Dest,
							x + (m_pParticles[i].x / PARTICLE_METRIC),
							y - (m_pParticles[i].y / PARTICLE_METRIC),
							m_iTick,
							0);
					}
#endif
	}

void CParticleArray::PaintLine (CG16bitImage &Dest,
								int xPos,
								int yPos,
								SViewportPaintCtx &Ctx,
								COLORREF wPrimaryColor)

//	PaintLine
//
//	Paints particle as a line

	{
	//	Figure out the velocity of our object

	int xVel = 0;
	int yVel = 0;
	if (Ctx.pObj)
		PosToXY(Ctx.pObj->GetVel(), &xVel, &yVel);

	//	Paint all the particles

	SParticle *pParticle = m_pArray;
	SParticle *pEnd = pParticle + m_iCount;

	while (pParticle < pEnd)
		{
		if (pParticle->fAlive)
			{
			//	Compute the position of the particle

			int xFrom = xPos + pParticle->x / FIXED_POINT;
			int yFrom = yPos + pParticle->y / FIXED_POINT;

			int xTo = xFrom - (xVel + pParticle->xVel) / FIXED_POINT;
			int yTo = yFrom - (yVel + pParticle->yVel) / FIXED_POINT;

			//	Paint the particle

			Dest.DrawLine(xFrom, yFrom,
					xTo, yTo,
					1,
					wPrimaryColor);
			}

		//	Next

		pParticle++;
		}
	}

void CParticleArray::PosToXY (const CVector &xy, int *retx, int *rety)

//	PosToXY
//
//	Convert from system coordinates to particle coordinates

	{
	*retx = (int)(FIXED_POINT * xy.GetX() / g_KlicksPerPixel);
	*rety = -(int)(FIXED_POINT * xy.GetY() / g_KlicksPerPixel);
	}

void CParticleArray::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Reads from a stream

	{
	DWORD dwLoad;

	CleanUp();

	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	if (dwLoad > 0x00100000)
		return;

	m_iCount = dwLoad;
	m_pArray = new SParticle [m_iCount];
	Ctx.pStream->Read((char *)m_pArray, sizeof(SParticle) * m_iCount);
	}

void CParticleArray::UpdateMotionLinear (bool *retbAlive)

//	UpdateMotionLinear
//
//	Updates the position of all particles

	{
	int xLeft = 0;
	int xRight = 0;
	int yTop = 0;
	int yBottom = 0;

	bool bAllParticlesDead = true;

	SParticle *pParticle = m_pArray;
	SParticle *pEnd = pParticle + m_iCount;

	while (pParticle < pEnd)
		{
		if (pParticle->fAlive)
			{
			//	Update lifetime

			if (pParticle->iLifeLeft > 0)
				{
				pParticle->iLifeLeft--;

				//	Update position

				pParticle->x += pParticle->xVel;
				pParticle->y += pParticle->yVel;

				//	Update the bounding box

				if (pParticle->x > xRight)
					xRight = pParticle->x;
				else if (pParticle->x < xLeft)
					xLeft = pParticle->x;

				if (pParticle->y > yTop)
					yTop = pParticle->y;
				else if (pParticle->y < yBottom)
					yBottom = pParticle->y;

				bAllParticlesDead = false;
				}

			//	Otherwise, particle is dead

			else
				{
				pParticle->fAlive = false;
				}
			}

		//	Next

		pParticle++;
		}

	//	Set the bounding rect

	m_rcBounds.left = xLeft / FIXED_POINT;
	m_rcBounds.top = yTop / FIXED_POINT;
	m_rcBounds.right = xRight / FIXED_POINT;
	m_rcBounds.bottom = yBottom / FIXED_POINT;

	if (retbAlive)
		*retbAlive = !bAllParticlesDead;
	}

void CParticleArray::WriteToStream (IWriteStream *pStream) const

//	WriteToStream
//
//	Writes the array to a stream
//
//	DWORD			count of particles
//	SParticle[]		array of particles

	{
	pStream->Write((char *)&m_iCount, sizeof(DWORD));
	pStream->Write((char *)m_pArray, sizeof(SParticle) * m_iCount);
	}

CVector CParticleArray::XYToPos (int x, int y)

//	XYToPos
//
//	Converts from particle coordinates to screen coordinates

	{
	return CVector((x * g_KlicksPerPixel / FIXED_POINT), -(y * g_KlicksPerPixel / FIXED_POINT));
	}
