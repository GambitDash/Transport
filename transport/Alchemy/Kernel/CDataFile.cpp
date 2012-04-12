//	CDataFile.cpp
//
//	Implements CDataFile object

#include "SDL.h"

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"

#include "CFile.h"

#define DATAFILE_SIGNATURE					STRSIG('A', 'L', 'D', 'F')
#define DATAFILE_VERSION					1
#define DATAFILE_MINBLOCKSIZE				64

#define FREE_ENTRY							0xFFFFFFFF
#define ENTRY_TABLE_GRANULARITY				256

#define ENTRY_FLAG_FREEBLOCK				0x00000001

typedef struct
	{
	DWORD dwSignature;							//	Always 'ALDF'
	DWORD dwVersion;							//	Version of DataFile structure
	DWORD dwBlockSize;							//	Size of blocks
	DWORD dwBlockCount;							//	Number of block
	DWORD dwEntryTableCount;					//	Number of allocated entries
	DWORD dwEntryTablePos;						//	File Pos of entry table
	DWORD dwDefaultEntry;						//	Default entry
	DWORD dwSpare[8];
	} HEADERSTRUCT, *PHEADERSTRUCT;

static CObjectClass<CDataFile>g_Class(OBJID_CDATAFILE, NULL);

CDataFile::CDataFile (void) : CObject(&g_Class)

//	CDataFile constructor

	{
	}

CDataFile::CDataFile (const CString &sFilename) : CObject(&g_Class),
		m_sFilename(sFilename),
		m_hFile(NULL),
		m_pEntryTable(NULL),
		m_fFlushing(FALSE)

//	CDataFile constructor

	{
	}

CDataFile::~CDataFile (void)

//	CDataFile destructor

	{
	Close();
	}

ALERROR CDataFile::AddEntry (const CString &sData, int *retiEntry)

//	AddEntry
//
//	Does some stuff

	{
	ALERROR error;
	int i, iEntry;
	DWORD dwStartingBlock;
	DWORD dwBlockCount;

	ASSERT(IsOpen());
	ASSERT(!m_fReadOnly);

	//	Look for a free entry

	for (i = 0; i < m_iEntryTableCount; i++)
		if (m_pEntryTable[i].dwBlock == FREE_ENTRY)
			break;

	//	If we could not find a free entry, grow the entry table

	if (i == m_iEntryTableCount)
		{
		if ((error = GrowEntryTable(&iEntry)))
			goto Fail;
		}
	else
		iEntry = i;

	//	Figure out how many blocks we need

	dwBlockCount = (sData.GetLength() / m_iBlockSize) + 1;

	//	Allocate a block chain large enough to contain the entry

	if ((error = AllocBlockChain(dwBlockCount, &dwStartingBlock)))
		goto Fail;

	//	Write the block chain

	if ((error = WriteBlockChain(dwStartingBlock, sData.GetPointer(), sData.GetLength())))
		{
		FreeBlockChain(dwStartingBlock, dwBlockCount);
		goto Fail;
		}

	//	Set the entry

	m_pEntryTable[iEntry].dwBlock = dwStartingBlock;
	m_pEntryTable[iEntry].dwBlockCount = dwBlockCount;
	m_pEntryTable[iEntry].dwSize = (DWORD)sData.GetLength();
	m_pEntryTable[iEntry].dwFlags = 0;
	m_fEntryTableModified = TRUE;

	//	Flush

	if ((error = Flush()))
		goto Fail;

	//	Done

	*retiEntry = iEntry;

	return NOERROR;

Fail:

	return error;
	}

ALERROR CDataFile::AllocBlockChain (DWORD dwBlockCount, DWORD *retdwStartingBlock)

