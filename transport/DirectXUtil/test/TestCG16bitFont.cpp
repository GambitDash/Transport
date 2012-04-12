#include <cppunit/config/SourcePrefix.h>
#include "TestCG16bitFont.h"

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"
#include "CString.h"

#include "CG16bitImage.h"
#include "CG16bitFont.h"

#include "TestCommon.h"

#define TEST_IMG_0 "test_img_0.png"
#define TEST_IMG_1 "test_img_1.png"
#define TEST_IMG_2 "test_img_2.jpg"
#define OUT_FNT_0  "out_fnt_0.png"
#define OUT_FNT_1  "out_fnt_1.png"
#define OUT_FNT_2  "out_fnt_2.png"
#define OUT_FNT_3  "out_fnt_3.png"

#define TEST_FNT_0 "test_fnt_0.ttf"

#define TEST_STR_0 "abcdefghijklmnopqrstuvwxyz0123456789"
#define TEST_STR_1 "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"

CPPUNIT_TEST_SUITE_REGISTRATION( TestCG16bitFont );

void TestCG16bitFont::create_and_delete()
{
	CG16bitFont *font;
	font = new CG16bitFont();
	delete font;
}

void TestCG16bitFont::load_font()
{
	CG16bitFont *font;
	font = new CG16bitFont();

	CPPUNIT_ASSERT(font->Create(TEST_FNT_0, 12) == NOERROR);

	delete font;
}

void TestCG16bitFont::write_basic()
{
	CG16bitFont *font;
	CG16bitImage *img;
	CString sText(TEST_STR_0);

	font = new CG16bitFont();
	img = new CG16bitImage();
	img->CreateBlank(512, 40);
	img->DrawRectFilled(0, 0, img->GetWidth(), img->GetHeight(), CGImage::RGBAColor(0, 0, 0, 0xFF));

	CPPUNIT_ASSERT(font->Create(TEST_FNT_0, 12) == NOERROR);
	font->DrawText(*img, 0, 0, 0xff0000, sText);

	CPPUNIT_ASSERT(SaveImg(img, OUT_FNT_1) == NOERROR);

	delete img;
	delete font;
}

void TestCG16bitFont::write_paragraph()
{
	CG16bitFont *font;
	CG16bitImage *img;
	CString sText;
	unsigned int i = 1;

	font = new CG16bitFont();
	img = new CG16bitImage();
	img->CreateBlank(512, 512, false);
	img->DrawRectFilled(0, 0, img->GetWidth(), img->GetHeight(), CGImage::RGBAColor(0, 0, 0, 0xFF));

	CPPUNIT_ASSERT(font->Create(TEST_FNT_0, 12) == NOERROR);
	for (int y = 0; y < 512 + font->GetHeight(); y += font->GetHeight()) {
		sText.Transcribe((i % 2) ? TEST_STR_1 : TEST_STR_0, i);
		switch (i % 3) {
		case 0:
			font->DrawText(*img, 0, y, CGImage::RGBColor(0, 0, 0xFF), sText);
			break;
		case 1:
			font->DrawText(*img, 0, y, CGImage::RGBColor(0xFF, 0, 0), sText);
			break;
		case 2:
			font->DrawText(*img, 0, y, CGImage::RGBColor(0, 0xFF, 0), sText);
			break;
		}
		if (++i > strlen(TEST_STR_0)) i = 0;
	}

	CPPUNIT_ASSERT(SaveImg(img, OUT_FNT_2) == NOERROR);

	delete img;
	delete font;
}

void TestCG16bitFont::write_alpha()
{
	CG16bitFont *font;
	CG16bitImage *img;
	CString sText;
	CString sFilename(TEST_IMG_2);
	unsigned int i = 1;

	font = new CG16bitFont();
	img = new CG16bitImage();
	CPPUNIT_ASSERT(img->CreateFromFile(sFilename) == NOERROR);

	CPPUNIT_ASSERT(font->Create(TEST_FNT_0, 12) == NOERROR);
	for (int y = 0; y < img->GetHeight() + font->GetHeight(); y += font->GetHeight()) {
		sText.Transcribe((i % 2) ? TEST_STR_1 : TEST_STR_0, i);
		switch (i % 3) {
		case 0:
			font->DrawText(*img, 0, y, CGImage::RGBColor(0, 0, 0xFF), sText);
			break;
		case 1:
			font->DrawText(*img, 0, y, CGImage::RGBColor(0xFF, 0, 0), sText);
			break;
		case 2:
			font->DrawText(*img, 0, y, CGImage::RGBColor(0, 0xFF, 0), sText);
			break;
		}
		if (++i > strlen(TEST_STR_0)) i = 0;
	}

	CPPUNIT_ASSERT(SaveImg(img, OUT_FNT_3) == NOERROR);

	delete img;
	delete font;
}

void TestCG16bitFont::setUp()
{
	CPPUNIT_ASSERT(TTF_Init() != -1);
}

void TestCG16bitFont::tearDown()
{
	if (TTF_WasInit())
		TTF_Quit();
}

