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

// Not defined with ansi C
#define PI 3.14159265

#define MAX_CARS 4

// Car
#define CAR_MASS 20 /*in kg*/
#define CAR_DRAG_COEFF 0.1
#define CAR_ROLL_COEFF 3.0

// Network
#define NET_INPUT_UP 1<<0
#define NET_INPUT_DOWN 1<<1
#define NET_INPUT_LEFT 1<<2
#define NET_INPUT_RIGHT 1<<3
#define NET_INPUT_SPACE 1<<4
#define NET_INPUT_RETURN 1<<5
#define NET_PORT 31337

// Settings
#define FRAMETIME_MS 30
#define SECS_PER_FRAME 1.0/FRAMETIME_MS

#endif
