#include "sound.h"

#define MUSIC_PATH "pegasus.wav"

#define FREQ 200.0 /* the frequency we want */

unsigned int audio_pos; /* which sample we are up to */
float audio_frequency; /* audio frequency in cycles per sample */
float audio_volume; /* audio volume, 0 - ~32000 */

int freq = 200;

static SoundType current_sound_type;

static Uint8 *music_buffer = 0;
static Uint32 music_position = 0;
static Uint32 music_length = 0;

SDL_AudioDeviceID audio_device;

void sound_init()
{
    current_sound_type = SOUND_NONE;

    SDL_AudioSpec want;
    SDL_AudioSpec have;

    want.freq = 44100;
    want.format = AUDIO_S16;
    want.channels = 1;
    want.samples = 4096;

    audio_pos = 0;
    audio_frequency = FREQ / have.freq; /* 1.0 to make it a float */
    audio_volume = 6000; /* ~1/5 max volume */

    SDL_LoadWAV(MUSIC_PATH, &want, &music_buffer, &music_length);

    want.callback = sound_car_audio_cb;
    audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
}
void sound_destroy()
{
    current_sound_type = SOUND_NONE;
    SDL_CloseAudio();
    SDL_FreeWAV(music_buffer);
}

void sound_car_audio_cb(void *userdata, Uint8 *stream, int len)
{
    (void)userdata; // to squelch the warning about unused variables

    if (current_sound_type == SOUND_GAME) {
        return; // TODO: make better sound
        len /= 2; /* 16 bit */
        Sint16* buf = (Sint16*)stream;
        int m=1000;
        for(int i = 0; i < len; i++) {
            buf[i] = audio_volume * (m - abs((audio_pos/freq) % (2*m) - m));
            audio_pos++;
        }
    } else if (current_sound_type == SOUND_MENU && music_buffer) {
        for(int i = 0; i < len; i++) {
            stream[i] = music_buffer[music_position];
            music_position++;
            music_position %= music_length;
        }
    }
}

void sound_set_car_freq(int new_freq)
{
    freq = new_freq;
}

void sound_set_type(SoundType type)
{
    if (current_sound_type == type) {
        return;
    }

    if (type == SOUND_NONE) {
        SDL_PauseAudioDevice(audio_device, 1);
    } else if (current_sound_type == SOUND_NONE) {
        SDL_PauseAudioDevice(audio_device, 0);
    }

    current_sound_type = type;
}
