#ifndef __ISS_OPTIONS__
#define __ISS_OPTIONS__

#include <string>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

class ISSOptions
{
public:
	ISSOptions(int argc, char* argv[]);
	~ISSOptions();

	//PATH
	void setPath(char *string) { this->path = new std::string(string); }
	std::string *getPath() { return this->path; }

	//RECURSIVE
	void setRecursive(bool recursive) { this->recursive = recursive; }
	bool getRecursive() { return this->recursive; }

	//VERBOSE
	void setVerbose(bool verbose) { this->verbose = verbose; }
	bool getVerbose() { return this->verbose; }

	//SPEED
	void setSpeed(int speed) { this->speed = speed; }
	int getSpeed() { return this->speed; }

	//FULLSCREEN
	void setFullscreen(bool fullscreen) { this->fullscreen = fullscreen; }
	bool getFullscreen() { return this->fullscreen; }

private:
	std::string *path;
	bool recursive;
	bool verbose;
	int speed;
	int fullscreen;
};

#endif //__ISS_OPTIONS__
