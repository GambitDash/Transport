#ifndef INCL_CCNIL
#define INCL_CCNIL

//	Nil class

class CCNil : public ICCAtom
	{
	public:
		CCNil (void);

		//	ICCItem virtuals

		virtual ICCItem *Clone (CCodeChain *pCC);
		virtual int GetCount (void) { return 0; }
		virtual int GetIntegerValue (void) { return 0; }
		virtual CString GetStringValue (void) { return LITERAL("Nil"); }
		virtual ValueTypes GetValueType (void) { return Boolean; }
		virtual BOOL IsIdentifier (void) { return FALSE; }
		virtual BOOL IsInteger (void) { return TRUE; }
		virtual BOOL IsFunction (void) { return FALSE; }
		virtual BOOL IsNil (void) { return TRUE; }
		virtual CString Print (CCodeChain *pCC) { return LITERAL("Nil"); }
		virtual void Reset (void) { }

	protected:
		virtual void DestroyItem (CCodeChain *pCC);
		virtual ICCItem *StreamItem (CCodeChain *pCC, IWriteStream *pStream);
		virtual ICCItem *UnstreamItem (CCodeChain *pCC, IReadStream *pStream);
	};


#endif

