#ifndef INCL_CCLIST
#define INCL_CCLIST

//	A list is a list of items

class ICCList : public ICCItem
	{
	public:
		ICCList (IObjectClass *pClass) : ICCItem(pClass) { }

		//	ICCItem virtuals

		virtual ValueTypes GetValueType (void) { return List; }
		virtual BOOL IsAtom (void) { return FALSE; }
		virtual BOOL IsFunction (void) { return FALSE; }
		virtual BOOL IsIdentifier (void) { return FALSE; }
		virtual BOOL IsInteger (void) { return FALSE; }
		virtual BOOL IsNil (void) { return (GetCount() == 0); }
	};

//	This is a linked-list implementation of a list

class CCLinkedList : public ICCList
	{
	public:
		CCLinkedList (void);
		virtual ~CCLinkedList (void);

		void Append (CCodeChain *pCC, ICCItem *pItem, ICCItem **retpError);
		void CreateIndex (void);
		void RemoveElement (CCodeChain *pCC, int iIndex);
		void ReplaceElement (CCodeChain *pCC, int iIndex, ICCItem *pNewItem);
		void Shuffle (CCodeChain *pCC);

		//	ICCItem virtuals

		virtual ICCItem *Clone (CCodeChain *pCC);
		virtual ICCItem *Enum (CEvalContext *pCtx, ICCItem *pCode);
		virtual int GetCount (void) { return m_iCount; }
		virtual ICCItem *GetElement (int iIndex);
		virtual ICCItem *Head (CCodeChain *pCC) { return GetElement(0); }
		virtual CString Print (CCodeChain *pCC);
		virtual ICCItem *Tail (CCodeChain *pCC);
		virtual void Reset (void);

	protected:
		virtual void DestroyItem (CCodeChain *pCC);
		virtual ICCItem *StreamItem (CCodeChain *pCC, IWriteStream *pStream);
		virtual ICCItem *UnstreamItem (CCodeChain *pCC, IReadStream *pStream);

	private:
		CCons *m_pFirst;						//	First element in the list
		CCons *m_pLast;							//	Last element in the list
		int m_iCount;							//	Number of elements

		CCons **m_pIndex;						//	GetElement array
	};

#endif

