//	Extensions.cpp
//
//	Loads extensions
//
//	VERSION HISTORY
//
//	 0: Unknown version
//
//	 1: 95-0.96b
//		Original Extensions
//
//	 2: 0.97
//		Changed gStation to gSource

#include "PreComp.h"

#include "Kernel.h"
#include "CFile.h"
#include "CFileDirectory.h"

#define TRANSCENDENCE_EXTENSION_TAG					CONSTLIT("TranscendenceExtension")
#define TRANSCENDENCE_ADVENTURE_TAG					CONSTLIT("TranscendenceAdventure")

#define EXTENSIONS_FOLDER							CONSTLIT("Extensions")
#define EXTENSIONS_FILTER							CONSTLIT("*.*")
#define EXTENSION_XML								CONSTLIT("xml")
#define EXTENSION_TDB								CONSTLIT("tdb")

#define UNID_ATTRIB									CONSTLIT("UNID")
#define VERSION_ATTRIB								CONSTLIT("version")

class CSavedDesignLoadCtx
	{
	public:
		CSavedDesignLoadCtx (SDesignLoadCtx &Ctx) :
				m_pCtx(&Ctx),
				m_sSavedResDb(Ctx.sResDb),
				m_pSavedResDb(Ctx.pResDb)
			{ }

		~CSavedDesignLoadCtx (void)
			{
			m_pCtx->sResDb = m_sSavedResDb;
			m_pCtx->pResDb = m_pSavedResDb;
			}

	private:
		SDesignLoadCtx *m_pCtx;
		CString m_sSavedResDb;
		CResourceDb *m_pSavedResDb;
	};

ALERROR CUniverse::InitExtensions (SDesignLoadCtx &Ctx, const CString &sFilespec)

//	InitExtensions
//
//	Load extensions into the game

	{
	ALERROR error;

	CString sPath = pathGetPath(sFilespec);
	CString sExtensionsPath = pathAddComponent(sPath, EXTENSIONS_FOLDER);
	if (!pathExists(sExtensionsPath))
		return NOERROR;

	CSymbolTable LoadedExtensions(FALSE, TRUE);

	//	Load all extensions in the folder

	CFileDirectory Dir(sExtensionsPath);
	while (Dir.HasMore())
		{
		CString sFilepath = pathAddComponent(sExtensionsPath, Dir.GetNext());
		CString sFileExtension = pathGetExtension(sFilepath);
		CString sExtensionRoot = pathStripExtension(sFilepath);

		//	If this is not .xml or .tdb, continue

		if (!strEquals(sFileExtension, EXTENSION_XML)
				&& !strEquals(sFileExtension, EXTENSION_TDB))
			continue;

		//	If we've already loaded this extension, continue. This avoids loading
		//	both a .xml and .tdb file for the same extension.

		if (LoadedExtensions.Lookup(sExtensionRoot, NULL) == NOERROR)
			continue;

		LoadedExtensions.AddEntry(sExtensionRoot, NULL);

		//	Open the file

		CResourceDb ExtDb(sExtensionRoot, Ctx.pResDb);
		if (error = ExtDb.Open(DFOPEN_FLAG_READ_ONLY))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Unable to open file: %s"), sExtensionRoot.GetASCIIZPointer());
			return ERR_FAIL;
			}

		//	Parse the XML file into a structure

		CXMLElement *pRoot;
		if (error = ExtDb.LoadGameFile(&pRoot, &Ctx.sError))
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Error parsing %s: %s"), sExtensionRoot.GetASCIIZPointer(), Ctx.sError.GetASCIIZPointer());
			return ERR_FAIL;
			}

		//	Setup

		CSavedDesignLoadCtx SavedCtx(Ctx);
		Ctx.sResDb = sExtensionRoot;
		Ctx.pResDb = &ExtDb;

		//	Handle Extensions

		if (strEquals(pRoot->GetTag(), TRANSCENDENCE_EXTENSION_TAG))
			{
			kernelDebugLogMessage("Loading extension: %s", sExtensionRoot.GetASCIIZPointer());

			//	Add to design collection

			if (error = m_Design.BeginLoadExtension(Ctx, pRoot))
				return error;

			//	Load the design elements

			if (error = LoadExtension(Ctx, pRoot))
				return error;

			//	Done

			m_Design.EndLoadExtension(Ctx);
			}

		//	Handle Adventures

		else if (strEquals(pRoot->GetTag(), TRANSCENDENCE_ADVENTURE_TAG))
			{
			kernelDebugLogMessage("Loading adventure desc: %s", sExtensionRoot.GetASCIIZPointer());

			//	Add to design collection

			if (error = m_Design.BeginLoadAdventureDesc(Ctx, pRoot))
				return error;

			//	Load the design elements

			if (error = LoadExtension(Ctx, pRoot))
				return error;

			//	Done

			m_Design.EndLoadAdventureDesc(Ctx);
			}
		else
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Unknown root element: %s"), sExtensionRoot.GetASCIIZPointer());
			return ERR_FAIL;
			}

		//	Done

		delete pRoot;
		}

	return NOERROR;
	}

ALERROR CUniverse::LoadAdventure (SDesignLoadCtx &Ctx, CAdventureDesc *pAdventure)

//	LoadAdventure
//
//	Loads the adventure extension

	{
	ALERROR error;

	ASSERT(pAdventure);

	//	If we've already loaded this adventure, then we're done

	if (m_Design.IsAdventureExtensionLoaded(pAdventure->GetExtensionUNID()))
		return NOERROR;

	//	Open the file

	CString sFilespec = pAdventure->GetFilespec();
	CResourceDb ExtDb(sFilespec, Ctx.pResDb);
	if (error = ExtDb.Open(DFOPEN_FLAG_READ_ONLY))
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Unable to open file: %s"), sFilespec.GetASCIIZPointer());
		return ERR_FAIL;
		}

	//	Parse the XML file into a structure

	CXMLElement *pRoot;
	if (error = ExtDb.LoadGameFile(&pRoot, &Ctx.sError))
		{
		Ctx.sError = strPatternSubst(CONSTLIT("Error parsing %s: %s"), sFilespec.GetASCIIZPointer(), Ctx.sError.GetASCIIZPointer());
		return ERR_FAIL;
		}

	//	Setup

	CSavedDesignLoadCtx SavedCtx(Ctx);
	Ctx.sResDb = sFilespec;
	Ctx.pResDb = &ExtDb;

	//	Make sure we have the right file

	if (!strEquals(pRoot->GetTag(), TRANSCENDENCE_ADVENTURE_TAG))
		{
		Ctx.sError = CONSTLIT("Adventure expected");
		return ERR_FAIL;
		}

	//	Load the adventure

	kernelDebugLogMessage("Loading adventure: %s", sFilespec.GetASCIIZPointer());

	//	Add to design collection

	if (error = m_Design.BeginLoadAdventure(Ctx, pAdventure))
		return error;

	//	Load the design elements

	if (error = LoadExtension(Ctx, pRoot))
		return error;

	//	Done

	m_Design.EndLoadAdventure(Ctx);

	//	Done

	delete pRoot;
	return NOERROR;
	}

ALERROR CUniverse::LoadExtension (SDesignLoadCtx &Ctx, CXMLElement *pExtension)

//	LoadExtension
//
//	Loads the extension

	{
	ALERROR error;
	int i;

	//	Load all the design elements

	for (i = 0; i < pExtension->GetContentElementCount(); i++)
		{
		CXMLElement *pElement = pExtension->GetContentElement(i);

		if (error = LoadDesignElement(Ctx, pElement))
			return error;
		}

	return NOERROR;
	}
