//	Pattern.cpp
//
//	String pattern package
//
//	Pattern format:
//
//	%type
//
//	where type is one of the following:
//
//	%	Evaluates to a single percent sign
//
//	d	Argument is a signed 32-bit integer. The number is substituted
//
//	p	If the last integer argument not 1, then 's' is substituted.
//		This is used to pluralize words in the English language.
//
//	s	Argument is a CString. The string is substituted
//
//	x	Argument is an unsigned 32-bit integer. The hex value is substituted
#include <stdio.h>
#include <stdarg.h>

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"
#include "CMath.h"

#include "CStream.h"

void WritePadding (CMemoryWriteStream &Stream, char chChar, int iLen);

CString strPattern (const CString &sPattern, va_list *ap)

//	strPattern
//
//	Returns a string with a pattern substitution

	{
	ALERROR error;
	CMemoryWriteStream Stream(0);
	char *pPos = sPattern.GetPointer();
	int iLength = sPattern.GetLength();
	char *pRunStart;
	int iRunLength;
	int iLastInteger = 1;

	//	Initialize the memory stream that will hold the result

	if (error = Stream.Create())
		return LITERAL("%ERROR");

	//	Start

	pRunStart = pPos;
	iRunLength = 0;

	//	Loop

	while (iLength > 0)
		{
		if (*pPos == '%')
			{
			//	Save out what we've got til now

			if (iRunLength > 0)
				{
				if (error = Stream.Write(pRunStart, iRunLength, NULL))
					return LITERAL("%ERROR");
				}

			//	Check the actual pattern code

			pPos++;
			iLength--;
			if (iLength > 0)
				{
				//	See if we've got a field width value

				int iMinFieldWidth = 0;
				bool bPadWithZeros = false;
				if (*pPos >= '0' && *pPos <= '9')
					{
					char *pNewPos;
					bPadWithZeros = (*pPos == '0');
					iMinFieldWidth = strParseInt(pPos, 0, &pNewPos, NULL);

					if (iLength > (pNewPos - pPos))
						{
						iLength -= (pNewPos - pPos);
						pPos = pNewPos;
						}
					}

				//	Parse the type

				if (*pPos == 's')
					{
					char *pParam = va_arg(*ap, char *);

					if (iMinFieldWidth > 0)
						WritePadding(Stream, (bPadWithZeros ? '0' : ' '), iMinFieldWidth - strlen(pParam));

					if (error = Stream.Write(pParam, strlen(pParam), NULL))
						return LITERAL("%ERROR");

					*pPos++;
					iLength--;
					}
				else if (*pPos == 'd')
					{
					int i = va_arg(*ap, int);
					CString sNew;

					sNew = strFromInt(i, TRUE);

					if (iMinFieldWidth > 0)
						WritePadding(Stream, (bPadWithZeros ? '0' : ' '), iMinFieldWidth - sNew.GetLength());

					if (error = Stream.Write(sNew.GetPointer(), sNew.GetLength(), NULL))
						return LITERAL("%ERROR");

					//	Remember the last integer

					iLastInteger = i;

					//	Next

					*pPos++;
					iLength--;
					}
				else if (*pPos == 'x')
					{
					int i = va_arg(*ap, int);
					char szBuffer[256];
					int iLen = sprintf(szBuffer, "%x", i);

					if (iMinFieldWidth > 0)
						WritePadding(Stream, (bPadWithZeros ? '0' : ' '), iMinFieldWidth - iLen);

					if (error = Stream.Write(szBuffer, iLen, NULL))
						return LITERAL("%ERROR");

					//	Remember the last integer

					iLastInteger = i;

					//	Next

					*pPos++;
					iLength--;
					}
				else if (*pPos == 'p')
					{
					if (iLastInteger != 1)
						{
						if (error = Stream.Write("s", 1, NULL))
							return LITERAL("%ERROR");
						}

					*pPos++;
					iLength--;
					}
				else if (*pPos == '%')
					{
					if (error = Stream.Write("%", 1, NULL))
						return LITERAL("%ERROR");

					*pPos++;
					iLength--;
					}
				}

			pRunStart = pPos;
			iRunLength = 0;
			}
		else
			{
			iRunLength++;
			iLength--;
			pPos++;
			}
		}

	//	Save out the last run

	if (iRunLength > 0)
		{
		if (error = Stream.Write(pRunStart, iRunLength, NULL))
			return LITERAL("%ERROR");
		}

	//	Convert the stream to a string

	CString sResult(Stream.GetPointer(), Stream.GetLength());
	return sResult;
	}

CString strPatternSubst (CString sLine, ...)

//	strPatternSubst
//
//	Substitutes patterns

	{
	CString sParsedLine;

	va_list ap;
	va_start(ap, sLine);

	sParsedLine = strPattern(sLine, &ap);
	return sParsedLine;
	}

void WritePadding (CMemoryWriteStream &Stream, char chChar, int iLen)
	{
	if (iLen > 0)
		{
		char szBuffer[256];
		char *pBuffer;
		if (iLen <= sizeof(szBuffer))
			pBuffer = szBuffer;
		else
			pBuffer = new char [iLen];

		char *pPos = pBuffer;
		char *pEndPos = pPos + iLen;
		while (pPos < pEndPos)
			*pPos++ = chChar;

		Stream.Write(pBuffer, iLen, NULL);

		if (pBuffer != szBuffer)
			delete [] pBuffer;
		}
	}
