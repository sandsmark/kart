#ifndef CAR_H
#define CAR_H

#include <SDL2/SDL.h>

#include "defines.h"

typedef struct {
	int angle; // 0-360 degrees

	float acceleration;
	float speed;
	float x, y;

	int width, height;
	int wheel_x[4];
	int wheel_y[4];

	SDL_Texture *texture;
} Car;

Point rotate_point(int x, int y, const Point center, int angle);
void move_car(Car *car, SDL_Surface *map);
void friction(Car *car);

#endif

/* vim: set ts=8 sw=8 tw=0 noexpandtab cindent softtabstop=8 :*/
