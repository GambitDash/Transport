//	CSovereign.cpp
//
//	CSovereign class

#include "PreComp.h"

#define LANGUAGE_TAG						CONSTLIT("Language")
#define RELATIONSHIPS_TAG					CONSTLIT("Relationships")
#define RELATIONSHIP_TAG					CONSTLIT("Relationship")

#define ALIGNMENT_ATTRIB					CONSTLIT("alignment")
#define ID_ATTRIB							CONSTLIT("id")
#define TEXT_ATTRIB							CONSTLIT("text")
#define SOVEREIGN_ATTRIB					CONSTLIT("sovereign")
#define DISPOSITION_ATTRIB					CONSTLIT("disposition")

#define CONSTRUCTIVE_CHAOS_ALIGN			CONSTLIT("constructive chaos")
#define CONSTRUCTIVE_ORDER_ALIGN			CONSTLIT("constructive order")
#define NEUTRAL_ALIGN						CONSTLIT("neutral")
#define DESTRUCTIVE_ORDER_ALIGN				CONSTLIT("destructive order")
#define DESTRUCTIVE_CHAOS_ALIGN				CONSTLIT("destructive chaos")

#define DISP_FRIEND							CONSTLIT("friend")
#define DISP_NEUTRAL						CONSTLIT("neutral")
#define DISP_ENEMY							CONSTLIT("enemy")

static char *g_DefaultText[] =
	{
	//	0
	"",

	//	msgAttack
	"Attack target",

	//	msgDestroyBroadcast
	"",

	//	msgHitByFriendlyFire
	"",

	//	msgQueryEscortStatus
	"",

	//	msgQueryFleetStatus
	"",

	//	msgEscortAttacked
	"",

	//	msgEscortReportingIn
	"",

	//	msgWatchTargets
	"Watch your targets!",

	//	msgNiceShooting
	"Nice shooting!",

	//	msgFormUp
	"Form up!",

	//	msgBreakAndAttack
	"Break & attack",

	//	msgQueryCommunications
	"",

	//	msgAbortAttac
	"Cancel attack"
	};

#define DEFAULT_TEXT_COUNT					(sizeof(g_DefaultText) / sizeof(g_DefaultText[0]))

static CString *g_pDefaultText = NULL;

static char g_UNIDAttrib[] = "UNID";
static char g_NameAttrib[] = "name";

CSovereign::Disposition g_DispositionTable[5][5] =
	{
		//	WE are constructive chaos...
		{
			CSovereign::dispFriend,			//	THEY are constructive chaos
			CSovereign::dispFriend,			//	THEY are constructive order
			CSovereign::dispNeutral,		//	THEY are neutral
			CSovereign::dispEnemy,			//	THEY are destructive order
			CSovereign::dispEnemy,			//	THEY are destructive chaos
		},

		//	WE are constructive order...
		{
			CSovereign::dispNeutral,		//	THEY are constructive chaos
			CSovereign::dispFriend,			//	THEY are constructive order
			CSovereign::dispNeutral,		//	THEY are neutral
			CSovereign::dispEnemy,			//	THEY are destructive order
			CSovereign::dispEnemy,			//	THEY are destructive chaos
		},

		//	WE are neutral...
		{
			CSovereign::dispNeutral,		//	THEY are constructive chaos
			CSovereign::dispNeutral,		//	THEY are constructive order
			CSovereign::dispNeutral,		//	THEY are neutral
			CSovereign::dispNeutral,		//	THEY are destructive order
			CSovereign::dispEnemy,			//	THEY are destructive chaos
		},

		//	WE are destructive order...
		{
			CSovereign::dispEnemy,			//	THEY are constructive chaos
			CSovereign::dispEnemy,			//	THEY are constructive order
			CSovereign::dispNeutral,		//	THEY are neutral
			CSovereign::dispNeutral,		//	THEY are destructive order
			CSovereign::dispEnemy,			//	THEY are destructive chaos
		},

		//	WE are destructive chaos...
		{
			CSovereign::dispEnemy,			//	THEY are constructive chaos
			CSovereign::dispEnemy,			//	THEY are constructive order
			CSovereign::dispEnemy,			//	THEY are neutral
			CSovereign::dispEnemy,			//	THEY are destructive order
			CSovereign::dispEnemy,			//	THEY are destructive chaos
		},
	};

CSovereign::CSovereign (void) : 
		m_Language(TRUE, FALSE),
		m_pEnemyObjectsSystem(NULL),
		m_pFirstRelationship(NULL),
		m_pInitialRelationships(NULL)

//	CSovereign constructor

	{
	}

