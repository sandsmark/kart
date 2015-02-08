#include <math.h>

#include "renderer.h"
#include "car.h"
#include "defines.h"
#include "map.h"
#include "vector.h"
#include "box.h"
#include "libs/cJSON/cJSON.h"
#include "shell.h"

#define STUN_TIMEOUT 2500
#define TURBO_TIMEOUT 2500
#define INVINCIBLE_TIMEOUT 2500
#define TIPPED_TIMEOUT 500

#define MAX_CARS 8
Car cars[MAX_CARS];
int cars_count = 0;
extern ivec2 map_starting_position;
const vec2 car_start_dir = {1.0, 0.0};


Car *car_add()
{
	if (cars_count + 1 >= MAX_CARS) {
		printf("Asked to add a new car when we are at max!\n");
		return 0;
	}

	int i = cars_count;
	cars_count++;

	cars[i].id = i;
	cars[i].stunned_at = 0;
	cars[i].turbo_at = 0;
	cars[i].invincible_at = 0;
	cars[i].tipped_at = 0;
	cars[i].pos.x = map_starting_position.x;
	cars[i].pos.y = map_starting_position.y + i * 20;
	cars[i].direction = car_start_dir;
	char filename[10];
	sprintf(filename, "car%d.bmp", i);
	cars[i].texture = ren_load_image_with_dims(filename, &cars[i].width, &cars[i].height);

	return &cars[i];
}

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
		if (!car1->invincible_at) {
			car_apply_force(car1, difference);
		} else {
			car2->tipped_at = SDL_GetTicks();
		}
		vec_scale(&difference, -1);
		if (!car2->invincible_at) {
			car_apply_force(car2, difference);
		} else {
			car1->tipped_at = SDL_GetTicks();
		}
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
	case MAP_BANANA:
		vec_rotate(&car->direction, 45);
		car->velocity.x = 0;
		car->velocity.y = 0;
		car->force.x = -10;
		car->force.y = -10;
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

	if (car->tipped_at) {
		if (SDL_GetTicks() - car->tipped_at > TIPPED_TIMEOUT) {
			car->tipped_at = 0;
		}
		vec_rotate(&car->direction, 100);
	}
	car->velocity.x += acceleration.x * TIME_CONSTANT;
	car->velocity.y += acceleration.y * TIME_CONSTANT;

	// Check effects
	if (car->stunned_at) {
		if (SDL_GetTicks() - car->stunned_at > STUN_TIMEOUT) {
			car->stunned_at = 0;
			car->width *= 2;
			car->height *= 2;
		}

		vec_scale(&car->velocity, 0.9);
	}
	if (car->turbo_at) {
		if (SDL_GetTicks() - car->turbo_at > TURBO_TIMEOUT) {
			car->turbo_at = 0;
		}
		vec_scale(&car->velocity, 1.05);
	}
	if (car->invincible_at && SDL_GetTicks() - car->invincible_at > INVINCIBLE_TIMEOUT) {
		car->invincible_at = 0;
	}

	/* Kill orthogonal velocity */
	float drift = 0.9;
	if (car->drift)
	{
		drift = 0.97;
	}
	car->drift = 0;
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

	if (car->powerup == POWERUP_NONE) {
		SDL_Rect car_geometry;
		car_geometry.x = car->pos.x;
		car_geometry.y = car->pos.y;
		car_geometry.w = car->width;
		car_geometry.h = car->height;
		PowerUp powerup = boxes_check_hit(car_geometry);

		if (powerup != POWERUP_NONE) {
			car->powerup = powerup;
		}
	}

	map_check_tile_passed(&car->tiles_passed, car->pos);
}

void cars_move()
{
	for (int i=0; i<cars_count; i++) {
		if (shells_check_collide(cars[i].pos)) {
			cars[i].tipped_at = SDL_GetTicks();
		}
		for (int j=i+1; j<cars_count; j++)
		{
			car_collison(&cars[i], &cars[j]);
		}
		car_move(&cars[i]);
		memset(&cars[i].force, 0, sizeof(cars[i].force));
	}
}

