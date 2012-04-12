//	CDamageSource.cpp
//
//	CDamageSource class

#include "PreComp.h"

CString CDamageSource::GetDamageCauseNounPhrase (DWORD dwFlags)

//	GetDamageCauseNounPhrase
//
//	Returns the name of the damage source

	{
	if (!m_sSourceName.IsBlank())
		return ::ComposeNounPhrase(m_sSourceName, 1, NULL_STR, m_dwSourceFlags, dwFlags);
	else
		return CONSTLIT("damage");
	}

void CDamageSource::OnObjDestroyed (CSpaceObject *pObjDestroyed)

//	OnObjDestroyed
//
//	An object was destroyed

	{
	if (pObjDestroyed == m_pSource)
		{
		m_sSourceName = m_pSource->GetName(&m_dwSourceFlags);
		m_pSource = NULL;
		}

	if (pObjDestroyed == m_pSecondarySource)
		m_pSecondarySource = NULL;
	}

void CDamageSource::ReadFromStream (SLoadCtx &Ctx)

//	ReadFromStream
//
//	Read data from a stream
//
//	DWORD		m_pSource
//	CString		m_sSourceName
//	DWORD		m_dwSourceFlags
//	DWORD		m_pSecondarySource

	{
	Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_pSource);
	if (Ctx.dwVersion >= 30)
		{
		m_sSourceName.ReadFromStream(Ctx.pStream);
		Ctx.pStream->Read((char *)&m_dwSourceFlags, sizeof(DWORD));
		}

	if (Ctx.dwVersion >= 32)
		Ctx.pSystem->ReadObjRefFromStream(Ctx, &m_pSecondarySource);
	}

void CDamageSource::WriteToStream (CSystem *pSystem, IWriteStream *pStream)

//	WriteToStream
//
//	Write data to a stream
//
//	DWORD		m_pSource
//	CString		m_sSourceName
//	DWORD		m_dwSourceFlags
//	DWORD		m_pSecondarySource

	{
	pSystem->WriteObjRefToStream(m_pSource, pStream);
	m_sSourceName.WriteToStream(pStream);
	pStream->Write((char *)&m_dwSourceFlags, sizeof(DWORD));
	pSystem->WriteObjRefToStream(m_pSecondarySource, pStream);
	}


