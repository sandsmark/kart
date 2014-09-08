#ifndef MAP_H
#define MAP_H
#include <SDL2/SDL.h>

#include "defines.h"

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

AreaType map_get_type(const Point pos, const SDL_Surface *map);
#endif
