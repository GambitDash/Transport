//	CCPrimitive.cpp
//
//	Implements CCPrimitive class

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"

#include "CStream.h"
#include "CCCodeChain.h"

static CObjectClass<CCPrimitive>g_Class(OBJID_CCPRIMITIVE, NULL);

CCPrimitive::CCPrimitive (void) : ICCAtom(&g_Class)

//	CCInteger constructor

	{
	}

ICCItem *CCPrimitive::Clone (CCodeChain *pCC)

//	Clone
//
//	Clone this item

	{
	//	Better not happen
	ASSERT(FALSE);
	return NULL;
	}

void CCPrimitive::DestroyItem (CCodeChain *pCC)

//	DestroyItem
//
//	Destroy this item

	{
	pCC->DestroyPrimitive(this);
	}

ICCItem *CCPrimitive::Execute (CEvalContext *pCtx, ICCItem *pArgs)

//	Execute
//
//	Executes the function and returns a result

	{
	bool bCustomArgEval = ((m_dwFlags & PPFLAG_CUSTOM_ARG_EVAL) ? true : false);

	//	Evaluate args, if necessary

	ICCItem *pEvalArgs;
	if (!bCustomArgEval)
		{
		pEvalArgs = pCtx->pCC->EvaluateArgs(pCtx, pArgs, m_sArgPattern);
		if (pEvalArgs->IsError())
			return pEvalArgs;
		}
	else
		pEvalArgs = pArgs;

	//	Invoke the function

	ICCItem *pResult;
	try
		{
		pResult = m_pfFunction(pCtx, pEvalArgs, m_dwData);
		}
	catch (...)
		{
		pResult = pCtx->pCC->CreateError(strPatternSubst(CONSTLIT("Crash in function: %s"), m_sName.GetASCIIZPointer()), pEvalArgs);
		}

	//	Done

	if (!bCustomArgEval)
		pEvalArgs->Discard(pCtx->pCC);

	return pResult;
	}

CString CCPrimitive::Print (CCodeChain *pCC)

//	Print
//
//	Render as text

	{
	return strPatternSubst(LITERAL("[primitive: %s]"), m_sName.GetASCIIZPointer());
	}

void CCPrimitive::Reset (void)

//	Reset
//
//	Reset the internal variables

	{
	//	Better not happen
	ASSERT(FALSE);
	}

void CCPrimitive::SetProc (PRIMITIVEPROCDEF *pDef)

//	SetProc
//
//	Initializes primitive

	{
	m_sName = LITERAL(pDef->pszName);
	m_pfFunction = pDef->pfFunction;
	m_dwData = pDef->dwData;
	m_dwFlags = pDef->dwFlags;

	//	Description

	if (pDef->pszDescription && *(pDef->pszDescription))
		m_sDesc = LITERAL(pDef->pszDescription);

	//	Arg pattern

	if (pDef->pszArguments)
		m_sArgPattern = LITERAL(pDef->pszArguments);
	else
		m_dwFlags |= PPFLAG_CUSTOM_ARG_EVAL;
	}

ICCItem *CCPrimitive::StreamItem (CCodeChain *pCC, IWriteStream *pStream)

//	StreamItem
//
//	Stream the sub-class specific data

	{
	ICCItem *pName;
	ICCItem *pError;

	//	Save out the name of the function

	pName = pCC->CreateString(m_sName);
	if (pName->IsError())
		return pName;

	pError = pCC->StreamItem(pName, pStream);
	if (pError->IsError())
		return pError;

	pError->Discard(pCC);

	return pName;
	}

ICCItem *CCPrimitive::UnstreamItem (CCodeChain *pCC, IReadStream *pStream)

//	UnstreamItem
//
//	Unstream the sub-class specific data

	{
	ICCItem *pName;
	ICCItem *pFunction;

	//	Load the function name

	pName = pCC->UnstreamItem(pStream);
	if (pName->IsError())
		return pName;

	//	Look up the function name

	pFunction = pCC->LookupFunction(NULL, pName);
	pName->Discard(pCC);

	return pFunction;
	}
