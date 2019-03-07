#undef _GNU_SOURCE // gets rid of vim warning
#define _GNU_SOURCE
#define FILEPATH_MAXLEN 512

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "vector.h"

double max(double a, double b){
	return (a > b) ? a : b;
}


double min(double a, double b){
	return (a < b) ? a : b;
}

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
 * Parses command line arguments.
 */
int parse_args(int argc, char *argv[], char *filepath) {
	if (argc == 1) {
		printf("Usage: erodr FILE\n");
		return 1;	
	} else {
		filepath = strncpy(filepath, argv[1], FILEPATH_MAXLEN);
		return 0;
	}
}

/*
 * Loads *.pgm into heightmap.
 */
int load_pgm(
		const char *filepath,
	   	double **heightmap,
	   	int *width,
	   	int *height,
		int *precision
) {
	FILE	*fp = fopen(filepath, "r");
	char	*token;
	char	*line = NULL;
	size_t	len = 0;

	if(fp == NULL)
		return 1;
	
	// read width, height and precision
	// TODO do properly
	if (getline(&line, &len, fp) == EOF) return 1; // magic
	if (getline(&line, &len, fp) == EOF) return 1; // comment
	if (getline(&line, &len, fp) == EOF) return 1; // width height
	token   = strtok(line, " ");
	*width  = atoi(token);
	token   = strtok(NULL, " ");
	*height = atoi(token);
	if (getline(&line, &len, fp) == EOF) return 1; // precision
	*precision = atoi(line);
		
	//allocate heightmap
	*heightmap = (double*)malloc(sizeof(double) * (*height) * (*width));
	if(*heightmap == NULL)
		return 1;

	// read heightmap values
	int i = 0;
	for(; getline(&line, &len, fp) != EOF; i++) {
		(*heightmap)[i] = atof(line) / 10000;
	}
	
	fclose(fp);
	if(line)
		free(line);

	return 0;
}

/*
 * Saves heightmap to *.pgm.
 */
int save_pgm(
		const char *filepath, 
		double *heightmap, 
		int width, 
		int height,
		int precision
) {
	FILE *fp = fopen(filepath, "w");

	// write "header"
	fputs("P2\n", fp);	
	fputs("# Generated by erodr\n", fp);
	fprintf(fp, "%d %d\n", width, height);	
	fprintf(fp, "%d\n", precision);	
	
	for(int i = 0; i < width * height; i++) {
		//fprintf(fp, "%d\n", (int)max(round(heightmap[i]*10000), 0));	
		fprintf(fp, "%d\n", (int)round(heightmap[i]*10000));	
	}	
	fclose(fp);
	
	return 0;
}

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

int n = 75000;
int ttl = 100;
int p_radius = 1;
double p_enertia = 0.05;
double p_capacity = 8;
double p_gravity = 10;
double p_evaporation = 0.01;
double p_erosion = 0.3;
double p_deposition = 0.3;
double p_min_slope = 0;// 0.02; //0.00001;

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
void erode(double *hmap, vec2 *gmap, vec2 pos, double amount, int width, int height) {
	int x0 = (int)pos.x - p_radius;
	int y0 = (int)pos.y - p_radius;
	
	// construct erosion/deposition kernel.
	double kernel[2*p_radius + 1][2*p_radius + 1];
	double kernel_sum = 0;
	for(int y = y0; y < y0 + 2*p_radius + 1; y++) {
		for(int x = x0; x < x0 + 2*p_radius + 1; x++) {
			double d_x = x - pos.x;
			double d_y = y - pos.y;
			double distance = sqrt(d_x*d_x + d_y*d_y);
			double w = max(0, p_radius - distance);
			kernel[y-y0][x-x0] = w;
			kernel_sum += w;
		}	
	}

	// normalize weights and apply changes on heighmap.
	for(int y = y0; y < y0 + 2*p_radius + 1; y++) {
		for(int x = x0; x < x0 + 2*p_radius + 1; x++) {
			int idx = y*width + x;
			if(idx >= width * height || idx < 0)
				continue;
			kernel[y-y0][x-x0] /= kernel_sum;
			hmap[y*width + x] -= amount * kernel[y-y0][x-x0];
		}	
	}
	
	//Apply changes to gradientmap
	for(int y = y0; y < y0 + 2*p_radius + 1; y++) {
		for(int x = x0; x < x0 + 2*p_radius + 1; x++) {
			int idx = y*width + x;
			if(idx >= width * height || idx < 0)
				continue;
			update_gradient_at(hmap, gmap, idx, width);	
		}	
	}
}

