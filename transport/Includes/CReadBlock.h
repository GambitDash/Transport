#ifndef INCL_CREADBLOCK
#define INCL_CREADBLOCK

//	Memory Blocks
class IReadBlock
	{
	public:
		virtual ALERROR Close (void) = 0;
		virtual ALERROR Open (void) = 0;
		virtual int GetLength (void) = 0;
		virtual char *GetPointer (int iOffset, int iLength) = 0;

		virtual ~IReadBlock() { }
	};

struct SDL_RWops;

class CFileReadBlock : public CObject, public IReadBlock
	{
	public:
		CFileReadBlock (void);
		CFileReadBlock (const CString &sFilename);
		virtual ~CFileReadBlock (void);

		//	IReadBlock virtuals

		virtual ALERROR Close (void);
		virtual ALERROR Open (void);
		virtual int GetLength (void) { return (int)m_dwFileSize; }
		virtual char *GetPointer (int iOffset, int iLength) { return m_pFile + iOffset; }

	private:
		CString m_sFilename;
		char *m_pFile;
		DWORD m_hFile;
		DWORD m_dwFileSize;
	};

class CResourceReadBlock : public CObject, public IReadBlock
	{
	public:
		CResourceReadBlock (void);
		CResourceReadBlock (char *pszRes);
		virtual ~CResourceReadBlock (void);

		//	IReadBlock virtuals

		virtual ALERROR Close (void);
		virtual ALERROR Open (void);
		virtual int GetLength (void) { return m_dwLength; }
		virtual char *GetPointer (int iOffset, int iLength) { return m_pData + iOffset; }

	private:
		char *m_pszRes;

		char *m_pData;
		DWORD m_dwLength;
	};

class CBufferReadBlock : public CObject, public IReadBlock
	{
	public:
		CBufferReadBlock (void) : CObject(NULL) { }
		CBufferReadBlock (const CString &sData) : CObject(NULL), m_sData(sData) { }

		//	IReadBlock virtuals

		virtual ALERROR Close (void) { return NOERROR; }
		virtual ALERROR Open (void) { return NOERROR; }
		virtual int GetLength (void) { return m_sData.GetLength(); }
		virtual char *GetPointer (int iOffset, int iLength) { return m_sData.GetPointer() + iOffset; }

	private:
		CString m_sData;
	};


#endif

