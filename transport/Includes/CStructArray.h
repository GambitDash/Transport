#ifndef INCL_CSTRUCTARRAY
#define INCL_CSTRUCTARRAY

#include "CArray.h"

class CArchiver;
class CUnarchiver;

//	CStructArray. Implements a dynamic array of simple structures

class CStructArray : public CObject
	{
	public:
		CStructArray (void);
		CStructArray (int iElementSize, int iInitSize);

		inline ALERROR AppendStruct (void *pData, int *retiIndex = NULL) { return InsertStruct(pData, -1, retiIndex); }
		ALERROR ExpandArray (int iPos, int iCount);
		int GetCount (void) const;
		inline int GetElementSize (void) { return m_iElementSize; }
		void *GetStruct (int iIndex) const;
		ALERROR InsertStruct (void *pData, int iPos, int *retiIndex);
		void Remove (int iIndex);
		void RemoveAll (void);
		void SetStruct (int iIndex, void *pData);

	protected:
		virtual void CopyHandler (CObject *pOriginal);
		virtual ALERROR LoadHandler (CUnarchiver *pUnarchiver);
		virtual ALERROR SaveHandler (CArchiver *pArchiver);

	private:
		CINTDynamicArray m_Array;

		int m_iElementSize;
		int m_iInitSize;
	};

#endif

