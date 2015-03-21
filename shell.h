#ifndef SHELL_H
#define SHELL_H

#include <SDL2/SDL.h>
#include "vector.h"
#include "libs/cJSON/cJSON.h"

typedef enum {
	SHELL_GREEN,
	SHELL_RED,
	SHELL_BLUE
} ShellType;

void shell_destroy();

void shells_render(SDL_Renderer *ren);
void shell_add(ShellType type, vec2 pos, vec2 direction);
void shells_move();

int shells_check_collide(vec2 pos);

cJSON *shells_serialize();
void shells_deserialize(cJSON *root);

#endif//SHELL_H
/* vim: set ts=8 sw=8 tw=0 noexpandtab cindent softtabstop=8 :*/
