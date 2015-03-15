#ifndef BOX_H
#define BOX_H

#include <SDL2/SDL.h>
#include "powerup.h"
#include "vector.h"
#include "libs/cJSON/cJSON.h"

int boxes_init();
void boxes_destroy();
void boxes_render(SDL_Renderer *ren);

PowerUp boxes_check_hit(SDL_Rect car);

cJSON *boxes_serialize();
void boxes_deserialize(cJSON *boxes);

#endif//BOX_H
