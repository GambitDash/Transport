#include <cppunit/config/SourcePrefix.h>
#include "TestCString.h"

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"

CPPUNIT_TEST_SUITE_REGISTRATION( TestCString );

void TestCString::create()
{
	char *data = "0123456789";
		{
		CString str;
		CPPUNIT_ASSERT(str.IsBlank());
		CPPUNIT_ASSERT_EQUAL(str.GetLength(), 0);
		}
		{
		CString str(data);
		CPPUNIT_ASSERT(!str.IsBlank());
		CPPUNIT_ASSERT(!memcmp(data, str.GetASCIIZPointer(), strlen(data) + 1));
		CPPUNIT_ASSERT_EQUAL((size_t)str.GetLength(), strlen(data));
		}
		{
		const int len = 5;
		CString str(data, len);
		CPPUNIT_ASSERT(!str.IsBlank());
		CPPUNIT_ASSERT(!memcmp(data, str.GetASCIIZPointer(), len));
		CPPUNIT_ASSERT_EQUAL(str.GetLength(), len);
		}
		{
		const int len = 5;
		CString str(data, len, true);
		CPPUNIT_ASSERT(!str.IsBlank());
		CPPUNIT_ASSERT(!memcmp(data, str.GetASCIIZPointer(), len));
		CPPUNIT_ASSERT_EQUAL(str.GetLength(), len);
		}
}


void TestCString::setUp()
{
}

