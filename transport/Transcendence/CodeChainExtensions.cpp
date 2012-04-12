//	CodeChainExtensions.cpp
//
//	CodeChainExtensions

#include "PreComp.h"
#include "Transcendence.h"

#define FN_GAME_SET_CRAWL_TEXT		1
#define FN_GAME_SET_CRAWL_IMAGE		2

ICCItem *fnGameSet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

#define	FN_SCR_GET_ITEM				1
#define FN_SCR_REMOVE_ITEM			2
#define FN_SCR_GET_ITEM_LIST_CURSOR	3
#define FN_SCR_REFRESH_ITEM			4

ICCItem *fnScrItem (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);

#define FN_PLY_CHARGE				1
#define FN_PLY_CREDITS				2
#define FN_PLY_DESTROYED			3
#define FN_PLY_MESSAGE				4
#define FN_PLY_INSURANCE_CLAIMS		5
#define FN_PLY_IS_INSURED			6
#define FN_PLY_INSURE				7
#define FN_PLY_INC_ORACUS			9
#define FN_PLY_COMPOSE_STRING		10
#define FN_PLY_REDIRECT_MESSAGE		11
#define FN_PLY_SHOW_HELP_REFUEL		12
#define FN_PLY_GENOME				13

ICCItem *fnPlyGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);
ICCItem *fnPlyGetOld (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnPlySet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);
ICCItem *fnPlySetOld (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);

#define FN_SCR_SHOW_PANE			1
#define FN_SCR_DESC					2
#define FN_SCR_SHOW_SCREEN			3
#define FN_SCR_COUNTER				4
#define FN_SCR_EXIT_DOCK			5
#define FN_SCR_ENABLE_ACTION		6
#define FN_SCR_ACTION_LABEL			7
#define FN_SCR_LIST_FILTER			8
#define FN_SCR_IS_FIRST_ON_INIT		9
#define FN_SCR_INPUT_TEXT			10
#define FN_SCR_LIST_ENTRY			11
#define FN_SCR_SHOW_ACTION			12
#define FN_SCR_SET_DISPLAY_TEXT		13

ICCItem *fnScrGet (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);
ICCItem *fnScrSet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);
ICCItem *fnScrSetOld (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData);

#define FN_DEBUG_OUTPUT				1
#define FN_DEBUG_LOG				2
#define FN_DEBUG_FLUSH			3

