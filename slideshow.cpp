#include <signal.h>

#include "ISSOptions.h"
#include "ISSImages.h"

static bool running = true;

void handle_quit(int signum)
{
	printf("Quitting!\n");
	running = false;
}

int main(int argc, char *argv[])
{
	ISSOptions options(argc, argv);

	if ( options.getPath() == NULL )
	{
		printf("No path given!\n");
		return -1;
	}

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE);

	if (signal(SIGINT, handle_quit) == SIG_ERR)
	{
		SDL_Quit();
		fprintf(stderr, "Error setting up signal handler!\n");
		return -1;
	}

    Uint32 flags = SDL_SWSURFACE;
	if ( options.getFullscreen() )
		flags |= SDL_FULLSCREEN;
	else
		flags |= SDL_RESIZABLE;
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

	while (running)
	{
		images.loadNextImage();
		images.displayImage();

		sleep(options.getSpeed());
	}

	SDL_Quit();
	return 0;
}
