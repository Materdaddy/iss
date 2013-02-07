#include <iostream>

#include <assert.h>
#include <dirent.h>
#include <sys/stat.h>

#include "SDL/SDL_image.h"

#include "easyexif/exif.h"

#include "ISSImages.h"

ISSImages::ISSImages(SDL_Surface *sdl, const std::string directory, bool recursive) :
	mSDL(sdl),
	current_image_surface(NULL),
	previous_image_surface(NULL)
{
	loadFilenames(directory, recursive);

	if ( filenames.size() > 0 )
		current_image = filenames.begin();
	else
		printf("ERROR, no filenames!\n");
}

int ISSImages::loadFilenames(const std::string directory, bool recursive)
{
	DIR *dir;
	class dirent *ent;
	class stat st;

	dir = opendir(directory.c_str());
	while ( (ent = readdir(dir)) != NULL )
	{
		const std::string filename = ent->d_name;
		const std::string full_filename = directory + '/' + filename;

		// Skip "." and ".."
		if ( filename[0] == '.' )
			continue;

		// Skip files that error out.  If we care, we can
		// check errno and see why we failed, but for now,
		// simply skip 'em
		if ( stat(full_filename.c_str(), &st) == -1 )
			continue;

		// If we're a directory, act accordingly
		if ( (st.st_mode & S_IFDIR) != 0 )
		{
			// Skip directory symlinks to take the easy way out
			// of a potential infinite recursion loop. TODO: fancy
			// this up to look for a subdir of our current dir or
			// something more sophisticated than simply skipping
			if ( (st.st_mode & S_IFLNK ) != 0 )
				continue;

			// Recurse if set
			if ( recursive )
				loadFilenames( full_filename, recursive );
			// Otherwise, skip the directory
			else
				continue;
		}
		// We're just a file
		else
		{
			std::string jpg(".jpg");
			std::string jpeg(".jpeg");
			//Check that it's a .jpg or .jpeg
			if ( full_filename.find(jpg, 0) != -1 || full_filename.find(jpeg, 0) != -1 )
				filenames.push_back(full_filename);
			else
				std::cout << "Skipping non-JPG file " << full_filename << std::endl;
		}
	}
	closedir(dir);

	return 0;
}

void print_sdl_surface_info(char *prefix, SDL_Surface *image)
{
	if ( ! image )
	{
		printf("Why the heck are you trying to print info about nothing?!?\n");
		return;
	}

	//uint32_t flags;                           /* Read-only */
	//SDL_PixelFormat *format;                /* Read-only */
	//int w, h;                               /* Read-only */
	//uint16_t pitch;                           /* Read-only */
	//void *pixels;                           /* Read-write */
	//SDL_Rect clip_rect;                     /* Read-only */
	//int refcount;

	printf("%s Flags: ", prefix);
	if ( image->flags & SDL_ANYFORMAT )
		printf("\t\t\tSDL_ANYFORMAT\n");
	if ( image->flags & SDL_ASYNCBLIT )
		printf("\t\t\tSDL_ASYNCBLIT\n");
	if ( image->flags & SDL_DOUBLEBUF )
		printf("\t\t\tSDL_DOUBLEBUF\n");
	if ( image->flags & SDL_HWACCEL )
		printf("\t\t\tSDL_HWACCEL\n");
	if ( image->flags & SDL_HWPALETTE )
		printf("\t\t\tSDL_HWPALETTE\n");
	if ( image->flags & SDL_HWSURFACE )
		printf("\t\t\tSDL_HWSURFACE\n");
	if ( image->flags & SDL_FULLSCREEN )
		printf("\t\t\tSDL_FULLSCREEN\n");
	if ( image->flags & SDL_OPENGL )
		printf("\t\t\tSDL_OPENGL\n");
	if ( image->flags & SDL_OPENGLBLIT )
		printf("\t\t\tSDL_OPENGLBLIT\n");
	if ( image->flags & SDL_RESIZABLE )
		printf("\t\t\tSDL_RESIZABLE\n");
	if ( image->flags & SDL_RLEACCEL )
		printf("\t\t\tSDL_RLEACCEL\n");
	if ( image->flags & SDL_SRCALPHA )
		printf("\t\t\tSDL_SRCALPHA\n");
	if ( image->flags & SDL_SRCCOLORKEY )
		printf("\t\t\tSDL_SRCCOLORKEY\n");
	if ( image->flags & SDL_SWSURFACE )
		printf("\t\t\tSDL_SWSURFACE\n");
	if ( image->flags & SDL_PREALLOC )
		printf("\t\t\tSDL_PREALLOC\n");
	printf("%s PixelFormat->BPP: %hhu\n", prefix, image->format->BytesPerPixel);
	printf("%s PixelFormat->colorkey: %x\n", prefix, image->format->colorkey);
	printf("%s PixelFormat->alpha: %u\n", prefix, image->format->alpha);
	printf("%s w=%u, h=%u\n", prefix, image->w, image->h);
	printf("%s pitch: %u\n", prefix, image->pitch);
	printf("%s pixels: %p\n", prefix, image->pixels);
	printf("%s RECT: x=%hd, y=%hd, w=%hu, h=%hu\n", prefix,
		image->clip_rect.x,
		image->clip_rect.y,
		image->clip_rect.w,
		image->clip_rect.h);
	printf("%s Reference Count: %d\n", prefix, image->refcount);
}
 
