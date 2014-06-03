#include <SDL2/SDL.h>

typedef struct {
    int x;
    int y;
    int angle; // 0-360 degrees
    int speed;

    SDL_Texture *texture;
} Car;


