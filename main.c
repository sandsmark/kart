#include <SDL2/SDL.h>
#include <math.h>

#include "car.h"

const int SCREEN_WIDTH  = 1024;
const int SCREEN_HEIGHT = 768;

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

// Not defined with ansi C
#define PI 3.14159265

SDL_Texture *load_texture(SDL_Renderer *renderer, const char *filepath)
{
	SDL_Surface *image = SDL_LoadBMP(filepath);
	if (image == 0) {
		printf("SDL error while loading BMP: %s\n", SDL_GetError());
		return 0;
	}

	SDL_SetColorKey(image, SDL_TRUE, SDL_MapRGB(image->format, 0, 255, 0));

	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, image);
	SDL_FreeSurface(image);

	if (texture == 0) {
		printf("SDL error while creating texture: %s\n", SDL_GetError());
	}

	return texture;
}

typedef struct {
	int x;
	int y;
} Point;

Point rotate_point(int x, int y, const Point center, int angle)
{
	Point point;
	point.x = cos(angle * PI / 180) * (x - center.x) - sin(angle * PI / 180) * (y - center.y) + center.x;
	point.y = sin(angle * PI / 180) * (x - center.x) + cos(angle * PI / 180) * (y - center.y) + center.y;
	return point;
}

AreaType map_get_type(const Point pos, const SDL_Surface *map)
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

void render_car(SDL_Renderer *ren, Car *car)
{
	SDL_Rect target;
	target.x = car->x;
	target.y = car->y;
	target.w = car->width;
	target.h = car->height;
	SDL_RenderCopyEx(ren, car->texture, 0, &target, car->angle, 0, 0);
}

void car_move(Car *car, SDL_Surface *map)
{

	// Normalize rotation and speed
	if (car->speed > 5) {
		car->speed = 5;
	}
	if (car->speed < -2) {
		car->speed = -2;
	}
	if (car->angle < 0) {
		car->angle += 360;
	}
	if (car->angle > 360) {
		car->angle -= 360;
	}

	// Calculate next position
	int newX = car->x + car->speed * cos(PI * car->angle / 180);
	int newY = car->y + car->speed * sin(PI * car->angle / 180);

	// Check if we're passing over something funny
	Point center;
	center.x = newX + car->width/2;
	center.y = newY + car->height/2;
	for (int i=0; i<4; i++) {
		AreaType type = map_get_type(rotate_point(newX + car->wheelX[i], newY + car->wheelY[i], center, car->angle), map);

		switch(type){
		case MAP_GRASS:
			if (car->speed > 1) {
				car->speed = 1;
			}
			break;
		case MAP_BOOST:
			car->speed *= 2;
			break;
		case MAP_MUD:
			if (car->speed > 2) {
				car->speed = 2;
			}
			break;
		case MAP_WALL:
			//TODO moar stuffs
			car->speed = -car->speed;
			car->angle = car->oldAngle;
			i=5;
			break;
		case MAP_OIL:
			car->angle += 3;
			car->speed++;
			break;
		case MAP_ICE:
			if ((rand() % 2) == 1) {
				car->angle += 2;
			} else {
				car->angle -= 2;
			}
			break;
		default:
			break;
		}
	}

	car->x += car->speed * cos(PI * car->angle / 180);
        car->y += car->speed * sin(PI * car->angle / 180);

	car->oldAngle = car->angle;

	// Bounds checking
	if (car->x < 0) {
		car->speed = -1;
		car->x = 0;
	}
	if (car->x > map->w - car->width) {
		car->speed = -1;
		car->x = map->w - car->width;
	}

	if (car->y < 0) {
		car->speed = -1;
		car->y = 0;
	}
	if (car->y > map->h - car->height) {
		car->speed = -1;
		car->y = map->h - car->height;
	}
}

