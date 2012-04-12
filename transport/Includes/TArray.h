//	TArray.h
//
//	TArray Class

#ifndef INCL_TARRAY
#define INCL_TARRAY
#error Not used.
enum ESortOptions
	{
	DescendingSort = 1,
	AscendingSort = -1,
	};

//	Explicit placement operator
inline void *operator new (size_t, void *p) { return p; }

class CArrayBase
	{
	protected:
		CArrayBase (HANDLE hHeap, int iGranularity);
		~CArrayBase (void);

		void DeleteBytes (int iOffset, int iLength);
		void InsertBytes (int iOffset, void *pData, int iLength, int iAllocQuantum);
		ALERROR Resize (int iNewSize, bool bPreserve, int iAllocQuantum);

		HANDLE m_hHeap;
		int m_iSize;
		int m_iAllocSize;
		char *m_pArray;
		int m_iGranularity;
	};

#pragma warning(disable:4291)			//	No need for a delete because we're placing object
template <class VALUE> class TRawArray : public CArrayBase
	{
	public:
		TRawArray (void) : CArrayBase(NULL, 10), m_iExtraBytes(0) { }
		TRawArray (int iExtraBytes) : CArrayBase(NULL, 10), m_iExtraBytes(iExtraBytes) { }

		inline VALUE &operator [] (int iIndex) const { return GetAt(iIndex); }

		void Delete (int iIndex)
			{
			VALUE *pElement = (VALUE *)(m_pArray + iIndex * GetElementSize());
			ASSERT((char *)pElement - m_pArray < m_iSize);
			pElement->VALUE::~VALUE();
			DeleteBytes(iIndex * GetElementSize(), GetElementSize());
			}

		void DeleteAll (void)
			{
			VALUE *pElement = (VALUE *)m_pArray;
			for (int i = 0; i < GetCount(); i++, pElement++)
				pElement->VALUE::~VALUE();

			DeleteBytes(0, m_iSize);
			}

		inline VALUE &GetAt (int iIndex) const
			{
			VALUE *pElement = (VALUE *)(m_pArray + iIndex * GetElementSize());
			return *pElement;
			}

		inline int GetCount (void) const { return (m_iSize / GetElementSize()); }

		inline int GetElementSize (void) const { return sizeof(VALUE) + m_iExtraBytes; }

		inline int GetExtraBytes (void) const { return m_iExtraBytes; }

		void Insert (const VALUE &Value, int iIndex = -1)
			{
			int iOffset;
			if (iIndex == -1) iIndex = GetCount();
			iOffset = iIndex * GetElementSize();
			InsertBytes(iOffset, NULL, GetElementSize(), 10 * GetElementSize());

			VALUE *pElement = new(m_pArray + iOffset) VALUE(Value);
			}

		VALUE *InsertEmpty (int iCount = 1, int iIndex = -1)
			{
			int iOffset;
			if (iIndex == -1) iIndex = GetCount();
			iOffset = iIndex * GetElementSize();
			InsertBytes(iOffset, NULL, iCount * GetElementSize(), iCount * 10 * GetElementSize());

			for (int i = 0; i < iCount; i++)
				{
				VALUE *pElement = new(m_pArray + iOffset + (i * GetElementSize())) VALUE;
				}

			return &GetAt(iIndex);
			}

		void SetExtraBytes (int iExtraBytes)
			{
			ASSERT(GetCount() == 0);
			m_iExtraBytes = iExtraBytes;
			}

	private:
		int m_iExtraBytes;
	};

