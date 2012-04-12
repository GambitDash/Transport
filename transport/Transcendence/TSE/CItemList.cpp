//	CItemList.cpp
//
//	CItemList object

#include "PreComp.h"

const int ALLOC_SIZE = 4;

CItemList::CItemList (void) : m_pList(NULL)

//	CItemList constructor

	{
	}

CItemList::~CItemList (void)

//	CItemList destructor

	{
	DeleteAll();
	}

CItemList &CItemList::operator= (const CItemList &Copy)

//	CItemList assign operator

	{
	DeleteAll();

	if (Copy.m_pList)
		{
		SHeader *pSrcHeader = (SHeader *)Copy.m_pList;

		int iSize = sizeof(SHeader) + sizeof(CItem) * pSrcHeader->iAlloc;
		m_pList = new char [iSize];
		memset(m_pList, 0, iSize);

		SHeader *pDstHeader = (SHeader *)m_pList;
		pDstHeader->iAlloc = pSrcHeader->iAlloc;
		pDstHeader->iCount = pSrcHeader->iCount;

		//	Copy items via constructor

		const CItem *pSrc = Copy.GetItems();
		for (int i = 0; i < pSrcHeader->iCount; i++)
			{
			GetItem(i) = *pSrc;
			pSrc++;
			}
		}

	return *this;
	}

void CItemList::AddItem (const CItem &Item)

//	AddItem
//
//	Adds an item to the list

	{
	//	If the list doesn't exist, allocate it

	if (m_pList == NULL)
		{
		int iSize = sizeof(SHeader) + sizeof(CItem) * ALLOC_SIZE;
		m_pList = new char [iSize];
		memset(m_pList, 0, iSize);

		SHeader *pHeader = (SHeader *)m_pList;
		pHeader->iAlloc = ALLOC_SIZE;
		pHeader->iCount = 0;
		}

	//	Grow the list if necessary

	SHeader *pHeader = (SHeader *)m_pList;
	if (pHeader->iAlloc == pHeader->iCount)
		{
		int iNewAlloc = pHeader->iAlloc + ALLOC_SIZE;
		int iNewSize = sizeof(SHeader) + sizeof(CItem) * iNewAlloc;
		char *pNewList = new char [iNewSize];

		//	No need to go through copy constructor since we are just
		//	transfering memory

		int iOldSize = sizeof(SHeader) + sizeof(CItem) * pHeader->iCount;
		memcpy(pNewList, m_pList, iOldSize);
		memset(pNewList + iOldSize, 0, iNewSize - iOldSize);

		SHeader *pNewHeader = (SHeader *)pNewList;
		pNewHeader->iAlloc = iNewAlloc;

		delete m_pList;
		m_pList = pNewList;
		pHeader = (SHeader *)m_pList;
		}

	//	Add the item at the end

	GetItem(pHeader->iCount) = Item;
	pHeader->iCount++;
	}

void CItemList::DeleteAll (void)

//	DeleteAll
//
//	Delete all items

	{
	if (m_pList)
		{
		SHeader *pHeader = (SHeader *)m_pList;

		for (int i = 0; i < pHeader->iCount; i++)
			GetItem(i) = CItem();

		delete m_pList;
		m_pList = NULL;
		}
	}

void CItemList::DeleteItem (int iIndex)

//	DeleteItem
//
//	Deletes the item at the index position

	{
	if (m_pList)
		{
		SHeader *pHeader = (SHeader *)m_pList;

		ASSERT(iIndex < pHeader->iCount);

		//	Call a destructor on the item going away

		GetItem(iIndex) = CItem();

		//	Move the rest of the slots down (no need for constructors)

		char *pDest = &m_pList[sizeof(SHeader) + sizeof(CItem) * iIndex];
		char *pSrc = &m_pList[sizeof(SHeader) + sizeof(CItem) * (iIndex + 1)];
		char *pSrcEnd = &m_pList[sizeof(SHeader) + sizeof(CItem) * pHeader->iCount];

		while (pSrc < pSrcEnd)
			*pDest++ = *pSrc++;

		//	Zero out the last slot (no need for destructors)

		memset((char *)&m_pList[sizeof(SHeader) + sizeof(CItem) * (pHeader->iCount - 1)], 0, sizeof(CItem));

		pHeader->iCount--;
		}
	}

int CItemList::GetCount (void) const

//	GetCount
//
//	Returns the number of items

	{
	if (m_pList)
		return ((SHeader *)m_pList)->iCount;
	else
		return 0;
	}

CItem &CItemList::GetItem (int iIndex)

//	GetItem
//
//	Returns the item at the given index position

	{
	return *(GetItems() + iIndex);
	}

const CItem &CItemList::GetItem (int iIndex) const

//	GetItem
//
//	Returns the item at the given index position

	{
	return *(GetItems() + iIndex);
	}