int main(int argc, char *argv[])
{
	if (argc > 1) {
		printf("Usage: %s\n", argv[0]);
		return 1;
	}

	// Set up SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		printf("SDL init failed: %s\n", SDL_GetError());
		return 1;
	}
	SDL_Window *win = SDL_CreateWindow("The Kartering", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (win == NULL){
		printf("SDL error creating window: %s\n", SDL_GetError());
		return 1;
	}
	SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (ren == NULL){
		printf("SDL error while creating renderer: %s\n", SDL_GetError());
		return 1;
	}

	// Load map
	SDL_Surface *map = SDL_LoadBMP("background.bmp");
	if (map == NULL) {
		printf("SDL error while loading BMP: %s\n", SDL_GetError());
		return 1;
	}
	SDL_Texture *mapTexture = SDL_CreateTextureFromSurface(ren, map);
	if (mapTexture == NULL) {
		printf("SDL error while creating map texture: %s\n", SDL_GetError());
		return 1;
	}

	// Create cars
	int car_count = 2;
	Car *cars = malloc(sizeof(Car) * car_count);

	for (int i=0; i<car_count; i++) {
		// Initialize car
		cars[i].x = 250;
		cars[i].y = 30 + i*20;
		cars[i].angle = 0;
		cars[i].oldAngle = 0;
		cars[i].speed = 0;

		SDL_Surface *image = SDL_LoadBMP("car1.bmp");
		if (image == NULL) {
			printf("SDL error while loading BMP: %s\n", SDL_GetError());
			return 0;
		}
		cars[i].width = image->w;
		cars[i].height = image->h;

		// Store wheel positions individually
		cars[i].wheelX[0] = 0;
		cars[i].wheelY[0] = 0;
		cars[i].wheelX[1] = cars[i].width;
		cars[i].wheelY[1] = 0;
		cars[i].wheelX[2] = 0;
		cars[i].wheelY[2] = cars[i].height;
		cars[i].wheelX[3] = cars[i].width;
		cars[i].wheelY[3] = cars[i].height;

		SDL_SetColorKey(image, SDL_TRUE, SDL_MapRGB(image->format, 0, 255, 0));

		cars[i].texture = SDL_CreateTextureFromSurface(ren, image);
		SDL_FreeSurface(image);

		if (cars[i].texture == NULL) {
			printf("SDL error while creating texture: %s\n", SDL_GetError());
			return 0;
		}
	}

	int human_player = 0;

	int quit = 0;
	SDL_Event event;

	while (!quit) {
		while (SDL_PollEvent(&event)){
			//If user closes the window
			if (event.type == SDL_QUIT) {
				quit = 1;
			}
			//If user presses any key
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					quit = 1;
					break;
				}
			}
		}
		if (human_player != -1) {
			const Uint8 *keystates = SDL_GetKeyboardState(NULL);
			if (keystates[SDL_SCANCODE_UP]) {
				cars[human_player].speed += 3;
			}
			if (keystates[SDL_SCANCODE_DOWN]) {
				cars[human_player].speed -= 3;
			}
			if (keystates[SDL_SCANCODE_LEFT]) {
				cars[human_player].angle -= 5;
			}
			if (keystates[SDL_SCANCODE_RIGHT]) {
				cars[human_player].angle += 5;
			}
		}

		SDL_RenderClear(ren);
		SDL_Rect rect;
		rect.x = 0;
		rect.y = 0;
		rect.w = map->w;
		rect.h = map->h;
		SDL_RenderCopy(ren, mapTexture, NULL, &rect);

		for (int i=0; i<car_count; i++) {
			car_move(&cars[i], map);
			render_car(ren, &cars[i]);
		}

		SDL_RenderPresent(ren);
	}


	// Clean up
	SDL_FreeSurface(map);
	SDL_DestroyTexture(mapTexture);
	for (int i=0; i<car_count; i++) {
		SDL_DestroyTexture(cars[i].texture);
	}
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();
}

/* vim: set ts=8 sw=8 tw=0 noexpandtab cindent softtabstop=8 :*/
