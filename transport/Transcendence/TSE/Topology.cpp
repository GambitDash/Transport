//	Topology.cpp
//
//	Star system topology

#include "PreComp.h"

#include "Kernel.h"

#define SYSTEM_TAG								CONSTLIT("System")
#define STARGATES_TAG							CONSTLIT("StarGates")

#define ID_ATTRIB								CONSTLIT("ID")
#define UNID_ATTRIB								CONSTLIT("UNID")
#define NAME_ATTRIB								CONSTLIT("Name")
#define DESTID_ATTRIB							CONSTLIT("DestID")
#define DESTGATE_ATTRIB							CONSTLIT("DestGate")
#define VARIANT_ATTRIB							CONSTLIT("variant")
#define LEVEL_ATTRIB							CONSTLIT("level")
#define ROOT_NODE_ATTRIB						CONSTLIT("rootNode")
#define EPITAPH_ATTRIB							CONSTLIT("epitaph")
#define END_GAME_REASON_ATTRIB					CONSTLIT("endGameReason")
#define END_GAME_ATTRIB							CONSTLIT("endGame")
#define ATTRIBUTES_ATTRIB						CONSTLIT("attributes")

#define PREV_DEST								CONSTLIT("[Prev]")
#define END_GAME_VALUE							CONSTLIT("[EndGame]")

ALERROR CUniverse::InitTopology (CString *retsError)

//	InitTopology
//
//	Initializes the star system topology

	{
	ALERROR error;
	int i;

	//	No need to initialize if we've already loaded it

	if (m_Topology.GetCount() > 0)
		return NOERROR;

	//	Add all root nodes in order. AddTopologyNode will recurse and
	//	add all nodes connected to each root.

	CTopologyDescTable *pTopology = m_Design.GetTopologyDesc();
	for (i = 0; i < pTopology->GetRootNodeCount(); i++)
		{
		if (error = AddTopologyNode(pTopology->GetRootNode(i),
				pTopology,
				NULL,
				NULL,
				retsError))
			return error;
		}

	//	Call OnGlobalTopologyCreated

	for (i = 0; i < m_Design.GetCount(); i++)
		if (error = m_Design.GetEntry(i)->FireOnGlobalTopologyCreated(retsError))
			return error;

	return NOERROR;
	}

ALERROR CUniverse::AddTopologyNode (CXMLElement *pNode, 
									CTopologyDescTable *pNodeMap, 
									CTopologyNode *pPrevNode, 
									CTopologyNode **retpNewNode,
									CString *retsError)

