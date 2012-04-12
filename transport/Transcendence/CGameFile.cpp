//	CGameFile.cpp
//
//	CGameFile class

#include "PreComp.h"
#include "Transcendence.h"
#include "CFile.h"
#include "CFileDirectory.h"

#define MIN_GAME_FILE_VERSION					5
#define GAME_FILE_VERSION						8

CGameFile::CGameFile (void) : 
		m_pFile(NULL),
		m_iRefCount(0),
		m_SystemMap(FALSE, TRUE)

//	CGameFile constructor

	{
	}

CGameFile::~CGameFile (void)

//	CGameFile destructor

	{
	ASSERT(m_iRefCount == 0);

	if (m_iRefCount)
		{
		m_iRefCount = 1;
		Close();
		}
	}

void CGameFile::Close (void)

//	Close
//
//	Close the game file

	{
	ASSERT(m_iRefCount > 0);

	if (--m_iRefCount == 0)
		{
		ASSERT(m_pFile);

		m_pFile->Close();
		delete m_pFile;
		m_pFile = NULL;
		}
	}

ALERROR CGameFile::Create (const CString &sFilename)

//	Create
//
//	Create the game file

	{
	ALERROR error;

	ASSERT(m_pFile == NULL);
	ASSERT(m_iRefCount == 0);

	//	Create and open the file

	error = CDataFile::Create(sFilename, 4096, 8);
	if (error)
		return error;

	m_pFile = new CDataFile(sFilename);
	if (m_pFile == NULL)
		return ERR_MEMORY;

	error = m_pFile->Open();
	if (error)
		{
		delete m_pFile;
		m_pFile = NULL;
		return error;
		}

	//	Universe not yet saved

	memset(&m_Header, 0, sizeof(m_Header));

	m_Header.dwVersion = GAME_FILE_VERSION;
	m_Header.dwUniverse = INVALID_ENTRY;
	m_Header.dwResurrectCount = 0;
	m_Header.dwFlags = 0;

	//	Save out an empty system map

	m_SystemMap.RemoveAll();
	CString sData;
	SaveSystemMapToStream(&sData);

	if ((error = m_pFile->AddEntry(sData, (int *)&m_Header.dwSystemMap)))
		{
		delete m_pFile;
		m_pFile = NULL;
		return error;
		}

	//	Save out the game header information

	sData = CString((char *)&m_Header, sizeof(m_Header));

	error = m_pFile->AddEntry(sData, &m_iHeaderID);
	if (error)
		{
		delete m_pFile;
		m_pFile = NULL;
		return error;
		}

	m_pFile->SetDefaultEntry(m_iHeaderID);
	m_pFile->Flush();

	//	Done

	m_iRefCount = 1;

	return NOERROR;
	}

CString CGameFile::GenerateFilename (const CString &sName)

//	GenerateFilename
//
//	Generates a filename from a name

	{
	CString sFilename = sName;
	sFilename.Append(CONSTLIT(".sav"));
	pathValidateFilename(sFilename, &sFilename);
	return sFilename;
	}

ALERROR CGameFile::LoadGameHeader (SGameHeader *retHeader)

//	LoadGameHeader
//
//	Load the game header

	{
	ALERROR error;

	CString sHeader;
	if ((error = m_pFile->ReadEntry(m_iHeaderID, &sHeader)))
		return error;

	if (sHeader.GetLength() < sizeof(SGameHeader))
		return ERR_FAIL;

	*retHeader = *(SGameHeader *)sHeader.GetPointer();

	return NOERROR;
	}

ALERROR CGameFile::LoadSystem (DWORD dwUNID, 
							   CSystem **retpSystem, 
							   DWORD dwObjID,
							   CSpaceObject **retpObj)

