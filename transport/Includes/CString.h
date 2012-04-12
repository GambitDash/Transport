#ifndef INCL_CSTRING
#define INCL_CSTRING

//	CString. Implementation of a standard string class

#define LITERAL(str)		((CString)(str))
#define CONSTLIT(str)		(CString(str, sizeof(str)-1, TRUE))
#define DELIMIT_TRIM_WHITESPACE					0x00000001
#define DELIMIT_ALLOW_BLANK_STRINGS				0x00000002

class IReadStream;
class IWriteStream;
class CStringArray;
class CArchiver;
class CUnarchiver;

class CString : public CObject
	{
	public:
		enum CapitalizeOptions
			{
			capFirstLetter,
			};

		CString (void);
		CString (const char *pString);
		CString (const char *pString, int iLength);
		CString (const char *pString, int iLength, BOOL bExternal);
		virtual ~CString (void);

		CString (const CString &pString);
		CString &operator= (const CString &pString);

		char *GetASCIIZPointer (void) const;
		ALERROR Append (const CString &sString);
		void Capitalize (CapitalizeOptions iOption);
		int GetLength (void) const;
		char *GetPointer (void) const;
		char *GetWritePointer (int iLength);
		inline BOOL IsBlank (void) const { return (GetLength() == 0); }
		void ReadFromStream (IReadStream *pStream);
		ALERROR Transcribe (const char *pString, int iLen);
		void Truncate (int iLength);
		void WriteToStream (IWriteStream *pStream) const;

		//	These are used internally only

		static void INTStringCleanUp (void);
		static ALERROR INTStringInit (void);

		//	These are used for custom string arrays

		static void *INTGetStorage (const CString &sString);
		static void INTFreeStorage (void *pStore);
		static void INTSetStorage (CString &sString, void *pStore);
		void INTTakeStorage (void *pStore);

		//	Debugging APIs

#ifdef DEBUG_STRING_LEAKS
		static int DebugGetStringCount (void);
		static void DebugMark (void);
		static void DebugOutputLeakedStrings (void);
#endif

	protected:
		virtual void CopyHandler (CObject *pOriginal);
		virtual ALERROR LoadHandler (CUnarchiver *pUnarchiver);
		virtual ALERROR SaveHandler (CArchiver *pArchiver);

	private:
		struct STORESTRUCT
			{
			int iRefCount;
			int iAllocSize;				//	If negative, this is a read-only external allocation
			int iLength;
			char *pString;
#ifdef DEBUG_STRING_LEAKS
			int iMark;
#endif
			};
		typedef struct STORESTRUCT *PSTORESTRUCT;

		static void AddToFreeList (PSTORESTRUCT pStore, int iSize);
		PSTORESTRUCT AllocStore (int iSize, BOOL bAllocString);
#ifdef INLINE_DECREF
		inline void DecRefCount (void)
			{
			if (m_pStore && (--m_pStore->iRefCount) == 0)
				FreeStore(m_pStore);
			}
#else
		void DecRefCount (void);
#endif

		static void FreeStore (PSTORESTRUCT pStore);
		inline void IncRefCount (void) { if (m_pStore) m_pStore->iRefCount++; }
		inline BOOL IsExternalStorage (void) { return (m_pStore->iAllocSize < 0 ? TRUE : FALSE); }
		BOOL Size (int iLength, BOOL bPreserveContents);

		static void InitLowerCaseAbsoluteTable (void);

		PSTORESTRUCT m_pStore;

		static PSTORESTRUCT g_pStore;
		static int g_iStoreSize;
		static PSTORESTRUCT g_pFreeStore;

	friend class CStringArray;
	};

// External globals
extern const CString NULL_STR;

// Function prototypes
CString strCat (const CString &sString1, const CString &sString2);
int strCompare (const CString &sString1, const CString &sString2);
int strCompareAbsolute (const CString &sString1, const CString &sString2);

ALERROR strDelimitEx (const CString &sString, char cDelim, DWORD dwFlags, int iMinParts, CStringArray *pStringList);

inline ALERROR strDelimit (const CString &sString, char cDelim, int iMinParts, CStringArray *pStringList)
	{ return strDelimitEx(sString, cDelim, 0, iMinParts, pStringList); }

CString strCapitalize (const CString &sString, int iOffset = 0);
CString strCapitalizeWords (const CString &sString);
CString strCEscapeCodes (const CString &sString);
bool strEquals (const CString &sString1, const CString &sString2);
int strFind (const CString &sString, const CString &sTarget);
CString strFromInt (int iInteger, BOOL bSigned = TRUE);
bool strIsAlphaNumeric (const char *pPos);
inline BOOL strIsWhitespace (const char *pPos) { return *pPos == ' ' || *pPos == '\t' || *pPos == '\n' || *pPos == '\r'; }
int strParseInt (char *pStart, int iNullResult, char **retpEnd, BOOL *retbNullValue);
int strParseIntOfBase (char *pStart, int iBase, int iNullResult, char **retpEnd, BOOL *retbNullValue);
void strParseWhitespace (char *pPos, char **retpPos);
CString strPattern (const CString &sPattern, LPVOID *pArgs);
CString strPatternSubst (CString sLine, ...);
CString strRepeat (const CString &sString, int iCount);
CString strRomanNumeral (int i);
CString strSubString (const CString &sString, int iOffset, int iLength);
int strToInt (const CString &sString, int iFailResult, BOOL *retbFailed = NULL);
CString strToUpper (const CString &sString);
CString strTrimWhitespace (const CString &sString);
CString strWord (const CString &sString, int iWordPos);

#endif

