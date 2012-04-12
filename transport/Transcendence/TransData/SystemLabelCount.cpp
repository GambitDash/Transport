//	SystemLabelCount.cpp
//
//	Generate statistics about label frequency

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

void GenerateSystemLabelCount (CUniverse &Universe, CXMLElement *pCmdLine)
	{
	ALERROR error;
	int i;
	CSovereign *pPlayer = Universe.FindSovereign(g_PlayerSovereignUNID);

	int iSystemSample = pCmdLine->GetAttributeInteger(CONSTLIT("count"));
	if (iSystemSample == 0)
		iSystemSample = 1;

	//	Generate systems for multiple games

	CSystemCreateStats Stats;
	for (i = 0; i < iSystemSample; i++)
		{
		printf("pass %d...\n", i+1);

		CTopologyNode *pNode = Universe.GetFirstTopologyNode();

		while (true)
			{
			//	Create the system

			CSystem *pSystem;
			if (error = Universe.CreateStarSystem(pNode, &pSystem, &Stats))
				{
				printf("ERROR: Unable to create star system.\n");
				return;
				}

			//	Get the next node

			CString sEntryPoint;
			pNode = pSystem->GetStargateDestination(CONSTLIT("Outbound"), &sEntryPoint);
			if (pNode == NULL)
				break;

			//	Done with old system

			Universe.DestroySystem(pSystem);
			}

		Universe.Reinit();
		}

	printf("LABEL STATISTICS\n\n");

	int iTotalLabels100 = Stats.GetTotalLabelCount() * 100 / iSystemSample;
	printf("Total labels: %d.%02d\n\n",
			iTotalLabels100 / 100,
			iTotalLabels100 % 100);

	for (i = 0; i < Stats.GetLabelAttributesCount(); i++)
		{
		CString sAttribs;
		int iCount;

		Stats.GetLabelAttributes(i, &sAttribs, &iCount);

		int iCount100 = iCount * 100 / iSystemSample;
		int iPercent100 = iCount * 10000 / Stats.GetTotalLabelCount();

		printf("%s: %d.%02d (%d.%02d%%)\n", 
				sAttribs.GetASCIIZPointer(), 
				iCount100 / 100,
				iCount100 % 100,
				iPercent100 / 100,
				iPercent100 % 100);
		}
	}
