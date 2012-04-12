//	CCLinkedList.cpp
//
//	Implements CCLinkedList class

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"
#include "CMath.h"

#include "CStream.h"
#include "CCCodeChain.h"

#define MIN_UNINDEXED_LOOKUP					5

static CObjectClass<CCLinkedList>g_Class(OBJID_CCLINKEDLIST, NULL);

CCLinkedList::CCLinkedList (void) : ICCList(&g_Class),
		m_pFirst(NULL),
		m_pLast(NULL),
		m_iCount(0),
		m_pIndex(NULL)

//	CCInteger constructor

	{
	}

CCLinkedList::~CCLinkedList (void)

//	CCLinkedList destructor

	{
	}

void CCLinkedList::Append (CCodeChain *pCC, ICCItem *pItem, ICCItem **retpError)

//	Append
//
//	Appends the item to the list

	{
	CCons *pCons;

	//	Create a new cons

	pCons = pCC->CreateCons();
	if (pCons == NULL)
		{
		if (retpError)
			*retpError = pCC->CreateMemoryError();
		return;
		}

	pCons->m_pItem = pItem->Reference();
	pCons->m_pNext = NULL;

	//	Link it to the rest of the list

	if (m_pLast)
		m_pLast->m_pNext = pCons;
	m_pLast = pCons;

	if (m_pFirst == NULL)
		m_pFirst = pCons;

	m_iCount++;

	//	Discard index since we've changed things

	if (m_pIndex)
		{
		delete [] m_pIndex;
		m_pIndex = NULL;
		}

	//	Done

	if (retpError)
		*retpError = pCC->CreateTrue();
	}

ICCItem *CCLinkedList::Clone (CCodeChain *pCC)

//	Clone
//
//	Returns a copy of the item

	{
	ICCItem *pNew;
	CCLinkedList *pClone;
	CCons *pCons;

	pNew = pCC->CreateLinkedList();
	if (pNew->IsError())
		return pNew;

	pClone = dynamic_cast<CCLinkedList *>(pNew);
	pClone->CloneItem(this);

	//	Copy all the items

	pCons = m_pFirst;
	while (pCons)
		{
		pClone->Append(pCC, pCons->m_pItem, NULL);
		pCons = pCons->m_pNext;
		}

	return pClone;
	}

void CCLinkedList::CreateIndex (void)

//	CreateIndex
//
//	Create an index so that we can do lookups by position

	{
	if (m_pIndex)
		return;

	m_pIndex = new CCons *[m_iCount];
	if (m_pIndex)
		{
		int i;
		CCons *pCons;

		pCons = m_pFirst;
		i = 0;
		while (pCons)
			{
			m_pIndex[i] = pCons;
			i++;
			pCons = pCons->m_pNext;
			}
		}
	}

void CCLinkedList::DestroyItem (CCodeChain *pCC)

//	DestroyItem
//
//	Destroys the item

	{
	CCons *pCons;

	//	Discard all items that we contain

	pCons = m_pFirst;
	while (pCons)
		{
		CCons *pNext = pCons->m_pNext;
		pCons->m_pItem->Discard(pCC);
		pCC->DestroyCons(pCons);
		pCons = pNext;
		}

	//	Free the index

	if (m_pIndex)
		delete [] m_pIndex;

	//	Give the item back

	pCC->DestroyLinkedList(this);
	}

ICCItem *CCLinkedList::Enum (CEvalContext *pCtx, ICCItem *pCode)

//	Enum
//
//	Iterate over all the items in the list and execute pCode

	{
	return pCtx->pCC->CreateNil();
	}

ICCItem *CCLinkedList::GetElement (int iIndex)

//	GetElement
//
//	Returns the nth element in the list. iIndex is 0-based.
//	If iIndex is out of range, we return NULL.

	{
	CCons *pCons;

	//	If iIndex is pretty large and we don't have an
	//	index, then create one

	if (iIndex > MIN_UNINDEXED_LOOKUP && m_pIndex == NULL)
		CreateIndex();

	//	If we've got an index, just look it up

	if (m_pIndex)
		{
		if (iIndex <= 0)
			pCons = m_pFirst;
		else if (iIndex < m_iCount)
			pCons = m_pIndex[iIndex];
		else
			pCons = NULL;
		}

	//	Otherwise we need to iterate

	else
		{
		pCons = m_pFirst;
		while (pCons && iIndex > 0)
			{
			pCons = pCons->m_pNext;
			iIndex--;
			}
		}

	//	Done

	if (pCons)
		return pCons->m_pItem;
	else
		return NULL;
	}

CString CCLinkedList::Print (CCodeChain *pCC)

//	Print
//
//	Returns a text representation of this item

	{
	CCons *pNext = m_pFirst;
	CString sString;

	//	Open paren

	if (IsQuoted())
		sString = LITERAL("'(");
	else
		sString = LITERAL("(");

	//	Items

	while (pNext)
		{
		sString.Append(pNext->m_pItem->Print(pCC));

		pNext = pNext->m_pNext;

		if (pNext)
			sString.Append(LITERAL(" "));
		}

	//	Close paren

	sString.Append(LITERAL(")"));
	return sString;
	}

void CCLinkedList::RemoveElement (CCodeChain *pCC, int iIndex)

