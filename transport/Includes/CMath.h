#ifndef INCL_CMATH
#define INCL_CMATH

const double g_Pi = 3.14159265358979323846;			//	Pi

inline int Absolute(int iValue) { return (iValue < 0 ? -iValue : iValue); }
inline double Absolute(double rValue) { return (rValue < 0.0 ? -rValue : rValue); }
inline int AlignUp (int iValue, int iGranularity) { return ((iValue + (iGranularity - 1)) / iGranularity) * iGranularity; }
int mathNearestPowerOf2 (int x);
int mathPower (int x, int n);
int mathRandom (int iFrom, int iTo);
int mathSeededRandom (int iSeed, int iFrom, int iTo);
int mathSqrt (int x);
inline double mathDegreesToRadians (double rDegrees) { return g_Pi * rDegrees / 180.0; }

#endif

