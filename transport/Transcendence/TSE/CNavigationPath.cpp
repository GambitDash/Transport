//	CNavigationPath.cpp
//
//	CNavigationPath class

#include "PreComp.h"


const int MAX_PATH_RECURSION =					4;
const Metric MIN_SPEED =						(0.001 * LIGHT_SPEED);
const Metric MIN_SPEED2 =						(MIN_SPEED * MIN_SPEED);
const Metric MAX_SAFE_DIST =					(60.0 * LIGHT_SECOND);
const Metric MAX_SAFE_DIST2 =					(MAX_SAFE_DIST * MAX_SAFE_DIST);
const Metric AVOID_DIST =						(MAX_SAFE_DIST + (10.0 * LIGHT_SECOND));

CNavigationPath::CNavigationPath (void) :
		m_dwID(0),
		m_pSovereign(NULL),
		m_iStartIndex(0),
		m_iEndIndex(0),
		m_iSuccesses(0),
		m_iFailures(0),
		m_iWaypointCount(0),
		m_Waypoints(NULL)

//	CNavigationPath constructor

	{
	}

CNavigationPath::~CNavigationPath (void)

//	CNavigationPath destructor

	{
	if (m_Waypoints)
		delete [] m_Waypoints;
	}

int CNavigationPath::ComputePath (CSystem *pSystem, CSovereign *pSovereign, const CVector &vFrom, const CVector &vTo, int iDepth, CVector **retpPoints)

//	ComputePath
//
//	This is a recursive function that returns a set of points that define a safe path
//	between the two given points.
//
//	We return an allocated array of CVectors in retpPoints

	{
	int i;

	//	If we're very close to our destination, then the best option is a direct path
	//	If the path is clear, then a direct path is also the best option.

	CSpaceObject *pEnemy;
	CVector vAway;
	if (iDepth >= MAX_PATH_RECURSION
			|| ((vTo - vFrom).Length2() <= MAX_SAFE_DIST2)
			|| PathIsClear(pSystem, pSovereign, vFrom, vTo, &pEnemy, &vAway))
		{
		(*retpPoints) = new CVector;
		(*retpPoints)[0] = vTo;
		return 1;
		}

	//	Otherwise, we deflect the path at the enemy base and recurse for the
	//	two path segments.

	else
		{
		//	Compute the mid-point

		CVector vMidPoint = pEnemy->GetPos() + (AVOID_DIST * vAway);

		//	Recurse

		CVector *pLeftPoints;
		int iLeftCount = ComputePath(pSystem, pSovereign, vFrom, vMidPoint, iDepth+1, &pLeftPoints);

		CVector *pRightPoints;
		int iRightCount = ComputePath(pSystem, pSovereign, vMidPoint, vTo, iDepth+1, &pRightPoints);

		//	Compose the two paths together

		int iCount = iLeftCount + iRightCount;
		ASSERT(iCount > 0);
		(*retpPoints) = new CVector [iCount];

		int iPos = 0;
		for (i = 0; i < iLeftCount; i++)
			(*retpPoints)[iPos++] = pLeftPoints[i];

		for (i = 0; i < iRightCount; i++)
			(*retpPoints)[iPos++] = pRightPoints[i];

		delete [] pLeftPoints;
		delete [] pRightPoints;

		return iCount;
		}
	}

Metric CNavigationPath::ComputePathLength (CSystem *pSystem) const

//	ComputePathLength
//
//	Returns the length of the path in Klicks

	{
	int i;
	Metric rDist = 0.0;
	CVector vPos = m_vStart;

	for (i = 0; i < m_iWaypointCount; i++)
		{
		rDist += (m_Waypoints[i] - vPos).Length();
		vPos = m_Waypoints[i];
		}

	return rDist;
	}

CVector CNavigationPath::ComputePointOnPath (CSystem *pSystem, Metric rDist) const

//	ComputePointOnPath
//
//	Returns the point on the path a given distance from the start

	{
	CVector vPos = m_vStart;
	int iWaypoint = 0;

	while (rDist > 0.0 && iWaypoint < m_iWaypointCount)
		{
		CVector vLeg = m_Waypoints[iWaypoint] - vPos;

		Metric rLength;
		CVector vNormal = vLeg.Normal(&rLength);

		if (rLength > rDist)
			return vPos + (vNormal * rDist);
		else
			{
			rDist -= rLength;
			vPos = m_Waypoints[iWaypoint];
			iWaypoint++;
			}
		}

	return vPos;
	}

