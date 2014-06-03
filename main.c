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

int is_on_track(const Point pos, const SDL_Surface *background)
{
	if (pos.x < 0 || pos.y < 0 || pos.x >= background->w || pos.y >= background->h) {
		return 0;
	}

	int bpp = background->format->BytesPerPixel;
	Uint8 r, g, b;
	Uint32 pixel;
	Uint8 *p = (Uint8 *)background->pixels + pos.y * background->pitch + pos.x * bpp;
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
			printf("unsupported format for background\n");
			exit(1);
			pixel = 0;       /* shouldn't happen, but avoids warnings */
	}

	SDL_GetRGB(pixel, background->format, &r, &g, &b);

	if (r == 0 && g == 0 && b == 0) {
		return 1;
	} else {
		return 0;
	}
}

int check_car_off_track(Car *car, SDL_Surface *background)
{
	Point center;
	center.x = car->rect.x + car->rect.w/2;
	center.y = car->rect.y + car->rect.h/2;

	if (!is_on_track(rotate_point(car->rect.x, car->rect.y, center, car->angle), background)) {
		return 1;
	}

	if (!is_on_track(rotate_point(car->rect.x + car->rect.w, car->rect.y, center, car->angle), background)) {
		return 1;
	}

	if (!is_on_track(rotate_point(car->rect.x, car->rect.y + car->rect.h, center, car->angle), background)) {
		return 1;
	}

	if (!is_on_track(rotate_point(car->rect.x + car->rect.w, car->rect.y + car->rect.h, center, car->angle), background)) {
		return 1;
	}

	return 0;
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

	// Load background
	SDL_Surface *background = SDL_LoadBMP("background.bmp");
	if (background == 0) {
		printf("SDL error while loading BMP: %s\n", SDL_GetError());
		return 0;
	}
	SDL_Texture *backgroundTexture = SDL_CreateTextureFromSurface(ren, background);
	if (backgroundTexture == 0) {
		printf("SDL error while creating background texture: %s\n", SDL_GetError());
	}

	// Initialize car
	Car car;
	car.rect.x = SCREEN_WIDTH / 4;
	car.rect.y = SCREEN_HEIGHT / 4;
	car.angle = 45;
	car.speed = 0;
	car.texture = load_texture(ren, "car1.bmp");
	SDL_QueryTexture(car.texture, 0, 0, &car.rect.w, &car.rect.h);
	if (backgroundTexture == 0 || car.texture == 0) {
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
					car.speed++;
					break;
				case SDLK_DOWN:
					car.speed--;
					break;
				case SDLK_LEFT:
					car.angle -= 5;
					break;
				case SDLK_RIGHT:
					car.angle += 5;
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
		if (car.speed > 5) {
			car.speed = 5;
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

		// Penalize for driving off piste
		if (check_car_off_track(&car, background)) {
			if (car.speed > 2) {
				car.speed = 2;
			}
		}


		// Update position
		car.rect.x += car.speed * cos(PI * car.angle / 180);
		car.rect.y += car.speed * sin(PI * car.angle / 180);

		// Bounds checking
		if (car.rect.x < 0) {
			car.speed = -1;
			car.rect.x = 0;
		}
		if (car.rect.x > SCREEN_WIDTH - car.rect.w) {
			car.speed = -1;
			car.rect.x = SCREEN_WIDTH - car.rect.w;
		}

		if (car.rect.y < 0) {
			car.speed = -1;
			car.rect.y = 0;
		}
		if (car.rect.y > SCREEN_HEIGHT - car.rect.h) {
			car.speed = -1;
			car.rect.y = SCREEN_HEIGHT - car.rect.h;
		}

		// Render
		SDL_RenderClear(ren);
		SDL_RenderCopy(ren, backgroundTexture, NULL, NULL);
		SDL_RenderCopyEx(ren, car.texture, 0, &car.rect, car.angle, 0, 0);
		SDL_RenderPresent(ren);
	}


	// Clean up
	SDL_FreeSurface(background);
	SDL_DestroyTexture(backgroundTexture);
	SDL_DestroyTexture(car.texture);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();
}

/* vim: set ts=8 sw=8 tw=0 noexpandtab cindent softtabstop=8 :*/
