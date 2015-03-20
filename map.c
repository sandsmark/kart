#include "map.h"

#include "renderer.h"
#include "box.h"

#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define MAX_WIDTH  64
#define MAX_HEIGHT 64

unsigned map_tile_height = 128;
unsigned map_tile_width = 128;

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
static SDL_Texture *mod_banana_texture  = 0;

static TileType **map_tiles = 0;

static unsigned int map_height, map_width;

static const unsigned int MODIFIER_WIDTH = 64;
static const unsigned int MODIFIER_HEIGHT = 64;
static int modifiers_count = 0;
static int modifiers_size = 0;
static Modifier *modifiers = 0;

ivec2 map_starting_positions[MAX_CARS];

#define MAX_PATH_LENGTH 100
ivec2 map_path[MAX_PATH_LENGTH];
int map_path_length = 0;

int map_laps = 4;

void remove_modifier(int index)
{
	modifiers_count--;
	for (int i=index; i<modifiers_count; i++) {
		modifiers[i] = modifiers[i+1];
	}
}

int map_add_modifier(AreaType type, ivec2 pos)
{
	if (modifiers_size <= modifiers_count) {
		modifiers_size += 10; // grow linearly, makes more sense for this
		Modifier *old_address = modifiers;
		modifiers = realloc(modifiers, modifiers_size * sizeof(Modifier));
		if (!modifiers) {
			printf("failed to grow array for modifiers\n");
			free(old_address);
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
	case MAP_BANANA:
		modifier.texture = mod_banana_texture;
		break;
	default:
		printf("invalid modifier type: %d\n", type);
		return 1;
	}

	modifiers[modifiers_count] = modifier;

	modifiers_count++;

	return 0;
}

int map_load_tiles();
int map_load_file(const char *file);

int map_init(const char *map_file)
{
	if (!map_load_tiles()) {
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
	for (unsigned int x = 0; x < map_width; x++) {
		free(map_tiles[x]);
	}
	free(map_tiles);
	free(modifiers);
}

static int get_r(int x, int y, int ox, int oy)
{
	int w = x - ox;
	int h = y - oy;
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
			if (modifiers[i].type == MAP_BANANA) {
				remove_modifier(i);
			}
			return modifiers[i].type;
		}
	}

	const unsigned int px = pos.x / map_tile_width;
	const unsigned int py = pos.y / map_tile_height;
	if (px >= map_width || py >= map_height) {
		return MAP_WALL;
	}

	unsigned int rel_x = pos.x - (px * map_tile_width);
	unsigned int rel_y = pos.y - (py * map_tile_height);

	int distance;
	switch (map_tiles[px][py]) {
	case TILE_HORIZONTAL:
		distance = rel_y;
		break;
	case TILE_VERTICAL:
		distance = rel_x;
		break;
	case TILE_UPPERLEFT:
		distance = get_r(rel_x, rel_y, map_tile_width, map_tile_height);
		break;
	case TILE_UPPERRIGHT:
		distance = get_r(rel_x, rel_y, 0, map_tile_height);
		break;
	case TILE_BOTTOMLEFT:
		distance = get_r(rel_x, rel_y, map_tile_width, 0);
		break;
	case TILE_BOTTOMRIGHT:
		distance = get_r(rel_x, rel_y, 0, 0);
		break;
	case TILE_NONE:
	default:
		return MAP_GRASS;
	}

	if (distance < 16 || distance > 112) {
		return MAP_GRASS;
	} else {
		return MAP_TRACK;
	}
	return MAP_TRACK;
}

int map_load_tiles()
{
	tile_horizontal = ren_load_image("map-horizontal.bmp");
	tile_vertical   = ren_load_image("map-vertical.bmp");
	tile_upperleft  = ren_load_image("map-topleft.bmp");
	tile_upperright = ren_load_image("map-topright.bmp");
	tile_lowerleft  = ren_load_image("map-bottomleft.bmp");
	tile_lowerright = ren_load_image("map-bottomright.bmp");
	tile_none       = ren_load_image("map-none.bmp");

	mod_booster_texture = ren_load_image("booster.bmp");
	mod_ice_texture     = ren_load_image("ice.bmp");
	mod_mud_texture     = ren_load_image("mud.bmp");
	mod_oil_texture     = ren_load_image("oil_spill.bmp");
	mod_banana_texture  = ren_load_image("placed_banana.bmp");

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
		mod_oil_texture     &&
		mod_banana_texture
		);
}

