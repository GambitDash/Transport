//	TransData.h
//
//	Include file for TranData app

#ifndef INCL_TRANSDATA
#define INCL_TRANSDATA

#ifndef INCL_TSE
#include "../TSE/TSE.h"
#endif

//	Used by sim tables

class EntryInfo : public CObject
	{
	public:
		EntryInfo (void) : CObject(NULL) { }

		DWORD dwUNID;
		double rTotalCount;
	};

class ItemInfo : public CObject
	{
	public:
		ItemInfo (void) : CObject(NULL) { }

		CItemType *pType;							//	Item type
		int iTotalCount;							//	Total times this item type has appeared
		double rTotalCount;
	};

const int MAX_FREQUENCY_COUNT = 12;

class StationInfo : public CObject
	{
	public:
		StationInfo (void) : CObject(NULL) { }

		CStationType *pType;
		CString sCategory;

		int iFreqCount[MAX_FREQUENCY_COUNT];		//	For each count, then number of times that
													//	number of stations have appeared in the
													//	system instance.

		int iTotalCount;							//	Total times this station type has appeared
													//	the system instance.
		double rTotalCount;

		int iSystemCount;							//	Number of system instances with this
													//	station type

		int iTempCount;								//	Temp count for a specific system instance
	};

//	Functions

char *FrequencyChar (int iFreq);
char *RomanNumeral (int iNumber);
void ShowHelp (CXMLElement *pCmdLine);
void MarkItemsKnown (CUniverse &Universe);
ALERROR LoadTotalCount (const CString &sFilename, CSymbolTable &TotalCount);

void Decompile (const CString &sDataFile, CXMLElement *pCmdLine);
void GenerateArmorTable (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateEncounterTable (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateEntitiesTable (const CString &sDataFile, CXMLElement *pCmdLine);
void GenerateItemFrequencyTable (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateItemTable (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateRandomItemTables (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateShieldStats (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateShipImages (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateShipTable (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateSimTables (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateStationFrequencyTable (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateStats (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateSystemLabelCount (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateSystemTest (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateTopology (CUniverse &Universe, CXMLElement *pCmdLine);
void GenerateWordList (const CString &sDataFile, CXMLElement *pCmdLine);

//	Don't care about warning of depecrated functions (e.g., sprintf)
#pragma warning(disable: 4996)

#endif