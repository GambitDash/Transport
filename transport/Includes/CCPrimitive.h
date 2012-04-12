#ifndef INCL_CCPRIMITIVE
#define INCL_CCPRIMITIVE

//	This is a primitive function definition

class CCPrimitive : public ICCAtom
	{
	public:
		CCPrimitive (void);

		void SetProc (PRIMITIVEPROCDEF *pDef);

		//	ICCItem virtuals

		virtual ICCItem *Clone (CCodeChain *pCC);
		virtual ICCItem *Execute (CEvalContext *pCtx, ICCItem *pArgs);
		virtual CString GetHelp (void) { return m_sDesc; }
		virtual CString GetStringValue (void) { return m_sName; }
		virtual ValueTypes GetValueType (void) { return Function; }
		virtual BOOL IsIdentifier (void) { return FALSE; }
		virtual BOOL IsFunction (void) { return TRUE; }
		virtual CString Print (CCodeChain *pCC);
		virtual void Reset (void);

	protected:
		virtual void DestroyItem (CCodeChain *pCC);
		virtual ICCItem *StreamItem (CCodeChain *pCC, IWriteStream *pStream);
		virtual ICCItem *UnstreamItem (CCodeChain *pCC, IReadStream *pStream);

	private:
		CString m_sName;
		PRIMITIVEPROC m_pfFunction;
		CString m_sArgPattern;
		CString m_sDesc;
		DWORD m_dwData;
		DWORD m_dwFlags;
	};

#endif

