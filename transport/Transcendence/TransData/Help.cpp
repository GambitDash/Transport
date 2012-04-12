//	Help.cpp
//
//	Show help

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define ARMOR_TABLE_SWITCH					CONSTLIT("armortable")
#define ENTITIES_SWITCH						CONSTLIT("entities")
#define SHIP_TABLE_SWITCH					CONSTLIT("shiptable")
#define STATS_SWITCH						CONSTLIT("stats")
#define RANDOM_ITEMS_SWITCH					CONSTLIT("randomitems")
#define RANDOM_NUMBER_TEST					CONSTLIT("randomnumbertest")
#define SIM_TABLES_SWITCH					CONSTLIT("simTables")
#define STATION_FREQUENCY_SWITCH			CONSTLIT("stationfrequency")
#define SYSTEM_TEST_SWITCH					CONSTLIT("systemtest")
#define SHIELD_TEST_SWITCH					CONSTLIT("shieldtest")
#define ITEM_TABLE_SWITCH					CONSTLIT("itemtable")
#define ENCOUNTER_TABLE_SWITCH				CONSTLIT("encountertable")
#define ITEM_FREQUENCY_SWITCH				CONSTLIT("itemsim")
#define DEBUG_SWITCH						CONSTLIT("debug")

void ShowHelp (CXMLElement *pCmdLine)

//	ShowHelp
//
//	Show help for each command

	{
	bool bDebug = pCmdLine->GetAttributeBool(DEBUG_SWITCH);

	if (pCmdLine->GetAttributeBool(SYSTEM_TEST_SWITCH))
		{
		printf("  /systemtest           Generates sample of all systems.\n");
		printf("       /count:n             n iterations.\n");
		}
	else if (pCmdLine->GetAttributeBool(SIM_TABLES_SWITCH))
		{
		printf("  /simtables            Generates tables for sim counts.\n");
		printf("       /count:n             n iterations.\n");
		}
	else if (pCmdLine->GetAttributeBool(SHIELD_TEST_SWITCH))
		{
		printf("  /shieldtest           Outputs capabilities of shield against weapons.\n");
		printf("       /unid:n              unid or name of shield to test.\n");
		printf("      [/verbose]            output weapon stats.\n");
		}
	else if (pCmdLine->GetAttributeBool(ITEM_TABLE_SWITCH))
		{
		printf("  /itemtable            Item table.\n");
		printf("      [/criteria:xxx]       only items that match criteria.\n");
		printf("\n");
		printf("      [/averageCount]       average number appearing.\n");
		printf("      [/averageDamage]      average damage (in HP).\n");
		printf("      [/cost]               cost (in credits).\n");
		printf("      [/fireDelay]          time between shots (in ticks).\n");
		printf("      [/hp]                 hit points.\n");
		printf("      [/mass]               mass (in kg).\n");
		printf("      [/maxSpeed]           maximum speed (in %% of lightspeed).\n");
		printf("      [/power]              power consumed (in MW).\n");
		printf("      [/powerPerShot]       power per shot (in MW-minutes).\n");
		printf("      [/reference]          reference column.\n");
		printf("      [/regen]              regeneration (in HP per tick).\n");
		printf("      [/thrust]             thrust (in giganewtons).\n");
		printf("      [/totalCount]         total number appearing per game.\n");
		}
	else if (pCmdLine->GetAttributeBool(ENCOUNTER_TABLE_SWITCH))
		{
		printf("  /encountertable       Encounter table.\n");
		printf("      [/all]                include non-random stations.\n");
		printf("      [/criteria:xxx]       only encounters that match criteria.\n");
		printf("                            e.g., \"+enemy\"\n");
		printf("\n");
		printf("      [/armorClass]         armor class.\n");
		printf("      [/canAttack]          station can attack.\n");
		printf("      [/explosionType]      type of explosion when destroyed.\n");
		printf("      [/fireRateAdj]        fire rate adjustment.\n");
		printf("      [/hp]                 initial hit points.\n");
		printf("      [/totalCount]         total number appearing per game.\n");
		}
	else if (pCmdLine->GetAttributeBool(ITEM_FREQUENCY_SWITCH))
		{
		printf("  /itemsim              Simulation of items encountered.\n");
		printf("       /count:n             n iterations.\n");
		}
	else if (pCmdLine->GetAttributeBool(SHIP_TABLE_SWITCH))
		{
		printf("  /shiptable            Ship table.\n");
		printf("      [/allClasses]         include special classes (such as wingmen).\n");
		printf("\n");
		printf("      [/fireAccuracy]       AI fire accuracy (%%).\n");
		printf("      [/fireRangeAdj]       AI fire range adjustment (%% of normal).\n");
		printf("      [/fireRateAdj]        AI fire rate adjustment (%% of normal).\n");
		printf("      [/launcher]           launcher.\n");
		printf("      [/maneuver]           time for a complete rotation.\n");
		printf("      [/maxSpeed]           maximum speed (in %% of lightspeed).\n");
		printf("      [/primaryArmor]       primary armor.\n");
		printf("      [/primaryWeapon]      primary weapon.\n");
		printf("      [/primaryWeaponRange] max effective weapon range (in ls).\n");
		printf("      [/primaryWeaponRangeAdj] adjusted effective weapon range (in ls).\n");
		printf("      [/score]              score.\n");
		printf("      [/shield]             shield generator.\n");
		printf("      [/thrustToWeight]     thrust-to-weight ratio.\n");
		}
	else
		{
		printf("  /decompile            Extracts resources from .tdb (overwrites existing).\n");
		if (bDebug)
			printf("  /itemsim              Simulation of items encountered.\n");
		printf("  /itemtable            Item table.\n");
		printf("  /encountertable       Encounter table.\n");
		if (bDebug)
			printf("  /shieldtest           Outputs capabilities of shield against weapons.\n");
		printf("  /shiptable            Ship table.\n");
		printf("  /simtables            Generate tables for sim counts.\n");
		printf("  /stationfrequency     Table of station types by level.\n");
		printf("  /stats                Shows a list of basic stats.\n");
		if (bDebug)
			printf("  /systemlabels         Generate counts for all labels.\n");
		if (bDebug)
			printf("  /systemtest           Generates sample of all systems.\n");
		if (bDebug)
			printf("  /wordlist             Lists all unique words.\n");
		}

	printf("\n");
	}