//	AllocBlockChain
//
//	Allocates a new chain of blocks large enough to store data of length iLength.

	{
	int i;
	DWORD dwStartingBlock;

	//	Look for a free block large enough to hold the data

	for (i = 0; i < m_iEntryTableCount; i++)
		if ((m_pEntryTable[i].dwBlock != FREE_ENTRY)
				&& (m_pEntryTable[i].dwFlags & ENTRY_FLAG_FREEBLOCK)
				&& m_pEntryTable[i].dwBlockCount >= dwBlockCount)
			break;

	//	If we could not find a free block large enough, then we allocate
	//	at the end of the file.

	if (i == m_iEntryTableCount)
		{
		*retdwStartingBlock = (DWORD)m_iBlockCount;
		m_iBlockCount += (int)dwBlockCount;
		m_fHeaderModified = TRUE;
		return NOERROR;
		}

	//	Otherwise, we carve up the free block

	dwStartingBlock = m_pEntryTable[i].dwBlock;
	if (m_pEntryTable[i].dwBlockCount == dwBlockCount)
		m_pEntryTable[i].dwBlock = FREE_ENTRY;
	else
		{
		m_pEntryTable[i].dwBlock += dwBlockCount;
		m_pEntryTable[i].dwBlockCount -= dwBlockCount;
		}

	//	Done

	m_fEntryTableModified = TRUE;
	*retdwStartingBlock = dwStartingBlock;

	return NOERROR;
	}

ALERROR CDataFile::Close (void)

//	Close
//
//	Does some stuff

	{
	ALERROR error;

	//	If we're not open, return

	if (!IsOpen())
		return NOERROR;

	//	Flush entry table

	if ((error = Flush()))
		return error;

	//	Close file handles

	SDL_RWclose(m_hFile);
	m_hFile = NULL;

	MemFree(m_pEntryTable);
	m_pEntryTable = NULL;

	return NOERROR;
	}

ALERROR CDataFile::Create (const CString &sFilename,
							int iBlockSize,
							int iInitialEntries)

//	Create
//
//	Creates a new data file

	{
	ALERROR error;
	HEADERSTRUCT header;
	SDL_RWops *hFile;
	DWORD dwWritten;
	int iEntryTableSize;
	ENTRYSTRUCT entry;
	int i;

	if (iBlockSize < DATAFILE_MINBLOCKSIZE)
		iBlockSize = DATAFILE_MINBLOCKSIZE;

	//	Create the file
	hFile = SDL_RWFromFile(sFilename.GetASCIIZPointer(), "w");
	if (hFile == NULL)
		{
		error = ERR_FAIL;
		goto Fail;
		}

	if (SDL_RWseek(hFile, 0, SEEK_SET) != 0)
		{
		error = ERR_FAIL;
		goto Fail;
		}

	//	Figure out how big the entry table will be

	iEntryTableSize = (iInitialEntries + 1) * sizeof(ENTRYSTRUCT);

	//	Prepare the header

	memset(&header, 0, sizeof(header));
	header.dwSignature = DATAFILE_SIGNATURE;
	header.dwVersion = DATAFILE_VERSION;
	header.dwBlockCount = (iEntryTableSize / iBlockSize) + 1;
	header.dwBlockSize = (DWORD)iBlockSize;
	header.dwEntryTableCount = (DWORD)iInitialEntries + 1;
	header.dwEntryTablePos = sizeof(header);

	//	Write the header

	dwWritten = SDL_RWwrite(hFile, &header, 1, sizeof(header));
	if (dwWritten != sizeof(header))
		{
		error = ERR_FAIL;
		goto Fail;
		}

	//	Prepare the single entry that describes the entry table

	entry.dwBlock = 0;
	entry.dwBlockCount = header.dwBlockCount;
	entry.dwSize = (DWORD)iEntryTableSize;
	entry.dwFlags = 0;

	//	Write the entry

	dwWritten = SDL_RWwrite(hFile, &entry, 1, sizeof(entry));
	if (dwWritten != sizeof(entry))
		{
		error = ERR_FAIL;
		goto Fail;
		}

	//	Write the rest of the entries

	entry.dwBlock = FREE_ENTRY;
	entry.dwBlockCount = 0;
	entry.dwSize = 0;
	entry.dwFlags = 0;

	for (i = 0; i < iInitialEntries; i++)
		dwWritten = SDL_RWwrite(hFile, &entry, 1, sizeof(entry));
		if (dwWritten != sizeof(entry))
			{
			error = ERR_FAIL;
			goto Fail;
			}

	//	Done

	SDL_RWclose(hFile);

	return NOERROR;

Fail:

	if (hFile)
		{
		SDL_RWclose(hFile);
		/* XXX We used to delete the file here. */
		}

	return error;
	}

