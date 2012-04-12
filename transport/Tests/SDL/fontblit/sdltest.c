#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"
#include "SDL_syswm.h"
#include "SDL_image.h"
#include "SDL_gfxPrimitives.h"
#include "SDL_ttf.h"

int main()
{
	int w = 1024;
	int h = 768;
	int bpp = 32;

	int ticks;
	int frame_delay = 0;
	int blit_delay = 0;
	int ttf_delay = 0;
	int frames = 0;
	SDL_Surface *screen;
	SDL_Surface *input;
	SDL_Surface *img;
	TTF_Font *font;
	SDL_Color black = {0, 0, 0, 0};
	SDL_Color fg = { 0xff, 0x40, 0xcd, 0 };

	int i;

	putenv("SDL_VIDEO_WINDOW_POS");
	putenv("SDL_VIDEO_CENTERED=1");

	if(SDL_Init(SDL_INIT_VIDEO) < 0 || !SDL_GetVideoInfo()) {
		error("SDL_init");
		return -1;
	}
	if (TTF_Init() == -1) {
		error("TTF_init");
		return -1;
	}

	/* Create base screen. */
	screen = SDL_SetVideoMode(w, h, bpp, SDL_HWSURFACE | SDL_DOUBLEBUF);

	/* Fill it with Green. */
	boxColor(screen, 0, 0, w, h, 0x00ff00ff);

	/* Render some text. */
	font = TTF_OpenFont("test.ttf", 50);
	if (!font) {
		printf("TTF_OpenFont: %s\n", TTF_GetError());
		exit(0);
	}
	input = TTF_RenderText_Blended(font, "hello world", fg); //, black);
	printf("Font renders to: %ux%u %ubpp\n", input->w, input->h, input->format->BitsPerPixel);

	//SDL_SetColorKey(input, SDL_SRCCOLORKEY, SDL_MapRGB(input->format, 0, 0, 0));
	// input = SDL_DisplayFormatAlpha(input);

	printf("Screen %dx%d@%d\n", w, h, bpp);

	while (1) {
		if (!(frames % 10)) {
			printf("%d - TTF: %d Blit: %d Load: %d\n", frames, ttf_delay / 10, frame_delay / 10, blit_delay / 10);
			fflush(stdout);
			frame_delay = 0;
			blit_delay = 0;
			ttf_delay = 0;
		}
		if (frames == 100)
			break;
		ticks = SDL_GetTicks();

		for (i = 0; i < 100; i++) {
			SDL_FreeSurface(input);
			input = TTF_RenderText_Solid(font, "hello world", fg); //, black);
		}
		ttf_delay += SDL_GetTicks() - ticks;
		ticks = SDL_GetTicks();

		SDL_BlitSurface(input, NULL, screen, NULL);
		//SDL_BlitSurface(img, NULL, screen, NULL);

		blit_delay += SDL_GetTicks() - ticks;
		ticks = SDL_GetTicks();

		SDL_Flip(screen);
		//SDL_UpdateRect(screen, 0, 0, 0, 0);

		frame_delay += SDL_GetTicks() - ticks;
		if (frames == 0) {
			SDL_SaveBMP(screen, "output.bmp");
		}
		frames++;
	}
	SDL_Quit();
}
