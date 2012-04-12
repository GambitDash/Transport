//	CString.cpp
//
//	Implementation of standard string class

#include "SDL.h"
#include "SDL_thread.h"

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"

#include "CArchive.h"
#include "CStringArray.h"

#define STORE_SIZE_INCREMENT				256
#define STORE_ALLOC_MAX						(8 * 1024 * 1024)
#define STORE_SIZE_INIT						(STORE_ALLOC_MAX / sizeof(STORESTRUCT))

static DATADESCSTRUCT g_DataDesc[] =
	{	{ DATADESC_OPCODE_REFERENCE,	1,	0 },		//	m_pStore
		{ DATADESC_OPCODE_STOP,	0,	0 } };
static CObjectClass<CString>g_Class(OBJID_CSTRING, g_DataDesc);

CString::PSTORESTRUCT			CString::g_pStore;
int									CString::g_iStoreSize;
CString::PSTORESTRUCT			CString::g_pFreeStore;
static SDL_mutex *				g_csStore;
const CString						NULL_STR;

#ifdef DEBUG_STRING_LEAKS
int g_iStoreCount = 0;
#endif

static char g_LowerCaseAbsoluteTable[256];

inline char strLowerCaseAbsolute (char chChar) { return g_LowerCaseAbsoluteTable[(BYTE)chChar]; }

CString::CString (void) :
		CObject(&g_Class),
		m_pStore(NULL)

//	CString constructor

	{
	}

CString::CString (const char *pString) :
		CObject(&g_Class),
		m_pStore(NULL)

//	CString constructor

	{
	if (pString)
		Transcribe(pString, -1);
	}

CString::CString (const char *pString, int iLength) :
		CObject(&g_Class),
		m_pStore(NULL)

//	CString constructor

	{
	if (pString)
		Transcribe(pString, iLength);
	}

CString::CString (const char *pString, int iLength, BOOL bExternal) :
		CObject(&g_Class),
		m_pStore(NULL)

//	CString constructor

	{
	if (pString)
		{
		if (bExternal)
			{
			//	Allocate a storage block (if necessary)

			if (iLength > 0)
				{
				m_pStore = AllocStore(0, FALSE);
				if (m_pStore)
					{
					//	A negative value means that this is an external
					//	read-only storage

					m_pStore->iAllocSize = -iLength;
					m_pStore->iLength = iLength;
					m_pStore->pString = (char *)pString;
					}
				}
			}
		else
			Transcribe(pString, iLength);
		}
	}

CString::~CString (void)

//	CString destructor

	{
	DecRefCount();
	}

CString::CString (const CString &pString) :
		CObject(&g_Class),
		m_pStore(NULL)

//	CString copy constructor

	{
	m_pStore = pString.m_pStore;

	//	Up the ref count

	if (m_pStore)
		m_pStore->iRefCount++;
	}

CString &CString::operator= (const CString &pString)

//	operator =
//
//	Overrides the assignment operator

	{
	DecRefCount();

	m_pStore = pString.m_pStore;

	//	If we've got a storage, bump up the ref count

	if (m_pStore)
		m_pStore->iRefCount++;

	return *this;
	}

void CString::AddToFreeList (PSTORESTRUCT pStore, int iSize)

//	AddToFreeList
//
//	Adds the array of storage locations to the free list. Note that
//	this routine assumes that the critical section has been locked

	{
	int i;

	for (i = 0; i < iSize; i++)
		{
		pStore->iRefCount = 0;
		pStore->pString = (char *)g_pFreeStore;
		g_pFreeStore = pStore;
		pStore++;
		}
	}

CString::PSTORESTRUCT CString::AllocStore (int iSize, BOOL bAllocString)

//	AllocStore
//
//	Allocates a new string store of at least the given size

	{
	PSTORESTRUCT pStore;

	//	Critical section

	SDL_MutexTake(g_csStore);

	//	If we haven't yet created the store array, do it now

	if (g_pStore == NULL)
		{
		//	Reserve a megabyte of virtual memory

		g_pStore = new STORESTRUCT[STORE_SIZE_INIT];
		if (g_pStore == NULL)
			{
			SDL_MutexGive(g_csStore);
			ASSERT(false);
			return NULL;
			}

		//	Initialize the free list

		AddToFreeList(g_pStore, STORE_SIZE_INIT);

		g_iStoreSize = STORE_SIZE_INIT;
		}

	//	If there're no more free entries, re-alloc the store array

	if (g_pFreeStore == NULL)
		{
		ASSERT(false);
		}

	//	Pick a block off the free list

	pStore = g_pFreeStore;
	g_pFreeStore = (PSTORESTRUCT)pStore->pString;

#ifdef DEBUG_STRING_LEAKS
	g_iStoreCount++;
#endif

	SDL_MutexGive(g_csStore);

	//	Initialize it

	if (bAllocString)
		{
		pStore->iRefCount = 1;
		pStore->iAllocSize = iSize;
		pStore->iLength = 0;
		pStore->pString = new char[iSize];
		}
	else
		{
		pStore->iRefCount = 1;
		pStore->iAllocSize = 0;
		pStore->iLength = 0;
		pStore->pString = NULL;
		}

#ifdef DEBUG_STRING_LEAKS
	pStore->iMark = 0;
	if (iSize == 1)
		::GetTickCount();
#endif

	return pStore;
	}

