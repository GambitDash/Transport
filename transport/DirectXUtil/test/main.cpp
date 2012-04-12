#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TextOutputter.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>

#include "SDL.h"

#include "portage.h"
#include "CObject.h"
#include "KernelObjID.h"
#include "CError.h"

#include "Kernel.h"
#include "CGImage.h"

/* Necessary globals for CGImage. */
CGImage *g_screen;

int
main(int, char* [])
{
  const struct SDL_VideoInfo *info;
  struct SDL_PixelFormat fmt;
  kernelInit();

  SDL_Init(SDL_INIT_VIDEO);
  g_screen = new CGImage();
  g_screen->BindSurface(SDL_SetVideoMode(1024, 768, 32, SDL_SWSURFACE));

  info = SDL_GetVideoInfo();
  memcpy(&fmt, info->vfmt, sizeof(struct SDL_PixelFormat));
  fmt.Aloss = 0;
  fmt.Ashift = 24;
  fmt.Amask = 0xFF000000;
  CGImage::DispPixFormat = &fmt;


  // Create the event manager and test controller
  CPPUNIT_NS::TestResult controller;

  // Add a listener that colllects test result
  CPPUNIT_NS::TestResultCollector result;
  controller.addListener( &result );        

  // Add a listener that print dots as test run.
  CPPUNIT_NS::BriefTestProgressListener progress;
  controller.addListener( &progress );      

  // Add the top suite to the test runner
  CPPUNIT_NS::TestRunner runner;
  runner.addTest( CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest() );
  runner.run( controller );

  // Print test in a compiler compatible format.
  CPPUNIT_NS::TextOutputter outputter( &result, CPPUNIT_NS::stdCOut() );
  outputter.write(); 

  delete g_screen;
  SDL_Quit();
  kernelCleanUp();

  return result.wasSuccessful() ? 0 : 1;
}

