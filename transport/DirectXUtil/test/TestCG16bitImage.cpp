#include <cppunit/config/SourcePrefix.h>
#include "TestCG16bitImage.h"

#include "SDL.h"
#include "SDL_image.h"

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"

#include "CGImage.h"

#include "TestCommon.h"

#define TEST_IMG_0 "test_img_0.png"
#define TEST_IMG_1 "test_img_1.png"
#define TEST_IMG_2 "test_img_2.jpg"
#define TEST_IMG_3 "test_img_3.jpg"
#define TEST_IMG_4 "test_img_4.png"
#define TEST_IMG_4A "test_img_4a.png"
#define TEST_IMG_5 "test_img_5.jpg"
#define TEST_IMG_5A "test_img_5a.bmp"
#define TEST_IMG_6 "test_img_6.bmp"
#define TEST_IMG_7 "test_img_7.jpg"
#define TEST_IMG_7A "test_img_7a.bmp"
#define TEST_IMG_8 "test_img_8.png"
#define TEST_IMG_9 "test_img_9.png"
#define TEST_IMG_9A "test_img_9a.bmp"

#define OUT_IMG_0  "out_img_0.png"
#define OUT_IMG_1  "out_img_1.png"
#define OUT_IMG_1B "out_img_1b.png"
#define OUT_IMG_1C "out_img_1c.png"
#define OUT_IMG_2  "out_img_2.png"
#define OUT_IMG_3  "out_img_3.png"
#define OUT_IMG_4  "out_img_4.png"
#define OUT_IMG_5  "out_img_5.png"
#define OUT_IMG_5B "out_img_5b.png"
#define OUT_IMG_5C "out_img_5c.png"
#define OUT_IMG_5D "out_img_5d.png"
#define OUT_IMG_6  "out_img_6.png"
#define OUT_IMG_7  "out_img_7.png"
#define OUT_IMG_7B  "out_img_7b.png"
#define OUT_IMG_8  "out_img_8.png"
#define OUT_IMG_9  "out_img_9.png"
#define OUT_IMG_10  "out_img_10.png"
#define OUT_IMG_11  "out_img_11.png"
#define OUT_IMG_12  "out_img_12.png"
#define OUT_IMG_13  "out_img_13.png"
#define OUT_IMG_14  "out_img_14.png"

CPPUNIT_TEST_SUITE_REGISTRATION( TestCG16bitImage );

void TestCG16bitImage::create_and_delete()
{
	CGImage *img;
	img = new CGImage();
	delete img;
}

void TestCG16bitImage::create_blank()
{
	CGImage *img;
	img = new CGImage();
	img->CreateBlank(1, 1);
	CPPUNIT_ASSERT(!img->IsEmpty());
	delete img;
}

void TestCG16bitImage::load_image()
{
	ALERROR error;

	CGImage *img;
	SDL_Surface *src;
	src = IMG_Load(TEST_IMG_0);
	CPPUNIT_ASSERT(src != NULL);

	img = new CGImage();
	CPPUNIT_ASSERT(img != NULL);

	CPPUNIT_ASSERT((error = img->CreateFromSurface(src, NULL)) == NOERROR);

	delete img;
}

void TestCG16bitImage::load_and_saveimage()
{
	ALERROR error;

	CGImage *img;
	SDL_Surface *src;
	src = IMG_Load(TEST_IMG_3);
	CPPUNIT_ASSERT(src != NULL);

	img = new CGImage();
	CPPUNIT_ASSERT(img != NULL);

	error = img->CreateFromSurface(src, NULL);
	CPPUNIT_ASSERT(error == NOERROR);

	SaveImg(img, OUT_IMG_1B);

	img->BltToDC(0, 0);

	CPPUNIT_ASSERT(SaveImg(g_screen, OUT_IMG_1) == 0);
	delete img;

	src = IMG_Load(TEST_IMG_6);
	CPPUNIT_ASSERT(src != NULL);

	img = new CGImage();
	CPPUNIT_ASSERT(img != NULL);

	error = img->CreateFromSurface(src, NULL);
	CPPUNIT_ASSERT(error == NOERROR);

	SaveImg(img, OUT_IMG_1C);
	delete img;
}

