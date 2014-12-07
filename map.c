#include "map.h"

#include <SDL2/SDL.h>

typedef enum {
	TILE_HORIZONTAL = 0,
	TILE_VERTICAL,
	TILE_UPPERLEFT,
	TILE_UPPERRIGHT,
	TILE_BOTTOMLEFT,
	TILE_BOTTOMRIGHT,
	TILE_NONE
} TileType;

typedef struct
{
	AreaType type;
	SDL_Rect rect;
	SDL_Texture *texture;
} Modifier;


static SDL_Texture *tile_horizontal = 0;
static SDL_Texture *tile_vertical   = 0;
static SDL_Texture *tile_upperleft  = 0;
static SDL_Texture *tile_upperright = 0;
static SDL_Texture *tile_lowerleft  = 0;
static SDL_Texture *tile_lowerright = 0;
static SDL_Texture *tile_none       = 0;

static SDL_Texture *mod_booster_texture = 0;
static SDL_Texture *mod_ice_texture     = 0;
static SDL_Texture *mod_mud_texture     = 0;
static SDL_Texture *mod_oil_texture     = 0;

static TileType **map_tiles = 0;

static unsigned int height, width;

static const unsigned int TILE_WIDTH = 128;
static const unsigned int TILE_HEIGHT = 128;

static const unsigned int MODIFIER_WIDTH = 64;
static const unsigned int MODIFIER_HEIGHT = 64;
static int modifiers_count = 0;
static int modifiers_size = 8;
static Modifier *modifiers = 0;

int add_modifier(AreaType type, ivec2 pos)
{
	if (modifiers_size < modifiers_count) {
		modifiers_size <<= 1;
		modifiers = realloc(modifiers, modifiers_size + 1);
		if (!modifiers) {
			printf("failed to grow array for modifiers\n");
			return 1;
		}
	}

	Modifier modifier;
	modifier.rect.x = pos.x;
	modifier.rect.y = pos.y;
	modifier.rect.w = MODIFIER_WIDTH;
	modifier.rect.h = MODIFIER_HEIGHT;
	modifier.type = type;
	switch (type) {
	case MAP_BOOST:
		modifier.texture = mod_booster_texture;
		modifier.rect.w /= 2;
		modifier.rect.h /= 2;
		break;
	case MAP_ICE:
		modifier.texture = mod_ice_texture;
		break;
	case MAP_MUD:
		modifier.texture = mod_mud_texture;
		break;
	case MAP_OIL:
		modifier.texture = mod_oil_texture;
		break;
	default:
		printf("invalid modifier type: %d\n", type);
		return 1;
	}

	modifiers[modifiers_count] = modifier;

	modifiers_count++;

	return 0;
}

int map_load_tiles(SDL_Renderer *ren);
void map_unload_tiles();
int map_load_file(const char *file);

int map_init(SDL_Renderer *ren, const char *map_file)
{
	if (!map_load_tiles(ren)) {
		printf("failed to load tiles\n");
		return 0;
	}

	if (!map_load_file(map_file)) {
		printf("failed to load map file\n");
		return 0;
	}

	return 1;
}

void map_destroy()
{
	for (unsigned int x = 0; x < width; x++) {
		free(map_tiles[x]);
	}
	free(map_tiles);
}

static int get_r(unsigned int x, unsigned int y, unsigned int ox, unsigned int oy)
{
	unsigned int w = abs(x - ox);
	unsigned int h = abs(y - oy);
	return sqrtl(w*w + h*h);
}

