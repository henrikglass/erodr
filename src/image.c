#include <stdlib.h>
#include <stdio.h> // debug
#include "image.h"

void image_free(Image *img) {
    free(img->data);
}

bool image_clamp(Image *img) 
{
    bool clamped = false;
    int size = img->width * img->height;
    float *data = (float *)img->data;
    for (int i = 0; i < size; i++) {
        float value = data[i];
        if (value > 0.0f && value < 1.0f)
            continue;
        value = (value < 0.0f) ? 0.0f : value;
        value = (value > 1.0f) ? 1.0f : value;
        data[i] = value;
        clamped = true;
    }    

    return clamped;
}
