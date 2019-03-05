#undef _GNU_SOURCE // gets rid of vim warning
#define _GNU_SOURCE
#define FILEPATH_MAXLEN 512

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "vector.h"

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
	   	int *height
) {
	FILE	*fp = fopen(filepath, "r");
	char	*token;
	char	*line = NULL;
	size_t	len = 0;
	//int		p;

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
	//p = atoi(line);
		
	//allocate heightmap
	*heightmap = (double*)malloc(sizeof(double) * (*height) * (*width));

	// read heightmap values
	int i = 0;
	for(; getline(&line, &len, fp) != EOF; i++) {
		(*heightmap)[i] = atof(line); // / p;
	}
	
	fclose(fp);
	if(line)
		free(line);

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
	for(int i = 0; i < width*height; i++) {
		//skip last column.
		if(i % width == width - 1) {
			gradientmap[i] = (vec2){0,0};
			continue;
		}
		
		// calculate slope at i. TODO improve w. sobel.
		int r = i + 1;
		int b = i + width;
		gradientmap[i].x = heightmap[i] - heightmap[r];
		gradientmap[i].y = heightmap[i] - heightmap[b];
	}
}

int n = 1;
int ttl = 30;
double p_enertia = 0.2;
double p_capacity = 8;
void erode(double *heightmap, vec2 *gradientmap, int width, int height) {
	// spawn randomized particles.
	particle p[n];
	srand(time(NULL));
	double denom = (RAND_MAX/width);
	for(int i = 0; i < n; i++) {
		p[i].pos = (vec2){(double)rand() / denom, (double)rand() / denom};	
		p[i].dir = (vec2){0, 0};
		p[i].vel = 0;
		//printf("(%g, %g) ", p[i].pos.x, p[i].pos.y);		
	}
	
	// simulate each particle
	for(int i = 0; i < n; i++) {
		for(int j = 0; j < ttl; j++) {
			// bilinearly interpolate gradient g. 
			double x, y, u, v;
			x = p[i].pos.x;
			y = p[i].pos.y;
			u = x - floor(x);
			v = y - floor(y);
			vec2 ul = gradientmap[(int)y*width + (int)x];
			vec2 ur = gradientmap[(int)y*width + (int)x + 1];
			vec2 ll = gradientmap[((int)y + 1)*width + (int)x];
			vec2 lr = gradientmap[((int)y + 1)*width + (int)x + 1];
			vec2 ipl_l = add(scalar_mul(1 - v, ul), scalar_mul(v, ll));
			vec2 ipl_r = add(scalar_mul(1 - v, ur), scalar_mul(v, lr));
			vec2 g = add(scalar_mul(1 - u, ipl_l), scalar_mul(u, ipl_r));
			
			// calculate new dir vector
			p[i].dir = sub(
					scalar_mul(p_enertia, p[i].dir),
					scalar_mul(1 - p_enertia, g)
			);
			normalize(&p[i].dir); // unsure

			//calculate new pos
			p[i].pos = add(p[i].pos, p[i].dir);
			
			// check bounds
			if(p[i].pos.x > width || p[i].pos.x < 0 ||
					p[i].pos.y > height || p[i].pos.y < 0)
				break;

			printf("%g %g\n", p[i].pos.x, p[i].pos.y);		
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
	int width, height;
	if(load_pgm(filepath, &heightmap, &width, &height))
		return 1;

	// construct gradientmap.
	vec2 *gradientmap = (vec2 *) malloc(sizeof(vec2) * width * height);
	construct_gradientmap(heightmap, gradientmap, width, height);

	// simulate hydraulic erosion
	erode(heightmap, gradientmap, width, height);
	
	// debug - TODO remove
	//printf("%f\n", heightmap[1000000]);
	//sparse_heightmap_print(heightmap, width, height);
	//sparse_gradientmap_print(gradientmap, width, height);

	// free memory
	free(heightmap);
	free(gradientmap);
}
