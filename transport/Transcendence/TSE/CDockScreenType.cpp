//	CDockScreenType.cpp
//
//	CDockScreenType class

#include "PreComp.h"

CDockScreenType::CDockScreenType (void) : 
		m_pDesc(NULL)

//	CDockScreenType constructor

	{
	}

CDockScreenType::~CDockScreenType (void)

//	CDockScreenType destructor

	{
	if (m_pDesc)
		delete m_pDesc;
	}

ALERROR CDockScreenType::OnCreateFromXML (SDesignLoadCtx &Ctx, CXMLElement *pDesc)

//	OnCreateFromXML
//
//	Create from XML

	{
	m_pDesc = pDesc->OrphanCopy();
	return NOERROR;
	}

//	CDockScreenTypeRef --------------------------------------------------------

ALERROR CDockScreenTypeRef::Bind (CXMLElement *pLocalScreens)
	{
	SDesignLoadCtx Dummy;
	return Bind(Dummy, pLocalScreens);
	}

ALERROR CDockScreenTypeRef::Bind (SDesignLoadCtx &Ctx, CXMLElement *pLocalScreens)
	{
	if (!m_sUNID.IsBlank())
		{
		BOOL bNotANumber;
		DWORD dwUNID = (DWORD)strToInt(m_sUNID, 0, &bNotANumber);

		//	If the UNID is not a number, then we're looking for a local screen

		if (bNotANumber)
			{
			if (pLocalScreens == NULL)
				{
				Ctx.sError = strPatternSubst(CONSTLIT("No local screen found: %s"), m_sUNID.GetASCIIZPointer());
				return ERR_FAIL;
				}

			m_pLocal = pLocalScreens->GetContentElementByTag(m_sUNID);
			if (m_pLocal == NULL)
				{
				Ctx.sError = strPatternSubst(CONSTLIT("No local screen found: %s"), m_sUNID.GetASCIIZPointer());
				return ERR_FAIL;
				}
			}

		//	Otherwise, look for a global screen

		else
			{
			CDesignType *pBaseType = g_pUniverse->FindDesignType(dwUNID);
			if (pBaseType == NULL)
				{
				Ctx.sError = strPatternSubst(CONSTLIT("Unknown design type: %x"), dwUNID);
				return ERR_FAIL;
				}

			m_pType = CDockScreenType::AsType(pBaseType);
			if (m_pType == NULL)
				{
				Ctx.sError = strPatternSubst(CONSTLIT("Dock screen type expected: %x"), dwUNID);
				return ERR_FAIL;
				}
			}
		}

	return NOERROR;
	}

CXMLElement *CDockScreenTypeRef::GetDesc (void) const
	{
	if (m_pLocal)
		return m_pLocal;
	else if (m_pType)
		return m_pType->GetDesc();
	else
		return NULL;
	}

void CDockScreenTypeRef::LoadUNID (SDesignLoadCtx &Ctx, const CString &sUNID)
	{
	if (Ctx.pExtension)
		{
		char *pPos = sUNID.GetASCIIZPointer();
		if (*pPos == '@')
			{
			WORD wLow = LOWORD(Ctx.pExtension->dwUNID) + (WORD)strParseIntOfBase(pPos+1, 16, 0, NULL, NULL);
			m_sUNID = strFromInt(MAKELONG(wLow, HIWORD(Ctx.pExtension->dwUNID)));
			}
		else
			m_sUNID = sUNID;
		}
	else
		m_sUNID = sUNID;
	}

