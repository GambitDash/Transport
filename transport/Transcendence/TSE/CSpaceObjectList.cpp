//	CSpaceObjectList.cpp
//
//	CSpaceObjectList class

#include "PreComp.h"

#define ALLOC_GRANULARITY				16

CSpaceObjectList::CSpaceObjectList (void)

//	CSpaceObjectList constructor

	{
	}

CSpaceObjectList::~CSpaceObjectList (void)

//	CSpaceObjectList destructor

	{
	}

void CSpaceObjectList::Add (CSpaceObject *pObj, int *retiIndex)

//	Add
//
//	Add object to the list. If the object is already in the list, we
//	don't add it.

	{
	if (FindObj(pObj))
		return;

	FastAdd(pObj, retiIndex);
	}

void CSpaceObjectList::FastAdd (CSpaceObject *pObj, int *retiIndex)

//	FastAdd
//
//	Adds the object without checking to see if it is already
//	in the list.

	{
	m_pList.push_back(pObj);
	if (retiIndex) *retiIndex = m_pList.size() - 1;
	}

bool CSpaceObjectList::FindObj (CSpaceObject *pObj, int *retiIndex) const

//	FindObj
//
//	Find the object in the list

	{
	std::vector<CSpaceObject *>::const_iterator itr;

	itr = std::find(m_pList.begin(), m_pList.end(), pObj);
	if (itr != m_pList.end())
		{
		if (retiIndex) *retiIndex = itr - m_pList.begin();
		return true;
		}
	return false;
	}

int CSpaceObjectList::GetCount (void) const

//	GetCount
//
//	Returns the number of objects in the list

	{
	return m_pList.size();
	}

void CSpaceObjectList::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Read the list from a stream

	{
	DWORD dwCount;

	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	if (dwCount)
		{
		m_pList.resize(dwCount);
		for (int i = 0; i < (int)dwCount; i++)
			{
			Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_pList[i]);
			}

		assert(dwCount == m_pList.size());
		}
	}

void CSpaceObjectList::Remove (int iIndex)

//	Remove
//
//	Remove the object

	{
	assert(iIndex >= 0); /* YYY Because I didn't want to rebuild right then. */
	assert((unsigned int)iIndex < m_pList.size());
	m_pList.erase(m_pList.begin() + iIndex);
	}

bool CSpaceObjectList::Remove (CSpaceObject *pObj)

//	Remove
//
//	Remove the object

	{
	std::vector<CSpaceObject *>::iterator itr = std::find(m_pList.begin(), m_pList.end(), pObj);
	if (itr != m_pList.end())
		{
		m_pList.erase(itr);
		return true;
		}
	return false;
	}

void CSpaceObjectList::RemoveAll (void)

//	RemoveAll
//
//	Remove all objects

	{
	m_pList.clear();
	}

void CSpaceObjectList::WriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	WriteToStream
//
//	Write list to stream

	{
	int iCount = GetCount();
	pStream->Write((char *)&iCount, sizeof(int));

	for (int i = 0; i < iCount; i++)
		{
		assert(m_pList[i] != NULL);
		pSystem->WriteObjRefToStream(m_pList[i], pStream);
		}
	}

