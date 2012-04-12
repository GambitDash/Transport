#ifndef INCL_CCVECTOR
#define INCL_CCVECTOR

//	This is an array of 32-bit integers

class CCVector : public ICCList
	{
	public:
		CCVector (void);
		CCVector (CCodeChain *pCC);
		virtual ~CCVector (void);

		int *GetArray (void);
		BOOL SetElement (int iIndex, int iElement);
		ICCItem *SetSize (CCodeChain *pCC, int iNewSize);

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
		CCodeChain *m_pCC;						//	CodeChain
		int m_iCount;							//	Number of elements
		int *m_pData;							//	Array of elements
	};

#endif


