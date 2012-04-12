#ifndef INCL_CERROR
#define INCL_CERROR

#include <assert.h>

//	Error definitions
typedef DWORD ALERROR;

#ifndef NOERROR
#define NOERROR									0
#endif

#define ERR_FAIL								1	//	Generic failure
#define ERR_MEMORY								2	//	Out of memory
#define ERR_ENDOFFILE							3	//	Read past end of file
#define ERR_CANCEL								4	//	User cancelled operation
#define ERR_NOTFOUND							5	//	Entry not found
#define ERR_FILEOPEN							6	//	Unable to open file
#define ERR_CLASSNOTFOUND						7	//	Constructor for class not found

#define ERR_MODULE						0x00010000	//	First module error message (see GlobalErr.h)
#define ERR_APPL						0x01000000	//	First application error message
#define ERR_FLAG_DISPLAYED				0x80000000	//	Error message already displayed

inline BOOL ErrorWasDisplayed (ALERROR error) { return (error & ERR_FLAG_DISPLAYED) ? TRUE : FALSE; }
inline ALERROR ErrorSetDisplayed (ALERROR error) { return error | ERR_FLAG_DISPLAYED; }
inline ALERROR ErrorCode (ALERROR error) { return error & ~ERR_FLAG_DISPLAYED; }

//	Define ASSERT macro, if necessary
#ifndef ASSERT
#ifdef WIN32
#ifdef _DEBUG
#define ASSERT(exp)						\
			{							\
			if (!(exp))					\
				DebugBreak();			\
			}
#else
#define ASSERT(exp)
#endif
#else
#define ASSERT(exp) assert(exp)
#endif
#endif

#define INLINE_DECREF				TRUE


#endif

