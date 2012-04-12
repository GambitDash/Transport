//	CSystemType.cpp
//
//	CSystemType class

#include "PreComp.h"

#define TABLES_TAG								CONSTLIT("Tables")

#define ON_CREATE_EVENT							CONSTLIT("OnCreate")

CSystemType::CSystemType (void) : 
		m_pDesc(NULL)

//	CSystemType constructor

	{
	}

CSystemType::~CSystemType (void)

//	CSystemType destructor

	{
	if (m_pDesc)
		delete m_pDesc;
	}

ALERROR CSystemType::FireOnCreate (CString *retsError)

//	FireOnCreate
//
//	Fire OnCreate event

	{
	ICCItem *pCode;
	if (FindEventHandler(ON_CREATE_EVENT, &pCode))
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
		if (pResult->IsError())
			{
			if (retsError)
				*retsError = strPatternSubst(CONSTLIT("System OnCreate: %s"), pResult->GetStringValue());
			return ERR_FAIL;
			}

		pResult->Discard(&CC);
		}

	return NOERROR;
	}

CXMLElement *CSystemType::GetLocalSystemTables (void)

//	GetLocalSystemTables
//
//	Returns the local tables

	{
	return m_pDesc->GetContentElementByTag(TABLES_TAG);
	}

ALERROR CSystemType::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Create from XML

	{
	m_pDesc = pDesc->OrphanCopy();
	return NOERROR;
	}