//	AddTopologyNode
//
//	Add node

	{
	ALERROR error;
	int i;

	//	Get the ID of the node

	CString sID = pNode->GetAttribute(ID_ATTRIB);
	bool bIsRootNode = pNode->GetAttributeBool(ROOT_NODE_ATTRIB);

	//	Is this an EndGame node?

	DWORD dwUNID;
	CXMLElement *pSystem;
	if (pNode->GetAttributeBool(END_GAME_ATTRIB))
		{
		dwUNID = END_GAME_SYSTEM_UNID;
		pSystem = NULL;
		}

	//	Otherwise, load the system element

	else
		{
		pSystem = pNode->GetContentElementByTag(SYSTEM_TAG);
		if (pSystem == NULL)
			{
			*retsError = strPatternSubst(CONSTLIT("Topology %s: <System> element not found"), sID.GetASCIIZPointer());
			return ERR_FAIL;
			}

		CString sSystemUNID = pSystem->GetAttribute(UNID_ATTRIB);
		dwUNID = strToInt(sSystemUNID, 0, NULL);
		}

	//	If the system node contains a table of different system types, then
	//	remember the root node because some of the system information (such as the
	//	name) may be there.

	CXMLElement *pSystemParent = NULL;

	//	If there is no UNID attribute then it means that the system
	//	is randomly determined based on a table

	if (dwUNID == 0 && pSystem->GetContentElementCount() == 1)
		{
		CXMLElement *pTableElement = pSystem->GetContentElement(0);
		if (pTableElement == NULL)
			return ERR_FAIL;

		CRandomEntryResults System;
		if (error = CRandomEntryGenerator::Generate(pTableElement, System))
			{
			*retsError = strPatternSubst(CONSTLIT("Topology %s: Unable to generate random system UNID"), sID.GetASCIIZPointer());
			return ERR_FAIL;
			}

		if (System.GetCount() != 1)
			return ERR_FAIL;

		pSystemParent = pSystem;
		pSystem = System.GetResult(0);
		dwUNID = pSystem->GetAttributeInteger(UNID_ATTRIB);
		}

	//	We must have a system

	if (dwUNID == 0)
		{
		*retsError = strPatternSubst(CONSTLIT("Topology %s: No system specified"), sID.GetASCIIZPointer());
		return ERR_FAIL;
		}

	//	Create a topology node and add it to the universe list

	CTopologyNode *pNewNode = new CTopologyNode(sID, dwUNID);
	m_Topology.AddEntry(sID, (CObject *)pNewNode);

	//	If this is the end game node, then load stuff about the end game

	if (dwUNID == END_GAME_SYSTEM_UNID)
		{
		pNewNode->SetEpitaph(pNode->GetAttribute(EPITAPH_ATTRIB));
		pNewNode->SetEndGameReason(pNode->GetAttribute(END_GAME_REASON_ATTRIB));
		}

	//	Set the name of the system

	if (pSystem)
		{
		pNewNode->SetName(pSystem->GetAttribute(NAME_ATTRIB));
		if (pSystemParent && pNewNode->GetSystemName().IsBlank())
			pNewNode->SetName(pSystemParent->GetAttribute(NAME_ATTRIB));

		//	Set the level

		int iLevel = pSystem->GetAttributeInteger(LEVEL_ATTRIB);
		if (pSystemParent && iLevel == 0)
			iLevel = pSystemParent->GetAttributeInteger(LEVEL_ATTRIB);
		if (iLevel == 0)
			iLevel = 1;
		pNewNode->SetLevel(iLevel);

		//	Add variants for the system

		CString sVariant = pSystem->GetAttribute(VARIANT_ATTRIB);
		if (!sVariant.IsBlank())
			pNewNode->AddVariantLabel(sVariant);
		if (pSystemParent)
			{
			sVariant = pSystemParent->GetAttribute(VARIANT_ATTRIB);
			if (!sVariant.IsBlank())
				pNewNode->AddVariantLabel(sVariant);
			}

		//	Add attributes for the system

		pNewNode->AddAttributes(pSystem->GetAttribute(ATTRIBUTES_ATTRIB));
		if (pSystemParent)
			pNewNode->AddAttributes(pSystemParent->GetAttribute(ATTRIBUTES_ATTRIB));

		//	Traverse the stargates list

		CXMLElement *pStarGates = pNode->GetContentElementByTag(STARGATES_TAG);
		if (pStarGates == NULL)
			{
			*retsError = strPatternSubst(CONSTLIT("Topology %s: <Stargates> element not found"), sID.GetASCIIZPointer());
			return ERR_FAIL;
			}

		CRandomEntryResults StarGates;
		if (error = CRandomEntryGenerator::GenerateAsGroup(pStarGates, StarGates))
			return error;

		for (i = 0; i < StarGates.GetCount(); i++)
			{
			CXMLElement *pGate = StarGates.GetResult(i);
			CString sDest = pGate->GetAttribute(DESTID_ATTRIB);
			CString sDestEntryPoint = pGate->GetAttribute(DESTGATE_ATTRIB);

			//	Figure out where the stargate goes

			CString sDestNode;

			if (strEquals(sDest, PREV_DEST))
				{
				//	If this is a root node, then we keep the "Prev" keyword
				//	(we will fix it up later, when the system actually gets created.)

				if (bIsRootNode)
					sDestNode = PREV_DEST;

				//	If we don't have a previous node, this is an error.

				else if (pPrevNode == NULL)
					{
					*retsError = strPatternSubst(CONSTLIT("Topology %s: Previous node not defined."), sID.GetASCIIZPointer());
					return ERR_FAIL;
					}

				//	Otherwise, fix up the node ID

				else
					sDestNode = pPrevNode->GetID();
				}
			else
				{
				//	Has this node already been added?
				
				CTopologyNode *pDest = FindTopologyNode(sDest);

				//	If not, then we need to recurse because this node has
				//	not been created yet.

				if (pDest == NULL)
					{
					CXMLElement *pDestNode = pNodeMap->FindNode(sDest);
					
					if (pDestNode == NULL)
						{
						*retsError = strPatternSubst(CONSTLIT("Topology %s: Gate destination node '%s' not found"), sID.GetASCIIZPointer(), sDest.GetASCIIZPointer());
						return ERR_FAIL;
						}

					if (error = AddTopologyNode(pDestNode, pNodeMap, pNewNode, &pDest, retsError))
						return error;
					}

				sDestNode = pDest->GetID();
				}

			//	Add the stargate

			pNewNode->AddGateInt(pGate->GetAttribute(NAME_ATTRIB), sDestNode, sDestEntryPoint);
			}
		}

	//	Done

	if (retpNewNode)
		*retpNewNode = pNewNode;

	return NOERROR;
	}