ALERROR CString::Append (const CString &sString)

//	Append
//
//	Appends the given string

	{
	int iLength = sString.GetLength();
	char *pString = sString.GetPointer();
	int i;
	int iStart;

	iStart = GetLength();
	if (!Size(GetLength() + iLength+1, TRUE))
		return ERR_MEMORY;

	for (i = 0; i < iLength; i++)
		m_pStore->pString[iStart + i] = pString[i];

	m_pStore->iLength += iLength;

	//	NULL terminate

	m_pStore->pString[GetLength()] = '\0';

	return NOERROR;
	}

void CString::Capitalize (CapitalizeOptions iOption)

//	Capitalize
//
//	Capitalizes the string in place

	{
	//	Can't deal with NULL strings

	if (GetLength() == 0)
		return;

	//	Make sure we have our own copy

	Size(GetLength()+1, true);
	char *pPos = GetASCIIZPointer();

	//	Capitalize

	switch (iOption)
		{
		case capFirstLetter:
			{
			//	Capitalize first letter

			*pPos = toupper(*pPos);
			break;
			}

		default:
			ASSERT(false);
		}
	}

void CString::CopyHandler (CObject *pOriginal)

//	CopyHandler
//
//	Handle special tasks when copying object

	{
	//	If we've got a store, up the refcount

	if (m_pStore)
		m_pStore->iRefCount++;
	}

#ifndef INLINE_DECREF
void CString::DecRefCount (void)

//	DecRefCount
//
//	Decrements reference count on storage

	{
	//	If we've got a storage block, de-reference it

	if (m_pStore)
		{
		ASSERT(m_pStore->iRefCount > 0);
		m_pStore->iRefCount--;

		//	If we're done, free the block

		if (m_pStore->iRefCount == 0)
			{
			SDL_MutexTake(g_csStore);
			if (!IsExternalStorage())
				delete []m_pStore->pString;
			AddToFreeList(m_pStore, 1);
			SDL_MutexGive(g_csStore);
			}
		}
	}
#endif

void CString::FreeStore (PSTORESTRUCT pStore)

//	FreeStore
//
//	Free the store

	{
	SDL_MutexTake(g_csStore);
	if (pStore->iAllocSize >= 0)	//	!IsExternalStorage()
		delete []pStore->pString;
	AddToFreeList(pStore, 1);

#ifdef DEBUG_STRING_LEAKS
	g_iStoreCount--;
#endif

	SDL_MutexGive(g_csStore);
	}

char *CString::GetASCIIZPointer (void) const

//	GetASCIIZPointer
//
//	Returns a pointer to a NULL terminated array of characters

	{
	//	We always NULL terminate strings

	return GetPointer();
	}

int CString::GetLength (void) const

//	GetLength
//
//	Returns the number of characters in the string

	{
	if (m_pStore)
		return m_pStore->iLength;
	else
		return 0;
	}

char *CString::GetPointer (void) const

//	GetPointer
//
//	Returns a pointer to an array of characters. The caller must not
//	access elements beyond the length of the string

	{
	if (m_pStore)
		return m_pStore->pString;
	else
		return "";
	}

char *CString::GetWritePointer (int iLength)

//	GetWritePointer
//
//	Returns a pointer that allows the user to write to the string.
//	On return, the buffer is guaranteed to be at least iLength.

	{
	if (!Size(iLength + 1, TRUE))
		return NULL;

	m_pStore->iLength = iLength;
	m_pStore->pString[iLength] = '\0';

	return m_pStore->pString;
	}

void CString::InitLowerCaseAbsoluteTable (void)

//	InitLowerCaseAbsoluteTable
//
//	Initialize lowercase table. The table generates a lowercase map
//	for the Windows Western character set. This should be used only
//	for non-localized purposes, such as absolute sorting used by CSymbolTable

	{
	int chChar;

	for (chChar = 0; chChar < 256; chChar++)
		{
		if (chChar >= 'A' && chChar <= 'Z')
			g_LowerCaseAbsoluteTable[chChar] = chChar + 0x20;
		else if (chChar == 0x8A)
			g_LowerCaseAbsoluteTable[chChar] = (BYTE)0x9A;
		else if (chChar == 0x8C)
			g_LowerCaseAbsoluteTable[chChar] = (BYTE)0x9C;
		else if (chChar == 0x8E)
			g_LowerCaseAbsoluteTable[chChar] = (BYTE)0x9E;
		else if (chChar == 0x9F)
			g_LowerCaseAbsoluteTable[chChar] = (BYTE)0xFF;
		else if (chChar >= 0xC0 && chChar <= 0xD6)
			g_LowerCaseAbsoluteTable[chChar] = chChar + 0x20;
		else if (chChar >= 0xD8 && chChar <= 0xDE)
			g_LowerCaseAbsoluteTable[chChar] = chChar + 0x20;
		else
			g_LowerCaseAbsoluteTable[chChar] = chChar;
		}
	}

