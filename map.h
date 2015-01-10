#ifndef MAP_H
#define MAP_H
#include <SDL2/SDL.h>

#include "defines.h"

#define TILE_WIDTH 128
#define TILE_HEIGHT 128

typedef enum {
	MAP_TRACK     = 0x808080, // rgb: 128,128,128
	MAP_GRASS     = 0x00ff00, // rgb: 000,255,000
	MAP_WALL      = 0x00ffff, // rgb: 255,255,000
	MAP_ICE       = 0xffffff, // rgb: 255,255,255
	MAP_OIL       = 0x000000, // rgb: 000,000,000
	MAP_BOOST     = 0x0000ff, // rgb: 255,000,000
	MAP_STARTAREA = 0xc0c0c0, // rgb: 192,192,192
	MAP_MUD       = 0x004080, // rgb: 128,064,000
	MAP_WATER     = 0xff0000  // rgb: 000,000,255
} AreaType;

int map_init(SDL_Renderer *ren, const char *map_file);
void map_destroy();
void map_render(SDL_Renderer *ren);
int map_add_modifier(AreaType type, ivec2 pos);

AreaType map_get_type(const ivec2 pos);
#endif
/* vim: set ts=8 sw=8 tw=0 noexpandtab cindent softtabstop=8 :*/