template <class VALUE> class TArray : public CArrayBase
	{
	public:
		TArray (HANDLE hHeap = NULL) : CArrayBase(NULL, 10) { }
		TArray (int iGranularity) : CArrayBase(NULL, iGranularity) { }
		TArray (const TArray<VALUE> &Obj) : CArrayBase(Obj.m_hHeap, 10)
			{
			m_iSize = 0;
			m_iAllocSize = 0;
			m_pArray = NULL;
			Resize(Obj.m_iAllocSize, false, sizeof(VALUE));
			for (int i = 0; i < Obj.GetCount(); i++)
				{
				VALUE *pElement = new(m_pArray + (i * sizeof(VALUE))) VALUE(Obj[i]);
				}
			m_iSize = Obj.m_iSize;
			}

		~TArray (void) { DeleteAll(); }

		TArray<VALUE> &operator= (const TArray<VALUE> &Obj)
			{
			DeleteAll();
			m_hHeap = Obj.m_hHeap;
			m_iSize = 0;
			Resize(Obj.m_iAllocSize, false, sizeof(VALUE));
			for (int i = 0; i < Obj.GetCount(); i++)
				{
				VALUE *pElement = new(m_pArray + (i * sizeof(VALUE))) VALUE(Obj[i]);
				}
			m_iSize = Obj.m_iSize;
			return *this;
			}

		inline VALUE &operator [] (int iIndex) const { return GetAt(iIndex); }

		void Delete (int iIndex)
			{
			VALUE *pElement = (VALUE *)(m_pArray + iIndex * sizeof(VALUE));
			ASSERT((char *)pElement - m_pArray < m_iSize);
			pElement->VALUE::~VALUE();
			DeleteBytes(iIndex * sizeof(VALUE), sizeof(VALUE));
			}

		void DeleteAll (void)
			{
			VALUE *pElement = (VALUE *)m_pArray;
			for (int i = 0; i < GetCount(); i++, pElement++)
				pElement->VALUE::~VALUE();

			DeleteBytes(0, m_iSize);
			}

		bool Find (const VALUE &ToFind, int *retiIndex = NULL) const
			{
			int iCount = GetCount();

			for (int i = 0; i < iCount; i++)
				if (GetAt(i) == ToFind)
					{
					if (retiIndex)
						*retiIndex = i;
					return true;
					}

			return false;
			}

		inline VALUE &GetAt (int iIndex) const
			{
			VALUE *pElement = (VALUE *)(m_pArray + iIndex * sizeof(VALUE));
			return *pElement;
			}

		inline int GetCount (void) const
			{
			return m_iSize / sizeof(VALUE);
			}

		void Insert (const VALUE &Value, int iIndex = -1)
			{
			int iOffset;
			if (iIndex == -1) iIndex = GetCount();
			iOffset = iIndex * sizeof(VALUE);
			InsertBytes(iOffset, NULL, sizeof(VALUE), m_iGranularity * sizeof(VALUE));

			VALUE *pElement = new(m_pArray + iOffset) VALUE(Value);
			}

		VALUE *Insert (void)
			{
			int iOffset = GetCount() * sizeof(VALUE);
			InsertBytes(iOffset, NULL, sizeof(VALUE), m_iGranularity * sizeof(VALUE));

			return new(m_pArray + iOffset) VALUE;
			}

		VALUE *InsertAt (int iIndex)
			{
			int iOffset;
			if (iIndex == -1) iIndex = GetCount();
			iOffset = iIndex * sizeof(VALUE);
			InsertBytes(iOffset, NULL, sizeof(VALUE), m_iGranularity * sizeof(VALUE));

			return new(m_pArray + iOffset) VALUE;
			}

		void InsertEmpty (int iCount = 1, int iIndex = -1)
			{
			int iOffset;
			if (iIndex == -1) iIndex = GetCount();
			iOffset = iIndex * sizeof(VALUE);
			InsertBytes(iOffset, NULL, iCount * sizeof(VALUE), m_iGranularity * sizeof(VALUE));

			for (int i = 0; i < iCount; i++)
				{
				VALUE *pElement = new(m_pArray + iOffset + (i * sizeof(VALUE))) VALUE;
				}
			}

		void Sort (ESortOptions Order = AscendingSort)
			{
			if (GetCount() < 2)
				return;

			TArray<int> Result;

			//	Binary sort the contents into an indexed array

			SortRange(Order, 0, GetCount() - 1, Result);

			//	Create a new sorted array

			TArray<VALUE> SortedArray;
			SortedArray.InsertEmpty(GetCount());
			for (int i = 0; i < GetCount(); i++)
				SortedArray[i] = GetAt(Result[i]);

			TakeHandoff(SortedArray);
			}

		void TakeHandoff (TArray<VALUE> &Obj)
			{
			DeleteAll();

			m_hHeap = Obj.m_hHeap;
			m_iSize = Obj.m_iSize;
			m_iAllocSize = Obj.m_iAllocSize;
			m_pArray = Obj.m_pArray;

			Obj.m_iSize = 0;
			Obj.m_iAllocSize = 0;
			Obj.m_pArray = NULL;
			}

	private:
		void SortRange (ESortOptions Order, int iLeft, int iRight, TArray<int> &Result)
			{
			if (iLeft == iRight)
				Result.Insert(iLeft);
			else if (iLeft + 1 == iRight)
				{
				int iCompare = Order * KeyCompare(GetAt(iLeft), GetAt(iRight));
				if (iCompare == 1)
					{
					Result.Insert(iLeft);
					Result.Insert(iRight);
					}
				else
					{
					Result.Insert(iRight);
					Result.Insert(iLeft);
					}
				}
			else
				{
				int iMid = iLeft + ((iRight - iLeft) / 2);

				TArray<int> Buffer1;
				TArray<int> Buffer2;

				SortRange(Order, iLeft, iMid, Buffer1);
				SortRange(Order, iMid+1, iRight, Buffer2);

				//	Merge

				int iPos1 = 0;
				int iPos2 = 0;
				bool bDone = false;
				while (!bDone)
					{
					if (iPos1 < Buffer1.GetCount() && iPos2 < Buffer2.GetCount())
						{
						int iCompare = Order * KeyCompare(GetAt(Buffer1[iPos1]), GetAt(Buffer2[iPos2]));
						if (iCompare == 1)
							Result.Insert(Buffer1[iPos1++]);
						else if (iCompare == -1)
							Result.Insert(Buffer2[iPos2++]);
						else
							{
							Result.Insert(Buffer1[iPos1++]);
							Result.Insert(Buffer2[iPos2++]);
							}
						}
					else if (iPos1 < Buffer1.GetCount())
						Result.Insert(Buffer1[iPos1++]);
					else if (iPos2 < Buffer2.GetCount())
						Result.Insert(Buffer2[iPos2++]);
					else
						bDone = true;
					}
				}
			}
	};
#pragma warning(default:4291)

//	Simple array classes

//class CStringArray : public TArray<CString> { };
//class CIntArray : public TArray<int> { };

#endif
