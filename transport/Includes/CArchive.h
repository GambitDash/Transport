#ifndef INCL_CARCHIVE
#define INCL_CARCHIVE

#include "CStream.h"
#include "CObjectArray.h"
#include "CDictionary.h"
#include "CSymbolTable.h"

//	CArchive. This is an object that knows how to archive objects to a 
//	stream.
class CArchiver : public CObject
	{
	public:
		CArchiver (void);
		CArchiver (IWriteStream *pStream);
		virtual ~CArchiver (void);

		ALERROR AddExternalReference (CString sTag, void *pReference);
		ALERROR AddObject (CObject *pObject);
		ALERROR BeginArchive (void);
		ALERROR EndArchive (void);
		inline void SetVersion (DWORD dwVersion) { m_dwVersion = dwVersion; }

		//	These methods should only be called by objects
		//	that are being saved

		ALERROR Reference2ID (void *pReference, int *retiID);
		ALERROR SaveObject (CObject *pObject);
		ALERROR WriteData (char *pData, int iLength);

	private:
		IWriteStream *m_pStream;					//	Stream to save to
		CObjectArray m_List;						//	List of objects to save
		CDictionary m_ReferenceList;				//	Pointer references
		CSymbolTable m_ExternalReferences;			//	List of external references
		int m_iNextID;								//	Next ID to use for references
		DWORD m_dwVersion;							//	User-defined version
	};

//	CUnarchiver. This is an object that knows how to load objects from
//	a stream.

class CUnarchiver : public CObject
	{
	public:
		CUnarchiver (void);
		CUnarchiver (IReadStream *pStream);
		virtual ~CUnarchiver (void);

		ALERROR BeginUnarchive (void);
		ALERROR EndUnarchive (void);
		inline CObjectArray *GetList (void) { return &m_List; }
		CObject *GetObject (int iIndex);
		DWORD GetVersion (void) { return m_dwVersion; }
		ALERROR ResolveExternalReference (CString sTag, void *pReference);
		void SetMinVersion (DWORD dwVersion) { m_dwMinVersion = dwVersion; }

		//	These methods should only be called by objects
		//	that are being loaded

		ALERROR LoadObject (CObject **retpObject);
		ALERROR ReadData (char *pData, int iLength);
		ALERROR ResolveReference (int iID, void **pReference);

	private:
		IReadStream *m_pStream;
		CObjectArray m_List;
		CSymbolTable *m_pExternalReferences;
		CIntArray m_ReferenceList;
		CIntArray m_FixupTable;
		DWORD m_dwVersion;
		DWORD m_dwMinVersion;
	};

#endif

