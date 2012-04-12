//	Words.cpp
//
//	Generate statistics about text

#include <stdio.h>

#include <windows.h>
#include <ddraw.h>
#include "Alchemy.h"
#include "XMLUtil.h"
#include "TransData.h"

#define MODULES_TAG							(CONSTLIT("Modules"))
#define FILENAME_ATTRIB						(CONSTLIT("filename"))

struct TraverseCtx
	{
	CSymbolTable *pWordList;
	};

void AddText (TraverseCtx &Ctx, const CString &sText);
void AddWord (TraverseCtx &Ctx, const CString &sWord);
void ParseWordList (TraverseCtx &Ctx, CXMLElement *pElement);

void GenerateWordList (const CString &sDataFile, CXMLElement *pCmdLine)

//	GenerateWordList
//
//	Generate a list of unique words used in the game

	{
	ALERROR error;
	int i;
	CString sError;

	//	Open the XML file

	CResourceDb Resources(sDataFile);
	if (error = Resources.Open())
		{
		printf("Unable to initialize data file.\n");
		return;
		}

	CXMLElement *pGameFile;
	if (error = Resources.LoadGameFile(&pGameFile, &sError))
		{
		printf("%s\n", sError.GetASCIIZPointer());
		return;
		}

	//	Create the context

	CSymbolTable WordList(FALSE, TRUE);
	TraverseCtx Ctx;
	Ctx.pWordList = &WordList;

	//	Recursive descent

	ParseWordList(Ctx, pGameFile);

	//	Parse all modules too

	CXMLElement *pModules = pGameFile->GetContentElementByTag(MODULES_TAG);
	if (pModules)
		{
		for (i = 0; i < pModules->GetContentElementCount(); i++)
			{
			CXMLElement *pModule = pModules->GetContentElement(i);
			CXMLElement *pModuleXML;
			if (error = Resources.LoadModule(NULL_STR, pModule->GetAttribute(FILENAME_ATTRIB), &pModuleXML, &sError))
				{
				printf("%s\n", sError.GetASCIIZPointer());
				return;
				}

			ParseWordList(Ctx, pModuleXML);
			}
		}

	//	Print out the word list

	for (i = 0; i < WordList.GetCount(); i++)
		printf("%s\n", WordList.GetKey(i).GetASCIIZPointer());
	}

void AddText (TraverseCtx &Ctx, const CString &sText)
	{
	enum States
		{
		stateInText,
		stateInDelimeter,
		stateDone,
		};

	//	Parse the text into words

	char *pPos = sText.GetASCIIZPointer();
	char *pStart = pPos;
	int iState = stateInText;

	while (iState != stateDone)
		{
		bool bText = (*pPos >= 'A' && *pPos <= 'Z') 
				|| (*pPos >= 'a' && *pPos <= 'z');

		switch (iState)
			{
			case stateInText:
				{
				if (!bText)
					{
					AddWord(Ctx, CString(pStart, pPos - pStart));
					iState = stateInDelimeter;
					}
				break;
				}

			case stateInDelimeter:
				{
				if (bText)
					{
					pStart = pPos;
					iState = stateInText;
					}
				break;
				}
			}

		//	Next

		if (*pPos == '\0')
			iState = stateDone;
		else
			pPos++;
		}
	}

void AddWord (TraverseCtx &Ctx, const CString &sWord)
	{
	//	If this is a single character, then skip it

	if (sWord.GetLength() == 1)
		;

	//	Otherwise, add it

	else
		{
		Ctx.pWordList->AddEntry(sWord, NULL);
		}
	}

void ParseWordList (TraverseCtx &Ctx, CXMLElement *pElement)
	{
	int i;

	//	Parse all attributes

	for (i = 0; i < pElement->GetAttributeCount(); i++)
		AddText(Ctx, pElement->GetAttribute(i));

	//	If this has sub-elements, then recurse

	if (pElement->GetContentElementCount())
		{
		for (i = 0; i < pElement->GetContentElementCount(); i++)
			ParseWordList(Ctx, pElement->GetContentElement(i));
		}

	//	Otherwise, add the content 

	else
		{
		AddText(Ctx, pElement->GetContentText(0));
		}
	}
