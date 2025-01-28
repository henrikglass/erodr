
#define HGL_FLAGS_IMPLEMENTATION
#include "hgl_flags.h"

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

typedef struct
{
    const char *input_filepath; 
    const char *output_filepath; 
    const char *params_filepath; 
    bool ascii_encode_output;
    SimulationParameters sim_params;
} Args;

Args parse_args(int argc, char *argv[])
{
    Args args = {0};

    /* input, output */
    const char **opt_input_filepath  = hgl_flags_add_str("-i,--input", "path to input heightmap *.pgm file", NULL, HGL_FLAGS_OPT_MANDATORY);
    const char **opt_output_filepath = hgl_flags_add_str("-o,--output", "path to output heightmap *.pgm file", "output.pgm", 0);
    const char **opt_params_filepath = hgl_flags_add_str("-p,--params", "path to simulation parameters *.ini file", NULL, 0);
    bool *opt_ascii_encode_output    = hgl_flags_add_bool("-a, --ascii", "Use ascii encoding for output *.pgm file.", false, 0);
    int64_t *opt_n          = hgl_flags_add_i64("-n,--num-particles", "Number of particles to simulate", DEFAULT_PARAM_N, 0);
    int64_t *opt_ttl        = hgl_flags_add_i64("-t,--ttl", "Maximum lifetime of a particle", DEFAULT_PARAM_TTL, 0);
    int64_t *opt_radius     = hgl_flags_add_i64("-r,--radius", "Particle erosion radius", DEFAULT_PARAM_RADIUS, 0);
    double *opt_gravity     = hgl_flags_add_f64("-g,--gravity", "Gravitational constant", DEFAULT_PARAM_GRAVITY, 0);
    double *opt_inertia     = hgl_flags_add_f64("-y,--inertia", "Particle inertia coefficient", DEFAULT_PARAM_INERTIA, 0);
    double *opt_capacity    = hgl_flags_add_f64("-c,--capacity", "Particle capacity coefficient", DEFAULT_PARAM_CAPACITY, 0);
    double *opt_evaporation = hgl_flags_add_f64("-v,--evaporation-rate", "Particle evaporation rate", DEFAULT_PARAM_EVAPORATION, 0);
    double *opt_erosion     = hgl_flags_add_f64("-s,--erosion-coefficient", "Particle erosion coefficient", DEFAULT_PARAM_EROSION, 0);
    double *opt_deposition  = hgl_flags_add_f64("-d,--deposition-coefficient", "Particle deposition coefficient", DEFAULT_PARAM_DEPOSITION, 0);
    double *opt_min_slope   = hgl_flags_add_f64("-m,--minimum-slope", "Minimum slope", DEFAULT_PARAM_MIN_SLOPE, 0);
    bool *opt_help          = hgl_flags_add_bool("--help", "Show this message", false, 0);

    int err = hgl_flags_parse(argc, argv);
    if (err != 0 || *opt_help) {
        printf("Usage: %s [Options]\n", argv[0]);
        hgl_flags_print();
        exit(1);
    }

    args.input_filepath      = *opt_input_filepath;
    args.output_filepath     = (*opt_output_filepath == NULL) ? "output.pgm" : *opt_output_filepath;
    args.params_filepath     = *opt_params_filepath;
    args.ascii_encode_output = *opt_ascii_encode_output;

    if (args.params_filepath != NULL) {
        args.sim_params = io_read_params_ini(args.params_filepath);
    } else {
        args.sim_params.n             = (int) *opt_n;
        args.sim_params.ttl           = (int) *opt_ttl;
        args.sim_params.p_radius      = (int) *opt_radius;
        args.sim_params.p_inertia     = (float) *opt_inertia;
        args.sim_params.p_capacity    = (float) *opt_capacity;
        args.sim_params.p_gravity     = (float) *opt_gravity;
        args.sim_params.p_evaporation = (float) *opt_evaporation;
        args.sim_params.p_erosion     = (float) *opt_erosion;
        args.sim_params.p_deposition  = (float) *opt_deposition;
        args.sim_params.p_min_slope   = (float) *opt_min_slope;
    }

    return args;
}

int main(int argc, char *argv[]) 
{
    //SimulationParameters params = DEFAULT_PARAM;  
    ErodrImage hmap;

    /* parse cli args */
    Args args = parse_args(argc, argv);


    // parse args.
    //char filepath[IO_FILEPATH_MAXLEN];
    //char outputfilepath[IO_FILEPATH_MAXLEN];
    //strcpy(outputfilepath, IO_OUTPUTFILEPATH_DEFAULT);
    //bool ascii_out = false;
    //if(io_parse_args(argc, argv, filepath, outputfilepath, &params, &ascii_out)) {
    //    exit_with_info(1);
    //}

    // load pgm heightmap.
    if(io_load_pgm(args.input_filepath, &hmap)) {
        printf("Usage: %s [Options]\n", argv[0]);
        hgl_flags_print();
        exit(1);
    }

    /* make copy */
    ErodrImage original_hmap = image_alloc(hmap.width, hmap.height);
    image_copy(&original_hmap, &hmap);

    printf("hmap %d %d\n", hmap.width, hmap.height);

    HglChan c = hgl_chan_make();

    /* start UI */
    pthread_t ui_thread;
    UiArgs ui_args = (UiArgs) {
        .hmap       = &hmap,
        .chan       = &c,
        .sim_params = &args.sim_params,
    };
    pthread_create(&ui_thread, NULL, ui_run, &ui_args);

    bool running = true;
    while (running) {
        UiCommand cmd = (UiCommand) hgl_chan_recv(&c);
        switch (cmd) {
            case CMD_RERUN_SIMULATION: {
                erosion_sim_run(&hmap, &args.sim_params);
            } break;

            case CMD_RELOAD_SIMPARAMS: {
                if (args.params_filepath != NULL) {
                    args.sim_params = io_read_params_ini(args.params_filepath);
                }
            } break;

            case CMD_RESET_HMAP: {
                image_copy(&hmap, &original_hmap);
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
    io_save_pgm(args.output_filepath, &hmap, args.ascii_encode_output);

    /* cleanup (Be polite to the operating system :) )*/
    image_free(&hmap);    
    hgl_chan_destroy(&c);
}
