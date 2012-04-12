//	CCTrue.cpp
//
//	Implements CCTrue class

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"

#include "CStream.h"
#include "CCCodeChain.h"

static CObjectClass<CCTrue>g_Class(OBJID_CCTRUE, NULL);

CCTrue::CCTrue (void) : ICCAtom(&g_Class)

//	CCTrue constructor

	{
	SetNoRefCount();
	}

ICCItem *CCTrue::Clone (CCodeChain *pCC)

//	Clone
//
//	Returns a new item with a single ref-count

	{
	ICCItem *pNew;
	CCTrue *pClone;

	pNew = pCC->CreateTrue();
	if (pNew->IsError())
		return pNew;

	pClone = dynamic_cast<CCTrue *>(pNew);
	pClone->CloneItem(this);

	return pClone;
	}

void CCTrue::DestroyItem (CCodeChain *pCC)

//	DestroyItem
//
//	Destroys the item

	{
	}

ICCItem *CCTrue::StreamItem (CCodeChain *pCC, IWriteStream *pStream)

//	StreamItem
//
//	Stream the sub-class specific data

	{
	return pCC->CreateTrue();
	}

ICCItem *CCTrue::UnstreamItem (CCodeChain *pCC, IReadStream *pStream)

//	UnstreamItem
//
//	Unstream the sub-class specific data

	{
	return pCC->CreateTrue();
	}
