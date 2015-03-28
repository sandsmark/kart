#ifndef MAP_H
#define MAP_H
#include <SDL2/SDL.h>

#include "defines.h"
#include "libs/cJSON/cJSON.h"

extern unsigned map_tile_width;
extern unsigned map_tile_height;

typedef enum {
	MAP_TRACK,
	MAP_GRASS,
	MAP_WALL,
	MAP_ICE,
	MAP_BOOST,
	MAP_MUD,
	MAP_OIL,
	MAP_BANANA
} AreaType;

int map_init(const char *map_file);
int map_load_file(const char *file);
void map_destroy();
void map_render(SDL_Renderer *ren);
int map_add_modifier(AreaType type, ivec2 pos);

vec2 map_get_edge_normal(int x, int y);

AreaType map_get_type(const ivec2 pos, int remove_banana);

int map_dist_left_in_tile(int pathcount, vec2 pos);

cJSON *map_serialize();
void map_deserialize(cJSON *map);

cJSON *map_items_serialize();
void map_items_deserialize(cJSON *items);

#endif
/* vim: set ts=8 sw=8 tw=0 noexpandtab cindent softtabstop=8 :*/
