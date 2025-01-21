#ifndef IMAGE_H
#define IMAGE_H

#include <stdbool.h>

/*
 * Image type.
 */
typedef struct Image {
    float *data;
    int width;
    int height;
} Image;

/*
 * Frees image data.
 */
void image_free(Image *img);

/*
 * Checks if image `img` is black or white clipping and performs
 * clamping to [0.0, 1.0] if necessary. Returns true if `img` is 
 * clipping and output is clamped.
 */
bool image_clamp(Image *img);

#endif

