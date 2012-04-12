//	CEventHandler.cpp
//
//	CEventHandler object

#include "PreComp.h"


CEventHandler::CEventHandler (void) :
		m_Handlers(FALSE, FALSE)

//	CEventHandler constructor

	{
	}

CEventHandler::~CEventHandler (void)

//	CEventHandler destructor

	{
	for (int i = 0; i < m_Handlers.GetCount(); i++)
		{
		ICCItem *pItem = (ICCItem *)m_Handlers.GetValue(i);
		pItem->Discard(&g_pUniverse->GetCC());
		}
	}

void CEventHandler::AddEvent (const CString &sEvent, ICCItem *pCode)

//	AddEvent
//
//	Adds an event

	{
	m_Handlers.AddEntry(sEvent, pCode);
	}

ALERROR CEventHandler::AddEvent (const CString &sEvent, const CString &sCode, CString *retsError)

//	AddEvent
//
//	Adds an event

	{
	ICCItem *pCode = g_pUniverse->GetCC().Link(sCode, 0, NULL);
	if (pCode->IsError())
		{
		if (retsError)
			*retsError = pCode->GetStringValue();
		return ERR_FAIL;
		}

	m_Handlers.AddEntry(sEvent, pCode);

	return NOERROR;
	}

bool CEventHandler::FindEvent (const CString &sEvent, ICCItem **retpCode) const

//	FindEvent
//
//	Finds the event handler by name

	{
	return (m_Handlers.Lookup(sEvent, (CObject **)retpCode) == NOERROR);
	}

CString CEventHandler::GetEvent (int iIndex, ICCItem **retpCode) const

//	GetEvent
//
//	Returns the event by index

	{
	if (retpCode)
		*retpCode = (ICCItem *)m_Handlers.GetValue(iIndex);

	return m_Handlers.GetKey(iIndex);
	}

ALERROR CEventHandler::InitFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	InitFromXML
//
//	Load all handlers

	{
	int i;

	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pHandler = pDesc->GetContentElement(i);
		ICCItem *pCode = g_pUniverse->GetCC().Link(pHandler->GetContentText(0), 0, NULL);
		if (pCode->IsError())
			{
			Ctx.sError = pCode->GetStringValue();
			return ERR_FAIL;
			}

		//	If this is an old extension, then make sure the code is not using the
		//	gStation variable, because we no longer support it

		if (Ctx.pExtension && Ctx.pExtension->dwVersion < 2)
			{
			if (g_pUniverse->GetCC().HasIdentifier(pCode, CONSTLIT("gStation")))
				{
				Ctx.sError = CONSTLIT("gStation variable has been deprecated--use gSource instead.");
				return ERR_FAIL;
				}
			}

		//	Done

		m_Handlers.AddEntry(pHandler->GetTag(), pCode);
		}

	return NOERROR;
	}
