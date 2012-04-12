//	CFileWriteStream.cpp
//
//	Implements CFileWriteStream object

#include "portage.h"
#include "SDL.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"

#include "CStream.h"

static DATADESCSTRUCT g_DataDesc[] =
	{	{ DATADESC_OPCODE_VTABLE,		1,	0 },		//	IWriteStream virtuals
		{ DATADESC_OPCODE_EMBED_OBJ,	1,	0 },		//	m_sFilename
		{ DATADESC_OPCODE_INT,			1,	0 },		//	m_bUnique
		{ DATADESC_OPCODE_INT,			1,	0 },		//	m_hFile
		{ DATADESC_OPCODE_STOP,	0,	0 } };
static CObjectClass<CFileWriteStream>g_Class(OBJID_CFILEWRITESTREAM, g_DataDesc);

CFileWriteStream::CFileWriteStream (void) :
		CObject(&g_Class)

//	CFileWriteStream constructor

	{
	}

CFileWriteStream::CFileWriteStream (const CString &sFilename, BOOL bUnique) :
		CObject(&g_Class),
		m_sFilename(sFilename),
		m_bUnique(bUnique),
		m_hFile(NULL)

//	CFileWriteStream constructor

	{
	}

CFileWriteStream::~CFileWriteStream (void)

//	CFileWriteStream destructor

	{
	//	Close the file if necessary

	Close();
	}

ALERROR CFileWriteStream::Close (void)

//	CFileWriteStream
//
//	Close the stream

	{
	if (m_hFile == NULL)
		return NOERROR;

	SDL_RWclose(m_hFile);
	
	m_hFile = NULL;

	return NOERROR;
	}

ALERROR CFileWriteStream::Create (void)

//	Create
//
//	Creates a new file

	{
	ASSERT(m_hFile == NULL);

	m_hFile = SDL_RWFromFile(m_sFilename.GetASCIIZPointer(), "w");
	if (m_hFile == NULL)
		{
		return ERR_FAIL;
		}

	return NOERROR;
	}

ALERROR CFileWriteStream::Write (char *pData, int iLength, int *retiBytesWritten)

//	Write
//
//	Writes the given bytes to the file. If this call returns NOERROR, it is
//	guaranteed that the requested number of bytes were written.

	{
	int iBytesWritten = 0;
	DWORD dwWritten;

	ASSERT(m_hFile);

	//	Pass through the filters

	dwWritten = SDL_RWwrite(m_hFile, pData, iLength, 1);
	if (dwWritten < 0)
		{
		if (retiBytesWritten)
			*retiBytesWritten = 0;

		return ERR_FAIL;
		}

	if (retiBytesWritten)
		*retiBytesWritten = iLength;

	return NOERROR;

	}

