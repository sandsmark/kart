#include <SDL2/SDL.h>

typedef struct {
    int angle; // 0-360 degrees
    int speed;

    SDL_Rect rect;
    SDL_Texture *texture;
} Car;