int map_load_file(const char *filename)
{
	FILE *file = fopen(filename, "rb");
	if (file == NULL) {
		printf("failed to open map file %s\n", filename);
		return 0;
	}
	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	rewind(file);
	if (size > 1000 * 1000) {
		printf("map file over 1MB, refusing to load\n");
		fclose(file);
		return 0;
	}

	char *buf = malloc(size + 1);
	if (!buf) {
		printf("unable to allocate memory for map file buffer\n");
		fclose(file);
		return 0;
	}
	fread(buf, 1, size, file);
	fclose(file);

	cJSON *root = cJSON_Parse(buf);
	free(buf);

	if (!root) {
		printf("invalid json in map file %s\n", filename);
		return 0;
	}
	cJSON *map = cJSON_GetObjectItem(root, "map");

	if (!map) {
		printf("missing map item in map file %s\n", filename);
		return 0;
	}

	map_deserialize(map);

	cJSON *boxes_array = cJSON_GetObjectItem(root, "boxes");
	if (!boxes_array) {
		printf("missing boxes array in %s\n", filename);
		return 0;
	}
	boxes_deserialize(boxes_array);

	cJSON *startpos_array = cJSON_GetObjectItem(root, "starting_positions");
	if (cJSON_GetArraySize(startpos_array) != MAX_CARS) {
		printf("a map needs exactly %d starting positions, got %d\n", MAX_CARS, cJSON_GetArraySize(startpos_array));
		return 0;
	}
	for (int i=0; i < MAX_CARS; ++i) {
		cJSON *cur = cJSON_GetArrayItem(startpos_array, i);
		cJSON *x = cJSON_GetObjectItem(cur, "x");
		if (!x) {
			printf("no x for startpos\n");
			return 0;
		}
		cJSON *y = cJSON_GetObjectItem(cur, "y");
		if (!y) {
			printf("no y for startpos\n");
			return 0;
		}
		map_starting_positions[i].x = x->valueint;
		map_starting_positions[i].y = y->valueint;
	}

	return 1;
}

