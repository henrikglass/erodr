#ifndef IO_H
#define IO_H

#define FILEPATH_MAXLEN 512
#define OUTPUTFILEPATH_DEFAULT "output.pgm"

#include <stdbool.h>
#include "image.h"
#include "params.h"

/*
 * Parses command line arguments.
 */
int parse_args(
		int argc,
	   	char *argv[],
	   	char *filepath,
	   	char *outputfilepath,
	   	sim_params *params,
		bool *ascii_encoding
);

/*
 * Loads *.pgm into image `img`. `img` contains an internal buffer which is
 * dynamically allocated in load_pgm and should be free'd after use.
 */
int load_pgm(
		const char *filepath,
	   	image *img
);

/*
 * Saves image `img` to a *.pgm file.
 */
int save_pgm(
		const char *filepath,
	   	image *img,
		bool ascii_encoding
);

/*
 * Checks if image `img` is black or white clipping and performs
 * clamping to [0.0, 1.0] if necessary. Returns true if `img` is 
 * clipping and output is clamped.
 */
bool maybe_clamp(image *img);

#endif
