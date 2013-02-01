#include "ISSOptions.h"
#include "ISSImages.h"

int main(int argc, char *argv[])
{
	ISSOptions options(argc, argv);

	if ( options.getPath() == NULL )
	{
		printf("No path given!\n");
		return -1;
	}

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE);
    Uint32 flags = SDL_SWSURFACE;
#ifdef CHUMBY_COMPILE
		flags |= SDL_FULLSCREEN;
#else
		flags |= SDL_RESIZABLE;
#endif
	SDL_Surface *sdl = NULL;

	sdl = SDL_SetVideoMode(800, 600, 32, flags);

	if ( !sdl )
	{
		fprintf(stderr, "%s\n", SDL_GetError());
		return -1;
	}
	SDL_WM_SetCaption("International Space Station -> ISS -> Infocast SDL Slideshow", "Slideshow");
	SDL_ShowCursor(SDL_DISABLE);

	ISSImages images(sdl, *options.getPath(), options.getRecursive());

	while (true)
	{
		images.loadNextImage();
		images.displayImage();

		sleep(options.getSpeed());
	}

	SDL_Quit();
	return 0;
}