// Replaces current_image_surface with an images scaled to WxH 
// using bilinear interpolation.
int ISSImages::scaleImage(int w, int h)
{
	static const uint32_t surface_create_mask 
		= SDL_SWSURFACE | SDL_HWSURFACE | SDL_SRCCOLORKEY | SDL_SRCALPHA;

	uint32_t flags = current_image_surface->flags & surface_create_mask;
	
	SDL_Surface *scaled = SDL_CreateRGBSurface(flags, w, h,
		current_image_surface->format->BitsPerPixel,
		current_image_surface->format->Rmask,
		current_image_surface->format->Gmask,
		current_image_surface->format->Bmask,
		current_image_surface->format->Amask);

	if ( !scaled )
		return -1;

	int Bpp = current_image_surface->format->BytesPerPixel;
	assert( Bpp == scaled->format->BytesPerPixel );
	assert( Bpp == 3 || Bpp == 4 );

	int x, y, i;
	uint8_t c[4];
	double u = 0.0, v = 0.0;
	double du = (double)current_image_surface->w / w;
	double dv = (double)current_image_surface->h / h;
	
	int dr = scaled->pitch - (w * Bpp);

	unsigned char *p = (unsigned char *)scaled->pixels;

	for(y = 0; y < h; y++)
	{
		u = 0.0;

		for(x = 0; x < w; x++)
		{
			bilinearPix(current_image_surface, u, v, c);

			for(i = 0; i < Bpp; i++)
			{
				p[i] = c[i];
			}
			p += Bpp;

			u += du;
		}

		p += dr;
		v += dv;
	}

	// Free the un-scaled surface...
	SDL_FreeSurface(current_image_surface);
	// ...and put the newly scaled one in it's place
	current_image_surface = scaled;

	return 0;
}

// Returns the value of the "pixel" at noninteger coordinates (u, v) using
// bilinear interpolation.
void ISSImages::bilinearPix(SDL_Surface *in, double u, double v, uint8_t *col)
{
    int Bpp = in->format->BytesPerPixel;
	
	unsigned int x = (unsigned int)u, y = (unsigned int)v;
    int i;
    uint8_t c00[4], c01[4], c10[4], c11[4];
    double dx = u - x;
    double dy = v - y;
    double fx0, fx1;

    // error:  return a red pixel, for no good reason
    if( x >= (unsigned int)in->w || y >= (unsigned int)in->h )
    {
        col[0] = 255;
        col[1] = 0;
        col[2] = 0;
        return;
    }

    // ensure we have pixels both down and to the right to work with
    if(x == (unsigned int)in->w - 1) { x--; dx = 1.0; }
    if(y == (unsigned int)in->h - 1) { y--; dy = 1.0; }

    // Now... we have the pixels at (x, y) .. (x+1, y+1) to interpolate about
    // dx = weight toward (x+1); dy = weight toward (y+1)

    // Find pointer to upper-left pixel
    uint8_t *p = (uint8_t *)in->pixels + in->pitch * y + Bpp * x;

    // Get colors of 4 neighbors
    memcpy(c00, p,                   Bpp);
    memcpy(c10, p + Bpp,             Bpp);
    memcpy(c01, p       + in->pitch, Bpp);
    memcpy(c11, p + Bpp + in->pitch, Bpp);

    // Perform bilinear interpolation
    for(i = 0; i < Bpp; i++)
    {
        fx0 = c00[i] + dx * (c10[i] - c00[i]);
        fx1 = c01[i] + dx * (c11[i] - c01[i]);
        col[i] = (uint8_t)(fx0 + dy * (fx1 - fx0));
    }
}

