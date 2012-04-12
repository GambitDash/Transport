#ifndef INCL_CCLAMBDA
#define INCL_CCLAMBDA

//	This is a lambda structure

class CCLambda : public ICCAtom
	{
	public:
		CCLambda (void);

		ICCItem *CreateFromList (CCodeChain *pCC, ICCItem *pList, BOOL bArgsOnly);

		//	ICCItem virtuals

		virtual ICCItem *Clone (CCodeChain *pCC);
		virtual ICCItem *Execute (CEvalContext *pCtx, ICCItem *pArgs);
		virtual CString GetStringValue (void) { return LITERAL("[lambda expression]"); }
		virtual ValueTypes GetValueType (void) { return Function; }
		virtual BOOL IsIdentifier (void) { return FALSE; }
		virtual BOOL IsFunction (void) { return TRUE; }
		virtual CString Print (CCodeChain *pCC) { return LITERAL("[lambda expression]"); }
		virtual void Reset (void);

	protected:
		virtual void DestroyItem (CCodeChain *pCC);
		virtual ICCItem *StreamItem (CCodeChain *pCC, IWriteStream *pStream);
		virtual ICCItem *UnstreamItem (CCodeChain *pCC, IReadStream *pStream);

	private:
		ICCItem *m_pArgList;
		ICCItem *m_pCode;
	};

#endif

