//	Topology.cpp
//
//	Generate statistics about system topology

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

void OutputTopologyNode (CTopologyNode *pNode);

void GenerateTopology (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	int i;

	for (i = 0; i < Universe.GetTopologyNodeCount(); i++)
		{
		CTopologyNode *pNode = Universe.GetTopologyNode(i);

		OutputTopologyNode(pNode);
		}
	}

void OutputTopologyNode (CTopologyNode *pNode)
	{
	}