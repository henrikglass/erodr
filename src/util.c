#include "util.h"
#include <stdio.h>
#include <stdlib.h>

int min(int a, int b) {
	return (a > b) ? b : a;
}

int max(int a, int b) {
	return (a > b) ? a : b;
}

/*
 * Prints clipping outpute warning.
 */
void print_clipping_warning() {
    printf("\n\nWARNING: Output is clipping.\n\n");
    printf("The image has been clamped. Some information is lost.\n");
    printf("To avoid this warning, make sure the input image is not\n");
    printf("clipping or nearly clipping.\n");
}

/*
 * Exit with exit code `exit_code` and print info.
 */
void exit_with_info(int exit_code) {
	printf("Usage: erodr -f file [-options]\n");
	printf("Simulation options:\n");
	printf(" -n ## \t\t Number of particles to simulate (default: 70'000)\n");
	printf(" -t ## \t\t Maximum lifetime of a particle (default: 30)\n");
	printf(" -g ## \t\t Gravitational constant (default: 4)\n");
	printf(" -r ## \t\t Particle erosion radius (default: 2)\n");
	printf(" -e ## \t\t Particle enertia coefficient (default: 0.1)\n");
	printf(" -c ## \t\t Particle capacity coefficient (default: 10)\n");
	printf(" -v ## \t\t Particle evaporation rate (default: 0.1)\n");
	printf(" -s ## \t\t Particle erosion coefficient (default: 0.1)\n");
	printf(" -d ## \t\t Particle deposition coefficient (default: 1.0)\n");
	printf(" -m ## \t\t Minimum slope (default: 0.0001)\n");
	printf("Other options:\n");
	printf(" -o <file> \t Place the output into <file>\n");
	printf(" -a \t\t Output is ASCII encoded\n");
	exit(exit_code);
}
