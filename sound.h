#ifndef SOUND_H
#define SOUND_H

#include <SDL2/SDL.h>

void sound_init();
void sound_car_audio_cb(void *userdata, Uint8 *stream, int len);
void sound_set_car_freq(int freq);

#endif//SOUND_H
