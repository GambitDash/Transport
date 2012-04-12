//	CTextFileLog.cpp
//
//	Implements CTextFileLog object

#include "SDL.h"

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"

#include "CLog.h"

static CObjectClass<CTextFileLog>g_Class(OBJID_CTEXTFILELOG, NULL);

CTextFileLog::CTextFileLog (void) : CObject(&g_Class),
		m_hFile(NULL)

//	CTextFileLog constructor

	{
	}

CTextFileLog::CTextFileLog (const CString &sFilename) : CObject(&g_Class),
		m_hFile(NULL),
		m_sFilename(sFilename)

//	CTextFileLog constructor

	{
	}

CTextFileLog::~CTextFileLog (void)

//	CTextFileLog destructor

	{
	Close();
	}

ALERROR CTextFileLog::Close (void)

//	Close
//
//	Close the log

	{
	if (m_hFile == NULL)
		return NOERROR;

	SDL_RWclose(m_hFile);
	m_hFile = NULL;

	return NOERROR;
	}

ALERROR CTextFileLog::Create (BOOL bAppend)

//	Create
//
//	Create a new log file

	{
	ASSERT(m_hFile == NULL);

	m_hFile = SDL_RWFromFile(m_sFilename.GetASCIIZPointer(),
			bAppend ? "a" : "w");
	if (m_hFile == NULL)
		{
		return ERR_FAIL;
		}

	return NOERROR;
	}

void CTextFileLog::LogOutput (DWORD dwFlags, const char *pszLine, ...)

//	LogOutput
//
//	Output a line to the log

	{
	char szLine[1024];
	va_list ap;
	int l;

	ASSERT(m_hFile);

	va_start(ap, pszLine);
	l = vsnprintf(szLine, 1024, pszLine, ap);
	szLine[l] = '\n';

	//	XXX Append time date

	//	Write out the line

	SDL_RWwrite(m_hFile, szLine, l + 1, 1);

	}

void CTextFileLog::Flush (void)
	{
		Close();
		Create(true);
	}

void CTextFileLog::SetFilename (const CString &sFilename)

//	SetFilename
//
//	Sets the filename of the log file

	{
	ASSERT(m_hFile == NULL);
	m_sFilename = sFilename;
	}

