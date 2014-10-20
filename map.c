#include "map.h"

AreaType map_get_type(const ivec2 pos, const SDL_Surface *map)
{
	if (pos.x < 0 || pos.y < 0 || pos.x >= map->w || pos.y >= map->h) {
		return 0;
	}

	if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
		printf("TODO: port to big endian.\n");
		exit(1);
	}

	if (map->format->BytesPerPixel != 3) {
		printf("Unexpected map format.\n");
		exit(1);
	}

	Uint8 *p = (Uint8 *)map->pixels + pos.y * map->pitch + pos.x * 3;
	return p[0] << 16 | p[1] << 8 | p[2];
}

