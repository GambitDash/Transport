#ifndef INCL_COBJECTARRAY
#define INCL_COBJECTARRAY

class CArchiver;
class CUnarchiver;

#include "CArray.h"

//	CObjectArray. Implements a dynamic array of objects
class CObjectArray : public CObject
	{
	public:
		CObjectArray (void);
		CObjectArray (BOOL bOwned);
		CObjectArray (const CObjectArray &);
		virtual ~CObjectArray (void);

		ALERROR AppendObject (CObject *pObj, int *retiIndex = NULL)
				{ return m_Array.AppendElement((int)pObj, retiIndex); }
		CObject *DetachObject (int iIndex);
		int FindObject (CObject *pObj);
		int GetCount (void) { return m_Array.GetCount(); }
		inline CObject *GetObject (int iIndex)
				{ return (CObject *)m_Array.GetElement(iIndex); }
		ALERROR InsertObject (CObject *pObj, int iPos, int *retiIndex)
				{ return m_Array.InsertElement((int)pObj, iPos, retiIndex); }
		ALERROR Set (int iCount, CObject **pData)
				{ return m_Array.Set(iCount, (int *)pData); }
		void RemoveAll (void);
		void RemoveObject (int iPos);
		void ReplaceObject (int iPos, CObject *pObj, bool bDelete = true);

	protected:
		virtual void CopyHandler (CObject *pOriginal);
		virtual ALERROR LoadDoneHandler (void);
		virtual ALERROR LoadHandler (CUnarchiver *pUnarchiver);
		virtual ALERROR SaveHandler (CArchiver *pArchiver);

	private:
		BOOL m_bOwned;
		CIntArray m_Array;
	};

#endif

