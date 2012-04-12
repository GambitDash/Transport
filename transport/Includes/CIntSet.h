#ifndef INCL_CINTSET
#define INCL_CINTSET

//	CIntSet. Implementation of a set of integers. This implementation
//	can have any number of elements and any range of elements. However,
//	the code is very inefficient if the elements are wildly discontiguous.
class CIntSet : public CObject
	{
	public:
		CIntSet (void);

		ALERROR AddElement (int iElement);
		ALERROR EnumElements (CIntArray *pArray) const;
		bool HasElement (int iElement) const;
		inline void RemoveAll (void) { m_Set.RemoveAll(); }
		void RemoveElement (int iElement);

	private:
		CIDTable m_Set;
	};

#endif

