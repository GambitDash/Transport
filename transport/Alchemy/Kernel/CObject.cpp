//	CObject.cpp
//
//	Methods for basic objects and class objects

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"

#include "CArchive.h"

//	For now we hard-code the maximum number of object classes

#define MAX_OBJCLASS_COUNT							1000

//	Array to hold all object classes

typedef IObjectClass *ClassArray[MAX_OBJCLASS_COUNT];
static ClassArray g_Classes[OBJCLASS_MODULE_COUNT];

//	CObject methods

CObject::CObject (IObjectClass *pClass) : m_pClass(pClass)

//	CObject constructor

	{
	}

CObject::~CObject (void)

//	CObject destructor

	{
	}

CObject *CObject::Copy (void)

//	Copy
//
//	Creates a copy of the object and returns it

	{
	return m_pClass->Clone(this);
	}

BOOL CObject::CopyData (PDATADESCSTRUCT pPos, BYTE **iopSource, BYTE **iopDest)

//	CopyData
//
//	Copies the data from source to destination using the given data descriptor
//	table. Returns the updated positions

	{
	assert(0);
	return FALSE;
	}

PDATADESCSTRUCT CObject::DataDescNext (PDATADESCSTRUCT pPos)

//	DataDescNext
//
//	Returns the next data descriptor record. If there are no more, it
//	returns NULL

	{
	assert(0);
	return NULL;
	}

PDATADESCSTRUCT CObject::DataDescStart (void)

//	DataDescStart
//
//	Returns the first data descriptor record

	{
	return GetClass()->GetDataDesc();
	}

BYTE *CObject::DataStart (void)

//	DataStart
//
//	Returns a pointer to the start of member data for this
//	object.

	{
	BYTE *pData = (BYTE *)this;

	//	Skip past the CObject class

	pData += sizeof(CObject);

	return pData;
	}

ALERROR CObject::Load (CUnarchiver *pUnarchiver)

//	Load
//
//	Loads object from an archive

	{
	return LoadHandler(pUnarchiver);
	}

ALERROR CObject::LoadDone (void)

//	LoadDone
//
//	This method is called when an unarchiver is done loading an object.

	{
	/* XXX This never seemed to do anything anyways. */
	return NOERROR;
	}

ALERROR CObject::LoadHandler (CUnarchiver *pUnarchiver)

//	LoadHandler
//
//	Handles loading the actual object data

	{
	/* XXX This needs some kind of implementation! */
	assert(0);
	return NOERROR;
	}

ALERROR CObject::Save (CArchiver *pArchiver)

//	Save
//
//	Saves object to an archive

	{
	ALERROR error;
	DWORD dwID;

	//	Write out the ID of object class

	dwID = m_pClass->GetObjID();
	if (error = pArchiver->WriteData((char *)&dwID, sizeof(DWORD)))
		return error;

	if (error = SaveHandler(pArchiver))
		return error;

	return NOERROR;
	}

ALERROR CObject::SaveHandler (CArchiver *pArchiver)

//	SaveHandler
//
//	Saves the object data

	{
	assert(0);
	return NOERROR;
	}

void CObject::VerifyDataDesc (void)

//	VerifyDataDesc
//
//	Verify that the data desc matches the object
//	ASSERTs if it does not

	{
	}

//	CObjectClassFactory methods

CObject *CObjectClassFactory::Create (OBJCLASSID ObjID)
	{
	IObjectClass *pClass = GetClass(ObjID);
	return pClass->Instantiate();
	}

IObjectClass *CObjectClassFactory::GetClass (OBJCLASSID ObjID)

//	GetClass
//
//	Returns a pointer to the class for the given object

	{
	int iModule = OBJCLASSIDGetModule(ObjID);
	int iID = OBJCLASSIDGetID(ObjID);

	ASSERT(iModule >= 0 && iModule < OBJCLASS_MODULE_COUNT);
	ASSERT(iID >= 0 && iID < MAX_OBJCLASS_COUNT);

	return g_Classes[iModule][iID];
	}

bool CObject::IsValidPointer (CObject *pObj)

//	IsValidPointer
//
//	Returns TRUE if pObj is a valid pointer

	{
	return TRUE;

	/* XXX Dangerous; examine cases, consider shared_ptr; */
#if 0
	MEMORY_BASIC_INFORMATION mi;
	VirtualQuery(pObj, &mi, sizeof(mi));
	return (mi.State == MEM_COMMIT
			&& (mi.AllocationProtect & PAGE_READWRITE));
#endif
	}

void CObjectClassFactory::NewClass (IObjectClass *pClass)
	{
	int iModule = OBJCLASSIDGetModule(pClass->GetObjID());
	int iID = OBJCLASSIDGetID(pClass->GetObjID());

	ASSERT(g_Classes[iModule][iID] == NULL);
	g_Classes[iModule][iID] = pClass;
	}

//	Flatten methods

ALERROR CObject::Flatten (CObject *pObject, CString *retsData)

//	Flatten
//
//	Flattens the object to a memory block. This function should only be
//	used for objects that do not have references to other objects.

	{
	ALERROR error;
	CMemoryWriteStream Stream(10000000);
	CArchiver Archiver(&Stream);

	if (error = Archiver.BeginArchive())
		return error;

	if (error = Archiver.AddObject(pObject))
		return error;

	if (error = Archiver.EndArchive())
		return error;

	//	Store as a string

	*retsData = CString(Stream.GetPointer(), Stream.GetLength());

	return NOERROR;
	}

ALERROR CObject::Unflatten (CString sData, CObject **retpObject)

//	UnflattenObject
//
//	Unflattens an object flattened by FlattenObject

	{
	ALERROR error;
	CMemoryReadStream Stream(sData.GetPointer(), sData.GetLength());
	CUnarchiver Unarchiver(&Stream);

	if (error = Unarchiver.BeginUnarchive())
		return error;

	if (error = Unarchiver.EndUnarchive())
		return error;

	*retpObject = Unarchiver.GetObject(0);
	if (*retpObject == NULL)
		return ERR_FAIL;

	return NOERROR;
	}