void simulate_particles(double *hmap, vec2 *gmap, int width, int height) {
	srand(time(NULL));
	
	// simulate each particle
	for(int i = 0; i < n; i++) {
		if(!((i+1) % 10000))
			printf("Particles simulated: %d\n", i+1);

		// spawn particle.
		particle p;
		double denom = (RAND_MAX / (width - 1));
		p.pos = (vec2){(double)rand() / denom, (double)rand() / denom};	
		p.dir = (vec2){0, 0};
		p.vel = 1;
		p.sediment = 0;
		p.water = 1;

		for(int j = 0; j < ttl; j++) {
			// interpolate gradient g and height h_old at p's position. 
			vec2 pos_old = p.pos;
			vec2 g = bil_interpolate_map_vec2(gmap, pos_old, width);
			double h_old = bil_interpolate_map_double(hmap, pos_old, width);
		
			//printf("(%g, %g) has:\t", pos_old.x, pos_old.y);	

			// calculate new dir vector
			p.dir = sub(
					scalar_mul(p_enertia, p.dir),
					scalar_mul(1 - p_enertia, g)
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
		
			
			//printf("p.sed=%g\t", p.sediment);	

			if(h_diff > 0) {
				double to_deposit = min(p.sediment, h_diff);

				//printf("deposit=%g\t", to_deposit);	
				p.sediment -= to_deposit;
				deposit(hmap, gmap, pos_old, to_deposit, width);
			} else {	
				double c = max(-h_diff, p_min_slope) * p.vel * p.water * p_capacity;
				if (p.sediment > c) {
					double to_deposit = (p.sediment - c) * p_deposition;
					//printf("deposit=%g\t", to_deposit);	
					p.sediment -= to_deposit;
					deposit(hmap, gmap, pos_old, to_deposit, width);
				} else {
					double to_erode = min((c - p.sediment) * p_erosion, -h_diff); 
					//printf("erode=%g\t", to_erode);	
					p.sediment += to_erode;
					erode(hmap, gmap, pos_old, to_erode, width, height);
				} 
				
				//printf("capacity=%g\t", c);	
			}



			// sediment capacity
			/*double c = max(-h_diff, p_min_slope) * p.vel * p.water * p_capacity;

			// decide whether to erode or deposit depending on particle properties
			if(h_diff > 0 || p.sediment > c) {
				double to_deposit = (h_diff > 0) ? 
						min(p.sediment, h_diff) :
						(p.sediment - c) * p_deposition;
				p.sediment -= to_deposit;
				deposit(hmap, pos_old, to_deposit, width);	
			} else {
				double to_erode = min((c - p.sediment) * p_erosion, -h_diff);
				p.sediment += to_erode;
				erode(hmap, pos_old, to_erode, width, height);
			}*/

			// crudely just draw paths
			//hmap[(int)pos_new.y*width + (int)pos_new.x] = 0;	
			
			// update `vel` and `water`
			h_diff = sqrt(h_diff*h_diff); // uh?
			//printf("vel=%g\twater=%g\t\n", p.vel, p.water);
			p.vel = sqrt(p.vel*p.vel + h_diff*p_gravity);
			p.water *= (1 - p_evaporation);

		}	
	}
}

/*
 * Main.
 */
int main(int argc, char *argv[]) {
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
	vec2 *gradientmap = (vec2 *) malloc(sizeof(vec2) * width * height);
	construct_gradientmap(heightmap, gradientmap, width, height);

	// simulate hydraulic erosion
	simulate_particles(heightmap, gradientmap, width, height);
	
	save_pgm("output.pgm", heightmap, width, height, precision);
	// debug - TODO remove
	//printf("%f\n", heightmap[1000000]);
	//sparse_heightmap_print(heightmap, width, height);
	//sparse_gradientmap_print(gradientmap, width, height);

	// free memory
	free(heightmap);
	free(gradientmap);
}
