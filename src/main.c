
#define HGL_CHAN_IMPLEMENTATION
#include "erosion_sim.h"
#include "ui.h"
#include "io.h"
#include "image.h"


#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

/*
 * Exit with exit code `exit_code` and print info.
 */
void exit_with_info(int exit_code) {
    printf("Usage: erodr -f file [-options]\n");
    printf("Simulation options:\n");
    printf("  -n ##            Number of particles to simulate (default: 70'000)\n");
    printf("  -t ##            Maximum lifetime of a particle (default: 30)\n");
    printf("  -g ##            Gravitational constant (default: 4)\n");
    printf("  -r ##            Particle erosion radius (default: 2)\n");
    printf("  -e ##            Particle enertia coefficient (default: 0.1)\n");
    printf("  -c ##            Particle capacity coefficient (default: 10)\n");
    printf("  -v ##            Particle evaporation rate (default: 0.1)\n");
    printf("  -s ##            Particle erosion coefficient (default: 0.1)\n");
    printf("  -d ##            Particle deposition coefficient (default: 1.0)\n");
    printf("  -m ##            Minimum slope (default: 0.0001)\n");
    printf("Other options:\n");
    printf("  -p <ini-file>    Use provided parameter ini file. See examples/params.ini for an example.\n");
    printf("  -o <file>        Place the output into <file>\n");
    printf("  -a               Output is ASCII encoded\n");
    exit(exit_code);
}

int main(int argc, char *argv[]) 
{
    SimulationParameters params = DEFAULT_PARAM;  
    ErodrImage hmap;

    // parse args.
    char filepath[IO_FILEPATH_MAXLEN];
    char outputfilepath[IO_FILEPATH_MAXLEN];
    strcpy(outputfilepath, IO_OUTPUTFILEPATH_DEFAULT);
    bool ascii_out = false;
    if(io_parse_args(argc, argv, filepath, outputfilepath, &params, &ascii_out)) {
        exit_with_info(1);
    }

    // load pgm heightmap.
    if(io_load_pgm(filepath, &hmap)) {
        exit_with_info(1);
    }

    /* make copy */
    ErodrImage original_hmap = image_alloc(hmap.width, hmap.height);
    image_copy(&original_hmap, &hmap);

    printf("hmap %d %d\n", hmap.width, hmap.height);

    HglChan c = hgl_chan_make();

    /* start UI */
    pthread_t ui_thread;
    UiArgs ui_args = (UiArgs) {
        .hmap = &hmap,
        .chan = &c, 
    };
    pthread_create(&ui_thread, NULL, ui_run, &ui_args);

    bool running = true;
    while (running) {
        UiCommand cmd = (UiCommand) hgl_chan_recv(&c);
        switch (cmd) {
            case CMD_RESET_HMAP: {
                image_copy(&hmap, &original_hmap);
            } break;

            case CMD_RERUN_SIMULATION: {
                erosion_sim_run(&hmap, &params);
            } break;

            case CMD_EXIT: {
                running = false;
            } break;
        }
    }

    pthread_join(ui_thread, NULL);

    // Maybe clamp
    if (image_clamp(&hmap)) {
        printf("\n\nWARNING: Output is clipping.\n\n");
        printf("The image has been clamped. Some information is lost.\n");
        printf("To avoid this warning, make sure the input image is not\n");
        printf("clipping or nearly clipping.\n");
    }

    // Save results 
    io_save_pgm(outputfilepath, &hmap, ascii_out);

    // free memory
    image_free(&hmap);    

    hgl_chan_destroy(&c);
}
