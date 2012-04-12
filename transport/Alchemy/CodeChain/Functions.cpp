//	Functions.cpp
//
//	Implements primitive functions
//
//	Validation codes for EvaluateArgs:
//
//		f	function expression
//		i	Integer
//		k	A linked-list
//		l	List
//		q	Quoted identifier
//		s	String
//		u	Any item (not evaluated)
//		v	Any item (including errors)
//		x	Atom table
//		y	Symbol table
//		*	Any number of items (must be last)

#include "SDL.h"

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"
#include "CMath.h"

#include "CStream.h"
#include "CCCodeChain.h"

#include "Functions.h"

//	Forwards

int HelperCompareItems (ICCItem *pFirst, ICCItem *pSecond);

ICCItem *fnAppend (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnAppend
//
//	Appends two or more elements together

	{
	int i, j;
	CCodeChain *pCC = pCtx->pCC;

	//	Create a new list to store the result in

	ICCItem *pResult = pCC->CreateLinkedList();
	if (pResult->IsError())
		return pResult;

	CCLinkedList *pList = (CCLinkedList *)pResult;

	//	Loop over all arguments and add to result list

	for (i = 0; i < pArgs->GetCount(); i++)
		{
		ICCItem *pItem = pArgs->GetElement(i);

		for (j = 0; j < pItem->GetCount(); j++)
			pList->Append(pCC, pItem->GetElement(j), NULL);
		}

	//	Done

	return pResult;
	}

ICCItem *fnApply (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnApply
//
//	Applies the given parameter list to the lambda expression
//
//	(apply exp arg1 arg2 ... argn list)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;
	ICCItem *pFunction;
	ICCItem *pLast;
	CCLinkedList *pList;
	int i;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("v*"));
	if (pArgs->IsError())
		return pArgs;

	//	We better have at least two arguments

	if (pArgs->GetCount() < 2)
		{
		pArgs->Discard(pCC);
		return pCC->CreateError(CONSTLIT("apply needs a function and a list of arguments."), NULL);
		}

	//	The last argument better be a list

	pLast = pArgs->GetElement(pArgs->GetCount() - 1);
	if (!pLast->IsList())
		{
		pArgs->Discard(pCC);
		return pCC->CreateError(CONSTLIT("Last argument for apply must be a list."), NULL);
		}

	//	The first argument is the function

	pFunction = pArgs->Head(pCC);

	//	Create a new list to store the arguments in

	pResult = pCC->CreateLinkedList();
	if (pResult->IsError())
		{
		pArgs->Discard(pCC);
		return pResult;
		}

	pList = (CCLinkedList *)pResult;

	//	Add each of the arguments except the last

	for (i = 1; i < pArgs->GetCount() - 1; i++)
		{
		pList->Append(pCC, pArgs->GetElement(i), &pResult);
		if (pResult->IsError())
			{
			pList->Discard(pCC);
			pArgs->Discard(pCC);
			return pResult;
			}

		pResult->Discard(pCC);
		}

	//	Add each of the elements of the last list

	for (i = 0; i < pLast->GetCount(); i++)
		{
		pList->Append(pCC, pLast->GetElement(i), &pResult);
		if (pResult->IsError())
			{
			pList->Discard(pCC);
			pArgs->Discard(pCC);
			return pResult;
			}

		pResult->Discard(pCC);
		}

	//	Set the literal flag to indicate that the arguments should
	//	not be evaluated.

	pList->SetQuoted();

	//	Execute the function

	if (pFunction->IsFunction())
		pResult = pFunction->Execute(pCtx, pList);
	else
		pResult = pFunction->Reference();

	pList->Discard(pCC);

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnAtmCreate (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnAtmCreate
//
//	Creates a new atom table
//
//	(atmAtomTable ((atom1 entry1) (atom2 entry2) ... (atomn entryn))) -> atmtable

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pAtomTable;
	ICCItem *pList;
	int i;

	//	Evaluate the argument

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("l"));
	if (pArgs->IsError())
		return pArgs;

	//	Create the table

	pAtomTable = pCC->CreateAtomTable();
	if (pAtomTable->IsError())
		return pAtomTable;

	//	Add each entry

	pList = pArgs->Head(pCC);
	for (i = 0; i < pList->GetCount(); i++)
		{
		ICCItem *pPair = pList->GetElement(i);
		ICCItem *pResult;

		//	Make sure we have two elements

		if (pPair->GetCount() != 2)
			{
			pAtomTable->Discard(pCC);
			return pCC->CreateError(CONSTLIT("Invalid format for atom table entry:"), pPair);
			}

		//	Get the atom and the entry

		pResult = pAtomTable->AddEntry(pCC, pPair->GetElement(0), pPair->GetElement(1));
		if (pResult->IsError())
			{
			pAtomTable->Discard(pCC);
			return pResult;
			}

		pResult->Discard(pCC);
		}

	//	Done

	pArgs->Discard(pCC);
	return pAtomTable;
	}

