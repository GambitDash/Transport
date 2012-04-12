#ifndef INCL_CSYMBOLTABLE
#define INCL_CSYMBOLTABLE

#include "CDictionary.h"

class CArchiver;
class CUnarchiver;

//	CSymbolTable. Implementation of a symbol table
class CSymbolTable : public CDictionary
	{
	public:
		CSymbolTable (void);
		CSymbolTable (BOOL bOwned, BOOL bNoReference);
		CSymbolTable (const CSymbolTable &rhs);
		virtual ~CSymbolTable (void);

		ALERROR AddEntry (const CString &sKey, CObject *pValue);
		CString GetKey (int iEntry) const;
		CObject *GetValue (int iEntry) const;
		ALERROR Lookup (const CString &sKey, CObject **retpValue) const;
		ALERROR LookupEx (const CString &sKey, int *retiEntry) const;
		ALERROR RemoveAll (void);
		ALERROR RemoveEntry (const CString &sKey, CObject **retpOldValue);
		ALERROR ReplaceEntry (const CString &sKey, CObject *pValue, BOOL bAdd, CObject **retpOldValue);
		void SetValue (int iEntry, CObject *pValue, CObject **retpOldValue);
		const CSymbolTable &operator=(const CSymbolTable &rhs)
			{
			m_bOwned = TRUE;
			m_Array = rhs.m_Array;
			m_bNoReference = rhs.m_bNoReference;
			CopyHandler(NULL);
			return *this;
			}

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

