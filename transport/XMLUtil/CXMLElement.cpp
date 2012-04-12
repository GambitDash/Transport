//	CXMLElement.cpp
//
//	CXMLElement class

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"

#include "CArray.h"
#include "CSymbolTable.h"
#include "CObjectArray.h"
#include "CStringArray.h"

#include "XMLUtil.h"

static DATADESCSTRUCT g_DataDesc[] =
	{	{ DATADESC_OPCODE_EMBED_OBJ,	1,	0 },		//	m_sTag
		{ DATADESC_OPCODE_REFERENCE,	1,	0 },		//	m_pParent
		{ DATADESC_OPCODE_EMBED_OBJ,	1,	0 },		//	m_Attributes
		{ DATADESC_OPCODE_EMBED_OBJ,	1,	0 },		//	m_ContentElements
		{ DATADESC_OPCODE_EMBED_OBJ,	1,	0 },		//	m_ContentText
		{ DATADESC_OPCODE_STOP,	0,	0 } };
static CObjectClass<CXMLElement>g_Class(OBJID_CXMLELEMENT, g_DataDesc);

CXMLElement::CXMLElement (void) : CObject(&g_Class),
		m_pParent(NULL),
		m_Attributes(TRUE, FALSE),
		m_ContentElements(TRUE)

//	CXMLElement constructor

	{
	m_ContentText.AppendString(CString(), NULL);
	}

CXMLElement::CXMLElement (const CString &sTag, CXMLElement *pParent) : CObject(&g_Class),
		m_sTag(sTag),
		m_pParent(pParent),
		m_Attributes(TRUE, FALSE),
		m_ContentElements(TRUE)

//	CXMLElement constructor

	{
	m_ContentText.AppendString(CString(), NULL);
	}

ALERROR CXMLElement::AddAttribute (const CString &sAttribute, const CString &sValue)

//	AddAttribute
//
//	Add the given attribute to our table

	{
	ALERROR error;
	CString *pValue;

	pValue = new CString(sValue);
	if (pValue == NULL)
		return ERR_MEMORY;

	if (error = m_Attributes.AddEntry(sAttribute, pValue))
		{
		delete pValue;
		return error;
		}

	return NOERROR;
	}

ALERROR CXMLElement::AppendContent (const CString &sContent)

//	AppendContent
//
//	Appends some content

	{
	ALERROR error;

	//	Always append to the last content element

	CString sValue = m_ContentText.GetStringValue(m_ContentText.GetCount() - 1);
	if (error = sValue.Append(sContent))
		return error;

	if (error = m_ContentText.SetStringValue(m_ContentText.GetCount() - 1, sValue))
		return error;

	return NOERROR;
	}

ALERROR CXMLElement::AppendSubElement (CXMLElement *pElement)

//	AppendSubElement
//
//	Append a sub element

	{
	ALERROR error;

	//	Append the element

	if (error = m_ContentElements.AppendObject(pElement, NULL))
		return error;

	//	We always add a new content text value at the end

	if (m_ContentText.AppendString(CString(), NULL))
		return error;

	return NOERROR;
	}

bool CXMLElement::AttributeExists (const CString &sName)

//	AttributeExists
//
//	Returns TRUE if the attribute exists in the element

	{
	CString *pValue;

	if (m_Attributes.Lookup(sName, (CObject **)&pValue) == NOERROR)
		return true;
	else
		return false;
	}

bool CXMLElement::FindAttribute (const CString &sName, CString *retsValue)

//	FindAttribute
//
//	If the attribute exists, returns TRUE and the attribute value.
//	Otherwise, returns FALSE

	{
	CString *pValue;

	if (m_Attributes.Lookup(sName, (CObject **)&pValue) == NOERROR)
		{
		*retsValue = *pValue;
		return true;
		}
	else
		return false;
	}

bool CXMLElement::FindAttributeInteger (const CString &sName, int *retiValue)

//	FindAttributeInteger
//
//	If the attribute exists, returns TRUE and the attribute value.
//	Otherwise, returns FALSE

	{
	CString *pValue;

	if (m_Attributes.Lookup(sName, (CObject **)&pValue) == NOERROR)
		{
		*retiValue = strToInt(*pValue, 0, NULL);
		return true;
		}
	else
		return false;
	}

CString CXMLElement::GetAttribute (const CString &sName)

//	GetAttribute
//
//	Returns the attribute

	{
	CString *pValue;

	if (m_Attributes.Lookup(sName, (CObject **)&pValue) == NOERROR)
		return *pValue;
	else
		return CString();
	}

bool CXMLElement::GetAttributeBool (const CString &sName)