AreaType map_get_type(const ivec2 pos)
{
	if (pos.x < 0 || pos.y < 0) {
		return MAP_WALL;
	}

	for (int i = 0; i < modifiers_count; i++) {
		const int mod_min_x = modifiers[i].rect.x;
		const int mod_min_y = modifiers[i].rect.y;
		const int mod_max_x = mod_min_x + modifiers[i].rect.w;
		const int mod_max_y = mod_min_y + modifiers[i].rect.h;
		if (pos.x > mod_min_x && pos.x < mod_max_x && pos.y > mod_min_y && pos.y < mod_max_y) {
			return modifiers[i].type;
		}
	}

	const unsigned int px = pos.x / TILE_WIDTH;
	const unsigned int py = pos.y / TILE_HEIGHT;
	if (px > width || py > height) {
		return MAP_WALL;
	}

	unsigned int rel_x = pos.x - (px * TILE_WIDTH);
	unsigned int rel_y = pos.y - (py * TILE_HEIGHT);

	int distance;
	switch (map_tiles[px][py]) {
	case TILE_HORIZONTAL:
		distance = rel_y;
		break;
	case TILE_VERTICAL:
		distance = rel_x;
		break;
	case TILE_UPPERLEFT:
		distance = get_r(rel_x, rel_y, TILE_WIDTH, TILE_HEIGHT);
		break;
	case TILE_UPPERRIGHT:
		distance = get_r(rel_x, rel_y, 0, TILE_HEIGHT);
		break;
	case TILE_BOTTOMLEFT:
		distance = get_r(rel_x, rel_y, TILE_WIDTH, 0);
		break;
	case TILE_BOTTOMRIGHT:
		distance = get_r(rel_x, rel_y, 0, 0);
		break;
	case TILE_NONE:
	default:
		return MAP_GRASS;
	}

	if (distance < 16 || distance > 112) {
		return MAP_WALL;
	} else {
		return MAP_TRACK;
	}
	return MAP_TRACK;
}

SDL_Texture *load_image(SDL_Renderer *ren, const char *file)
{
	SDL_Surface *surface = SDL_LoadBMP(file);
	if (surface == NULL) {
		printf("SDL error while loading BMP %s: %s\n", file, SDL_GetError());
		return 0;
	}
	Uint32 colorkey = SDL_MapRGB(surface->format, 0xFF, 0, 0xFF);
	SDL_SetColorKey(surface, SDL_TRUE, colorkey);
	SDL_Texture *texture = SDL_CreateTextureFromSurface(ren, surface);
	if (texture == NULL) {
		printf("SDL error while creating map texture from %s: %s\n", file, SDL_GetError());
		SDL_FreeSurface(surface);
		return 0;
	}
	SDL_FreeSurface(surface);
	return texture;
}

int map_load_tiles(SDL_Renderer *ren)
{
	tile_horizontal = load_image(ren, "map-horizontal.bmp");
	tile_vertical   = load_image(ren, "map-vertical.bmp");
	tile_upperleft  = load_image(ren, "map-topleft.bmp");
	tile_upperright = load_image(ren, "map-topright.bmp");
	tile_lowerleft  = load_image(ren, "map-bottomleft.bmp");
	tile_lowerright = load_image(ren, "map-bottomright.bmp");
	tile_none       = load_image(ren, "map-none.bmp");

	mod_booster_texture = load_image(ren, "booster.bmp");
	mod_ice_texture     = load_image(ren, "ice.bmp");
	mod_mud_texture     = load_image(ren, "mud.bmp");
	mod_oil_texture     = load_image(ren, "oil.bmp");

	return (tile_horizontal &&
		tile_vertical   &&
		tile_upperleft  &&
		tile_upperright &&
		tile_lowerleft  &&
		tile_lowerright &&
		tile_none       &&
		mod_booster_texture &&
		mod_ice_texture     &&
		mod_mud_texture     &&
		mod_oil_texture
		);
}

void map_unload_tiles()
{
	SDL_DestroyTexture(tile_horizontal);
	SDL_DestroyTexture(tile_vertical);
	SDL_DestroyTexture(tile_upperleft);
	SDL_DestroyTexture(tile_upperright);
	SDL_DestroyTexture(tile_lowerleft);
	SDL_DestroyTexture(tile_lowerright);
	SDL_DestroyTexture(tile_none);
}