//	LoadSystem
//
//	Load a star system from the game file

	{
	ALERROR error;

	ASSERT(m_pFile);

	//	Get the entry where this system is stored. If we can't find it,
	//	then this must be a new system.

	DWORD dwEntry;
	if (m_SystemMap.Lookup(dwUNID, (CObject **)&dwEntry) != NOERROR)
		{
		kernelDebugLogMessage("Unable to find system ID: %x", dwUNID);
		return ERR_FAIL;
		}

	//	Read the system data

	CString sData;
	if ((error = m_pFile->ReadEntry(dwEntry, &sData)))
		{
		kernelDebugLogMessage("Unable to read system data entry: %x", dwEntry);
		return error;
		}

	//	Convert to a stream

	CMemoryReadStream Stream(sData.GetPointer(), sData.GetLength());

	error = Stream.Open();
	if (error)
		{
		kernelDebugLogMessage("Unable to open data stream for system entry: %x", dwEntry);
		return error;
		}

	//	Load the system from the stream

	error = CSystem::CreateFromStream(g_pUniverse, 
			&Stream, 
			retpSystem,
			dwObjID,
			retpObj);
	if (error)
		{
		Stream.Close();
		kernelDebugLogMessage("Unable to load system: %x", dwEntry);
		return error;
		}

	//	Tell the universe

	error = g_pUniverse->AddStarSystem((*retpSystem)->GetTopology(), *retpSystem);
	if (error)
		{
		delete *retpSystem;
		Stream.Close();
		kernelDebugLogMessage("Unable to add system to topology: %x", dwEntry);
		return error;
		}

	//	Done

	error = Stream.Close();
	if (error)
		return error;

	return NOERROR;
	}

void CGameFile::LoadSystemMapFromStream (DWORD dwVersion, const CString &sStream)

//	LoadSystemMapFromStream
//
//	Loads the system map

	{
	int i;
	DWORD *pPos = (DWORD *)sStream.GetASCIIZPointer();
	DWORD *pPosEnd = pPos + (sStream.GetLength() / sizeof(DWORD));
	bool bLoadV6;

	//	Because of a bug in version 7 we don't actually know whether
	//	version 6 is the new version or the old version. Try to
	//	heuristically determine it here.

	int iCount = (int)*pPos++;

	if (dwVersion < 7)
		bLoadV6 = (iCount != ((sStream.GetLength() / sizeof(DWORD)) - 1) / 2);
	else
		bLoadV6 = false;

	//	For the older version, we need to do some processing

	if (bLoadV6)
		{
		m_SystemMap.RemoveAll();
		for (i = 0; i < iCount && pPos < pPosEnd; i++)
			{
			DWORD dwValue = *pPos++;
			if (dwValue)
				m_SystemMap.AddEntry(i + 1, (CObject *)dwValue);
			}
		}

	//	The newer version stores a compact ID table

	else
		{
		m_SystemMap.RemoveAll();
		for (i = 0; i < iCount && pPos < pPosEnd; i++)
			{
			DWORD dwKey = *pPos++;
			DWORD dwValue = *pPos++;

			m_SystemMap.AddEntry(dwKey, (CObject *)dwValue);
			}
		}
	}

ALERROR CGameFile::LoadUniverse (CUniverse &Univ, DWORD *retdwSystemID, DWORD *retdwPlayerID, CString *retsError)

//	LoadUniverse
//
//	Load the universe

	{
	ALERROR error;

	ASSERT(m_pFile);
	if (m_Header.dwUniverse == INVALID_ENTRY)
		{
		*retsError = CONSTLIT("Invalid save file: can't find universe entry.");
		return ERR_FAIL;
		}

	//	Read the data

	CString sData;
	if ((error = m_pFile->ReadEntry(m_Header.dwUniverse, &sData)))
		{
		*retsError = CONSTLIT("Invalid save file: error loading universe entry.");
		return error;
		}

	//	Convert to a stream

	CMemoryReadStream Stream(sData.GetPointer(), sData.GetLength());

	error = Stream.Open();
	if (error)
		{
		*retsError = CONSTLIT("Invalid save file: unable to open universe stream.");
		return error;
		}

	//	Load the universe from the stream

	if ((error = Univ.LoadFromStream(&Stream, retdwSystemID, retdwPlayerID, retsError)))
		{
		Stream.Close();
		return error;
		}

	//	Set debug

	Univ.SetDebugMode((m_Header.dwFlags & GAME_FLAG_DEBUG) ? true : false);

	//	Done

	Stream.Close();

	return NOERROR;
	}