//	GetAttributeBool
//
//	Returns TRUE or FALSE for the attribute

	{
	CString *pValue;

	if (m_Attributes.Lookup(sName, (CObject **)&pValue) == NOERROR)
		return strEquals(*pValue, CONSTLIT("true"));
	else
		return false;
	}

float CXMLElement::GetAttributeFloat (const CString &sName)

//	GetAttributeFloat
//
//	Returns a floating point attribute

	{
#ifndef LATER
	return (float)GetAttributeInteger(sName);
#endif
	}

int CXMLElement::GetAttributeInteger (const CString &sName)

//	GetAttributeInteger
//
//	Returns an integer attribute

	{
	return strToInt(GetAttribute(sName), 0, NULL);
	}

int CXMLElement::GetAttributeIntegerBounded (const CString &sName, int iMin, int iMax, int iNull)

//	GetAttributeIntegerBounded
//
//	Returns an integer, insuring that it is in range

	{
	CString sValue;
	if (FindAttribute(sName, &sValue))
		{
		BOOL bFailed;
		int iValue = strToInt(sValue, iNull, &bFailed);
		if (bFailed)
			return iNull;

		if (iMax < iMin)
			return std::max(iValue, iMin);
		else
			return std::max(std::min(iValue, iMax), iMin);
		}
	else
		return iNull;
	}

ALERROR CXMLElement::GetAttributeIntegerList (const CString &sName, CIntArray *pList)

//	GetAttributeIntegerList
//
//	Appends a list of integers separated by commas

	{
	return ParseAttributeIntegerList(GetAttribute(sName), pList);
	}

CXMLElement *CXMLElement::GetContentElementByTag (const CString &sTag)

//	GetContentElementByTag
//
//	Returns a sub element of the given tag

	{
	for (int i = 0; i < GetContentElementCount(); i++)
		{
		CXMLElement *pElement = GetContentElement(i);

		if (strCompareAbsolute(sTag, pElement->GetTag()) == 0)
			return pElement;
		}

	return NULL;
	}

CXMLElement *CXMLElement::OrphanCopy (void)

//	OrphanCopy
//
//	Creates a copy of the element and makes it top-level

	{
	CXMLElement *pCopy = (CXMLElement *)Copy();

	if (pCopy)
		pCopy->m_pParent = NULL;

	return pCopy;
	}

CString CXMLElement::MakeAttribute (const CString &sText)

//	MakeAttribute
//
//	Returns a string that has been stripped of all characters
//	that cannot appear in an attribute

	{
	CString sResult;
	int iExtra = 0;

	//	Optimistically assume that the text has no bad
	//	characters.

	char *pDest = sResult.GetWritePointer(sText.GetLength());
	char *pPos = sText.GetASCIIZPointer();
	while (*pPos != '\0')
		{
		switch (*pPos)
			{
			case '&':
				iExtra += 4;		//	&amp
				break;

			case '<':
				iExtra += 3;		//	&lt
				break;

			case '>':
				iExtra += 3;		//	&gt
				break;

			case '\"':
				iExtra += 5;		//	&quot
				break;

			case '\'':
				iExtra += 5;		//	apos
				break;
			}

		*pDest++ = *pPos++;
		}

	//	Done?

	if (iExtra == 0)
		{
		*pDest = '\0';
		return sResult;
		}

	//	Need to escape

	pDest = sResult.GetWritePointer(sText.GetLength() + iExtra);
	pPos = sText.GetASCIIZPointer();
	while (*pPos != '\0')
		{
		switch (*pPos)
			{
			case '&':
				*pDest++ = '&';
				*pDest++ = 'a';
				*pDest++ = 'm';
				*pDest++ = 'p';
				*pDest++ = ';';
				break;

			case '<':
				*pDest++ = '&';
				*pDest++ = 'l';
				*pDest++ = 't';
				*pDest++ = ';';
				break;

			case '>':
				*pDest++ = '&';
				*pDest++ = 'g';
				*pDest++ = 't';
				*pDest++ = ';';
				break;

			case '\"':
				*pDest++ = '&';
				*pDest++ = 'q';
				*pDest++ = 'u';
				*pDest++ = 'o';
				*pDest++ = 't';
				*pDest++ = ';';
				break;

			case '\'':
				*pDest++ = '&';
				*pDest++ = 'a';
				*pDest++ = 'p';
				*pDest++ = 'o';
				*pDest++ = 's';
				*pDest++ = ';';
				break;

			default:
				*pDest++ = *pPos;
				break;
			}

		pPos++;
		}

	//	Done!

	*pDest = '\0';
	return sResult;
	}

