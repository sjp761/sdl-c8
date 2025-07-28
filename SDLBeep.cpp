#include "SDLBeep.h"

namespace {
    constexpr int SAMPLE_RATE = 44100;
    constexpr SDL_AudioFormat AUDIO_FORMAT = AUDIO_S16LSB;
    constexpr int CHANNELS = 1;
    constexpr int SAMPLES = 4096;
    constexpr int SQUARE_WAVE_FREQ = 440;
    constexpr int16_t SQUARE_WAVE_HIGH = 32767;
    constexpr int16_t SQUARE_WAVE_LOW = -32768;
}

SDLBeep::SDLBeep()
{
    want = {
        .freq = SAMPLE_RATE,
        .format = AUDIO_FORMAT,
        .channels = CHANNELS,
        .samples = SAMPLES,
        .callback = audioCallback,
        .userdata = this,
    };

    dev = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
}

void SDLBeep::audioCallback(void *userdata, uint8_t *stream, int len)
{
    int16_t *data = reinterpret_cast<int16_t *>(stream);
    uint32_t running_sample_index = 0;
    int32_t square_wave_period = SAMPLE_RATE / SQUARE_WAVE_FREQ;

    for (int i = 0; i < len / sizeof(int16_t); ++i)
    {
        data[i] = (running_sample_index++ / (square_wave_period / 2)) % 2 == 0
            ? SQUARE_WAVE_HIGH
            : SQUARE_WAVE_LOW;
    }
}



