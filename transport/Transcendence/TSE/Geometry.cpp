//	Geometry.cpp
//
//	Basic geometric functions

#include "PreComp.h"

#include "math.h"

const CVector NullVector = CVector();

static Metric g_Cosine[360];
static Metric g_Sine[360];
static bool g_bTrigInit = false;

//	CVector -------------------------------------------------------------------

void CVector::GenerateOrthogonals (const CVector &vNormal, Metric *retvPara, Metric *retvPerp) const

//	GenerateOrthogonals
//
//	Generates two orthogonal vectors that would generate this vector
//	where one orthogonal is parallel to the given normal.
//
//	vNormal must be a unit vector.

	{
	*retvPara = Dot(vNormal);
	*retvPerp = Dot(vNormal.Perpendicular());
	}

CVector CVector::Rotate (int iAngle) const

//	Rotate
//
//	Returns the vector rotated by the given angle

	{
	return CVector(x * g_Cosine[iAngle % 360] - y * g_Sine[iAngle % 360],
			x * g_Sine[iAngle % 360] + y * g_Cosine[iAngle % 360]);
	}

//	Functions -----------------------------------------------------------------

Metric CalcDistanceToPath (const CVector &Pos,
						   const CVector &Path1,
						   const CVector &Path2,
						   CVector *retvNearestPoint,
						   CVector *retvAway)

//	CalcDistanceToPath
//
//	Returns the distance from Pos to the path from Path1 to Path2.

	{
	//	Create a unit vector from Path1 to Path2

	CVector vPath;
	vPath = Path2 - Path1;
	vPath = vPath.Normal();

	//	Create a vector perpendicular to the path

	CVector vPerp = vPath.Perpendicular();

	//	Create a vector from the point to one of the endpoints

	CVector vPoint = Path1 - Pos;

	//	Project this vector on to the perpendicular

	CVector vToLine = vPoint.Dot(vPerp) * vPerp;

	//	Compute the point on the line that is nearest Pos

	CVector vIntersect = Pos + vToLine;

	//	See if the point is between the two path endpoints

	bool bBetween = false;
	if (Path1.GetX() < Path2.GetX())
		bBetween = vIntersect.GetX() > Path1.GetX() && vIntersect.GetX() < Path2.GetX();
	else
		bBetween = vIntersect.GetX() > Path2.GetX() && vIntersect.GetX() < Path1.GetX();

	if (bBetween)
		{
		if (Path1.GetY() < Path2.GetY())
			bBetween = vIntersect.GetY() > Path1.GetY() && vIntersect.GetY() < Path2.GetY();
		else
			bBetween = vIntersect.GetY() > Path2.GetY() && vIntersect.GetY() < Path1.GetY();
		}

	//	If we're between, then the nearest distance is the distance to the path

	if (bBetween)
		{
		if (retvNearestPoint)
			*retvNearestPoint = vIntersect;
		if (retvAway)
			*retvAway = vToLine.Normal();
		return vToLine.Length();
		}

	//	Otherwise, the nearest distance is the nearest distance to the two endpoints

	else
		{
		CVector vDist1 = Path1 - Pos;
		CVector vDist2 = Path2 - Pos;
		Metric rLen1 = vDist1.Length();
		Metric rLen2 = vDist2.Length();

		if (rLen1 < rLen2)
			{
			if (retvNearestPoint)
				*retvNearestPoint = Path1;
			if (retvAway)
				*retvAway = vDist1.Normal();
			return rLen1;
			}
		else
			{
			if (retvNearestPoint)
				*retvNearestPoint = Path2;
			if (retvAway)
				*retvAway = vDist2.Normal();
			return rLen2;
			}
		}
	}

Metric CalcInterceptTime (const CVector &vTarget, const CVector &vTargetVel, Metric rMissileSpeed, Metric *retrRange)

//	CalcInterceptTime
//
//	Returns the time that it would take to intercept a target
//	at vTarget, moving with velocity vTargetVel, with
//	a missile of speed rMissileSpeed. Returns < 0.0 if the missile cannot
//	intercept the target.
//
//	The formula for interception is:
//
//			A +- B sqrt(C)
//	t	=	--------------
//				  D
//
//	Where	A = B rVi
//			B = 2 rRange
//			C = rMissileSpeed^2 - rVj^2
//			D = 2 (C - rVi^2)

	{
	Metric rRange = vTarget.Length();
	CVector vPosNormal = vTarget / rRange;

	if (retrRange)
		*retrRange = rRange;

	//	Compute the orthogonals of the velocity along the position vector

	Metric rVi, rVj;
	vTargetVel.GenerateOrthogonals(vPosNormal, &rVi, &rVj);

	//	Figure out the inside of the square root. If this value is negative
	//	then we don't have an interception course.

	Metric C = rMissileSpeed * rMissileSpeed - rVj * rVj;
	if (C < 0.0)
		return -1.0;

	//	Figure out the denominator. If this value is 0 then we don't
	//	have an interception course.

	Metric D = 2 * (C - rVi * rVi);
	if (D == 0.0)
		return -1.0;

	//	Compute A and B

	Metric B = 2 * rRange;
	Metric A = B * rVi;

	//	Compute both roots

	Metric Z = B * sqrt(C);
	Metric R1 = (A + Z) / D;
	Metric R2 = (A - Z) / D;

	//	If the first root is positive then return it

	if (R1 > 0.0)
		return R1;

	//	Otherwise we return the second root, which may or may not
	//	be positive

	return R2;
	}

