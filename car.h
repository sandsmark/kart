#ifndef CAR_H
#define CAR_H

#include <SDL2/SDL.h>

#include "powerup.h"
#include "defines.h"
#include "libs/cJSON/cJSON.h"

typedef struct {
	int id;
	vec2 direction;

	vec2 force;
	vec2 velocity;
	vec2 pos;

	char drift;

	int width, height;
	int wheel_x[4];
	int wheel_y[4];

	SDL_Texture *texture;

	PowerUp powerup;

	int active_effects;
	int tiles_passed;

	Uint32 effect_started;
} Car;

void car_apply_force(Car *car, vec2 force);
void car_collison(Car *car1, Car *car2);
void car_move(Car *car);
void car_use_powerup(Car *car);
cJSON *car_serialize(Car *car);

#endif

/* vim: set ts=8 sw=8 tw=0 noexpandtab cindent softtabstop=8 :*/
