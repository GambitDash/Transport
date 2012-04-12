//	CTopologDescTable.cpp
//
//	CTopologyDescTable class

#include "PreComp.h"

#define ID_ATTRIB							CONSTLIT("ID")
#define ROOT_NODE_ATTRIB					CONSTLIT("rootNode")

CTopologyDescTable::CTopologyDescTable (void) :
		m_Table(TRUE, FALSE)

//	CTopologyDescTable constructor

	{
	}

CTopologyDescTable::~CTopologyDescTable (void)

//	CTopologyDescTable destructor

	{
	}

CXMLElement *CTopologyDescTable::FindNode (const CString &sNodeID)

//	FindNode
//
//	Returns a topology node descriptor of the given ID or returns NULL
//	if the node is not found

	{
	CXMLElement *pNode;
	if (m_Table.Lookup(sNodeID, (CObject **)&pNode) != NOERROR)
		return NULL;

	return pNode;
	}

ALERROR CTopologyDescTable::LoadFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadFromXML
//
//	Load the table from an XML element (OK if this gets called multiple times)

	{
	ALERROR error;
	int i;

	CXMLElement *pFirstNode = NULL;
	for (i = 0; i < pDesc->GetContentElementCount(); i++)
		{
		CXMLElement *pNode = pDesc->GetContentElement(i);
		CString sNodeID = pNode->GetAttribute(ID_ATTRIB);

		CXMLElement *pNodeCopy = pNode->OrphanCopy();
		if (error = m_Table.AddEntry(sNodeID, pNodeCopy))
			{
			Ctx.sError = CONSTLIT("Unable to add topology node.");
			return error;
			}

		//	If this is a root node, add it to our list. We add root nodes
		//	in the order that we find them.

		if (pNodeCopy->GetAttributeBool(ROOT_NODE_ATTRIB))
			{
			m_RootNodes.push_back(pNodeCopy);

			//	Remember the first root node

			if (m_sFirstNode.IsBlank())
				m_sFirstNode = sNodeID;
			}

		//	Keep track of the first node in case we find no other root node

		if (pFirstNode == NULL)
			pFirstNode = pNodeCopy;
		}

	//	If we have no nodes, then we have an error

	if (m_Table.GetCount() == 0)
		{
		Ctx.sError = CONSTLIT("No topology nodes found.");
		return ERR_FAIL;
		}

	//	If no system is marked as a root node then we take the first node in the list

	if (m_sFirstNode.IsBlank())
		{
		m_RootNodes.push_back(pFirstNode);
		m_sFirstNode = pFirstNode->GetAttribute(ID_ATTRIB);
		}

	return NOERROR;
	}
