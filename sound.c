#include "sound.h"

#define FREQ 200.0 /* the frequency we want */

unsigned int audio_pos; /* which sample we are up to */
float audio_frequency; /* audio frequency in cycles per sample */
float audio_volume; /* audio volume, 0 - ~32000 */

int freq = 200;

void sound_init()
{
    SDL_AudioSpec want;
    SDL_AudioSpec have;

    want.freq = 44100;
    want.format = AUDIO_S16;
    want.channels = 1;
    want.samples = 4096;
    want.callback = sound_car_audio_cb;

    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);


    audio_pos = 0;
    audio_frequency = FREQ / have.freq; /* 1.0 to make it a float */
    audio_volume = 6000; /* ~1/5 max volume */

    SDL_PauseAudioDevice(dev, 0); /* play! */
}

void sound_car_audio_cb(void *userdata, Uint8 *stream, int len)
{
    len /= 2; /* 16 bit */
    int i;
    Sint16* buf = (Sint16*)stream;
    int m=1000;
    for(i = 0; i < len; i++) { 
        buf[i] = audio_volume * (m - abs((audio_pos/freq) % (2*m) - m));
        audio_pos++;
    }
    return;
}

void sound_set_car_freq(int new_freq)
{
    freq = new_freq;
}
