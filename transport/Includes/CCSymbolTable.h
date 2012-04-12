#ifndef INCL_CCSYMBOLTABLE
#define INCL_CCSYMBOLTABLE

#include "CSymbolTable.h"

//	This is a symbol table object

class CCSymbolTable : public ICCAtom
	{
	public:
		CCSymbolTable (void);

		//	ICCItem virtuals

		virtual ICCItem *Clone (CCodeChain *pCC);
		virtual ValueTypes GetValueType (void) { return Complex; }
		virtual BOOL IsIdentifier (void) { return FALSE; }
		virtual BOOL IsFunction (void) { return FALSE; }
		virtual BOOL IsLocalFrame (void) { return m_bLocalFrame; }
		virtual BOOL IsSymbolTable (void) { return TRUE; }
		virtual CString Print (CCodeChain *pCC);
		virtual void Reset (void);

		virtual void AddByOffset (CCodeChain *pCC, int iOffset, ICCItem *pEntry);
		virtual ICCItem *AddEntry (CCodeChain *pCC, ICCItem *pKey, ICCItem *pEntry);
		virtual int FindOffset (CCodeChain *pCC, ICCItem *pKey);
		virtual int FindValue (ICCItem *pValue);
		virtual ICCItem *GetParent (void) { return m_pParent; }
		virtual ICCItem *ListSymbols (CCodeChain *pCC);
		virtual ICCItem *Lookup (CCodeChain *pCC, ICCItem *pKey);
		virtual ICCItem *LookupByOffset (CCodeChain *pCC, int iOffset);
		virtual ICCItem *LookupEx (CCodeChain *pCC, ICCItem *pKey, BOOL *retbFound);
		virtual void SetLocalFrame (void) { m_bLocalFrame = TRUE; }
		virtual void SetParent (ICCItem *pParent) { m_pParent = pParent->Reference(); }
		virtual ICCItem *SimpleLookup (CCodeChain *pCC, ICCItem *pKey, BOOL *retbFound, int *retiOffset);

	protected:
		virtual void DestroyItem (CCodeChain *pCC);
		virtual ICCItem *StreamItem (CCodeChain *pCC, IWriteStream *pStream);
		virtual ICCItem *UnstreamItem (CCodeChain *pCC, IReadStream *pStream);

	private:
		CSymbolTable m_Symbols;
		ICCItem *m_pParent;
		BOOL m_bLocalFrame;
	};

#endif

