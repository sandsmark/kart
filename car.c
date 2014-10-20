#include <SDL2/SDL.h>
#include <math.h>

#include "car.h"
#include "defines.h"
#include "map.h"
#include "vector.h"

void apply_force(Car *car, vec2 force)
{
	car->force.x += force.x;
	car->force.y += force.y;
}

void move_car(Car *car, SDL_Surface *map)
{
	/*
	if (car->speed > CAR_TOPSPEED) {
		car->acceleration = 0;
	}
	else if (car->speed < CAR_TOPSPEED_REV) {
		car->acceleration = 0;
	}

	// Check if we're passing over something funny
	Point center;
	center.x = car->x + car->width/2;
	center.y = car->y + car->height/2;
	for (int i=0; i<4; i++) {
		AreaType type = map_get_type(rotate_point(car->x + car->wheel_x[i], car->y + car->wheel_y[i], center, car->angle), map);

		switch(type){
		case MAP_WALL:
			//TODO moar stuffs
			if (car->speed > 0) {
				car->speed = -5;
				car->acceleration = 0;
			}
			else if (car->speed < 0) {
				car->speed = 5;
				car->acceleration = 0;
			}
			i=5;
			break;
		case MAP_GRASS:
			if (car->speed > 1) {
				car->acceleration -= 5;
			}
			else if (car->speed < -1) {
				car->acceleration += 5;
			}
			break;
		case MAP_BOOST:
			car->acceleration += 20;
			break;
		case MAP_MUD:
			if (car->speed > 0.5) {
				car->acceleration -= 7;
			}
			else if (car->speed < -0.5) {
				car->acceleration += 7;
			}
			break;
		case MAP_OIL:
			car->angle += 3;
			car->acceleration += 2;
			break;
		case MAP_ICE:
			if ((rand() % 2) == 1) {
				car->angle += 2;
			} else {
				car->angle -= 2;
			}
			break;
		default:
			break;
		}
	}
	*/

	/* Add up forces, resistances etc. */
	/* Drag */
	car->force.x += -CAR_DRAG_COEFF * car->velocity.x * length(car->velocity);
	car->force.y += -CAR_DRAG_COEFF * car->velocity.y * length(car->velocity);
	/* Roll resistance */
	car->force.x += -CAR_ROLL_COEFF * car->velocity.x;
	car->force.y += -CAR_ROLL_COEFF * car->velocity.y;

	vec2 acceleration = {car->force.x/CAR_MASS, car->force.y/CAR_MASS};

	car->velocity.x += acceleration.x * TIME_CONSTANT;
	car->velocity.y += acceleration.y * TIME_CONSTANT;

	car->pos.x += car->velocity.x * TIME_CONSTANT;
        car->pos.y += car->velocity.y * TIME_CONSTANT;;
}
