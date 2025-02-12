#include <stdlib.h>
#include <stdio.h> // debug
#include "image.h"
#include <assert.h>
#include <string.h>

ErodrImage image_alloc(int width, int height) {
    return (ErodrImage) {
        .data   = malloc(sizeof(float) * width * height),
        .width  = width,
        .height = height,
    };
}

void image_free(ErodrImage *img) {
    free(img->data);
}
void image_copy(ErodrImage *dst, ErodrImage *src)
{
    assert(src->width == dst->width);
    assert(src->height == dst->height);
    assert(src->data != NULL);
    assert(dst->data != NULL);
    memcpy(dst->data, src->data, sizeof(float)*src->width*src->height);
}

bool image_clamp(ErodrImage *img) 
{
    bool clamped = false;
    int size = img->width * img->height;
    float *data = (float *)img->data;
    for (int i = 0; i < size; i++) {
        float value = data[i];
        if (value > 0.0f && value < 1.0f) {
            continue;
        }
        value = (value < 0.0f) ? 0.0f : value;
        value = (value > 1.0f) ? 1.0f : value;
        data[i] = value;
        clamped = true;
    }    

    return clamped;
}
