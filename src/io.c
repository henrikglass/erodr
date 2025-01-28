#undef _GNU_SOURCE // gets rid of vim warning
#define _GNU_SOURCE
#define PRECISION_8 255
#define PRECISION_16 65535

#define HGL_INI_IMPLEMENTATION
#include "hgl_ini.h"

#include "io.h"
#include <math.h>
#include <stdio.h> 
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define GET_INI_PARAM_INT(params, ini, key)                                    \
    do {                                                                       \
        if (hgl_ini_has(ini, "SimulationParameters", #key)) {                  \
            (params).key = hgl_ini_get_f64(ini, "SimulationParameters", #key); \
        }                                                                      \
    } while (0)

#define GET_INI_PARAM_FLOAT(params, ini, key)                                  \
    do {                                                                       \
        if (hgl_ini_has(ini, "SimulationParameters", #key)) {                  \
            (params).key = hgl_ini_get_f64(ini, "SimulationParameters", #key); \
        }                                                                      \
    } while (0)

#define MIN(a, b) ((a) < (b) ? (a) : (b))

/*
 * Reads a parameter *.ini file.
 */
SimulationParameters io_read_params_ini(const char *filepath)
{
    SimulationParameters parameters = DEFAULT_PARAM;

    HglIni *params_ini = hgl_ini_parse(filepath);
    if (params_ini == NULL) {
        fprintf(stderr, "Error opening/parsing `%s`.\n", optarg);
    }

    GET_INI_PARAM_INT(parameters, params_ini, ttl);
    GET_INI_PARAM_INT(parameters, params_ini, n);
    GET_INI_PARAM_INT(parameters, params_ini, p_radius);
    GET_INI_PARAM_FLOAT(parameters, params_ini, p_enertia);
    GET_INI_PARAM_FLOAT(parameters, params_ini, p_capacity);
    GET_INI_PARAM_FLOAT(parameters, params_ini, p_gravity);
    GET_INI_PARAM_FLOAT(parameters, params_ini, p_evaporation);
    GET_INI_PARAM_FLOAT(parameters, params_ini, p_erosion);
    GET_INI_PARAM_FLOAT(parameters, params_ini, p_deposition);
    GET_INI_PARAM_FLOAT(parameters, params_ini, p_min_slope);

    hgl_ini_free(params_ini);

    return parameters;
}

/*
 * Parses command line arguments.
 */
int io_parse_args(int argc,
                  char *argv[],
                  char *filepath,
                  char *outputfilepath,
                  SimulationParameters *params,
                  bool *ascii_encoding) 
{
    int c;
    while((c = getopt(argc, argv, "af:o:p:n:t:r:e:c:g:v:s:d:m:")) != -1){
        switch(c){
            /* input heightmap filepath */
            case 'f': {
                strncpy(filepath, optarg, IO_FILEPATH_MAXLEN);
            } break;

            /* parameter *.ini filepath */
            case 'p': {
                *params = io_read_params_ini(optarg);
                //HglIni *params_ini = hgl_ini_parse(optarg);
                //if (params_ini == NULL) {
                //    fprintf(stderr, "Error opening/parsing `%s`.\n", optarg);
                //}

                ////if (hgl_ini_has(params_ini, "SimulationParameters", "p_gravity")) { params->p_gravity = hgl_ini_get_u64(params_ini, "SimulationParameters", "p_gravity");}
                //GET_INI_PARAM_INT(params_ini, ttl);
                //GET_INI_PARAM_INT(params_ini, n);
                //GET_INI_PARAM_INT(params_ini, p_radius);
                //GET_INI_PARAM_FLOAT(params_ini, p_enertia);
                //GET_INI_PARAM_FLOAT(params_ini, p_capacity);
                //GET_INI_PARAM_FLOAT(params_ini, p_gravity);
                //GET_INI_PARAM_FLOAT(params_ini, p_evaporation);
                //GET_INI_PARAM_FLOAT(params_ini, p_erosion);
                //GET_INI_PARAM_FLOAT(params_ini, p_deposition);
                //GET_INI_PARAM_FLOAT(params_ini, p_min_slope);

                //hgl_ini_free(params_ini);
            } break;

            /* output heightmap filepath */
            case 'o': {
                strncpy(outputfilepath, optarg, IO_FILEPATH_MAXLEN);
            } break;

            /* use ascii encoding for the output *.pgm file */
            case 'a': {
                *ascii_encoding = true;
            } break;

            /* parameter overrides */
            case 't': params->ttl = atoi(optarg); break;
            case 'n': params->n = atoi(optarg); break;
            case 'r': params->p_radius = atoi(optarg); break;
            case 'e': params->p_enertia = atof(optarg); break;
            case 'c': params->p_capacity = atof(optarg); break;
            case 'g': params->p_gravity = atof(optarg); break;
            case 'v': params->p_evaporation = atof(optarg); break;
            case 's': params->p_erosion = atof(optarg); break;
            case 'd': params->p_deposition = atof(optarg); break;
            case 'm': params->p_min_slope = atof(optarg); break;
            default:
                fprintf(stderr, "Usage: %s\n", argv[0]);
                return 1;
        }   
    }

    return 0;
}

/*
 * gets next value from *.pgm file.
 */
int pgm_next_value(FILE *fp, char *buffer, size_t size)
{
    int c = '\0';   
    size_t i = 1;

    while(c != EOF){
        // skip spaces
        while((c = fgetc(fp)) != EOF && isspace(c));

        // skip commented lines 
        if(c == '#'){
            while(c != '\r' && c != '\n' && (c = fgetc(fp)) != EOF);
        } else if (c != EOF){
            // read values to buffer
            buffer[0] = c;
            for(; i < size && (c = fgetc(fp)) != EOF && !isspace(c); i++) {
                buffer[i] = c;  
            }

            // null terminate
            buffer[MIN(size - 1, i)] = '\0';
            return i;
        }
    }

    return EOF; 
}

/*
 * Loads *.pgm into image `img`. `img` contains an internal buffer which is
 * dynamically allocated in load_pgm and should be free'd after use.
 */
int io_load_pgm(const char *filepath, ErodrImage *img) {
    FILE    *fp = fopen(filepath, "rb");
    char    *line = NULL;
    char    magic[16];
    char    value_buffer[16];
    int     precision;

    if(fp == NULL)
        return 1;

    // read header
    if(pgm_next_value(fp, value_buffer, 16) == EOF) return 1;
    strncpy(magic, value_buffer, 16);
    if(pgm_next_value(fp, value_buffer, 16) == EOF) return 1;
    img->width = atoi(value_buffer);
    if(pgm_next_value(fp, value_buffer, 16) == EOF) return 1;
    img->height = atoi(value_buffer);
    if(pgm_next_value(fp, value_buffer, 16) == EOF) return 1;
    precision = atoi(value_buffer);

    // Allocate buffer for pixel values
    img->data = malloc(sizeof(float) * (img->height) * (img->width));
    float *data = (float *) img->data;
    if(data == NULL)
        return 1;

    // Read pixel values to data. 
    // If magic is "P2" then values are ASCII encoded. 
    // If magic is "P5" then values are binary encoded. 
    if(strncmp(magic, "P2", 2) == 0){
        for(int i = 0; pgm_next_value(fp, value_buffer, 16) != EOF; i++)
            data[i] = atof(value_buffer) / precision;
    } else if(strncmp(magic, "P5", 2) == 0) {
        int byte_depth = precision <= PRECISION_8 ? 1 : 2; 
        char tmp[byte_depth];
        for(int i = 0; i < img->width * img->height; i++) {
            fread(tmp, sizeof(char), byte_depth, fp);
            int val = (byte_depth == 2) ? 
                ((tmp[0] << 8) & 0xFF00) | (tmp[1] & 0x00FF) :
                tmp[0] & 0xFF;
            data[i] = (float) val / precision;
        }
    }

    // cleanup
    fclose(fp);
    if(line)
        free(line);

    return 0;
}

#ifdef _WIN32
static inline uint16_t bswap16(uint16_t v)
{
    return (v << 8) | ((v >> 8) & 0xFF);
}
#endif

/*
 * Saves image `img` to a *.pgm file.
 */
int io_save_pgm(const char *filepath, ErodrImage *img, bool ascii_encoding)
{
    FILE *fp = fopen(filepath, "wb");

    // write "header"
    fputs((ascii_encoding) ? "P2\n" : "P5\n", fp);  
    fputs("# Generated by erodr\n", fp);
    fprintf(fp, "%d %d\n", img->width, img->height);    
    fprintf(fp, "%d\n", PRECISION_16);  
    
    // write data.
    float *data = (float *)img->data;
    if (ascii_encoding) {   
        for(int i = 0; i < img->width * img->height; i++) {
            fprintf(fp, "%d\n", (int)round(data[i]*PRECISION_16));    
        }
    } else {
        float *image_data32f = (float *) img->data;
        for (int y = 0; y < img->height; y++) {
            for (int x = 0; x < img->width; x++) {
#ifdef _WIN32
                // better hope you're not running windows on a BE machine...
                uint16_t r = bswap16((uint16_t)(image_data32f[y*img->width + x] * PRECISION_16));
#else
                uint16_t r = htobe16((uint16_t)(image_data32f[y*img->width + x] * PRECISION_16));
#endif
                fwrite(&r, 1, sizeof(r), fp);
            }
        }
        fflush(fp);
    }   
    fclose(fp);
    
    return 0;
}

