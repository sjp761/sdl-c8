#include <SDL3/SDL.h>
#pragma once

class SDLBeep
{
    public:
        SDL_AudioSpec want;
        SDL_AudioStream *stream;
        SDLBeep();
        static void audioCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount);
};