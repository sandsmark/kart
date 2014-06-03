struct SDL_Texture;

typedef struct {
    int angle; // 0-360 degrees
    float speed;
    float x, y;
    int width, height;

    SDL_Texture *texture;
} Car;


