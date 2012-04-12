//	CCommunicationsHandler.cpp
//
//	CCommunicationsHandler class

#include "PreComp.h"


#define ON_SHOW_TAG									CONSTLIT("OnShow")
#define CODE_TAG									CONSTLIT("Code")
#define INVOKE_TAG									CONSTLIT("Invoke")

#define NAME_ATTRIB									CONSTLIT("name")
#define KEY_ATTRIB									CONSTLIT("key")

CCommunicationsHandler::CCommunicationsHandler (void) : m_iCount(0),
		m_pMessages(NULL)

//	CCommunicationsHandler constructor

	{
	}

CCommunicationsHandler::~CCommunicationsHandler (void)

//	CCommunicationsHandler destructor

	{
	for (int i = 0; i < m_iCount; i++)
		{
		if (m_pMessages[i].pCode)
			m_pMessages[i].pCode->Discard(&g_pUniverse->GetCC());

		if (m_pMessages[i].pOnShow)
			m_pMessages[i].pOnShow->Discard(&g_pUniverse->GetCC());
		}

	if (m_pMessages)
		delete [] m_pMessages;
	}

ALERROR CCommunicationsHandler::InitFromXML (CXMLElement *pDesc, CString *retsError)

//	InitFromXML
//
//	Load from an XML element

	{
	int i, j;
	CString sError;

	//	Allocate the structure

	int iCount = pDesc->GetContentElementCount();
	if (iCount == 0)
		return NOERROR;

	ASSERT(m_pMessages == NULL);
	m_pMessages = new SMessage [iCount];
	m_iCount = iCount;

	for (i = 0; i < iCount; i++)
		{
		CXMLElement *pMessage = pDesc->GetContentElement(i);

		//	Get the name

		m_pMessages[i].sMessage = pMessage->GetAttribute(NAME_ATTRIB);
		m_pMessages[i].sShortcut = pMessage->GetAttribute(KEY_ATTRIB);

		//	If no sub elements, just get the code from the content

		if (pMessage->GetContentElementCount() == 0)
			{
			m_pMessages[i].pCode = g_pUniverse->GetCC().Link(pMessage->GetContentText(0), 0, NULL);
			m_pMessages[i].pOnShow = NULL;
			}

		//	If we've got sub elements, then load the different code blocks

		else
			{
			m_pMessages[i].pCode = NULL;
			m_pMessages[i].pOnShow = NULL;

			for (j = 0; j < pMessage->GetContentElementCount(); j++)
				{
				CXMLElement *pItem = pMessage->GetContentElement(j);

				//	OnShow

				if (strEquals(pItem->GetTag(), ON_SHOW_TAG))
					m_pMessages[i].pOnShow = g_pUniverse->GetCC().Link(pItem->GetContentText(0), 0, NULL);
				else if (strEquals(pItem->GetTag(), INVOKE_TAG) || strEquals(pItem->GetTag(), CODE_TAG))
					m_pMessages[i].pCode = g_pUniverse->GetCC().Link(pItem->GetContentText(0), 0, NULL);
				else
					{
					*retsError = strPatternSubst(CONSTLIT("Unknown element: <%s>"), pItem->GetTag().GetASCIIZPointer());
					return ERR_FAIL;
					}
				}
			}

		//	Deal with error

		if (m_pMessages[i].pCode && m_pMessages[i].pCode->IsError())
			sError = m_pMessages[i].pCode->GetStringValue();

		if (m_pMessages[i].pOnShow && m_pMessages[i].pOnShow->IsError())
			sError = m_pMessages[i].pOnShow->GetStringValue();
		}

	//	Done

	if (retsError)
		*retsError = sError;

	return (sError.IsBlank() ? NOERROR : ERR_FAIL);
	}