void CNavigationPath::Create (CSystem *pSystem, CSovereign *pSovereign, CSpaceObject *pStart, CSpaceObject *pEnd, CNavigationPath **retpPath)

//	Create
//
//	Create a path from pStart to pEnd that avoids enemies of pSovereign

	{
	ASSERT(pStart);
	ASSERT(pEnd);

	CVector vStart = pStart->GetPos();
	CVector vEnd = pEnd->GetPos();

	CNavigationPath *pNewPath = new CNavigationPath;
	pNewPath->m_dwID = g_pUniverse->CreateGlobalID();
	pNewPath->m_pSovereign = pSovereign;
	pNewPath->m_iStartIndex = pStart->GetID();
	pNewPath->m_iEndIndex = pEnd->GetID();

	//	Compute the path

	pNewPath->m_vStart = vStart;
	pNewPath->m_iWaypointCount = ComputePath(pSystem, pSovereign, vStart, vEnd, 0, &pNewPath->m_Waypoints);
	ASSERT(pNewPath->m_iWaypointCount > 0);

	//	Done

	*retpPath = pNewPath;
	}

void CNavigationPath::CreateFromStream (SLoadCtx &Ctx, CNavigationPath **retpNavPath)

//	CreateFromStream
//
//	Load a new nav path from a stream
//
//	DWORD		m_dwID
//	DWORD		sovereign UNID
//	DWORD		m_iStartIndex
//	DWORD		m_iEndIndex
//	DWORD		m_iSuccesses
//	DWORD		m_iFailures
//
//	CVector		m_vStart
//	DWORD		m_iWaypointCount
//	CVector		Waypoints

	{
	CNavigationPath *pPath = new CNavigationPath;

	Ctx.pStream->Read((char *)&pPath->m_dwID, sizeof(DWORD));
	Ctx.pSystem->ReadSovereignRefFromStream(Ctx, &pPath->m_pSovereign);
	Ctx.pStream->Read((char *)&pPath->m_iStartIndex, sizeof(DWORD));
	Ctx.pStream->Read((char *)&pPath->m_iEndIndex, sizeof(DWORD));
	Ctx.pStream->Read((char *)&pPath->m_iSuccesses, sizeof(DWORD));
	Ctx.pStream->Read((char *)&pPath->m_iFailures, sizeof(DWORD));

	if (Ctx.dwVersion >= 13)
		Ctx.pStream->Read((char *)&pPath->m_vStart, sizeof(CVector));
	
	DWORD dwCount;
	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	pPath->m_iWaypointCount = (int)dwCount;

	if (dwCount)
		{
		pPath->m_Waypoints = new CVector [dwCount];
		Ctx.pStream->Read((char *)pPath->m_Waypoints, sizeof(CVector) * dwCount);
		}

	*retpNavPath = pPath;
	}

CString CNavigationPath::DebugDescribe (CSpaceObject *pObj, CNavigationPath *pNavPath)

//	DebugDescribe
//
//	Outputs debug info about the path

	{
	if (pNavPath == NULL)
		return CONSTLIT("none");

	//	Get the system

	CSystem *pSystem;
	try
		{
		pSystem = pObj->GetSystem();
		}
	catch (...)
		{
		return strPatternSubst(CONSTLIT("invalid system"));
		}

	//	Get the indeces for the objects

	int iFrom;
	int iTo;
	try
		{
		iFrom = pNavPath->m_iStartIndex;
		iTo = pNavPath->m_iEndIndex;
		}
	catch (...)
		{
		return strPatternSubst(CONSTLIT("%x [invalid]"), (DWORD)pNavPath);
		}

	//	Get the objects

	CSpaceObject *pFrom;
	CSpaceObject *pTo;
	try
		{
		pFrom = pSystem->GetObject(iFrom);
		pTo = pSystem->GetObject(iTo);
		}
	catch (...)
		{
		return strPatternSubst(CONSTLIT("invalid objects: %d, %d"), iFrom, iTo);
		}

	//	Return the object names

	try
		{
		return strPatternSubst(CONSTLIT("FROM: %s TO: %s"),
				CSpaceObject::DebugDescribe(pFrom).GetASCIIZPointer(),
				CSpaceObject::DebugDescribe(pTo).GetASCIIZPointer());
		}
	catch (...)
		{
		return strPatternSubst(CONSTLIT("CObject::DebugDescribe failed"));
		}
	}

