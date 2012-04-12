#include <cppunit/config/SourcePrefix.h>
#include "TestCDataFile.h"

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"

#include "CFile.h"

#define OUT_DF_1 "out_df_1.bin"

CPPUNIT_TEST_SUITE_REGISTRATION( TestCDataFile );

void TestCDataFile::create()
{
	char buf[1024];
	memset(buf, 0xfe, 1024);
	const int datasz = 50;

	int i;
	CString fn = OUT_DF_1;

	CPPUNIT_ASSERT(CDataFile::Create(fn, 64, 5) == NOERROR);

	{
		CDataFile f(fn);
		CPPUNIT_ASSERT(f.Open() == NOERROR);
		CPPUNIT_ASSERT(f.IsOpen() == TRUE);

		CString data = CString(buf, datasz);

		CPPUNIT_ASSERT(f.AddEntry(data, &i) == NOERROR);
		CPPUNIT_ASSERT(i == 1);
		f.SetDefaultEntry(i); 
		CPPUNIT_ASSERT(f.Flush() == NOERROR);
		CPPUNIT_ASSERT(f.Close() == NOERROR);
	}
	{
		CDataFile f(fn);
		CPPUNIT_ASSERT(f.Open() == NOERROR);
		CPPUNIT_ASSERT(f.IsOpen() == TRUE);
		CPPUNIT_ASSERT(f.GetDefaultEntry() == 1);

		CString data;

		CPPUNIT_ASSERT(f.ReadEntry(i, &data) == NOERROR);
		CPPUNIT_ASSERT(data.GetLength() == datasz);
		CPPUNIT_ASSERT(memcmp(data.GetPointer(), buf, datasz) == 0);
		CPPUNIT_ASSERT(f.Close() == NOERROR);
	}
}


void TestCDataFile::setUp()
{
}

