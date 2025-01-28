#ifndef UI_H
#define UI_H

#include "image.h"

#include "hgl_chan.h"

typedef enum
{
    CMD_RERUN_SIMULATION,
    CMD_RESET_HMAP,
    CMD_EXIT,
} UiCommand;

typedef struct
{
    ErodrImage *hmap; 
    HglChan *chan;
} UiArgs;

void *ui_run(void *args);

#endif /* UI_H */

