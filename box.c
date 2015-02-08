#include "box.h"
#include "map.h"
#include "renderer.h"

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

int boxes_init()
{
    box_texture = ren_load_image("box.bmp");

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

PowerUp boxes_check_hit(SDL_Rect car)
{
    for (int i=0; i<box_count; i++) {
        if (boxes[i].hit_time != 0) {
            continue;
        }
        SDL_Rect box_geometry;
        box_geometry.x = boxes[i].pos.x;
        box_geometry.y = boxes[i].pos.y;
        box_geometry.w = BOX_WIDTH;
        box_geometry.h = BOX_HEIGHT;

        if (SDL_HasIntersection(&car, &box_geometry)) {
            boxes[i].hit_time = SDL_GetTicks();
            int random_number = rand() % (POWERUP_STAR - 1);
            random_number++; // the first is POWERUP_NONE
            return POWERUP_LIGHTNING;
//            return (PowerUp)random_number;
        }
    }
    return POWERUP_NONE;
}

cJSON *boxes_serialize()
{

    cJSON *boxes_array = cJSON_CreateArray();

    for (int i=0; i<box_count; i++) {
        if (boxes[i].hit_time) continue; // it is not visible
        cJSON *box_object = cJSON_CreateObject();
        cJSON_AddNumberToObject(box_object, "x", boxes[i].pos.x);
        cJSON_AddNumberToObject(box_object, "y", boxes[i].pos.y);
        cJSON_AddNumberToObject(box_object, "width", BOX_WIDTH);
        cJSON_AddNumberToObject(box_object, "height", BOX_HEIGHT);
        cJSON_AddItemToArray(boxes_array, box_object);
    }
	return boxes_array;
}
