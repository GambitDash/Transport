//	CResourceReadBlock.cpp
//
//	Implements CResourceReadBlock object

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"

#include "CString.h"
#include "CReadBlock.h"

static CObjectClass<CResourceReadBlock>g_Class(OBJID_CRESOURCEREADBLOCK, NULL);

CResourceReadBlock::CResourceReadBlock (void) :
		CObject(&g_Class)

//	CResourceReadBlock constructor

	{
	}

CResourceReadBlock::CResourceReadBlock (char *pszRes) :
		CObject(&g_Class),
		m_pData(NULL)

//	CResourceReadBlock constructor

	{
	}

CResourceReadBlock::~CResourceReadBlock (void)

//	CResourceReadBlock destructor

	{
	//	Close the file if necessary

	Close();
	}

ALERROR CResourceReadBlock::Close (void)

//	CResourceReadBlock
//
//	Close the stream

	{
	return NOERROR;
	}

ALERROR CResourceReadBlock::Open (void)

//	Open
//
//	Opens the stream for reading

	{
	/* XXX Resources will be compiled in bytecode (and preferably gziped).
	 *     This function provides the bytes for reading.
	 */
	return NOERROR;
	}
