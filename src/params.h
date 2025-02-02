#ifndef PARAMS_H
#define PARAMS_H

#define DEFAULT_PARAM_N               70000
#define DEFAULT_PARAM_TTL                32
#define DEFAULT_PARAM_RADIUS              2
#define DEFAULT_PARAM_INERTIA             0.3
#define DEFAULT_PARAM_CAPACITY            8.0
#define DEFAULT_PARAM_GRAVITY             1.0
#define DEFAULT_PARAM_EVAPORATION         0.02
#define DEFAULT_PARAM_EROSION             0.7
#define DEFAULT_PARAM_DEPOSITION          0.3
#define DEFAULT_PARAM_MIN_SLOPE           0.0001
#define DEFAULT_PARAM_INITIAL_VELOCITY    0.9
#define DEFAULT_PARAM_INITIAL_WATER       1.0

#define DEFAULT_PARAM                                         \
    (SimulationParameters) {                                  \
        .n                  = DEFAULT_PARAM_N,                \
        .ttl                = DEFAULT_PARAM_TTL,              \
        .p_radius           = DEFAULT_PARAM_RADIUS,           \
        .p_inertia          = DEFAULT_PARAM_INERTIA,          \
        .p_capacity         = DEFAULT_PARAM_CAPACITY,         \
        .p_gravity          = DEFAULT_PARAM_GRAVITY,          \
        .p_evaporation      = DEFAULT_PARAM_EVAPORATION,      \
        .p_erosion          = DEFAULT_PARAM_EROSION,          \
        .p_deposition       = DEFAULT_PARAM_DEPOSITION,       \
        .p_min_slope        = DEFAULT_PARAM_MIN_SLOPE,        \
        .p_initial_velocity = DEFAULT_PARAM_INITIAL_VELOCITY, \
        .p_initial_water    = DEFAULT_PARAM_INITIAL_WATER,    \
    }

/*
 * Simulation parameters.
 */
typedef struct SimulationParameters {
    int n;
    int ttl;
    int p_radius;
    float p_inertia;
    float p_capacity;
    float p_gravity;
    float p_evaporation;
    float p_erosion;
    float p_deposition;
    float p_min_slope;
    float p_initial_velocity;
    float p_initial_water;
} SimulationParameters;

#endif