ALERROR CDataFile::DeleteEntry (int iEntry)

//	DeleteEntry
//
//	Does some stuff

	{
	ALERROR error;
	DWORD dwOldBlock;
	DWORD dwOldBlockCount;

	ASSERT(IsOpen());
	ASSERT(!m_fReadOnly);

	//	Make sure we're in bounds

	if (iEntry < 0 || iEntry >= m_iEntryTableCount)
		return ERR_FAIL;

	if (m_pEntryTable[iEntry].dwBlock == FREE_ENTRY)
		return ERR_FAIL;

	//	Delete it

	dwOldBlock = m_pEntryTable[iEntry].dwBlock;
	dwOldBlockCount = m_pEntryTable[iEntry].dwBlockCount;
	m_pEntryTable[iEntry].dwBlock = FREE_ENTRY;

	FreeBlockChain(dwOldBlock, dwOldBlockCount);

	//	Flush

	if ((error = Flush()))
		return error;

	return NOERROR;
	}

ALERROR CDataFile::Flush (void)

//	Flush
//
//	Flush the entry table and header

	{
	ALERROR error;

	//	If we're already flushing, don't bother. This can happens since
	//	We're using the normal WriteEntry call to write out the entry table.

	if (m_fFlushing)
		return NOERROR;

	m_fFlushing = TRUE;

	//	Write out the entry table, if necessary

	if (m_fEntryTableModified)
		{
		int i, iEntryTableSize;

		//	Make sure there's at least one free entry in the entry table
		//	Otherwise we might have to grow the entry table while trying to
		//	save it.

		for (i = 0; i < m_iEntryTableCount; i++)
			if (m_pEntryTable[i].dwBlock == FREE_ENTRY)
				break;

		if (i == m_iEntryTableCount)
			if ((error = GrowEntryTable(NULL)))
				goto Fail;

		//	Figure out how many blocks we need to hold the entry

		iEntryTableSize = m_iEntryTableCount * sizeof(ENTRYSTRUCT);

		//	Write out the entry table. Note that the act of writing the
		//	entry table may modify the entry table, so we first resize
		//	the entry.

		if ((error = ResizeEntry(0, iEntryTableSize, NULL)))
			goto Fail;

		//	Write the stuff

		if ((error = WriteBlockChain(m_pEntryTable[0].dwBlock, (char *)m_pEntryTable, iEntryTableSize)))
			goto Fail;

		m_fEntryTableModified = FALSE;
		m_fHeaderModified = TRUE;
		}

	//	Write out the header

	if (m_fHeaderModified)
		{
		HEADERSTRUCT header;
		DWORD dwWritten;

		memset(&header, 0, sizeof(header));
		header.dwSignature = DATAFILE_SIGNATURE;
		header.dwVersion = DATAFILE_VERSION;
		header.dwBlockCount = (DWORD)m_iBlockCount;
		header.dwBlockSize = (DWORD)m_iBlockSize;
		header.dwEntryTableCount = (DWORD)m_iEntryTableCount;
		header.dwEntryTablePos = sizeof(HEADERSTRUCT) + (m_pEntryTable[0].dwBlock * m_iBlockSize);
		header.dwDefaultEntry = (DWORD)m_iDefaultEntry;

		//	Position the file pointer
		if (SDL_RWseek(m_hFile, 0, SEEK_SET) != 0)
			{
			error = ERR_FAIL;
			goto Fail;
			}

		//	Write the header
		dwWritten = SDL_RWwrite(m_hFile, &header, 1, sizeof(header));
		if (dwWritten != sizeof(header))
			{
			error = ERR_FAIL;
			goto Fail;
			}

		m_fHeaderModified = FALSE;
		}

	m_fFlushing = FALSE;

	return NOERROR;

Fail:

	m_fFlushing = FALSE;

	return error;
	}

