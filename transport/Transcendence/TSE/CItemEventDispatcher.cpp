//	CItemEventDispatcher.cpp
//
//	CItemEventDispatcher object

#include "PreComp.h"

#define ON_AI_UPDATE_EVENT						CONSTLIT("OnAIUpdate")
#define ON_UPDATE_EVENT							CONSTLIT("OnUpdate")

#define STR_G_ITEM								CONSTLIT("gItem")
#define STR_G_SOURCE							CONSTLIT("gSource")

CItemEventDispatcher::CItemEventDispatcher (void) : m_pFirstEntry(NULL)

//	CItemEventDispatcher constructor

	{
	}

CItemEventDispatcher::~CItemEventDispatcher (void)

//	CItemEventDispatcher destructor

	{
	RemoveAll();
	}

CItemEventDispatcher::SEntry *CItemEventDispatcher::AddEntry (void)

//	AddEntry
//
//	Adds a new entry to the beginning of the list

	{
	SEntry *pEntry = new SEntry;
	pEntry->pNext = m_pFirstEntry;
	m_pFirstEntry = pEntry;
	return pEntry;
	}

void CItemEventDispatcher::Init (CSpaceObject *pSource)

//	Init
//
//	Initializes the dispatcher from the item list

	{
	int i;

	RemoveAll();

	CItemListManipulator ItemList(pSource->GetItemList());
	while (ItemList.MoveCursorForward())
		{
		const CItem &Item = ItemList.GetItemAtCursor();
		const CEventHandler &Handlers = Item.GetType()->GetEventHandlers();
		for (i = 0; i < Handlers.GetCount(); i++)
			{
			ItemEventTypes iType;

			ICCItem *pCode;
			CString sEvent = Handlers.GetEvent(i, &pCode);
			if (strEquals(sEvent, ON_AI_UPDATE_EVENT))
				iType = eventOnAIUpdate;
			else if (strEquals(sEvent, ON_UPDATE_EVENT))
				iType = eventOnUpdate;
			else
				iType = eventNone;

			if (iType != eventNone)
				{
				SEntry *pEntry = AddEntry();
				pEntry->iType = iType;
				pEntry->pCode = pCode;
				pEntry->theItem = Item;
				}
			}
		}
	}

void CItemEventDispatcher::FireEventFull (CSpaceObject *pSource, ItemEventTypes iType)

//	FireEventFull
//
//	Fires the given event

	{
	CCodeChain &CC = g_pUniverse->GetCC();

	bool bSavedVars = false;
	ICCItem *pOldSource;
	ICCItem *pOldItem;

	//	Fire event for all items that have it

	SEntry *pEntry = m_pFirstEntry;
	while (pEntry)
		{
		if (pEntry->iType == iType)
			{
			//	Save variable, if necessary

			if (!bSavedVars)
				{
				pOldSource = CC.LookupGlobal(STR_G_SOURCE, &g_pUniverse);
				DefineGlobalSpaceObject(CC, STR_G_SOURCE, pSource);

				pOldItem = CC.LookupGlobal(STR_G_ITEM, &g_pUniverse);
				bSavedVars = true;
				}

			//	Define gItem

			ICCItem *pItem = CreateListFromItem(CC, pEntry->theItem);
			CC.DefineGlobal(STR_G_ITEM, pItem);
			pItem->Discard(&CC);

			//	Run code

			ICCItem *pResult = CC.TopLevel(pEntry->pCode, &g_pUniverse);
			if (pResult->IsError())
				pSource->ReportEventError(strPatternSubst(CONSTLIT("Item %x Event"), pEntry->theItem.GetType()->GetUNID()), pResult);
			pResult->Discard(&CC);
			}

		pEntry = pEntry->pNext;
		}

	//	Restore

	if (bSavedVars)
		{
		CC.DefineGlobal(STR_G_SOURCE, pOldSource);
		pOldSource->Discard(&CC);

		CC.DefineGlobal(STR_G_ITEM, pOldItem);
		pOldItem->Discard(&CC);
		}
	}

void CItemEventDispatcher::RemoveAll (void)

//	RemoveAll
//
//	Remove all entries

	{
	SEntry *pEntry = m_pFirstEntry;
	while (pEntry)
		{
		SEntry *pDelete = pEntry;
		pEntry = pEntry->pNext;
		delete pDelete;
		}

	m_pFirstEntry = NULL;
	}