CVector CNavigationPath::GetNavPoint (int iIndex) const

//	GetNavPoint
//
//	Return the nav point at the given index

	{
	iIndex = Min(iIndex, m_iWaypointCount - 1);
	return m_Waypoints[iIndex];
	}

bool CNavigationPath::Matches (CSovereign *pSovereign, CSpaceObject *pStart, CSpaceObject *pEnd)

//	Matches
//
//	Returns TRUE if this path matches the request

	{
	if (pSovereign && m_pSovereign && pSovereign != m_pSovereign)
		return false;

	if (pStart->GetID() != m_iStartIndex)
		return false;

	if (pEnd->GetID() != m_iEndIndex)
		return false;

	return true;
	}

bool CNavigationPath::PathIsClear (CSystem *pSystem,
								   CSovereign *pSovereign,
								   const CVector &vFrom, 
								   const CVector &vTo, 
								   CSpaceObject **retpEnemy, 
								   CVector *retvAway)

//	PathIsClear
//
//	Returns TRUE if the path from vFrom to vTo is free from enemy stations.
//	If FALSE, retpEnemy is initialized with the enemy that is blocking the
//	path and retvAway is a unit vector away from the enemy that avoids it

	{
	int i;

	//	Loop over all objects in the system

	CSpaceObject *pNearestEnemy = NULL;
	Metric rNearestDist = MAX_SAFE_DIST;
	CVector vNearestAway;

	for (i = 0; i < pSystem->GetObjectCount(); i++)
		{
		CSpaceObject *pObj = pSystem->GetObject(i);
		CSovereign *pObjSovereign;

		if (pObj
				&& (pObj->GetScale() == scaleStructure 
					|| ((pObj->GetScale() == scaleShip) && (pObj->GetVel().Length2() < MIN_SPEED2)))
				&& (pObjSovereign = pObj->GetSovereign())
				&& (pObjSovereign->IsEnemy(pSovereign))
				&& pObj->CanAttack())
			{
			CVector vAway;
			Metric rDist = CalcDistanceToPath(pObj->GetPos(), vFrom, vTo, NULL, &vAway);
			if (rDist < rNearestDist)
				{
				rNearestDist = rDist;
				pNearestEnemy = pObj;
				vNearestAway = vAway;
				}
			}
		}

	//	If we found a threatening object, return it

	if (pNearestEnemy)
		{
		if (retpEnemy)
			*retpEnemy = pNearestEnemy;
		if (retvAway)
			*retvAway = vNearestAway;
		return false;
		}

	//	Otherwise, the path is OK

	return true;
	}

void CNavigationPath::WriteToStream (CSystem *pSystem, IWriteStream *pStream) const

//	WriteToStream
//
//	Save navigation path
//
//	DWORD		m_dwID
//	DWORD		sovereign UNID
//	DWORD		m_iStartIndex
//	DWORD		m_iEndIndex
//	DWORD		m_iSuccesses
//	DWORD		m_iFailures
//
//	CVector		m_vStart
//	DWORD		m_iWaypointCount
//	CVector		Waypoints

	{
	pStream->Write((char *)&m_dwID, sizeof(DWORD));
	pSystem->WriteSovereignRefToStream(m_pSovereign, pStream);
	pStream->Write((char *)&m_iStartIndex, sizeof(DWORD));
	pStream->Write((char *)&m_iEndIndex, sizeof(DWORD));

	pStream->Write((char *)&m_iSuccesses, sizeof(DWORD));
	pStream->Write((char *)&m_iFailures, sizeof(DWORD));

	pStream->Write((char *)&m_vStart, sizeof(CVector));
	pStream->Write((char *)&m_iWaypointCount, sizeof(DWORD));
	if (m_iWaypointCount)
		pStream->Write((char *)m_Waypoints, sizeof(CVector) * m_iWaypointCount);
	}