int ISSImages::rotateImage(int rotation, bool mirrored)
{
	static const uint32_t surface_create_mask 
		= SDL_SWSURFACE | SDL_HWSURFACE | SDL_SRCCOLORKEY | SDL_SRCALPHA;

	uint32_t flags = current_image_surface->flags & surface_create_mask;

	// 90 or 270 we swap w/h, but 180 it's the same
	int h = ( rotation == 180 ? current_image_surface->h : current_image_surface->w );
	int w = ( rotation == 180 ? current_image_surface->w : current_image_surface->h );

	SDL_Surface *rotated = SDL_CreateRGBSurface(flags, w, h,
		current_image_surface->format->BitsPerPixel,
		current_image_surface->format->Rmask,
		current_image_surface->format->Gmask,
		current_image_surface->format->Bmask,
		current_image_surface->format->Amask);

	if ( !rotated )
		return NULL;

	int Bpp = rotated->format->BytesPerPixel;
	assert( Bpp == current_image_surface->format->BytesPerPixel );
	assert( current_image_surface->format->BytesPerPixel == 3 || current_image_surface->format->BytesPerPixel == 4 );

	unsigned char *inpp = (unsigned char *)current_image_surface->pixels;
	unsigned char *outp = (unsigned char *)rotated->pixels;

	unsigned char *s, *d;

	switch(rotation)
	{
		case 90:
			for ( int i = 0; i < current_image_surface->h; ++i )
			{
				// Move source to ith row
				s = inpp + ( current_image_surface->pitch * i );
				// Move destination to start of w-ith column
				d = outp + ( (current_image_surface->h-i-1) * Bpp);

				for ( int j = 0; j < current_image_surface->w; ++j )
				{
					memcpy( d, s, Bpp );

					// Move our source pixel right along the image
					s+=Bpp;
					// Move our destination pixel down the column
					d+=rotated->pitch;
				}
			}
			break;
		case 180:
			for ( int i = 0; i < current_image_surface->h; ++i )
			{
				// Move source to the ith row
				s = inpp + ( current_image_surface->pitch * i );
				// Move destination to right-most pixel of each row
				d = outp + ( current_image_surface->pitch * (current_image_surface->h - i - 1)) + current_image_surface->pitch;

				for ( int j = 0; j < current_image_surface->w; ++j )
				{
					memcpy( d, s, Bpp );

					// Move our source pixel right along the image
					s+=Bpp;
					// Move our destination pixel left along the image
					d-=Bpp;
				}
			}
			break;
		case 270:
			for ( int i = 0; i < current_image_surface->h; ++i )
			{
				// Move source to ith row
				s = inpp + ( current_image_surface->pitch * i );
				// Move destination to 
				d = outp + ( rotated->pitch * (current_image_surface->w - 1)) + (Bpp * i);

				for ( int j = 0; j < current_image_surface->w; ++j )
				{
					memcpy( d, s, Bpp );

					// Move our source pixel right along the image
					s+=Bpp;
					// Move our destination pixel up the column
					d-=rotated->pitch;
				}
			}
			break;
	}

	if ( mirrored )
		printf("We don't support mirroring just yet...\n");

	// Free the un-scaled surface...
	SDL_FreeSurface(current_image_surface);
	// ...and put the newly scaled one in it's place
	current_image_surface = rotated;

	return 0;
}

// Replaces image in current_image_surface with one centered inside a WxH frame.
int ISSImages::frameImage(int w, int h)
{
	static const uint32_t surface_create_mask 
		= SDL_SWSURFACE | SDL_HWSURFACE | SDL_SRCCOLORKEY | SDL_SRCALPHA;

	uint32_t flags = current_image_surface->flags & surface_create_mask;

	SDL_Surface *framed = SDL_CreateRGBSurface(flags, w, h,
		current_image_surface->format->BitsPerPixel,
		current_image_surface->format->Rmask,
		current_image_surface->format->Gmask,
		current_image_surface->format->Bmask,
		current_image_surface->format->Amask);

	if ( !framed )
		return -1;

	SDL_FillRect(framed, NULL, SDL_MapRGB(framed->format, 0, 0, 0));

	SDL_Rect srcrect;
	srcrect.x = 0;
	srcrect.y = 0;
	srcrect.w = current_image_surface->w;
	srcrect.h = current_image_surface->h;

	SDL_Rect dstrect;
	dstrect.x = (w - current_image_surface->w) / 2;
	dstrect.y = (h - current_image_surface->h) / 2;
	dstrect.w = current_image_surface->w;
	dstrect.h = current_image_surface->h;

	SDL_BlitSurface(current_image_surface, &srcrect, framed, &dstrect);

	// Free the un-scaled surface...
	SDL_FreeSurface(current_image_surface);
	// ...and put the newly scaled one in it's place
	current_image_surface = framed;

	return 0;
}

