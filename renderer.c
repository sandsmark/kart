#include "renderer.h"


SDL_Texture *font_texture = 0;

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

int renderer_init(SDL_Renderer *ren)
{
    font_texture = ren_load_image(ren, "font.bmp");
    if (!font_texture) {
        printf("failed to load font\n");
    }

    return (font_texture != 0);
}

void render_string(SDL_Renderer *ren, const char *string, int x, int y, int size)
{
    for (unsigned int i=0; i<strlen(string); i++) {
        if (string[i] < 32 || string[i] > 126) continue;

        int offset = (string[i] - ' ') * 6;
        SDL_Rect source_rect;
        source_rect.y = 0;
        source_rect.x = offset;
        source_rect.w = 6;
        source_rect.h = 11;

        SDL_Rect target_rect;
        target_rect.y = y;
        target_rect.x = x + i*size;
        target_rect.w = size;
        target_rect.h = size;

        SDL_RenderCopy(ren, font_texture, &source_rect, &target_rect);
    }
}
