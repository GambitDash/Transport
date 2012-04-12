#ifndef INCL_CCCODECHAIN
#define INCL_CCCODECHAIN

#include "CReadBlock.h"

#include "CCAtom.h"
#include "CCAtomTable.h"
#include "CCConsPool.h"
#include "CCInteger.h"
#include "CCItem.h"
#include "CCItemPool.h"
#include "CCLambda.h"
#include "CCList.h"
#include "CCMisc.h"
#include "CCNil.h"
#include "CCPrimitive.h"
#include "CCString.h"
#include "CCSymbolTable.h"
#include "CCTrue.h"
#include "CCVector.h"

//	This is the main CodeChain context

class CCodeChain : public CObject
	{
	public:
		CCodeChain (void);
		virtual ~CCodeChain (void);

		ALERROR Boot (void);

		//	Create/Destroy routines

		ICCItem *CreateAtomTable (void);
		ICCItem *CreateBool (bool bValue);
		inline CCons *CreateCons (void) { return m_ConsPool.CreateCons(); }
		ICCItem *CreateError (const CString &sError, ICCItem *pData);
		ICCItem *CreateErrorCode (int iErrorCode);
		ICCItem *CreateInteger (int iValue);
		ICCItem *CreateLambda (ICCItem *pList, BOOL bArgsOnly);
		ICCItem *CreateLinkedList (void);
		inline ICCItem *CreateMemoryError (void) { return m_sMemoryError.Reference(); }
		inline ICCItem *CreateNil (void) { return m_pNil->Reference(); }
		ICCItem *CreatePrimitive (PRIMITIVEPROCDEF *pDef);
		ICCItem *CreateString (const CString &sString);
		ICCItem *CreateSymbolTable (void);
		ICCItem *CreateSystemError (ALERROR error);
		inline ICCItem *CreateTrue (void) { return m_pTrue->Reference(); }
		ICCItem *CreateVector (int iSize);
		inline void DestroyAtomTable (ICCItem *pItem) { m_AtomTablePool.DestroyItem(this, pItem); }
		inline void DestroyCons (CCons *pCons) { m_ConsPool.DestroyCons(pCons); }
		inline void DestroyInteger (ICCItem *pItem) { m_IntegerPool.DestroyItem(this, pItem); }
		inline void DestroyLambda (ICCItem *pItem) { m_LambdaPool.DestroyItem(this, pItem); }
		inline void DestroyLinkedList (ICCItem *pItem) { m_ListPool.DestroyItem(this, pItem); }
		inline void DestroyPrimitive (ICCItem *pItem) { m_PrimitivePool.DestroyItem(this, pItem); }
		inline void DestroyString (ICCItem *pItem) { m_StringPool.DestroyItem(this, pItem); }
		inline void DestroySymbolTable (ICCItem *pItem) { m_SymbolTablePool.DestroyItem(this, pItem); }
		inline void DestroyVector (ICCItem *pItem) { delete pItem; }

		//	Load/save routines

		ICCItem *StreamItem (ICCItem *pItem, IWriteStream *pStream);
		ICCItem *UnstreamItem (IReadStream *pStream);

		//	Evaluation and parsing routines

		ICCItem *Apply (ICCItem *pFunc, ICCItem *pArgs, LPVOID pExternalCtx);
		inline ICCItem *GetNil (void) { return m_pNil; }
		inline ICCItem *GetTrue (void) { return m_pTrue; }
		ICCItem *Eval (CEvalContext *pEvalCtx, ICCItem *pItem);
		ICCItem *Link (const CString &sString, int iOffset, int *retiLinked);
      ICCItem *LoadApp (char *pszRes);
		ICCItem *LoadInitFile (const CString &sFilename);
		ICCItem *LookupGlobal (const CString &sGlobal, LPVOID pExternalCtx);
		ICCItem *TopLevel (ICCItem *pItem, LPVOID pExternalCtx);
		CString Unlink (ICCItem *pItem);

		//	Extensions

		ALERROR DefineGlobal (const CString &sVar, ICCItem *pValue);
		ALERROR DefineGlobalInteger (const CString &sVar, int iValue);
		ALERROR DefineGlobalString (const CString &sVar, const CString &sValue);
		ICCItem *EvaluateArgs (CEvalContext *pCtx, ICCItem *pArgs, const CString &sArgValidation);
		ICCItem *ListGlobals (void);
		ICCItem *LookupFunction (CEvalContext *pCtx, ICCItem *pName);
		ICCItem *PoolUsage (void);
		ALERROR RegisterPrimitive (PRIMITIVEPROCDEF *pDef);

		//	Miscellaneous

		bool HasIdentifier (ICCItem *pCode, const CString &sIdentifier);

	private:
		ICCItem *Lookup (CEvalContext *pCtx, ICCItem *pItem);
		ALERROR LoadDefinitions (IReadBlock *pBlock);
		char *SkipWhiteSpace (char *pPos);

		CCItemPool<CCInteger> m_IntegerPool;
		CCItemPool<CCString> m_StringPool;
		CCItemPool<CCLinkedList> m_ListPool;
		CCItemPool<CCPrimitive> m_PrimitivePool;
		CCItemPool<CCAtomTable> m_AtomTablePool;
		CCItemPool<CCSymbolTable> m_SymbolTablePool;
		CCItemPool<CCLambda> m_LambdaPool;

		CConsPool m_ConsPool;

		ICCItem *m_pNil;
		ICCItem *m_pTrue;
		CCString m_sMemoryError;

		ICCItem *m_pGlobalSymbols;
	};



#endif