ALERROR CDataFile::FreeBlockChain (DWORD dwStartingBlock, DWORD dwBlockCount)

//	FreeBlockChain
//
//	Frees the sequence of blocks starting at dwStartingBlock. Note: If we can
//	guarantee that there is at least one free entry, this routine will always
//	return NOERROR.

	{
	ALERROR error;
	int i;
	int iFreeAfter = -1;
	int iFreeBefore = -1;
	int iFreeEntry = -1;

	//	If this entry is at the end of the file, just truncate the file

	if (dwStartingBlock + dwBlockCount == (DWORD)m_iBlockCount)
		{
		m_iBlockCount -= (int)dwBlockCount;
		m_fHeaderModified = TRUE;
		return NOERROR;
		}

	//	Look for free entries before and after this chain

	for (i = 0; i < m_iEntryTableCount; i++)
		if (m_pEntryTable[i].dwBlock != FREE_ENTRY)
			{
			if (m_pEntryTable[i].dwFlags & ENTRY_FLAG_FREEBLOCK)
				{
				if (m_pEntryTable[i].dwBlock + m_pEntryTable[i].dwBlockCount == dwStartingBlock)
					iFreeBefore = i;
				else if (m_pEntryTable[i].dwBlock == dwStartingBlock + dwBlockCount)
					iFreeAfter = i;
				}
			}
		else
			iFreeEntry = i;

	//	If we've got an entry before, grow the previous entry to
	//	include ours

	if (iFreeBefore != -1)
		{
		m_pEntryTable[iFreeBefore].dwBlockCount += dwBlockCount;

		//	If we've also got a free entry afterwards, include that too

		if (iFreeAfter != -1)
			{
			m_pEntryTable[iFreeBefore].dwBlockCount += m_pEntryTable[iFreeAfter].dwBlockCount;
			m_pEntryTable[iFreeAfter].dwBlock = FREE_ENTRY;
			}
		}
	else if (iFreeAfter != -1)
		{
		m_pEntryTable[iFreeAfter].dwBlock -= dwBlockCount;
		m_pEntryTable[iFreeAfter].dwBlockCount += dwBlockCount;
		}
	else
		{
		if (iFreeEntry == -1)
			{
			if ((error = GrowEntryTable(&iFreeEntry)))
				return error;
			}

		m_pEntryTable[iFreeEntry].dwBlock = dwStartingBlock;
		m_pEntryTable[iFreeEntry].dwBlockCount = dwBlockCount;
		m_pEntryTable[iFreeEntry].dwFlags = ENTRY_FLAG_FREEBLOCK;
		}

	m_fEntryTableModified = TRUE;

	return NOERROR;
	}

int CDataFile::GetDefaultEntry (void)

//	GetDefaultEntry
//
//	Does some stuff

	{
	return m_iDefaultEntry;
	}

int CDataFile::GetEntryLength (int iEntry)

//	GetEntryLength
//
//	Does some stuff

	{
	ASSERT(IsOpen());

	return (int)m_pEntryTable[iEntry].dwSize;
	}

ALERROR CDataFile::GrowEntryTable (int *retiEntry)

