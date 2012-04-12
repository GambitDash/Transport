#ifndef INCL_CCATOM
#define INCL_CCATOM

class CCodeChain;
class IObjectClass;

#include "CCItem.h"

//	An atom is a single value

class ICCAtom : public ICCItem
	{
	public:
		ICCAtom (IObjectClass *pClass) : ICCItem(pClass) { }

		//	ICCItem virtuals

		virtual ICCItem *Enum (CEvalContext *pCtx, ICCItem *pCode);
		virtual ICCItem *GetElement (int iIndex) { return (iIndex == 0 ? Reference() : NULL); }
		virtual int GetCount (void) { return 1; }
		virtual ICCItem *Head (CCodeChain *pCC) { return Reference(); }
		virtual BOOL IsAtom (void) { return TRUE; }
		virtual BOOL IsInteger (void) { return FALSE; }
		virtual BOOL IsNil (void) { return FALSE; }
		virtual ICCItem *Tail (CCodeChain *pCC);
	};

#endif

