#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"
#include "SDL_syswm.h"

int main()
{
	int w = 1024;
	int h = 768;
	int bpp = 16;

	int ticks;
	int frame_delay = 0;
	int blit_delay = 0;
	int frames = 0;
	SDL_Surface *screen;
	SDL_Surface *input;

	putenv("SDL_VIDEO_WINDOW_POS");
	putenv("SDL_VIDEO_CENTERED=1");

	if(SDL_Init(SDL_INIT_VIDEO) < 0 || !SDL_GetVideoInfo()) {
		error("SDL_init");
		return -1;
	}

	/* Create base screen. */
	screen = SDL_SetVideoMode(w, h, bpp, SDL_HWSURFACE | SDL_DOUBLEBUF);
	input = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, 0, 0, 0, 0);

	printf("Screen %dx%d@%d\n", w, h, bpp);

	while (1) {
		if (!(frames % 10)) {
			printf("%d - Flip: %d Blit: %d\r", frames, frame_delay / 10, blit_delay / 10);
			fflush(stdout);
			frame_delay = 0;
			blit_delay = 0;
		}
		ticks = SDL_GetTicks();

		SDL_BlitSurface(input, NULL, screen, NULL);
		blit_delay += SDL_GetTicks() - ticks;
		ticks = SDL_GetTicks();

		SDL_Flip(screen);
		//SDL_UpdateRect(screen, 0, 0, 0, 0);

		frame_delay += SDL_GetTicks() - ticks;
		frames++;
	}
}
