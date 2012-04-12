//	XMLUtil.h
//
//	Library to help parse XML files
//
//	Supported Format:
//
//		document :: <?XML version="1.0"?> {element}
//		
//		element :: {empty-element} | {open-tag} {content}* {close-tag}
//		empty-element :: < TAG {attribute}* />
//		open-tag :: < TAG {attribute}* >
//		close-tag :: </ TAG >
//		attribute :: ATTRIBUTE = " VALUE "
//		content :: {element} | {text} | {cdata} | {comment}
//		text :: any character except & and <
//		cdata :: <![CDATA[ any-text ]]>
//		comment :: <!-- any-text -->

#ifndef INCL_XMLUTIL
#define INCL_XMLUTIL

class CExternalEntityTable;
class IReadBlock;

class IXMLParserController
	{
	public:
		virtual ~IXMLParserController() { }
		virtual CString ResolveExternalEntity (const CString &sName, bool *retbFound = NULL) = 0;
	};

class CXMLElement : public CObject
	{
	public:
		CXMLElement (void);
		CXMLElement (const CString &sTag, CXMLElement *pParent);

		static ALERROR ParseXML (IReadBlock *pStream, 
								 CXMLElement **retpElement, 
								 CString *retsError,
								 CExternalEntityTable *retEntityTable = NULL);
		static ALERROR ParseXML (IReadBlock *pStream, 
								 IXMLParserController *pController,
								 CXMLElement **retpElement, 
								 CString *retsError,
								 CExternalEntityTable *retEntityTable = NULL);
		static ALERROR ParseEntityTable (IReadBlock *pStream, CExternalEntityTable *retEntityTable, CString *retsError);
		static ALERROR ParseRootTag (IReadBlock *pStream, CString *retsTag);

		ALERROR AddAttribute (const CString &sAttribute, const CString &sValue);
		ALERROR AppendContent (const CString &sContent);
		ALERROR AppendSubElement (CXMLElement *pElement);
		bool AttributeExists (const CString &sName);
		bool FindAttribute (const CString &sName, CString *retsValue);
		bool FindAttributeInteger (const CString &sName, int *retiValue);
		CString GetAttribute (const CString &sName);
		inline CString GetAttribute (int iIndex) { return *(CString *)m_Attributes.GetValue(iIndex); }
		bool GetAttributeBool (const CString &sName);
		inline int GetAttributeCount (void) { return m_Attributes.GetCount(); }
		int GetAttributeInteger (const CString &sName);
		int GetAttributeIntegerBounded (const CString &sName, int iMin, int iMax = -1, int iNull = 0);
		ALERROR GetAttributeIntegerList (const CString &sName, CIntArray *pList);
		float GetAttributeFloat (const CString &sName);
		inline int GetContentElementCount (void) { return m_ContentElements.GetCount(); }
		inline CXMLElement *GetContentElement (int iOrdinal) { return (CXMLElement *)m_ContentElements.GetObject(iOrdinal); }
		CXMLElement *GetContentElementByTag (const CString &sTag);
		inline CString GetContentText (int iOrdinal) { return m_ContentText.GetStringValue(iOrdinal); }
		inline const CString &GetTag (void) { return m_sTag; }
		CXMLElement *OrphanCopy (void);

		static CString MakeAttribute (const CString &sText);

	private:
		CString m_sTag;							//	Element tag
		CXMLElement *m_pParent;					//	Parent of this element
		CSymbolTable m_Attributes;				//	Table of CStrings
		CObjectArray m_ContentElements;			//	Array of CXMLElements
		CStringArray m_ContentText;				//	Interleaved content
	};

class CExternalEntityTable : public IXMLParserController
	{
	public:
		CExternalEntityTable (void);
		virtual ~CExternalEntityTable() { }
		void AddTable (CSymbolTable &Table);
		virtual CString ResolveExternalEntity (const CString &sName, bool *retbFound = NULL);

	private:
		CSymbolTable m_Entities;
	};

//	Some utilities

ALERROR CreateXMLElementFromCommandLine (int argc, char *argv[], CXMLElement **retpElement);
ALERROR ParseAttributeIntegerList (const CString &sValue, CIntArray *pList);

#endif

