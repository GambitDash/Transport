#ifndef INCL_CCSTRING
#define INCL_CCSTRING

//	A string is an atom that represents a sequence of characters

class ICCString : public ICCAtom
	{
	public:
		ICCString (IObjectClass *pClass) : ICCAtom(pClass) { }

		//	ICCItem virtuals

		virtual ValueTypes GetValueType (void) { return String; }
		virtual BOOL IsIdentifier (void) { return TRUE; }
		virtual BOOL IsFunction (void) { return FALSE; }
	};

//	This is the standard implementation of a string

class CCString : public ICCString
	{
	public:
		CCString (void);

		inline CString GetValue (void) { return m_sValue; }
		inline void SetValue (CString sValue) { m_sValue = sValue; }

		//	ICCItem virtuals

		virtual ICCItem *Clone (CCodeChain *pCC);
		virtual BOOL GetBinding (int *retiFrame, int *retiOffset);
		virtual int GetIntegerValue (void) { return strToInt(m_sValue, 0); }
		virtual ICCItem *GetFunctionBinding (void) { if (m_pBinding) return m_pBinding->Reference(); else return NULL; }
		virtual CString GetStringValue (void) { return m_sValue; }
		virtual CString Print (CCodeChain *pCC);
		virtual void SetBinding (int iFrame, int iOffset);
		virtual void SetFunctionBinding (CCodeChain *pCC, ICCItem *pBinding);
		virtual void Reset (void);

	protected:
		virtual void DestroyItem (CCodeChain *pCC);
		virtual ICCItem *StreamItem (CCodeChain *pCC, IWriteStream *pStream);
		virtual ICCItem *UnstreamItem (CCodeChain *pCC, IReadStream *pStream);

	private:
		CString m_sValue;						//	Value of string
		int m_dwBinding;						//	Index into binding
		ICCItem *m_pBinding;					//	Function binding
	};

#endif

