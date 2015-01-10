#ifndef SOUND_H
#define SOUND_H

#include <SDL2/SDL.h>

typedef enum {
    SOUND_NONE,
    SOUND_MENU,
    SOUND_GAME
} SoundType;

void sound_init();
void sound_destroy();
void sound_car_audio_cb(void *userdata, Uint8 *stream, int len);
void sound_set_car_freq(int freq);
void sound_set_type(SoundType type);


#endif//SOUND_H
