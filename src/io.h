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
 * Loads *.pgm into buffer `buffer`. `buffer` is dynamically allocated in 
 * load_pgm and should be free'd after use.
 */
int load_pgm(
		const char *filepath,
	   	image *img,
		int *precision
);

/*
 * Saves buffer `buffer` to a file.
 */
int save_pgm(
		const char *filepath,
	   	image *img,
		int precision,
		bool ascii_encoding
);

#endif
