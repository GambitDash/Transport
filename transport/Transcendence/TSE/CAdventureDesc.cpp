//	CAdventureDesc.cpp
//
//	CAdventureDesc class

#include "PreComp.h"

#include "Kernel.h"

#define EVENTS_TAG									CONSTLIT("Events")

#define NAME_ATTRIB									CONSTLIT("name")
#define BACKGROUND_ID_ATTRIB						CONSTLIT("backgroundID")

#define ON_GAME_START_EVENT							CONSTLIT("OnGameStart")
#define ON_GAME_END_EVENT							CONSTLIT("OnGameEnd")

void CAdventureDesc::FireOnGameEnd (const SGameStats &Game)

//	FireOnGameEnd
//
//	Fire OnGameEnd event

	{
	ICCItem *pCode;

	if (FindEventHandler(ON_GAME_END_EVENT, &pCode))
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		//	If the epitaph starts with a past tense verb, then we need to
		//	add "was" at the beginning

		CString sEpitaph;
		CString sFirstWord = strWord(Game.sEpitaph, 0);
		if (sFirstWord.GetLength() > 2)
			{
			char *pPos = sFirstWord.GetASCIIZPointer() + sFirstWord.GetLength() - 2;
			if (pPos[0] == 'e' && pPos[1] == 'd')
				sEpitaph = strPatternSubst(CONSTLIT("was %s"), Game.sEpitaph.GetPointer());
			else
				sEpitaph = Game.sEpitaph;
			}
		else
			sEpitaph = Game.sEpitaph;

		//	Initialize variables

		CC.DefineGlobalInteger(CONSTLIT("aScore"), Game.iScore);
		CC.DefineGlobalInteger(CONSTLIT("aResurrectCount"), Game.iResurrectCount);
		CC.DefineGlobalInteger(CONSTLIT("aSystemsVisited"), Game.iSystemsVisited);
		CC.DefineGlobalInteger(CONSTLIT("aEnemiesDestroyed"), Game.iEnemiesDestroyed);
		CC.DefineGlobalInteger(CONSTLIT("aBestEnemiesDestroyed"), Game.iBestEnemyDestroyedCount);
		if (Game.pBestEnemyDestroyed)
			CC.DefineGlobalInteger(CONSTLIT("aBestEnemyClass"), Game.pBestEnemyDestroyed->GetUNID());
		else
			CC.DefineGlobal(CONSTLIT("aBestEnemyClass"), CC.CreateNil());

		CC.DefineGlobalString(CONSTLIT("aEndGameReason"), Game.sEndGameReason);
		CC.DefineGlobalString(CONSTLIT("aEpitaph"), sEpitaph);
		CC.DefineGlobalString(CONSTLIT("aEpitaphOriginal"), Game.sEpitaph);
		CC.DefineGlobalString(CONSTLIT("aTime"), Game.sTime);

		//	Invoke

		ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
		if (pResult->IsError())
			kernelDebugLogMessage("OnGameEnd error: %s", pResult->GetStringValue().GetASCIIZPointer());
		pResult->Discard(&CC);
		}
	}

void CAdventureDesc::FireOnGameStart (void)

//	FireOnGameStart
//
//	Fire OnGameStart event

	{
	ICCItem *pCode;

	if (FindEventHandler(ON_GAME_START_EVENT, &pCode))
		{
		CCodeChain &CC = g_pUniverse->GetCC();

		//	Run code

		ICCItem *pResult = CC.TopLevel(pCode, &g_pUniverse);
		if (pResult->IsError())
			kernelDebugLogMessage("OnGameStart error: %s", pResult->GetStringValue().GetASCIIZPointer());
		pResult->Discard(&CC);
		}
	}

ALERROR CAdventureDesc::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind design elements

	{
	return NOERROR;
	}

ALERROR CAdventureDesc::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Load from XML

	{
	//	Remember the original adventure file

	m_sFilespec = Ctx.sResDb;
	m_dwExtensionUNID = (Ctx.pExtension ? Ctx.pExtension->dwUNID : 0);

	//	Load the name, etc

	m_sName = pDesc->GetAttribute(NAME_ATTRIB);
	m_dwBackgroundUNID = ::LoadUNID(Ctx, pDesc->GetAttribute(BACKGROUND_ID_ATTRIB));

	return NOERROR;
	}
