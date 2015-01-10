#include "box.h"
#include "common.h"

#include "map.h"

#define BOXES_PER_TILE 4

static SDL_Texture *box_texture = 0;

// TODO: get these from the texture
static int BOX_HEIGHT = 16;
static int BOX_WIDTH = 16;

extern int boxlocations_count;
extern ivec2 *boxlocations;

typedef struct {
    ivec2 pos;
    PowerUp type;
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
    for (int i=0; i<box_count; i++) {
        SDL_Rect target;
        target.x = boxes[i].pos.x;
        target.y = boxes[i].pos.y;
        target.w = BOX_WIDTH;
        target.h = BOX_HEIGHT;
        SDL_RenderCopy(ren, box_texture, 0, &target);
    }
}
