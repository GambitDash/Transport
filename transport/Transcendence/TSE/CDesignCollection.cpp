//	CDesignCollection.cpp
//
//	CDesignCollection class

#include "PreComp.h"

#define ADVENTURE_DESC_TAG					CONSTLIT("AdventureDesc")
#define STAR_SYSTEM_TOPOLOGY_TAG			CONSTLIT("StarSystemTopology")
#define SYSTEM_TOPOLOGY_TAG					CONSTLIT("SystemTopology")

#define UNID_ATTRIB							CONSTLIT("UNID")
#define VERSION_ATTRIB						CONSTLIT("version")

CDesignCollection::CDesignCollection (void) :
		m_Extensions(FALSE, TRUE),
		m_dwNextAnonymousUNID(0xf0000001)

//	CDesignCollection construtor

	{
	}

CDesignCollection::~CDesignCollection (void)

//	CDesignCollection destructor

	{
	RemoveAll();
	}

ALERROR CDesignCollection::AddEntry (SDesignLoadCtx &Ctx, CDesignType *pEntry)

//	AddEntry
//
//	Adds an entry to the collection

	{
	ALERROR error;

	DWORD dwUNID = pEntry->GetUNID();

	//	If this is an extension, then add to the appropriate extension

	CDesignTable *pTable = NULL;
	if (Ctx.pExtension)
		{
		pTable = &Ctx.pExtension->Table;

		//	If the UNID of the entry does not belong to the extension, then make sure it
		//	overrides a valid base entry

		if ((dwUNID & UNID_DOMAIN_AND_MODULE_MASK) != (Ctx.pExtension->dwUNID & UNID_DOMAIN_AND_MODULE_MASK))
			{
			//	Cannot override AdventureDesc

			if (pEntry->GetType() == designAdventureDesc)
				{
				Ctx.sError = CONSTLIT("<AdventureDesc> UNID must be part of extension.");
				return ERR_FAIL;
				}

			//	Make sure we override a base type

			else if (m_Base.FindByUNID(dwUNID) == NULL)
				{
				Ctx.sError = strPatternSubst(CONSTLIT("Invalid UNID: %x [does not match extension UNID or override base type]"), dwUNID);
				return ERR_FAIL;
				}
			}
		}

	//	Otherwise, add to the base design types

	else
		pTable = &m_Base;

	//	Can't have a duplicate

	if (pTable->FindByUNID(dwUNID))
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Duplicate UNID: %x"), dwUNID);
		return ERR_FAIL;
		}

	//	Add

	if (error = pTable->AddEntry(pEntry))
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Error adding design entry UNID: %x"), dwUNID);
		return error;
		}

	return NOERROR;
	}

ALERROR CDesignCollection::BeginLoadAdventure (SDesignLoadCtx &Ctx, CAdventureDesc *pAdventure)

//	BeginLoadAdventure
//
//	Begin loading an adventure (not just the desc)

	{
	ASSERT(pAdventure);

	SExtensionDesc *pEntry = FindExtension(pAdventure->GetExtensionUNID());
	if (pEntry == NULL)
		{
		ASSERT(false);
		Ctx.sError = CONSTLIT("ERROR: Unexpectedly unable to find adventure");
		return ERR_FAIL;
		}

	pEntry->bLoaded = true;

	//	Set context

	Ctx.pExtension = pEntry;

	return NOERROR;
	}

