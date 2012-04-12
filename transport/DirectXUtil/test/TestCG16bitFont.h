
#ifndef INCL_TESTCG16BITIMAGE
#define INCL_TESTCG16BITIMAGE

#include <cppunit/extensions/HelperMacros.h>

class TestCG16bitFont : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( TestCG16bitFont );
  CPPUNIT_TEST( create_and_delete );
  CPPUNIT_TEST( load_font );
  CPPUNIT_TEST( write_basic );
  CPPUNIT_TEST( write_paragraph );
  CPPUNIT_TEST( write_alpha );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();
  void create_and_delete();
  void load_font();
  void write_basic();
  void write_paragraph();
  void write_alpha();
};

#endif
