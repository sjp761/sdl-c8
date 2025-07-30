#include "SDLBeep.h"

namespace {
    constexpr int SAMPLE_RATE = 44100;
    constexpr SDL_AudioFormat AUDIO_FORMAT = SDL_AUDIO_S16LE;
    constexpr int CHANNELS = 1;
    constexpr int SAMPLES = 4096;
    constexpr int SQUARE_WAVE_FREQ = 440;
    constexpr int16_t SQUARE_WAVE_HIGH = 32767;
    constexpr int16_t SQUARE_WAVE_LOW = -32768;
}

SDLBeep::SDLBeep()
{
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_FORMAT;
    want.channels = CHANNELS;
    dev = SDL_OpenAudioDevice(0, &want);
    stream = SDL_OpenAudioDeviceStream(dev, &want, &audioCallback, this);
}

void SDLBeep::audioCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount)
{
    uint8_t buffer[SAMPLES * sizeof(int16_t)];
    int16_t *data = reinterpret_cast<int16_t *>(buffer);
    static uint32_t running_sample_index = 0;
    int32_t square_wave_period = SAMPLE_RATE / SQUARE_WAVE_FREQ;

    for (int i = 0; i < SAMPLES; ++i)
    {
        data[i] = (running_sample_index++ / (square_wave_period / 2)) % 2 == 0
            ? SQUARE_WAVE_HIGH
            : SQUARE_WAVE_LOW;
    }

    SDL_PutAudioStreamData(stream, buffer, SAMPLES * sizeof(int16_t));
}



