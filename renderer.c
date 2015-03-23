#include "renderer.h"
#include "vector.h"
#include "defines.h"

SDL_Renderer *ren = 0;

SDL_Texture *font_texture = 0;
SDL_Texture *background_texture = 0;
ivec2 background_dims;

extern int screen_height, screen_width;

SDL_Texture *ren_load_image(const char *file)
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

SDL_Texture *ren_load_image_with_dims(const char *file, int *w, int *h)
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

int renderer_init(SDL_Renderer *renderer)
{
    ren = renderer;

    font_texture = ren_load_image("font.bmp");
    if (!font_texture) {
        printf("failed to load font\n");
    }

    background_texture = ren_load_image_with_dims("background.bmp", &background_dims.x, &background_dims.y);


    return (font_texture != 0);
}

void render_string(const char *string, int x, int y, int size)
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

void render_background()
{
    if (!background_texture) return;
    const Uint32 t = SDL_GetTicks() / 66 % background_dims.y;
    for (int x=-1; x<screen_width / background_dims.x + 1; x++) {
        for (int y=0; y<=screen_height / background_dims.y; y++) {
            SDL_Rect target;
            target.x = x * background_dims.x + t;
            target.y = y * background_dims.y;
            target.w = background_dims.x;
            target.h = background_dims.y;
            SDL_RenderCopy(ren, background_texture, 0, &target);
        }
    }
}

void render_time(Uint32 time, int x, int y, int size)
{
    Uint32 m = time / 60000;
    Uint32 s = (time / 1000) % 60;
    Uint32 ms = (time / 10) % 100;
    char *time_string = malloc(500);
    snprintf(time_string, 500, "%01d:%02d.%02d", m, s, ms);
    render_string(time_string, x, y, size);
    free(time_string);
}
