//	CCString.cpp
//
//	Implements CCString class

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"
#include "CMath.h"

#include "CStream.h"
#include "CCCodeChain.h"

static CObjectClass<CCString>g_Class(OBJID_CCSTRING, NULL);

CCString::CCString (void) : ICCString(&g_Class),
		m_dwBinding(0),
		m_pBinding(NULL)

//	CCString constructor

	{
	}

ICCItem *CCString::Clone (CCodeChain *pCC)

//	Clone
//
//	Returns a new item with a single ref-count

	{
	ICCItem *pResult;
	CCString *pClone;
	
	pResult = pCC->CreateString(m_sValue);
	if (pResult->IsError())
		return pResult;

	pClone = (CCString *)pResult;
	pClone->CloneItem(this);
	pClone->m_dwBinding = m_dwBinding;
	if (m_pBinding)
		pClone->m_pBinding = m_pBinding->Reference();
	else
		pClone->m_pBinding = NULL;

	return pClone;
	}

void CCString::DestroyItem (CCodeChain *pCC)

//	DestroyItem
//
//	Destroys the item

	{
	if (m_pBinding)
		{
		m_pBinding->Discard(pCC);
		m_pBinding = NULL;
		}

#ifdef DEBUG
	//	Clear out the value so that this string doesn't
	//	appear to be leaked.
	m_sValue = CString();
#endif
	pCC->DestroyString(this);
	}

BOOL CCString::GetBinding (int *retiFrame, int *retiOffset)

//	GetBinding
//
//	Returns the binding of this identifier

	{
	int iFrame, iOffset;

	iFrame = LOWORD(m_dwBinding);
	iOffset = HIWORD(m_dwBinding);

	if (iFrame == 0)
		return FALSE;
	else
		{
		*retiFrame = iFrame - 1;
		*retiOffset = iOffset;
		return TRUE;
		}
	}

CString CCString::Print (CCodeChain *pCC)

//	Print
//
//	Print item

	{
	//	If it is quoted, or there is whitespace, we need to surround the entry
	//	with quotes

	if (IsQuoted() || (strFind(m_sValue, CONSTLIT(" ")) != -1))
		{
		char *pBuffer = new char [2 * m_sValue.GetLength() + 32];
		char *pDest = pBuffer;
		char *pPos = m_sValue.GetASCIIZPointer();

		*pDest++ = '\"';

		while (*pPos != '\0')
			{
			if (*pPos == '\\')
				{
				*pDest++ = '\\';
				*pDest++ = '\\';
				}
			else if (*pPos == '\"')
				{
				*pDest++ = '\\';
				*pDest++ = '\"';
				}
			else
				*pDest++ = *pPos;

			pPos++;
			}

		*pDest++ = '\"';
		*pDest++ = '\0';

		//	Done

		CString sResult(pBuffer);
		delete pBuffer;
		return sResult;
		}
	else
		return m_sValue;
	}

void CCString::Reset (void)

//	Reset
//
//	Reset to initial conditions

	{
	ASSERT(m_dwRefCount == 0);
	m_sValue = LITERAL("");
	m_dwBinding = 0;
	m_pBinding = NULL;
	}

void CCString::SetBinding (int iFrame, int iOffset)

//	SetBinding
//
//	Sets the binding

	{
	m_dwBinding = MAKELONG(iFrame + 1, iOffset);
	}

void CCString::SetFunctionBinding (CCodeChain *pCC, ICCItem *pBinding)

//	SetFunctionBinding
//
//	Associates a function with the string

	{
	if (m_pBinding)
		m_pBinding->Discard(pCC);

	m_pBinding = pBinding->Reference();
	}

ICCItem *CCString::StreamItem (CCodeChain *pCC, IWriteStream *pStream)

//	StreamItem
//
//	Stream the sub-class specific data

	{
	ALERROR error;
	DWORD dwLength;
	DWORD dwPaddedLength;

	//	Write out the length of the string

	dwLength = m_sValue.GetLength();
	error = pStream->Write((char *)&dwLength, sizeof(dwLength), NULL);
	if (error)
		return pCC->CreateSystemError(error);

	//	Write out the string

	error = pStream->Write(m_sValue.GetPointer(), dwLength, NULL);
	if (error)
		return pCC->CreateSystemError(error);

	//	Write out any padding

	dwPaddedLength = AlignUp(dwLength, sizeof(DWORD));
	if (dwPaddedLength - dwLength > 0)
		{
		error = pStream->Write((char *)&dwLength, dwPaddedLength - dwLength, NULL);
		if (error)
			return pCC->CreateSystemError(error);
		}

	return pCC->CreateTrue();
	}

ICCItem *CCString::UnstreamItem (CCodeChain *pCC, IReadStream *pStream)

//	UnstreamItem
//
//	Unstream the sub-class specific data

	{
	ALERROR error;
	DWORD dwLength;
	DWORD dwPaddedLength;
	char *pPos;

	//	Read the length
	error = pStream->Read((char *)&dwLength, sizeof(dwLength), NULL);
	if (error)
		return pCC->CreateSystemError(error);

	//	Grow the string

	pPos = m_sValue.GetWritePointer(dwLength);
	if (pPos == NULL)
		return pCC->CreateMemoryError();

	//	Read the string
	error = pStream->Read(pPos, dwLength, NULL);
	if (error)
		return pCC->CreateSystemError(error);

	//	Read the padding

	dwPaddedLength = AlignUp(dwLength, sizeof(DWORD));
	if (dwPaddedLength - dwLength > 0)
		{
		error = pStream->Read((char *)&dwLength, dwPaddedLength - dwLength, NULL);
		if (error)
			return pCC->CreateSystemError(error);
		}

	return pCC->CreateTrue();
	}

