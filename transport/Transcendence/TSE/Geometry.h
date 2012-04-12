//	Geometry.h
//
//	Transcendence Space Engine geometry

#ifndef INCL_TSE_GEOMETRY
#define INCL_TSE_GEOMETRY

typedef double Metric;

//	2d vector class

class CVector
	{
	public:
		CVector (void) : x(0.0), y(0.0) { }
		CVector (Metric ix, Metric iy) : x(ix), y(iy) { }

		inline bool operator == (const CVector &vA) const { return (x == vA.x && y == vA.y); }

		inline Metric Dot (const CVector &vA) const { return x * vA.x + y * vA.y; }
		inline const Metric &GetX (void) const { return x; }
		inline const Metric &GetY (void) const { return y; }
		inline bool IsNull (void) const { return (x == 0.0 && y == 0.0); }
		inline Metric Length (void) const { return sqrt(x * x + y * y); }
		inline Metric Length2 (void) const { return (x * x + y * y); }
		inline Metric Longest (void) const
			{
			Metric ax = (x < 0.0 ? -x : x);
			Metric ay = (y < 0.0 ? -y : y);
			return (ax > ay ? ax : ay);
			}
		inline CVector Normal (void) const 
			{
			Metric rLength = Length();
			if (rLength == 0.0)
				return CVector();
			else
				return CVector(x / rLength, y / rLength);
			}
		inline CVector Normal (Metric *retrLength) const
			{
			*retrLength = Length();
			if (*retrLength == 0.0)
				return CVector();
			else
				return CVector(x / *retrLength, y / *retrLength);
			}
		void GenerateOrthogonals (const CVector &vNormal, Metric *retvPara, Metric *retvPerp) const;
		inline CVector Perpendicular (void) const { return CVector(-y, x); }
		inline CVector Reflect (void) const { return CVector(-x, -y); }
		CVector Rotate (int iAngle) const;
		inline void SetX (Metric NewX) { x = NewX; }
		inline void SetY (Metric NewY) { y = NewY; }

	private:

		Metric x;
		Metric y;
	};

extern const CVector NullVector;

//	Vector-Vector operations
inline const CVector operator+ (const CVector &op1, const CVector &op2) { return CVector(op1.GetX() + op2.GetX(), op1.GetY() + op2.GetY()); }
inline const CVector operator- (const CVector &op1, const CVector &op2) { return CVector(op1.GetX() - op2.GetX(), op1.GetY() - op2.GetY()); }
inline const CVector operator* (const CVector &op1, const CVector &op2) { return CVector(op1.GetX() * op2.GetX(), op1.GetY() * op2.GetY()); }
inline const CVector operator- (const CVector &op) { return CVector(-op.GetX(), -op.GetY()); }

//	Vector-scalar operations
inline const CVector operator* (const CVector &op1, const Metric op2) { return CVector(op1.GetX() * op2, op1.GetY() * op2); }
inline const CVector operator* (const Metric op2, const CVector &op1) { return CVector(op1.GetX() * op2, op1.GetY() * op2); }
inline const CVector operator/ (const CVector &op1, const Metric op2) { return CVector(op1.GetX() / op2, op1.GetY() / op2); }

//	Transform class

enum XFormType
	{
	xformIdentity,

	xformTranslate,
	xformScale,
	xformRotate,
	};

class XForm
	{
	public:
		XForm (XFormType type);
		XForm (XFormType type, Metric rX, Metric rY);
		XForm (XFormType type, int iAngle);

		void Transform (Metric x, Metric y, Metric *retx, Metric *rety);

	private:
		XForm (void);

		Metric m_Xform[3][3];

	friend const XForm operator* (const XForm &op1, const XForm &op2);
	};

const XForm operator* (const XForm &op1, const XForm &op2);

class ViewportTransform
	{
	public:
		ViewportTransform (void) : m_xScale(1.0), m_yScale(1.0), m_xCenterTrans(0), m_yCenterTrans(0) { }
		ViewportTransform (const CVector &vCenter, Metric xScale, Metric yScale, int xCenter, int yCenter);
		ViewportTransform (const CVector &vCenter, Metric rScale, int xCenter, int yCenter);
		void Transform (const CVector &vP, int *retx, int *rety) const;

	private:
		Metric m_xScale;
		Metric m_yScale;
		int m_xCenterTrans;
		int m_yCenterTrans;
	};

//	Utilities

Metric CalcDistanceToPath (const CVector &Pos, 
						   const CVector &Path1, 
						   const CVector &Path2, 
						   CVector *retvNearestPoint = NULL,
						   CVector *retvAway = NULL);
Metric CalcInterceptTime (const CVector &vTarget, const CVector &vTargetVel, Metric rMissileSpeed, Metric *retrRange = NULL);
CVector PolarToVector (int iAngle, Metric rRadius);
CVector TileToVector (int x, int y);
int VectorToPolar (const CVector &vP, Metric *retrRadius = NULL);
Metric VectorToPolarRadians (const CVector &vP, Metric *retrRadius = NULL);
void VectorToTile (const CVector &vPos, int *retx, int *rety);
void InitTrig (void);
bool IntersectRect(const CVector &vUR1, const CVector &vLL1,
							const CVector &vUR2, const CVector &vLL2);
void IntPolarToVector (int iAngle, Metric rRadius, int *iox, int *ioy);

#endif
