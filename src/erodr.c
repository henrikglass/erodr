#include <time.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "vector.h"
#include "io.h"
#include "image.h"
#include "params.h"
#include "util.h"

/*
 * Particle type.
 */
typedef struct Particle {
    Vec2 pos;
    Vec2 dir;
    float vel;
    float sediment;
    float water;
} Particle;

/*
 * gradient & height tuple.
 */
typedef struct HeigthGradientTuple {
    Vec2 gradient;
    float height;
} HeigthGradientTuple;

/*
 * Bilinearly interpolate float value at (x, y) in map.
 */
float bilerp_map(Image *hmap, Vec2 pos) {
    float u, v, ul, ur, ll, lr, ipl_l, ipl_r;
    int x_i = (int)pos.x;
    int y_i = (int)pos.y;
    u = pos.x - x_i;
    v = pos.y - y_i;
    ul = hmap->data[y_i*hmap->width + x_i];
    ur = hmap->data[y_i*hmap->width + x_i + 1];
    ll = hmap->data[(y_i + 1)*hmap->width + x_i];
    lr = hmap->data[(y_i + 1)*hmap->width + x_i + 1];
    ipl_l = (1 - v) * ul + v * ll;
    ipl_r = (1 - v) * ur + v * lr;
    return (1 - u) * ipl_l + u * ipl_r; 
}

/*
 * Deposits sediment at position `pos` in heighmap `hmap`.
 * Deposition only affect immediate neighbouring gridpoints
 * to `pos`.
 */
void deposit(Image *hmap, Vec2 pos, float amount) {
    int x_i = (int)pos.x;
    int y_i = (int)pos.y;
    float u = pos.x - x_i;
    float v = pos.y - y_i;
    hmap->data[y_i*hmap->width + x_i] += amount * (1 - u) * (1 - v);
    hmap->data[y_i*hmap->width + x_i + 1] += amount * u * (1 - v);
    hmap->data[(y_i + 1)*hmap->width + x_i] += amount * (1 - u) * v;
    hmap->data[(y_i + 1)*hmap->width + x_i + 1] += amount * u * v;
}

/*
 * Erodes heighmap `hmap` at position `pos` by amount `amount`.
 * Erosion is distributed over an area defined through p_radius.
 */
void erode(Image *hmap, Vec2 pos, float amount, int radius) {  
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
    float kernel[2*radius + 1][2*radius + 1];
    float kernel_sum = 0;
    for(int y = y_start; y < y_end; y++) {
        for(int x = x_start; x < x_end; x++) {
            float d_x = x - pos.x;
            float d_y = y - pos.y;
            float distance = sqrt(d_x*d_x + d_y*d_y);
            float w = fmax(0, radius - distance);
            kernel_sum += w;
            kernel[y-y0][x-x0] = w;
        }   
    }

    // normalize weights and apply changes on heighmap.
    for(int y = y_start; y < y_end; y++) {
        for(int x = x_start; x < x_end; x++) {
            kernel[y-y0][x-x0] /= kernel_sum;
            hmap->data[y*hmap->width + x] -= amount * kernel[y-y0][x-x0];
        }   
    }
}

/*
 * Returns gradient at (int x, int y) on heightmap `hmap`.
 */
Vec2 gradient_at(Image *hmap, int x, int y) {
    int idx = y * hmap->width + x;
    //int right = y * hmap->width + min(x, hmap->width - 2);
    //int below = min(y, hmap->height - 2) * hmap->width + x;
    int right = idx + ((x > hmap->width - 2) ? 0 : 1);
    int below = idx + ((y > hmap->height - 2) ? 0 : hmap->width);
    Vec2 g;
    g.x = hmap->data[right] - hmap->data[idx]; 
    g.y = hmap->data[below] - hmap->data[idx];
    return g;   
}

/*
 * Returns interpolated gradient and height at (float x, float y) on
 * heightmap `hmap`.
 */