void InitTrig (void)

//	InitTrig
//
//	Initializes sine and cosine tables

	{
	if (!g_bTrigInit)
		{
		for (int i = 0; i < 360; i++)
			{
			Metric rRadian = 2 * g_Pi * i / 360;

			g_Sine[i] = sin(rRadian);
			g_Cosine[i] = cos(rRadian);
			}

		g_bTrigInit = true;
		}

#if 0
	for (int i = 0; i < 360; i++)
		{
		CVector vTest = PolarToVector(i, 100);

		kernelDebugLogMessage("Angle %d = (%d,%d) = %d", 
				i,
				(int)vTest.GetX(), (int)vTest.GetY(),
				VectorToPolar(vTest, NULL));
		}
#endif
	}

bool IntersectRect(const CVector &vUR1, const CVector &vLL1,
				   const CVector &vUR2, const CVector &vLL2)

//	IntersectRect
//
//	Returns TRUE if the two rects intersect

	{
	return (vUR1.GetX() > vLL2.GetX()
			&& vLL1.GetX() < vUR2.GetX()
			&& vUR1.GetY() > vLL2.GetY()
			&& vLL1.GetY() < vUR2.GetY());
	}

void IntPolarToVector (int iAngle, Metric rRadius, int *iox, int *ioy)

//	IntPolarToVector
//
//	PolarToVector using integers

	{
	*iox = (int)(rRadius * g_Cosine[iAngle % 360]);
	*ioy = (int)(rRadius * g_Sine[iAngle % 360]);
	}

CVector PolarToVector (int iAngle, Metric rRadius)

//	PolarToVector
//
//	Creates a vector from polar coordinates. iAngle is an angle from
//	0-359 with 0 pointing to the right and going counter-clockwise.
//	rRadius is a magnitude.

	{
	return CVector(rRadius * g_Cosine[iAngle % 360], rRadius * g_Sine[iAngle % 360]);
	}

CVector TileToVector (int x, int y)

//	TileToVector
//
//	Returns the coordinates of the center of the given tile

	{
	ASSERT(seaScale == 1);

	int xyHalfSize = (seaArraySize * seaArraySize) / 2;
	return CVector(
			(x - xyHalfSize + 0.5) * seaTileSize * g_KlicksPerPixel,
			-((y - xyHalfSize + 0.5) * seaTileSize * g_KlicksPerPixel)
			);
	}

int VectorToPolar (const CVector &vP, Metric *retrRadius)

//	VectorToPolar
//
//	Converts from a vector to polar coordinates (see PolarToVector)

	{
	int iAngle;
	Metric rRadius;
	Metric rSqrRadius = vP.Dot(vP);

	//	If we are at the origin then the angle is undefined

	if (rSqrRadius == 0.0)
		{
		iAngle = 0;
		rRadius = 0.0;
		}
	else
		{
		rRadius = sqrt(rSqrRadius);
		if (vP.GetX() >= 0.0)
			iAngle = (((int)(180 * asin(vP.GetY() / rRadius) / g_Pi)) + 360) % 360;
		else
			iAngle = 180 - ((int)(180 * asin(vP.GetY() / rRadius) / g_Pi));
		}

	//	Done

	if (retrRadius)
		*retrRadius = rRadius;

	return iAngle;
	}

Metric VectorToPolarRadians (const CVector &vP, Metric *retrRadius)

//	VectorToPolarRadians
//
//	Converts from a vector to polar coordinates (see PolarToVector)

	{
	Metric rAngle;
	Metric rRadius;
	Metric rSqrRadius = vP.Dot(vP);

	//	If we are at the origin then the angle is undefined

	if (rSqrRadius == 0.0)
		{
		rAngle = 0.0;
		rRadius = 0.0;
		}
	else
		{
		rRadius = sqrt(rSqrRadius);
		if (vP.GetX() >= 0.0)
			rAngle = asin(vP.GetY() / rRadius);
		else
			rAngle = g_Pi - asin(vP.GetY() / rRadius);
		}

	//	Done

	if (retrRadius)
		*retrRadius = rRadius;

	return rAngle;
	}

void VectorToTile (const CVector &vPos, int *retx, int *rety)

