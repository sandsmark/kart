#include "shell.h"
#include "renderer.h"
#include "powerup.h"
#include "map.h"
#include "car.h"

#include <SDL2/SDL.h>

typedef struct {
	vec2 direction;

	vec2 pos;

	ShellType type;
} Shell;

static int shells_count = 0;
static int shells_size = 0;
static Shell *shells = 0;

void shell_destroy()
{
	free(shells);
}

void shells_render(SDL_Renderer *ren)
{
	ivec2 pos;
	for (int i=0; i<shells_count; i++) {
		pos.x = shells[i].pos.x;
		pos.y = shells[i].pos.y;
		switch(shells[i].type) {
			case SHELL_BLUE:
				powerup_render(ren, POWERUP_BLUE_SHELL, pos);
				break;
			case SHELL_GREEN:
				powerup_render(ren, POWERUP_GREEN_SHELL, pos);
				break;
			case SHELL_RED:
				powerup_render(ren, POWERUP_RED_SHELL, pos);
				break;
		}
	}
}

void shell_add(ShellType type, vec2 pos, vec2 direction)
{
	if (shells_count >= shells_size) {
		shells_size += 10;
		Shell *old_address = shells;
		shells = realloc(shells, shells_size * sizeof(Shell));
		if (!shells) {
			printf("failed to allocate memory for shells!\n");
			shells_size -= 10;
			shells = old_address;
			return;
		}
	}

	Shell shell;
	shell.pos = pos;
	vec_normalize(&direction);
	vec_scale(&direction, 5.5);
	shell.pos.x += direction.x * 5;
	shell.pos.y += direction.y * 5;
	shell.direction = direction;
	shell.type = type;

	shells[shells_count] = shell;
	shells_count++;
}

void shell_remove(int index)
{
	shells_count--;
	for (int i=index; i<shells_count; i++) {
		shells[i] = shells[i + 1];
	}
}

void shell_move(Shell *shell)
{
	Car *target_car = 0;
	if (shell->type == SHELL_BLUE) {
		target_car = car_get_leader();
	} else if (shell->type == SHELL_GREEN) {
		target_car = car_get_closest(shell->pos);
	}

	if (target_car) {
		vec2 target = target_car->pos;
		if (target.x > shell->pos.x) {
			shell->direction.x += 0.1;
		} else {
			shell->direction.x -= 0.1;
		}
		if (target.y > shell->pos.y) {
			shell->direction.y += 0.1;
		} else {
			shell->direction.y -= 0.1;
		}
		vec_normalize(&shell->direction);
		vec_scale(&shell->direction, 5.5);
	}


	ivec2 next_pos;
	next_pos.x = shell->pos.x + shell->direction.x;
	next_pos.y = shell->pos.y + shell->direction.y;

	AreaType type = map_get_type(next_pos);
	if (type == MAP_GRASS) { // bounce
		vec2 direction = shell->direction;
		vec_scale(&direction, -1);
		vec2 edge_normal = map_get_edge_normal(next_pos.x, next_pos.y);
		vec_normalize(&edge_normal);
		float angle = vec_angle(edge_normal, direction);
		vec_rotate(&direction, -2.0* angle);
		shell->direction = direction;
	}
	next_pos.x = shell->pos.x + shell->direction.x;
	next_pos.y = shell->pos.y + shell->direction.y;
	shell->pos.x = next_pos.x;
	shell->pos.y = next_pos.y;
}

void shells_move()
{
	for (int i=0; i<shells_count; i++) {
		shell_move(&shells[i]);
	}
	for (int i=0; i<shells_count; i++) {
		if (shells[i].pos.x < 0 || shells[i].pos.x > SCREEN_WIDTH ||
		    shells[i].pos.y < 0 || shells[i].pos.y > SCREEN_HEIGHT){
			shell_remove(i);
		}
	}
}

cJSON *shells_serialize()
{
	cJSON *root = cJSON_CreateArray();
	for (int i=0; i<shells_count; i++) {
		cJSON *type_string;
		switch(shells[i].type) {
		case SHELL_GREEN:
			type_string = cJSON_CreateString("green");
			break;
		case SHELL_BLUE:
			type_string = cJSON_CreateString("blue");
			break;
		case SHELL_RED:
			type_string = cJSON_CreateString("red");
			break;
		default:
			type_string = cJSON_CreateString("none");
		}

		cJSON *shell = cJSON_CreateObject();
		cJSON_AddItemToObject(shell, "type", type_string);
		cJSON_AddNumberToObject(shell, "x", shells[i].pos.x);
		cJSON_AddNumberToObject(shell, "y", shells[i].pos.y);
		cJSON_AddNumberToObject(shell, "dx", shells[i].direction.x);
		cJSON_AddNumberToObject(shell, "dy", shells[i].direction.y);
		cJSON_AddItemToArray(root, shell);
	}
	return root;
}

void shells_deserialize(cJSON *root)
{
	cJSON *shell, *cur;
	for (int i=0; i<cJSON_GetArraySize(root); i++) {
		shell = cJSON_GetArrayItem(root, i);

		vec2 pos;
		cur = cJSON_GetObjectItem(shell, "x");
		pos.x = cur->valueint;
		cur = cJSON_GetObjectItem(shell, "y");
		pos.y = cur->valueint;

		vec2 direction;
		cur = cJSON_GetObjectItem(shell, "dx");
		direction.x = cur->valueint;
		cur = cJSON_GetObjectItem(shell, "dy");
		direction.y = cur->valueint;

		cur = cJSON_GetObjectItem(shell, "type");
		const char *typestr = cur->valuestring;
		if (!strcmp(typestr, "green")) {
			shell_add(SHELL_GREEN, pos, direction);
		} else if (!strcmp(typestr, "blue")) {
			shell_add(SHELL_BLUE, pos, direction);
		} else if (!strcmp(typestr, "red")) {
			shell_add(SHELL_RED, pos, direction);
		}
	}
}

int shells_check_collide(vec2 pos)
{
	for (int i=0; i<shells_count; i++) {
		vec2 diff = pos;
		diff.x -= shells[i].pos.x;
		diff.y -= shells[i].pos.y;
		if (vec_length(diff) < 10) {
			shell_remove(i);
			return 1;
		}
	}
	return 0;
}

/* vim: set ts=8 sw=8 tw=0 noexpandtab cindent softtabstop=8 :*/
