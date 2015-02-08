#ifndef COMMON_H
#define COMMON_H

#include <SDL2/SDL.h>

SDL_Texture *ren_load_image(const char *file);
SDL_Texture *ren_load_image_with_dims(const char *file, int *w, int *h);

int renderer_init(SDL_Renderer *ren);
void render_string(const char *string, int x, int y, int size);

#endif//COMMON_H

