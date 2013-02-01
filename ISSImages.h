#ifndef __ISS_IMAGES_H__
#define __ISS_IMAGES_H__

#include <string>
#include <vector>

#include "SDL/SDL.h"

class ISSImages
{
public:
	ISSImages(SDL_Surface *sdl, const std::string directory, bool recursive);
	~ISSImages();

	/*
	 * Incriments images pointed to by iterator and in
	 * SDL_Surfaces.  Takes care of any necessary
	 * scaling, rotation, etc.
	 */
	int loadNextImage();

	/*
	 * Flip currently loaded image to the display.
	 * TODO: Add transitions to the logic.
	 */
	int displayImage();

private:
	/*
	 * Load files from the given directory.
	 */
	int loadFilenames(const std::string directory, bool recursive);

	/*
	 * Rotate the image currently in current_image based on
	 * the rotation/mirror parameters.  If successful puts
	 * the newly rotated image back in current_image
	 */
	int rotateImage(int rotation, bool mirrored);

	/*
	 * Scale the image currently in current_image based on
	 * the WxH parameters.  If successful, it puts the
	 * newly scaled image back in current_image
	 */
	int scaleImage(int w, int h);

	/*
	 * Frame the image to fit the full screen.  This is so
	 * images that don't fit the display aspect ratio aren't
	 * stretched funny, they're pillar-boxed.
	 */
	int frameImage(int w, int h);

	/*
	 * Helper function using a bilinear function to give a pixel
	 * value back for scaling.
	 *
	 * Credit goes to: http://jstanley.pingerthinger.com/slideshow.html
	 */
	void bilinearPix(SDL_Surface *in, double u, double v, uint8_t *col);

	std::vector<std::string>			filenames;					// List of all filenames
	std::vector<std::string>::iterator	current_image;				// Index in filename structure

	SDL_Surface							*current_image_surface;		// Currently decoded/displaying image
	SDL_Surface							*previous_image_surface;	// Previous image (TODO: Use this for transitions)

	SDL_Surface							*mSDL;						// Display surface
};
#endif //__ISS_IMAGES_H__