int map_load_file(const char *filename)
{
	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		printf("failed to open map file %s\n", filename);
		return 0;
	}


	if (fscanf(file, "%ux%u\n", &width, &height) != 2) {
		printf("invalid map file format in file %s\n", filename);
		fclose(file);
		return 0;
	}

	map_tiles = malloc((height + 1) * sizeof(TileType*));
	if (!map_tiles) {
		printf("failed to allocate %lu bytes for map tiles\n", height * sizeof(TileType*));
		return 0;
	}
	for (unsigned int x = 0; x < width; x++) {
		map_tiles[x] = malloc((height + 1) * sizeof(TileType*));

		if (!map_tiles[x]) {
			printf("failed to allocate %lu bytes of memory for map tile line\n", height * sizeof(TileType*));
		}
	}

	char buf[128] = { 0 };
	for (unsigned int y=0; y<height; y++) {
		if (fgets(buf, sizeof(buf), file) == NULL || strlen(buf) < width) {
			printf("line length %lu is less than map width %u\n", strlen(buf), width);
			return 0;
		}

		for (unsigned int x=0; x<width; x++) {
			switch(buf[x]) {
			case '-':
				map_tiles[x][y] = TILE_HORIZONTAL;
				break;
			case '|':
				map_tiles[x][y] = TILE_VERTICAL;
				break;
			case '/':
				map_tiles[x][y] = TILE_UPPERLEFT;
				break;
			case '`':
				map_tiles[x][y] = TILE_UPPERRIGHT;
				break;
			case '\\':
				map_tiles[x][y] = TILE_BOTTOMLEFT;
				break;
			case ',':
				map_tiles[x][y] = TILE_BOTTOMRIGHT;
				break;
			case ' ':
				map_tiles[x][y] = TILE_NONE;
				break;
			default:
				printf("invalid map tile type %c at x: %u y: %u\n", buf[y], x, y);
				return 0;
			}
		}
	}


	modifiers = malloc(sizeof(Modifier) * (modifiers_size + 1));
	if (!modifiers) {
		printf("failed to allocate memory for modifiers\n");
		return 0;
	}
	ivec2 modifier_pos;
	int ret = 0;
	while (fgets(buf, sizeof(buf), file)) {
		sscanf(buf, "%d %d %s\n", &modifier_pos.x, &modifier_pos.y, buf);
		if (strcmp(buf, "mud") == 0) {
			ret = add_modifier(MAP_MUD, modifier_pos);
		} else if (strcmp(buf, "oil") == 0) {
			ret = add_modifier(MAP_OIL, modifier_pos);
		} else if (strcmp(buf, "ice") == 0) {
			ret = add_modifier(MAP_ICE, modifier_pos);
		} else if (strcmp(buf, "booster") == 0) {
			ret = add_modifier(MAP_BOOST, modifier_pos);
		}
		if (ret) {
			printf("error while adding modifier\n");
			return 1;
		}
	}
	rewind(file);
	fclose(file);
	return 1;
}

void map_render(SDL_Renderer *ren)
{
	for (unsigned int x=0; x<width; x++) {
		for (unsigned int y=0; y<height; y++) {
			SDL_Rect target;
			target.x = x * TILE_WIDTH;
			target.y = y * TILE_HEIGHT;
			target.w = TILE_WIDTH;
			target.h = TILE_HEIGHT;
			SDL_Texture *texture;
			switch(map_tiles[x][y]) {
			case TILE_HORIZONTAL:
				texture = tile_horizontal;
				break;
			case TILE_VERTICAL:
				texture = tile_vertical;
				break;
			case TILE_UPPERLEFT:
				texture = tile_upperleft;
				break;
			case TILE_UPPERRIGHT:
				texture = tile_upperright;
				break;
			case TILE_BOTTOMLEFT:
				texture = tile_lowerleft;
				break;
			case TILE_BOTTOMRIGHT:
				texture = tile_lowerright;
				break;
			case TILE_NONE:
			default:
				texture = tile_none;
				break;
			}

			SDL_RenderCopy(ren, texture, 0, &target);
		}
	}
	for (int i = 0; i < modifiers_count; i++) {
		SDL_RenderCopy(ren, modifiers[i].texture, 0, &modifiers[i].rect);
	}
}

/* vim: set ts=8 sw=8 tw=0 noexpandtab cindent softtabstop=8 :*/