ICCItem *fnAtmTable (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnAtmTable
//
//	Various atom table manipulations
//
//	(atmAddEntry symTable symbol entry) -> entry
//	(atmDeleteEntry symTable symbol) -> True
//	(atmLookup symTable symbol) -> entry

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pSymTable;
	ICCItem *pSymbol;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	if (dwData == FN_ATMTABLE_ADDENTRY)
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("xiv"));
	else if (dwData == FN_ATMTABLE_LIST)
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("x"));
	else
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("xi"));

	if (pArgs->IsError())
		return pArgs;

	//	Get the args

	pSymTable = pArgs->Head(pCC);

	//	Do the right thing

	switch (dwData)
		{
		case FN_ATMTABLE_ADDENTRY:
			{
			ICCItem *pEntry;

			pSymbol = pArgs->GetElement(1);
			pEntry = pArgs->GetElement(2);
			pResult = pSymTable->AddEntry(pCC, pSymbol, pEntry);

			//	If we succeeded, return the entry

			if (!pResult->IsError())
				{
				pResult->Discard(pCC);
				pResult = pEntry->Reference();
				}

			break;
			}

		case FN_ATMTABLE_DELETEENTRY:
			{
			pResult = pCC->CreateNil();
			break;
			}

		case FN_ATMTABLE_LIST:
			{
			pResult = pSymTable->ListSymbols(pCC);
			break;
			}

		case FN_ATMTABLE_LOOKUP:
			{
			pSymbol = pArgs->GetElement(1);
			pResult = pSymTable->Lookup(pCC, pSymbol);
			break;
			}

		default:
			ASSERT(FALSE);
			return NULL;
		}

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnBlock (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnBlock
//
//	Evaluates a list of expressions
//
//	(block (locals ...) exp1 exp2 ... expn)
//	(errblock (error locals ...) exp1 exp2 ... expn onerror)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pResult;
	ICCItem *pLocals;
	ICCItem *pExp;
	ICCItem *pLocalSymbols;
	ICCItem *pVar;
	ICCItem *pOldSymbols;
	int i;

	//	The first argument must be a list of locals

	pLocals = pArguments->Head(pCC);
	if (pLocals == NULL || !pLocals->IsList())
		return pCC->CreateError(CONSTLIT("Locals list expected:"), pLocals);

	//	If this is an error block then we must have at least one local

	if (dwData == FN_BLOCK_ERRBLOCK && pLocals->GetCount() == 0)
		return pCC->CreateError(CONSTLIT("errblock must have an 'error' local variable"), NULL);

	//	Now loop over the remaining arguments, evaluating each in turn

	pExp = pArguments->GetElement(1);

	//	If there are no expressions, then we just return Nil

	if (pExp == NULL)
		return pCC->CreateNil();

	//	Setup the locals. We start by creating a local symbol table

	pLocalSymbols = pCC->CreateSymbolTable();
	if (pLocalSymbols->IsError())
		return pLocalSymbols;

	pLocalSymbols->SetLocalFrame();

	//	Loop over each item and associate it

	for (i = 0; i < pLocals->GetCount(); i++)
		{
		ICCItem *pItem;

		pVar = pLocals->GetElement(i);

		pItem = pLocalSymbols->AddEntry(pCC, pVar, pCC->CreateNil());
		if (pItem->IsError())
			{
			pLocalSymbols->Discard(pCC);
			return pItem;
			}

		pItem->Discard(pCC);
		}

	//	Setup the context

	if (pCtx->pLocalSymbols)
		pLocalSymbols->SetParent(pCtx->pLocalSymbols);
	else
		pLocalSymbols->SetParent(pCtx->pLexicalSymbols);
	pOldSymbols = pCtx->pLocalSymbols;
	pCtx->pLocalSymbols = pLocalSymbols;

	//	Start with a default result

	pResult = pCC->CreateNil();

	//	Loop (starting with the second arg)

	for (i = 1; i < pArguments->GetCount(); i++)
		{
		pExp = pArguments->GetElement(i);

		//	If this is an error block and this is the last expression,
		//	then it must be error condition and we don't want to 
		//	execute it.

		if (i+1 == pArguments->GetCount() && dwData == FN_BLOCK_ERRBLOCK)
			break;

		//	Evaluate the expression

		pResult->Discard(pCC);
		pResult = pCC->Eval(pCtx, pExp);

		//	If we got an error, handle it

		if (pResult->IsError())
			{
			//	If this is an error block, then find the last expression
			//	and evaluate it.

			if (dwData == FN_BLOCK_ERRBLOCK)
				{
				ICCItem *pItem;

				//	Set the first local variable to be the error result

				pVar = pLocals->Head(pCC);
				pItem = pLocalSymbols->AddEntry(pCC, pVar, pResult);

				pItem->Discard(pCC);
				pResult->Discard(pCC);

				//	Find the last expression

				pExp = pArguments->GetElement(pArguments->GetCount() - 1);

				//	Evaluate it

				pResult = pCC->Eval(pCtx, pExp);
				}

			//	Regardless, leave the block and return the result

			break;
			}
		}

	//	Clean up

	pCtx->pLocalSymbols = pOldSymbols;
	pLocalSymbols->Discard(pCC);

	//	Done

	return pResult;
	}