ALERROR CGameFile::Open (const CString &sFilename)

//	Open
//
//	Open an existing game file

	{
	ALERROR error;

	if (m_iRefCount == 0)
		{
		ASSERT(m_pFile == NULL);

		//	Open the file

		m_pFile = new CDataFile(sFilename);
		if (m_pFile == NULL)
			return ERR_MEMORY;

		if ((error = m_pFile->Open()))
			goto Fail;

		//	Load the header

		m_iHeaderID = m_pFile->GetDefaultEntry();
		if ((error = LoadGameHeader(&m_Header)))
			goto Fail;

		//	Check the version

		if (m_Header.dwVersion < MIN_GAME_FILE_VERSION
				|| m_Header.dwVersion > GAME_FILE_VERSION)
			{
			error = ERR_FAIL;
			goto Fail;
			}

		//	Load the system map

		CString sSystemMap;
		if ((error = m_pFile->ReadEntry(m_Header.dwSystemMap, &sSystemMap)))
			goto Fail;

		LoadSystemMapFromStream(m_Header.dwVersion, sSystemMap);

		//	If this is a previous version, upgrade to the latest

		bool bUpgrade = false;
		if (m_Header.dwVersion < 8)
			{
			//	Save out the system map because we changed the format in version 7

			SaveSystemMapToStream(&sSystemMap);
			if ((error = m_pFile->WriteEntry(m_Header.dwSystemMap, sSystemMap)))
				goto Fail;

			bUpgrade = true;
			}

		if (bUpgrade)
			{
			m_Header.dwVersion = GAME_FILE_VERSION;
			if (error = SaveGameHeader(m_Header))
				goto Fail;

			m_pFile->Flush();
			}
		}

	m_iRefCount++;

	return NOERROR;

Fail:

	delete m_pFile;
	m_pFile = NULL;
	return error;
	}

ALERROR CGameFile::SaveGameHeader (SGameHeader &Header)

//	SaveGameHeader
//
//	Save the game header

	{
	ALERROR error;

	CString sData = CString((char *)&Header, sizeof(SGameHeader));

	if ((error = m_pFile->WriteEntry(m_iHeaderID, sData)))
		return error;

	return NOERROR;
	}

ALERROR CGameFile::SaveSystem (DWORD dwUNID, CSystem *pSystem)

//	SaveSystem
//
//	Save a star system

	{
	ALERROR error;
	DWORD *pNewMap = NULL;

	ASSERT(m_pFile);

	//	Save the system to a stream

	CMemoryWriteStream Stream;

	if ((error = Stream.Create()))
		return error;

	if ((error = pSystem->SaveToStream(&Stream)))
		return error;

	if ((error= Stream.Close()))
		return error;

	CString sStream(Stream.GetPointer(), Stream.GetLength(), true);

	//	See if this system has already been saved

	DWORD dwEntry;
	if (m_SystemMap.Lookup(dwUNID, (CObject **)&dwEntry) == NOERROR)
		{
		//	Save the system

		if ((error = m_pFile->WriteEntry(dwEntry, sStream)))
			return error;
		}

	//	Otherwise, we add the system

	else
		{
		//	Save the system

		if ((error = m_pFile->AddEntry(sStream, (int *)&dwEntry)))
			return error;

		//	Add to the map

		m_SystemMap.AddEntry(dwUNID, (CObject *)dwEntry);

		//	Save the map

		CString sData;
		SaveSystemMapToStream(&sData);

		if ((error = m_pFile->WriteEntry(m_Header.dwSystemMap, sData)))
			return error;
		}

	//	Done

	m_pFile->Flush();

	return NOERROR;
	}

void CGameFile::SaveSystemMapToStream (CString *retsStream)