void TestCG16bitImage::load_copy_save()
{
	ALERROR error;

	CGImage *img;
	CGImage *dst;
	SDL_Surface *src;

	src = IMG_Load(TEST_IMG_2);
	CPPUNIT_ASSERT(src != NULL);

	img = new CGImage();
	CPPUNIT_ASSERT(img != NULL);

	dst = new CGImage();
	CPPUNIT_ASSERT(dst != NULL);

	error = img->CreateFromSurface(src, NULL);
	CPPUNIT_ASSERT(error == NOERROR);

	CPPUNIT_ASSERT(dst->CreateBlank(img->GetWidth() * 2, img->GetHeight() * 2) == NOERROR);

	dst->SetAlphaChannel(0xFF);
	dst->Blt(0, 0, *img, 0, 0, img->GetWidth(), img->GetHeight());

	CPPUNIT_ASSERT(src != NULL);

	dst->BltToDC(0, 0);

	CPPUNIT_ASSERT(SaveImg(g_screen, OUT_IMG_2) == 0);
	delete img;
	delete dst;
}

void TestCG16bitImage::load_display()
{
	ALERROR error;

	CGImage *img;
	CGImage *screen;
	SDL_Surface *src;

	/* Load the test image. */
	src = IMG_Load(TEST_IMG_0);
	CPPUNIT_ASSERT(src != NULL);

	img = new CGImage();
	CPPUNIT_ASSERT(img != NULL);

	/* Place the image in the CGImage. */
	error = img->CreateFromSurface(src, NULL);
	CPPUNIT_ASSERT(error == NOERROR);

	/* Load a temporary screen. */
	screen = new CGImage();
	CPPUNIT_ASSERT(screen != NULL);
	screen->CreateBlank(1024, 768);
	screen->SetAlphaChannel(0xFF);

	screen->Blt(0, 0, *img, 0, 0, img->GetWidth(), img->GetHeight());
	screen->Blt(img->GetWidth(), 0, *img, 0, 0, img->GetWidth(), img->GetHeight());
	screen->Blt(img->GetWidth() * 2, 0, *img, 0, 0, img->GetWidth(), img->GetHeight());
	screen->Blt(0, img->GetHeight(), *img, 0, 0, img->GetWidth(), img->GetHeight());
	screen->BltToDC(0, 0);

	CPPUNIT_ASSERT(SaveImg(g_screen, OUT_IMG_3) == 0);
	delete img;
	delete screen;
}

void TestCG16bitImage::img_mask()
{
	CGImage *img;
	SDL_Surface *src;
	SDL_Surface *alpha;

	ALERROR err;

	/* Load the test image. */
	src = IMG_Load(TEST_IMG_4);
	CPPUNIT_ASSERT(src != NULL);
	alpha = IMG_Load(TEST_IMG_4A);
	CPPUNIT_ASSERT(alpha != NULL);

	img = new CGImage();
	err = img->CreateFromSurface(src, alpha);
	CPPUNIT_ASSERT(err == NOERROR);

	SDL_FillRect(g_screen->GetSurface(), NULL, 0);
	img->BltToDC(0, 0);
	img->BltToDC(15, 0);
	img->BltToDC(30, 0);
	CPPUNIT_ASSERT(SaveImg(g_screen, OUT_IMG_4) == 0);

	delete img;
}

void TestCG16bitImage::img_mask_scale()
{
	CGImage *img;
	SDL_Surface *src;
	SDL_Surface *alpha;

	ALERROR err;

	/* Load the test image. */
	src = IMG_Load(TEST_IMG_5);
	CPPUNIT_ASSERT(src != NULL);
	alpha = IMG_Load(TEST_IMG_5A);
	CPPUNIT_ASSERT(alpha != NULL);

	img = new CGImage();
	err = img->CreateFromSurface(src, alpha);
	CPPUNIT_ASSERT(err == NOERROR);

	SaveImg(img, OUT_IMG_5B);
	SDL_FillRect(g_screen->GetSurface(), NULL, 0);
	img->BltToDC(0, 0);
	img->BltToDC(5, 0);
	img->BltToDC(10, 0);
	CPPUNIT_ASSERT(SaveImg(g_screen, OUT_IMG_5) == 0);

	delete img;

	/* Load the test image. */
	src = IMG_Load(TEST_IMG_7);
	CPPUNIT_ASSERT(src != NULL);
	alpha = IMG_Load(TEST_IMG_7A);
	CPPUNIT_ASSERT(alpha != NULL);

	img = new CGImage();
	err = img->CreateFromSurface(src, alpha);
	CPPUNIT_ASSERT(err == NOERROR);

	SaveImg(img, OUT_IMG_5C);
	SDL_FillRect(g_screen->GetSurface(), NULL, 0);
	img->BltToDC(0, 0);
	img->BltToDC(5, 0);
	img->BltToDC(10, 0);
	CPPUNIT_ASSERT(SaveImg(g_screen, OUT_IMG_5D) == 0);

	delete img;
}

