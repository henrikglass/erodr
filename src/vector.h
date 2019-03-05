#ifndef VECTOR_H
#define VECTOR_H

/*
 * 2D vector.
 */
typedef struct vec2 {
	double x;
	double y;
} vec2;

/*
 * return v0 + v1.
 */
vec2 add(vec2 v0, vec2 v1);

/*
 * return v0 - v1.
 */
vec2 sub(vec2 v0, vec2 v1);

/*
 * return s * v.
 */
vec2 scalar_mul(double s, vec2 v);

/*
 * Normalizes vector v.
 */
void normalize(vec2 *v);

#endif