ALERROR CString::LoadHandler (CUnarchiver *pUnarchiver)

//	Load
//
//	Load the string

	{
	ALERROR error;
	DWORD dwLength;

	//	Load the length of the string

	if (error = pUnarchiver->ReadData((char *)&dwLength, sizeof(DWORD)))
		return error;

	//	If we've got nothing, leave now

	if (dwLength == 0)
		return NOERROR;

	//	Size the string appropriately

	Size(dwLength+1, FALSE);

	//	Load the string

	if (error = pUnarchiver->ReadData(m_pStore->pString, dwLength))
		return error;

	m_pStore->iLength = (int)dwLength;

	//	NULL terminate

	m_pStore->pString[dwLength] = '\0';

	//	Skip beyond to pad to DWORD boundary

	dwLength %= sizeof(DWORD);
	if (dwLength > 0)
		{
		if (error = pUnarchiver->ReadData((char *)&dwLength, sizeof(DWORD) - dwLength))
			return error;
		}

	return NOERROR;
	}

void CString::ReadFromStream (IReadStream *pStream)

//	ReadFromStream
//
//	Reads the string from a stream

	{
	DWORD dwLength;

	//	Load the length of the string and size the string

	pStream->Read((char *)&dwLength, sizeof(DWORD));
	if (dwLength == 0)
		{
		Truncate(0);
		return;
		}

	Size(dwLength+1, FALSE);

	//	Load the string

	pStream->Read(m_pStore->pString, dwLength);
	m_pStore->iLength = (int)dwLength;

	//	NULL terminate

	m_pStore->pString[dwLength] = '\0';

	//	Skip beyond to pad to DWORD boundary

	dwLength %= sizeof(DWORD);
	if (dwLength > 0)
		pStream->Read((char *)&dwLength, sizeof(DWORD) - dwLength);
	}

ALERROR CString::SaveHandler (CArchiver *pArchiver)

//	Save
//
//	Override saving the string because we need to do special stuff
//	with the string store.

	{
	ALERROR error;
	DWORD dwLength;

	//	Write out the length of the string

	dwLength = (DWORD)GetLength();
	if (error = pArchiver->WriteData((char *)&dwLength, sizeof(DWORD)))
		return error;

	//	Write out the string itself

	if (dwLength > 0)
		{
		if (error = pArchiver->WriteData(GetPointer(), dwLength))
			return error;

		//	Pad the string so that we're always on a DWORD boundary

		dwLength %= sizeof(DWORD);
		if (dwLength > 0)
			{
			if (error = pArchiver->WriteData((char *)&dwLength, sizeof(DWORD)-dwLength))
				return error;
			}
		}

	return NOERROR;
	}

BOOL CString::Size (int iLength, BOOL bPreserveContents)

//	Size
//
//	Resizes the storage so that it is at least as big as the
//	given length. This routine also makes sure that we are the only
//	object using this storage.
//	Return FALSE if failed

	{
	//	If we don't have storage yet, allocate a new one

	if (m_pStore == NULL)
		{
		m_pStore = AllocStore(iLength, TRUE);
		if (m_pStore == NULL)
			goto Fail;
		}

	//	If we're sharing the store with someone else, make our own copy

	if (m_pStore->iRefCount > 1)
		{
		PSTORESTRUCT pNewStore;

		pNewStore = AllocStore(iLength, TRUE);
		if (pNewStore == NULL)
			goto Fail;

		//	If we're supposed to preserve contents, copy the content over

		if (bPreserveContents)
			{
			int i;
			int iCopyLen = std::min(m_pStore->iLength+1, iLength);

			for (i = 0; i < iCopyLen; i++)
				pNewStore->pString[i] = m_pStore->pString[i];

			pNewStore->iLength = m_pStore->iLength;
			}

		m_pStore->iRefCount--;
		m_pStore = pNewStore;
		}

	//	If we're not big enough, re-allocate
	//	Note that when iAllocSize is negative (meaning that we have an
	//	external storage) we always reallocate

	if (IsExternalStorage() || m_pStore->iAllocSize < iLength)
		{
		char *pNewString;

		pNewString = new char[iLength];
		if (pNewString == NULL)
			goto Fail;

		//	If we're supposed to preserve contents, copy the content over

		if (bPreserveContents)
			{
			int i;
			int iCopyLen = std::min(m_pStore->iLength, iLength);

			for (i = 0; i < iCopyLen; i++)
				pNewString[i] = m_pStore->pString[i];
			}

		//	Only free if this is our storage

		if (!IsExternalStorage())
			delete []m_pStore->pString;

		m_pStore->pString = pNewString;
		m_pStore->iAllocSize = iLength;
		}

	//	Done

	ASSERT(m_pStore);
	ASSERT(m_pStore->iRefCount == 1);
	ASSERT(m_pStore->iAllocSize >= iLength);

	return TRUE;

Fail:

	return FALSE;
	}

