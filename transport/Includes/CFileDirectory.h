#ifndef INCL_CFILEDIRECTORY
#define INCL_CFILEDIRECTORY

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

//	Directory classes

class CFileDirectory
	{
	public:
		CFileDirectory (const CString &sFilespec);
		~CFileDirectory (void);

		bool HasMore (void);
		CString GetNext (void);

	private:
		CString m_sFilespec;
		boost::filesystem::path m_Path;
		boost::filesystem::directory_iterator m_Itr;
	};

CString pathAddComponent (const CString &sPath, const CString &sComponent);
bool pathCreate (const CString &sPath);
CString pathGetExecutablePath ();
bool pathExists (const CString &sPath);
static inline CString pathSanitize(const CString &sPath)
	{
	std::string s(sPath.GetASCIIZPointer());
	std::replace(s.begin(), s.end(), '\\', '/');
	return CString(s.c_str());
	}

CString pathGetExtension (const CString &sPath);
CString pathGetFilename (const CString &sPath);
CString pathGetPath (const CString &sPath);
CString pathStripExtension (const CString &sPath);
bool pathValidateFilename (const CString &sFilename, CString *retsValidFilename = NULL);

#endif

