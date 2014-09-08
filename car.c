#include <SDL2/SDL.h>
#include <math.h>

#include "car.h"
#include "map.h"
#include "defines.h"

Point rotate_point(int x, int y, const Point center, int angle)
{
	Point point;
	point.x = cos(angle * PI / 180) * (x - center.x) - sin(angle * PI / 180) * (y - center.y) + center.x;
	point.y = sin(angle * PI / 180) * (x - center.x) + cos(angle * PI / 180) * (y - center.y) + center.y;
	return point;
}

void move_car(Car *car, SDL_Surface *map)
{
	// Normalize rotation and speed
	if (car->angle < 0) {
		car->angle += 360;
	}
	else if (car->angle > 360) {
		car->angle -= 360;
	}

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

	car->speed += car->acceleration * TIME_CONSTANT;
	car->x += car->speed * cos(PI * car->angle / 180);
        car->y += car->speed * sin(PI * car->angle / 180);
}

void friction(Car *car)
{
	car->speed *= 0.95;
}
