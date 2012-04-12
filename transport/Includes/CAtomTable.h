#ifndef INCL_CATOMTABLE
#define INCL_CATOMTABLE

#include "CSymbolTable.h"

//	CAtomTable. Implementation of a string hash table
class CAtomTable : public CObject
	{
	public:
		CAtomTable (void);
		CAtomTable (int iHashSize);
		virtual ~CAtomTable (void);

		ALERROR AppendAtom (const CString &sString, int *retiAtom);
		int Atomize (const CString &sString);

	private:
		CSymbolTable *Hash (const CString &sString);

		int m_iHashSize;
		int m_iNextAtom;
		CSymbolTable *m_pBackbone;
	};

#endif

