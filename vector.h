#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>

#include "defines.h"

inline float dot(vec2 a, vec2 b)
{
	return a.x * b.x + a.y * b.y;
}

inline float length(vec2 v)
{
	return sqrt(dot(v, v));
}

inline float vec_angle(vec2 a, vec2 b)
{
	/*return acos(dot(a,b)) * 180 / PI;*/
	return (atan2(b.y, b.x) - atan2(a.y, a.x)) * 180 / PI;
}

inline void normalize(vec2 *v)
{
	v->x /= length(*v);
	v->y /= length(*v);
}

inline void rotate(vec2 *v, float angle)
{
	float theta = angle * PI / 180;
	float cs = cos(theta);
	float sn = sin(theta);
	float old_x = v->x;
	float old_y = v->y;

	v->x = old_x * cs - old_y * sn;
	v->y = old_x * sn + old_y * cs;
}

#endif /*VECTOR_H*/