HeigthGradientTuple height_gradient_at(Image *hmap, Vec2 pos) {
    HeigthGradientTuple ret;
    Vec2 ul, ur, ll, lr, ipl_l, ipl_r;
    int x_i = (int)pos.x;
    int y_i = (int)pos.y;
    float u = pos.x - x_i;
    float v = pos.y - y_i;
    ul = gradient_at(hmap, x_i, y_i);
    ur = gradient_at(hmap, x_i + 1, y_i);
    ll = gradient_at(hmap, x_i, y_i + 1);
    lr = gradient_at(hmap, x_i + 1, y_i + 1);
    ipl_l = vec2_add(vec2_scalar_mul(1 - v, ul), vec2_scalar_mul(v, ll));
    ipl_r = vec2_add(vec2_scalar_mul(1 - v, ur), vec2_scalar_mul(v, lr));
    ret.gradient = vec2_add(vec2_scalar_mul(1 - u, ipl_l), vec2_scalar_mul(u, ipl_r));
    ret.height = bilerp_map(hmap, pos);
    return ret;
}

/*
 * Runs hydraulic erosion simulation.
 */
void simulate_particles(Image *hmap, SimulationParameters *params) {
    srand(time(NULL));

    // simulate each particle
    #pragma omp parallel for
    for(int i = 0; i < params->n; i++) {
        if(!((i+1) % 10000))
            printf("Particles simulated: %d\n", i+1);

        // spawn particle.
        Particle p;
        float denom = (RAND_MAX / ((float)hmap->width - 1.0f));
        p.pos = (Vec2){(float)rand() / denom, (float)rand() / denom}; 
        p.dir = (Vec2){0, 0};
        p.vel = 0;
        p.sediment = 0;
        p.water = 1;

        for(int j = 0; j < params->ttl; j++) {
            // interpolate gradient g and height h_old at p's position. 
            Vec2 pos_old = p.pos;
            HeigthGradientTuple hg = height_gradient_at(hmap, pos_old);
            Vec2 g = hg.gradient;
            float h_old = hg.height; 

            // calculate new dir vector
            p.dir = vec2_sub(vec2_scalar_mul(params->p_enertia, p.dir),
                             vec2_scalar_mul(1 - params->p_enertia, g));
            p.dir = vec2_normalize(p.dir);

            // calculate new pos
            p.pos = vec2_add(p.pos, p.dir);

            // check bounds
            Vec2 pos_new = p.pos;
            if(pos_new.x > (hmap->width-1) || pos_new.x < 0 || 
                    pos_new.y > (hmap->height-1) || pos_new.y < 0)
                break;

            // new height
            float h_new = bilerp_map(hmap, pos_new);
            float h_diff = h_new - h_old;

            // sediment capacity
            float c = fmaxf(-h_diff, params->p_min_slope) * p.vel * p.water * params->p_capacity;

            // decide whether to erode or deposit depending on particle properties
            if(h_diff > 0 || p.sediment > c) {
                float to_deposit = (h_diff > 0) ? fminf(p.sediment, h_diff) :
                                                  (p.sediment - c) * params->p_deposition;
                p.sediment -= to_deposit;
                deposit(hmap, pos_old, to_deposit); 
            } else {
                float to_erode = fminf((c - p.sediment) * params->p_erosion, -h_diff);
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
    SimulationParameters params = DEFAULT_PARAM;  
    Image img;

    // parse args.
    char filepath[IO_FILEPATH_MAXLEN];
    char outputfilepath[IO_FILEPATH_MAXLEN];
    strcpy(outputfilepath, IO_OUTPUTFILEPATH_DEFAULT);
    bool ascii_out = false;
    if(io_parse_args(argc, argv, filepath, outputfilepath, &params, &ascii_out)) {
        exit_with_info(1);
    }

    // load pgm heightmap.
    if(io_load_pgm(filepath, &img)) {
        exit_with_info(1);
    }

    // simulate hydraulic erosion
    simulate_particles(&img, &params);

    // Maybe clamp
    if (image_clamp(&img)) {
        print_clipping_warning();
    }

    // Save results 
    io_save_pgm(outputfilepath, &img, ascii_out);

    // free memory
    image_free(&img);    
}
