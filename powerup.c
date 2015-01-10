#include "powerup.h"
#include "common.h"
#include "map.h"

// none doesnæt have an image
static SDL_Texture *textures[POWERUP_STAR+1] = {0};

void powerup_render(SDL_Renderer *ren, PowerUp type, ivec2 pos)
{
        SDL_Rect target;
        target.x = pos.x;
        target.y = pos.y;
        target.w = POWERUPS_WIDTH;
        target.h = POWERUPS_HEIGHT;
        SDL_RenderCopy(ren, textures[type], 0, &target);
}

int powerups_init(SDL_Renderer *ren)
{
    textures[POWERUP_NONE] = load_image(ren, "none_powerup.bmp");
    textures[POWERUP_BANANA] = load_image(ren, "banana.bmp");
    textures[POWERUP_GREEN_SHELL] = load_image(ren, "green_shell.bmp");
    textures[POWERUP_RED_SHELL] = load_image(ren, "red_shell.bmp");
    textures[POWERUP_BLUE_SHELL] = load_image(ren, "blue_shell.bmp");
    textures[POWERUP_OIL] = load_image(ren, "oil.bmp");
    textures[POWERUP_MUSHROOM] = load_image(ren, "mushroom.bmp");
    textures[POWERUP_GOLD_MUSHROOM] = load_image(ren, "gold_mushroom.bmp");
    textures[POWERUP_BIG_MUSHROOM] = load_image(ren, "big_mushroom.bmp");
    textures[POWERUP_LIGHTNING] = load_image(ren, "lightning.bmp");
    textures[POWERUP_STAR] = load_image(ren, "star.bmp");

    for (int i=0; i<POWERUP_STAR; i++) {
        if (!textures[i]) {
            return 0;
        }
    }

    return 1;
}

void powerup_trigger(vec2 pos, PowerUp type)
{
    ivec2 pos_integer;
    pos_integer.x = pos.x;
    pos_integer.y = pos.y;
    switch(type) {
    case POWERUP_NONE:
        return;
    case POWERUP_BANANA:
        map_add_modifier(MAP_BANANA, pos_integer);
        break;
    case POWERUP_GREEN_SHELL:
        break;
    case POWERUP_RED_SHELL:
        break;
    case POWERUP_BLUE_SHELL:
        break;
    case POWERUP_OIL:
        map_add_modifier(MAP_OIL, pos_integer);
        break;
    case POWERUP_MUSHROOM:
        break;
    case POWERUP_GOLD_MUSHROOM:
        break;
    case POWERUP_BIG_MUSHROOM:
        break;
    case POWERUP_LIGHTNING:
        break;
    case POWERUP_STAR:
        break;
    default:
        printf("tried to trigger unknown powerup: %d\n", type);
        break;
    }
}
