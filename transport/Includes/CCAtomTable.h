#ifndef INCL_CCATOMTABLE
#define INCL_CCATOMTABLE

#include "CDictionary.h"
#include "CCAtom.h"

class ICCItem;
class CCodeChain;
class IWriteStream;
class IReadStream;

//	This is an atom table object

class CCAtomTable : public ICCAtom
	{
	public:
		CCAtomTable (void);

		//	ICCItem virtuals

		virtual ICCItem *Clone (CCodeChain *pCC);
		virtual ValueTypes GetValueType (void) { return Complex; }
		virtual BOOL IsIdentifier (void) { return FALSE; }
		virtual BOOL IsFunction (void) { return FALSE; }
		virtual BOOL IsAtomTable (void) { return TRUE; }
		virtual CString Print (CCodeChain *pCC);
		virtual void Reset (void);

		virtual ICCItem *AddEntry (CCodeChain *pCC, ICCItem *pAtom, ICCItem *pEntry);
		virtual ICCItem *ListSymbols (CCodeChain *pCC);
		virtual ICCItem *Lookup (CCodeChain *pCC, ICCItem *pAtom);
		virtual ICCItem *LookupEx (CCodeChain *pCC, ICCItem *pAtom, BOOL *retbFound);

	protected:
		virtual void DestroyItem (CCodeChain *pCC);
		virtual ICCItem *StreamItem (CCodeChain *pCC, IWriteStream *pStream);
		virtual ICCItem *UnstreamItem (CCodeChain *pCC, IReadStream *pStream);

	private:
		CDictionary m_Table;
	};

#endif

