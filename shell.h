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

int shell_init();

void shell_render(SDL_Renderer *ren);
void shell_add(ShellType type, vec2 pos, vec2 direction);
void shell_move();

cJSON *shells_serialize();

#endif//SHELL_H
/* vim: set ts=8 sw=8 tw=0 noexpandtab cindent softtabstop=8 :*/
