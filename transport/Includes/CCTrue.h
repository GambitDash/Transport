#ifndef INCL_CCTRUE
#define INCL_CCTRUE

//	True class

class CCTrue : public ICCAtom
	{
	public:
		CCTrue (void);

		//	ICCItem virtuals

		virtual ICCItem *Clone (CCodeChain *pCC);
		virtual int GetIntegerValue (void) { return 1; }
		virtual CString GetStringValue (void) { return LITERAL("True"); }
		virtual ValueTypes GetValueType (void) { return Boolean; }
		virtual BOOL IsIdentifier (void) { return FALSE; }
		virtual BOOL IsFunction (void) { return FALSE; }
		virtual CString Print (CCodeChain *pCC) { return LITERAL("True"); }
		virtual void Reset (void) { }

	protected:
		virtual void DestroyItem (CCodeChain *pCC);
		virtual ICCItem *StreamItem (CCodeChain *pCC, IWriteStream *pStream);
		virtual ICCItem *UnstreamItem (CCodeChain *pCC, IReadStream *pStream);
	};

#endif

