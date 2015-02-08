#ifndef POWERUP_H
#define POWERUP_H
#include <SDL2/SDL.h>
#include "vector.h"

typedef enum {
    POWERUP_NONE = 0,
    POWERUP_BANANA,
    POWERUP_GREEN_SHELL,
    POWERUP_RED_SHELL,
    POWERUP_BLUE_SHELL,
    POWERUP_OIL,
    POWERUP_MUSHROOM,
    POWERUP_GOLD_MUSHROOM,
    POWERUP_BIG_MUSHROOM,
    POWERUP_LIGHTNING,
    POWERUP_STAR
} PowerUp;

#define POWERUPS_WIDTH 32
#define POWERUPS_HEIGHT 32

int powerups_init();

void powerup_render(SDL_Renderer *ren, PowerUp type, ivec2 pos);
void powerup_trigger(PowerUp type, vec2 pos, vec2 direction);

#endif//POWERUP_H
