#include "vector.h"
#include <math.h>

/*
 * return v0 + v1
 */
vec2 add(vec2 v0, vec2 v1) {
	return (vec2){v0.x + v1.x, v0.y + v1.y};
}

/*
 * return v0 - v1
 */
vec2 sub(vec2 v0, vec2 v1) {
	return (vec2){v0.x - v1.x, v0.y - v1.y};
}

/*
 * return s * v
 */
vec2 scalar_mul(double s, vec2 v) {
	return (vec2){s*v.x, s*v.y};
}

/*
 * normalizes v.
 */
void normalize(vec2 *v) {
	double x_2 = v->x * v->x;
	double y_2 = v->y * v->y;
	double len = sqrt(x_2 + y_2);
	if (len == 0)
		return;
	v->x = v->x / len;
	v->y = v->y / len;
}