//	GrowEntryTable
//
//	Grows the entry table and returns the first newly allocated free entry

	{
	int iNewEntryTableCount = m_iEntryTableCount + ENTRY_TABLE_GRANULARITY;
	int iNewEntryTableSize = iNewEntryTableCount * sizeof(ENTRYSTRUCT);
	PENTRYSTRUCT pNewEntryTable;
	int i;

	pNewEntryTable = (PENTRYSTRUCT)MemAlloc(iNewEntryTableSize);
	if (pNewEntryTable == NULL)
		return ERR_MEMORY;

	//	Copy over the old table

	memcpy(pNewEntryTable, m_pEntryTable, m_iEntryTableCount * sizeof(ENTRYSTRUCT));

	//	Initialize the rest of the  table

	for (i = m_iEntryTableCount; i < iNewEntryTableCount; i++)
		pNewEntryTable[i].dwBlock = FREE_ENTRY;

	//	Set the next free entry

	if (retiEntry)
		*retiEntry = m_iEntryTableCount;

	//	Do the move

	MemFree(m_pEntryTable);
	m_pEntryTable = pNewEntryTable;
	m_iEntryTableCount = iNewEntryTableCount;
	m_fEntryTableModified = TRUE;

	return NOERROR;
	}

ALERROR CDataFile::Open (DWORD dwFlags)

//	Open
//
//	Opens the data file.
//
//	Errors:
//
//		ERR_NOTFOUND: File does not exist
//		ERR_FILEOPEN: Unable to open file
//		ERR_MEMORY: Out of memory

	{
	ALERROR error;
	HEADERSTRUCT header;
	DWORD dwRead;
	int iEntryTableSize;

	//	Open file either for read or read/write

	const char *access;
	if (dwFlags & DFOPEN_FLAG_READ_ONLY)
		{
		access = "r";
		}
	else
		{
		access = "r+";
		}

	//	Open the file for reading and writing

	ASSERT(m_hFile == NULL);
	m_hFile = SDL_RWFromFile(m_sFilename.GetASCIIZPointer(), access);
	if (m_hFile == NULL)
		{
		error = ERR_FILEOPEN;
		goto Fail;
		}

	//	Read the file header

	dwRead = SDL_RWread(m_hFile, &header, 1, sizeof(header));
	if (dwRead != sizeof(header))
		{
		error = ERR_FILEOPEN;
		goto Fail;
		}

	//	If the signature is not right, bail

	if (header.dwSignature != DATAFILE_SIGNATURE)
		{
		error = ERR_FILEOPEN;
		goto Fail;
		}

	//	If the version is not right, bail

	if (header.dwVersion != DATAFILE_VERSION)
		{
		error = ERR_FILEOPEN;
		goto Fail;
		}

	//	Store header info

	m_iBlockSize = (int)header.dwBlockSize;
	m_iBlockCount = (int)header.dwBlockCount;
	m_iDefaultEntry = (int)header.dwDefaultEntry;

	//	Allocate an entry table of the appropriate size

	ASSERT(m_pEntryTable == NULL);
	m_iEntryTableCount = (int)header.dwEntryTableCount;
	iEntryTableSize = header.dwEntryTableCount * sizeof(ENTRYSTRUCT);
	m_pEntryTable = (PENTRYSTRUCT)MemAlloc(iEntryTableSize);
	if (m_pEntryTable == NULL)
		{
		error = ERR_MEMORY;
		goto Fail;
		}

	//	Read the entry table

	if (SDL_RWseek(m_hFile, header.dwEntryTablePos, SEEK_SET) != header.dwEntryTablePos)
		{
		error = ERR_FILEOPEN;
		goto Fail;
		}

	dwRead = SDL_RWread(m_hFile, m_pEntryTable, 1, iEntryTableSize);
	if (dwRead != iEntryTableSize)
		{
		error = ERR_FILEOPEN;
		goto Fail;
		}

	//	Reset modification flags

	m_fHeaderModified = FALSE;
	m_fEntryTableModified = FALSE;
	m_fReadOnly = ((dwFlags & DFOPEN_FLAG_READ_ONLY) ? true : false);

	return NOERROR;

Fail:

	if (m_hFile != NULL)
		{
		SDL_RWclose(m_hFile);
		m_hFile = NULL;
		}

	if (m_pEntryTable)
		{
		MemFree(m_pEntryTable);
		m_pEntryTable = NULL;
		}

	return error;
	}