ALERROR CString::Transcribe (const char *pString, int iLen)

//	Transcribe
//
//	Copies the string for a char * buffer.
//	If iLen is -1, we assume ASCIIZ

	{
	int i;

	//	Handle NULL

	if (pString == NULL)
		{
		DecRefCount();
		m_pStore = NULL;
		return NOERROR;
		}

	//	Handle ASCIIZ

	if (iLen == -1)
		iLen = strlen(pString);

	//	Handle 0 length

	if (iLen == 0)
		{
		DecRefCount();
		m_pStore = NULL;
		return NOERROR;
		}

	//	Allocate size

	if (!Size(iLen+1, FALSE))
		return ERR_MEMORY;

	for (i = 0; i < iLen; i++)
		m_pStore->pString[i] = pString[i];

	m_pStore->iLength = iLen;

	//	NULL terminate

	m_pStore->pString[iLen] = '\0';

	return NOERROR;
	}

void CString::Truncate (int iLength)

//	Truncate
//
//	Truncate the string to the given length. The length cannot be greater
//	than the current length of the string

	{
	ASSERT(iLength >= 0 && iLength <= GetLength());

	if (iLength == 0)
		{
		DecRefCount();
		m_pStore = NULL;
		return;
		}

	//	Call this just to make sure that we have our own copy

	Size(iLength+1, TRUE);

	//	Set the new length

	m_pStore->iLength = iLength;
	m_pStore->pString[iLength] = '\0';
	}

void CString::WriteToStream (IWriteStream *pStream) const

//	WriteToStream
//
//	Write out to a stream

	{
	DWORD dwLength;

	//	Write out the length of the string

	dwLength = GetLength();
	pStream->Write((char *)&dwLength, sizeof(dwLength));

	//	Write out the string itself

	if (dwLength > 0)
		{
		pStream->Write(GetPointer(), dwLength);

		//	Pad the string so that we're always on a DWORD boundary

		dwLength %= sizeof(DWORD);
		if (dwLength > 0)
			pStream->Write((char *)&dwLength, sizeof(DWORD)-dwLength);
		}
	}

int strCompare (const CString &sString1, const CString &sString2)

//	strCompare
//
//	Compares two strings are returns 1 if sString1 is > sString2; -1 if sString1
//	is < sString2; and 0 if both strings are equal

	{
	char *pPos1;
	char *pPos2;
	int i, iLen;

	//	Setup

	pPos1 = sString1.GetPointer();
	pPos2 = sString2.GetPointer();
	iLen = std::min(sString1.GetLength(), sString2.GetLength());

	//	Compare by commonality

	for (i = 0; i < iLen; i++)
		{
		if (tolower(*pPos1) > tolower(*pPos2))
			return 1;
		else if (tolower(*pPos1) < tolower(*pPos2))
			return -1;

		pPos1++;
		pPos2++;
		}

	//	If the strings match up to a point, check to see which is 
	//	longest.

	if (sString1.GetLength() > sString2.GetLength())
		return 1;
	else if (sString1.GetLength() < sString2.GetLength())
		return -1;
	else
		return 0;
	}

int strCompareAbsolute (const CString &sString1, const CString &sString2)

//	strCompareAbsolute
//
//	Compares two strings are returns 1 if sString1 is > sString2; -1 if sString1
//	is < sString2; and 0 if both strings are equal.
//
//	The resulting sort order does not change with locale. Use this only for
//	internal sorting (e.g., symbol tables).

	{
	char *pPos1;
	char *pPos2;
	int i, iLen;

	//	Setup

	pPos1 = sString1.GetPointer();
	pPos2 = sString2.GetPointer();
	iLen = Min(sString1.GetLength(), sString2.GetLength());

	//	Compare by commonality

	for (i = 0; i < iLen; i++)
		{
		char chChar1 = strLowerCaseAbsolute(*pPos1++);
		char chChar2 = strLowerCaseAbsolute(*pPos2++);

		if (chChar1 > chChar2)
			return 1;
		else if (chChar1 < chChar2)
			return -1;
		}

	//	If the strings match up to a point, check to see which is 
	//	longest.

	if (sString1.GetLength() > sString2.GetLength())
		return 1;
	else if (sString1.GetLength() < sString2.GetLength())
		return -1;
	else
		return 0;
	}

ALERROR strDelimitEx (const CString &sString, 
					  char cDelim, 
					  DWORD dwFlags,
					  int iMinParts, 
					  CStringArray *pStringList)

