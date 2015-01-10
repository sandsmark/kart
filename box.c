#include "common.h"
#include "box.h"
#include "map.h"

#include <stdlib.h>

#define BOXES_PER_TILE 4

static SDL_Texture *box_texture = 0;

// TODO: get these from the texture
static int BOX_HEIGHT = 16;
static int BOX_WIDTH = 16;
static int RESPAWN_TIMEOUT = 5000;

extern int boxlocations_count;
extern ivec2 *boxlocations;

typedef struct {
    ivec2 pos;
    Uint32 hit_time; // for respawning, 0 when never
} Box;

int box_count = 0;
Box *boxes = 0;

int boxes_init(SDL_Renderer *ren)
{
    box_texture = load_image(ren, "box.bmp");

    box_count = boxlocations_count * BOXES_PER_TILE;
    boxes = malloc((box_count + 1) * sizeof(Box));
    if (!boxes) {
        printf("failed to grow array for modifiers\n");
        return 1;
    }
    for (int i=0; i<boxlocations_count; i++) {
        ivec2 box_position;
        box_position.x = boxlocations[i].x * TILE_WIDTH + TILE_WIDTH / 2;
        box_position.y = boxlocations[i].y * TILE_HEIGHT + TILE_HEIGHT / 5;
        for (int j=0; j<4; j++) {
            boxes[i*BOXES_PER_TILE + j].pos = box_position;
            boxes[i*BOXES_PER_TILE + j].hit_time = 0;

            box_position.y += TILE_HEIGHT / 6;
        }
    }

    return (box_texture != 0);
}

void boxes_destroy()
{
	SDL_DestroyTexture(box_texture);
}

void boxes_render(SDL_Renderer *ren)
{
    const Uint32 current_ticks = SDL_GetTicks();
    for (int i=0; i<box_count; i++) {
        if (boxes[i].hit_time != 0 && boxes[i].hit_time + RESPAWN_TIMEOUT > current_ticks) {
            continue;
        } else {
            boxes[i].hit_time = 0;
        }

        SDL_Rect target;
        target.x = boxes[i].pos.x;
        target.y = boxes[i].pos.y;
        target.w = BOX_WIDTH;
        target.h = BOX_HEIGHT;
        SDL_RenderCopy(ren, box_texture, 0, &target);
    }
}

PowerUp boxes_check_hit(vec2 position)
{
    for (int i=0; i<box_count; i++) {
        if (boxes[i].hit_time != 0) {
            continue;
        }

        if (position.x > boxes[i].pos.x && position.y > boxes[i].pos.y &&
            position.x < boxes[i].pos.x + BOX_WIDTH &&
            position.y < boxes[i].pos.y + BOX_HEIGHT) {

            printf("hit box\n");
            boxes[i].hit_time = SDL_GetTicks();
            int random_number = rand() % (int)(POWERUP_BIG_STAR);
            return (PowerUp)random_number;
        }
    }
    return POWERUP_NONE;
}