void CItemList::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Reads the item list from a stream
//
//	DWORD		Number of items
//	DWORD		Allocation granularity
//	CItem[number of items]

	{
	ASSERT(m_pList == NULL);

	DWORD dwCount, dwAlloc;
	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	Ctx.pStream->Read((char *)&dwAlloc, sizeof(DWORD));

	if (dwCount > 0)
		{
		int iSize = sizeof(SHeader) + sizeof(CItem) * dwAlloc;
		m_pList = new char [iSize];
		memset(m_pList, 0, iSize);

		SHeader *pHeader = (SHeader *)m_pList;
		pHeader->iAlloc = dwAlloc;
		pHeader->iCount = dwCount;
		CItem *pLoad = GetItems();

		for (int i = 0; i < (int)dwCount; i++)
			{
			pLoad->ReadFromStream(Ctx);
			pLoad++;
			}
		}
	}

void CItemList::SortItems (void)

//	SortItems
//
//	Sorts items in order:
//
//	installed/not-installed
//	armor/weapon/device/other

	{
	if (GetCount() == 0)
		return;

	int i;
	CSymbolTable Sort(false, true);

	for (i = 0; i < GetCount(); i++)
		{
		CItem &Item = GetItem(i);
		CItemType *pType = Item.GetType();

		//	All installed items first

		CString sInstalled;
		if (Item.IsInstalled())
			sInstalled = CONSTLIT("0");
		else
			sInstalled = CONSTLIT("1");

		//	Next, sort on category

		CString sCat;
		switch (pType->GetCategory())
			{
			case itemcatWeapon:
			case itemcatLauncher:
				sCat = CONSTLIT("0");
				break;

			case itemcatMissile:
				sCat = CONSTLIT("1");
				break;

			case itemcatShields:
				sCat = CONSTLIT("2");
				break;

			case itemcatReactor:
				sCat = CONSTLIT("3");
				break;

			case itemcatDrive:
				sCat = CONSTLIT("4");
				break;

			case itemcatCargoHold:
				sCat = CONSTLIT("5");
				break;

			case itemcatMiscDevice:
				sCat = CONSTLIT("6");
				break;

			case itemcatArmor:
				sCat = CONSTLIT("7");
				break;

			case itemcatFuel:
			case itemcatUseful:
				sCat = CONSTLIT("8");
				break;

			default:
				sCat = CONSTLIT("9");
			}

		//	Next, sort by install location

		if (Item.IsInstalled())
			sCat.Append(strPatternSubst(CONSTLIT("%03d%08x"), Item.GetInstalled(), Item.GetType()->GetUNID()));
		else
			sCat.Append(CONSTLIT("99900000000"));

		//	Within category, sort by level (highest first)

		sCat.Append(strPatternSubst(CONSTLIT("%02d"), 25 - Item.GetType()->GetApparentLevel()));

		//	Enhanced items before others

		if (Item.IsEnhanced())
			sCat.Append(CONSTLIT("0"));
		else if (Item.IsDamaged())
			sCat.Append(CONSTLIT("2"));
		else
			sCat.Append(CONSTLIT("1"));

		CString sName = pType->GetSortName();
		CString sSort = strPatternSubst(CONSTLIT("%s%s%s%d"), sInstalled.GetASCIIZPointer(), sCat.GetASCIIZPointer(), sName.GetASCIIZPointer(), (i * (int)this) % 0x10000);
		Sort.AddEntry(sSort, (CObject *)i);
		}

	//	Allocate a new list

	SHeader *pSrcHeader = (SHeader *)m_pList;

	int iSize = sizeof(SHeader) + sizeof(CItem) * pSrcHeader->iAlloc;
	int iSizeInUse = sizeof(SHeader) + sizeof(CItem) * pSrcHeader->iCount;
	char *pNewList = new char [iSize];
	memset(pNewList + iSizeInUse, 0, iSize - iSizeInUse);

	SHeader *pDstHeader = (SHeader *)pNewList;
	pDstHeader->iAlloc = pSrcHeader->iAlloc;
	pDstHeader->iCount = pSrcHeader->iCount;

	for (i = 0; i < GetCount(); i++)
		{
		int iOld = (int)Sort.GetValue(i);

		//	Copy memory (no need for constructors)

		memcpy(pNewList + sizeof(SHeader) + i * sizeof(CItem),
				m_pList + sizeof(SHeader) + iOld * sizeof(CItem),
				sizeof(CItem));
		}

	delete m_pList;
	m_pList = pNewList;
	}

void CItemList::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Writes the item list to stream
//
//	DWORD		Number of items
//	DWORD		Allocation granularity
//	CItem[number of items]

	{
	int i;

	if (m_pList)
		{
		SHeader *pHeader = (SHeader *)m_pList;
		pStream->Write((char *)&pHeader->iCount, sizeof(DWORD));
		pStream->Write((char *)&pHeader->iAlloc, sizeof(DWORD));

		for (i = 0; i < GetCount(); i++)
			GetItem(i).WriteToStream(pStream);
		}
	else
		{
		DWORD dwZero = 0;
		pStream->Write((char *)&dwZero, sizeof(DWORD));
		pStream->Write((char *)&dwZero, sizeof(DWORD));
		}
	}
