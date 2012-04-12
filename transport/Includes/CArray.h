#ifndef INCL_CARRAY
#define INCL_CARRAY

//	CINTDynamicArray. Implementation of a dynamic array.
//	(NOTE: To save space, this class does not have a virtual
//	destructor. Do not sub-class this class without taking that into account).

class CINTDynamicArray
	{
	public:
		CINTDynamicArray (void);
		~CINTDynamicArray ();

		inline ALERROR Append (BYTE *pData, int iLength, int iAllocQuantum)
			{ return Insert(-1, pData, iLength, iAllocQuantum); }
		ALERROR Delete (int iOffset, int iLength);
		inline ALERROR DeleteAll (void) { return Delete(0, m_iLength); }
		inline int GetLength (void) const { return m_iLength; }
		inline void SetLength (int iLength) { m_iLength = iLength; }
		inline BYTE *GetPointer (int iOffset) const { return (m_pArray ? m_pArray + iOffset : NULL); }
		ALERROR Insert (int iOffset, BYTE *pData, int iLength, int iAllocQuantum);
		ALERROR Resize (int iNewSize, BOOL bPreserve, int iAllocQuantum);

	private:
		int m_iLength;							//	Length of the array in bytes
		int m_iAllocSize;						//	Allocated size of the array
		BYTE *m_pArray;							//	Array data
	};

//	Link list classes

template <class TYPE> class TListNode
	{
	public:
		TListNode (void) : m_pNext(NULL) { }
		virtual ~TListNode (void) { }

		int GetCount (void)
			{
			int iCount = 0;
			TListNode<TYPE> *pNext = m_pNext;
			while (pNext)
				{
				iCount++;
				pNext = pNext->m_pNext;
				}
			return iCount;
			}

		inline TYPE *GetNext (void) { return (TYPE *)m_pNext; }

		void Insert (TListNode<TYPE> *pNewNode)
			{
			pNewNode->m_pNext = m_pNext;
			m_pNext = pNewNode;
			}

		void Remove (TListNode<TYPE> *pNodeToRemove)
			{
			TListNode<TYPE> *pNext = m_pNext;
			while (pNext)
				{
				if (pNext == pNodeToRemove)
					{
					RemoveNext();
					break;
					}
				pNext = pNext->m_pNext;
				}
			}

	private:
		void RemoveNext (void)
			{
			TListNode<TYPE> *pDelete = m_pNext;
			m_pNext = m_pNext->m_pNext;
			delete pDelete;
			}

		TListNode<TYPE> *m_pNext;
	};

//	CIntArray. Implementation of a dynamic array of integers

class CIntArray : public CObject
	{
	public:
		CIntArray (void);
		CIntArray (const CIntArray &);
		virtual ~CIntArray (void);

		ALERROR AppendElement (int iElement, int *retiIndex = NULL);
		inline ALERROR CollapseArray (int iPos, int iCount) { return RemoveRange(iPos, iPos + iCount - 1); }
		ALERROR ExpandArray (int iPos, int iCount);
		int FindElement (int iElement) const;
		int GetCount (void) const;
		int GetElement (int iIndex) const;
		ALERROR InsertElement (int iElement, int iPos, int *retiIndex);
		ALERROR InsertRange (CIntArray *pList, int iStart, int iEnd, int iPos);
		ALERROR MoveRange (int iStart, int iEnd, int iPos);
		ALERROR Set (int iCount, int *pData);
		ALERROR RemoveAll (void);
		ALERROR RemoveElement (int iPos) { return RemoveRange(iPos, iPos); }
		ALERROR RemoveRange (int iStart, int iEnd);
		void ReplaceElement (int iPos, int iElement);
		void Shuffle (void);
		const CIntArray &operator= (const CIntArray &rhs)
			{
			CopyArray(rhs);
			return *this;
			}


	private:
		void CopyArray(const CIntArray &rhs)
			{
			m_iAllocSize = rhs.m_iAllocSize;
			if (m_pData != NULL)
				{
				MemFree(m_pData);
				}
			m_pData = (int *)MemAlloc(sizeof(int) * m_iAllocSize);
			m_iLength = rhs.m_iLength;
			memcpy(m_pData, rhs.m_pData, sizeof(int) * m_iAllocSize);
			}

		int m_iAllocSize;					//	Number of integers allocated
		int *m_pData;						//	Pointer to integer array
		int m_iLength;						//	Number of integers used
	};

#ifdef LATER
class CSet : public CObject
	{
	public:
		CSet (void);
		virtual ~CSet (void);

		virtual ALERROR AddElement (DWORD dwElement);
		virtual int GetCount (void);
		virtual ALERROR GetAllElement (CIntArray *retpArray);
		virtual BOOL IsMember (DWORD dwElement);
		virtual ALERROR RemoveElement (DWORD dwElement);
		virtual void RemoveAll (void);

	private:
		const int RUN_LENGTH = 128;

		class CRun
			{
			int iCount;
			DWORD Bits[RUN_LENGTH / sizeof(DWORD)];
			};

		CStructArray m_Runs;				//	Run array
		int m_iCount;						//	Number of elements
	};
#endif

#endif