void map_render(SDL_Renderer *ren)
{
	for (unsigned int x=0; x<map_width; x++) {
		for (unsigned int y=0; y<map_height; y++) {
			SDL_Rect target;
			target.x = x * map_tile_width;
			target.y = y * map_tile_height;
			target.w = map_tile_width;
			target.h = map_tile_height;
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

vec2 map_get_edge_normal(int x, int y)
{
	vec2 normal;
	normal.x = 0;
	normal.y = 0;

	if (x < 0 || y < 0) {
		printf("asked for edge normal for invalid x: %d y: %d\n", x, y);
		return normal;
	}

	const unsigned int px = x / map_tile_width;
	const unsigned int py = y / map_tile_height;
	if (px >= map_width || py >= map_height) {
		printf("asked for edge normal at out of bounds coordinates x: %d y: %d\n", x, y);
		return normal;
	}

	unsigned int rel_x = x - (px * map_tile_width);
	unsigned int rel_y = y - (py * map_tile_height);

	switch (map_tiles[px][py]) {
	case TILE_HORIZONTAL:
		if (rel_y > map_tile_height / 2) {
			normal.x = 0;
			normal.y = -1;
		} else {
			normal.x = 0;
			normal.y = 1;
		}
		return normal;
	case TILE_VERTICAL:
		if (rel_x > map_tile_width / 2) {
			normal.x = -1;
			normal.y = 0;
		} else {
			normal.x = 1;
			normal.y = 0;
		}
		return normal;
	case TILE_UPPERLEFT:
		normal.x = (px + 1) * map_tile_width;
		normal.y = (py + 1) * map_tile_height;
		break;
	case TILE_UPPERRIGHT:
		normal.x = (px) * map_tile_width;
		normal.y = (py + 1) * map_tile_height;
		break;
	case TILE_BOTTOMLEFT:
		normal.x = (px + 1) * map_tile_width;
		normal.y = (py) * map_tile_height;
		break;
	case TILE_BOTTOMRIGHT:
		normal.x = (px) * map_tile_width;
		normal.y = (py) * map_tile_height;
		break;
	case TILE_NONE:
	default:
		break;
	}
	normal.x -= x;
	normal.y -= y;

	return normal;
}

cJSON *map_serialize()
{
	cJSON *map_object = cJSON_CreateObject();
	cJSON_AddNumberToObject(map_object, "tile_width", map_tile_width);
	cJSON_AddNumberToObject(map_object, "tile_height", map_tile_height);

	cJSON *tile_array = cJSON_CreateArray();
	for (unsigned int y=0; y<map_height; y++) {
		cJSON *tile_row = cJSON_CreateArray();
		for (unsigned int x=0; x<map_width; x++) {
			cJSON *tile_item;
			switch(map_tiles[x][y]) {
			case TILE_HORIZONTAL:
				tile_item = cJSON_CreateString("-");
				break;
			case TILE_VERTICAL:
				tile_item = cJSON_CreateString("|");
				break;
			case TILE_UPPERLEFT:
				tile_item = cJSON_CreateString("/");
				break;
			case TILE_UPPERRIGHT:
				tile_item = cJSON_CreateString("`");
				break;
			case TILE_BOTTOMLEFT:
				tile_item = cJSON_CreateString("\\");
				break;
			case TILE_BOTTOMRIGHT:
				tile_item = cJSON_CreateString(",");
				break;
			case TILE_NONE:
			default:
				tile_item = cJSON_CreateString(".");
				break;
			}
			cJSON_AddItemToArray(tile_row, tile_item);
		}
		cJSON_AddItemToArray(tile_array, tile_row);
	}
	cJSON_AddItemToObject(map_object, "tiles", tile_array);

	cJSON *modifiers_array = cJSON_CreateArray();
	for (int i=0; i<modifiers_count; i++) {
		cJSON *modtypestr;
		switch(modifiers[i].type) {
		case MAP_MUD:
			modtypestr = cJSON_CreateString("mud");
			break;
		case MAP_ICE:
			modtypestr = cJSON_CreateString("ice");
			break;
		case MAP_BOOST:
			modtypestr = cJSON_CreateString("booster");
			break;
		default:
			continue;
		}

		cJSON *modifier_object = cJSON_CreateObject();
		cJSON_AddItemToObject(modifier_object, "type", modtypestr);
		cJSON_AddNumberToObject(modifier_object, "x", modifiers[i].rect.x);
		cJSON_AddNumberToObject(modifier_object, "y", modifiers[i].rect.y);
		cJSON_AddNumberToObject(modifier_object, "width", modifiers[i].rect.w);
		cJSON_AddNumberToObject(modifier_object, "height", modifiers[i].rect.h);

		cJSON_AddItemToArray(modifiers_array, modifier_object);
	}
	cJSON_AddItemToObject(map_object, "modifiers", modifiers_array);

	cJSON *path_array = cJSON_CreateArray();
	for (int i=0; i<map_path_length; i++) {
		cJSON *path_item = cJSON_CreateObject();
		cJSON_AddNumberToObject(path_item, "tile_x", map_path[i].x);
		cJSON_AddNumberToObject(path_item, "tile_y", map_path[i].y);
		cJSON_AddItemToArray(path_array, path_item);
	}
	cJSON_AddItemToObject(map_object, "path", path_array);

	return map_object;
}
void map_deserialize(cJSON *root)
{
	cJSON *cur, *tiles, *tile_row, *modifiers, *modifier, *path, *path_item;

	// Read tile size
	cur = cJSON_GetObjectItem(root, "tile_width");
	map_tile_width = cur->valueint;
	cur = cJSON_GetObjectItem(root, "tile_height");
	map_tile_height = cur->valueint;

	// Read map size
	tiles = cJSON_GetObjectItem(root, "tiles");
	map_height = cJSON_GetArraySize(tiles);
	map_width = cJSON_GetArraySize(cJSON_GetArrayItem(tiles, 0));

	// Alocate space for tiles
	map_tiles = malloc((map_width + 1) * sizeof(TileType*));
	if (!map_tiles) {
		printf("failed to allocate %lu bytes for map tiles\n", map_height * sizeof(TileType*));
		return;
	}
	for (unsigned int x = 0; x < map_width; x++) {
		map_tiles[x] = malloc((map_height + 1) * sizeof(TileType));

		if (!map_tiles[x]) {
			printf("failed to allocate %lu bytes of memory for map tile line\n", map_height * sizeof(TileType*));
			return;
		}
	}

	// Parse tiles
	for (unsigned y=0; y<map_height; y++) {
		tile_row = cJSON_GetArrayItem(tiles, y);
		for (unsigned x=0; x<map_width; x++) {
			cur = cJSON_GetArrayItem(tile_row, x);
			const char *type = cur->valuestring;
			switch(type[0]) {
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
			case '.':
			default:
				map_tiles[x][y] = TILE_NONE;
			}
		}
	}

	// Parse modifiers
	modifiers = cJSON_GetObjectItem(root, "modifiers");
	for (int i=0; i<cJSON_GetArraySize(modifiers); i++) {
		modifier = cJSON_GetArrayItem(modifiers, i);

		ivec2 pos;
		cur = cJSON_GetObjectItem(modifier, "x");
		pos.x = cur->valueint;
		cur = cJSON_GetObjectItem(modifier, "y");
		pos.y = cur->valueint;

		cur = cJSON_GetObjectItem(modifier, "type");
		const char *typestr = cur->valuestring;
		if (!strcmp(typestr, "mud")) {
			map_add_modifier(MAP_MUD, pos);
		} else if (!strcmp(typestr, "ice")) {
			map_add_modifier(MAP_ICE, pos);
		} else if (!strcmp(typestr, "booster")) {
			map_add_modifier(MAP_BOOST, pos);
		}
	}

	// Parse path
	path = cJSON_GetObjectItem(root, "path");
	map_path_length = cJSON_GetArraySize(path);
	for (int i=0; i<map_path_length; i++) {
		path_item = cJSON_GetArrayItem(path, i);
		cur = cJSON_GetObjectItem(path_item, "tile_x");
		map_path[i].x = cur->valueint;
		cur = cJSON_GetObjectItem(path_item, "tile_y");
		map_path[i].y = cur->valueint;
	}
}

cJSON *map_items_serialize()
{
	cJSON *item_array = cJSON_CreateArray();
	for (int i=0; i<modifiers_count; i++) {
		cJSON *type_str;
		switch(modifiers[i].type) {
		case MAP_BANANA:
			type_str = cJSON_CreateString("banana");
			break;
		case MAP_OIL:
			type_str = cJSON_CreateString("oil");
			break;
		default:
			continue;
		}

		cJSON *item_object = cJSON_CreateObject();
		cJSON_AddItemToObject(item_object, "type", type_str);
		cJSON_AddNumberToObject(item_object, "x", modifiers[i].rect.x);
		cJSON_AddNumberToObject(item_object, "y", modifiers[i].rect.y);
		cJSON_AddNumberToObject(item_object, "width", modifiers[i].rect.w);
		cJSON_AddNumberToObject(item_object, "height", modifiers[i].rect.h);

		cJSON_AddItemToArray(item_array, item_object);
	}

	return item_array;
}

void map_items_deserialize(cJSON *item_array)
{
	for (int i = 0; i < modifiers_count; ++i) {
		if (modifiers[i].type == MAP_OIL || modifiers[i].type == MAP_BANANA) {
			remove_modifier(i);
		}
	}
	cJSON *cur;
	for (int i=0; i<cJSON_GetArraySize(item_array); i++) {
		cJSON *modifier = cJSON_GetArrayItem(item_array, i);

		ivec2 pos;
		cur = cJSON_GetObjectItem(modifier, "x");
		pos.x = cur->valueint;
		cur = cJSON_GetObjectItem(modifier, "y");
		pos.y = cur->valueint;

		cur = cJSON_GetObjectItem(modifier, "type");
		const char *typestr = cur->valuestring;
		if (!strcmp(typestr, "oil")) {
			map_add_modifier(MAP_OIL, pos);
		} else if (!strcmp(typestr, "banana")) {
			map_add_modifier(MAP_BANANA, pos);
		}
	}
}

int map_dist_left_in_tile(int pathcount, vec2 pos)
{
	pathcount++;
	pathcount %= map_path_length;

	const unsigned int next_x = map_path[pathcount].x;
	const unsigned int next_y = map_path[pathcount].y;

	const unsigned int cur_tilex = pos.x / map_tile_width;
	const unsigned int cur_tiley = pos.y / map_tile_height;

	if (next_x == cur_tilex) {
		if (next_y < cur_tiley) {
			return (pos.y - next_y * map_tile_height);
		} else {
			return (next_y * map_tile_height - pos.y);
		}
	} else {
		if (next_x < cur_tilex) {
			return (pos.x - next_x * map_tile_width);
		} else {
			return (next_x * map_tile_width - pos.x);
		}
	}
}

/* vim: set ts=8 sw=8 tw=0 noexpandtab cindent softtabstop=8 :*/
