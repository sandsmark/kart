#ifndef BOX_H
#define BOX_H

#include <SDL2/SDL.h>
#include "powerup.h"
#include "vector.h"

int boxes_init(SDL_Renderer *ren);
void boxes_render(SDL_Renderer *ren);

PowerUp boxes_check_hit(vec2 position);

#endif//BOX_H