// Loads the next image in the set, deletes the "last image" and
// moves the currently decoded one to last image.
int ISSImages::loadNextImage()
{
	if ( filenames.size() <= 0 )
	{
		printf("ERROR, no files to load\n");
		return -1;
	}

	// Iterate our image safely
	if ( current_image + 1 == filenames.end() )
		current_image = filenames.begin();
	else
		current_image++;

	// Rotate decoded images
	if ( previous_image_surface )
		SDL_FreeSurface(previous_image_surface);
	if ( current_image_surface )
		previous_image_surface = current_image_surface;

	// Begin our load
	std::cout << "Loading image " << *current_image << "...";

	FILE *fp = fopen((*current_image).c_str(), "rb");
	if ( !fp )
	{
		printf("Can't open file.\n");
		return -1;
	}

	// Determine the size of the file for our buffer
	fseek(fp, 0, SEEK_END);
	unsigned long fsize = ftell(fp);
	rewind(fp);

	// Make a memory buffer the size of the image on our heap
	unsigned char *buf = new unsigned char[fsize];

	// Read the file into our buffer
	if ( fread(buf, 1, fsize, fp) != fsize)
	{
		printf("Can't read file.\n");
		return -1;
	}
	fclose(fp);

	// Parse the exif data to get the rotation
	EXIFInfo result;
	ParseEXIF(buf, fsize, result);

	// Create an SDL_RWops since we already have the file loaded, we don't
	// need another buffer with it, we'll load it and create a surface
	// from the buffer already containing the JPEG
	SDL_RWops *file;
	file = SDL_RWFromMem( buf, fsize );
	current_image_surface = IMG_Load_RW( file, 0 );
	SDL_FreeRW(file);

	if ( ! current_image_surface )
	{
		printf("ERROR Loading image!\n");
		return -1;
	}
	delete[] buf;
	printf("done\n");


	// rotate the image if necessary
	// we want to do this before scaling because it will change whether it's
	// scaled to the W or the H to fit our display
	// -----------------------------------------------------------------------------
	int rotation = 0;
	bool mirrored = false;
	switch (result.orientation)
	{
		case 7:
			mirrored = true;
		case 8:
			rotation = 270;
			break;

		case 5:
			mirrored = true;
		case 6:
			rotation = 90;
			break;

		case 4:
			mirrored = true;
		case 3:
			rotation = 180;
			break;

		case 2:
			mirrored = true;
		case 1:
			rotation = 0;
			break;
	}

	if ( mirrored || rotation )
	{
		printf("Rotating image...");
		if ( rotateImage(rotation, mirrored) )
			printf("ERROR Rotating image!\n");
		else
			printf("done\n");
	}

	// scale the image if necessary
	// -----------------------------------------------------------------------------

	int w = current_image_surface->w;
	int h = current_image_surface->h;

	if (w != 800)
	{
		h = (h * 800) / w;
		w = 800;
	}

	if (h > 600)
	{
		w = (w * 600) / h;
		h = 600;
	}

	if ( w != current_image_surface->w || h != current_image_surface->h)
	{
		printf("Scaling image...");
		if ( scaleImage(w, h) )
			printf("ERROR Scaling image!\n");
		else
			printf("done\n");
	}

	// frame the scaled image, if necessary
	// -----------------------------------------------------------------------------

	if ( 800 != current_image_surface->w || 600 != current_image_surface->h)
	{
		printf("Framing image...");
		if ( frameImage(800, 600) )
			printf("ERROR Framing image!\n");
		else
			printf("done\n");
	}

	// return success
	// -----------------------------------------------------------------------------
	return 0;
}

int ISSImages::displayImage()
{
	printf("Blitting...");
	SDL_BlitSurface(current_image_surface, NULL, mSDL, NULL);
	printf("done\n");

	printf("Flipping...");
	SDL_Flip(mSDL);
	printf("done\n");
	return 0;
}

ISSImages::~ISSImages()
{
}