void TestCG16bitImage::draw_primitives()
{
	CGImage *img;
	/* Load a temporary screen. */
	img = new CGImage();
	CPPUNIT_ASSERT(img != NULL);
	img->CreateBlank(100, 100);

	img->DrawDot(5, 5,  0xFFFFFFFF, CGImage::markerPixel); 
	img->DrawDot(15, 5, 0xFFFF0000, CGImage::markerSmallRound);
	img->DrawDot(25, 5, 0xFF00FF00, CGImage::markerSmallSquare);
	img->DrawDot(35, 5, 0xFF0000FF, CGImage::markerSmallCross); 
	img->DrawDot(45, 5, 0xFFFFFF00, CGImage::markerMediumCross); 
	img->DrawDot(55, 5, 0xFF00FFFF, CGImage::markerSmallFilledSquare); 
	img->DrawLine(0, 10, 60, 20, 3, 0xFFFF00FF);
	img->DrawLine(60, 20, 60, 0, 1, 0xFFCDCD00);
	img->DrawRect(5, 20, 10, 10, 0xFFAA00CD);
	CPPUNIT_ASSERT(SaveImg(img, OUT_IMG_6) == 0);
	delete img;
}

void TestCG16bitImage::draw_glow_img()
{
	CGImage *img, *glow;
	SDL_Surface *src, *alpha;
	ALERROR err;

	img = new CGImage();
	glow = new CGImage();
	CPPUNIT_ASSERT(img != NULL);
	CPPUNIT_ASSERT(glow != NULL);

	img->CreateBlank(100, 100);
	img->SetAlphaChannel(0xFF);
	img->DrawRectFilled(0, 10, 10, 20, 0xFFFF0000);
	img->DrawRectFilled(20, 0, 30, 10, 0xFFFF0000);
	img->DrawRectFilled(40, 40, 50, 50, 0xFFFF0000);

	glow->Create(23, 28);
	glow->DrawRectFilled(0, 0, 23, 28, 0xFF00FF00);
	glow->DrawGlowImage(0, 0, *img, 0, 5, 15, 25, 4);
	img->Blt(0, 1, *glow, 4, 0, glow->GetWidth() - 4, glow->GetHeight());
	glow->Create(28, 23);
	glow->DrawRectFilled(0, 0, 23, 28, 0xFF0000FF);
	glow->DrawGlowImage(0, 0, *img, 15, 0, 35, 15, 4);
	img->Blt(11, 0, *glow, 0, 4, glow->GetWidth(), glow->GetHeight() - 4);
	glow->Create(28, 28);
	glow->DrawRectFilled(0, 0, 28, 28, 0xFF00FFFF);
	glow->DrawGlowImage(0, 0, *img, 35, 35 , 55, 55, 4);
	img->Blt(31, 31, *glow, 0, 0, glow->GetWidth(), glow->GetHeight());
	CPPUNIT_ASSERT(SaveImg(img, OUT_IMG_7) == 0);
	delete img;
	delete glow;

	/* Load the test image. */
	src = IMG_Load(TEST_IMG_4);
	CPPUNIT_ASSERT(src != NULL);
	alpha = IMG_Load(TEST_IMG_4A);
	CPPUNIT_ASSERT(alpha != NULL);

	img = new CGImage();
	err = img->CreateFromSurface(src, alpha);
	CPPUNIT_ASSERT(err == NOERROR);

	glow = new CGImage();
	glow->Create(img->GetWidth() + 10, img->GetHeight() + 10, 8);

	glow->DrawGlowImage(0, 0, *img, 0, 0, img->GetWidth(), img->GetHeight(), 4);
	CPPUNIT_ASSERT(SaveImg(glow, OUT_IMG_7B) == 0);

	delete img;
	delete glow;
}

