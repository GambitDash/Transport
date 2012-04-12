#ifndef INCL_CCITEMPOOL
#define INCL_CCITEMPOOL

//	Item pools

template <class ItemClass>
class CCItemPool
	{
	public:
		CCItemPool (void);
		~CCItemPool (void);

		ICCItem *CreateItem (CCodeChain *pCC);
		void DestroyItem (CCodeChain *pCC, ICCItem *pItem);
		inline int GetCount (void) { return m_iCount; }

	private:
		ICCItem *m_pFreeList;
		ItemClass **m_pBackbone;
		int m_iBackboneSize;
		int m_iCount;
	};

#endif

