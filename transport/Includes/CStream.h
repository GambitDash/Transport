#ifndef INCL_CSTREAM
#define INCL_CSTREAM

//	File System
class IWriteStream
	{
	public:
		virtual ALERROR Close (void) = 0;
		virtual ALERROR Create (void) = 0;
		virtual ALERROR Write (char *pData, int iLength, int *retiBytesWritten = NULL) = 0;

		virtual ~IWriteStream() { }
	};

class IReadStream
	{
	public:
		virtual ALERROR Close (void) = 0;
		virtual ALERROR Open (void) = 0;
		virtual ALERROR Read (char *pData, int iLength, int *retiBytesRead = NULL) = 0;

		virtual ~IReadStream() { }
	};

//	CMemoryWriteStream. This object is used to write variable length
//	data to a memory block.

class CMemoryWriteStream : public CObject, public IWriteStream
	{
	public:
		CMemoryWriteStream (void);
		CMemoryWriteStream (int iMaxSize);
		virtual ~CMemoryWriteStream (void);

		inline char *GetPointer (void) { return m_pBlock; }
		inline int GetLength (void) { return m_iCurrentSize; }

		//	IWriteStream virtuals

		virtual ALERROR Close (void);
		virtual ALERROR Create (void);
		virtual ALERROR Write (char *pData, int iLength, int *retiBytesWritten = NULL);

	private:
		int m_iMaxSize;
		int m_iCommittedSize;
		int m_iCurrentSize;
		char *m_pBlock;
	};

//	CMemoryReadStream. This object is used to read variable length data

class CMemoryReadStream : public CObject, public IReadStream
	{
	public:
		CMemoryReadStream (void);
		CMemoryReadStream (char *pData, int iDataSize);
		virtual ~CMemoryReadStream (void);

		//	IReadStream virtuals

		virtual ALERROR Close (void) { return NOERROR; }
		virtual ALERROR Open (void) { m_iPos = 0; return NOERROR; }
		virtual ALERROR Read (char *pData, int iLength, int *retiBytesRead = NULL);

	private:
		char *m_pData;
		int m_iDataSize;
		int m_iPos;
	};

//	CFileWriteStream. This object is used to write a file out
struct SDL_RWops;

class CFileWriteStream : public CObject, public IWriteStream
	{
	public:
		CFileWriteStream (void);
		CFileWriteStream (const CString &sFilename, BOOL bUnique);
		virtual ~CFileWriteStream (void);

		//	IWriteStream virtuals

		virtual ALERROR Close (void);
		virtual ALERROR Create (void);
		virtual ALERROR Write (char *pData, int iLength, int *retiBytesWritten = NULL);

	private:
		CString m_sFilename;
		BOOL m_bUnique;
		SDL_RWops *m_hFile;
	};

#if 0
#error XXX Doesnt seem to be used anywhere.
//	CFileReadStream. This object is used to read a file in

class CFileReadStream : public CObject, public IReadStream
	{
	public:
		CFileReadStream (void);
		CFileReadStream (const CString &sFilename);
		virtual ~CFileReadStream (void);

		inline DWORD GetFileSize (void) { return m_dwFileSize; }

		//	IReadStream virtuals

		virtual ALERROR Close (void);
		virtual ALERROR Open (void);
		virtual ALERROR Read (char *pData, int iLength, int *retiBytesRead = NULL);

	private:
		CString m_sFilename;
		SDL_RWops *m_hFile;
		char *m_pFile;
		char *m_pPos;
		DWORD m_dwFileSize;
	};

#endif
#endif

