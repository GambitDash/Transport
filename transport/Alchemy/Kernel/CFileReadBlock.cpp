//	CFileReadBlock.cpp
//
//	Implements CFileReadBlock object

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

#include "CReadBlock.h"

static CObjectClass<CFileReadBlock>g_Class(OBJID_CFILEREADBLOCK, NULL);

CFileReadBlock::CFileReadBlock (void) :
		CObject(&g_Class),
		m_pFile(NULL),
		m_dwFileSize(0)

//	CFileReadBlock constructor

	{
	}

CFileReadBlock::CFileReadBlock (const CString &sFilename) :
		CObject(&g_Class),
		m_sFilename(sFilename),
		m_hFile(0),
		m_dwFileSize(0)

//	CFileReadBlock constructor

	{
	}

CFileReadBlock::~CFileReadBlock (void)

//	CFileReadBlock destructor

	{
	//	Close the file if necessary

	Close();
	}

ALERROR CFileReadBlock::Close (void)

//	CFileReadBlock
//
//	Close the stream

	{
	if (m_hFile == 0)
		return NOERROR;

	munmap(m_pFile, m_dwFileSize);

	close(m_hFile);

	m_hFile = 0;
	m_dwFileSize = 0;
	m_pFile = NULL;

	return NOERROR;
	}

ALERROR CFileReadBlock::Open (void)

//	Open
//
//	Maps a file into process memory for direct access.

	{
	struct stat st;
	int err;

	ASSERT(m_hFile == NULL);

	m_hFile = open(m_sFilename.GetASCIIZPointer(), O_RDONLY);
	if (m_hFile == 0)
		{
		return ERR_FAIL;
		}

	err = fstat(m_hFile, &st);
	if (err < 0)
		{
		return ERR_FAIL;
		}

	m_dwFileSize = st.st_size;

	m_pFile = (char *)mmap(NULL, m_dwFileSize, PROT_READ, MAP_SHARED, m_hFile, 0);
	if (m_pFile == NULL)
		{
		close(m_hFile);
		return ERR_FAIL;
		}

	return NOERROR;
	}
