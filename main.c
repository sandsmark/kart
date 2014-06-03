#include <SDL2/SDL.h>
#include <math.h>

#include "car.h"

const int SCREEN_WIDTH  = 640;
const int SCREEN_HEIGHT = 480;

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

void render_texture(SDL_Renderer *renderer, SDL_Texture *texture, int x, int y, int angle)
{
	SDL_Rect dest;
	dest.x = x;
	dest.y = y;
	SDL_QueryTexture(texture, 0, 0, &dest.w, &dest.h);
	SDL_RenderCopyEx(renderer, texture, 0, &dest, angle, 0, 0);
}

int main(int argc, char *argv[])
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		printf("SDL init failed: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Window *win = SDL_CreateWindow("The Kartering", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
	if (win == 0){
		printf("SDL error creating window: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (ren == 0){
		printf("SDL error while creating renderer: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Texture *background = load_texture(ren, "background.bmp");

	Car car;
	car.x = SCREEN_WIDTH / 2;
	car.y = SCREEN_HEIGHT / 2;
	car.angle = 45;
	car.texture = load_texture(ren, "car1.bmp");
	if (background == 0 || car.texture == 0) {
		return 1;
	}

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
				case SDLK_UP:
					car.speed += 1;
					break;
				case SDLK_DOWN:
					car.speed -= 1;
					break;
				case SDLK_LEFT:
					car.angle -= 10;
					break;
				case SDLK_RIGHT:
					car.angle += 10;
					break;
				case SDLK_ESCAPE:
					quit = 1;
					break;
				}
			}
			//If user clicks the mouse
			if (event.type == SDL_MOUSEBUTTONDOWN) {
				quit = 1;
			}
		}


		// Normalize rotation and speed
		if (car.speed > 10) {
			car.speed = 10;
		}
		if (car.speed < -2) {
			car.speed = -2;
		}
		if (car.angle < 0) {
			car.angle += 360;
		}
		if (car.angle > 360) {
			car.angle -= 360;
		}

		// Update position
		car.x += car.speed * cos(car.angle * PI / 180);
		car.y += car.speed * sin(car.angle * PI / 180);

		// Bounds checking
		if (car.x < 0) {
			car.speed = -1;
			car.x = 0;
		}
		if (car.x > SCREEN_WIDTH - 64) {
			car.speed = -1;
			car.x = SCREEN_WIDTH - 64;
		}

		if (car.y < 0) {
			car.speed = -1;
			car.y = 0;
		}
		if (car.y > SCREEN_HEIGHT - 64) {
			car.speed = -1;
			car.y = SCREEN_HEIGHT - 64;
		}

		// Render
		SDL_RenderClear(ren);
		SDL_RenderCopy(ren, background, NULL, NULL);
		render_texture(ren, car.texture, car.x, car.y, car.angle);
		SDL_RenderPresent(ren);
	}


	// Clean up
	SDL_DestroyTexture(background);
	SDL_DestroyTexture(car.texture);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();
}

/* vim: set ts=8 sw=8 tw=0 noexpandtab cindent softtabstop=8 :*/
