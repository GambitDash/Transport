//	CHighScoreList.cpp
//
//	CHighScoreList class

#include "PreComp.h"
#include "Transcendence.h"

#define SCORE_ATTRIB						CONSTLIT("score")
#define NAME_ATTRIB							CONSTLIT("name")
#define EPITAPH_ATTRIB						CONSTLIT("epitaph")
#define TIME_ATTRIB							CONSTLIT("time")
#define RESURRECT_COUNT_ATTRIB				CONSTLIT("resurrectCount")
#define GENOME_ATTRIB						CONSTLIT("genome")
#define LAST_PLAYER_NAME_ATTRIB				CONSTLIT("lastPlayerName")
#define LAST_PLAYER_GENOME_ATTRIB			CONSTLIT("lastPlayerGenome")
#define DEBUG_ATTRIB						CONSTLIT("debug")
#define SHIP_CLASS_ATTRIB					CONSTLIT("shipClass")
#define ADVENTURE_ATTRIB					CONSTLIT("adventureUNID")
#define ADVENTURE_NAME_ATTRIB				CONSTLIT("adventureName")

CHighScoreList::CHighScoreList (void) : m_bModified(false),
		m_iCount(0),
		m_iMostRecentPlayerGenome(genomeUnknown)

//	CHighScoreList constructor

	{
	}

ALERROR CHighScoreList::Load (const CString &sFilename)

//	Load
//
//	Load the high score list

	{
	ALERROR error;

	//	Load XML

	CFileReadBlock DataFile(sFilename);
	CXMLElement *pData;
	CString sError;

	error = CXMLElement::ParseXML(&DataFile, &pData, &sError);
	if (error)
		//	Means we can't find it or is corrupt...
		return NOERROR;

	//	Get the most recent player name

	m_sMostRecentPlayerName = pData->GetAttribute(LAST_PLAYER_NAME_ATTRIB);
	m_iMostRecentPlayerGenome = LoadGenome(pData->GetAttribute(LAST_PLAYER_GENOME_ATTRIB));

	//	Fill the structures

	for (int i = 0; i < pData->GetContentElementCount(); i++)
		{
		CXMLElement *pItem = pData->GetContentElement(i);

		m_List[m_iCount].dwAdventure = pItem->GetAttributeInteger(ADVENTURE_ATTRIB);
		if (m_List[m_iCount].dwAdventure == 0)
			m_List[m_iCount].dwAdventure = DEFAULT_ADVENTURE_UNID;
		m_List[m_iCount].iScore = pItem->GetAttributeInteger(SCORE_ATTRIB);
		m_List[m_iCount].iResurrectCount = pItem->GetAttributeInteger(RESURRECT_COUNT_ATTRIB);
		m_List[m_iCount].sName = pItem->GetAttribute(NAME_ATTRIB);
		m_List[m_iCount].iGenome = LoadGenome(pItem->GetAttribute(GENOME_ATTRIB));
		m_List[m_iCount].sEpitaph = pItem->GetAttribute(EPITAPH_ATTRIB);
		m_List[m_iCount].sTime = pItem->GetAttribute(TIME_ATTRIB);
		m_List[m_iCount].bDebugGame = (pItem->GetAttributeInteger(DEBUG_ATTRIB) ? true : false);
		if (!pItem->FindAttribute(SHIP_CLASS_ATTRIB, &m_List[m_iCount].sShipClass))
			m_List[m_iCount].sShipClass = CONSTLIT("Sapphire-class yacht");
		m_iCount++;
		}

	//	Done

	delete pData;

	return NOERROR;
	}

int CHighScoreList::LoadGenome (const CString &sAttrib)

//	LoadGenome
//
//	Load genome value

	{
	int iGenome = strToInt(sAttrib, genomeUnknown, NULL);

	switch (iGenome)
		{
		case genomeHumanMale:
		case genomeHumanFemale:
			return iGenome;

		default:
			return genomeUnknown;
		}
	}

