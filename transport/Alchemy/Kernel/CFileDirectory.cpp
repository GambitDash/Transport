//	CFileDirectory.cpp
//
//	Implements CFileDirectory object

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"

#include "CFileDirectory.h"

using namespace boost::filesystem;

CFileDirectory::CFileDirectory (const CString &sFilespec) :
		m_sFilespec(sFilespec),
		m_Path(sFilespec.GetASCIIZPointer()),
		m_Itr(m_Path)

//	CFileDirectory constructor

	{
	}

CFileDirectory::~CFileDirectory (void)

//	CFileDirectory destructor

	{
	}

bool CFileDirectory::HasMore (void)

//	HasMore
//
//	Returns TRUE if there are more files in the directory

	{
	directory_iterator end_itr;
	return (m_Itr != end_itr);
	}

CString CFileDirectory::GetNext (void)

//	GetNext
//
//	Returns the next filename

	{

	ASSERT(HasMore());
	CString sFilename(m_Itr->leaf().c_str());

	m_Itr++;

	return sFilename;
	}

