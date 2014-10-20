#ifndef DEFINES_H
#define DEFINES_H

typedef struct {
	float x;
	float y;
} vec2;

typedef struct {
	int x;
	int y;
} ivec2;

#define CAR_MASS 20 /*in kg*/
#define CAR_DRAG_COEFF 0.2
#define CAR_ROLL_COEFF 0.03

// Not defined with ansi C
#define PI 3.14159265
#define TIME_CONSTANT 1/30

#endif
