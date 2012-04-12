//	CExternalEntityTable.cpp
//
//	CExternalEntityTable class

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"

#include "CArray.h"
#include "CSymbolTable.h"
#include "CObjectArray.h"
#include "CStringArray.h"

#include "XMLUtil.h"

CExternalEntityTable::CExternalEntityTable (void) :
		m_Entities(TRUE, FALSE)

//	CExternalEntityTable constructor

	{
	}

void CExternalEntityTable::AddTable (CSymbolTable &Table)

//	AddTable
//
//	Adds all entities in the given table

	{
	int i;

	for (i = 0; i < Table.GetCount(); i++)
		{
		CString *pNewValue = new CString(*(CString *)Table.GetValue(i));
		m_Entities.AddEntry(Table.GetKey(i), pNewValue);
		}
	}

CString CExternalEntityTable::ResolveExternalEntity (const CString &sName, bool *retbFound)

//	ResolveExternalEntity
//
//	Resolves the entity

	{
	CString *pValue;
	if (m_Entities.Lookup(sName, (CObject **)&pValue) != NOERROR)
		{
		if (retbFound) *retbFound = false;
		return sName;
		}

	if (retbFound) *retbFound = true;
	return *pValue;
	}
