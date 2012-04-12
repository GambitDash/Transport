#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"
#include "SDL_syswm.h"
#include "SDL_image.h"
#include "SDL_gfxPrimitives.h"

void BlitTest(SDL_Surface *surface)
{
	int ticks;
	int frame_delay = 0;
	int blit_delay = 0;
	int draw_delay = 0;
	int frames = 0;
	while (1) {
		if (!(frames % 10)) {
			printf("%d - Frame: %d Blit: %d\n", frames, frame_delay / 10, blit_delay / 10);
			fflush(stdout);
			frame_delay = 0;
			blit_delay = 0;
			if (frames == 200) exit(0);
		}
		ticks = SDL_GetTicks();

//		int x;
//		for (x = 0; x < 500; x++) {
//			boxRGBA(surface, 0, 0, 1024, 768, 0xff, 0, 0, 0xff);
//		}
//		SDL_BlitSurface(input, NULL, screen, NULL);
		//SDL_BlitSurface(img, NULL, screen, NULL);

		blit_delay += SDL_GetTicks() - ticks;
		ticks = SDL_GetTicks();

		SDL_Flip(surface);
		//SDL_UpdateRect(screen, 0, 0, 0, 0);

		frame_delay += SDL_GetTicks() - ticks;
		frames++;
	}
}

int main()
{
	int w = 1024;
	int h = 768;
	int bpp = 24;

	int ticks;
	int frame_delay = 0;
	int blit_delay = 0;
	int draw_delay = 0;
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
	input = SDL_DisplayFormatAlpha(SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, 0, 0, 0, 0));
	//img = SDL_DisplayFormatAlpha(IMG_Load("Missiles.jpg"));
	printf("Created video screen %dx%d@%dbpp in %s mode.\n", screen->w, screen->h, screen->format->BitsPerPixel, screen->flags & SDL_HWSURFACE ? "hardware" : "software");
	BlitTest(screen);

	boxRGBA(input, 0, 0, 600, 600, 0xff, 0, 0, 0xf0);

	while (1) {
		if (!(frames % 10)) {
			printf("%d - Frame: %d Blit: %d\n", frames, frame_delay / 10, blit_delay / 10);
			fflush(stdout);
			frame_delay = 0;
			blit_delay = 0;
			if (frames == 300) exit(0);
		}
		ticks = SDL_GetTicks();

		int x;
//		for (x = 0; x < 500; x++) {
			boxRGBA(screen, 0, 0, w, h, 0xff, 0, 0, 0xff);
//		}
//		SDL_BlitSurface(input, NULL, screen, NULL);
		//SDL_BlitSurface(img, NULL, screen, NULL);

		blit_delay += SDL_GetTicks() - ticks;
		ticks = SDL_GetTicks();

		SDL_Flip(screen);
		//SDL_UpdateRect(screen, 0, 0, 0, 0);

		frame_delay += SDL_GetTicks() - ticks;
		frames++;
	}
}
