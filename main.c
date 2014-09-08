#include <SDL2/SDL.h>

#include "car.h"
#include "map.h"

const int SCREEN_WIDTH  = 1024;
const int SCREEN_HEIGHT = 768;

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

void render_car(SDL_Renderer *ren, Car *car)
{
	SDL_Rect target;
	target.x = car->x;
	target.y = car->y;
	target.w = car->width;
	target.h = car->height;
	SDL_RenderCopyEx(ren, car->texture, 0, &target, car->angle, 0, 0);
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
	SDL_Window *win = SDL_CreateWindow("The Kartering", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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
		cars[i].speed = 0;
		cars[i].acceleration = 0;

		char filename[10];
		sprintf(filename, "car%d.bmp", i);
		SDL_Surface *image = SDL_LoadBMP(filename);
		if (image == NULL) {
			printf("SDL error while loading BMP: %s\n", SDL_GetError());
			return 0;
		}
		cars[i].width = image->w;
		cars[i].height = image->h;

		// Store wheel positions individually
		cars[i].wheel_x[0] = 0;
		cars[i].wheel_y[0] = 0;
		cars[i].wheel_x[1] = cars[i].width;
		cars[i].wheel_y[1] = 0;
		cars[i].wheel_x[2] = 0;
		cars[i].wheel_y[2] = cars[i].height;
		cars[i].wheel_x[3] = cars[i].width;
		cars[i].wheel_y[3] = cars[i].height;

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
				cars[human_player].acceleration = 10;
			}
			if (keystates[SDL_SCANCODE_DOWN]) {
				cars[human_player].acceleration = -10;
			}
			if (keystates[SDL_SCANCODE_LEFT]) {
				if (cars[human_player].speed > 0)
				cars[human_player].angle -= 5;
			}
			if (keystates[SDL_SCANCODE_RIGHT]) {
				if (cars[human_player].speed > 0)
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
			move_car(&cars[i], map);
			friction(&cars[i]);
			cars[i].acceleration = 0;
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
	return 0;
}

/* vim: set ts=8 sw=8 tw=0 noexpandtab cindent softtabstop=8 :*/