ICCItem *fnCat (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnCat
//
//	Concatenates the given strings
//
//	(cat string1 string2 ... stringn)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;
	CString sResult;
	int i;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("*"));
	if (pArgs->IsError())
		return pArgs;

	//	Append each of the items

	for (i = 0; i < pArgs->GetCount(); i++)
		sResult.Append(pArgs->GetElement(i)->GetStringValue());

	pResult = pCC->CreateString(sResult);

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnCount (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnCount
//
//	Returns the number of elements in the list
//
//	(count list)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;
	ICCItem *pList;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("l"));
	if (pArgs->IsError())
		return pArgs;

	//	The first argument is the list

	pList = pArgs->Head(pCC);
	pResult = pCC->CreateInteger(pList->GetCount());

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnEnum (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnEnum
//
//	Enumerates the elements of a list
//
//	(enum list item-var exp)
//	(enumwhile list condition item-var exp)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;
	ICCItem *pVar;
	ICCItem *pBody;
	ICCItem *pList;
	ICCItem *pCondition;
	ICCItem *pLocalSymbols;
	ICCItem *pOldSymbols;
	ICCItem *pError;
	int i, iVarOffset;

	//	Evaluate the arguments and validate them

	if (dwData == FN_ENUM_WHILE)
		{
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("vuqu"));
		if (pArgs->IsError())
			return pArgs;

		pList = pArgs->GetElement(0);
		pCondition = pArgs->GetElement(1);
		pVar = pArgs->GetElement(2);
		pBody = pArgs->GetElement(3);
		}
	else
		{
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("vqu"));
		if (pArgs->IsError())
			return pArgs;

		pList = pArgs->GetElement(0);
		pCondition = NULL;
		pVar = pArgs->GetElement(1);
		pBody = pArgs->GetElement(2);
		}

	//	Setup the locals. We start by creating a local symbol table

	pLocalSymbols = pCC->CreateSymbolTable();
	if (pLocalSymbols->IsError())
		{
		pArgs->Discard(pCC);
		return pLocalSymbols;
		}

	//	Associate the enumaration variable

	pError = pLocalSymbols->AddEntry(pCC, pVar, pCC->CreateNil());
	if (pError->IsError())
		{
		pArgs->Discard(pCC);
		return pError;
		}

	pError->Discard(pCC);

	//	Setup the context

	if (pCtx->pLocalSymbols)
		pLocalSymbols->SetParent(pCtx->pLocalSymbols);
	else
		pLocalSymbols->SetParent(pCtx->pLexicalSymbols);
	pOldSymbols = pCtx->pLocalSymbols;
	pCtx->pLocalSymbols = pLocalSymbols;

	//	Start with a default result

	pResult = pCC->CreateNil();

	//	Get the offset of the variable so we don't have to
	//	search for it all the time

	iVarOffset = pLocalSymbols->FindOffset(pCC, pVar);

	//	Loop over all elements of the list

	for (i = 0; i < pList->GetCount(); i++)
		{
		//	Check the condition

		if (pCondition)
			{
			ICCItem *pEval;

			//	Evaluate the condition

			pEval = pCC->Eval(pCtx, pCondition);
			if (pEval->IsError())
				{
				pResult->Discard(pCC);
				pResult = pEval;
				break;
				}

			//	If the condition is Nil, then we're done

			if (pEval->IsNil())
				{
				pEval->Discard(pCC);
				break;
				}
			}

		ICCItem *pItem = pList->GetElement(i);

		//	Clean up the previous result

		pResult->Discard(pCC);

		//	Set the element

		pLocalSymbols->AddByOffset(pCC, iVarOffset, pItem);

		//	Eval

		pResult = pCC->Eval(pCtx, pBody);
		if (pResult->IsError())
			break;
		}

	//	Clean up

	pCtx->pLocalSymbols = pOldSymbols;
	pLocalSymbols->Discard(pCC);

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnEquality (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnEquality
//
//	Equality and inequality
//
//	(eq exp1 exp2 ... expn)
//	(> exp1 exp2 ... expn)
//	(>= exp1 exp2 ... expn)
//	(< exp1 exp2 ... expn)
//	(<= exp1 exp2 ... expn)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pResult;
	ICCItem *pExp;
	ICCItem *pPrev = NULL;
	ICCItem *pArgs;
	int i;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("*"));
	if (pArgs->IsError())
		return pArgs;

	//	Loop over all arguments

	for (i = 0; i < pArgs->GetCount(); i++)
		{
		pExp = pArgs->GetElement(i);
		if (pExp->IsError())
			{
			pExp->Reference();
			pArgs->Discard(pCC);
			return pExp;
			}

		if (pPrev)
			{
			int iResult = HelperCompareItems(pPrev, pExp);
			BOOL bOk;

			switch (dwData)
				{
				case FN_EQUALITY_EQ:
					{
					bOk = (iResult == 0);
					break;
					}

				case FN_EQUALITY_LESSER:
					{
					bOk = (iResult < 0);
					break;
					}

				case FN_EQUALITY_LESSER_EQ:
					{
					bOk = (iResult <= 0);
					break;
					}

				case FN_EQUALITY_GREATER:
					{
					bOk = (iResult > 0);
					break;
					}

				case FN_EQUALITY_GREATER_EQ:
					{
					bOk = (iResult >= 0);
					break;
					}

				default:
					ASSERT(FALSE);
				}

			//	If we don't have a match, return

			if (!bOk)
				{
				pArgs->Discard(pCC);
				return pCC->CreateNil();
				}
			}

		//	Remember the previous element so that we can compare

		pPrev = pExp;
		}

	//	If we get here, then all items are ok

	pArgs->Discard(pCC);
	pResult = pCC->CreateTrue();

	//	Done

	return pResult;
	}