ICCItem *fnDebug (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

ICCItem *fnPlyComposeString (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData);

#define GENOME_HUMAN_MALE			CONSTLIT("humanMale")
#define GENOME_HUMAN_FEMALE			CONSTLIT("humanFemale")

static PRIMITIVEPROCDEF g_Extensions[] =
	{
		//	Game function
		//	-------------

		{	"gamSetCrawlImage",				fnGameSet,		FN_GAME_SET_CRAWL_IMAGE,
			"(gamSetCrawlImage imageUNID) -> True/Nil",
			"i",	PPFLAG_SIDEEFFECTS, },

		{	"gamSetCrawlText",				fnGameSet,		FN_GAME_SET_CRAWL_TEXT,
			"(gamSetCrawlText text) -> True/Nil",
			"s",	PPFLAG_SIDEEFFECTS, },

		//	Dock Screen functions
		//	---------------------

		{	"scrEnableAction",				fnScrSetOld,		FN_SCR_ENABLE_ACTION,	"",		NULL,	PPFLAG_SIDEEFFECTS, },
		//	(scrEnableAction screen action enabled)

		{	"scrExitDock",					fnScrGet,		FN_SCR_EXIT_DOCK,	"",		NULL,	PPFLAG_SIDEEFFECTS, },
		//	(scrExitDock screen)

		{	"scrGetCounter",				fnScrGet,		FN_SCR_COUNTER,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(scrGetCounter screen)

		{	"scrGetInputText",				fnScrGet,		FN_SCR_INPUT_TEXT,	"",		NULL,	PPFLAG_SIDEEFFECTS, },
		//	(scrGetInputText screen) => string

		{	"scrGetItem",					fnScrItem,		FN_SCR_GET_ITEM,	"",		NULL,	PPFLAG_SIDEEFFECTS, },
		//	(scrGetItem screen) => item

		{	"scrGetItemListCursor",			fnScrItem,		FN_SCR_GET_ITEM_LIST_CURSOR,
			"DEPRECATED: Use scrGetItem instead",		NULL,	PPFLAG_SIDEEFFECTS, },

		{	"scrGetListEntry",				fnScrGet,		FN_SCR_LIST_ENTRY,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(scrGetListEntry screen) -> entry

		{	"scrIsFirstOnInit",				fnScrGet,		FN_SCR_IS_FIRST_ON_INIT,	"",		NULL,	PPFLAG_SIDEEFFECTS, },
		//	(scrIsFirstOnInit screen) => True/Nil

		{	"scrRefreshItemListCursor",		fnScrItem,		FN_SCR_REFRESH_ITEM,	
			"DEPRECATED",		NULL,	PPFLAG_SIDEEFFECTS, },
		//	(scrRefreshItemListCursor screen item)

		{	"scrRemoveItem",				fnScrItem,		FN_SCR_REMOVE_ITEM,	"",		NULL,	PPFLAG_SIDEEFFECTS, },
		//	(scrRemoveItem screen count) => item

		{	"scrSetActionLabel",			fnScrSet,		FN_SCR_ACTION_LABEL,
			"(scrSetActionLabel screen action label [key])",
			"iis*",		PPFLAG_SIDEEFFECTS, },

		{	"scrSetCounter",				fnScrSetOld,		FN_SCR_COUNTER,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(scrSetCounter screen counter)

		{	"scrSetDesc",					fnScrSetOld,		FN_SCR_DESC,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(scrSetDesc screen description)

		{	"scrSetDisplayText",			fnScrSet,			FN_SCR_SET_DISPLAY_TEXT,
			"(scrSetDisplayText screen ID text)",
			"iss",	PPFLAG_SIDEEFFECTS,	},

		{	"scrSetInputText",				fnScrSetOld,		FN_SCR_INPUT_TEXT,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(scrSetInputText screen text)

		{	"scrSetListFilter",				fnScrSetOld,		FN_SCR_LIST_FILTER,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(scrSetListFilter screen filter)

		{	"scrShowAction",				fnScrSetOld,		FN_SCR_SHOW_ACTION,
			"(scrShowAction screen action shown)",
			NULL,	PPFLAG_SIDEEFFECTS, },

		{	"scrShowPane",					fnScrSetOld,		FN_SCR_SHOW_PANE,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(scrShowPane screen pane)

		{	"scrShowScreen",				fnScrSetOld,		FN_SCR_SHOW_SCREEN,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(scrShowPane screen screen [pane])

		//	Player functions
		//	----------------

		{	"plyCharge",					fnPlySetOld,		FN_PLY_CHARGE,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(plyCharge player charge)

		{	"plyClearShowHelpRefuel",		fnPlyGetOld,		FN_PLY_SHOW_HELP_REFUEL,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(plyClearShowHelpRefuel player)

		{	"plyComposeString",				fnPlyComposeString,		0,
			"(plyComposeString player string [arg1 arg2 ... argn]) -> string",
			"is*",	0,	},
		//	(plyComposeString player string)
		//		%name%				player name
		//		%he%				he or she
		//		%his%				his or her (matching case)
		//		%hers%				his or hers (matching case)
		//		%him%				him or her (matching case)
		//		%sir%				sir or ma'am (matching case)
		//		%man%				man or woman (matching case)
		//		%brother%			brother or sister (matching case)
		//		%%					%
		//		%1%					arg1
		//		%2%					...

		{	"plyCredit",					fnPlySetOld,	FN_PLY_CREDITS,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(plyCredit player credit)

		{	"plyDestroyed",					fnPlySetOld,		FN_PLY_DESTROYED, "",	NULL,	PPFLAG_SIDEEFFECTS, },
		//	(plyDestroyed player destroyed-text)

		{	"plyGetCredits",				fnPlyGetOld,		FN_PLY_CREDITS,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(plyGetCredits player) -> Credits left

		{	"plyGetGenome",					fnPlyGet,			FN_PLY_GENOME,
			"(plyGetGenome player) -> 'humanMale | 'humanFemale",
			"i",	0,	},

		{	"plyGetInsuranceClaims",		fnPlyGetOld,		FN_PLY_INSURANCE_CLAIMS,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(plyGetInsuranceClaims player) -> No of insurance claims

		{	"plyGetRedirectMessage",		fnPlyGetOld,		FN_PLY_REDIRECT_MESSAGE,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(plyGetRedirectMessage player)

		{	"plyInsure",					fnPlyGetOld,		FN_PLY_INSURE,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(plyInsure player)

		{	"plyIsInsured",					fnPlyGetOld,		FN_PLY_IS_INSURED,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(plyIsInsured player) -> True if insured

		{	"plyMessage",					fnPlySetOld,		FN_PLY_MESSAGE,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(plyMessage player message)

		{	"plyRedirectMessage",			fnPlySetOld,		FN_PLY_REDIRECT_MESSAGE,	"",		NULL,	PPFLAG_SIDEEFFECTS,	},
		//	(plyRedirectMessage player True/Nil)

		//	Debug functions
		//	---------------

		{	"dbgOutput",					fnDebug,		FN_DEBUG_OUTPUT,
			"(dbgOutput string [string]*)",		"*",	PPFLAG_SIDEEFFECTS,	},

		{	"dbgLog",						fnDebug,		FN_DEBUG_LOG,
			"(dbgLog string [string]*)",		"*",	PPFLAG_SIDEEFFECTS,	},

		{	"dbgFlush",						fnDebug,		FN_DEBUG_FLUSH,
			"(dbgFlush)",		"*",	PPFLAG_SIDEEFFECTS | PPFLAG_NOERRORS,	},
	};

#define EXTENSIONS_COUNT		(sizeof(g_Extensions) / sizeof(g_Extensions[0]))

inline CSpaceObject *GetObjArg (ICCItem *pArg) { return (CSpaceObject *)pArg->GetIntegerValue(); }
inline CShip *GetShipArg (ICCItem *pArg) { return ((CSpaceObject *)(pArg->GetIntegerValue()))->AsShip(); }
inline CStation *GetStationArg (ICCItem *pArg) { return (CStation *)pArg->GetIntegerValue(); }
inline CDockScreen *GetDockScreenArg (ICCItem *pArg) { return (CDockScreen *)pArg->GetIntegerValue(); }
inline CArmorClass *GetArmorClassArg (ICCItem *pArg) { return (CArmorClass *)pArg->GetIntegerValue(); }
inline CPlayerShipController *GetPlayerArg (ICCItem *pArg) { return (CPlayerShipController *)pArg->GetIntegerValue(); }

ALERROR InitCodeChainExtensions (CCodeChain &CC)

//	InitCodeChainExtensions
//
//	Registers extensions

	{
	ALERROR error;
	unsigned int i;

	for (i = 0; i < EXTENSIONS_COUNT; i++)
		if ((error = CC.RegisterPrimitive(&g_Extensions[i])))
			return error;

	return NOERROR;
	}

ICCItem *fnDebug (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnDebug
//
//	Debug functions

	{
	int i;
	CCodeChain *pCC = pEvalCtx->pCC;

	switch (dwData)
		{
		case FN_DEBUG_OUTPUT:
		case FN_DEBUG_LOG:
			{
			CString sResult;

			//	Append each of the arguments together

			for (i = 0; i < pArgs->GetCount(); i++)
				{
				if (pArgs->GetElement(i)->IsList())
					sResult.Append(pCC->Unlink(pArgs->GetElement(i)));
				else
					sResult.Append(pArgs->GetElement(i)->GetStringValue());
				}

			//	Output to console or log

			if (dwData == FN_DEBUG_OUTPUT)
				{
				g_pTrans->DebugConsoleOutput(sResult);
				return pCC->CreateTrue();
				}
			else
				{
				kernelDebugLogMessage(sResult.GetASCIIZPointer());
				return pCC->CreateString(sResult);
				}
			}

		case FN_DEBUG_FLUSH:
			{
			kernelDebugLogFlush();
			return pCC->CreateNil();
			}

		default:
			ASSERT(false);
			return pCC->CreateNil();
		}
	}

ICCItem *fnGameSet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnGameSet
//
//	Game functions

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	switch (dwData)
		{
		case FN_GAME_SET_CRAWL_IMAGE:
			g_pTrans->SetCrawlImage((DWORD)pArgs->GetElement(0)->GetIntegerValue());
			return pCC->CreateTrue();

		case FN_GAME_SET_CRAWL_TEXT:
			g_pTrans->SetCrawlText(pArgs->GetElement(0)->GetStringValue());
			return pCC->CreateTrue();

		default:
			ASSERT(false);
			return pCC->CreateNil();
		}
	}

ICCItem *fnScrItem (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnScrItem
//
//	Gets and sets items
//
//	(scrGetItem screen) => itemStruct
//	(scrRemoveItem screen count) => itemStruct

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	if (dwData == FN_SCR_REMOVE_ITEM)
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("ii"));
	else if (dwData == FN_SCR_REFRESH_ITEM)
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("il"));
	else
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("i"));
	if (pArgs->IsError())
		return pArgs;

	//	Convert the first argument into a dock screen object

	CDockScreen *pScreen = GetDockScreenArg(pArgs->GetElement(0));
	if (pScreen == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateError(LITERAL("Entry pointer expected:"), pArgs->GetElement(0));
		}

	//	Do the appropriate command

	CItem Result;
	switch (dwData)
		{
		case FN_SCR_GET_ITEM:
		case FN_SCR_GET_ITEM_LIST_CURSOR:
			if (pScreen->IsCurrentItemValid())
				{
				Result = pScreen->GetCurrentItem();
				pResult = CreateListFromItem(*pCC, Result);
				}
			else
				pResult = pCC->CreateNil();

			pArgs->Discard(pCC);
			break;

		case FN_SCR_REMOVE_ITEM:
			{
			if (pScreen->IsCurrentItemValid())
				{
				int iToDelete = pArgs->GetElement(1)->GetIntegerValue();

				Result = pScreen->GetCurrentItem();
				iToDelete = std::min(iToDelete, Result.GetCount());
				pScreen->DeleteCurrentItem(iToDelete);
				Result.SetCount(iToDelete);
				pResult = CreateListFromItem(*pCC, Result);
				}
			else
				pResult = pCC->CreateNil();

			pArgs->Discard(pCC);
			break;
			}

		case FN_SCR_REFRESH_ITEM:
			{
			CItem Item(CreateItemFromList(*pCC, pArgs->GetElement(1)));
			pArgs->Discard(pCC);
			CItemListManipulator *pItemList = &pScreen->GetItemListManipulator();
			pItemList->Refresh(Item);
			pResult = pCC->CreateNil();
			break;
			}

		default:
			ASSERT(FALSE);
		}

	//	Create the result

	return pResult;
	}

ICCItem *fnPlyGet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnPlyGet
//
//	Gets data about the player

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pResult;

	//	Convert the first argument into a player controller

	CPlayerShipController *pPlayer = GetPlayerArg(pArgs->GetElement(0));
	if (pPlayer == NULL)
		return pCC->CreateNil();

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_PLY_GENOME:
			if (pPlayer->GetGenome() == genomeHumanMale)
				pResult = pCC->CreateString(GENOME_HUMAN_MALE);
			else
				pResult = pCC->CreateString(GENOME_HUMAN_FEMALE);
			break;

		default:
			ASSERT(FALSE);
			pResult = pCC->CreateNil();
		}

	return pResult;
	}

ICCItem *fnPlyGetOld (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnPlyGetOld
//
//	Gets data about the player
//
//	(plyGetCredits player) -> Credits left

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("i"));
	if (pArgs->IsError())
		return pArgs;

	//	Convert the first argument into a player controller

	CPlayerShipController *pPlayer = GetPlayerArg(pArgs->GetElement(0));
	if (pPlayer == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateNil();
		}

	//	No longer needed

	pArgs->Discard(pCC);

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_PLY_CREDITS:
			pResult = pCC->CreateInteger(pPlayer->GetCredits());
			break;

		case FN_PLY_INSURE:
			pPlayer->Insure();
			pResult = pCC->CreateTrue();
			break;

		case FN_PLY_INSURANCE_CLAIMS:
			pResult = pCC->CreateInteger(pPlayer->GetInsuranceClaims());
			break;

		case FN_PLY_IS_INSURED:
			pResult = pCC->CreateBool(pPlayer->IsInsured());
			break;

		case FN_PLY_REDIRECT_MESSAGE:
			{
			const CString &sMsg = pPlayer->GetTrans()->GetRedirectMessage();
			if (!sMsg.IsBlank())
				pResult = pCC->CreateString(sMsg);
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_PLY_SHOW_HELP_REFUEL:
			pPlayer->ClearShowHelpRefuel();
			pResult = pCC->CreateTrue();
			break;

		default:
			ASSERT(FALSE);
		}

	return pResult;
	}

ICCItem *fnPlyComposeString (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnPlyComposeString
//
//	Composes a string

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	//	Convert the first argument into a player controller

	CPlayerShipController *pPlayer = GetPlayerArg(pArgs->GetElement(0));
	if (pPlayer == NULL)
		return pCC->CreateNil();

	//	Compose

	return pCC->CreateString(pPlayer->GetTrans()->ComposePlayerNameString(pArgs->GetElement(1)->GetStringValue(), pArgs));
	}

#if 0
ICCItem *fnPlySet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnPlySet
//
//	Sets player data

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pResult;

	//	Convert the first argument into a player controller

	CPlayerShipController *pPlayer = GetPlayerArg(pArgs->GetElement(0));
	if (pPlayer == NULL)
		return pCC->CreateNil();

	//	Do the appropriate command

	switch (dwData)
		{
		default:
			ASSERT(FALSE);
			pResult = pCC->CreateNil();
		}

	return pResult;
	}
#endif

ICCItem *fnPlySetOld (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnPlySetOld
//
//	Sets player data
//
//	(plyCharge player charge)

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("iv"));
	if (pArgs->IsError())
		return pArgs;

	//	Convert the first argument into a player controller

	CPlayerShipController *pPlayer = GetPlayerArg(pArgs->GetElement(0));
	if (pPlayer == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateNil();
		}

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_PLY_CHARGE:
			pPlayer->Charge(pArgs->GetElement(1)->GetIntegerValue());
			pArgs->Discard(pCC);
			pResult = pCC->CreateInteger(pPlayer->GetCredits());
			break;

		case FN_PLY_COMPOSE_STRING:
			{
			pResult = pCC->CreateString(pPlayer->GetTrans()->ComposePlayerNameString(pArgs->GetElement(1)->GetStringValue()));
			pArgs->Discard(pCC);
			break;
			}

		case FN_PLY_CREDITS:
			pPlayer->Payment(pArgs->GetElement(1)->GetIntegerValue());
			pArgs->Discard(pCC);
			pResult = pCC->CreateInteger(pPlayer->GetCredits());
			break;

		case FN_PLY_DESTROYED:
			{
			CString sText = pArgs->GetElement(1)->GetStringValue();
			pArgs->Discard(pCC);
			pPlayer->DestroyPlayer(sText);
			pResult = pCC->CreateTrue();
			break;
			}

		case FN_PLY_MESSAGE:
			{
			CString sText = pArgs->GetElement(1)->GetStringValue();
			pArgs->Discard(pCC);
			pPlayer->GetTrans()->DisplayMessage(sText);
			pResult = pCC->CreateTrue();
			break;
			}

		case FN_PLY_REDIRECT_MESSAGE:
			{
			bool bRedirect = !pArgs->GetElement(1)->IsNil();
			pArgs->Discard(pCC);
			pPlayer->GetTrans()->RedirectDisplayMessage(bRedirect);
			pResult = pCC->CreateTrue();
			break;
			}

		default:
			ASSERT(FALSE);
		}

	return pResult;
	}

ICCItem *fnScrGet (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnScrGet
//
//	Sets screen data
//
//	(scrGetCounter screen)

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("i"));
	if (pArgs->IsError())
		return pArgs;

	//	Convert the first argument into a dock screen object

	CDockScreen *pScreen = GetDockScreenArg(pArgs->GetElement(0));
	if (pScreen == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateError(LITERAL("Entry pointer expected:"), pArgs->GetElement(0));
		}

	//	Done with args

	pArgs->Discard(pCC);

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_SCR_COUNTER:
			pResult = pCC->CreateInteger(pScreen->GetCounter());
			break;

		case FN_SCR_EXIT_DOCK:
			pScreen->Undock();
			pResult = pCC->CreateTrue();
			break;

		case FN_SCR_INPUT_TEXT:
			pResult = pCC->CreateString(pScreen->GetTextInput());
			break;

		case FN_SCR_IS_FIRST_ON_INIT:
			pResult = pCC->CreateBool(pScreen->IsFirstOnInit());
			break;

		case FN_SCR_LIST_ENTRY:
			pResult = pScreen->GetCurrentListEntry();
			break;

		default:
			ASSERT(FALSE);
		}

	return pResult;
	}

ICCItem *fnScrSet (CEvalContext *pEvalCtx, ICCItem *pArgs, DWORD dwData)

//	fnScrSet
//
//	Sets screen data

	{
	CCodeChain *pCC = pEvalCtx->pCC;

	//	Convert the first argument into a dock screen object

	CDockScreen *pScreen = GetDockScreenArg(pArgs->GetElement(0));
	if (pScreen == NULL)
		return pCC->CreateError(CONSTLIT("Screen expected:"), pArgs->GetElement(0));

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_SCR_SET_DISPLAY_TEXT:
			{
			if (pScreen->SetDisplayText(
					pArgs->GetElement(1)->GetStringValue(), 
					pArgs->GetElement(2)->GetStringValue()) != NOERROR)
				return pCC->CreateError(CONSTLIT("Invalid display ID"), pArgs->GetElement(1));

			return pCC->CreateTrue();
			}

		case FN_SCR_ACTION_LABEL:
			{
			int iAction = pArgs->GetElement(1)->GetIntegerValue();
			CString sLabel = pArgs->GetElement(2)->GetStringValue();
			CString sKey;
			if (pArgs->GetCount() > 3)
				sKey = pArgs->GetElement(3)->GetStringValue();

			pScreen->SetActionLabel(iAction, sLabel, sKey);
			return pCC->CreateTrue();
			}

		default:
			ASSERT(false);
			return pCC->CreateNil();
		}
	}

ICCItem *fnScrSetOld (CEvalContext *pEvalCtx, ICCItem *pArguments, DWORD dwData)

//	fnScrSetOld
//
//	Sets screen data
//
//	(scrShowPane scr pane)

	{
	CCodeChain *pCC = pEvalCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	if (dwData == FN_SCR_LIST_FILTER)
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("iv"));
	else if (dwData == FN_SCR_COUNTER)
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("ii"));
	else if (dwData == FN_SCR_SHOW_SCREEN)
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("is*"));
	else if (dwData == FN_SCR_ENABLE_ACTION || dwData == FN_SCR_ACTION_LABEL || dwData == FN_SCR_SHOW_ACTION)
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("iiv"));
	else
		pArgs = pCC->EvaluateArgs(pEvalCtx, pArguments, CONSTLIT("is"));
	if (pArgs->IsError())
		return pArgs;

	//	Convert the first argument into a dock screen object

	CDockScreen *pScreen = GetDockScreenArg(pArgs->GetElement(0));
	if (pScreen == NULL)
		{
		pArgs->Discard(pCC);
		return pCC->CreateError(LITERAL("Entry pointer expected:"), pArgs->GetElement(0));
		}

	//	Do the appropriate command

	switch (dwData)
		{
		case FN_SCR_SHOW_PANE:
			pScreen->ShowPane(pArgs->GetElement(1)->GetStringValue());
			pArgs->Discard(pCC);
			pResult = pCC->CreateTrue();
			break;

		case FN_SCR_SHOW_SCREEN:
			{
			CString sPane;
			ICCItem *pPane = pArgs->GetElement(2);
			if (pPane && !pPane->IsNil())
				sPane = pPane->GetStringValue();
			
			pScreen->ShowScreen(pArgs->GetElement(1)->GetStringValue(), sPane);
			pArgs->Discard(pCC);
			pResult = pCC->CreateTrue();
			break;
			}

		case FN_SCR_DESC:
			pScreen->SetDescription(pArgs->GetElement(1)->GetStringValue());
			pArgs->Discard(pCC);
			pResult = pCC->CreateTrue();
			break;

		case FN_SCR_COUNTER:
			pScreen->SetCounter(pArgs->GetElement(1)->GetIntegerValue());
			pArgs->Discard(pCC);
			pResult = pCC->CreateTrue();
			break;

		case FN_SCR_ENABLE_ACTION:
			{
			int iAction = pArgs->GetElement(1)->GetIntegerValue();
			bool bEnable = !pArgs->GetElement(2)->IsNil();
			pArgs->Discard(pCC);
			pScreen->EnableAction(iAction, bEnable);
			pResult = pCC->CreateTrue();
			break;
			}

		case FN_SCR_SHOW_ACTION:
			{
			int iAction = pArgs->GetElement(1)->GetIntegerValue();
			bool bShow = !pArgs->GetElement(2)->IsNil();
			pArgs->Discard(pCC);
			pScreen->ShowAction(iAction, bShow);
			pResult = pCC->CreateTrue();
			break;
			}

		case FN_SCR_INPUT_TEXT:
			pScreen->SetTextInput(pArgs->GetElement(1)->GetStringValue());
			pArgs->Discard(pCC);
			pResult = pCC->CreateTrue();
			break;

		case FN_SCR_LIST_FILTER:
			{
			if (pArgs->GetElement(1)->IsFunction())
				{
				CItemCriteria Filter;
				Filter.pFilter = pArgs->GetElement(1)->Reference();
				pArgs->Discard(pCC);

				pScreen->SetListFilter(Filter);
				}
			else
				{
				CString sFilter = pArgs->GetElement(1)->GetStringValue();
				pArgs->Discard(pCC);

				CItemCriteria Filter;
				CItem::ParseCriteria(sFilter, &Filter);

				pScreen->SetListFilter(Filter);
				}

			pResult = pCC->CreateTrue();
			break;
			}

		default:
			ASSERT(FALSE);
		}

	return pResult;
	}

