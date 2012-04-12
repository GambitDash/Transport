#ifndef INCL_CCINTEGER
#define INCL_CCINTEGER

//	An integer is an atom that represents a natural number

class ICCInteger : public ICCAtom
	{
	public:
		ICCInteger (IObjectClass *pClass) : ICCAtom(pClass) { }

		//	ICCItem virtuals

		virtual ValueTypes GetValueType (void) { return Integer; }
		virtual BOOL IsIdentifier (void) { return FALSE; }
		virtual BOOL IsInteger (void) { return TRUE; }
		virtual BOOL IsFunction (void) { return FALSE; }
	};

//	This is a standard implementation of an integer

class CCInteger : public ICCInteger
	{
	public:
		CCInteger (void);

		inline int GetValue (void) { return m_iValue; }
		inline void SetValue (int iValue) { m_iValue = iValue; }

		//	ICCItem virtuals

		virtual ICCItem *Clone (CCodeChain *pCC);
		virtual int GetIntegerValue (void) { return m_iValue; }
		virtual CString GetStringValue (void) { return strFromInt(m_iValue, TRUE); }
		virtual CString Print (CCodeChain *pCC);
		virtual void Reset (void);

	protected:
		virtual void DestroyItem (CCodeChain *pCC);
		virtual ICCItem *StreamItem (CCodeChain *pCC, IWriteStream *pStream);
		virtual ICCItem *UnstreamItem (CCodeChain *pCC, IReadStream *pStream);

	private:
		int m_iValue;							//	Value of 32-bit integer
	};

#endif

