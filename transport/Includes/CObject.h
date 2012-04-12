#ifndef INCL_COBJECT
#define INCL_COBJECT

#include "CError.h"

class CArchiver;
class CUnarchiver;

//	Object class ID definitions

typedef DWORD OBJCLASSID;

#define OBJCLASS_MODULE_MASK					0xFFF00000
#define OBJCLASS_MODULE_SHIFT					20

#define OBJCLASS_MODULE_KERNEL					0
#define OBJCLASS_MODULE_APPLICATION				1
#define OBJCLASS_MODULE_GLOBAL					2
#define OBJCLASS_MODULE_COUNT					3

inline OBJCLASSID MakeOBJCLASSIDExt (int iModule, int iID) { return (((DWORD)iModule) << OBJCLASS_MODULE_SHIFT) + (DWORD)iID; }
inline OBJCLASSID MakeOBJCLASSID (int iID) { return MakeOBJCLASSIDExt(OBJCLASS_MODULE_APPLICATION, iID); }
inline OBJCLASSID MakeGlobalOBJCLASSID (int iID) { return MakeOBJCLASSIDExt(OBJCLASS_MODULE_GLOBAL, iID); }
inline int OBJCLASSIDGetID (OBJCLASSID ObjID) { return (int)(ObjID & ~OBJCLASS_MODULE_MASK); }
inline int OBJCLASSIDGetModule (OBJCLASSID ObjID) { return (int)((ObjID & OBJCLASS_MODULE_MASK) >> OBJCLASS_MODULE_SHIFT); }

//	Object data description

#define DATADESC_OPCODE_STOP				0	//	No more entries
#define DATADESC_OPCODE_INT					1	//	32-bit integer (iCount valid)
#define DATADESC_OPCODE_REFERENCE			2	//	Reference to memory location or object (iCount valid)
#define DATADESC_OPCODE_ALLOC_OBJ			3	//	Pointer to owned object (derrived from CObject)
#define DATADESC_OPCODE_EMBED_OBJ			4	//	Embedded object (derrived from CObject)
#define DATADESC_OPCODE_ZERO				5	//	32-bit of zero-init data (iCount valid)
#define DATADESC_OPCODE_VTABLE				6	//	This is a vtable (which is initialized by new) (iCount is valid)
#define DATADESC_OPCODE_ALLOC_SIZE32		7	//	Number of 32-bit words allocated in the following memory block
#define DATADESC_OPCODE_ALLOC_MEMORY		8	//	Block of memory; previous must be ALLOC_SIZE

#define DATADESC_FLAG_CUSTOM		0x00000001	//	Object handles saving this part

class CObject;
class CUnarchiver;
class CArchiver;
class CString;

typedef struct
	{
	int iOpCode:8;								//	Op-code
	int iCount:8;								//	Count
	DWORD dwFlags:16;							//	Miscellaneous flags
	} DATADESCSTRUCT, *PDATADESCSTRUCT;

//	Abstract object class

class IObjectClass
	{
	public:
		IObjectClass (OBJCLASSID ObjID, PDATADESCSTRUCT pDataDesc) : m_ObjID(ObjID), m_pDataDesc(pDataDesc) { }
		virtual ~IObjectClass() { }

		inline PDATADESCSTRUCT GetDataDesc (void) { return m_pDataDesc; }
		inline OBJCLASSID GetObjID (void) { return m_ObjID; }
		virtual CObject *Instantiate (void) = 0;
		virtual CObject *Clone(CObject *) = 0;
		virtual int GetObjSize (void) = 0;

	private:
		OBJCLASSID m_ObjID;
		PDATADESCSTRUCT m_pDataDesc;
	};

//	Base object class

class CObject
	{
	public:
		CObject (IObjectClass *pClass);
		virtual ~CObject ();

		CObject *Copy (void);
		inline IObjectClass *GetClass (void) { return m_pClass; }
		static bool IsValidPointer (CObject *pObj);
		ALERROR Load (CUnarchiver *pUnarchiver);
		ALERROR LoadDone (void);
		ALERROR Save (CArchiver *pArchiver);

		static ALERROR Flatten (CObject *pObject, CString *retsData);
		static ALERROR Unflatten (CString sData, CObject **retpObject);

	protected:
		virtual void CopyHandler (CObject *) { }
		virtual ALERROR LoadCustom (CUnarchiver *, BYTE *) { return NOERROR; }
		virtual ALERROR LoadDoneHandler (void) { return NOERROR; }
		virtual ALERROR LoadHandler (CUnarchiver *pUnarchiver);
		virtual LPVOID MemAlloc (int iSize) { return new char[iSize]; }
		virtual void MemFree (LPVOID pMem) { delete []((char *)pMem); }
		virtual ALERROR SaveCustom (CArchiver *, BYTE *) { return NOERROR; }
		virtual ALERROR SaveHandler (CArchiver *pArchiver);

	private:
		CObject *Clone (void) { return m_pClass->Instantiate(); }
		BOOL CopyData (PDATADESCSTRUCT pPos, BYTE **iopSource, BYTE **iopDest);
		PDATADESCSTRUCT DataDescNext (PDATADESCSTRUCT pPos);
		PDATADESCSTRUCT DataDescStart (void);
		BYTE *DataStart (void);
		void VerifyDataDesc (void);

		IObjectClass *m_pClass;
	};

//	Factory for creating objects

class CObjectClassFactory
	{
	public:
		static CObject *Create (OBJCLASSID ObjID);
		static IObjectClass *GetClass (OBJCLASSID ObjID);
		static void NewClass (IObjectClass *pClass);
	};

//	Template for object classes

template <class T>
class CObjectClass : public IObjectClass
	{
	public:
		CObjectClass (OBJCLASSID ObjID, PDATADESCSTRUCT pDataDesc)
				: IObjectClass(ObjID, pDataDesc)
				{ CObjectClassFactory::NewClass(this); }

		virtual ~CObjectClass() { }
 
		virtual CObject *Instantiate (void) { return new T; }
		virtual CObject *Clone(CObject *t) { return new T(*(T *)t); }
		virtual int GetObjSize (void) { return sizeof(T); }
	};


#endif

