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

#define NET_PORT 31337

#define CAR_MASS 20 /*in kg*/
#define CAR_DRAG_COEFF 0.1
#define CAR_ROLL_COEFF 3.0

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768

// Not defined with ansi C
#define PI 3.14159265

#define FRAMETIME_MS 30
#define SECS_PER_FRAME 1.0/FRAMETIME_MS

#endif
