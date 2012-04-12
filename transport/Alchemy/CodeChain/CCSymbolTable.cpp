//	CCSymbolTable.cpp
//
//	Implements CCSymbolTable class

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"
#include "CMath.h"

#include "CStream.h"
#include "CCCodeChain.h"

static CObjectClass<CCSymbolTable>g_Class(OBJID_CCSYMBOLTABLE, NULL);

CCSymbolTable::CCSymbolTable (void) : ICCAtom(&g_Class),
		m_Symbols(FALSE, FALSE),
		m_pParent(NULL),
		m_bLocalFrame(FALSE)

//	SymbolTable constructor

	{
	}

void CCSymbolTable::AddByOffset (CCodeChain *pCC, int iOffset, ICCItem *pEntry)

//	AddByOffset
//
//	Adds an entry directly

	{
	CObject *pOldEntry;

	m_Symbols.SetValue(iOffset, pEntry->Reference(), &pOldEntry);

	//	Discard old entry

	((ICCItem *)pOldEntry)->Discard(pCC);
	}

ICCItem *CCSymbolTable::AddEntry (CCodeChain *pCC, ICCItem *pKey, ICCItem *pEntry)

//	AddEntry
//
//	Adds an entry to the symbol table and returns
//	True for success.

	{
	ALERROR error;
	ICCItem *pPrevEntry = NULL;

	//	If this is the global symbol table (no parent) then we add the entry
	//	regardless of whether it already exists or not.

	if (m_pParent == NULL)
		{
		CObject *pOldEntry;

		if (m_Symbols.ReplaceEntry(pKey->GetStringValue(), pEntry->Reference(), TRUE, &pOldEntry) != NOERROR)
			return pCC->CreateMemoryError();

		//	If we have a previous entry, decrement its refcount since we're
		//	throwing it away

		pPrevEntry = (ICCItem *)pOldEntry;
		}

	//	Otherwise, if this is a local symbol table, try to replace the symbol
	//	but if the symbol is not found, let our parent handle it

	else
		{
		CObject *pOldEntry;

		error = m_Symbols.ReplaceEntry(pKey->GetStringValue(), pEntry->Reference(), FALSE, &pOldEntry);
		if (error == ERR_NOTFOUND)
			{
			pEntry->Discard(pCC);
			return m_pParent->AddEntry(pCC, pKey, pEntry);
			}
		else if (error != NOERROR)
			return pCC->CreateMemoryError();

		//	Get the previous entry

		pPrevEntry = (ICCItem *)pOldEntry;
		}

	//	Delete old entry

	if (pPrevEntry)
		pPrevEntry->Discard(pCC);

	SetModified();

	return pCC->CreateTrue();
	}

ICCItem *CCSymbolTable::Clone (CCodeChain *pCC)

//	Clone
//
//	Clone this item

	{
	ASSERT(FALSE);
	return pCC->CreateNil();
	}

void CCSymbolTable::DestroyItem (CCodeChain *pCC)

//	DestroyItem
//
//	Destroy this item

	{
	int i;

	//	Release our parent reference

	if (m_pParent)
		m_pParent->Discard(pCC);

	//	Release all the entries

	for (i = 0; i < m_Symbols.GetCount(); i++)
		{
		CObject *pValue = m_Symbols.GetValue(i);
		ICCItem *pItem = (ICCItem *)pValue;

		pItem->Discard(pCC);
		}

	//	Remove all symbols

	m_Symbols.RemoveAll();

	//	Destroy this item

	pCC->DestroySymbolTable(this);
	}

int CCSymbolTable::FindOffset (CCodeChain *pCC, ICCItem *pKey)

//	FindOffset
//
//	Returns the offset of the given variable (or -1 if not found)
//	Callers must be careful about this usage--the offset may
//	change if any new entries are added to the table.

	{
	int iOffset;

	if (m_Symbols.LookupEx(pKey->GetStringValue(), &iOffset) != NOERROR)
		iOffset = -1;

	return iOffset;
	}

int CCSymbolTable::FindValue (ICCItem *pValue)

//	FindValue
//
//	Returns the entry in the symbol table

	{
	int i;

	for (i = 0; i < m_Symbols.GetCount(); i++)
		{
		if ((CObject *)pValue == m_Symbols.GetValue(i))
			return i;
		}

	return -1;
	}

ICCItem *CCSymbolTable::ListSymbols (CCodeChain *pCC)

//	ListSymbols
//
//	Returns a list of all the symbols in the table

	{
	//	If there are no symbols, return Nil

	if (m_Symbols.GetCount() == 0)
		return pCC->CreateNil();

	//	Otherwise, make a list

	else
		{
		int i;
		ICCItem *pResult;
		CCLinkedList *pList;

		pResult = pCC->CreateLinkedList();
		if (pResult->IsError())
			return pResult;

		pList = (CCLinkedList *)pResult;

		for (i = 0; i < m_Symbols.GetCount(); i++)
			{
			ICCItem *pItem;
			CString sKey = m_Symbols.GetKey(i);

			//	Make an item for the symbol

			pItem = pCC->CreateString(sKey);

			//	Add the item to the list

			pList->Append(pCC, pItem, NULL);
			pItem->Discard(pCC);
			}

		return pList;
		}
	}

ICCItem *CCSymbolTable::Lookup (CCodeChain *pCC, ICCItem *pKey)

