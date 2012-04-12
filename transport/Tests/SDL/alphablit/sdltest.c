#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"
#include "SDL_syswm.h"
#include "SDL_image.h"
#include "SDL_gfxPrimitives.h"

void print_surface(SDL_Surface *screen)
{
	printf("%dx%d@%dbpp %s", screen->w, screen->h,
			screen->format->BitsPerPixel, screen->flags & SDL_HWSURFACE ?
				"hardware" : "software");
	if (screen->format->Amask) printf(" +alpha");
	if (screen->flags & SDL_SRCALPHA) printf(" +srcalpha");
	if (screen->flags & SDL_SRCCOLORKEY) printf(" +srccolorkey");
	printf("\n");
}


int main()
{
	int w = 1024;
	int h = 768;
	int bpp = 32;

	int ticks;
	int frame_delay = 0;
	int blit_delay = 0;
	int frames = 0;
	SDL_Surface *screen;
	SDL_Surface *input;
	SDL_Surface *img;

	putenv("SDL_VIDEO_WINDOW_POS");
	putenv("SDL_VIDEO_CENTERED=1");

	if(SDL_Init(SDL_INIT_VIDEO) < 0 || !SDL_GetVideoInfo()) {
		error("SDL_init");
		return -1;
	}

	/* Create base screen. */
	screen = SDL_SetVideoMode(w, h, bpp, SDL_HWSURFACE | SDL_DOUBLEBUF);
	input = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, bpp, 0xFF0000, 0xFF00, 0xFF, 0xFF000000);

	printf("Screen: "); print_surface(screen);
	printf("Input: "); print_surface(input);
//	SDL_SetColorKey(input, SDL_SRCCOLORKEY, SDL_MapRGB(input->format, 0, 0, 0));
	input = SDL_DisplayFormatAlpha(input);
	boxColor(screen, 0, 0, w/2, h/2, 0x00ff00ff);
	lineColor(input, 0, 0, w, h, 0xff0000ff);

	while (1) {
		if (!(frames % 10)) {
			printf("%d - Blit: %d Load: %d\n", frames, frame_delay / 10, blit_delay / 10);
			fflush(stdout);
			frame_delay = 0;
			blit_delay = 0;
		}
		if (frames == 100)
			break;
		ticks = SDL_GetTicks();

		SDL_BlitSurface(input, NULL, screen, NULL);
		//SDL_BlitSurface(img, NULL, screen, NULL);

		blit_delay += SDL_GetTicks() - ticks;
		ticks = SDL_GetTicks();

		SDL_Flip(screen);
		//SDL_UpdateRect(screen, 0, 0, 0, 0);

		frame_delay += SDL_GetTicks() - ticks;
		frames++;
	}
	SDL_Quit();
}
