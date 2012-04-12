#ifndef INCL_CCMISC
#define INCL_CCMISC

//	Misc structures

class CEvalContext
	{
	public:
		CCodeChain *pCC;
		ICCItem *pLexicalSymbols;
		ICCItem *pLocalSymbols;

		LPVOID pExternalCtx;
	};

#endif

