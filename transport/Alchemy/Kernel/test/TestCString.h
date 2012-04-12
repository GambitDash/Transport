
#ifndef INCL_TESTCSTRING
#define INCL_TESTCSTRING

#include <cppunit/extensions/HelperMacros.h>

class TestCString : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( TestCString );
  CPPUNIT_TEST( create );
  CPPUNIT_TEST_SUITE_END();

protected:

public:
  void setUp();

protected:
  void create();
};


#endif