CSovereign::~CSovereign (void)

//	CSovereign destructor

	{
	if (m_pInitialRelationships)
		delete m_pInitialRelationships;

	DeleteRelationships();
	}

void CSovereign::DeleteRelationships (void)

//	DeleteRelationships
//
//	Delete all relationships

	{
	SRelationship *pRel = m_pFirstRelationship;
	while (pRel)
		{
		SRelationship *pDelete = pRel;
		pRel = pRel->pNext;
		delete pDelete;
		}

	m_pFirstRelationship = NULL;
	}

CSovereign::SRelationship *CSovereign::FindRelationship (CSovereign *pSovereign)

//	FindRelationship
//
//	Finds a specific relationship with the given sovereign (or NULL
//	if none is found)

	{
	SRelationship *pRel = m_pFirstRelationship;
	while (pRel && pRel->pSovereign != pSovereign)
		pRel = pRel->pNext;

	return pRel;
	}

CSovereign::Disposition CSovereign::GetDispositionTowards (CSovereign *pSovereign)

//	GetDispositionTowards
//
//	Returns how this sovereign feels about the given sovereign

	{
	//	We are always friendly to ourselves

	if (pSovereign == this)
		return dispFriend;

	//	See if we have a specific relationship. If so, then return the
	//	disposition there.

	SRelationship *pRel = FindRelationship(pSovereign);
	if (pRel)
		return pRel->iDisp;

	//	Get the alignment of the other. We treat NULL as Neutral

	AlignmentTypes iAlign;
	if (pSovereign == NULL)
		iAlign = alignNeutral;
	else
		iAlign = pSovereign->GetAlignment();

	//	Consult the table

	return g_DispositionTable[m_iAlignment][iAlign];
	}

const CString &CSovereign::GetText (MessageTypes iMsg)

//	GetText
//
//	Get the text associated with the given message

	{
	//	Load the string array

	if (g_pDefaultText == NULL)
		{
		g_pDefaultText = new CString [DEFAULT_TEXT_COUNT];
		for (int i = 0; i < DEFAULT_TEXT_COUNT; i++)
			g_pDefaultText[i] = CString(g_DefaultText[i], strlen(g_DefaultText[i]), true);
		}

	//	Look up string

	CString *pString;
	if (m_Language.Lookup(iMsg, (CObject **)&pString) == NOERROR)
		return *pString;

	//	Get default text

	if ((int)iMsg > 0 && (int)iMsg < DEFAULT_TEXT_COUNT)
		return g_pDefaultText[iMsg];
	else
		return g_pDefaultText[0];
	}

void CSovereign::InitEnemyObjectList (CSystem *pSystem)

//	InitEnemyObjectList
//
//	Compiles and caches a list of enemy objects in the system

	{
	int i;

	if (m_pEnemyObjectsSystem != pSystem)
		{
		m_EnemyObjects.RemoveAll();

		for (i = 0; i < pSystem->GetObjectCount(); i++)
			{
			CSpaceObject *pObj = pSystem->GetObject(i);

			if (pObj 
					&& pObj->ClassCanAttack()
					&& IsEnemy(pObj->GetSovereign()))
				m_EnemyObjects.FastAdd(pObj);
			}

		m_pEnemyObjectsSystem = pSystem;
		}
	}

void CSovereign::InitRelationships (void)

//	InitRelationships
//
//	Initialize relationships from XML element

	{
	int i;

	DeleteRelationships();

	if (m_pInitialRelationships)
		{
		for (i = 0; i < m_pInitialRelationships->GetContentElementCount(); i++)
			{
			CXMLElement *pRelDesc = m_pInitialRelationships->GetContentElement(i);
			CSovereign *pTarget = g_pUniverse->FindSovereign(pRelDesc->GetAttributeInteger(SOVEREIGN_ATTRIB));
			if (pTarget)
				{
				CString sDisposition = pRelDesc->GetAttribute(DISPOSITION_ATTRIB);
				if (strEquals(sDisposition, DISP_FRIEND))
					SetDispositionTowards(pTarget, dispFriend);
				else if (strEquals(sDisposition, DISP_NEUTRAL))
					SetDispositionTowards(pTarget, dispNeutral);
				else if (strEquals(sDisposition, DISP_ENEMY))
					SetDispositionTowards(pTarget, dispEnemy);
				}
			}
		}
	}

ALERROR CSovereign::OnBindDesign (SDesignLoadCtx &Ctx)

//	OnBindDesign
//
//	Bind design

	{
	InitRelationships();
	return NOERROR;
	}

