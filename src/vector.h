#ifndef VECTOR_H
#define VECTOR_H

/*
 * 2D vector.
 */
typedef struct Vec2 {
    float x;
    float y;
} Vec2;

/*
 * return v0 + v1.
 */
static inline Vec2 vec2_add(Vec2 v0, Vec2 v1) {
    return (Vec2){v0.x + v1.x, v0.y + v1.y};
}

/*
 * return v0 - v1.
 */
static inline Vec2 vec2_sub(Vec2 v0, Vec2 v1) {
    return (Vec2){v0.x - v1.x, v0.y - v1.y};
}

/*
 * return s * v.
 */
static inline Vec2 vec2_scalar_mul(float s, Vec2 v) {
    return (Vec2){s*v.x, s*v.y};
}

/*
 * Normalizes vector v.
 */
static inline Vec2 vec2_normalize(Vec2 v) {
    const float EPSILON = 0.000001;
    float len = sqrt(v.x * v.x + v.y * v.y);
    if (len < EPSILON) return v;
    return (Vec2) {.x = v.x / len, .y = v.y / len};
}

#endif