void TestCG16bitImage::roto_zoom_img()
{
	SDL_Surface *src;
	CGImage *img;
	CGImage *dst;
	ALERROR error;

	src = IMG_Load(TEST_IMG_2);
	CPPUNIT_ASSERT(src != NULL);

	img = new CGImage();
	CPPUNIT_ASSERT(img != NULL);

	error = img->CreateFromSurface(src, NULL);
	CPPUNIT_ASSERT(error == NOERROR);

	SaveImg(img, "out1.png");
	
	dst = new CGImage();
	CPPUNIT_ASSERT(dst != NULL);
	
	dst->Create(200, 200);
	dst->DrawRectFilled(0, 0, dst->GetWidth(), dst->GetHeight(), 0xFF0000FF);
	SaveImg(dst, "out2.png");

	dst->BltRotoZ(0, 0, 50, 25, *img, 0, 0, img->GetWidth(), img->GetHeight(), 0.0, 0);
	dst->BltRotoZ(0, 25, 50, 50, *img, 0, 0, img->GetWidth(), img->GetHeight(), 180.0, 0);
	dst->BltRotoZ(0, 50, 25, 100, *img, 0, 0, img->GetWidth(), img->GetHeight(), 90.0, 0);

	CPPUNIT_ASSERT(SaveImg(dst, OUT_IMG_8) == 0);

	delete img;
	delete dst;
}

void TestCG16bitImage::intersect_test()
{
	bool found = false;
	CGImage *img;
	CGImage *sensor;

	img = new CGImage();
	sensor = new CGImage();

	img->Create(100, 100);
	sensor->Create(20, 20);

	img->DrawRectFilled(40, 40, 60, 60, 0xFF0000FF);
	img->DrawRectFilled(41, 41, 59, 59, 0xFF00FFFF);
	img->DrawRectFilled(42, 42, 58, 58, 0xFFFFFF00);
	img->DrawRectFilled(43, 43, 57, 57, 0xFFFF0000);

	sensor->SetAlphaBlit(false);
	sensor->BltRotoZ(0, 0, 20, 20, *img, 40, 40, 60, 60, 45.0, 0);
	sensor->SetAlphaBlit(true);

	SaveImg(img, "out.png");
	SaveImg(sensor, "out_sensor.png");

	CPPUNIT_ASSERT(img->Intersect(0, 0, img->GetWidth(), img->GetHeight(), *img, 0, 0));

	for (int y = 0; y < img->GetHeight(); y++)
		{
		if (img->Intersect(40, y, 60, y + 20, *sensor, 0, 0))
			{
			img->Blt(40, y, *sensor, 0, 0, sensor->GetWidth(), sensor->GetHeight(), false);
			found = true;
			break;
			}
		}
	CPPUNIT_ASSERT(found);
	found = false;
	for (int y = img->GetHeight(); y >= 0; --y)
		{
		if (img->Intersect(40, y, 60, y + 20, *sensor, 0, 0))
			{
			img->Blt(40, y, *sensor, 0, 0, sensor->GetWidth(), sensor->GetHeight(), false);
			found = true;
			break;
			}
		}
	CPPUNIT_ASSERT(found);
	found = false;
	for (int x = 0; x < img->GetWidth(); x++)
		{
		if (img->Intersect(x, 40, x + 20, 60, *sensor, 0, 0))
			{
			img->Blt(x, 40, *sensor, 0, 0, sensor->GetWidth(), sensor->GetHeight(), false);
			found = true;
			break;
			}
		}
	CPPUNIT_ASSERT(found);
	found = false;
	for (int x = img->GetWidth(); x >= 0; --x)
		{
		if (img->Intersect(x, 40, x + 20, 60, *sensor, 0, 0))
			{
			img->Blt(x, 40, *sensor, 0, 0, sensor->GetWidth(), sensor->GetHeight(), false);
			found = true;
			break;
			}
		}
	CPPUNIT_ASSERT(found);

	CPPUNIT_ASSERT(SaveImg(img, OUT_IMG_9) == 0);
	printf(" : WARNING ignoring left/top extra lines : WARNING looks bogus due to rotozoom issues");
}

