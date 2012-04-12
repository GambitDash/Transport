#ifndef INCL_CDICTIONARY
#define INCL_CDICTIONARY

#include "CArray.h"

//	CDictionary. Implementation of a dynamic array of entries
class CDictionary : public CObject
	{
	public:
		CDictionary (void);
		CDictionary (IObjectClass *pClass);
		CDictionary (const CDictionary &rhs) :
			CObject(rhs),
			m_Array(rhs.m_Array)
			{ }
		virtual ~CDictionary (void);

		ALERROR AddEntry (int iKey, int iValue);
		ALERROR Find (int iKey, int *retiValue) const;
		ALERROR FindEx (int iKey, int *retiEntry) const;
		ALERROR FindOrAdd (int iKey, int iValue, BOOL *retbFound, int *retiValue);
		inline int GetCount (void) const { return m_Array.GetCount() / 2; }
		void GetEntry (int iEntry, int *retiKey, int *retiValue) const;
		ALERROR ReplaceEntry (int iKey, int iValue, BOOL bAdd, BOOL *retbAdded, int *retiOldValue);
		ALERROR RemoveAll (void) { return m_Array.RemoveAll(); }
		ALERROR RemoveEntry (int iKey, int *retiOldValue);

	protected:
		virtual int Compare (int iKey1, int iKey2) const;
		ALERROR ExpandArray (int iPos, int iCount);
		void SetEntry (int iEntry, int iKey, int iValue);
		BOOL FindSlot (int iKey, int *retiPos) const;

		CIntArray m_Array;
	};

#endif