//	strDelimitEx
//
//	Parses the given string into multiple strings using the given delimeter. 

	{
	ALERROR error;
	char *pPos;
	char *pPartStart;
	int iPartLength;
	int iPartCount;

	ASSERT(cDelim != '\0');

	//	Initialize string list

	pStringList->RemoveAll();

	//	Get info about the string

	pPos = sString.GetPointer();

	//	Parse the string

	iPartCount = 0;
	pPartStart = pPos;
	iPartLength = 0;

	while (*pPos != '\0')
		{
		//	If we've found a delimeter, then flush the string up to now
		//	to the current part.

		if (*pPos == cDelim)
			{
			CString sPart(pPartStart, iPartLength);
			if (dwFlags & DELIMIT_TRIM_WHITESPACE)
				sPart = strTrimWhitespace(sPart);

			if ((dwFlags & DELIMIT_ALLOW_BLANK_STRINGS) || !sPart.IsBlank())
				{
				if (error = pStringList->AppendString(sPart, NULL))
					goto Fail;

				iPartCount++;
				}

			//	Skip to the next part

			pPos++;
			iPartLength = 0;

			pPartStart = pPos;
			}
		else
			{
			pPos++;
			iPartLength++;
			}
		}

	//	Transcribe the last part

	if (iPartLength > 0)
		{
		CString sPart(pPartStart, iPartLength);
		if (dwFlags & DELIMIT_TRIM_WHITESPACE)
			sPart = strTrimWhitespace(sPart);

		if ((dwFlags & DELIMIT_ALLOW_BLANK_STRINGS) || !sPart.IsBlank())
			{
			if (error = pStringList->AppendString(sPart, NULL))
				goto Fail;

			iPartCount++;
			}
		}

	//	If we haven't yet created the minimum number of parts,
	//	create the rest

	if (iMinParts != -1 && iPartCount < iMinParts)
		{
		int i;

		for (i = 0; i < iMinParts - iPartCount; i++)
			{
			if (error = pStringList->AppendString(CString(), NULL))
				goto Fail;
			}
		}

	//	Done

	return NOERROR;

Fail:

	return error;
	}

bool strEquals (const CString &sString1, const CString &sString2)

//	strEquals
//
//	Returns TRUE if the strings are equal

	{
	char *pPos1;
	char *pPos2;
	int i, iLen;

	//	If the strings aren't the same length then don't bother

	iLen = sString1.GetLength();
	if (iLen != sString2.GetLength())
		return false;

	//	Setup

	pPos1 = sString1.GetPointer();
	pPos2 = sString2.GetPointer();

	//	Compare by commonality

	for (i = 0; i < iLen; i++)
		if (tolower(pPos1[i]) != tolower(pPos2[i]))
			return false;

	return true;
	}

int strFind (const CString &sString, const CString &sTarget)

//	strFind
//
//	Finds the target string in the given string and returns the
//	offset in sString at which the target starts. If the target is
//	not found anywhere in sString then we return -1

	{
	int iStringLen = sString.GetLength();
	int iTargetLen = sTarget.GetLength();
	char *pString = sString.GetPointer();
	int i;

	//	If the target is null, then we don't match

	if (sTarget.IsBlank())
		return -1;

	//	Search for a match at successive offsets of sString
	//	until we're past the point where sTarget would not
	//	fit.

	for (i = 0; i <= iStringLen - iTargetLen; i++)
		{
		CString sTest(pString + i, iTargetLen, TRUE);

		if (strCompare(sTest, sTarget) == 0)
			return i;
		}

	//	Didn't find it

	return -1;
	}

CString strFromInt (int iInteger, BOOL bSigned)

//	CStringFromInt
//
//	Converts an integer to a string

	{
	char szString[256];
	int iLen;

	if (bSigned)
		iLen = sprintf(szString, "%d", iInteger);
	else
		iLen = sprintf(szString, "%u", iInteger);

	CString sString(szString, iLen);
	return sString;
	}

void CString::INTStringCleanUp (void)

//	INTStringCleanUp
//
//	Cleans up the basic global data

	{
	SDL_DestroyMutex(g_csStore);
	delete []g_pStore;
	}

ALERROR CString::INTStringInit (void)

//	INTStringInit
//
//	Initializes the subsystem. This routine must be called before
//	any other call.

	{
	g_csStore = SDL_CreateMutex();
	InitLowerCaseAbsoluteTable();
	return NOERROR;
	}

CString strCapitalize (const CString &sString, int iOffset)

//	strCapitalize
//
//	Capitalizes the given letter

	{
	CString sUpper = sString;
	char *str = sUpper.GetWritePointer(sUpper.GetLength());
	str[iOffset] = toupper(str[iOffset]);
	return sUpper;
	}

CString strCapitalizeWords (const CString &sString)

