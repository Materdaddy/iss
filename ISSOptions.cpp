#include "ISSOptions.h"

ISSOptions::ISSOptions(int argc, char* argv[]) :
	recursive(false),
	verbose(false),
	speed(5),
	fullscreen(false)
{
	int c;
	int digit_optind = 0;

	enum long_options_enum
	{
		OPT_PATH = 1,
		OPT_RECURSIVE,
		OPT_VERBOSE,
		OPT_SPEED,
		OPT_FULLSCREEN
	};

	while (1) {
		int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		static struct option long_options[] = {
			{"path",       required_argument, 0,  OPT_PATH },
			{"recursive",  no_argument,       0,  OPT_RECURSIVE },
			{"verbose",    no_argument,       0,  OPT_VERBOSE },
			{"speed",      required_argument, 0,  OPT_SPEED },
			{"fullscreen", no_argument,       0,  OPT_FULLSCREEN },
			{0,            0,                 0,  0 }
		};

		c = getopt_long(argc, argv, "p:rvs:f",
						long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
			case 'v':
			case OPT_VERBOSE:
				printf("Verbose\n");
				setVerbose(true);
				break;

			case 'p':
			case OPT_PATH:
				printf("Path: '%s'\n", optarg);
				setPath(optarg);
				break;

			case 'r':
			case OPT_RECURSIVE:
				printf("Recursive\n");
				setRecursive(true);
				break;

			case 's':
			case OPT_SPEED:
				printf("Speed: '%s'\n", optarg);
				setSpeed(atoi(optarg));
				break;

			case 'f':
			case OPT_FULLSCREEN:
				printf("Fullscreen\n");
				setFullscreen(true);
				break;

			default:
				fprintf(stderr, "Error parsing arguments!\n");
				return;
		}
	}

	if (optind < argc) {
		printf("non-option ARGV-elements: ");
		while (optind < argc)
			printf("%s ", argv[optind++]);
			printf("\n");
	}
}

ISSOptions::~ISSOptions()
{
	if ( this->path != NULL )
		delete this->path;
}
