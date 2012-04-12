//	Link.cpp
//
//	Parsing routines

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"

#include "CStream.h"
#include "CCCodeChain.h"

#define SYMBOL_QUOTE					'\''	//	Quote symbol
#define SYMBOL_OPENPAREN				'('		//	Open list
#define SYMBOL_CLOSEPAREN				')'		//	Close list
#define SYMBOL_OPENQUOTE				'\"'	//	Open quote
#define SYMBOL_CLOSEQUOTE				'\"'	//	Close quote
#define SYMBOL_BACKSLASH				'\\'	//	Back-slash

ICCItem *CCodeChain::Link (const CString &sString, int iOffset, int *retiLinked)

//	Link
//
//	Parses the given string and converts it into a linked
//	chain of items

	{
	char *pStart;
	char *pPos;
	ICCItem *pResult = NULL;

	pStart = sString.GetPointer() + iOffset;
	pPos = pStart;

	//	Skip any whitespace

	pPos = SkipWhiteSpace(pPos);

	//	If we've reached the end, then we have
	//	nothing

	if (*pPos == '\0')
		pResult = CreateNil();

	//	If we've got a literal quote, then remember it

	else if (*pPos == SYMBOL_QUOTE)
		{
		int iLinked;

		pPos++;

		pResult = Link(sString, iOffset + (pPos - pStart), &iLinked);
		if (pResult->IsError())
			return pResult;

		pPos += iLinked;

		//	Make it a literal

		pResult->SetQuoted();
		}

	//	If we've got an open paren then we start a list

	else if (*pPos == SYMBOL_OPENPAREN)
		{
		ICCItem *pNew = CreateLinkedList();
		if (pNew->IsError())
			return pResult;

		CCLinkedList *pList = dynamic_cast<CCLinkedList *>(pNew);

		//	Keep reading until we find the end

		pPos++;

		//	If the list is empty, then there's nothing to do

		pPos = SkipWhiteSpace(pPos);
		if (*pPos == SYMBOL_CLOSEPAREN)
			{
			pList->Discard(this);
			pResult = CreateNil();
			pPos++;
			}

		//	Get all the items in the list

		else
			{
			while (*pPos != SYMBOL_CLOSEPAREN && *pPos != '\0')
				{
				ICCItem *pItem;
				int iLinked;

				pItem = Link(sString, iOffset + (pPos - pStart), &iLinked);
				if (pItem->IsError())
					return pItem;

				//	Add the item to the list

				pList->Append(this, pItem, NULL);
				pItem->Discard(this);

				//	Move the position

				pPos += iLinked;

				//	Skip whitespace

				pPos = SkipWhiteSpace(pPos);
				}

			//	If we have a close paren then we're done; Otherwise we've
			//	got an error of some kind

			if (*pPos == SYMBOL_CLOSEPAREN)
				{
				pPos++;
				pResult = pList;
				}
			else
				{
				pList->Discard(this);
				pResult = CreateError(LITERAL("Mismatched open parenthesis"), NULL);
				}
			}
		}

	//	If this is an open quote, then read everything until
	//	the close quote

	else if (*pPos == SYMBOL_OPENQUOTE)
		{
		//	Parse the string, until the end quote, parsing escape codes

		char *pStartFragment = NULL;
		CString sResultString;

		bool bDone = false;
		while (!bDone)
			{
			pPos++;

			switch (*pPos)
				{
				case SYMBOL_CLOSEQUOTE:
				case '\0':
					{
					if (pStartFragment)
						{
						sResultString.Append(CString(pStartFragment, pPos - pStartFragment));
						pStartFragment = NULL;
						}

					bDone = true;
					break;
					}

				case SYMBOL_BACKSLASH:
					{
					if (pStartFragment)
						{
						sResultString.Append(CString(pStartFragment, pPos - pStartFragment));
						pStartFragment = NULL;
						}

					pPos++;
					if (*pPos == '\0')
						bDone = true;
					else
						{
						if (*pPos == 'n')
							sResultString.Append(CString("\n", 1));
						else
							sResultString.Append(CString(pPos, 1));
						}

					break;
					}

				default:
					{
					if (pStartFragment == NULL)
						pStartFragment = pPos;

					break;
					}
				}
			}

		//	If we found the close, then create a string; otherwise,
		//	it is an error

		if (*pPos == SYMBOL_CLOSEQUOTE)
			{
			pResult = CreateString(sResultString);

			//	Always a literal

			pResult->SetQuoted();

			//	Skip past quote

			pPos++;
			}
		else
			pResult = CreateError(LITERAL("Mismatched quote"), NULL);
		}

	//	If this is a close paren, then it is an error

	else if (*pPos == SYMBOL_CLOSEPAREN)
		pResult = CreateError(LITERAL("Mismatched close parenthesis"), NULL);

	//	Otherwise this is an string of some sort

	else
		{
		char *pStartString;
		CString sIdentifier;
		int iInt;
		BOOL bNotInteger;

		pStartString = pPos;

		//	Look for whitespace

    	while (*pPos != '\0'
        		&& *pPos != ' ' && *pPos != '\n' && *pPos != '\r' && *pPos != '\t'
            	&& *pPos != SYMBOL_OPENPAREN
				&& *pPos != SYMBOL_CLOSEPAREN
				&& *pPos != SYMBOL_OPENQUOTE
				&& *pPos != SYMBOL_CLOSEQUOTE)
        	pPos++;

		//	Create a string from the portion

		sIdentifier = strSubString(sString, iOffset + (pStartString - pStart), (pPos - pStartString));

		//	Check to see if this is a reserved identifier

		if (strCompareAbsolute(sIdentifier, CONSTLIT("Nil")) == 0)
			pResult = CreateNil();
		else if (strCompareAbsolute(sIdentifier, CONSTLIT("True")) == 0)
			pResult = CreateTrue();
		else
			{
			//	If this is an integer, create an integer; otherwise
			//	create a string

			iInt = strToInt(sIdentifier, 0, &bNotInteger);
			if (bNotInteger)
				pResult = CreateString(sIdentifier);
			else
				pResult = CreateInteger(iInt);
			}
		}

	//	Return the result and the number of characters
	//	that we read

	if (retiLinked)
		*retiLinked = (pPos - pStart);

	return pResult;
	}

char *CCodeChain::SkipWhiteSpace (char *pPos)

//	SkipWhiteSpace
//
//	Skips white space and comments when parsing

	{
    BOOL bDone = FALSE;
    BOOL bInComment = FALSE;
    
    while (!bDone)
        {
    	/*	If we're inside a comment, keep going until we find the end of the line;
        	Otherwise, just skip white space */
        
        if (*pPos == '\0')
            bDone = TRUE;
        else if (bInComment)
            {
        	if (*pPos == '\n' || *pPos == '\r')
                bInComment = FALSE;
            pPos++;
        	}
        else
            {
        	if (*pPos == ';')
                {
                bInComment = TRUE;
                pPos++;
            	}
            else if (*pPos == ' ' || *pPos == '\n' || *pPos == '\r' || *pPos == '\t')
                pPos++;
            else
                bDone = TRUE;
        	}
    	}
    
    return pPos;
    }

CString CCodeChain::Unlink (ICCItem *pItem)

//	Unlink
//
//	Converts from a linked chain of items to a single
//	string.

	{
	return pItem->Print(this);
	}