ICCItem *fnEval (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnEval
//
//	Evaluates an expression
//
//	(eval exp)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("v"));
	if (pArgs->IsError())
		return pArgs;

	pResult = pCC->Eval(pCtx, pArgs->GetElement(0));

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnFilter (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnFilter
//
//	Filters a list based on a boolean expression.
//
//	(filter list var exp) -> list

	{
	int i;
	CCodeChain *pCC = pCtx->pCC;

	if (pArgs->GetElement(0)->IsNil())
		return pCC->CreateNil();

	//	Args

	ICCItem *pSource = pArgs->GetElement(0);
	ICCItem *pVar = pArgs->GetElement(1);
	ICCItem *pBody = pArgs->GetElement(2);

	//	Create a destination list

	ICCItem *pResult = pCC->CreateLinkedList();
	if (pResult->IsError())
		return pResult;

	CCLinkedList *pList = (CCLinkedList *)pResult;

	//	Setup the locals. We start by creating a local symbol table

	ICCItem *pLocalSymbols = pCC->CreateSymbolTable();
	if (pLocalSymbols->IsError())
		{
		pResult->Discard(pCC);
		return pLocalSymbols;
		}

	//	Associate the enumaration variable

	ICCItem *pError = pLocalSymbols->AddEntry(pCC, pVar, pCC->CreateNil());
	if (pError->IsError())
		{
		pLocalSymbols->Discard(pCC);
		pResult->Discard(pCC);
		return pError;
		}

	pError->Discard(pCC);

	//	Setup the context

	if (pCtx->pLocalSymbols)
		pLocalSymbols->SetParent(pCtx->pLocalSymbols);
	else
		pLocalSymbols->SetParent(pCtx->pLexicalSymbols);
	ICCItem *pOldSymbols = pCtx->pLocalSymbols;
	pCtx->pLocalSymbols = pLocalSymbols;

	//	Get the offset of the variable so we don't have to
	//	search for it all the time

	int iVarOffset = pLocalSymbols->FindOffset(pCC, pVar);

	//	Loop over all elements of the list

	for (i = 0; i < pSource->GetCount(); i++)
		{
		ICCItem *pItem = pSource->GetElement(i);

		//	Set the element

		pLocalSymbols->AddByOffset(pCC, iVarOffset, pItem);

		//	Eval

		ICCItem *pSelect = pCC->Eval(pCtx, pBody);
		if (pSelect->IsError())
			{
			pResult->Discard(pCC);
			pResult = pSelect;
			break;
			}

		//	If the evaluation is not Nil, then we include the
		//	item in the result

		if (!pSelect->IsNil())
			pList->Append(pCC, pItem, NULL);

		pSelect->Discard(pCC);
		}

	//	Clean up

	pCtx->pLocalSymbols = pOldSymbols;
	pLocalSymbols->Discard(pCC);

	//	Done

	if (pResult->GetCount() > 0)
		return pResult;
	else
		{
		pResult->Discard(pCC);
		return pCC->CreateNil();
		}
	}

ICCItem *fnFind (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnFind
//
//	Finds a target in a source

	{
	int i;
	CCodeChain *pCC = pCtx->pCC;

	//	Get the source and target

	ICCItem *pSource = pArgs->GetElement(0);
	ICCItem *pTarget = pArgs->GetElement(1);

	//	If this is a list, then look for the target in the list and
	//	return the item position in the list

	int iPos;
	if (pSource->IsList())
		{
		iPos = -1;
		for (i = 0; i < pSource->GetCount(); i++)
			{
			ICCItem *pItem = pSource->GetElement(i);
			if (HelperCompareItems(pItem, pTarget) == 0)
				{
				iPos = i;
				break;
				}
			}
		}

	//	Otherwise, look for the target string in the source string

	else
		{
		iPos = strFind(pSource->GetStringValue(), pTarget->GetStringValue());
		}

	//	Done

	if (iPos == -1)
		return pCC->CreateNil();
	else
		return pCC->CreateInteger(iPos);
	}

ICCItem *fnForLoop (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnForLoop
//
//	Evaluates an expression for a given number of iterations
//	Iterates from "from" to "to" inclusive.
//
//	(for var from to exp)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;
	ICCItem *pVar;
	ICCItem *pBody;
	ICCItem *pLocalSymbols;
	ICCItem *pOldSymbols;
	ICCItem *pError;
	int i, iFrom, iTo, iVarOffset;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("qiiu"));
	if (pArgs->IsError())
		return pArgs;

	pVar = pArgs->GetElement(0);
	iFrom = pArgs->GetElement(1)->GetIntegerValue();
	iTo = pArgs->GetElement(2)->GetIntegerValue();
	pBody = pArgs->GetElement(3);

	//	Setup the locals. We start by creating a local symbol table

	pLocalSymbols = pCC->CreateSymbolTable();
	if (pLocalSymbols->IsError())
		{
		pArgs->Discard(pCC);
		return pLocalSymbols;
		}

	//	Associate the enumaration variable

	pError = pLocalSymbols->AddEntry(pCC, pVar, pCC->CreateNil());
	if (pError->IsError())
		{
		pArgs->Discard(pCC);
		return pError;
		}

	pError->Discard(pCC);

	//	Setup the context

	if (pCtx->pLocalSymbols)
		pLocalSymbols->SetParent(pCtx->pLocalSymbols);
	else
		pLocalSymbols->SetParent(pCtx->pLexicalSymbols);
	pOldSymbols = pCtx->pLocalSymbols;
	pCtx->pLocalSymbols = pLocalSymbols;

	//	Start with a default result

	pResult = pCC->CreateNil();

	//	Get the offset of the variable so we don't have to
	//	search for it all the time

	iVarOffset = pLocalSymbols->FindOffset(pCC, pVar);

	//	Loop over all elements of the list

	for (i = iFrom; i <= iTo; i++)
		{
		ICCItem *pItem = pCC->CreateInteger(i);

		//	Clean up the previous result

		pResult->Discard(pCC);

		//	Set the element

		pLocalSymbols->AddByOffset(pCC, iVarOffset, pItem);
		pItem->Discard(pCC);

		//	Eval

		pResult = pCC->Eval(pCtx, pBody);
		if (pResult->IsError())
			break;
		}

	//	Clean up

	pCtx->pLocalSymbols = pOldSymbols;
	pLocalSymbols->Discard(pCC);

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnIf (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnIf
//
//	If control function
//
//	(if exp then else)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pTest;
	ICCItem *pThen;
	ICCItem *pElse;
	ICCItem *pResult;

	//	Get the arguments

	pTest = pArguments->GetElement(0);
	if (pTest == NULL)
		return pCC->CreateError(CONSTLIT("Conditional expression expected"), pCC->CreateNil());

	pThen = pArguments->GetElement(1);
	if (pThen == NULL)
		return pCC->CreateError(CONSTLIT("Then expression expected"), pCC->CreateNil());

	pElse = pArguments->GetElement(2);

	//	Evaluate the test expression

	pResult = pCC->Eval(pCtx, pTest);
	if (pResult->IsError())
		return pResult;

	//	If the result is not true, evaluate the else expression

	if (pResult->IsNil())
		{
		if (pElse)
			{
			pResult->Discard(pCC);
			return pCC->Eval(pCtx, pElse);
			}
		else
			return pResult;
		}

	//	Otherwise, evaluate the then expression

	pResult->Discard(pCC);
	return pCC->Eval(pCtx, pThen);
	}

ICCItem *fnItem (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnItem
//
//	Returns nth entry in a list (0-based)
//
//	(item list nth) -> item

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;
	ICCItem *pList;
	int iIndex;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("li"));
	if (pArgs->IsError())
		return pArgs;

	//	The first argument is the list

	pList = pArgs->Head(pCC);
	iIndex = pArgs->GetElement(1)->GetIntegerValue();
	pResult = pList->GetElement(iIndex);
	if (pResult == NULL)
		pResult = pCC->CreateNil();
	else
		pResult = pResult->Reference();

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnItemInfo (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnItemInfo
//
//	Returns info about a single item
//
//	(isatom item) -> True/Nil
//	(iserror item) -> True/Nil
//	(isfunction item) -> True/Nil

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("v"));
	if (pArgs->IsError())
		return pArgs;

	//	Do the right thing

	switch (dwData)
		{
		case FN_ITEMINFO_ISERROR:
			{
			if (pArgs->Head(pCC)->IsError())
				pResult = pCC->CreateTrue();
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_ITEMINFO_ISATOM:
			{
			if (pArgs->Head(pCC)->IsAtom())
				pResult = pCC->CreateTrue();
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_ITEMINFO_ISINT:
			{
			pResult = pCC->CreateBool(pArgs->Head(pCC)->IsInteger() ? true : false);
			break;
			}

		case FN_ITEMINFO_ISFUNCTION:
			{
			if (pArgs->Head(pCC)->IsFunction())
				pResult = pCC->CreateTrue();
			else
				pResult = pCC->CreateNil();
			break;
			}

		case FN_ITEMINFO_ASINT:
			{
			pResult = pCC->CreateInteger(pArgs->Head(pCC)->GetIntegerValue());
			break;
			}

		case FN_ITEMINFO_HELP:
			{
			CString sHelp = pArgs->GetElement(0)->GetHelp();
			if (sHelp.IsBlank())
				pResult = pCC->CreateNil();
			else
				pResult = pCC->CreateString(sHelp);
			break;
			}

		default:
			ASSERT(FALSE);
			return NULL;
		}

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnLambda (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnLambda
//
//	Lambda expression
//
//	(lambda (args...) code)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pItem;
	CCLambda *pLambda;

	//	Create a lambda expression based on this list

	pItem = pCC->CreateLambda(pArguments, TRUE);
	if (pItem->IsError())
		return pItem;

	pLambda = dynamic_cast<CCLambda *>(pItem);

	//	Done

	return pLambda;
	}

ICCItem *fnLink (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnLink
//
//	Converts a string into an expression

	{
	CCodeChain *pCC = pCtx->pCC;
	return pCC->Link(pArgs->GetElement(0)->GetStringValue(), 0, NULL);
	}

ICCItem *fnLinkedList (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnLinkedList
//
//	Handles linked-list specific functions
//
//	(lnkAppend linked-list item) -> list
//	(lnkRemove linked-list index) -> list
//	(lnkRemoveNil linked-list) -> list
//	(lnkReplace linked-list index item) -> list
//
//	HACK: This function has different behavior depending on the first
//	argument. If the first argument is a variable holding a linked list,
//	then the variable contents will be changed. If the variable holds Nil,
//	then the variable contents are not changed. In all cases, the caller
//	should structure the call as: (setq ListVar (lnkAppend ListVar ...))
//	in order to handle all cases.

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pList;
	CCLinkedList *pLinkedList;
	ICCItem *pResult;

	//	Evaluate the arguments

	if (dwData == FN_LINKEDLIST_APPEND)
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("lv"));
	else if (dwData == FN_LINKEDLIST_REMOVE_NIL)
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("l"));
	else
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("liv"));
	if (pArgs->IsError())
		return pArgs;

	//	Get the linked list

	pList = pArgs->GetElement(0);
	if (pList->GetClass()->GetObjID() == OBJID_CCLINKEDLIST)
		pLinkedList = (CCLinkedList *)pList->Reference();
	else if (pList->IsNil())
		{
		pList = pCC->CreateLinkedList();
		if (pList->IsError())
			{
			pArgs->Discard(pCC);
			return pList;
			}
		pLinkedList = (CCLinkedList *)pList;
		}
	else
		{
		pArgs->Discard(pCC);
		return pCC->CreateError(CONSTLIT("Linked-list expected:"), NULL);
		}

	//	Do the right thing

	switch (dwData)
		{
		case FN_LINKEDLIST_APPEND:
			{
			ICCItem *pItem = pArgs->GetElement(1);
			ICCItem *pError;

			pLinkedList->Append(pCC, pItem, &pError);
			if (pError->IsError())
				{
				pLinkedList->Discard(pCC);
				pResult = pError;
				}
			else
				{
				pError->Discard(pCC);
				pResult = pLinkedList;
				}
			break;
			}

		case FN_LINKEDLIST_REMOVE:
			{
			int iIndex = pArgs->GetElement(1)->GetIntegerValue();

			//	Make sure we're in range

			if (iIndex < 0 || iIndex >= pLinkedList->GetCount())
				{
				pLinkedList->Discard(pCC);
				pResult = pCC->CreateError(CONSTLIT("Index out of range:"), pArgs->GetElement(1));
				}
			else
				{
				pLinkedList->RemoveElement(pCC, iIndex);
				pResult = pLinkedList;
				}

			break;
			}

		case FN_LINKEDLIST_REMOVE_NIL:
			{
			//	Iterate over all elements and remove any elements that are Nil

			int iIndex = 0;
			while (iIndex < pLinkedList->GetCount())
				{
				if (pLinkedList->GetElement(iIndex)->IsNil())
					pLinkedList->RemoveElement(pCC, iIndex);
				else
					iIndex++;
				}

			//	Done

			pResult = pLinkedList;
			break;
			}

		case FN_LINKEDLIST_REPLACE:
			{
			int iIndex = pArgs->GetElement(1)->GetIntegerValue();
			ICCItem *pItem = pArgs->GetElement(2);

			//	Make sure we're in range

			if (iIndex < 0 || iIndex >= pLinkedList->GetCount())
				{
				pLinkedList->Discard(pCC);
				pResult = pCC->CreateError(CONSTLIT("Index out of range:"), pArgs->GetElement(1));
				}
			else
				{
				pLinkedList->ReplaceElement(pCC, iIndex, pItem);
				pResult = pLinkedList;
				}

			break;
			}

		default:
			ASSERT(FALSE);
			return NULL;
		}

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnList (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnList
//
//	Creates a list out of multiple arguments
//
//	(list exp1 exp2 ... expn)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;

	//	Evaluate the arguments

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("*"));
	if (pArgs->IsError())
		return pArgs;

	//	Return the args (since they are already in a list!)

	return pArgs;
	}

ICCItem *fnLogical (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnLogical
//
//	Logical operators
//
//	(and exp1 exp2 ... expn)
//	(or exp1 exp2 ... expn)

	{
	CCodeChain *pCC = pCtx->pCC;
	int i;

	//	Loop over all arguments

	for (i = 0; i < pArguments->GetCount(); i++)
		{
		ICCItem *pResult;
		ICCItem *pArg = pArguments->GetElement(i);

		//	Evaluate the item

		if (pArg->IsQuoted())
			pResult = pArg->Reference();
		else
			{
			pResult = pCC->Eval(pCtx, pArg);

			if (pResult->IsError())
				return pResult;
			}

		//	If we are evaluating NOT then reverse the value

		if (dwData == FN_LOGICAL_NOT)
			{
			if (pResult->IsNil())
				{
				pResult->Discard(pCC);
				return pCC->CreateTrue();
				}
			else
				{
				pResult->Discard(pCC);
				return pCC->CreateNil();
				}
			}

		//	If we are evaluating AND and we've got Nil, then
		//	we can guarantee that the expression is Nil

		else if (dwData == FN_LOGICAL_AND && pResult->IsNil())
			return pResult;

		//	Otherwise, if we're evaluating OR and we've got non-Nil,
		//	then we can guarantee that the expression is True

		else if (dwData == FN_LOGICAL_OR && !pResult->IsNil())
			{
			pResult->Discard(pCC);
			return pCC->CreateTrue();
			}

		//	Otherwise, we continue

		pResult->Discard(pCC);
		}

	//	If we get here then all the operands are the same (either all
	//	True or all Nil depending)

	if (dwData == FN_LOGICAL_AND)
		return pCC->CreateTrue();
	else
		return pCC->CreateNil();
	}

ICCItem *fnLoop (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnLoop
//
//	Evaluates an expression until the condition is Nil
//
//	(loop condition exp)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pResult;
	ICCItem *pCondition;
	ICCItem *pBody;
	BOOL bDone;

	//	The first argument must be a conditional expression

	pCondition = pArguments->Head(pCC);
	if (pCondition == NULL)
		return pCC->CreateError(CONSTLIT("Loop condition expected"), NULL);

	//	The second argument is the body

	pBody = pArguments->GetElement(1);
	if (pBody == NULL)
		return pCC->CreateError(CONSTLIT("Loop body expected"), NULL);

	//	Create a default result

	pResult = pCC->CreateNil();

	//	Keep evaluating the condition and looping
	//	until we get an error or the condition is Nil

	bDone = FALSE;
	do
		{
		ICCItem *pEval;

		//	Evaluate the condition

		pEval = pCC->Eval(pCtx, pCondition);
		if (pEval->IsError())
			return pResult;

		//	If the condition is Nil, then we're done

		if (pEval->IsNil())
			{
			pEval->Discard(pCC);
			bDone = TRUE;
			}

		//	Otherwise, evaluate the body of the loop

		else
			{
			pEval->Discard(pCC);

			//	Clean up the old result

			pResult->Discard(pCC);

			//	Evaluate the body

			pResult = pCC->Eval(pCtx, pBody);
			if (pResult->IsError())
				return pResult;
			}
		}
	while (!bDone);

	//	Done

	return pResult;
	}

ICCItem *fnMathList (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnMathList
//
//	Simple integer functions
//
//	(add x1 x2 ... xn) -> z
//	(max x1 x2 ... xn) -> z
//	(min x1 x2 ... xn) -> z
//	(multiply x1 x2 .. .xn) -> z

	{
	int i;
	CCodeChain *pCC = pCtx->pCC;

	//	Get the list

	ICCItem *pList;
	if (pArgs->GetElement(0)->IsList())
		{
		pList = pArgs->GetElement(0);

		if (pList->GetCount() < 1)
			return pCC->CreateNil();
		}
	else
		pList = pArgs;

	//	Do the computation

	switch (dwData)
		{
		case FN_MATH_ADD:
			{
			int iResult = pList->GetElement(0)->GetIntegerValue();
			for (i = 1; i < pList->GetCount(); i++)
				iResult += pList->GetElement(i)->GetIntegerValue();
			return pCC->CreateInteger(iResult);
			}

		case FN_MATH_MULTIPLY:
			{
			int iResult = pList->GetElement(0)->GetIntegerValue();
			for (i = 1; i < pList->GetCount(); i++)
				iResult *= pList->GetElement(i)->GetIntegerValue();
			return pCC->CreateInteger(iResult);
			}

		case FN_MATH_MAX:
			{
			int iResult = pList->GetElement(0)->GetIntegerValue();
			for (i = 1; i < pList->GetCount(); i++)
				{
				int iVal = pList->GetElement(i)->GetIntegerValue();
				if (iVal > iResult)
					iResult = iVal;
				}
			return pCC->CreateInteger(iResult);
			}

		case FN_MATH_MIN:
			{
			int iResult = pList->GetElement(0)->GetIntegerValue();
			for (i = 1; i < pList->GetCount(); i++)
				{
				int iVal = pList->GetElement(i)->GetIntegerValue();
				if (iVal < iResult)
					iResult = iVal;
				}
			return pCC->CreateInteger(iResult);
			}

		default:
			ASSERT(false);
			return pCC->CreateNil();
		}
	}

ICCItem *fnMath (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnMath
//
//	Simple integer arithmetic
//
//	(sqrt int1)

	{
	CCodeChain *pCC = pCtx->pCC;

	//	Compute

	switch (dwData)
		{
		case FN_MATH_SQRT:
			{
			int iValue = pArgs->GetElement(0)->GetIntegerValue();
			if (iValue >= 0)
				return pCC->CreateInteger(mathSqrt(iValue));
			else
				return pCC->CreateError(CONSTLIT("Imaginary number"), pArgs->GetElement(0));
			}

		default:
			ASSERT(false);
			return pCC->CreateNil();
		}
	}

ICCItem *fnMathOld (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnMathOld
//
//	Simple integer arithmetic
//
//	(divide int1 int2)
//	(modulo int1 int2)
//	(subtract int1 int2)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	int iResult, iOp1, iOp2;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("ii"));
	if (pArgs->IsError())
		return pArgs;

	//	Convert to integers

	iOp1 = pArgs->GetElement(0)->GetIntegerValue();
	iOp2 = pArgs->GetElement(1)->GetIntegerValue();

	//	Done with arguments

	pArgs->Discard(pCC);

	//	Compute

	switch (dwData)
		{
		case FN_MATH_SUBTRACT:
			iResult = iOp1 - iOp2;
			break;

		case FN_MATH_DIVIDE:
			if (iOp2 != 0)
				iResult = iOp1 / iOp2;
			else
				return pCC->CreateError(CONSTLIT("Division by zero"), pArguments);
			break;

		case FN_MATH_MODULUS:
			if (iOp2 != 0)
				iResult = iOp1 % iOp2;
			else
				return pCC->CreateError(CONSTLIT("Division by zero"), pArguments);
			break;

		case FN_MATH_POWER:
			iResult = mathPower(iOp1, iOp2);
			break;

		default:
			ASSERT(FALSE);
		}

	//	Done

	return pCC->CreateInteger(iResult);
	}

ICCItem *fnRandom (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnRandom
//
//	(random from to) -> number
//	(random list) -> random item in list

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("*"));
	if (pArgs->IsError())
		return pArgs;

	//	Do stuff based on parameters

	if (pArgs->GetCount() == 2)
		{
		int iOp1 = pArgs->GetElement(0)->GetIntegerValue();
		int iOp2 = pArgs->GetElement(1)->GetIntegerValue();
		pArgs->Discard(pCC);

		pResult = pCC->CreateInteger(mathRandom(iOp1, iOp2));
		}
	else if (pArgs->GetCount() == 1)
		{
		ICCItem *pList = pArgs->GetElement(0);

		if (pList->IsNil())
			pResult = pCC->CreateNil();
		else if (pList->GetCount() == 0)
			pResult = pCC->CreateNil();
		else
			pResult = pList->GetElement(mathRandom(0, pList->GetCount()-1))->Reference();

		pArgs->Discard(pCC);
		}
	else
		{
		pResult = pCC->CreateNil();
		pArgs->Discard(pCC);
		}

	//	Done

	return pResult;
	}

ICCItem *fnSeededRandom (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnSeededRandom
//
//	(seededRandom seed from to) -> number
//	(seededRandom seed list) -> random item in list

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pResult;

	int iSeed = pArgs->GetElement(0)->GetIntegerValue();

	//	Do stuff based on parameters

	if (pArgs->GetCount() == 3)
		{
		int iOp1 = pArgs->GetElement(1)->GetIntegerValue();
		int iOp2 = pArgs->GetElement(2)->GetIntegerValue();

		pResult = pCC->CreateInteger(mathSeededRandom(iSeed, iOp1, iOp2));
		}
	else if (pArgs->GetCount() == 2)
		{
		ICCItem *pList = pArgs->GetElement(1);

		if (pList->IsNil())
			pResult = pCC->CreateNil();
		else if (pList->GetCount() == 0)
			pResult = pCC->CreateNil();
		else
			pResult = pList->GetElement(mathSeededRandom(iSeed, 0, pList->GetCount()-1))->Reference();
		}
	else
		pResult = pCC->CreateNil();

	//	Done

	return pResult;
	}

ICCItem *fnSet (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnSet
//
//	Bind an identifier to some value
//
//	(set var exp)
//	(setq var exp)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pVar;
	ICCItem *pValue;
	int iFrame, iOffset;
	ICCItem *pSymTable;

	//	Evaluate the arguments and validate them

	if (dwData == FN_SET_SET)
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("sv"));
	else
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("qv"));
	if (pArgs->IsError())
		return pArgs;

	//	First is the variable; next is the value

	pVar = pArgs->GetElement(0);
	pValue = pArgs->GetElement(1);

	//	Figure out which symbol table to start with

	if (pCtx->pLocalSymbols)
		pSymTable = pCtx->pLocalSymbols;
	else
		pSymTable = pCtx->pLexicalSymbols;

	//	If this variable has already been bound, then use a short-cut

	if (pVar->GetBinding(&iFrame, &iOffset))
		{
		while (iFrame > 0)
			{
			pSymTable = pSymTable->GetParent();
			iFrame--;
			}

		pSymTable->AddByOffset(pCC, iOffset, pValue);
		}
	else
		{
		ICCItem *pError;

		pError = pSymTable->AddEntry(pCC, pVar, pValue);

		//	Check for error

		if (pError->IsError())
			{
			pArgs->Discard(pCC);
			return pError;
			}

		pError->Discard(pCC);
		}

	//	Keep a reference to the value, so we can return it

	pValue->Reference();

	//	Done with these

	pArgs->Discard(pCC);

	//	Done

	return pValue;
	}

ICCItem *fnShuffle (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnShuffle
//
//	Shuffles a list randomly

	{
	int i;
	CCodeChain *pCC = pCtx->pCC;

	if (pArgs->GetElement(0)->IsNil())
		return pCC->CreateNil();

	//	Create a destination list

	ICCItem *pResult = pCC->CreateLinkedList();
	if (pResult->IsError())
		return pResult;

	CCLinkedList *pList = (CCLinkedList *)pResult;

	//	Copy the list

	ICCItem *pSource = pArgs->GetElement(0);
	for (i = 0; i < pSource->GetCount(); i++)
		pList->Append(pCC, pSource->GetElement(i), NULL);

	//	Shuffle the new list

	pList->Shuffle(pCC);

	//	Done

	return pResult;
	}

ICCItem *fnStrCapitalize (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnStrCapitalize
//
//	Capitalizes the string
//
//	(strCapitalize string) -> string

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("s"));
	if (pArgs->IsError())
		return pArgs;

	CString sString = pArgs->GetElement(0)->GetStringValue();
	pArgs->Discard(pCC);

	//	Done

	sString.Capitalize(CString::capFirstLetter);
	return pCC->CreateString(sString);
	}

ICCItem *fnStrFind (CEvalContext *pCtx, ICCItem *pArgs, DWORD dwData)

//	fnStrFind
//
//	Finds a string in some text

	{
	CCodeChain *pCC = pCtx->pCC;
	int iPos = strFind(pArgs->GetElement(0)->GetStringValue(), pArgs->GetElement(1)->GetStringValue());

	if (iPos == -1)
		return pCC->CreateNil();
	else
		return pCC->CreateInteger(iPos);
	}

ICCItem *fnSubst (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnSubst
//
//	Substitutes string parameters
//
//	(subst string arg1 arg2 ... argn)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;

	//	Evaluate the arguments and validate them

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("s*"));
	if (pArgs->IsError())
		return pArgs;

	CString sPattern = pArgs->GetElement(0)->GetStringValue();

	//	Do the substitution

	char szResult[4096];
	char *pPos = sPattern.GetASCIIZPointer();
	char *pDest = szResult;
	char *pEndDest = szResult + sizeof(szResult) - 1;

	while (*pPos != '\0' && pDest < pEndDest)
		{
		if (*pPos == '%')
			{
			pPos++;

			int iArg = strParseInt(pPos, 0, &pPos, NULL);
			if (iArg > 0)
				{
				CString sParam = pArgs->GetElement(iArg)->GetStringValue();
				char *pParam = sParam.GetASCIIZPointer();

				while (*pParam != '\0' && pDest < pEndDest)
					*pDest++ = *pParam++;

				pPos++;
				}
			else
				{
				if (*pPos == '%')
					*pDest++ = *pPos++;
				}
			}
		else
			*pDest++ = *pPos++;
		}

	//	Done

	*pDest = '\0';
	pArgs->Discard(pCC);
	return pCC->CreateString(CString(szResult));
	}

ICCItem *fnSwitch (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnSwitch
//
//	Switch control function
//
//	(switch exp1 case1 exp2 case2 ... default)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pTest;
	ICCItem *pThen;
	ICCItem *pResult;

	int iArgPos = 0;
	while (iArgPos+1 < pArguments->GetCount())
		{
		//	Get the arguments

		pTest = pArguments->GetElement(iArgPos);
		pThen = pArguments->GetElement(iArgPos+1);

		//	Evaluate the expression

		pResult = pCC->Eval(pCtx, pTest);
		if (pResult->IsError())
			return pResult;

		//	If the result is not Nil, then evaluate the Then expression

		if (!pResult->IsNil())
			{
			pResult->Discard(pCC);
			return pCC->Eval(pCtx, pThen);
			}

		//	Otherwise, continue with the loop

		pResult->Discard(pCC);
		iArgPos += 2;
		}

	//	Do we have a default case?

	if (iArgPos < pArguments->GetCount())
		{
		ICCItem *pElse = pArguments->GetElement(iArgPos);
		return pCC->Eval(pCtx, pElse);
		}

	//	Otherwise, we return Nil

	return pCC->CreateNil();
	}

ICCItem *fnSymCreate (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnSymCreate
//
//	Creates a new symbol table
//
//	(symCreate) -> symtable

	{
	CCodeChain *pCC = pCtx->pCC;

	return pCC->CreateNil();
	}

ICCItem *fnSymTable (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnSymTable
//
//	Various symbol table manipulations
//
//	(symAddEntry symTable symbol entry) -> entry
//	(symDeleteEntry symTable symbol) -> True
//	(symLookup symTable symbol) -> entry

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pSymTable;
	ICCItem *pSymbol;
	ICCItem *pResult;

	//	Evaluate the arguments and validate them

	if (dwData == FN_SYMTABLE_ADDENTRY)
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("ysv"));
	else if (dwData == FN_SYMTABLE_LIST)
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("y"));
	else
		pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("ys"));

	if (pArgs->IsError())
		return pArgs;

	//	Get the args

	pSymTable = pArgs->Head(pCC);

	//	Do the right thing

	switch (dwData)
		{
		case FN_SYMTABLE_ADDENTRY:
			{
			ICCItem *pEntry;

			pSymbol = pArgs->GetElement(1);
			pEntry = pArgs->GetElement(2);
			pResult = pSymTable->AddEntry(pCC, pSymbol, pEntry);

			//	If we succeeded, return the entry

			if (!pResult->IsError())
				{
				pResult->Discard(pCC);
				pResult = pEntry->Reference();
				}

			break;
			}

		case FN_SYMTABLE_DELETEENTRY:
			{
			pResult = pCC->CreateNil();
			break;
			}

		case FN_SYMTABLE_LIST:
			{
			pResult = pSymTable->ListSymbols(pCC);
			break;
			}

		case FN_SYMTABLE_LOOKUP:
			{
			pSymbol = pArgs->GetElement(1);
			pResult = pSymTable->Lookup(pCC, pSymbol);
			break;
			}

		default:
			ASSERT(FALSE);
			return NULL;
		}

	//	Done

	pArgs->Discard(pCC);
	return pResult;
	}

