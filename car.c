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

void car_collison(Car *car1, Car *car2)
{
	ivec2 car1_center, car2_center;
	car1_center.x = car1->pos.x + car1->width/2;
	car1_center.y = car1->pos.y + car1->height/2;
	car2_center.x = car2->pos.x + car2->width/2;
	car2_center.y = car2->pos.y + car2->height/2;

	vec2 difference;
	difference.x = car1_center.x - car2_center.x;
	difference.y = car1_center.y - car2_center.y;

	if (abs(difference.x) < (car1->width/2 + car2->width/2) &&
	    abs(difference.y) < (car1->height/2 + car2->height/2))
	{
		vec_normalize(&difference);
		vec_scale(&difference, 3000);
		car_apply_force(car1, difference);
		vec_scale(&difference, -1);
		car_apply_force(car2, difference);
	}
}

void car_move(Car *car)
{
	float drag_coeff = CAR_DRAG_COEFF;
	float roll_coeff = CAR_ROLL_COEFF;

	// Check if we're passing over something funny
	ivec2 center;
	center.x = car->pos.x + car->width/2;
	center.y = car->pos.y + car->height/2;

	AreaType type = map_get_type(center);

	switch(type){
	case MAP_WALL:
		vec_scale(&car->velocity, -1);
		break;
	case MAP_GRASS:
		roll_coeff *= 10;
		drag_coeff *= 10;
		break;
	case MAP_BOOST:
		roll_coeff = 0;
		drag_coeff = 0;
		vec_scale(&car->velocity, 1.2);
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

	/* Kill orthogonal velocity */
	float drift = 0.8;
	vec2 fw, side, fw_velo, side_velo;
	vec_copy(car->direction, &fw);
	vec_normalize(&fw);
	vec_copy(fw, &side);
	vec_rotate(&side, 90);
	fw_velo.x = fw.x * vec_dot(car->velocity, fw);
	fw_velo.y = fw.y * vec_dot(car->velocity, fw);
	side_velo.x = side.x * vec_dot(car->velocity, side);
	side_velo.y = side.y * vec_dot(car->velocity, side);
	car->velocity.x = fw_velo.x + side_velo.x * drift;
	car->velocity.y = fw_velo.y + side_velo.y * drift;

	car->pos.x += car->velocity.x * TIME_CONSTANT;
	car->pos.y += car->velocity.y * TIME_CONSTANT;
}
