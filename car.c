#include <math.h>

#include "car.h"
#include "defines.h"
#include "map.h"
#include "vector.h"

void car_apply_force(Car *car, vec2 force)
{
	car->force.x += force.x;
	car->force.y += force.y;
}

void car_move(Car *car, SDL_Surface *map)
{
	float drag_coeff = CAR_DRAG_COEFF;
	float roll_coeff = CAR_ROLL_COEFF;

	// Check if we're passing over something funny
	ivec2 center;
	center.x = car->pos.x + car->width/2;
	center.y = car->pos.y + car->height/2;

	AreaType type = map_get_type(center, map);

	switch(type){
	case MAP_WALL:
		car->velocity.x *= -5;
		car->velocity.y *= -5;
		break;
	case MAP_GRASS:
		roll_coeff *= 10;
		drag_coeff *= 10;
		break;
	case MAP_BOOST:
		roll_coeff = 0;
		drag_coeff = 0;
		car->velocity.x *= 1.2;
		car->velocity.y *= 1.2;
		break;
	case MAP_MUD:
		roll_coeff *= 7;
		drag_coeff *= 7;
		break;
	case MAP_OIL:
		vec_rotate(&car->direction, 3);
		break;
	case MAP_ICE:
		if ((rand() % 2) == 1) {
			vec_rotate(&car->direction, 4);
		} else {
			vec_rotate(&car->direction, -4);
		}
		break;
	default:
		break;
	}

	/* Add up forces, resistances etc. */
	/* Drag */
	car->force.x += -drag_coeff * car->velocity.x * vec_length(car->velocity);
	car->force.y += -drag_coeff * car->velocity.y * vec_length(car->velocity);
	/* Roll resistance */
	car->force.x += -roll_coeff * car->velocity.x;
	car->force.y += -roll_coeff * car->velocity.y;

	vec2 acceleration = {car->force.x/CAR_MASS, car->force.y/CAR_MASS};

	car->velocity.x += acceleration.x * TIME_CONSTANT;
	car->velocity.y += acceleration.y * TIME_CONSTANT;

	car->pos.x += car->velocity.x * TIME_CONSTANT;
	car->pos.y += car->velocity.y * TIME_CONSTANT;
}