ICCItem *fnSysInfo (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnSysInfo
//
//	Returns system information
//
//	(globals)

	{
	CCodeChain *pCC = pCtx->pCC;

	switch (dwData)
		{
		case FN_SYSINFO_GLOBALS:
			return pCC->ListGlobals();

		case FN_SYSINFO_POOLUSAGE:
			return pCC->PoolUsage();

		case FN_SYSINFO_TICKS:
			return pCC->CreateInteger(SDL_GetTicks());

		default:
			ASSERT(FALSE);
			return NULL;
		}
	}

int HelperCompareItems (ICCItem *pFirst, ICCItem *pSecond)

//	HelperCompareItems
//
//	Compares two items and returns:
//
//		1	if pFirst > pSecond
//		0	if pFirst = pSecond
//		-1	if pFirst < pSecond
//		-2	if pFirst is not the same type as pSecond

	{
	//	Compare this with the first expression

	if (pFirst->GetValueType() == pSecond->GetValueType())
		{
		switch (pFirst->GetValueType())
			{
			case ICCItem::Boolean:
				{
				if (pFirst->IsNil() == pSecond->IsNil())
					return 0;
				else if (pFirst->IsNil())
					return -1;
				else
					return 1;
				}

			case ICCItem::Integer:
				{
				if (pFirst->GetIntegerValue() == pSecond->GetIntegerValue())
					return 0;
				else if (pFirst->GetIntegerValue() > pSecond->GetIntegerValue())
					return 1;
				else
					return -1;
				}

			case ICCItem::String:
				return strCompare(pFirst->GetStringValue(), pSecond->GetStringValue());

			case ICCItem::List:
				{
				if (pFirst->GetCount() == pSecond->GetCount())
					{
					int i;

					for (i = 0; i < pFirst->GetCount(); i++)
						{
						ICCItem *pItem1 = pFirst->GetElement(i);
						ICCItem *pItem2 = pSecond->GetElement(i);
						int iCompare;

						iCompare = HelperCompareItems(pItem1, pItem2);
						if (iCompare != 0)
							return iCompare;
						}

					return 0;
					}
				else if (pFirst->GetCount() > pSecond->GetCount())
					return 1;
				else
					return -1;
				break;
				}

			default:
				return -2;
			}
		}
	else
		return -2;
	}

ICCItem *fnVecCreate (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnVecCreate
//
//	Creates a new vector of a given size
//
//	(vecVector size) -> vector
//
//	All elements of the vector are initialized to 0

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	ICCItem *pVector;

	//	Evaluate the argument

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("i"));
	if (pArgs->IsError())
		return pArgs;

	//	Create the table

	pVector = pCC->CreateVector(pArgs->Head(pCC)->GetIntegerValue());

	//	Done

	pArgs->Discard(pCC);
	return pVector;
	}