//	strCapitalizeWords
//
//	Capitalizes the first letter in every word in the string

	{
	CString sUpper;
	char *pSrc = sString.GetASCIIZPointer();
	char *pDest = sUpper.GetWritePointer(sString.GetLength());
	bool bCapitalize = true;
	while (*pSrc != '\0')
		{
		*pDest = *pSrc;

		if (*pSrc == ' ' || *pSrc == '-')
			bCapitalize = true;
		else if (bCapitalize)
			{
			*pDest = toupper(*pDest);
			bCapitalize = false;
			}

		pDest++;
		pSrc++;
		}

	return sUpper;
	}

CString strCat (const CString &sString1, const CString &sString2)

//	strCat
//
//	Returns the concatenation of the two string

	{
	CString sCat;

	sCat = sString1;
	sCat.Append(sString2);
	return sCat;
	}

CString strCEscapeCodes (const CString &sString)

//	strCEscapeCodes
//
//	Parses C Language escape codes and returns the string

	{
	char *pPos = sString.GetASCIIZPointer();
	enum States
		{
		stateStart,
		stateFoundBackslash,
		stateDone,
		};
	int iState = stateStart;
	CString sResult;
	char *pStart = pPos;

	while (iState != stateDone)
		{
		switch (iState)
			{
			case stateStart:
				if (*pPos == '\0')
					{
					if (pStart == sString.GetASCIIZPointer())
						return sString;
					else
						return strCat(sResult, CString(pStart, pPos - pStart));
					}
				else if (*pPos == '\\')
					iState = stateFoundBackslash;
				break;

			case stateFoundBackslash:
				if (*pPos == '\0')
					return strCat(sResult, CString(pStart, pPos - pStart));
				else
					{
					CString sChar;
					if (*pPos == 'n')
						sChar = CString("\n");
					else if (*pPos == 'r')
						sChar = CString("\r");
					else if (*pPos == 't')
						sChar = CString("\t");
					else
						sChar = CString(pPos, 1);

					sResult.Append(CString(pStart, (pPos - pStart) - 1));
					sResult.Append(sChar);
					pStart = pPos + 1;
					}
				iState = stateStart;
				break;
			}

		pPos++;
		}

	return sResult;
	}

int strParseInt (char *pStart, int iNullResult, char **retpEnd, BOOL *retbNullValue)

//	strParseInt
//
//	pStart: Start parsing. Skips any leading whitespace
//	iNullResult: If there are no valid numbers, returns this value
//	retpEnd: Returns the character at which we stopped parsing
//	retbNullValue: Returns TRUE if there are no valid numbers.

	{
	char *pPos;
	BOOL bNegative;
	BOOL bFoundNumber;
	BOOL bHex;
	int iInt;

	//	Preset

	if (retbNullValue)
		*retbNullValue = FALSE;

	pPos = pStart;
	bNegative = FALSE;
	bFoundNumber = FALSE;
	bHex = FALSE;
	iInt = 0;

	//	Skip whitespace

	while (*pPos == ' ' || *pPos == '\t' || *pPos == '\n' || *pPos == '\r')
		pPos++;

	//	If NULL, then we're done

	if (*pPos == '\0')
		{
		if (retbNullValue)
			*retbNullValue = TRUE;

		if (retpEnd)
			*retpEnd = pPos;

		return iNullResult;
		}

	//	If negative, remember it

	if (*pPos == '-')
		{
		bNegative = TRUE;
		pPos++;
		}
	else if (*pPos == '+')
		pPos++;

	//	See if this is a hex number

	if (*pPos == '0')
		{
		pPos++;
		bFoundNumber = TRUE;

		//	If the next character is x (or X) then we've got
		//	a Hex number

		if (*pPos == 'x' || *pPos == 'X')
			{
			pPos++;
			bHex = TRUE;
			}
		}

	//	Keep parsing

	if (bHex)
		{
		DWORD dwInt = 0;

		while (*pPos != '\0' 
				&& ((*pPos >= '0' && *pPos <= '9') 
					|| (*pPos >= 'a' && *pPos <='f')
					|| (*pPos >= 'A' && *pPos <= 'F')))
			{
			if (*pPos >= '0' && *pPos <= '9')
				dwInt = 16 * dwInt + (*pPos - '0');
			else if (*pPos >= 'A' && *pPos <= 'F')
				dwInt = 16 * dwInt + (10 + (*pPos - 'A'));
			else
				dwInt = 16 * dwInt + (10 + (*pPos - 'a'));

			pPos++;
			}

		iInt = (int)dwInt;
		}
	else
		{
		while (*pPos != '\0' && *pPos >= '0' && *pPos <= '9')
			{
			iInt = 10 * iInt + (*pPos - '0');
			pPos++;
			bFoundNumber = TRUE;
			}
		}

	//	Done?

	if (!bFoundNumber)
		{
		if (retbNullValue)
			*retbNullValue = TRUE;

		if (retpEnd)
			*retpEnd = pPos;

		return iNullResult;
		}

	//	Done!

	if (bNegative)
		iInt = -iInt;

	if (retpEnd)
		*retpEnd = pPos;

	return iInt;
	}

