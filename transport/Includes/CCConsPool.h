#ifndef INCL_CCCONSPOOL
#define INCL_CCCONSPOOL

class CConsPool
	{
	public:
		CConsPool (void);
		~CConsPool (void);

		CCons *CreateCons (void);
		void DestroyCons (CCons *pCons);
		inline int GetCount (void) { return m_iCount; }

	private:
		CCons *m_pFreeList;
		CCons **m_pBackbone;
		int m_iBackboneSize;
		int m_iCount;
	};

#endif

