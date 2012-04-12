#ifndef INCL_CVERSION
#define INCL_CVERSION

struct SFileVersionInfo
	{
	CString sProductName;
	CString sProductVersion;
	CString sCompanyName;
	CString sCopyright;

	ULONG64 dwFileVersion;
	ULONG64 dwProductVersion;
	};

DWORD fileGetProductVersion (void);
ALERROR fileGetVersionInfo (const CString &sFilename, SFileVersionInfo *retInfo);

#endif

