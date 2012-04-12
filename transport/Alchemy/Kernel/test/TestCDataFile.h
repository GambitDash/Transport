
#ifndef INCL_TESTCDATAFILE
#define INCL_TESTCDATAFILE

#include <cppunit/extensions/HelperMacros.h>

class TestCDataFile : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( TestCDataFile );
  CPPUNIT_TEST( create );
  CPPUNIT_TEST_SUITE_END();

protected:

public:
  void setUp();

protected:
  void create();
};


#endif
