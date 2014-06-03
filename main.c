#include <SDL2/SDL.h>
#include <math.h>

#include "car.h"

const int SCREEN_WIDTH  = 1024;
const int SCREEN_HEIGHT = 768;

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
	point.x = cos(angle * PI / 180) * (x - center.x) - sin(angle * PI / 180) * (y - center.y) + x;
	point.y = sin(angle * PI / 180) * (x - center.x) + cos(angle * PI / 180) * (y - center.y) + y;
	return point;
}

int is_on_track(const Point pos, const SDL_Surface *map)
{
	if (pos.x < 0 || pos.y < 0 || pos.x >= map->w || pos.y >= map->h) {
		return 0;
	}

	int bpp = map->format->BytesPerPixel;
	Uint8 r, g, b;
	Uint32 pixel;
	Uint8 *p = (Uint8 *)map->pixels + pos.y * map->pitch + pos.x * bpp;
	switch(bpp) {
		case 1:
			pixel = *p;
			break;

		case 2:
			pixel = *(Uint16 *)p;
			break;

		case 3:
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
				pixel = p[0] << 16 | p[1] << 8 | p[2];
			else
				pixel= p[0] | p[1] << 8 | p[2] << 16;
			break;

		case 4:
			pixel = *(Uint32 *)p;
			break;

		default:
			printf("unsupported format for map\n");
			exit(1);
			pixel = 0;       /* shouldn't happen, but avoids warnings */
	}

	SDL_GetRGB(pixel, map->format, &r, &g, &b);

	if (r == 0 && g == 0 && b == 0) {
		return 1;
	} else {
		return 0;
	}
}

int check_car_off_track(Car *car, SDL_Surface *map)
{
	Point center;
	center.x = car->x + car->width/2;
	center.y = car->y + car->height/2;

	if (!is_on_track(rotate_point(car->x, car->y, center, car->angle), map)) {
		return 1;
	}

	if (!is_on_track(rotate_point(car->x + car->width, car->y, center, car->angle), map)) {
		return 1;
	}

	if (!is_on_track(rotate_point(car->x, car->y + car->height, center, car->angle), map)) {
		return 1;
	}

	if (!is_on_track(rotate_point(car->x + car->width, car->y + car->height, center, car->angle), map)) {
		return 1;
	}

	return 0;
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

	// Penalize for driving off piste
	if (check_car_off_track(car, map)) {
		if (car->speed > 1) {
			car->speed = 1;
		}
	}

	// Update position
	car->x += car->speed * cos(PI * car->angle / 180);
	car->y += car->speed * sin(PI * car->angle / 180);

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
	int car_count = 1;
	Car *cars = malloc(sizeof(Car) * car_count);

	for (int i=0; i<car_count; i++) {
		// Initialize car
		cars[i].x = 0;
		cars[i].y = 0;
		cars[i].angle = 45;
		cars[i].speed = 0;

		SDL_Surface *image = SDL_LoadBMP("car1.bmp");
		if (image == NULL) {
			printf("SDL error while loading BMP: %s\n", SDL_GetError());
			return 0;
		}
		cars[i].width = image->w;
		cars[i].height = image->h;

		printf("width: %d height: %d\n", image->w, image->h);

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
