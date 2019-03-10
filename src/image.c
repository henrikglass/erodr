#include <stdlib.h>
#include "image.h"

/*
 * Frees image buffer.
 */
void release_image(image *img) {
	free(img->buffer);
}
