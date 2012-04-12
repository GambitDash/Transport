#ifndef INCL_CIDTABLE
#define INCL_CIDTABLE

#include "CDictionary.h"

//	CIDTable. Implementation of a table that matches IDs with objects
class CIDTable : public CDictionary
	{
	public:
		CIDTable (void);
		CIDTable (BOOL bOwned, BOOL bNoReference);
		virtual ~CIDTable (void);

		inline ALERROR AddEntry (int iKey, CObject *pValue) { return CDictionary::AddEntry(iKey, (int)pValue); }
		int GetKey (int iEntry) const;
		CObject *GetValue (int iEntry) const;
		ALERROR Lookup (int iKey, CObject **retpValue) const;
		ALERROR LookupEx (int iKey, int *retiEntry) const;
		ALERROR RemoveAll (void);
		ALERROR RemoveEntry (int iKey, CObject **retpOldValue);
		ALERROR ReplaceEntry (int iKey, CObject *pValue, BOOL bAdd, CObject **retpOldValue);
		void SetValue (int iEntry, CObject *pValue, CObject **retpOldValue);

	protected:
		virtual int Compare (int iKey1, int iKey2) const;
		virtual void CopyHandler (CObject *pOriginal);
		virtual ALERROR LoadHandler (CUnarchiver *pUnarchiver);
		virtual ALERROR SaveHandler (CArchiver *pArchiver);

	private:
		BOOL m_bOwned;
		BOOL m_bNoReference;
	};


#endif

