#ifndef MAP_H
#define MAP_H
#include <SDL2/SDL.h>

#include "defines.h"

#define TILE_WIDTH 128
#define TILE_HEIGHT 128

typedef enum {
	MAP_TRACK,
	MAP_GRASS,
	MAP_WALL,
	MAP_ICE,
	MAP_OIL,
	MAP_BOOST,
	MAP_STARTAREA,
	MAP_MUD,
	MAP_WATER,
	MAP_BANANA
} AreaType;

int map_init(SDL_Renderer *ren, const char *map_file);
void map_destroy();
void map_render(SDL_Renderer *ren);
int map_add_modifier(AreaType type, ivec2 pos);

vec2 map_get_edge_normal(int x, int y);

AreaType map_get_type(const ivec2 pos);

void map_check_tile_passed(int *current_tile, vec2 pos);

#endif
/* vim: set ts=8 sw=8 tw=0 noexpandtab cindent softtabstop=8 :*/
