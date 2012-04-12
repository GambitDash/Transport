#ifndef INCL_CLOG
#define INCL_CLOG

struct SDL_RWops;

//	Logging classes
#define ILOG_FLAG_WARNING					0x00000001	//	Warning log entry
#define ILOG_FLAG_ERROR						0x00000002	//	Error log entry
#define ILOG_FLAG_FATAL						0x00000004	//	Fatal error log entry
#define ILOG_FLAG_TIMEDATE					0x00000008	//	Include time date

class ILog
	{
	public:
		virtual ~ILog() { }
		virtual ALERROR Close (void) = 0;
		virtual ALERROR Create (BOOL bAppend) = 0;
		virtual void LogOutput (DWORD dwFlags, const char *pszLine, ...) = 0;
		virtual void Flush (void) { };
	};

class CTextFileLog : public CObject, public ILog
	{
	public:
		CTextFileLog (void);
		CTextFileLog (const CString &sFilename);
		void SetFilename (const CString &sFilename);
		virtual ~CTextFileLog (void);

		//	ILog virtuals

		virtual ALERROR Close (void);
		virtual ALERROR Create (BOOL bAppend);
		virtual void LogOutput (DWORD dwFlags, const char *pszLine, ...);
		virtual void Flush (void);

	private:
		struct SDL_RWops *m_hFile;
		CString m_sFilename;
	};


#endif

