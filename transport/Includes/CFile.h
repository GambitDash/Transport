#ifndef INCL_CFILE
#define INCL_CFILE

//	CDataFile. This is a file-based collection of variable-sized records.
#define DFOPEN_FLAG_READ_ONLY					0x00000001

struct SDL_RWops;

class CDataFile : public CObject
	{
	public:
		CDataFile (void);
		CDataFile (const CString &sFilename);
		virtual ~CDataFile (void);

		ALERROR AddEntry (const CString &sData, int *retiEntry);
		ALERROR Close (void);
		ALERROR DeleteEntry (int iEntry);
		ALERROR Flush (void);
		inline CString GetFilename (void) { return m_sFilename; }
		int GetDefaultEntry (void);
		int GetEntryLength (int iEntry);
		inline BOOL IsOpen (void) { return m_hFile != NULL; }
		ALERROR Open (DWORD dwFlags = 0);
		ALERROR ReadEntry (int iEntry, CString *retsData);
		ALERROR ReadEntryPartial (int iEntry, int iPos, int iLength, CString *retsData);
		void SetDefaultEntry (int iEntry);
		ALERROR WriteEntry (int iEntry, const CString &sData);

		static ALERROR Create (const CString &sFilename,
							   int iBlockSize,
							   int iInitialEntries);

	private:
		typedef struct
			{
			DWORD dwBlock;								//	Block Number (-1 = unused)
			DWORD dwBlockCount;							//	Number of blocks reserved for entry
			DWORD dwSize;								//	Size of entry
			DWORD dwFlags;								//	Misc flags
			} ENTRYSTRUCT, *PENTRYSTRUCT;

		ALERROR AllocBlockChain (DWORD dwBlockCount, DWORD *retdwStartingBlock);
		ALERROR FreeBlockChain (DWORD dwStartingBlock, DWORD dwBlockCount);
		ALERROR GrowEntryTable (int *retiEntry);
		ALERROR ResizeEntry (int iEntry, DWORD dwSize, DWORD *retdwBlockCount);
		ALERROR WriteBlockChain (DWORD dwStartingBlock, char *pData, DWORD dwSize);

		CString m_sFilename;							//	Filename of data file

		int m_iBlockSize;								//	Size of each block
		int m_iBlockCount;								//	Number of blocks in file
		int m_iDefaultEntry;							//	Default entry

		SDL_RWops *m_hFile;									//	Open file handle

		int m_iEntryTableCount;							//	Number of entries
		PENTRYSTRUCT m_pEntryTable;						//	Entry table

		DWORD m_fHeaderModified:1;						//	TRUE if header has changed
		DWORD m_fEntryTableModified:1;					//	TRUE if entry table has changed
		DWORD m_fFlushing:1;							//	TRUE if we're inside ::Flush
		DWORD m_fReadOnly:1;							//	TRUE if we're open read-only
	};

#endif

