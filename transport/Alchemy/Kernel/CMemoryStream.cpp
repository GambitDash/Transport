//	CMemoryStream.cpp
//
//	Implements CMemoryWriteStream and CMemoryReadStream object

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"

#include "CStream.h"
#include "CUtility.h"
#include "CMath.h"

#define ALLOC_SIZE							4096
#define DEFAULT_MAX_SIZE					(1024 * 1024)

static CObjectClass<CMemoryWriteStream>g_WriteClass(OBJID_CMEMORYWRITESTREAM, NULL);

static CObjectClass<CMemoryReadStream>g_ReadClass(OBJID_CMEMORYREADSTREAM, NULL);

CMemoryWriteStream::CMemoryWriteStream (void) :
		CObject(&g_WriteClass),
		m_iMaxSize(DEFAULT_MAX_SIZE),
		m_pBlock(NULL)

//	CMemoryWriteStream constructor

	{
	}

CMemoryWriteStream::CMemoryWriteStream (int iMaxSize) :
		CObject(&g_WriteClass),
		m_iMaxSize(iMaxSize),
		m_pBlock(NULL)

//	CMemoryWriteStream constructor

	{
	if (m_iMaxSize == 0)
		m_iMaxSize = DEFAULT_MAX_SIZE;
	}

CMemoryWriteStream::~CMemoryWriteStream (void)

//	CMemoryWriteStream destructor

	{
	//	Close the stream if necessary

	if (m_pBlock)
		{
        munmap(m_pBlock, m_iCommittedSize);
		}
	}

ALERROR CMemoryWriteStream::Close (void)

//	CMemoryWriteStream
//
//	Close the stream

	{
	return NOERROR;
	}

ALERROR CMemoryWriteStream::Create (void)

//	Create
//
//	Creates a new file

	{
	//	Reserve a block of memory equal to the maximum size requested

	ASSERT(m_pBlock == NULL);
	m_pBlock = (char *)mmap(NULL, m_iMaxSize, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (m_pBlock == MAP_FAILED)
		{
		return ERR_MEMORY;
		}

	//	Initialize

	m_iCommittedSize = m_iMaxSize;
	m_iCurrentSize = 0;

	return NOERROR;
	}

ALERROR CMemoryWriteStream::Write (char *pData, int iLength, int *retiBytesWritten)

//	Write
//
//	Writes the given bytes to the file. If this call returns NOERROR, it is
//	guaranteed that the requested number of bytes were written.

	{
	void *res;
	//	Make sure we called Create

	ASSERT(m_pBlock);

	//	Commit the required space

	if (m_iCurrentSize + iLength > m_iCommittedSize)
		{
		assert(0);
		}

	//	Copy the stuff over

	memcpy(m_pBlock + m_iCurrentSize, pData, iLength);
	m_iCurrentSize += iLength;
	if (retiBytesWritten)
		*retiBytesWritten = iLength;

	return NOERROR;
	}

CMemoryReadStream::CMemoryReadStream (void) :
		CObject(&g_WriteClass)

//	CMemoryReadStream constructor

	{
	}

CMemoryReadStream::CMemoryReadStream (char *pData, int iDataSize) :
		CObject(&g_WriteClass),
		m_pData(pData),
		m_iDataSize(iDataSize)

//	CMemoryReadStream constructor

	{
#ifdef DEBUG
	m_iPos = -1;
#endif
	}

CMemoryReadStream::~CMemoryReadStream (void)

//	CMemoryReadStream destructor

	{
	}

ALERROR CMemoryReadStream::Read (char *pData, int iLength, int *retiBytesRead)

//	Read

	{
	ASSERT(m_iPos >= 0);	//	This happens if we don't Open the stream first

	ALERROR error = NOERROR;

	//	If we don't have enough data left, read out what we can

	if (m_iPos + iLength > m_iDataSize)
		{
		iLength = m_iDataSize - m_iPos;
		error = ERR_ENDOFFILE;
		}

	//	Copy the stuff over

	if (pData)
		memcpy(pData, m_pData + m_iPos, iLength);
	m_iPos += iLength;
	if (retiBytesRead)
		*retiBytesRead = iLength;

	return error;
	}

