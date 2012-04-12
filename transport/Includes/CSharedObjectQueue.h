#ifndef INCL_CSHAREDOBJECTQUEUE
#define INCL_CSHAREDOBJECTQUEUE

struct SDL_mutex;

#include "CObjectArray.h"

//	CSharedObjectQueue. Implements a thread-safe queue of objects
class CSharedObjectQueue : public CObject
	{
	public:
		CSharedObjectQueue (void);
		virtual ~CSharedObjectQueue (void);

		CObject *DequeueObject (void);
		ALERROR EnqueueObject (CObject *pObj);

	private:
		CObjectArray m_Array;
		int m_iHead;
		int m_iTail;

		struct SDL_mutex *m_csLock;
	};

#endif

