//	Math.cpp
//
//	Integer math package

#include "portage.h"

#include <math.h>

int mathNearestPowerOf2 (int x)

//	mathNearestPowerOf2
//
//	Returns the largest power of 2 that is <= x
//	NOTE: This doesn't work for very large integers because we overflow
//	iResult.

	{
	int iResult;

	iResult = 1;
	while (x > 0)
		{
		x = x >> 1;
		iResult = iResult << 1;
		}

	return iResult >> 1;
	}

int mathPower (int x, int n)

//	mathPower
//
//	Returns x raised to the nth power

	{
    int iResult = 1;

	while (n)
		{
		if (n & 1)
			{
			iResult = iResult * x;
			n = n - 1;
			}
		else
			{
			x = x * x;
			n = n / 2;
			}
		}

	return iResult;
	}

int mathRandom (int iFrom, int iTo)

//	mathRandom
//
//	Returns a random number between iFrom and iTo (inclusive)

	{
	int iRandom;
	int iRange = abs(iTo - iFrom) + 1;

	if (iRange > RAND_MAX)
		iRandom = ((10000 * rand()) + (rand() % 10000)) % iRange;
	else
		iRandom = rand() % iRange;

	return iRandom + iFrom;
	}

int mathSeededRandom (int iSeed, int iFrom, int iTo)

//	mathSeededRandom
//
//	Returns a random number between iFrom and iTo (inclusive)
//	The same random number is returned for any given value of iSeed

	{
#define rand_with_seed()		(((dwSeed = dwSeed * 214013L + 2531011L) >> 16) & 0x7fff)

	DWORD dwSeed = (DWORD)iSeed;
	int iRandom;
	int iRange = abs(iTo - iFrom) + 1;

	if (iRange > RAND_MAX)
		iRandom = ((10000 * rand_with_seed()) + (rand_with_seed() % 10000)) % iRange;
	else
		iRandom = rand_with_seed() % iRange;

	return iRandom + iFrom;

#undef rand_with_seed
	}

int mathSqrt (int x)

//	mathSqrt
//
//	Returns the square root of x

	{
	//	For now we use a floating point method
	return (int)sqrt((double)x);
	}