ICCItem *fnVector (CEvalContext *pCtx, ICCItem *pArguments, DWORD dwData)

//	fnVector
//
//	Vector functions
//
//	(vecSetElement vector index element)

	{
	CCodeChain *pCC = pCtx->pCC;
	ICCItem *pArgs;
	CCVector *pVector;
	BOOL bOk;

	//	Evaluate the arguments

	pArgs = pCC->EvaluateArgs(pCtx, pArguments, CONSTLIT("vii"));
	if (pArgs->IsError())
		return pArgs;

	//	Get the vector

	pVector = dynamic_cast<CCVector *>(pArgs->GetElement(0));
	if (pVector == NULL)
		{
		ICCItem *pError = pCC->CreateError(CONSTLIT("Vector expected:"), pArgs->GetElement(0));;
		pArgs->Discard(pCC);
		return pError;
		}

	//	Set the element

	bOk = pVector->SetElement(pArgs->GetElement(1)->GetIntegerValue(),
			pArgs->GetElement(2)->GetIntegerValue());

	if (!bOk)
		{
		pArgs->Discard(pCC);
		return pCC->CreateError(CONSTLIT("Unable to set vector element"), NULL);
		}

	//	Done

	pArgs->Discard(pCC);
	return pCC->CreateTrue();
	}
