#include "map.h"
#include "powerup.h"
#include "renderer.h"
#include "shell.h"

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

int powerups_init()
{
    textures[POWERUP_NONE]          = ren_load_image("none_powerup.bmp");
    textures[POWERUP_BANANA]        = ren_load_image("banana.bmp");
    textures[POWERUP_GREEN_SHELL]   = ren_load_image("green_shell.bmp");
    textures[POWERUP_RED_SHELL]     = ren_load_image("red_shell.bmp");
    textures[POWERUP_BLUE_SHELL]    = ren_load_image("blue_shell.bmp");
    textures[POWERUP_OIL]           = ren_load_image("oil.bmp");
    textures[POWERUP_MUSHROOM]      = ren_load_image("mushroom.bmp");
    textures[POWERUP_GOLD_MUSHROOM] = ren_load_image("gold_mushroom.bmp");
    textures[POWERUP_BIG_MUSHROOM]  = ren_load_image("big_mushroom.bmp");
    textures[POWERUP_LIGHTNING]     = ren_load_image("lightning.bmp");
    textures[POWERUP_STAR]          = ren_load_image("star.bmp");

    for (int i=0; i<POWERUP_STAR; i++) {
        if (!textures[i]) {
            return 0;
        }
    }

    return 1;
}

void powerup_trigger(PowerUp type, vec2 pos, vec2 direction)
{
    ivec2 pos_integer;
    pos_integer.x = pos.x;
    pos_integer.y = pos.y;
    switch(type) {
    case POWERUP_NONE:
        return;
    case POWERUP_BANANA: {
        printf("adding bananor\n");
        vec2 rot = direction;
        vec_normalize(&rot);
        vec_scale(&rot, 50);
        pos_integer.x -= rot.x;
        pos_integer.y -= rot.y;
        map_add_modifier(MAP_BANANA, pos_integer);
        break;
    }
    case POWERUP_GREEN_SHELL:
        printf("adding green shell\n");
        shell_add(SHELL_GREEN, pos, direction);
        break;
    case POWERUP_RED_SHELL:
        printf("adding red shell\n");
        shell_add(SHELL_RED, pos, direction);
        break;
    case POWERUP_BLUE_SHELL:
        printf("adding blue shell\n");
        shell_add(SHELL_RED, pos, direction);
        break;
    case POWERUP_OIL:
        printf("adding oil\n");
        map_add_modifier(MAP_OIL, pos_integer);
        break;
    case POWERUP_MUSHROOM:
        printf("adding mushram\n");
        break;
    case POWERUP_GOLD_MUSHROOM:
        printf("adding gold mushram\n");
        break;
    case POWERUP_BIG_MUSHROOM:
        printf("adding big mushram\n");
        break;
    case POWERUP_LIGHTNING:
        printf("triggering lightning\n");
        break;
    case POWERUP_STAR:
        printf("triggering star\n");
        break;
    default:
        printf("tried to trigger unknown powerup: %d\n", type);
        break;
    }
}
