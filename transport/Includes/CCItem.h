//	CodeChain.h
//
//	CodeChain is a micro-LISP variant designed for high performance
//	and efficiency.
//
//	Requires Kernel.h
//
//	Basic Coding Rules
//
//	1.	Routines that return an ICCItem * will always increase the refcount
//		if you do not keep the item returned, you must call Discard on the
//		item.
//
//	2.	When passing in ICCItem *, the called routine is responsible for
//		incrementing the refcount if it decides to keep a reference

#ifndef INCL_CCITEM
#define INCL_CCITEM

#include "CString.h"

class CCodeChain;
class CEvalContext;
class ICCItem;
class IWriteStream;
class IReadStream;

//	Simple definitions

typedef ICCItem *(*PRIMITIVEPROC)(CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData);

#define PPFLAG_SIDEEFFECTS						0x00000001	//	Function has side-effects
#define PPFLAG_NOERRORS							0x00000002	//	Function never returns errors
#define PPFLAG_SYNONYM							0x00000004	//	Function is a synonym (pszDescription is name of function)
#define PPFLAG_CUSTOM_ARG_EVAL					0x00000008	//	Raw args are passed to the function (before evaluation)

typedef struct
	{
	const char *pszName;
	PRIMITIVEPROC pfFunction;
	DWORD dwData;
	const char *pszDescription;
	const char *pszArguments;
	DWORD dwFlags;
	} PRIMITIVEPROCDEF, *PPRIMITIVEPROCDEF;

//	Error Definitions. These are common result codes returned by basic
//	primitives.

#define CCRESULT_NOTFOUND							1		//	Used for symbol tables, files, etc.
#define CCRESULT_CANCEL								2		//	User cancelled operation
#define CCRESULT_DISKERROR							3		//	Out of disk space, etc.

//	A Cons is the primitive list glue.

class CCons
	{
	public:
		ICCItem *m_pItem;
		CCons *m_pNext;
	};

//	An item is a generic element of a list. This is the basic unit of
//	CodeChain.

class ICCItem : public CObject
	{
	public:
		enum ValueTypes
			{
			Boolean,
			Integer,
			String,
			List,
			Function,
			Complex,
			};

		ICCItem (IObjectClass *pClass);

		//	Increment and decrement ref counts

		virtual ICCItem *Clone (CCodeChain *pCC) = 0;
		virtual void Discard (CCodeChain *pCC);
		inline ICCItem *Reference (void) { m_dwRefCount++; return this; }
		virtual void Reset (void) = 0;
		inline void SetNoRefCount (void) { m_bNoRefCount = true; }

		//	List interface

		virtual ICCItem *Enum (CEvalContext *pCtx, ICCItem *pCode) = 0;
		virtual int GetCount (void) = 0;
		virtual ICCItem *GetElement (int iIndex) = 0;
		virtual ICCItem *Head (CCodeChain *pCC) = 0;
		inline BOOL IsList (void) { return IsNil() || !IsAtom(); }
		virtual ICCItem *Tail (CCodeChain *pCC) = 0;

		//	Evaluation

		inline BOOL IsQuoted (void) { return m_bQuoted; }
		inline void SetQuoted (void) { m_bQuoted = TRUE; }
		inline void ClearQuoted (void) { m_bQuoted = FALSE; }

		//	Errors

		inline BOOL IsError (void) { return m_bError; }
		inline void SetError (void) { m_bError = TRUE; }

		//	Load/save

		ICCItem *Stream (CCodeChain *pCC, IWriteStream *pStream);
		ICCItem *Unstream (CCodeChain *pCC, IReadStream *pStream);
		inline void ClearModified (void) { m_bModified = FALSE; }
		inline BOOL IsModified (void) { return m_bModified; }
		inline void SetModified (void) { m_bModified = TRUE; }

		//	Virtuals that must be overridden

		virtual ICCItem *Execute (CEvalContext *pCtx, ICCItem *pArgs);
		virtual BOOL GetBinding (int *retiFrame, int *retiOffset) { return FALSE; }
		virtual ICCItem *GetFunctionBinding (void) { return NULL; }
		virtual CString GetHelp (void) { return NULL_STR; }
		virtual int GetIntegerValue (void) { return 0; }
		virtual CString GetStringValue (void) { return LITERAL(""); }
		virtual ValueTypes GetValueType (void) = 0;
		virtual BOOL IsAtom (void) = 0;
		virtual BOOL IsAtomTable (void) { return FALSE; }
		virtual BOOL IsFunction (void) = 0;
		virtual BOOL IsIdentifier (void) = 0;
		virtual BOOL IsInteger (void) = 0;
		virtual BOOL IsNil (void) = 0;
		virtual BOOL IsSymbolTable (void) { return FALSE; }
		virtual CString Print (CCodeChain *pCC) = 0;
		virtual void SetBinding (int iFrame, int iOffset) { }
		virtual void SetFunctionBinding (CCodeChain *pCC, ICCItem *pBinding) { }

		//	Miscellaneous utility functions

		BOOL IsLambdaSymbol (void);
		void ResetItem (void);

		//	Symbol/Atom table functions

		virtual ICCItem *AddEntry (CCodeChain *pCC, ICCItem *pKey, ICCItem *pEntry) { return NotASymbolTable(pCC); }
		virtual void AddByOffset (CCodeChain *pCC, int iOffset, ICCItem *pEntry) { ASSERT(FALSE); }
		virtual int FindOffset (CCodeChain *pCC, ICCItem *pKey) { return -1; }
		virtual int FindValue (ICCItem *pValue) { return -1; }
		virtual ICCItem *GetParent (void) { return NULL; }
		virtual BOOL IsLocalFrame (void) { return FALSE; }
		virtual ICCItem *ListSymbols (CCodeChain *pCC) { return NotASymbolTable(pCC); }
		virtual ICCItem *Lookup (CCodeChain *pCC, ICCItem *pKey) { return NotASymbolTable(pCC); }
		virtual ICCItem *LookupByOffset (CCodeChain *pCC, int iOffset) { return NotASymbolTable(pCC); }
		virtual ICCItem *LookupEx (CCodeChain *pCC, ICCItem *pKey, BOOL *retbFound) { return NotASymbolTable(pCC); }
		virtual void SetParent (ICCItem *pParent) { ASSERT(FALSE); }
		virtual void SetLocalFrame (void) { ASSERT(FALSE); }
		virtual ICCItem *SimpleLookup (CCodeChain *pCC, ICCItem *pKey, BOOL *retbFound, int *retiOffset) { return NotASymbolTable(pCC); }

		//	Pool access

		ICCItem *GetNextFree() { return (ICCItem *)m_dwRefCount; }
		void SetNextFree (ICCItem *pNext) { m_dwRefCount = (DWORD)pNext; }

	protected:
		void CloneItem (ICCItem *pItem);

		virtual void DestroyItem (CCodeChain *pCC) { }
		virtual ICCItem *StreamItem (CCodeChain *pCC, IWriteStream *pStream) = 0;
		virtual ICCItem *UnstreamItem (CCodeChain *pCC, IReadStream *pStream) = 0;

		ICCItem *NotASymbolTable(CCodeChain *pCC);

		DWORD m_dwRefCount;						//	Number of references to this item

		DWORD m_bQuoted:1;						//	TRUE if quoted
		DWORD m_bError:1;						//	TRUE if it represents a runtime error
		DWORD m_bModified:1;					//	TRUE if this item was modified
		DWORD m_bNoRefCount:1;					//	TRUE if we don't care about ref count
	};


#endif