//	VectorToTile
//
//	Converts from a vector to space environment tile coordinates

	{
	//	This algorithm is designed for only two levels; change
	//	if seaScale changes.
	ASSERT(seaScale == 1);

	*retx = (int)(((vPos.GetX() / g_KlicksPerPixel) / seaTileSize) + (seaArraySize * seaArraySize / 2));
	*rety = (int)(((-vPos.GetY() / g_KlicksPerPixel) / seaTileSize) + (seaArraySize * seaArraySize / 2));
	}

//	XForm ---------------------------------------------------------------------

XForm::XForm (void)
	{
	}

XForm::XForm (XFormType type)
	{
	ASSERT(type == xformIdentity);

	m_Xform[0][0] = 1.0;
	m_Xform[0][1] = 0.0;
	m_Xform[0][2] = 0.0;

	m_Xform[1][0] = 0.0;
	m_Xform[1][1] = 1.0;
	m_Xform[1][2] = 0.0;

	m_Xform[2][0] = 0.0;
	m_Xform[2][1] = 0.0;
	m_Xform[2][2] = 1.0;
	}

XForm::XForm (XFormType type, Metric rX, Metric rY)
	{
	switch (type)
		{
		case xformTranslate:
			m_Xform[0][0] = 1.0;
			m_Xform[0][1] = 0.0;
			m_Xform[0][2] = 0.0;

			m_Xform[1][0] = 0.0;
			m_Xform[1][1] = 1.0;
			m_Xform[1][2] = 0.0;

			m_Xform[2][0] = rX;
			m_Xform[2][1] = rY;
			m_Xform[2][2] = 1.0;
			break;

		case xformScale:
			m_Xform[0][0] = rX;
			m_Xform[0][1] = 0.0;
			m_Xform[0][2] = 0.0;

			m_Xform[1][0] = 0.0;
			m_Xform[1][1] = rY;
			m_Xform[1][2] = 0.0;

			m_Xform[2][0] = 0.0;
			m_Xform[2][1] = 0.0;
			m_Xform[2][2] = 1.0;
			break;

		default:
			ASSERT(false);
		}
	}

XForm::XForm (XFormType type, int iAngle)
	{
	switch (type)
		{
		case xformRotate:
			{
			Metric rCos = cos(g_Pi * (Metric)iAngle / 180.0);
			Metric rSin = sin(g_Pi * (Metric)iAngle / 180.0);

			m_Xform[0][0] = rCos;
			m_Xform[0][1] = rSin;
			m_Xform[0][2] = 0.0;

			m_Xform[1][0] = -rSin;
			m_Xform[1][1] = rCos;
			m_Xform[1][2] = 0.0;

			m_Xform[2][0] = 0.0;
			m_Xform[2][1] = 0.0;
			m_Xform[2][2] = 1.0;
			break;
			}

		default:
			ASSERT(false);
		}
	}

void XForm::Transform (Metric x, Metric y, Metric *retx, Metric *rety)
	{
	*retx = x * m_Xform[0][0] + y * m_Xform[1][0] + m_Xform[2][0];
	*rety = x * m_Xform[0][1] + y * m_Xform[1][1] + m_Xform[2][1];
	}

const XForm operator* (const XForm &op1, const XForm &op2)
	{
	XForm Result;

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			Result.m_Xform[i][j] = 
					  op1.m_Xform[i][0] * op2.m_Xform[0][j]
					+ op1.m_Xform[i][1] * op2.m_Xform[1][j]
					+ op1.m_Xform[i][2] * op2.m_Xform[2][j];

	return Result;
	}

//	ViewportTransform ---------------------------------------------------------

ViewportTransform::ViewportTransform (const CVector &vCenter, Metric xScale, Metric yScale, int xCenter, int yCenter)
	{
	m_xScale = xScale;
	m_yScale = yScale;

	//	Note: we use this manual transformation so that we don't get a 
	//	round-off error when scaling for klicks-per-pixel. We scale first
	//	and then substract for position because we want to keep the relative
	//	position of objects constant.

	m_xCenterTrans = (int)(vCenter.GetX() / xScale) - xCenter;
	m_yCenterTrans = (int)(vCenter.GetY() / yScale) + yCenter;
	}

ViewportTransform::ViewportTransform (const CVector &vCenter, Metric rScale, int xCenter, int yCenter)
	{
	m_xScale = rScale;
	m_yScale = rScale;

	//	Note: we use this manual transformation so that we don't get a 
	//	round-off error when scaling for klicks-per-pixel. We scale first
	//	and then substract for position because we want to keep the relative
	//	position of objects constant.

	m_xCenterTrans = (int)(vCenter.GetX() / rScale) - xCenter;
	m_yCenterTrans = (int)(vCenter.GetY() / rScale) + yCenter;
	}

void ViewportTransform::Transform (const CVector &vP, int *retx, int *rety) const
	{
	*retx = (int)(vP.GetX() / m_xScale) - m_xCenterTrans;
	*rety = m_yCenterTrans - (int)(vP.GetY() / m_yScale);
	}