//	CTopologyNode class --------------------------------------------------------

CTopologyNode::CTopologyNode (const CString &sID, DWORD SystemUNID) : m_sID(sID),
		m_SystemUNID(SystemUNID),
		m_dwID(0xffffffff),
		m_NamedGates(FALSE, TRUE),
		m_pSystem(NULL)

//	CTopology constructor

	{
	}

CTopologyNode::~CTopologyNode (void)

//	CTopology destructor

	{
	for (int i = 0; i < m_NamedGates.GetCount(); i++)
		{
		StarGateDesc *pDesc = (StarGateDesc *)m_NamedGates.GetValue(i);
		delete pDesc;
		}
	}

void CTopologyNode::AddAttributes (const CString &sAttribs)

//	AddAttributes
//
//	Append the given attributes

	{
	m_sAttributes = ::AppendModifiers(m_sAttributes, sAttribs);
	}

void CTopologyNode::AddGateInt (const CString &sName, const CString &sDestNode, const CString &sEntryPoint)

//	AddGateInt
//
//	Adds a gate to the topology

	{
	StarGateDesc *pDesc = new StarGateDesc;
	pDesc->sDestNode = sDestNode;
	pDesc->sDestEntryPoint = sEntryPoint;

	m_NamedGates.AddEntry(sName, (CObject *)pDesc);
	}

ALERROR CTopologyNode::AddStargate (const CString &sGateID, const CString &sDestNodeID, const CString &sDestGateID)

//	AddStargate
//
//	Adds a new stargate to the topology

	{
	//	Get the destination node

	CTopologyNode *pDestNode = g_pUniverse->FindTopologyNode(sDestNodeID);
	if (pDestNode == NULL)
		{
		kernelDebugLogMessage("Unable to find destination node: %s", sDestNodeID.GetASCIIZPointer());
		return ERR_FAIL;
		}

	//	Look for the destination stargate

	CString sReturnNodeID;
	CString sReturnEntryPoint;
	if (!pDestNode->FindStargate(sDestGateID, &sReturnNodeID, &sReturnEntryPoint))
		{
		kernelDebugLogMessage("Unable to find destination stargate: %s", sDestGateID.GetASCIIZPointer());
		return ERR_FAIL;
		}

	//	Add the gate

	AddGateInt(sGateID, sDestNodeID, sDestGateID);

	//	See if we need to fix up the return gate

	if (strEquals(sReturnNodeID, PREV_DEST))
		pDestNode->SetStargateDest(sDestGateID, GetID(), sGateID);

	return NOERROR;
	}

void CTopologyNode::CreateFromStream (SUniverseLoadCtx &Ctx, CTopologyNode **retpNode)

