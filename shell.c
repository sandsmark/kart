#include "shell.h"
#include "renderer.h"
#include "powerup.h"
#include "map.h"

#include <SDL2/SDL.h>

static SDL_Texture *shell_green_texture  = 0;
static SDL_Texture *shell_red_texture    = 0;
static SDL_Texture *shell_blue_texture   = 0;

static const int SHELL_LIFETIME = 5000;

typedef struct {
	vec2 direction;

	vec2 force;
	vec2 velocity;
	vec2 pos;

	ShellType type;
} Shell;

static int shells_count = 0;
static int shells_size = 0;
static Shell *shells = 0;


int shell_init()
{
	shell_green_texture = ren_load_image("green_shell.bmp");
	shell_red_texture   = ren_load_image("red_shell.bmp");
	shell_blue_texture  = ren_load_image("blue_shell.bmp");

	return (shell_green_texture &&
			shell_red_texture &&
			shell_blue_texture);
}

void shell_render(SDL_Renderer *ren)
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
		shells = realloc(shells, shells_size * sizeof(Shell));
		if (!shells) {
			printf("failed to allocate memory for shells!\n");
			shells_size -= 10;
			return;
		}
	}

	Shell shell;
	shell.pos = pos;
	vec_normalize(&direction);
	vec_scale(&direction, 5.5);
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

void shell_move()
{
	for (int i=0; i<shells_count; i++) {
		ivec2 next_pos;
		next_pos.x = shells[i].pos.x + shells[i].direction.x;
		next_pos.y = shells[i].pos.y + shells[i].direction.y;
		next_pos.x += POWERUPS_WIDTH / 2;
		next_pos.y += POWERUPS_HEIGHT / 2;
		AreaType type = map_get_type(next_pos);
		if (type == MAP_GRASS) {
			vec2 direction = shells[i].direction;
			vec_scale(&direction, -1);
			vec2 edge_normal = map_get_edge_normal(next_pos.x, next_pos.y);
			vec_normalize(&edge_normal);
			float angle = vec_angle(edge_normal, direction);
			vec_rotate(&direction, -2.0* angle);
			shells[i].direction = direction;
		}
		next_pos.x = shells[i].pos.x + shells[i].direction.x;
		next_pos.y = shells[i].pos.y + shells[i].direction.y;
		shells[i].pos.x = next_pos.x;
		shells[i].pos.y = next_pos.y;
	}
}

cJSON *shells_serialize()
{
	cJSON *root = cJSON_CreateArray();
	for (int i=0; i<shells_count; i++) {
		cJSON *shell = cJSON_CreateObject();
		cJSON_AddNumberToObject(shell, "type", shells[i].type);
		cJSON_AddNumberToObject(shell, "x", shells[i].pos.x);
		cJSON_AddNumberToObject(shell, "y", shells[i].pos.y);
		cJSON_AddNumberToObject(shell, "dx", shells[i].direction.x);
		cJSON_AddNumberToObject(shell, "dy", shells[i].direction.y);
		cJSON_AddItemToArray(root, shell);
	}
	return root;
}

/* vim: set ts=8 sw=8 tw=0 noexpandtab cindent softtabstop=8 :*/
