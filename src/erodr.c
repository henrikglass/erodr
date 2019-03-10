#define DEFAULT_PARAM {70000, 30, 2, 0.1, 10, 4, 0.1, 0.1, 1, 0.0001}

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "vector.h"
#include "io.h"

double max(double a, double b){
	return (a > b) ? a : b;
}

double min(double a, double b){
	return (a < b) ? a : b;
}

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

/*
 * Particle type.
 */
typedef struct particle {
	vec2 pos;
	vec2 dir;
	double vel;
	double sediment;
	double water;
} particle;

/*
 * Constructs the gradientmap.
 */
void construct_gradientmap(
		const double *heightmap,
	   	vec2 gradientmap[],
	   	int width,
	   	int height
) {
	//skip last line.
	for(int i = 0; i < width * (height - 1); i++) {
		//skip last column.
		if(i % width == width - 1) {
			gradientmap[i] = (vec2){0,0};
			continue;
		}
		
		// calculate slope at i. TODO improve w. sobel.
		int right = i + 1;
		int below = i + width;
		gradientmap[i].x = heightmap[right] - heightmap[i];
		gradientmap[i].y = heightmap[below] - heightmap[i];
	}
}

/*
 * Bilinearly interpolate vec2 value at (x, y) in map.
 */
vec2 bil_interpolate_map_vec2(const vec2 *map, vec2 pos, int width) {
	vec2 ul, ur, ll, lr, ipl_l, ipl_r;
	int x_i = (int)pos.x;
	int y_i = (int)pos.y;
	double u = pos.x - x_i;
	double v = pos.y - y_i;
	ul = map[y_i*width + x_i];
	ur = map[y_i*width + x_i + 1];
	ll = map[(y_i + 1)*width + x_i];
	lr = map[(y_i + 1)*width + x_i + 1];
	ipl_l = add(scalar_mul(1 - v, ul), scalar_mul(v, ll));
	ipl_r = add(scalar_mul(1 - v, ur), scalar_mul(v, lr));
	return add(scalar_mul(1 - u, ipl_l), scalar_mul(u, ipl_r));
}

/*
 * Bilinearly interpolate double value at (x, y) in map.
 */
double bil_interpolate_map_double(const double *map, vec2 pos, int width) {
	double u, v, ul, ur, ll, lr, ipl_l, ipl_r;
	int x_i = (int)pos.x;
	int y_i = (int)pos.y;
	u = pos.x - x_i;
	v = pos.y - y_i;
	ul = map[y_i*width + x_i];
	ur = map[y_i*width + x_i + 1];
	ll = map[(y_i + 1)*width + x_i];
	lr = map[(y_i + 1)*width + x_i + 1];
	ipl_l = (1 - v) * ul + v * ll;
	ipl_r = (1 - v) * ur + v * lr;
	return (1 - u) * ipl_l + u * ipl_r;	
}

/*
 * Updates gradientmap at index idx.
 */
inline void update_gradient_at(
		const double *hmap,
	   	vec2 *gmap,
	   	int idx,
	   	int width
) {
	int right = idx + 1;
	int below = idx + width;
	gmap[idx].x = hmap[right] - hmap[idx];
	gmap[idx].y = hmap[below] - hmap[idx];
}

/*
 * Deposits sediment at position `pos` in heighmap `hmap`.
 * Deposition only affect immediate neighbouring gridpoints
 * to `pos`.
 */
void deposit(double *hmap, vec2 *gmap, vec2 pos, double amount, int width) {
	int x_i = (int)pos.x;
	int y_i = (int)pos.y;
	double u = pos.x - x_i;
	double v = pos.y - y_i;
	hmap[y_i*width + x_i]			+= amount * (1 - u) * (1 - v);
	hmap[y_i*width + x_i + 1]		+= amount * u * (1 - v);
	hmap[(y_i + 1)*width + x_i]		+= amount * (1 - u) * v;
	hmap[(y_i + 1)*width + x_i + 1] += amount * u * v;
	update_gradient_at(hmap, gmap, y_i*width + x_i, width);
	update_gradient_at(hmap, gmap, y_i*width + x_i + 1, width);
	update_gradient_at(hmap, gmap, (y_i + 1)*width + x_i, width);
	update_gradient_at(hmap, gmap, (y_i + 1)*width + x_i + 1, width);
}

/*
 * Erodes heighmap `hmap` at position `pos` by amount `amount`.
 * Erosion is distributed over an area defined through p_radius.
 */
