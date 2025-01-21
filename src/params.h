#ifndef PARAMS_H
#define PARAMS_H

#define DEFAULT_PARAM           \
    (SimulationParameters) {    \
        .n             = 70000, \
        .ttl           = 30,    \
        .p_radius      = 2,     \
        .p_enertia     = 0.1,   \
        .p_capacity    = 10,    \
        .p_gravity     = 4,     \
        .p_evaporation = 0.1,   \
        .p_erosion     = 0.1,   \
        .p_deposition  = 1,     \
        .p_min_slope   = 0.0001 \
    }

/*
 * Simulation parameters.
 */
typedef struct SimulationParameters {
    int n;
    int ttl;
    int p_radius;
    float p_enertia;
    float p_capacity;
    float p_gravity;
    float p_evaporation;
    float p_erosion;
    float p_deposition;
    float p_min_slope;
} SimulationParameters;

#endif
