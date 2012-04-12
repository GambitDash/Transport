template <class CLASS> 
ALERROR CDesignTypeRef<CLASS>::Bind(SDesignLoadCtx &Ctx)
	{
	if (m_dwUNID)
		{
		CDesignType *pBaseType = g_pUniverse->FindDesignType(m_dwUNID);
		if (pBaseType == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Unknown design type: %x"), m_dwUNID);
			return ERR_FAIL;
			}

		m_pType = CLASS::AsType(pBaseType);
		if (m_pType == NULL)
			{
			Ctx.sError = strPatternSubst(CONSTLIT("Specified type is invalid: %x"), m_dwUNID);
			return ERR_FAIL;
			}
		}

	return NOERROR;
	}