void erode(
		double *hmap,
	   	vec2 *gmap,
	   	vec2 pos,
		double amount,
	   	int width,
	   	int height,
	   	sim_params *params
) {
	
	if(params->p_radius < 1){
		deposit(hmap, gmap, pos, -amount, width);
		return;
	}
	
	int x0 = (int)pos.x - params->p_radius;
	int y0 = (int)pos.y - params->p_radius;
	
	// construct erosion/deposition kernel.
	double kernel[2*params->p_radius + 1][2*params->p_radius + 1];
	double kernel_sum = 0;
	for(int y = y0; y < y0 + 2*params->p_radius + 1; y++) {
		for(int x = x0; x < x0 + 2*params->p_radius + 1; x++) {
			double d_x = x - pos.x;
			double d_y = y - pos.y;
			double distance = sqrt(d_x*d_x + d_y*d_y);
			double w = max(0, params->p_radius - distance);
			kernel_sum += w;
			if(x < 0 || y < 0 || x >= width || y >= height){
				continue;
			}
			kernel[y-y0][x-x0] = w;
			//kernel_sum += w;
		}	
	}

	// normalize weights and apply changes on heighmap.
	for(int y = y0; y < y0 + 2*params->p_radius + 1; y++) {
		for(int x = x0; x < x0 + 2*params->p_radius + 1; x++) {
			if(x < 0 || y < 0 || x >= width || y >= height)
				continue;
			kernel[y-y0][x-x0] /= kernel_sum;
			hmap[y*width + x] -= amount * kernel[y-y0][x-x0];
		}	
	}
	
	//Apply changes to gradientmap
	for(int y = y0; y < y0 + 2*params->p_radius + 1; y++) {
		for(int x = x0; x < x0 + 2*params->p_radius + 1; x++) {
			if(x < 0 || y < 0 || x >= width || y >= height)
				continue;
			int idx = y*width + x;
			update_gradient_at(hmap, gmap, idx, width);	
		}	
	}
}

void simulate_particles(
		double *hmap,
	   	vec2 *gmap,
	   	int width,
	   	int height,
	   	sim_params *params
) {
	srand(time(NULL));
	
	// simulate each particle
	for(int i = 0; i < params->n; i++) {
		if(!((i+1) % 10000))
			printf("Particles simulated: %d\n", i+1);

		// spawn particle.
		particle p;
		double denom = (RAND_MAX / (width - 1));
		p.pos = (vec2){(double)rand() / denom, (double)rand() / denom};	
		p.dir = (vec2){0, 0};
		p.vel = 0;
		p.sediment = 0;
		p.water = 1;

		for(int j = 0; j < params->ttl; j++) {
			// interpolate gradient g and height h_old at p's position. 
			vec2 pos_old = p.pos;
			vec2 g = bil_interpolate_map_vec2(gmap, pos_old, width);
			double h_old = bil_interpolate_map_double(hmap, pos_old, width);
		
			//printf("(%g, %g) has:\t", pos_old.x, pos_old.y);	

			// calculate new dir vector
			p.dir = sub(
					scalar_mul(params->p_enertia, p.dir),
					scalar_mul(1 - params->p_enertia, g)
			);
			normalize(&p.dir);
			
			// calculate new pos
			p.pos = add(p.pos, p.dir);
			
			// check bounds
			vec2 pos_new = p.pos;
			if(pos_new.x > (width-1) || pos_new.x < 0 || 
					pos_new.y > (height-1) || pos_new.y < 0)
				break;

			// new height
			double h_new = bil_interpolate_map_double(hmap, pos_new, width);
			double h_diff = h_new - h_old;
		
			// sediment capacity
			double c = max(-h_diff, params->p_min_slope) * p.vel * p.water * params->p_capacity;

			// decide whether to erode or deposit depending on particle properties
			if(h_diff > 0 || p.sediment > c) {
				double to_deposit = (h_diff > 0) ? 
						min(p.sediment, h_diff) :
						(p.sediment - c) * params->p_deposition;
				p.sediment -= to_deposit;
				deposit(hmap, gmap, pos_old, to_deposit, width);	
			} else {
				double to_erode = min((c - p.sediment) * params->p_erosion, -h_diff);
				p.sediment += to_erode;
				erode(hmap, gmap, pos_old, to_erode, width, height, params);
			}

			// update `vel` and `water`
			p.vel = sqrt(p.vel*p.vel + h_diff*params->p_gravity);
			p.water *= (1 - params->p_evaporation);
		}	
	}
}

/*
 * Main.
 */
int main(int argc, char *argv[]) {
	sim_params params = DEFAULT_PARAM;	
	
	// parse args.
	char filepath[FILEPATH_MAXLEN];
	if(parse_args(argc, argv, filepath))
		return 1;

	// load pgm heightmap.
	double *heightmap = NULL;
	int width, height, precision;
	if(load_pgm(filepath, &heightmap, &width, &height, &precision))
		return 1;

	// construct gradientmap.
	vec2 *gradientmap = (vec2 *)malloc(sizeof(vec2) * width * height);
	construct_gradientmap(heightmap, gradientmap, width, height);
	
	// simulate hydraulic erosion
	simulate_particles(heightmap, gradientmap, width, height, &params);

	// Save results	
	save_pgm("output.pgm", heightmap, width, height, precision, false);

	// free memory
	free(heightmap);
	free(gradientmap);
}