//	CreateFromStream
//
//	Creates a node from a stream
//
//	CString		m_sID
//	DWORD		m_SystemUNID
//	CString		m_sName
//	DWORD		m_iLevel
//	DWORD		m_dwID
//
//	DWORD		No of named gates
//	CString		gate: sName
//	CString		gate: sDestNode
//	CString		gate: sDestEntryPoint
//
//	DWORD		No of variant labels
//	CString		variant label
//
//	CAttributeDataBlock	m_Data

	{
	int i;
	DWORD dwLoad;
	CTopologyNode *pNode;

	CString sID;
	sID.ReadFromStream(Ctx.pStream);
	Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
	pNode = new CTopologyNode(sID, dwLoad);
	
	pNode->m_sName.ReadFromStream(Ctx.pStream);
	Ctx.pStream->Read((char *)&pNode->m_iLevel, sizeof(DWORD));
	Ctx.pStream->Read((char *)&pNode->m_dwID, sizeof(DWORD));

	DWORD dwCount;
	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		{
		StarGateDesc *pDesc = new StarGateDesc;
		CString sName;
		sName.ReadFromStream(Ctx.pStream);
		pDesc->sDestNode.ReadFromStream(Ctx.pStream);
		pDesc->sDestEntryPoint.ReadFromStream(Ctx.pStream);

		pNode->m_NamedGates.AddEntry(sName, (CObject *)pDesc);
		}

	Ctx.pStream->Read((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		{
		CString sLabel;
		sLabel.ReadFromStream(Ctx.pStream);
		pNode->m_VariantLabels.AppendString(sLabel, NULL);
		}

	if (Ctx.dwVersion >= 1)
		pNode->m_Data.ReadFromStream(Ctx.pStream);

	//	Done

	*retpNode = pNode;
	}

bool CTopologyNode::FindStargate (const CString &sName, CString *retsDestNode, CString *retsEntryPoint)

//	FindStargate
//
//	Looks for the stargate by name and returns the destination node id and entry point

	{
	StarGateDesc *pDesc;
	if (m_NamedGates.Lookup(sName, (CObject **)&pDesc) != NOERROR)
		return false;

	if (retsDestNode)
		*retsDestNode = pDesc->sDestNode;

	if (retsEntryPoint)
		*retsEntryPoint = pDesc->sDestEntryPoint;

	return true;
	}

CString CTopologyNode::FindStargateName (const CString &sDestNode, const CString &sEntryPoint)

//	FindStargateName
//
//	Returns the name of the stargate that matches the node and entry point

	{
	int i;

	for (i = 0; i < m_NamedGates.GetCount(); i++)
		{
		StarGateDesc *pDesc = (StarGateDesc *)m_NamedGates.GetValue(i);
		if (strEquals(pDesc->sDestNode, sDestNode)
				&& strEquals(pDesc->sDestEntryPoint, sEntryPoint))
			return m_NamedGates.GetKey(i);
		}

	return NULL_STR;
	}

CTopologyNode *CTopologyNode::GetGateDest (const CString &sName, CString *retsEntryPoint)

//	GetGateDest
//
//	Get stargate destination

	{
	StarGateDesc *pDesc;
	if (m_NamedGates.Lookup(sName, (CObject **)&pDesc) != NOERROR)
		return NULL;

	if (retsEntryPoint)
		*retsEntryPoint = pDesc->sDestEntryPoint;
	return g_pUniverse->FindTopologyNode(pDesc->sDestNode);
	}

CString CTopologyNode::GetStargate (int iIndex)

//	GetStargate
//
//	Returns the stargate ID

	{
	return m_NamedGates.GetKey(iIndex);
	}

bool CTopologyNode::HasVariantLabel (const CString &sVariant)

//	HasVariantLabel
//
//	Returns TRUE if it has the given variant label

	{
	for (int i = 0; i < m_VariantLabels.GetCount(); i++)
		{
		if (strEquals(sVariant, m_VariantLabels.GetStringValue(i)))
			return true;
		}

	return false;
	}

void CTopologyNode::SetStargateDest (const CString &sName, const CString &sDestNode, const CString &sEntryPoint)

//	SetStargateDest
//
//	Sets the destination information for the given stargate

	{
	StarGateDesc *pDesc;
	if (m_NamedGates.Lookup(sName, (CObject **)&pDesc) != NOERROR)
		{
		ASSERT(false);
		return;
		}

	pDesc->sDestNode = sDestNode;
	pDesc->sDestEntryPoint = sEntryPoint;
	}

void CTopologyNode::WriteToStream (IWriteStream *pStream)

//	WriteToStream
//
//	Writes out the variable portions of the node
//
//	CString		m_sID
//	DWORD		m_SystemUNID
//	CString		m_sName
//	DWORD		m_iLevel
//	DWORD		m_dwID
//
//	DWORD		No of named gates
//	CString		gate: sName
//	CString		gate: sDestNode
//	CString		gate: sDestEntryPoint
//
//	DWORD		No of variant labels
//	CString		variant label
//
//	CAttributeDataBlock	m_Data

	{
	int i;

	m_sID.WriteToStream(pStream);
	pStream->Write((char *)&m_SystemUNID, sizeof(DWORD));
	m_sName.WriteToStream(pStream);
	pStream->Write((char *)&m_iLevel, sizeof(DWORD));
	pStream->Write((char *)&m_dwID, sizeof(DWORD));

	DWORD dwCount = m_NamedGates.GetCount();
	pStream->Write((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		{
		StarGateDesc *pDesc = (StarGateDesc *)m_NamedGates.GetValue(i);
		CString sName = m_NamedGates.GetKey(i);
		sName.WriteToStream(pStream);
		pDesc->sDestNode.WriteToStream(pStream);
		pDesc->sDestEntryPoint.WriteToStream(pStream);
		}

	dwCount = m_VariantLabels.GetCount();
	pStream->Write((char *)&dwCount, sizeof(DWORD));
	for (i = 0; i < (int)dwCount; i++)
		m_VariantLabels.GetStringValue(i).WriteToStream(pStream);

	//	Write opaque data

	m_Data.WriteToStream(pStream);
	}

