#ifndef UI_H
#define UI_H

#include "image.h"
#include "params.h"
#include "hgl_chan.h"

typedef enum
{
    CMD_RERUN_SIMULATION,
    CMD_RELOAD_SIMPARAMS,
    CMD_RESET_HMAP,
    CMD_EXIT,
} UiCommand;

typedef struct
{
    ErodrImage *hmap; 
    HglChan *chan;
    SimulationParameters *sim_params;
} UiArgs;

void *ui_run(void *args);

#endif /* UI_H */

