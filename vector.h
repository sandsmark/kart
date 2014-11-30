#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>

#include "defines.h"

inline float vec_dot(vec2 a, vec2 b)
{
	return a.x * b.x + a.y * b.y;
}

inline float vec_length(vec2 v)
{
	return sqrt(vec_dot(v, v));
}

inline float vec_angle(vec2 a, vec2 b)
{
	return (atan2(b.y, b.x) - atan2(a.y, a.x)) * 180 / PI;
}

inline void vec_normalize(vec2 *v)
{
	v->x /= vec_length(*v);
	v->y /= vec_length(*v);
}

inline void vec_rotate(vec2 *v, float angle)
{
	float theta = angle * PI / 180;
	float cs = cos(theta);
	float sn = sin(theta);
	float old_x = v->x;
	float old_y = v->y;

	v->x = old_x * cs - old_y * sn;
	v->y = old_x * sn + old_y * cs;
}

inline void vec_scale(vec2 *v, float s)
{
	v->x *= s;
	v->y *= s;
}

#endif /*VECTOR_H*/
