#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>
#include <SDL_image.h>
#include <cairo-sdl.h>

int main()
{
	SDL_Surface *img;
	SDL_Surface *screen;

	putenv("SDL_VIDEO_WINDOW_POS");
	putenv("SDL_VIDEO_CENTERED=1");

	if(SDL_Init(SDL_INIT_VIDEO) < 0 || !SDL_GetVideoInfo()) {
		error("SDL_init");
		return -1;
	}
	/* Create base screen. */
	screen = SDL_SetVideoMode(1024, 768, 32, SDL_SWSURFACE);

	img = IMG_Load("test_img_2.jpg");
	assert(img);

	while (1) {
		SDL_BlitSurface(img, NULL, screen, NULL);
		SDL_Flip(screen);
	}
	
	return 0;
}
