#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"
#include "SDL_syswm.h"
#include "SDL_image.h"
#include "SDL_gfxPrimitives.h"
#include "SDL_rotozoom.h"

int main()
{
	int w = 800;
	int h = 600;
	int bpp = 24;

	SDL_Surface *screen;
	SDL_Surface *base;
	SDL_Surface *roto;
	SDL_Rect srcrect;
	SDL_Rect dstrect;

	dstrect.x = 0;
	dstrect.y = 0;
	dstrect.w = w;
	dstrect.h = h;

	/* Boilerplate SDL initialization. */
	putenv("SDL_VIDEO_WINDOW_POS");
	putenv("SDL_VIDEO_CENTERED=1");

	if(SDL_Init(SDL_INIT_VIDEO) < 0 || !SDL_GetVideoInfo()) {
		perror("SDL_init");
		return -1;
	}

	/* Create base screen. */
	screen = SDL_SetVideoMode(w, h, bpp, SDL_HWSURFACE | SDL_DOUBLEBUF);

	/* Draw some stuff on the screen. */
	base = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 24, 0xFF0000, 0xFF00, 0xFF, 0);
	boxColor(screen, 0, 0, w, h, 0x0000FFFF);
	boxColor(base, 0, 0, w/8, h/8, 0xFF00FF);
	lineColor(base, 0, 0, w, h, 0xFF0000FF);
	lineColor(base, 1, 0, w, h, 0xFF0000FF);
	lineColor(base, 2, 0, w, h, 0xFF0000FF);
	lineColor(base, 0, 1, w, h, 0xFF0000FF);
	lineColor(base, 0, 2, w, h, 0xFF0000FF);
	boxColor(base, w - w/8, h - h/8, w, h, 0x8800FF);
	printf("%d,%d\n", w/8, h/8);

	/* Rotate 180 degrees - off by 2 pixels. */
	roto = rotozoomSurfaceXY(base, 180, 0.1, 0.1, 0);
	SDL_SaveBMP(roto, "roto.bmp");

	srcrect.x = 0;
	srcrect.y = 0;
	srcrect.w = roto->w;
	srcrect.h = roto->h;
	SDL_BlitSurface(roto, &srcrect, screen, &dstrect);
	dstrect.y += roto->h;
	SDL_FreeSurface(roto);

	/* Rotate 1 degrees - off by 1-ish pixels. */
	roto = rotozoomSurfaceXY(base, 1, 0.1, 0.1, 0);

	srcrect.x = 0;
	srcrect.y = 0;
	srcrect.w = roto->w;
	srcrect.h = roto->h;
	SDL_BlitSurface(roto, &srcrect, screen, &dstrect);
	dstrect.y += roto->h;
	SDL_FreeSurface(roto);


	/* For reference, zoom without rotation - no pixel offsets. */
	roto = rotozoomSurfaceXY(base, 0, 0.1, 0.1, 0);
	srcrect.x = 0;
	srcrect.y = 0;
	srcrect.w = roto->w;
	srcrect.h = roto->h;
	SDL_BlitSurface(roto, &srcrect, screen, &dstrect);
	SDL_FreeSurface(roto);

	SDL_SaveBMP(screen, "output.bmp");

	SDL_Quit();

	return 0;
}
