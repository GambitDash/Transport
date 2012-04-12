#ifndef INCL_CSTRINGARRAY
#define INCL_CSTRINGARRAY

#include "CArray.h"

class CArchiver;
class CUnarchiver;

//	CStringArray. Implements a dynamic array of strings

class CStringArray : public CObject
	{
	public:
		CStringArray (void);
		virtual ~CStringArray (void);

		CStringArray (const CStringArray &Source);
		CStringArray &operator= (const CStringArray &Source);

		inline ALERROR AppendString (const CString &sString, int *retiIndex = NULL)
			{ return InsertString(sString, -1, retiIndex); }
		inline CString &GetStringRef (int iPos) const { return *GetString(iPos); }
		inline CString GetStringValue (int iPos) const { return *GetString(iPos); }
		ALERROR InsertString (const CString &sString, int iPos, int *retiIndex);
		inline ALERROR SetStringValue (int iPos, const CString &sString)
			{ *GetString(iPos) = sString; return NOERROR; }

		int FindString (const CString &sString);
		int GetCount (void) const;
		ALERROR RemoveString (int iPos);
		void RemoveAll (void);

	protected:
		virtual void CopyHandler (CObject *pOriginal);
		virtual ALERROR LoadDoneHandler (void);
		virtual ALERROR LoadHandler (CUnarchiver *pUnarchiver);
		virtual ALERROR SaveHandler (CArchiver *pArchiver);

	private:
		CString *GetString (int iIndex) const;

		CINTDynamicArray m_Array;
	};

#endif

