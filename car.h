struct SDL_Texture;

typedef struct {
    int angle; // 0-360 degrees
    float speed;
    float x, y;
    int width, height;

    int wheelX[4];
    int wheelY[4];

    SDL_Texture *texture;
} Car;

/* vim: set ts=8 sw=8 tw=0 noexpandtab cindent softtabstop=8 :*/
