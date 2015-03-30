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
#define POWERUP_COOLDOWN 1000

Car cars[MAX_CARS];
int cars_count = 0;
const vec2 car_start_dir = {1.0, 0.0};

extern ivec2 map_starting_positions[];
extern ivec2 map_path[];
extern int map_path_length;
extern int map_laps;

Car *car_add()
{
	if (cars_count >= MAX_CARS) {
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
	cars[i].big_at = 0;
	///* DEBUG: re-add once map does deserialization
	cars[i].pos.x = map_starting_positions[i].x;
	cars[i].pos.y = map_starting_positions[i].y;
	
//	cars[i].pos.x = 500;
//	cars[i].pos.y = 200;
	cars[i].direction = car_start_dir;
	char filename[10];
	sprintf(filename, "car%d.bmp", i);
	cars[i].texture = ren_load_image_with_dims(filename, &cars[i].width, &cars[i].height);

	cars[i].width /= 1.5;
	cars[i].height /= 1.5;
	cars[i].name[0] = '0' + i;
	cars[i].name[1] = 0;

	for (int j=0; j<TRAIL_LENGTH; j++) {
		cars[i].trail[j] = cars[i].pos;
	}

	cars[i].lap_started_at = SDL_GetTicks();

	return &cars[i];
}

void cars_start_round()
{
	for (int i=0; i<cars_count; i++) {
		cars[i].lap_started_at = SDL_GetTicks();
		cars[i].picked_up_at = SDL_GetTicks();
	}
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

	if (fabsf(difference.x) < (car1->width/2 + car2->width/2) &&
	    fabsf(difference.y) < (car1->height/2 + car2->height/2))
	{
		vec_normalize(&difference);
		vec_scale(&difference, 3000);
		if (!car1->invincible_at) {
			car_apply_force(car1, difference);
		}
		vec_scale(&difference, -1);
		if (!car2->invincible_at) {
			car_apply_force(car2, difference);
		}

		if (car1->big_at && !car2->invincible_at) {
			car2->tipped_at = SDL_GetTicks();
		}

		if (car2->big_at && !car1->invincible_at) {
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
		if (car->invincible_at) break;
		roll_coeff *= 7;
		drag_coeff *= 7;
		break;
	case MAP_BANANA:
		if (car->invincible_at) break;
		car->tipped_at = SDL_GetTicks();
		break;
	case MAP_OIL:
		if (car->invincible_at) break;
		vec_rotate(&car->direction, 3);
		break;
	case MAP_ICE:
		if (car->invincible_at) break;
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
	car->velocity.x += acceleration.x * SECS_PER_FRAME;
	car->velocity.y += acceleration.y * SECS_PER_FRAME;

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
	if (car->big_at && SDL_GetTicks() - car->big_at > INVINCIBLE_TIMEOUT) {
		car->big_at = 0;
		car->width /= 2;
		car->height /= 2;
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

	car->pos.x += car->velocity.x * SECS_PER_FRAME;
	car->pos.y += car->velocity.y * SECS_PER_FRAME;

	// Check if we hit a box with powerups
	if (car->powerup == POWERUP_NONE && SDL_GetTicks() - car->picked_up_at > POWERUP_COOLDOWN) {
		SDL_Rect car_geometry;
		car_geometry.x = car->pos.x;
		car_geometry.y = car->pos.y;
		car_geometry.w = car->width;
		car_geometry.h = car->height;
		PowerUp powerup = boxes_check_hit(car_geometry);

		if (powerup != POWERUP_NONE) {
			car->powerup = powerup;
			car->picked_up_at = SDL_GetTicks();
		}
	}

	// Check tile/lap count stuff
	const int px = center.x / map_tile_width;
	const int py = center.y / map_tile_height;

	int next_tile = (car->tiles_passed + 1) % map_path_length;
	if (map_path[next_tile].x == px && map_path[next_tile].y == py) {
		car->tiles_passed++;
		if (car->tiles_passed % map_path_length == 0) {
			Uint32 new_time = SDL_GetTicks();
			if (!car->best_lap_time || new_time - car->lap_started_at < car->best_lap_time) {
				car->best_lap_time = new_time - car->lap_started_at;
			}
			printf("laptime %d\n", new_time - car->lap_started_at);
			car->lap_started_at = new_time;
		}

		printf("tiles passed: %d, laps: %d\n", car->tiles_passed, car->tiles_passed / map_path_length);
	}

	// Update trail
	for (int j=1; j<TRAIL_LENGTH; j++) {
		car->trail[j-1] = car->trail[j];
	}
	car->trail[TRAIL_LENGTH-1] = car->pos;

}

void cars_move()
{
	for (int i=0; i<cars_count; i++) {
		if ((!cars[i].invincible_at) && shells_check_collide(cars[i].pos)) {
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

void bullet_move(Car *car)
{
	//TODO: implement me
	car_move(car);
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
	pos.x -= POWERUPS_WIDTH/2;
	pos.y -= POWERUPS_HEIGHT/2;
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
	if (!car->big_at) {
		car->width *= 2;
		car->height *= 2;
	}
	car->big_at = SDL_GetTicks();
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
			if (!cars[i].stunned_at) {
				cars[i].width /= 2;
				cars[i].height /= 2;
			}
			cars[i].stunned_at = SDL_GetTicks();
			cars[i].powerup = POWERUP_NONE;
		}
	}
        break;
    }
    case POWERUP_STAR:
        printf("triggering star\n");
	car->turbo_at = SDL_GetTicks();
	car->invincible_at = SDL_GetTicks();
	if (!car->big_at) {
		car->width *= 2;
		car->height *= 2;
	}
	car->big_at = SDL_GetTicks();
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

	ivec2 center;
	center.x = car->pos.x + car->width/2;
	center.y = car->pos.y + car->height/2;
	cJSON_AddItemToObject(root, "pos", pos = cJSON_CreateObject());
	cJSON_AddNumberToObject(pos, "x", center.x);
	cJSON_AddNumberToObject(pos, "y", center.y);

	cJSON_AddNumberToObject(root, "drift", car->drift);
	cJSON_AddNumberToObject(root, "width", car->width);
	cJSON_AddNumberToObject(root, "height", car->height);

	cJSON *powerup;
	switch(car->powerup) {
	case POWERUP_BANANA:
		powerup = cJSON_CreateString("banana");
		break;
	case POWERUP_GREEN_SHELL:
		powerup = cJSON_CreateString("greenshell");
		break;
	case POWERUP_RED_SHELL:
		powerup = cJSON_CreateString("redshell");
		break;
	case POWERUP_BLUE_SHELL:
		powerup = cJSON_CreateString("blueshell");
		break;
	case POWERUP_OIL:
		powerup = cJSON_CreateString("oil");
		break;
	case POWERUP_MUSHROOM:
		powerup = cJSON_CreateString("mushroom");
		break;
	case POWERUP_BIG_MUSHROOM:
		powerup = cJSON_CreateString("bigmushroom");
		break;
	case POWERUP_LIGHTNING:
		powerup = cJSON_CreateString("lightning");
		break;
	case POWERUP_STAR:
		powerup = cJSON_CreateString("star");
		break;
	case POWERUP_NONE:
	default:
		powerup = cJSON_CreateString("none");
		break;
	}
	cJSON_AddItemToObject(root, "powerup", powerup);

	return root;
}

void car_deserialize(cJSON *root)
{
	Car *car = NULL;
	cJSON *cur, *x, *y;
	cur = cJSON_GetObjectItem(root, "id");
	for (int i = 0; i < cars_count; i++)
	{
		if (cars[i].id == cur->valueint)
		{
			car = &cars[i];
			break;
		}
	}
	if (car == NULL)
		return;
	cur = cJSON_GetObjectItem(root, "direction");
	x = cJSON_GetObjectItem(cur, "x");
	car->direction.x = x->valuedouble;
	y = cJSON_GetObjectItem(cur, "y");
	car->direction.y = y->valuedouble;

	cur = cJSON_GetObjectItem(root, "velocity");
	x = cJSON_GetObjectItem(cur, "x");
	car->velocity.x = x->valuedouble;
	y = cJSON_GetObjectItem(cur, "y");
	car->velocity.y = y->valuedouble;

	cur = cJSON_GetObjectItem(root, "drift");
	car->drift = cur->valueint;
	cur = cJSON_GetObjectItem(root, "width");
	car->width = cur->valueint;
	cur = cJSON_GetObjectItem(root, "height");
	car->height = cur->valueint;

	cur = cJSON_GetObjectItem(root, "pos");
	x = cJSON_GetObjectItem(cur, "x");
	car->pos.x = x->valuedouble;
	car->pos.x -= car->width/2;
	y = cJSON_GetObjectItem(cur, "y");
	car->pos.y = y->valuedouble;
	car->pos.y -= car->height/2;


	cur = cJSON_GetObjectItem(root, "powerup");
	const char *powerup = cur->valuestring;
	if (!strcmp(powerup, "banana")) {
		car->powerup = POWERUP_BANANA;
	} else if (!strcmp(powerup, "greenshell")) {
		car->powerup = POWERUP_GREEN_SHELL;
	} else if (!strcmp(powerup, "redshell")) {
		car->powerup = POWERUP_RED_SHELL;
	} else if (!strcmp(powerup, "blueshell")) {
		car->powerup = POWERUP_BLUE_SHELL;
	} else if (!strcmp(powerup, "oil")) {
		car->powerup = POWERUP_OIL;
	} else if (!strcmp(powerup, "mushroom")) {
		car->powerup = POWERUP_MUSHROOM;
	} else if (!strcmp(powerup, "bigmushroom")) {
		car->powerup = POWERUP_BIG_MUSHROOM;
	} else if (!strcmp(powerup, "lightning")) {
		car->powerup = POWERUP_LIGHTNING;
	} else if (!strcmp(powerup, "star")) {
		car->powerup = POWERUP_STAR;
	} else {
		car->powerup = POWERUP_NONE;
	}
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

void cars_render(SDL_Renderer *ren)
{
	SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
	for (int i=0; i<cars_count; i++) {
		int r = 0xff, g = 0xff, b = 0xff;
		if (i == 0) {
			r = 0xff; g = 0; b = 0;
		} else if (i == 1) {
			r = 0xa0; g = 0xa0; b = 0xff;
		} else if (i == 2) {
			r = 0; g = 0xff; b = 0xa0;
		} else if (i == 3) {
			r = 0xff; g = 0xff; b = 0xff;
		}

		// Draw trail
		int ox = cars[i].width / 2;
		int oy = cars[i].height / 2;
		for (int j=0; j<TRAIL_LENGTH-1; j++) {
			SDL_SetRenderDrawColor(ren, r, g, b, j * 0xff / TRAIL_LENGTH);
			vec2 pos1 = cars[i].trail[j];
			vec2 pos2 = cars[i].trail[j+1];
			SDL_RenderDrawLine(ren, pos1.x + ox, pos1.y + oy, pos2.x + ox, pos2.y + oy);
		}

		// Draw the car itself
		SDL_Rect target;
		target.x = cars[i].pos.x;
		target.y = cars[i].pos.y;
		target.w = cars[i].width;
		target.h = cars[i].height;
		SDL_RenderCopyEx(ren, cars[i].texture, 0, &target, vec_angle(car_start_dir, cars[i].direction), 0, 0);

		// Draw list in top-left corner
		const int vertical_position = 5 + (POWERUPS_HEIGHT + 5) * cars[i].id;

		target.x = 5;
		target.y = vertical_position - 2;
		target.h = POWERUPS_HEIGHT + 3;
		target.w = 700;
		SDL_SetRenderDrawColor(ren, r, g, b, 0x7f);
		SDL_RenderFillRect(ren, &target);
		SDL_RenderDrawRect(ren, &target);

		target.x = 5;
		target.y = vertical_position;
		target.h = POWERUPS_HEIGHT;
		target.w = POWERUPS_HEIGHT * cars[i].width / cars[i].height;
		SDL_RenderCopy(ren, cars[i].texture, 0, &target);

		// Draw powerup in top-left corner
		ivec2 powerup_pos;
		powerup_pos.x = 50;
		powerup_pos.y = vertical_position;
		if (cars[i].powerup != POWERUP_NONE) {
			powerup_render(ren, cars[i].powerup, powerup_pos);
		}
		target.h = POWERUPS_HEIGHT + 1;
		target.w = POWERUPS_WIDTH + 1;
		target.x = powerup_pos.x - 1;
		target.y = powerup_pos.y - 1;
		SDL_SetRenderDrawColor(ren, r, g, b, 0xff);
		SDL_RenderDrawRect(ren, &target);

		// draw name
		render_string(cars[i].name, target.x + POWERUPS_WIDTH + 20, target.y + 5, 22);


		// Draw progress thing
		target.x = target.x + POWERUPS_WIDTH + 320;
		target.y = vertical_position + 5;
		target.h = 22;
		target.w = 22 * (cars[i].tiles_passed % map_path_length) / map_path_length + 1;
		SDL_SetRenderDrawColor(ren, r, g, b, 0x7f);
		SDL_RenderFillRect(ren, &target);
		target.w = 22;
		SDL_RenderDrawRect(ren, &target);

		// Draw lap count
		char laps_string[64];
		snprintf(laps_string, 64, "%d/%d laps", cars[i].tiles_passed / map_path_length, map_laps);
		render_string(laps_string, target.x + 2, target.y, 22);

		// Draw lap time
		target.y = vertical_position;
		render_time(SDL_GetTicks() - cars[i].lap_started_at, target.x + POWERUPS_WIDTH + 175, target.y, 11);
		if (cars[i].best_lap_time) {
			render_time(cars[i].best_lap_time, target.x + POWERUPS_WIDTH + 175, target.y + 16, 11);
		}
	}
}

void car_turn_left(Car *car)
{
	vec_rotate(&car->direction, -3);
}

void car_turn_right(Car *car)
{
	vec_rotate(&car->direction, 3);
}

void car_accelerate(Car *car)
{
	vec2 force = car->direction;
	vec_scale(&force, 2500);
	car_apply_force(car, force);
}

void car_decelerate(Car *car)
{
	vec2 force = car->direction;
	vec_scale(&force, -2500);
	car_apply_force(car, force);
}

int cars_finished()
{
	const int total_length = map_path_length * map_laps;
	for (int i=0; i<cars_count; i++) {
		if (cars[i].tiles_passed >= total_length) {
			return 1;
		}
	}
	return 0;
}

static int car_compare(const void *first, const void *second)
{
	const Car *car1 = (Car*)first;
	const Car *car2 = (Car*)second;
	if (car1->tiles_passed < car2->tiles_passed) {
		return 1;
	} else if (car1->tiles_passed > car2->tiles_passed) {
		return -1;
	} else {
		const int dist1 = map_dist_left_in_tile(car1->tiles_passed, car1->pos);
		const int dist2 = map_dist_left_in_tile(car2->tiles_passed, car2->pos);
		return dist2 - dist1;
	}
}

Car *cars_get_sorted()
{
	Car *sorted = malloc(sizeof(Car) * cars_count);
	memcpy(sorted, cars, sizeof(Car) * cars_count);
	qsort(sorted, cars_count, sizeof(Car), car_compare);
	return sorted;
}

Car *car_get_closest(vec2 pos)
{
	Car *ret = &cars[0];
	double min_dist = hypot(pos.x - ret->pos.x, pos.y - ret->pos.y);
	for (int i=1; i<cars_count; i++) {
		const double dist = hypot(pos.x - cars[i].pos.x, pos.y - cars[i].pos.y);
		if (dist < min_dist) {
			min_dist = dist;
			ret = &cars[i];
		}
	}
	return ret;
}

/* vim: set ts=8 sw=8 tw=0 noexpandtab cindent softtabstop=8 :*/