void car_use_powerup(Car *car)
{
    ivec2 pos;
    pos.x = car->pos.x;
    pos.y = car->pos.y;

    switch(car->powerup) {
    case POWERUP_NONE:
        return;
    case POWERUP_BANANA: {
        printf("adding bananor\n");
        vec2 rot = car->direction;
        vec_normalize(&rot);
        vec_scale(&rot, 50);
        pos.x -= rot.x;
        pos.y -= rot.y;
        map_add_modifier(MAP_BANANA, pos);
        break;
    }
    case POWERUP_GREEN_SHELL:
        printf("adding green shell\n");
        shell_add(SHELL_GREEN, car->pos, car->direction);
        break;
    case POWERUP_RED_SHELL:
        printf("adding red shell\n");
        shell_add(SHELL_RED, car->pos, car->direction);
        break;
    case POWERUP_BLUE_SHELL:
        printf("adding blue shell\n");
        shell_add(SHELL_BLUE, car->pos, car->direction);
        break;
    case POWERUP_OIL:
        printf("adding oil\n");
        map_add_modifier(MAP_OIL, pos);
        break;
    case POWERUP_MUSHROOM:
        printf("adding mushram\n");
	car->turbo_at = SDL_GetTicks();
        break;
    case POWERUP_BIG_MUSHROOM:
        printf("adding big mushram\n");
	car->turbo_at = SDL_GetTicks();
	car->invincible_at = SDL_GetTicks();
        break;
    case POWERUP_LIGHTNING: {
        printf("triggering lightning\n");

	// find all cars in front of us
	const int my_dist = map_dist_left_in_tile(car->tiles_passed, car->pos);
	for (int i=0; i<cars_count; i++) {
		if (car == &cars[i]) {
			continue;
		}
		if (cars[i].tiles_passed < car->tiles_passed) {
			continue;
		}
		const int distance = map_dist_left_in_tile(cars[i].tiles_passed, cars[i].pos);
		if (cars[i].tiles_passed > car->tiles_passed || my_dist > distance) {
			cars[i].stunned_at = SDL_GetTicks();
			cars[i].width /= 2;
			cars[i].height /= 2;
			cars[i].powerup = POWERUP_NONE;
		}
	}
        break;
    }
    case POWERUP_STAR:
        printf("triggering star\n");
	car->turbo_at = SDL_GetTicks();
	car->invincible_at = SDL_GetTicks();
        break;
    default:
        printf("tried to trigger unknown powerup: %d\n", car->powerup);
        break;
    }
    car->powerup = POWERUP_NONE;
}

cJSON *car_serialize(Car *car)
{
	cJSON *root, *dir, *velo, *pos;
	root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "id", car->id);
	cJSON_AddItemToObject(root, "direction", dir = cJSON_CreateObject());
	cJSON_AddNumberToObject(dir, "x", car->direction.x);
	cJSON_AddNumberToObject(dir, "y", car->direction.y);
	cJSON_AddItemToObject(root, "velocity", velo = cJSON_CreateObject());
	cJSON_AddNumberToObject(velo, "x", car->velocity.x);
	cJSON_AddNumberToObject(velo, "y", car->velocity.y);
	cJSON_AddItemToObject(root, "pos", pos = cJSON_CreateObject());
	cJSON_AddNumberToObject(pos, "x", car->pos.x);
	cJSON_AddNumberToObject(pos, "y", car->pos.y);
	cJSON_AddNumberToObject(root, "drift", car->drift);
	cJSON_AddNumberToObject(root, "width", car->width);
	cJSON_AddNumberToObject(root, "height", car->height);

	return root;
}

// Not the world's most efficient implementation, but it's just 4 cars at max
Car *car_get_leader()
{
	if (cars_count == 0) {
		return 0;
	}
	Car *leader = &cars[0];
	// Find the highest amount of tiles passed
	for (int i=1; i<cars_count; i++) {
		if (cars[i].tiles_passed > leader->tiles_passed) {
			leader = &cars[i];
		}
	}

	// in case of ties, we need to check the distance inside a tile
	for (int i=0; i<cars_count; i++) {
		if (cars[i].tiles_passed < leader->tiles_passed) {
			continue;
		}
		int distance = map_dist_left_in_tile(cars[i].tiles_passed, cars[i].pos);
		int leader_dist = map_dist_left_in_tile(leader->tiles_passed, leader->pos);
		if (distance < leader_dist) {
			leader = &cars[i];
		}
	}
	return leader;
}

/* vim: set ts=8 sw=8 tw=0 noexpandtab cindent softtabstop=8 :*/
