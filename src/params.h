#ifndef PARAMS_H
#define PARAMS_H

#define DEFAULT_PARAM {70000, 30, 2, 0.1, 10, 4, 0.1, 0.1, 1, 0.0001}

/*
 * Simulation parameters.
 */
typedef struct sim_params {
	int n;
	int ttl;
	int p_radius;
	double p_enertia;
	double p_capacity;
	double p_gravity;
	double p_evaporation;
	double p_erosion;
	double p_deposition;
	double p_min_slope;
} sim_params;

#endif