void TestCG16bitImage::set_trans_color()
{
	ALERROR error;
	CGImage *img;
	SDL_Surface *src;
	src = IMG_Load(TEST_IMG_8);
	CPPUNIT_ASSERT(src != NULL);

	img = new CGImage();
	CPPUNIT_ASSERT(img != NULL);

	error = img->CreateFromSurface(src, NULL);
	CPPUNIT_ASSERT(error == NOERROR);

	img->SetTransparentColor(0);
	CPPUNIT_ASSERT(SaveImg(img, OUT_IMG_10) == 0);

	delete img;
}

void TestCG16bitImage::circle_gradient()
{
	ALERROR error;
	CGImage *img;

	img = new CGImage();
	CPPUNIT_ASSERT(img != NULL);

	error = img->CreateBlank(256, 256);
	CPPUNIT_ASSERT(error == NOERROR);

	img->DrawRectFilled(0, 0, 150, 150, 0xFF00FF00);
	img->DrawCircleGradient(128, 128, 64, CGImage::RGBColor(0xff, 0, 0));

	CPPUNIT_ASSERT(SaveImg(img, OUT_IMG_11) == 0);
	delete img;
}

void TestCG16bitImage::draw_mask()
{
	CGImage *img, *glow;

	img = new CGImage();
	CPPUNIT_ASSERT(img != NULL);
	glow = new CGImage();
	CPPUNIT_ASSERT(glow != NULL);

	img->CreateBlank(100, 100);
	img->SetAlphaChannel(0xFF);
	img->DrawRectFilled(0, 10, 10, 20, 0xFFFF0000);
	img->DrawRectFilled(20, 0, 30, 10, 0xFFFF0000);
	img->DrawRectFilled(40, 40, 50, 50, 0xFFFF0000);

	glow->Create(23, 28, 8);
	glow->DrawRectFilled(0, 0, 23, 28, 0xFF00FF00);
	glow->DrawGlowImage(0, 0, *img, 0, 5, 15, 25, 4);

	delete img;
	img = new CGImage();
	img->CreateBlank(200, 200);
	img->SetAlphaChannel(0xFF);
	img->DrawMask(0, 0, *glow, 0, 0, glow->GetWidth(), glow->GetHeight(), CGImage::RGBColor(0x00, 0xcd, 0xcd));

	CPPUNIT_ASSERT(SaveImg(img, OUT_IMG_12) == 0);
	delete img;
	delete glow;
}
void TestCG16bitImage::circle_blt()
{
	ALERROR error;
	CGImage *img, *dest;
	SDL_Surface *src, *alpha;
	src = IMG_Load(TEST_IMG_9);
	CPPUNIT_ASSERT(src != NULL);
	alpha = IMG_Load(TEST_IMG_9A);
	CPPUNIT_ASSERT(alpha != NULL);

	img = new CGImage();
	CPPUNIT_ASSERT(img != NULL);

	error = img->CreateFromSurface(src, alpha);
	CPPUNIT_ASSERT(error == NOERROR);

	dest = new CGImage();
	CPPUNIT_ASSERT(dest != NULL);

	error = dest->Create(300, 300);
	dest->SetAlphaChannel(0xFF);
	dest->DrawRectFilled(0, 0, 150, 150, 0xFF0000FF);

	dest->DrawCircleBlt(150, 150, 100, *img, 0, 0, 512, 128, 255);

	CPPUNIT_ASSERT(SaveImg(dest, OUT_IMG_13) == 0);

	delete img;
	delete dest;
}