ALERROR CDataFile::ReadEntry (int iEntry, CString *retsData)

//	ReadEntry
//
//	Does some stuff

	{
	DWORD dwPos;
	char *pDest;
	DWORD dwRead;

	ASSERT(IsOpen());

	//	Make sure we're in bounds

	if (iEntry < 0 || iEntry >= m_iEntryTableCount)
		return ERR_FAIL;

	if (m_pEntryTable[iEntry].dwBlock == FREE_ENTRY)
		return ERR_FAIL;

	//	Allocate space in the string

	pDest = retsData->GetWritePointer((int)m_pEntryTable[iEntry].dwSize);
	if (pDest == NULL)
		return ERR_MEMORY;

	//	Calculate the position

	dwPos = sizeof(HEADERSTRUCT) + m_pEntryTable[iEntry].dwBlock * m_iBlockSize;

	//	Position the file

	if (SDL_RWseek(m_hFile, dwPos, SEEK_SET) != dwPos)
		return ERR_FAIL;

	//	Read the file data

	dwRead = SDL_RWread(m_hFile, pDest, 1, m_pEntryTable[iEntry].dwSize);
	if (dwRead != m_pEntryTable[iEntry].dwSize)
		return ERR_FAIL;

	return NOERROR;
	}

ALERROR CDataFile::ReadEntryPartial (int iEntry, int iPos, int iLength, CString *retsData)

//	ReadEntryPartial
//
//	Reads a portion of the given entry.
//	If iLength is -1, we read until the end of the buffer.

	{
	DWORD dwPos;
	char *pDest;
	DWORD dwRead;
	bool bOverrun = false;

	ASSERT(IsOpen());

	//	Make sure we're in bounds

	if (iEntry < 0 || iEntry >= m_iEntryTableCount)
		return ERR_FAIL;

	if (m_pEntryTable[iEntry].dwBlock == FREE_ENTRY)
		return ERR_FAIL;

	//	If we're off the end, then we're done. If we ask to read more than
	//	we have left, then it is an error.

	int iEntrySize = (int)m_pEntryTable[iEntry].dwSize;
	if (iPos > iEntrySize)
		{
		*retsData = NULL_STR;
		return ERR_FAIL;
		}

	//	Figure out how large the return buffer will be

	int iReadSize;
	if (iLength == -1)
		iReadSize = iEntrySize - iPos;
	else if (iPos + iLength > iEntrySize)
		{
		iReadSize = iEntrySize - iPos;
		bOverrun = true;
		}
	else
		iReadSize = iLength;

	if (iReadSize == 0)
		{
		*retsData = NULL_STR;
		return (bOverrun ? ERR_FAIL : NOERROR);
		}

	//	Allocate space in the string

	pDest = retsData->GetWritePointer(iReadSize);
	if (pDest == NULL)
		return ERR_MEMORY;

	//	Calculate the position

	dwPos = sizeof(HEADERSTRUCT) + (m_pEntryTable[iEntry].dwBlock * m_iBlockSize) + (DWORD)iPos;

	//	Position the file

	if (SDL_RWseek(m_hFile, dwPos, SEEK_SET) != dwPos)
		return ERR_FAIL;

	//	Read the file data

	dwRead = SDL_RWread(m_hFile, pDest, 1, iReadSize);
	if (dwRead != iReadSize)
		return ERR_FAIL;

	return NOERROR;
	}

ALERROR CDataFile::ResizeEntry (int iEntry, DWORD dwSize, DWORD *retdwBlockCount)