ALERROR CSovereign::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Create from XML

	{
	int i;

	//	Initialize

	m_sName = pDesc->GetAttribute(CONSTLIT(g_NameAttrib));

	//	Alignment

	CString sAlignment = pDesc->GetAttribute(ALIGNMENT_ATTRIB);
	if (strEquals(sAlignment, CONSTRUCTIVE_CHAOS_ALIGN))
		m_iAlignment = alignConstructiveChaos;
	else if (strEquals(sAlignment, CONSTRUCTIVE_ORDER_ALIGN))
		m_iAlignment = alignConstructiveOrder;
	else if (strEquals(sAlignment, NEUTRAL_ALIGN))
		m_iAlignment = alignNeutral;
	else if (strEquals(sAlignment, DESTRUCTIVE_ORDER_ALIGN))
		m_iAlignment = alignDestructiveOrder;
	else if (strEquals(sAlignment, DESTRUCTIVE_CHAOS_ALIGN))
		m_iAlignment = alignDestructiveChaos;
	else
		return ERR_FAIL;

	//	Load language

	CXMLElement *pLanguage = pDesc->GetContentElementByTag(LANGUAGE_TAG);
	if (pLanguage)
		{
		int iCount = pLanguage->GetContentElementCount();
		for (i = 0; i < iCount; i++)
			{
			CXMLElement *pItem = pLanguage->GetContentElement(i);
			CString *pText = new CString(pItem->GetAttribute(TEXT_ATTRIB));
			DWORD dwID = pItem->GetAttributeInteger(ID_ATTRIB);
			if (dwID != 0)
				m_Language.AddEntry(dwID, pText);
			}
		}

	//	Load relationships

	m_pInitialRelationships = pDesc->GetContentElementByTag(RELATIONSHIPS_TAG);
	if (m_pInitialRelationships)
		m_pInitialRelationships = m_pInitialRelationships->OrphanCopy();

	//	Done

	return NOERROR;
	}

void CSovereign::OnReadFromStream (SUniverseLoadCtx &Ctx)

//	ReadFromStream
//
//	Read from stream
//
//	For each relationship:
//	DWORD		Unid of target relationship (or NULL if no more)
//	DWORD		Disposition
//	DWORD		Flags

	{
	DWORD dwUNID;
	DWORD dwLoad;

	DeleteRelationships();
	SRelationship **pNext = &m_pFirstRelationship;

	//	Keep reading until there are no more

	Ctx.pStream->Read((char *)&dwUNID, sizeof(DWORD));
	while (dwUNID)
		{
		SRelationship *pRel = new SRelationship;
		pRel->pSovereign = g_pUniverse->FindSovereign(dwUNID);

		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));
		pRel->iDisp = (Disposition)dwLoad;

		Ctx.pStream->Read((char *)&dwLoad, sizeof(DWORD));

		//	Add to list

		if (pRel->pSovereign)
			{
			(*pNext) = pRel;
			pNext = &pRel->pNext;
			}
		else
			delete pRel;

		//	Next

		Ctx.pStream->Read((char *)&dwUNID, sizeof(DWORD));
		}

	//	Terminate

	(*pNext) = NULL;
	}

void CSovereign::OnReinit (void)

//	OnReinit
//
//	Reinitialize

	{
	InitRelationships();
	}

void CSovereign::OnWriteToStream (IWriteStream *pStream)

//	OnWriteToStream
//
//	Writes out the variable portions of the item type
//
//	For each relationship:
//	DWORD		Unid of target relationship (or NULL if no more)
//	DWORD		Disposition
//	DWORD		Flags

	{
	DWORD dwSave;

	SRelationship *pRel = m_pFirstRelationship;
	while (pRel)
		{
		dwSave = pRel->pSovereign->GetUNID();
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		dwSave = pRel->iDisp;
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		//	Flags
		dwSave = 0;
		pStream->Write((char *)&dwSave, sizeof(DWORD));

		pRel = pRel->pNext;
		}

	//	Write out end

	dwSave = 0;
	pStream->Write((char *)&dwSave, sizeof(DWORD));
	}

void CSovereign::SetDispositionTowards (CSovereign *pSovereign, Disposition iDisp)

//	SetDispositionTowards
//
//	Sets the disposition towards the given sovereign

	{
	SRelationship *pRel = FindRelationship(pSovereign);
	if (pRel == NULL)
		{
		pRel = new SRelationship;
		pRel->pSovereign = pSovereign;
		pRel->pNext = m_pFirstRelationship;
		m_pFirstRelationship = pRel;
		}

	pRel->iDisp = iDisp;

	//	Flush cache of enemy objects

	FlushEnemyObjectCache();
	}