int strParseIntOfBase (char *pStart, int iBase, int iNullResult, char **retpEnd, BOOL *retbNullValue)

//	strParseIntOfBase
//
//	Parses an integer of the given base

	{
	char *pPos;
	BOOL bNegative;
	BOOL bFoundNumber;
	int iInt;

	//	Preset

	if (retbNullValue)
		*retbNullValue = FALSE;

	pPos = pStart;
	bNegative = FALSE;
	bFoundNumber = FALSE;
	iInt = 0;

	//	Skip whitespace

	while (*pPos == ' ' || *pPos == '\t' || *pPos == '\n' || *pPos == '\r')
		pPos++;

	//	If NULL, then we're done

	if (*pPos == '\0')
		{
		if (retbNullValue)
			*retbNullValue = TRUE;

		if (retpEnd)
			*retpEnd = pPos;

		return iNullResult;
		}

	//	If negative, remember it

	if (*pPos == '-')
		{
		bNegative = TRUE;
		pPos++;
		}

	//	See if this is a hex number

	if (*pPos == '0')
		{
		pPos++;
		bFoundNumber = TRUE;

		//	If the next character is x (or X) then we've got
		//	a Hex number

		if (*pPos == 'x' || *pPos == 'X')
			{
			pPos++;
			iBase = 16;
			}
		}

	//	Now parse for numbers

	DWORD dwInt = 0;

	while (*pPos != '\0' 
			&& ((*pPos >= '0' && *pPos <= '9') 
				|| (*pPos >= 'a' && *pPos <='f')
				|| (*pPos >= 'A' && *pPos <= 'F')))
		{
		if (*pPos >= '0' && *pPos <= '9')
			dwInt = (DWORD)iBase * dwInt + (*pPos - '0');
		else if (*pPos >= 'A' && *pPos <= 'F')
			dwInt = (DWORD)iBase * dwInt + (10 + (*pPos - 'A'));
		else
			dwInt = (DWORD)iBase * dwInt + (10 + (*pPos - 'a'));

		pPos++;
		bFoundNumber = TRUE;
		}

	iInt = (int)dwInt;

	//	Done?

	if (!bFoundNumber)
		{
		if (retbNullValue)
			*retbNullValue = TRUE;

		if (retpEnd)
			*retpEnd = pPos;

		return iNullResult;
		}

	//	Done!

	if (bNegative)
		iInt = -iInt;

	if (retpEnd)
		*retpEnd = pPos;

	return iInt;
	}

void strParseWhitespace (char *pPos, char **retpPos)

//	strParseWhitespace
//
//	Skips whitespace

	{
	while (strIsWhitespace(pPos))
		pPos++;

	if (retpPos)
		*retpPos = pPos;
	}

CString strRepeat (const CString &sString, int iCount)
	{
	ASSERT(iCount >= 0);

	int iLen = sString.GetLength();
	int iFinalCount = iCount * iLen;
	if (iFinalCount == 0)
		return NULL_STR;

	CString sResult;
	char *pDest = sResult.GetWritePointer(iFinalCount);
	char *pSource = sString.GetASCIIZPointer();

	for (int i = 0; i < iFinalCount; i++)
		*pDest++ = pSource[i % iLen];

	return sResult;
	}

CString RomanNumeralBase (int iNumber, char *szTens, char *szFives, char *szSingles)
	{
	char szBuffer[10];
	char *pPos = szBuffer;

	switch (iNumber)
		{
		case 1:
		case 2:
		case 3:
			{
			for (int i = 0; i < iNumber; i++)
				*pPos++ = *szSingles;

			return CString(szBuffer, iNumber);
			}

		case 4:
			{
			*pPos++ = *szSingles;
			*pPos++ = *szFives;
			return CString(szBuffer, 2);
			}

		case 5:
		case 6:
		case 7:
		case 8:
			{
			*pPos++ = *szFives;

			for (int i = 0; i < iNumber - 5; i++)
				*pPos++ = *szSingles;

			return CString(szBuffer, 1 + iNumber - 5);
			}

		case 9:
			{
			*pPos++ = *szSingles;
			*pPos++ = *szTens;
			return CString(szBuffer, 2);
			}

		default:
			return NULL_STR;
		}
	}

CString strRomanNumeral (int i)

//	strRomanNumeral
//
//	Returns the roman numeral

	{
	if (i < 0)
		return CONSTLIT("(negative)");
	else if (i == 0)
		return CONSTLIT("(zero)");
	else
		{
		CString sResult = RomanNumeralBase((i % 10000) / 1000, " ", " ", "M");
		sResult.Append(RomanNumeralBase((i % 1000) / 100, "M", "D", "C"));
		sResult.Append(RomanNumeralBase((i % 100) / 10, "C", "L", "X"));
		sResult.Append(RomanNumeralBase((i % 10), "X", "V", "I"));

		return sResult;
		}
	}