//	ResizeEntry
//
//	Changes the blocks associated with the given entry. Note that this
//	routine does not necessarily preserve the previous contents of
//	the entry

	{
	ALERROR error;
	DWORD dwBlockCount;

	//	Figure out how many blocks we need to hold the entry

	dwBlockCount = (dwSize / (DWORD)m_iBlockSize) + 1;

	//	If this is less than the number of blocks that we've
	//	got allocated, free some

	if (dwBlockCount < m_pEntryTable[iEntry].dwBlockCount)
		{
		DWORD dwFreeBlocks = m_pEntryTable[iEntry].dwBlockCount - dwBlockCount;

		m_pEntryTable[iEntry].dwBlockCount = dwBlockCount;
		if ((error = FreeBlockChain(m_pEntryTable[iEntry].dwBlock + dwBlockCount, dwFreeBlocks)))
			return error;
		}

	//	If this is more than the number of entries for this block
	//	then we need to grow the block chain.

	else if (dwBlockCount > m_pEntryTable[iEntry].dwBlockCount)
		{
		DWORD dwOldBlock = m_pEntryTable[iEntry].dwBlock;
		DWORD dwOldBlockCount = m_pEntryTable[iEntry].dwBlockCount;
		DWORD dwStartingBlock;

		//	Add a new entry

		if ((error = AllocBlockChain(dwBlockCount, &dwStartingBlock)))
			return error;

		//	Store the new entry data

		m_pEntryTable[iEntry].dwBlock = dwStartingBlock;
		m_pEntryTable[iEntry].dwBlockCount = dwBlockCount;

		//	Free the current block chain

		if ((error = FreeBlockChain(dwOldBlock, dwOldBlockCount)))
			return error;
		}

	//	Set data length

	m_pEntryTable[iEntry].dwSize = dwSize;
	m_fEntryTableModified = TRUE;

	//	Return the block count, if necessary

	if (retdwBlockCount)
		*retdwBlockCount = dwBlockCount;

	return NOERROR;
	}

void CDataFile::SetDefaultEntry (int iEntry)

//	SetDefaultEntry
//
//	Does some stuff

	{
	ASSERT(!m_fReadOnly);

	m_iDefaultEntry = iEntry;
	m_fHeaderModified = TRUE;
	}

ALERROR CDataFile::WriteBlockChain (DWORD dwStartingBlock, char *pData, DWORD dwSize)

//	WriteBlockChain
//
//	Writes the data to the block chain. We assume that the block chain
//	has been previously allocated to the correct size

	{
	DWORD dwPos;
	DWORD dwWritten;

	//	Calculate the position

	dwPos = sizeof(HEADERSTRUCT) + dwStartingBlock * m_iBlockSize;

	//	Position the file pointer
	if (SDL_RWseek(m_hFile, dwPos, SEEK_SET) != dwPos)
		return ERR_FAIL;

	//	Write the stuff

	dwWritten = SDL_RWwrite(m_hFile, pData, 1, dwSize);
	if (dwWritten!= dwSize)
		return ERR_FAIL;

	return NOERROR;
	}

ALERROR CDataFile::WriteEntry (int iEntry, const CString &sData)

//	WriteEntry
//
//	Does some stuff

	{
	ALERROR error;

	ASSERT(IsOpen());
	ASSERT(!m_fReadOnly);

	//	Make sure we're in bounds

	if (iEntry < 0 || iEntry >= m_iEntryTableCount)
		return ERR_FAIL;

	if (m_pEntryTable[iEntry].dwBlock == FREE_ENTRY)
		return ERR_FAIL;

	//	Resize the entry

	if ((error = ResizeEntry(iEntry, sData.GetLength(), NULL)))
		return error;

	//	Write out the data.

	if ((error = WriteBlockChain(m_pEntryTable[iEntry].dwBlock, sData.GetPointer(), sData.GetLength())))
		return error;

	//	Flush

	if ((error = Flush()))
		return error;

	return NOERROR;
	}
