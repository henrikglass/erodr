#ifndef IMAGE_H
#define IMAGE_H

/*
 * Image type.
 */
typedef struct image {
	void *buffer;
	int width;
	int height;
} image;

/*
 * Frees image buffer.
 */
void release_image(image *img);

#endif