ALERROR CDesignCollection::BeginLoadAdventureDesc (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	BeginLoadAdventureDesc
//
//	Adds a new adventure extension to our internal list and sets Ctx.pExtension

	{
	ALERROR error;
	SExtensionDesc *pEntry;

	//	Load the structure

	if (error = LoadExtensionDesc(Ctx, pDesc, &pEntry))
		return error;

	pEntry->iType = extAdventure;
	pEntry->bEnabled = false;
	pEntry->bLoaded = false;

	//	Add it

	m_Extensions.AddEntry(pEntry->dwUNID, (CObject *)pEntry);

	//	Set context

	Ctx.pExtension = pEntry;
	Ctx.bLoadAdventureDesc = true;

	return NOERROR;
	}

ALERROR CDesignCollection::BeginLoadExtension (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	BeginLoadExtension
//
//	Adds a new extension to our internal list and sets Ctx.pExtension

	{
	ALERROR error;
	SExtensionDesc *pEntry;

	//	Load the structure

	if (error = LoadExtensionDesc(Ctx, pDesc, &pEntry))
		return error;

	pEntry->iType = extExtension;
	pEntry->bEnabled = true;
	pEntry->bLoaded = true;

	//	Add it

	m_Extensions.AddEntry(pEntry->dwUNID, (CObject *)pEntry);

	//	Set context

	Ctx.pExtension = pEntry;

	return NOERROR;
	}

ALERROR CDesignCollection::BindDesign (SDesignLoadCtx &Ctx)

//	BindDesign
//
//	Bind the design collection so that design types point the appropriate
//	pointers by UNID

	{
	ALERROR error;
	int i, j;

	//	Reset the bind tables

	m_AllTypes.RemoveAll();
	for (i = 0; i < designCount; i++)
		m_ByType[i].RemoveAll();

	//	We start with all the base types

	for (i = 0; i < m_Base.GetCount(); i++)
		m_AllTypes.AddEntry(m_Base.GetEntry(i));

	//	Start with base topology

	m_pTopology = &m_BaseTopology;
	m_pAdventureExtension = NULL;

	//	Now add all enabled extensions

	for (i = 0; i < GetExtensionCount(); i++)
		{
		SExtensionDesc *pExtension = GetExtension(i);

		if (pExtension->bEnabled)
			{
			//	Add design elements in extension

			for (j = 0; j < pExtension->Table.GetCount(); j++)
				m_AllTypes.AddOrReplaceEntry(pExtension->Table.GetEntry(j));

			//	Handle adventure extensions

			if (pExtension->iType == extAdventure)
				{
				//	Keep track of extension

				m_pAdventureExtension = pExtension;

				//	Add topology

				m_pTopology = &pExtension->Topology;
				}
			}
		else
			{
			if (pExtension->iType == extAdventure)
				{
				//	Adventure desc elements are added even if not enabled

				for (j = 0; j < pExtension->Table.GetCount(); j++)
					{
					CDesignType *pEntry = pExtension->Table.GetEntry(j);
					if (pEntry->GetType() == designAdventureDesc)
						m_AllTypes.AddOrReplaceEntry(pEntry);
					}
				}
			}
		}

	//	Initialize the byType lists

	for (i = 0; i < m_AllTypes.GetCount(); i++)
		{
		CDesignType *pEntry = m_AllTypes.GetEntry(i);
		m_ByType[pEntry->GetType()].AddEntry(pEntry);
		}

	//	Now call Bind on all active design entries

	for (i = 0; i < m_AllTypes.GetCount(); i++)
		{
		CDesignType *pEntry = m_AllTypes.GetEntry(i);
		if (error = pEntry->BindDesign(Ctx))
			return error;
		}

	//	Check to make sure we have at least one topology node

	if (m_pTopology->GetRootNodeCount() == 0)
		{
		Ctx.sError = CONSTLIT("No topology nodes found");
		return ERR_FAIL;
		}

	return NOERROR;
	}

SExtensionDesc *CDesignCollection::FindExtension (DWORD dwUNID)

//	FindExtension
//
//	Find the entry for the given extension

	{
	//	Look for an entry for the extension. If we find it, then we're done

	SExtensionDesc *pEntry;
	if (m_Extensions.Lookup(dwUNID, (CObject **)&pEntry) == NOERROR)
		return pEntry;

	//	Otherwise, we need to allocate a new entry and add it

	pEntry = new SExtensionDesc;
	pEntry->dwUNID = dwUNID;
	pEntry->bEnabled = true;

	m_Extensions.AddEntry(dwUNID, (CObject *)pEntry);
	return pEntry;
	}

bool CDesignCollection::IsAdventureExtensionBound (DWORD dwUNID)

//	IsAdventureExtensionBound
//
//	Returns TRUE if we have bound on the given adventure extension

	{
	if (m_pAdventureExtension)
		return (m_pAdventureExtension->dwUNID == dwUNID);
	else
		return (dwUNID == 0);
	}

bool CDesignCollection::IsAdventureExtensionLoaded (DWORD dwUNID)

//	IsAdventureExtensionLoaded
//
//	Returns TRUE if adventure extension is loaded

	{
	//	0 = no extension
	if (dwUNID == 0)
		return true;

	SExtensionDesc *pExt = FindExtension(dwUNID);
	if (pExt == NULL)
		{
		ASSERT(false);
		return false;
		}

	ASSERT(pExt->iType == extAdventure);
	return pExt->bLoaded;
	}

ALERROR CDesignCollection::LoadDesignType (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadDesignType
//
//	Loads a standard design type

	{
	ALERROR error;
	CDesignType *pEntry;

	try
		{
		if (error = CDesignType::CreateFromXML(Ctx, pDesc, &pEntry))
			return error;
		}
	catch (...)
		{
		char szBuffer[1024];
		sprintf(szBuffer, "Crash loading: %x", pDesc->GetAttributeInteger(CONSTLIT("UNID")));
		Ctx.sError = CString(szBuffer);
		return ERR_FAIL;
		}

	if (error = AddEntry(Ctx, pEntry))
		{
		delete pEntry;
		return error;
		}

	return NOERROR;
	}

ALERROR CDesignCollection::LoadEntryFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	LoadEntryFromXML
//
//	Load an entry into the collection

	{
	ALERROR error;

	//	Load topology

	if (strEquals(pDesc->GetTag(), STAR_SYSTEM_TOPOLOGY_TAG)
			|| strEquals(pDesc->GetTag(), SYSTEM_TOPOLOGY_TAG))
		{
		if (Ctx.pExtension)
			//	If we're in an extension (Adventure) then load into the extension
			error = Ctx.pExtension->Topology.LoadFromXML(Ctx, pDesc);
		else
			//	Otherwise, load into the base game
			error = m_BaseTopology.LoadFromXML(Ctx, pDesc);

		if (error)
			return error;
		}

	//	Load standard design elements

	else
		{
		CDesignType *pEntry;

		try
			{
			if (error = CDesignType::CreateFromXML(Ctx, pDesc, &pEntry))
				return error;
			}
		catch (...)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Crash loading: %x"), pDesc->GetAttributeInteger(CONSTLIT("UNID")));
			return ERR_FAIL;
			}

		if (error = AddEntry(Ctx, pEntry))
			{
			delete pEntry;
			return error;
			}
		}

	return NOERROR;
	}

ALERROR CDesignCollection::LoadExtensionDesc (SDesignLoadCtx &Ctx, CXMLElement *pDesc, SExtensionDesc **retpExtension)

//	LoadExtensionDesc
//
//	Loads a new extension descriptor

	{
	//	Load version

	DWORD dwVersion = ::LoadExtensionVersion(pDesc->GetAttribute(VERSION_ATTRIB));
	if (dwVersion == 0)
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Unable to load extension: incompatible version: %s"), pDesc->GetAttribute(VERSION_ATTRIB).GetASCIIZPointer());
		return ERR_FAIL;
		}

	//	Load UNID

	DWORD dwUNID = pDesc->GetAttributeInteger(UNID_ATTRIB);
	DWORD dwDomain = (dwUNID & 0xF0000000);
	if (dwDomain < 0xA0000000 || dwDomain > 0xEFFFFFFF)
		{
		Ctx.sError = CONSTLIT("Unable to load extension: invalid extension UNID");
		return ERR_FAIL;
		}

	//	Create structure

	SExtensionDesc *pEntry = new SExtensionDesc;
	pEntry->dwUNID = dwUNID;
	pEntry->dwVersion = dwVersion;
	pEntry->bEnabled = true;

	//	Done

	*retpExtension = pEntry;

	return NOERROR;
	}

void CDesignCollection::RemoveAll (void)

//	RemoveAll
//
//	Remove all entries

	{
	int i;

	//	Delete the base types

	m_Base.DeleteAll();

	//	Delete all extensions

	for (i = 0; i < GetExtensionCount(); i++)
		{
		SExtensionDesc *pEntry = GetExtension(i);
		pEntry->Table.DeleteAll();
		delete pEntry;
		}

	m_Extensions.RemoveAll();
	}

void CDesignCollection::SelectAdventure (DWORD dwUNID)

//	SelectAdventure
//
//	Enable the given adventure and disable all other adventures

	{
	int i;

	for (i = 0; i < GetExtensionCount(); i++)
		{
		SExtensionDesc *pEntry = GetExtension(i);
		if (pEntry->iType == extAdventure)
			pEntry->bEnabled = (pEntry->dwUNID == dwUNID);
		}
	}

//	CDesignList ---------------------------------------------------------------

const int ALLOC_INCREMENT =							128;

CDesignList::CDesignList (void) :
		m_iCount(0),
		m_iAlloc(0),
		m_pList(NULL)

//	CDesignList constructor

	{
	}

CDesignList::~CDesignList (void)

//	CDesignList destructor

	{
	RemoveAll();
	}

void CDesignList::AddEntry (CDesignType *pType)

//	AddEntry
//
//	Adds a new entry to the end of the list

	{
	if (m_iCount == m_iAlloc)
		{
		int iNewAlloc = m_iAlloc + ALLOC_INCREMENT;
		CDesignType **pNewList = new CDesignType *[iNewAlloc];

		if (m_pList)
			{
			memcpy((char *)pNewList, (char *)m_pList, (m_iCount * sizeof(m_pList[0])));
			delete [] m_pList;
			}

		m_pList = pNewList;
		m_iAlloc = iNewAlloc;
		}

	m_pList[m_iCount++] = pType;
	}

void CDesignList::DeleteAll (void)

//	DeleteAll
//
//	Delete all entries from the list

	{
	int i;

	for (i = 0; i < GetCount(); i++)
		delete GetEntry(i);

	RemoveAll();
	}

void CDesignList::RemoveAll (void)

//	RemoveAll
//
//	Remove all entries (without freeing the structures)

	{
	if (m_pList)
		{
		delete [] m_pList;
		m_pList = NULL;
		m_iAlloc = 0;
		m_iCount = 0;
		}
	}

//	CDesignTable --------------------------------------------------------------

ALERROR CDesignTable::AddEntry (CDesignType *pEntry)

//	AddEntry
//
//	Adds an entry to the table

	{
	return m_Table.AddEntry(pEntry->GetUNID(), (CObject *)pEntry);
	}

ALERROR CDesignTable::AddOrReplaceEntry (CDesignType *pEntry, CDesignType **retpOldEntry)

//	AddOrReplaceEntry
//
//	Adds or replaces an entry

	{
	return m_Table.ReplaceEntry(pEntry->GetUNID(), (CObject *)pEntry, TRUE, (CObject **)retpOldEntry);
	}

void CDesignTable::DeleteAll (void)

//	DeleteAll
//
//	Removes all entries and deletes the object that they point to

	{
	int i;

	for (i = 0; i < GetCount(); i++)
		delete GetEntry(i);

	RemoveAll();
	}

CDesignType *CDesignTable::FindByUNID (DWORD dwUNID) const

//	FindByUNID
//
//	Returns a pointer to the given entry or NULL

	{
	CDesignType *pObj;
	if (m_Table.Lookup((int)dwUNID, (CObject **)&pObj) == NOERROR)
		return pObj;
	else
		return NULL;
	}

