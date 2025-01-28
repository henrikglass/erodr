#ifndef IMAGE_H
#define IMAGE_H

#include <stdbool.h>

/*
 * Image type.
 */
typedef struct ErodrImage {
    float *data;
    int width;
    int height;
} ErodrImage;

/*
 * Allocates memory for image.
 */
ErodrImage image_alloc(int width, int height);

/*
 * Frees image data.
 */
void image_free(ErodrImage *img);

/*
 * copies image data from `src` to `dst`.
 */
void image_copy(ErodrImage *dst, ErodrImage *src);

/*
 * Checks if image `img` is black or white clipping and performs
 * clamping to [0.0, 1.0] if necessary. Returns true if `img` is 
 * clipping and output is clamped.
 */
bool image_clamp(ErodrImage *img);

#endif