//	SaveSystemMapToStream
//
//	Saves out the system map

	{
	int i;

	//	The system map is a DWORD length followed by n pair of DWORDs

	int iTotalLen = sizeof(DWORD) + m_SystemMap.GetCount() * 2 * sizeof(DWORD);
	CString sOutput;
	DWORD *pPos = (DWORD *)sOutput.GetWritePointer(iTotalLen);

	//	Write out the length

	DWORD dwSave = m_SystemMap.GetCount();
	*pPos++ = dwSave;

	//	Write out each mapping

	for (i = 0; i < m_SystemMap.GetCount(); i++)
		{
		dwSave = (DWORD)m_SystemMap.GetKey(i);
		*pPos++ = dwSave;

		dwSave = (DWORD)m_SystemMap.GetValue(i);
		*pPos++ = dwSave;
		}

	//	Done

	*retsStream = sOutput;
	}

ALERROR CGameFile::SaveUniverse (CUniverse &Univ, bool bCheckpoint)

//	SaveUniverse
//
//	Saves the universe

	{
	ALERROR error;

	//	Get the universe to stream itself out (note that we save
	//	systems separately)

	CMemoryWriteStream Stream;
	
	error = Stream.Create();
	if (error)
		return error;

	error = Univ.SaveToStream(&Stream);
	if (error)
		return error;

	error = Stream.Close();
	if (error)
		return error;

	CString sStream(Stream.GetPointer(), Stream.GetLength(), true);

	//	Keep track to see if we need to update the header

	bool bUpdateHeader = false;

	//	Figure out the name of the system that the player is at

	CSystem *pCurSystem = Univ.GetCurrentSystem();
	if (pCurSystem)
		{
		CString sSystemName = pCurSystem->GetName();
		ASSERT(!sSystemName.IsBlank());
		if (!strEquals(sSystemName, GetSystemName()))
			{
			strncpy(m_Header.szSystemName, sSystemName.GetASCIIZPointer(), sizeof(m_Header.szSystemName));
			bUpdateHeader = true;
			}
		}
	else
		ASSERT(false);

	//	Set the flags

	DWORD dwFlags = m_Header.dwFlags;
	dwFlags &= ~GAME_FLAG_RESURRECT;
	if (bCheckpoint)
		dwFlags |= GAME_FLAG_RESURRECT;

	if (Univ.InDebugMode())
		dwFlags |= GAME_FLAG_DEBUG;

	if (dwFlags != m_Header.dwFlags)
		{
		m_Header.dwFlags = dwFlags;
		bUpdateHeader = true;
		}

	//	If the universe has already been saved before then just
	//	save to the existing entry

	if (m_Header.dwUniverse != INVALID_ENTRY)
		{
		if ((error = m_pFile->WriteEntry(m_Header.dwUniverse, sStream)))
			return error;
		}

	//	Otherwise we need to create a new entry and update the header

	else
		{
		if ((error = m_pFile->AddEntry(sStream, (int *)&m_Header.dwUniverse)))
			return error;

		//	Update the header

		bUpdateHeader = true;
		}

	//	Save the header, if necessary

	if (bUpdateHeader)
		{
		if ((error = SaveGameHeader(m_Header)))
			return error;
		}

	//	Done

	m_pFile->Flush();

	return NOERROR;
	}

ALERROR CGameFile::SetGameResurrect (void)

//	SetGameResurrect
//
//	Sets the resurrect flag in the game (if not already set). This
//	Should only be called when we're loading a game.

	{
	ALERROR error;

	ASSERT(m_pFile);

	//	If we're about to start playing a game that has been
	//	resurrected, then increment our resurrect count (and save it)

	if (IsGameResurrect())
		m_Header.dwResurrectCount++;

	//	Otherwise, set the flag

	else
		m_Header.dwFlags |= GAME_FLAG_RESURRECT;

	//	Save the header

	if ((error = SaveGameHeader(m_Header)))
		return error;

	return NOERROR;
	}

