//	Path.cpp
//
//	Path routines

#include <string>
#include <algorithm>

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"

#include "CVersion.h"
#include "CFileDirectory.h"

struct SFileVersionInfo VerInfo;

using namespace boost::filesystem;

#define STR_COMPANY_NAME				CONSTLIT("CompanyName")
#define STR_COPYRIGHT					CONSTLIT("LegalCopyright")
#define STR_PRODUCT_NAME				CONSTLIT("ProductName")
#define STR_PRODUCT_VERSION				CONSTLIT("ProductVersion")

static CString GetVersionString (char *pData, WORD *pLangInfo, const CString &sString);

DWORD fileGetProductVersion (void)

//	fileGetProductVersion
//
//	Returns the version encoded as a DWORD

	{
	fileGetVersionInfo(NULL_STR, &VerInfo);

	return VerInfo.dwProductVersion;
	}

ALERROR fileGetVersionInfo (const CString &sFilename, SFileVersionInfo *retInfo)

//	fileGetVersionInfo
//
//	Returns version information for the file (if sFilename is NULL_STRING then
//	we return information for the current module.)

	{
	retInfo->dwProductVersion = 0x95d00;
	return NOERROR;
	}

CString pathAddComponent (const CString &sPath, const CString &sComponent)

//	pathAddComponent
//
//	Concatenates the given component to the given path and returns
//	the result
//
//	sPath: full pathname to a directory (e.g. "c:\", "\\lawrence\cdrom", "d:\test")
//	sComponent: directory, filename, or wildcard.

	{
	if (sPath.IsBlank())
		return sComponent;
	else if (sComponent.IsBlank())
		return sPath;

	path src(sPath.GetASCIIZPointer());
	path dst(sComponent.GetASCIIZPointer());
	src /= dst;

	return CString(src.string().c_str());
	}

bool pathCreate (const CString &sPath)

//	pathCreate
//
//	Makes sure that the given path exists. Creates all intermediate folders.
//	Returns TRUE if successful.

	{
	path tgt(sPath.GetASCIIZPointer());
	create_directories(tgt);
	return TRUE;
	}

CString pathGetExecutablePath ()

//	pathGetExecutablePath
//
//	Returns the path of the given executable. This is the path
//	(without the filename) of the executable (e.g., c:\bin\windows)

	{
	/* XXX Not yet really implemented. */
	return CString(".");
	}

bool pathExists (const CString &sPath)

//	pathExists
//
//	Returns TRUE if the given path exists

	{
	return exists(path(sPath.GetASCIIZPointer()));
	}

CString pathGetExtension (const CString &sPath)

//	pathGetExtension
//
//	Returns the extension (without dot)

	{
	/* Add one to the value returned to skip the dot. */
	const char *ext = extension(path(sPath.GetASCIIZPointer())).c_str();
	if (ext && *ext == '.')
		return ext + 1;
	return ext;
	}

CString pathGetFilename (const CString &sPath)

//	pathGetFilename
//
//	Returns the filename (without the path)

	{
	return path(sPath.GetASCIIZPointer()).leaf().c_str();
	}

CString pathGetPath (const CString &sPath)

//	pathGetPath
//
//	Returns the path without the filename

	{
	path pth(sPath.GetASCIIZPointer());

	if (is_directory(pth)) return sPath;

	return pth.remove_leaf().string().c_str();
	}

CString pathStripExtension (const CString &sPath)

//	pathStripExtension
//
//	Returns the path without the extension on the filename

	{
	path pth(sPath.GetASCIIZPointer());
	path sth = pth;
	pth.remove_leaf();
	pth /= basename(sth);
	return pth.string().c_str();
	}

bool pathValidateFilename (const CString &sFilename, CString *retsValidFilename)

//	pathValidateFilename
//
//	Return TRUE if the given filename is valid. If an output parameter is also
//	supplied then we return a valid filename.

	{
	return TRUE;
	}

//	HELPERS

CString GetVersionString (char *pData, WORD *pLangInfo, const CString &sString)
	{
	/* XXX Not yet implemented. */
	return CString("notyetimpl");
	}

