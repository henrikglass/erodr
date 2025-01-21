#ifndef IO_H
#define IO_H

#define IO_FILEPATH_MAXLEN 512
#define IO_OUTPUTFILEPATH_DEFAULT "output.pgm"

#include <stdbool.h>
#include "image.h"
#include "params.h"

/*
 * Parses command line arguments.
 */
int io_parse_args(int argc,
                  char *argv[],
                  char *filepath,
                  char *outputfilepath,
                  SimulationParameters *params,
                  bool *ascii_encoding);

/*
 * Loads *.pgm into image `img`. `img` contains an internal buffer which is
 * dynamically allocated in load_pgm and should be free'd after use.
 */
int io_load_pgm(const char *filepath, Image *img);

/*
 * Saves image `img` to a *.pgm file.
 */
int io_save_pgm(const char *filepath, Image *img, bool ascii_encoding);

#endif
