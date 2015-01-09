#include "common.h"

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
		printf("SDL error while creating texture from %s: %s\n", file, SDL_GetError());
		SDL_FreeSurface(surface);
		return 0;
	}
	SDL_FreeSurface(surface);
	return texture;
}

