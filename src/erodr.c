#include <time.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "vector.h"
#include "io.h"
#include "image.h"
#include "params.h"

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
 * gradient & height tuple.
 */
typedef struct hg_tuple {
	vec2 gradient;
	double height;
} hg_tuple;

int min(int a, int b) {
	return (a > b) ? b : a;
}

int max(int a, int b) {
	return (a > b) ? a : b;
}

/*
 * Bilinearly interpolate double value at (x, y) in map.
 */
double bil_interpolate_map_double(const image *map, vec2 pos) {
	double *map_buffer = (double *) map->buffer;
	double u, v, ul, ur, ll, lr, ipl_l, ipl_r;
	int x_i = (int)pos.x;
	int y_i = (int)pos.y;
	u = pos.x - x_i;
	v = pos.y - y_i;
	ul = map_buffer[y_i*map->width + x_i];
	ur = map_buffer[y_i*map->width + x_i + 1];
	ll = map_buffer[(y_i + 1)*map->width + x_i];
	lr = map_buffer[(y_i + 1)*map->width + x_i + 1];
	ipl_l = (1 - v) * ul + v * ll;
	ipl_r = (1 - v) * ur + v * lr;
	return (1 - u) * ipl_l + u * ipl_r;	
}

/*
 * Deposits sediment at position `pos` in heighmap `hmap`.
 * Deposition only affect immediate neighbouring gridpoints
 * to `pos`.
 */
void deposit(image *hmap, vec2 pos, double amount) {
	double *hmap_buffer = (double *) hmap->buffer;
	int x_i = (int)pos.x;
	int y_i = (int)pos.y;
	double u = pos.x - x_i;
	double v = pos.y - y_i;
	hmap_buffer[y_i*hmap->width + x_i] += amount * (1 - u) * (1 - v);
	hmap_buffer[y_i*hmap->width + x_i + 1] += amount * u * (1 - v);
	hmap_buffer[(y_i + 1)*hmap->width + x_i] += amount * (1 - u) * v;
	hmap_buffer[(y_i + 1)*hmap->width + x_i + 1] += amount * u * v;
}

/*
 * Erodes heighmap `hmap` at position `pos` by amount `amount`.
 * Erosion is distributed over an area defined through p_radius.
 */
void erode(image *hmap, vec2 pos, double amount, int radius) {	
	double *hmap_buffer = (double *) hmap->buffer;

	if(radius < 1){
		deposit(hmap, pos, -amount);
		return;
	}
	
	int x0 = (int)pos.x - radius;
	int y0 = (int)pos.y - radius;
	int x_start = max(0, x0);
	int y_start = max(0, y0);
	int x_end = min(hmap->width, x0+2*radius+1);
	int y_end = min(hmap->height, y0+2*radius+1);
	
	// construct erosion/deposition kernel.
	double kernel[2*radius + 1][2*radius + 1];
	double kernel_sum = 0;
	for(int y = y_start; y < y_end; y++) {
		for(int x = x_start; x < x_end; x++) {
			double d_x = x - pos.x;
			double d_y = y - pos.y;
			double distance = sqrt(d_x*d_x + d_y*d_y);
			double w = fmax(0, radius - distance);
			kernel_sum += w;
			kernel[y-y0][x-x0] = w;
		}	
	}

	// normalize weights and apply changes on heighmap.
	for(int y = y_start; y < y_end; y++) {
		for(int x = x_start; x < x_end; x++) {
			kernel[y-y0][x-x0] /= kernel_sum;
			hmap_buffer[y*hmap->width + x] -= amount * kernel[y-y0][x-x0];
		}	
	}
}

/*
 * Returns gradient at (int x, int y) on heightmap `hmap`.
 */
