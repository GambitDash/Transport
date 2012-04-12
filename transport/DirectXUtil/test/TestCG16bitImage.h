
#ifndef INCL_TESTCG16BITIMAGE
#define INCL_TESTCG16BITIMAGE

#include <cppunit/extensions/HelperMacros.h>

class TestCG16bitImage : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( TestCG16bitImage );
     CPPUNIT_TEST( create_and_delete );
     CPPUNIT_TEST( create_blank );
     CPPUNIT_TEST( load_image );
     CPPUNIT_TEST( load_and_saveimage );
     CPPUNIT_TEST( load_copy_save );
     CPPUNIT_TEST( load_display );
     CPPUNIT_TEST( img_mask );
     CPPUNIT_TEST( img_mask_scale );
     CPPUNIT_TEST( draw_primitives );
     CPPUNIT_TEST( draw_glow_img );
     CPPUNIT_TEST( intersect_test );
     CPPUNIT_TEST( roto_zoom_img );
	 CPPUNIT_TEST( set_trans_color );
  	 CPPUNIT_TEST( circle_gradient );
	 CPPUNIT_TEST( draw_mask );
	 CPPUNIT_TEST( circle_blt );
  	 CPPUNIT_TEST( bicolor_line );
  CPPUNIT_TEST_SUITE_END();
protected:
  void bicolor_line();
  void circle_blt();
  void draw_mask();
  void circle_gradient();
  void set_trans_color();
  void intersect_test();
  void roto_zoom_img();
  void draw_glow_img();
  void draw_primitives();
  void img_mask();
  void img_mask_scale();
  void load_display();
  void load_copy_save();
  void create_and_delete();
  void create_blank();
  void load_image();
  void load_and_saveimage();

public:
  void setUp();
  void tearDown();

};


#endif