void TestCG16bitImage::bicolor_line()
{
	ALERROR error;
	CGImage *img;
	img = new CGImage();
	CPPUNIT_ASSERT(img != NULL);
	error = img->Create(300, 300);

    // Horizontal biased lines.
	img->DrawDot(5, 5, 0xffff0000, CGImage::markerSmallCross);
    img->DrawDot(285, 5, 0xff00ff00, CGImage::markerSmallCross);
    img->DrawLineBiColor(0, 10, 300, 11, 1, CGImage::RGBColor(0xff, 0, 0), CGImage::RGBColor(0, 0xFF, 0));
    img->DrawLineBiColor(300, 13, 0, 12, 1, CGImage::RGBColor(0, 0xff, 0), CGImage::RGBColor(0xff, 0, 0));

	img->DrawDot(5, 18, 0xff00ff00, CGImage::markerSmallCross);
    img->DrawDot(285, 18, 0xff0000ff, CGImage::markerSmallCross);
    img->DrawLineBiColor(0, 21, 300, 20, 1, CGImage::RGBColor(0, 0xff, 0), CGImage::RGBColor(0, 0, 0xFF));
    img->DrawLineBiColor(300, 22, 0, 23, 1, CGImage::RGBColor(0, 0, 0xff), CGImage::RGBColor(0, 0xff, 0));

    // Completely horizontal line.
    img->DrawDot(5, 30, 0xff0000ff, CGImage::markerSmallCross);
    img->DrawDot(285, 30, 0xffff0000, CGImage::markerSmallCross);
	img->DrawLineBiColor(0, 40, 300, 40, 1, CGImage::RGBColor(0, 0, 0xff), CGImage::RGBColor(0xff, 0, 0));
	img->DrawLineBiColor(300, 42, 0, 42, 1, CGImage::RGBColor(0xff, 0, 0), CGImage::RGBColor(0, 0, 0xff));

    // Vertically biased lines
	img->DrawDot(25, 5, 0xff0000ff, CGImage::markerSmallCross);
	img->DrawDot(25, 285, 0xffff0000, CGImage::markerSmallCross);
    img->DrawLineBiColor(31, 0, 30, 300, 1, CGImage::RGBColor(0, 0, 0xff), CGImage::RGBColor(0xFF, 0, 0));
    img->DrawLineBiColor(32, 300, 33, 0, 1, CGImage::RGBColor(0xff, 0, 0), CGImage::RGBColor(0, 0, 0xff));

	img->DrawDot(38, 5, 0xffffffff, CGImage::markerSmallCross);
	img->DrawDot(38, 285, 0xff008080, CGImage::markerSmallCross);
    img->DrawLineBiColor(40, 0, 41, 300, 1, CGImage::RGBColor(0xff, 0xff, 0xff), CGImage::RGBColor(0, 0x80, 0x80));
    img->DrawLineBiColor(43, 300, 42, 0, 1, CGImage::RGBColor(0, 0x80, 0x80), CGImage::RGBColor(0xff, 0xff, 0xff));

    // Pure vertical lines
    img->DrawDot(45, 5, 0xffff0000, CGImage::markerSmallCross);
    img->DrawDot(45, 285, 0xff00ff00, CGImage::markerSmallCross);
	img->DrawLineBiColor(50, 0, 50, 300, 1, CGImage::RGBColor(0xff, 0, 0), CGImage::RGBColor(0, 0xff, 0));
	img->DrawLineBiColor(52, 300, 52, 0, 1, CGImage::RGBColor(0, 0xff, 0), CGImage::RGBColor(0xff, 0, 0));

    // Pure Diagonal lines
    img->DrawDot(15, 5, 0xff808000, CGImage::markerSmallCross);
    img->DrawDot(265, 250, 0xff0000ff, CGImage::markerSmallCross);
	img->DrawLineBiColor(5, 0, 255, 250, 1, CGImage::RGBColor(0x80, 0x80, 0), CGImage::RGBColor(0, 0x0, 0xff));
	img->DrawLineBiColor(259, 250, 9, 0, 1, CGImage::RGBColor(0x0, 0x0, 0xff), CGImage::RGBColor(0x80, 0x80, 0x0));

    img->DrawDot(20, 250, 0xffffffff, CGImage::markerSmallCross);
    img->DrawDot(265, 5, 0xffdd8000, CGImage::markerSmallCross);
    img->DrawLineBiColor(5, 250, 255, 0, 1, CGImage::RGBColor(0xff, 0xff, 0xff), CGImage::RGBColor(0xDD, 0x80, 0));
    img->DrawLineBiColor(260, 0, 10, 250, 1, CGImage::RGBColor(0xdd, 0x80, 0x00), CGImage::RGBColor(0xff, 0xff, 0xff));

//	img->DrawLine(0, 14, 300, 13, 1, CGImage::RGBColor(0, 0, 0xff));
//	img->DrawLine(0, 15, 300, 16, 1, CGImage::RGBColor(0xff, 0, 0));

	CPPUNIT_ASSERT(SaveImg(img, OUT_IMG_14) == 0);

	delete img;
}

void TestCG16bitImage::setUp()
{
}

void TestCG16bitImage::tearDown()
{
}