//	Lookup
//
//	Looks up the key and returns the association. If no
//	Association is found, returns Nil

	{
	return LookupEx(pCC, pKey, NULL);
	}

ICCItem *CCSymbolTable::LookupEx (CCodeChain *pCC, ICCItem *pKey, BOOL *retbFound)

//	LookupEx
//
//	Looks up the key and returns the association. If no
//	Association is found, we ask the parent. If none is found, returns an error

	{
	ALERROR error;
	CObject *pNew;
	ICCItem *pBinding;

	error = m_Symbols.Lookup(pKey->GetStringValue(), &pNew);
	if (error)
		{
		if (error == ERR_NOTFOUND)
			{
			//	If we could not find it in this symbol table, look for
			//	the symbol in the parent

			if (m_pParent)
				return m_pParent->LookupEx(pCC, pKey, retbFound);
			else
				{
				if (retbFound)
					*retbFound = FALSE;

				return pCC->CreateErrorCode(CCRESULT_NOTFOUND);
				}
			}
		else
			return pCC->CreateMemoryError();
		}

	pBinding = dynamic_cast<ICCItem *>(pNew);
	ASSERT(pBinding);

	if (retbFound)
		*retbFound = TRUE;

	return pBinding->Reference();
	}

ICCItem *CCSymbolTable::LookupByOffset (CCodeChain *pCC, int iOffset)

//	LookupByOffset
//
//	Returns the value at the given offset

	{
	CObject *pNew = m_Symbols.GetValue(iOffset);
	ICCItem *pBinding = dynamic_cast<ICCItem *>(pNew);

	if (pBinding)
		return pBinding->Reference();
	else
		return pCC->CreateErrorCode(CCRESULT_NOTFOUND);
	}

CString CCSymbolTable::Print (CCodeChain *pCC)

//	Print
//
//	Render as text

	{
	return LITERAL("[symbol table]");
	}

void CCSymbolTable::Reset (void)

//	Reset
//
//	Reset the internal variables

	{
	m_Symbols.RemoveAll();
	m_pParent = NULL;
	m_bLocalFrame = FALSE;
	}

ICCItem *CCSymbolTable::SimpleLookup (CCodeChain *pCC, ICCItem *pKey, BOOL *retbFound, int *retiOffset)

//	SimpleLookup
//
//	Looks up the key and returns the association. If no
//	Association is found, returns an error

	{
	ALERROR error;
	CObject *pNew;
	ICCItem *pBinding;

	int iOffset;
	
	error = m_Symbols.LookupEx(pKey->GetStringValue(), &iOffset);
	if (error)
		{
		if (error == ERR_NOTFOUND)
			{
			if (retbFound)
				*retbFound = FALSE;

			return pCC->CreateErrorCode(CCRESULT_NOTFOUND);
			}
		else
			return pCC->CreateMemoryError();
		}

	pNew = m_Symbols.GetValue(iOffset);
	pBinding = dynamic_cast<ICCItem *>(pNew);
	ASSERT(pBinding);

	if (retbFound)
		*retbFound = TRUE;

	if (retiOffset)
		*retiOffset = iOffset;

	return pBinding->Reference();
	}

ICCItem *CCSymbolTable::StreamItem (CCodeChain *pCC, IWriteStream *pStream)

//	StreamItem
//
//	Stream the sub-class specific data

	{
	ALERROR error;
	int iCount;
	int i;

	//	Write out the count

	iCount = m_Symbols.GetCount();

	error = pStream->Write((char *)&iCount, sizeof(iCount), NULL);
	if (error)
		return pCC->CreateSystemError(error);

	//	Write out each of the items in the list

	for (i = 0; i < iCount; i++)
		{
		CString sKey = m_Symbols.GetKey(i);
		CObject *pValue = m_Symbols.GetValue(i);
		ICCItem *pItem = (ICCItem *)pValue;
		ICCItem *pKey;
		ICCItem *pError;

		//	Write out the key

		pKey = pCC->CreateString(sKey);
		if (pKey->IsError())
			return pKey;

		pError = pCC->StreamItem(pKey, pStream);
		pKey->Discard(pCC);
		if (pError->IsError())
			return pError;

		pError->Discard(pCC);

		//	Write out the value

		pError = pCC->StreamItem(pItem, pStream);
		if (pError->IsError())
			return pError;

		pError->Discard(pCC);

		//	Note that there is no need to discard pItem
		//	since we did not increase its refcount
		}

	return pCC->CreateTrue();
	}

ICCItem *CCSymbolTable::UnstreamItem (CCodeChain *pCC, IReadStream *pStream)

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
		CString sKey;
		CObject *pOldEntry;

		//	Load the key

		pItem = pCC->UnstreamItem(pStream);
		if (pItem->IsError())
			return pItem;

		sKey = pItem->GetStringValue();
		pItem->Discard(pCC);

		//	Now load the value

		pItem = pCC->UnstreamItem(pStream);

		//	Note that we don't abort in case of an error
		//	because the list might contain errors

		//	Append the item to the symbol table

		if (m_Symbols.ReplaceEntry(sKey, pItem, TRUE, &pOldEntry) != NOERROR)
			return pCC->CreateMemoryError();

		//	No need to discard pItem because we're adding it to the
		//	symbol table.

		//	If we have a previous entry, decrement its refcount since we're
		//	throwing it away

		ASSERT(pOldEntry == NULL);
		}

	//	We are never a local symbol table

	m_bLocalFrame = FALSE;

	return pCC->CreateTrue();
	}
