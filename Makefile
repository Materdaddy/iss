CXX=$(CROSS_COMPILE)g++

LDFLAGS=-lSDL -lSDL_image

ifneq ($(CROSS_COMPILE),)
CPPFLAGS=-DCHUMBY_COMPILE -I/mnt/usb/include
LDFLAGS+=-L/mnt/usb/lib -lts
endif

OBJECTS = \
	ISSOptions.o \
	ISSImages.o \
	easyexif/exif.o \
	$(NULL)

all: $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(LDFLAGS) slideshow.cpp -o slide $(OBJECTS)

clean:
	@rm -f $(OBJECTS) slide
