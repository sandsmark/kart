#include "renderer.h"

SDL_Texture *ren_load_image(SDL_Renderer *ren, const char *file)
{
	const char *prefix = "assets/";
	char path[256];
	snprintf(path, sizeof(path), "%s%s", prefix, file);
	SDL_Surface *surface = SDL_LoadBMP(path);
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

SDL_Texture *ren_load_image_with_dims(SDL_Renderer *ren, const char *file, int *w, int *h)
{
	const char *prefix = "assets/";
	char path[256];
	snprintf(path, sizeof(path), "%s%s", prefix, file);
	SDL_Surface *surface = SDL_LoadBMP(path);
	if (surface == NULL) {
		printf("SDL error while loading BMP %s: %s\n", file, SDL_GetError());
		return 0;
	}
	*w = surface->w;
	*h = surface->h;
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
