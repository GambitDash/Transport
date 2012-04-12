//	TransData
//
//	TransData is used to report information out of a Transcendence
//	datafile

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define NOARGS								CONSTLIT("noArgs")
#define QUESTION_MARK_SWITCH				CONSTLIT("?")
#define HELP_SWITCH							CONSTLIT("help")
#define H_SWITCH							CONSTLIT("h")
#define NO_LOGO_SWITCH						CONSTLIT("nologo")

#define ARMOR_TABLE_SWITCH					CONSTLIT("armortable")
#define DECOMPILE_SWITCH					CONSTLIT("decompile")
#define ENCOUNTER_TABLE_SWITCH				CONSTLIT("encountertable")
#define ENTITIES_SWITCH						CONSTLIT("entities")
#define SHIP_TABLE_SWITCH					CONSTLIT("shiptable")
#define SHIP_IMAGES_SWITCH					CONSTLIT("shipimages")
#define STATS_SWITCH						CONSTLIT("stats")
#define RANDOM_ITEMS_SWITCH					CONSTLIT("randomitems")
#define RANDOM_NUMBER_TEST					CONSTLIT("randomnumbertest")
#define SHIELD_TEST_SWITCH					CONSTLIT("shieldtest")
#define SIM_TABLES_SWITCH					CONSTLIT("simTables")
#define STATION_FREQUENCY_SWITCH			CONSTLIT("stationfrequency")
#define SYSTEM_LABELS_SWITCH				CONSTLIT("systemlabels")
#define SYSTEM_TEST_SWITCH					CONSTLIT("systemtest")
#define TOPOLOGY_SWITCH						CONSTLIT("topology")
#define ITEM_FREQUENCY_SWITCH				CONSTLIT("itemsim")
#define ITEM_TABLE_SWITCH					CONSTLIT("itemtable")
#define WORD_LIST_SWITCH					CONSTLIT("wordlist")

void AlchemyMain (CXMLElement *pCmdLine);

int main (int argc, char *argv[ ], char *envp[ ])

//	main
//
//	main entry-point

	{
	if (!kernelInit())
		{
		printf("ERROR: Unable to initialize Alchemy kernel.\n");
		return 1;
		}

	//	Do it

	{
	ALERROR error;
	CXMLElement *pCmdLine;
	if (error = CreateXMLElementFromCommandLine(argc, argv, &pCmdLine))
		{
		printf("ERROR: Unable to parse command line.\n");
		return 1;
		}

	AlchemyMain(pCmdLine);

	delete pCmdLine;
	}

	//	Done

	kernelCleanUp();
	return 0;
	}

void AlchemyMain (CXMLElement *pCmdLine)

//	AlchemyMain
//
//	Main entry-point after kernel initialization

	{
	ALERROR error;
	bool bLogo = !pCmdLine->GetAttributeBool(NO_LOGO_SWITCH);

	if (bLogo)
		{
		printf("TransData v1.6\n");
		printf("Copyright (c) 2001-2008 by George Moromisato. All Rights Reserved.\n\n");
		}

	if (pCmdLine->GetAttributeBool(NOARGS)
			|| pCmdLine->GetAttributeBool(QUESTION_MARK_SWITCH)
			|| pCmdLine->GetAttributeBool(HELP_SWITCH)
			|| pCmdLine->GetAttributeBool(H_SWITCH))
		{
		ShowHelp(pCmdLine);
		return;
		}

	//	Figure out the data file that we're working on

	CString sDataFile = CONSTLIT("Transcendence");

	//	See if we are doing a command that does not require parsing

	if (pCmdLine->GetAttributeBool(WORD_LIST_SWITCH))
		{
		GenerateWordList(sDataFile, pCmdLine);
		return;
		}
	else if (pCmdLine->GetAttributeBool(ENTITIES_SWITCH))
		{
		GenerateEntitiesTable(sDataFile, pCmdLine);
		return;
		}
	else if (pCmdLine->GetAttributeBool(DECOMPILE_SWITCH))
		{
		Decompile(sDataFile, pCmdLine);
		return;
		}

	//	See if we need to load images

	DWORD dwInitFlags = 0;
	if (pCmdLine->GetAttributeBool(SHIP_IMAGES_SWITCH))
		;
	else
		dwInitFlags |= flagNoResources;

	//	We don't need a version check

	dwInitFlags |= flagNoVersionCheck;

	//	Open the universe

	if (bLogo)
		printf("Loading...");
	CUniverse Universe;
	CString sError;
	if (error = Universe.Init(sDataFile, &sError, dwInitFlags))
		{
		printf("\n%s\n", sError.GetASCIIZPointer());
		return;
		}
	if (error = Universe.InitAdventure(DEFAULT_ADVENTURE_UNID, &sError))
		{
		printf("\n%s\n", sError.GetASCIIZPointer());
		return;
		}
	if (bLogo)
		printf("done.\n");

	//	Mark everything as known

	MarkItemsKnown(Universe);

	//	Figure out what to do

	if (pCmdLine->GetAttributeBool(ARMOR_TABLE_SWITCH))
		GenerateArmorTable(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(ENCOUNTER_TABLE_SWITCH))
		GenerateEncounterTable(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(SHIP_TABLE_SWITCH))
		GenerateShipTable(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(SHIP_IMAGES_SWITCH))
		GenerateShipImages(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(RANDOM_ITEMS_SWITCH))
		GenerateRandomItemTables(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(STATS_SWITCH))
		GenerateStats(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(STATION_FREQUENCY_SWITCH))
		GenerateStationFrequencyTable(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(SYSTEM_LABELS_SWITCH))
		GenerateSystemLabelCount(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(SYSTEM_TEST_SWITCH))
		GenerateSystemTest(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(SHIELD_TEST_SWITCH))
		GenerateShieldStats(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(ITEM_TABLE_SWITCH))
		GenerateItemTable(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(ITEM_FREQUENCY_SWITCH))
		GenerateItemFrequencyTable(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(SIM_TABLES_SWITCH))
		GenerateSimTables(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(TOPOLOGY_SWITCH))
		GenerateTopology(Universe, pCmdLine);
	else if (pCmdLine->GetAttributeBool(RANDOM_NUMBER_TEST))
		{
		int i;
		int Results[100];
		for (i = 0; i < 100; i++) 
			Results[i] = 0;

		for (i = 0; i < 1000000; i++)
			Results[mathRandom(1, 100)-1]++;

		for (i = 0; i < 100; i++)
			printf("%d: %d\n", i, Results[i]);

		}
	else
		GenerateStats(Universe, pCmdLine);

	//	Done
	}

void MarkItemsKnown (CUniverse &Universe)
	{
	int i;

	for (i = 0; i < Universe.GetItemTypeCount(); i++)
		{
		CItemType *pItem = Universe.GetItemType(i);
		pItem->SetKnown();
		pItem->SetShowReference();
		}
	}