int strToInt (const CString &sString, int iFailResult, BOOL *retbFailed)

//	CStringToInt
//
//	Converts a string to an integer

	{
	return strParseInt(sString.GetASCIIZPointer(), iFailResult, NULL, retbFailed);
	}

CString strToUpper (const CString &sString)

//	strToUpper
//
//	Converts string to all upper-case

	{
	CString sUpper = sString;

	char *str = sUpper.GetWritePointer(sUpper.GetLength());
	int i = 0;
	int l = sUpper.GetLength();
	for (; i < l; i++)
		{
		*str = toupper(*str);
		}
	return sUpper;
	}

CString strTrimWhitespace (const CString &sString)

//	strTrimWhitespace
//
//	Removes leading and trailing whitespace

	{
	char *pPos = sString.GetASCIIZPointer();
	char *pStart;
	char *pEnd;

	strParseWhitespace(pPos, &pStart);

	pEnd = pPos + sString.GetLength();
	while (pEnd > pStart && strIsWhitespace(pEnd-1))
		pEnd--;

	return CString(pStart, pEnd - pStart);
	}

bool strIsAlphaNumeric(const char *str)
{
	for (int i = strlen(str) - 1; i != 0; i--) {
		if (!isalnum(str[i])) return false;
	}
	return true;
}

CString strSubString (const CString &sString, int iOffset, int iLength)

//	strSubString
//
//	Returns a substring of the given string

	{
	if (iOffset >= sString.GetLength())
		return LITERAL("");
	else
		{
		if (iLength == -1)
			iLength = sString.GetLength() - iOffset;
		else
			iLength = std::min(iLength, sString.GetLength() - iOffset);

		CString sSub(sString.GetPointer() + iOffset, iLength);
		return sSub;
		}
	}

CString strWord (const CString &sString, int iWordPos)

//	strWord
//
//	Returns the nth word (0-based) in the string. Words are
//	contiguous alphanumeric strings

	{
	enum States { stateNone, stateInWord };

	//	Find the beginning of the word

	int iState = stateNone;
	char *pStart = NULL;
	char *pPos = sString.GetASCIIZPointer();
	while (*pPos != '\0' && (iWordPos > 0 || iState != stateInWord))
		{
		if (iState == stateInWord)
			{
			if (!strIsAlphaNumeric(pPos))
				{
				iWordPos--;
				iState = stateNone;
				}

			pPos++;
			}
		else
			{
			if (strIsAlphaNumeric(pPos))
				iState = stateInWord;
			else
				pPos++;
			}
		}

	if (*pPos == '\0')
		return NULL_STR;

	//	Look for the end of the word

	pStart = pPos;
	while (*pPos != '\0' && strIsAlphaNumeric(pPos))
		pPos++;

	return CString(pStart, pPos - pStart);
	}

void *CString::INTGetStorage (const CString &sString)
	{
	if (sString.m_pStore)
		sString.m_pStore->iRefCount++;

	return sString.m_pStore;
	}

void CString::INTFreeStorage (void *pvStore)
	{
	PSTORESTRUCT pStore = (PSTORESTRUCT)pvStore;

	if (pStore && (--pStore->iRefCount) == 0)
		FreeStore(pStore);
	}

void CString::INTSetStorage (CString &sString, void *pvStore)
	{
	sString.DecRefCount();

	sString.m_pStore = (PSTORESTRUCT)pvStore;

	//	If we've got a storage, bump up the ref count

	if (sString.m_pStore)
		sString.m_pStore->iRefCount++;
	}

void CString::INTTakeStorage (void *pStore)
	{
	DecRefCount();
	m_pStore = (PSTORESTRUCT)pStore;
	}

#ifdef DEBUG_STRING_LEAKS

int CString::DebugGetStringCount (void)
	{
	return g_iStoreCount;
	}

void CString::DebugMark (void)
	{
	//	Iterate over the entire store and mark all currently
	//	allocated strings

	for (int i = 0; i < g_iStoreSize; i++)
		if (g_pStore[i].iRefCount > 0)
			g_pStore[i].iMark++;
	}

void CString::DebugOutputLeakedStrings (void)
	{
	//	All strings that have a mark == 0 are leaked

	int iLeaked = 0;
	for (int i = 0; i < g_iStoreSize; i++)
		if (g_pStore[i].iRefCount > 0)
			{
			if (g_pStore[i].iMark == 0)
				{
				char szBuffer[1024];
				snprintf(szBuffer, 1023, "'%s'\n", (g_pStore[i].pString ? g_pStore[i].pString : "(null)"));
				::OutputDebugString(szBuffer);
				iLeaked++;
				}
			else
				g_pStore[i].iMark--;
			}

	if (iLeaked > 0)
		{
		char szBuffer[1024];
		sprintf(szBuffer, "Leaked %d strings!\n", iLeaked);
		::OutputDebugString(szBuffer);
		}
	}

#endif