//	RemoveElement
//
//	Removes the nth entry

	{
	//	Check the range

	if (iIndex < 0 || iIndex >= m_iCount)
		return;

	//	Look for the cons entry

	CCons *pPrevCons;
	CCons *pCons;

	if (m_pIndex)
		{
		if (iIndex == 0)
			pPrevCons = NULL;
		else
			pPrevCons = m_pIndex[iIndex - 1];
		pCons = m_pIndex[iIndex];
		}
	else
		{
		pPrevCons = NULL;
		pCons = m_pFirst;
		while (pCons && iIndex > 0)
			{
			pPrevCons = pCons;
			pCons = pCons->m_pNext;
			iIndex--;
			}
		}

	//	Relink

	if (pPrevCons)
		pPrevCons->m_pNext = pCons->m_pNext;
	else
		{
		ASSERT(m_pFirst == pCons);
		m_pFirst = pCons->m_pNext;
		}

	if (pCons->m_pNext == NULL)
		{
		ASSERT(m_pLast == pCons);
		m_pLast = pPrevCons;
		}

	//	Count

	m_iCount--;
	ASSERT(m_iCount >= 0);

	//	Discard cons

	pCons->m_pItem->Discard(pCC);
	pCC->DestroyCons(pCons);

	//	Discard index since we've changed things

	if (m_pIndex)
		{
		delete [] m_pIndex;
		m_pIndex = NULL;
		}
	}

void CCLinkedList::ReplaceElement (CCodeChain *pCC, int iIndex, ICCItem *pNewItem)

//	ReplaceElement
//
//	Replaces then nth entry in the list with the given
//	item. If the nth item does not exist, nothing gets
//	added.

	{
	CCons *pCons;

	//	Check the range

	if (iIndex < 0 || iIndex >= m_iCount)
		return;

	//	Look for the cons entry

	if (m_pIndex)
		pCons = m_pIndex[iIndex];
	else
		{
		pCons = m_pFirst;
		while (pCons && iIndex > 0)
			{
			pCons = pCons->m_pNext;
			iIndex--;
			}
		}

	//	Change the link

	pCons->m_pItem->Discard(pCC);
	pCons->m_pItem = pNewItem->Reference();
	}

void CCLinkedList::Reset (void)

//	Reset
//
//	Reset to initial conditions

	{
	ASSERT(m_dwRefCount == 0);
	m_pFirst = NULL;
	m_pLast = NULL;
	m_iCount = 0;
	m_pIndex = NULL;
	}

void CCLinkedList::Shuffle (CCodeChain *pCC)

//	Shuffle
//
//	Shuffles the elements in the list

	{
	if (m_iCount < 2)
		return;

	//	We need an index for this

	CreateIndex();

	//	Fisher-Yates algorithm

	int i = m_iCount - 1;
	while (i > 0)
		{
		int x = mathRandom(0, i);

		CCons *pTemp = m_pIndex[x];
		m_pIndex[x] = m_pIndex[i];
		m_pIndex[i] = pTemp;

		i--;
		}

	//	Fixup all the pointers

	for (i = 0; i < m_iCount - 1; i++)
		m_pIndex[i]->m_pNext = m_pIndex[i + 1];

	m_pIndex[m_iCount - 1]->m_pNext = NULL;

	//	Update first and last

	m_pFirst = m_pIndex[0];
	m_pLast = m_pIndex[m_iCount - 1];
	}

ICCItem *CCLinkedList::StreamItem (CCodeChain *pCC, IWriteStream *pStream)

//	StreamItem
//
//	Stream the sub-class specific data

	{
	ALERROR error;
	CCons *pCons;

	//	Write out the count
	error = pStream->Write((char *)&m_iCount, sizeof(m_iCount), NULL);
	if (error)
		return pCC->CreateSystemError(error);

	//	Write out each of the items in the list

	pCons = m_pFirst;
	while (pCons)
		{
		ICCItem *pError;

		pError = pCC->StreamItem(pCons->m_pItem, pStream);
		if (pError->IsError())
			return pError;

		pError->Discard(pCC);

		pCons = pCons->m_pNext;
		}

	return pCC->CreateTrue();
	}

ICCItem *CCLinkedList::Tail (CCodeChain *pCC)

//	Tail
//
//	Returns a list that includes all items in this list
//	after the head. If there are no more items, it retuns
//	Nil

	{
	CCons *pNext = m_pFirst->m_pNext;

	if (pNext == NULL)
		return pCC->CreateNil();
	else
		{
		ICCItem *pNew;
		CCLinkedList *pTail;

		pNew = pCC->CreateLinkedList();
		if (pNew->IsError())
			return pNew;

		pTail = dynamic_cast<CCLinkedList *>(pNew);

		//	Add all but the first item to the list

		while (pNext)
			{
			pTail->Append(pCC, pNext->m_pItem, NULL);
			pNext = pNext->m_pNext;
			}

		//	Done

		return pTail;
		}
	}

ICCItem *CCLinkedList::UnstreamItem (CCodeChain *pCC, IReadStream *pStream)

//	UnstreamItem
//
//	Unstream the sub-class specific data

	{
	ALERROR error;
	int i, iCount;

	//	Read the count

	error = pStream->Read((char *)&iCount, sizeof(iCount), NULL);
	if (error)
		return pCC->CreateSystemError(error);

	//	Read all the items

	for (i = 0; i < iCount; i++)
		{
		ICCItem *pItem;

		pItem = pCC->UnstreamItem(pStream);

		//	Note that we don't abort in case of an error
		//	because the list might contain errors

		//	Append the item to the list

		Append(pCC, pItem, NULL);
		pItem->Discard(pCC);
		}

	return pCC->CreateTrue();
	}

