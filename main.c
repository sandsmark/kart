#include <SDL2/SDL.h>

#include "car.h"
#include "map.h"
#include "net.h"
#include "vector.h"

const int SCREEN_WIDTH  = 1024;
const int SCREEN_HEIGHT = 768;
const vec2 start = {1.0, 0.0};

static int sockfd;

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
	target.x = car->pos.x;
	target.y = car->pos.y;
	target.w = car->width;
	target.h = car->height;
	SDL_RenderCopyEx(ren, car->texture, 0, &target, vec_angle(start, car->direction), 0, 0);
}

void draw_circle(SDL_Surface *surface, int cx, int cy, int radius, Uint8 pixel)
{
	// Note that there is more to altering the bitrate of this 
	// method than just changing this value.  See how pixels are
	// altered at the following web page for tips:
	//   http://www.libsdl.org/intro.en/usingvideo.html
	static const int BPP = 1;

	double r = (double)radius;

	for (double dy = 1; dy <= r; dy += 1.0)
	{
		// This loop is unrolled a bit, only iterating through half of the
		// height of the circle.  The result is used to draw a scan line and
		// its mirror image below it.

		// The following formula has been simplified from our original.  We
		// are using half of the width of the circle because we are provided
		// with a center and we need left/right coordinates.

		double dx = floor(sqrt((2.0 * r * dy) - (dy * dy)));
		int x = cx - dx;

		// Grab a pointer to the left-most pixel for each half of the circle
		Uint8 *target_pixel_a = (Uint8 *)surface->pixels + ((int)(cy + r - dy)) * surface->pitch + x * sizeof(Uint8);
		Uint8 *target_pixel_b = (Uint8 *)surface->pixels + ((int)(cy - r + dy)) * surface->pitch + x * sizeof(Uint8);

		for (; x <= cx + dx; x++)
		{
			*target_pixel_a = pixel;
			*target_pixel_b = pixel;
			target_pixel_a += BPP;
			target_pixel_b += BPP;
		}
	}
}

int run_game(SDL_Renderer *ren)
{
	if (!map_init(ren, "map1.map")) {
		printf("unable to initialize map!\n");
		return 1;
	}

	// Create cars
	int car_count = 2;
	Car *cars = calloc(car_count, sizeof(Car));

	for (int i=0; i<car_count; i++) {
		// Initialize car
		cars[i].pos.x = 250;
		cars[i].pos.y = 30 + i*20;
		cars[i].direction.x = start.x;
		cars[i].direction.y = start.y;

		char filename[10];
		sprintf(filename, "car%d.bmp", i);
		SDL_Surface *image = SDL_LoadBMP(filename);
		if (image == NULL) {
			printf("SDL error while loading BMP: %s\n", SDL_GetError());
			return 0;
		}
		cars[i].width = image->w;
		cars[i].height = image->h;

		SDL_SetColorKey(image, SDL_TRUE, SDL_MapRGB(image->format, 0, 255, 0));

		cars[i].texture = SDL_CreateTextureFromSurface(ren, image);
		SDL_FreeSurface(image);

		if (cars[i].texture == NULL) {
			printf("SDL error while creating texture: %s\n", SDL_GetError());
			return 1;
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
			Car *car = &cars[human_player];
			if (keystates[SDL_SCANCODE_UP]) {
				vec2 force = car->direction;
				vec_scale(&force, 2500);
				car_apply_force(car, force);
			}
			if (keystates[SDL_SCANCODE_DOWN]) {
				vec2 force = car->direction;
				vec_scale(&force, -2500);
				car_apply_force(car, force);
			}
			if (keystates[SDL_SCANCODE_LEFT]) {
				vec_rotate(&car->direction, -3);
			}
			if (keystates[SDL_SCANCODE_RIGHT]) {
				vec_rotate(&car->direction, 3);
			}
			if (keystates[SDL_SCANCODE_SPACE]) {
				car->drift = 1;
			}
		}

		map_render(ren);

		for (int i=0; i<car_count; i++) {
			for (int j=i+1; j<car_count; j++)
			{
				car_collison(&cars[i], &cars[j]);
			}
			car_move(&cars[i]);
			render_car(ren, &cars[i]);
			memset(&cars[i].force, 0, sizeof(cars[i].force));
		}

		SDL_RenderPresent(ren);
	}

	// Clean up
	map_destroy();
	for (int i=0; i<car_count; i++) {
		SDL_DestroyTexture(cars[i].texture);
	}
	free(cars);
	return 0;
}

void show_menu(SDL_Renderer *ren)
{
	SDL_Surface *surface = SDL_LoadBMP("startscreen.bmp");
	if (surface == NULL) {
		printf("SDL error while loading BMP: %s\n", SDL_GetError());
		return;
	}
	SDL_Texture *image = SDL_CreateTextureFromSurface(ren, surface);
	if (image == NULL) {
		printf("SDL error while creating start screen texture from image: %s\n", SDL_GetError());
		SDL_FreeSurface(surface);
		return;
	}
	SDL_FreeSurface(surface);
	SDL_Event event;
	SDL_Rect target;
	target.x = 0;
	target.y = 0;
	target.w = SCREEN_WIDTH;
	target.h = SCREEN_HEIGHT;
	int quit = 0;
	int choice = 0;
	while (!quit) {
		while (SDL_PollEvent(&event)){
			//If user closes the window
			if (event.type == SDL_QUIT) {
				return;
			}
			//If user presses any key
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_RETURN:
					if (choice != 1) {
						quit = 1;
					}
					break;
				case SDLK_DOWN:
					choice = (choice + 1) % 3;
					break;
				case SDLK_UP:
					choice--;
					if (choice < 0) choice = 2;
					break;
				}
			}
		}


		SDL_RenderCopy(ren, image, 0, &target);

		                          //r    g     b     a
		SDL_SetRenderDrawColor(ren, 0x0, 0xff, 0xff, 0xff);
		SDL_Rect selection_rect;
		selection_rect.x = 364;
		selection_rect.y = 320 + choice * 62;
		selection_rect.h = 10;
		selection_rect.w = 10;
		SDL_RenderFillRect(ren, &selection_rect);

		SDL_RenderPresent(ren);
	}

	SDL_SetRenderDrawColor(ren, 0x0, 0x0, 0x0, 0xff);
	SDL_RenderClear(ren);
	if (choice == 0) {
		run_game(ren);
	}
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		sockfd = start_server(54321);
	} else {

		if (strcmp(argv[1], "server") == 0)
		{
			if (argc != 3)
			{
				printf("Usage: %s server <port>\n", argv[0]);
				return 1;
			}
			sockfd = start_server(atoi(argv[2]));
		}
		else if (strcmp(argv[1], "client") == 0)
		{
			if (argc != 4)
			{
				printf("Usage: %s client <address> <port>\n", argv[0]);
				return 1;
			}
			if (strcmp(argv[2], "localhost") == 0)
				sockfd = start_client("127.0.0.1", atoi(argv[3]));
			else
				sockfd = start_client(argv[2], atoi(argv[3]));
		}
		else if (strcmp(argv[1], "local") == 0)
		{
			if (argc != 2)
			{
				printf("Usage: %s local\n", argv[0]);
				return 1;
			}
		}
		else
		{
			printf("Invalid argument: %s\n", argv[1]);
		}
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

	show_menu(ren);

	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}

/* vim: set ts=8 sw=8 tw=0 noexpandtab cindent softtabstop=8 :*/