ALERROR CHighScoreList::Save (const CString &sFilename)

//	Save
//
//	Save the high score list

	{
	ALERROR error;

	if (m_bModified)
		{
		CFileWriteStream DataFile(sFilename, FALSE);

		error = DataFile.Create();
		if (error)
			return error;

		//	Write the XML header

		CString sData = strPatternSubst(CONSTLIT("<?xml version=\"1.0\"?>\r\n\r\n<TranscendenceHighScores lastPlayerName=\"%s\" lastPlayerGenome=\"%d\">\r\n\r\n"),
				m_sMostRecentPlayerName.GetASCIIZPointer(),
				m_iMostRecentPlayerGenome);

		error = DataFile.Write(sData.GetPointer(), sData.GetLength(), NULL);
		if (error)
			return error;

		//	Loop over scores

		for (int i = 0; i < m_iCount; i++)
			{
			sData = strPatternSubst(CONSTLIT("\t<Score name=\"%s\"\tshipClass=\"%s\"\tgenome=\"%d\"\tscore=\"%d\"\tepitaph=\"%s\"\ttime=\"%s\"\tresurrectCount=\"%d\"\tadventureUNID=\"%x\"\tdebug=\"%d\"/>\r\n"),
					CXMLElement::MakeAttribute(m_List[i].sName).GetASCIIZPointer(),
					CXMLElement::MakeAttribute(m_List[i].sShipClass).GetASCIIZPointer(),
					m_List[i].iGenome,
					m_List[i].iScore,
					CXMLElement::MakeAttribute(m_List[i].sEpitaph).GetASCIIZPointer(),
					m_List[i].sTime.GetASCIIZPointer(),
					m_List[i].iResurrectCount,
					m_List[i].dwAdventure,
					(m_List[i].bDebugGame ? 1 : 0));

			error = DataFile.Write(sData.GetPointer(), sData.GetLength(), NULL);
			if (error)
				return error;
			}

		//	Done

		sData = CONSTLIT("\r\n</TranscendenceHighScores>\r\n");

		error = DataFile.Write(sData.GetPointer(), sData.GetLength(), NULL);
		if (error)
			return error;

		error = DataFile.Close();
		if (error)
			return error;
		}

	return NOERROR;
	}

int CHighScoreList::AddEntry (const SGameStats &NewEntry)

//	AddEntry
//
//	Add another entry to the high score

	{
	int i, j;

	//	Score of 0 doesn't count

	if (NewEntry.iScore == 0)
		return -1;

	//	Find a spot on the list

	for (i = 0; i < m_iCount; i++)
		{
		if (NewEntry.iScore > m_List[i].iScore)
			break;
		}

	//	If we are the end of the list, then we didn't make the
	//	high score list.

	if (i == MAX_SCORES)
		return -1;

	//	Otherwise, move all scores below us by one

	m_bModified = true;

	if (m_iCount < MAX_SCORES)
		m_iCount++;

	for (j = m_iCount-2; j >= i; j--)
		m_List[j+1] = m_List[j];

	m_List[i] = NewEntry;

	//	Player name

	m_sMostRecentPlayerName = NewEntry.sName;
	m_iMostRecentPlayerGenome = NewEntry.iGenome;

	return i;
	}

void CHighScoreList::SetMostRecentPlayerName (const CString &sName)

//	SetMostRecentPlayerName
//
//	Set the player name

	{
	if (!strEquals(sName, m_sMostRecentPlayerName))
		{
		m_sMostRecentPlayerName = sName;
		m_bModified = true;
		}
	}

void CHighScoreList::SetMostRecentPlayerGenome (int iGenome) 

//	SetMostRecentPlayerGenome
//
//	Set the player genome

	{
	if (iGenome != m_iMostRecentPlayerGenome)
		{
		m_iMostRecentPlayerGenome = iGenome;
		m_bModified = true;
		}
	}