vec2 gradient_at(image *hmap, int x, int y) {
	double *hmap_buffer = (double *) hmap->buffer;
	int idx = y * hmap->width + x;
	//int right = y * hmap->width + min(x, hmap->width - 2);
	//int below = min(y, hmap->height - 2) * hmap->width + x;
	int right = idx + ((x > hmap->width - 2) ? 0 : 1);
	int below = idx + ((y > hmap->height - 2) ? 0 : hmap->width);
	vec2 g;
	g.x = hmap_buffer[right] - hmap_buffer[idx]; 
	g.y = hmap_buffer[below] - hmap_buffer[idx];
	return g;	
}

/*
 * Returns interpolated gradient and height at (double x, double y) on
 * heightmap `hmap`.
 */
hg_tuple height_gradient_at(image *hmap, vec2 pos) {
	hg_tuple ret;
	vec2 ul, ur, ll, lr, ipl_l, ipl_r;
	int x_i = (int)pos.x;
	int y_i = (int)pos.y;
	double u = pos.x - x_i;
	double v = pos.y - y_i;
	ul = gradient_at(hmap, x_i, y_i);
	ur = gradient_at(hmap, x_i + 1, y_i);
	ll = gradient_at(hmap, x_i, y_i + 1);
	lr = gradient_at(hmap, x_i + 1, y_i + 1);
	ipl_l = add(scalar_mul(1 - v, ul), scalar_mul(v, ll));
	ipl_r = add(scalar_mul(1 - v, ur), scalar_mul(v, lr));
	ret.gradient = add(scalar_mul(1 - u, ipl_l), scalar_mul(u, ipl_r));
	ret.height = bil_interpolate_map_double(hmap, pos);
	return ret;
}

/*
 * Runs hydraulic erosion simulation.
 */
void simulate_particles(image *hmap, sim_params *params) {
	srand(time(NULL));
	
	// simulate each particle
	for(int i = 0; i < params->n; i++) {
		if(!((i+1) % 10000))
			printf("Particles simulated: %d\n", i+1);

		// spawn particle.
		particle p;
		double denom = (RAND_MAX / (hmap->width - 1));
		p.pos = (vec2){(double)rand() / denom, (double)rand() / denom};	
		p.dir = (vec2){0, 0};
		p.vel = 0;
		p.sediment = 0;
		p.water = 1;

		for(int j = 0; j < params->ttl; j++) {
			// interpolate gradient g and height h_old at p's position. 
			vec2 pos_old = p.pos;
			hg_tuple hg = height_gradient_at(hmap, pos_old);
			vec2 g = hg.gradient;
			double h_old = hg.height; 

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
			if(pos_new.x > (hmap->width-1) || pos_new.x < 0 || 
					pos_new.y > (hmap->height-1) || pos_new.y < 0)
				break;

			// new height
			double h_new = bil_interpolate_map_double(hmap, pos_new);
			double h_diff = h_new - h_old;
				
			// sediment capacity
			double c = fmax(-h_diff, params->p_min_slope) * p.vel * p.water * params->p_capacity;

			// decide whether to erode or deposit depending on particle properties
			if(h_diff > 0 || p.sediment > c) {
				double to_deposit = (h_diff > 0) ? 
						fmin(p.sediment, h_diff) :
						(p.sediment - c) * params->p_deposition;
				p.sediment -= to_deposit;
				deposit(hmap, pos_old, to_deposit);	
			} else {
				double to_erode = fmin((c - p.sediment) * params->p_erosion, -h_diff);
				p.sediment += to_erode;
				erode(hmap, pos_old, to_erode, params->p_radius);
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
	image img;

	// parse args.
	char filepath[FILEPATH_MAXLEN];
	char outputfilepath[FILEPATH_MAXLEN];
	strcpy(outputfilepath, OUTPUTFILEPATH_DEFAULT);
	bool ascii_out = false;
	if(parse_args(argc, argv, filepath, outputfilepath, &params, &ascii_out))
		return 1;

	// load pgm heightmap.
	if(load_pgm(filepath, &img))
		return 1;
	
	// simulate hydraulic erosion
	simulate_particles(&img, &params);
		
	// Save results	
	save_pgm(outputfilepath, &img, ascii_out);

	// free memory
	release_image(&img);	
}
