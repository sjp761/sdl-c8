#include <SDL2/SDL.h>
#pragma once

class SDLBeep
{
    public:
        SDL_AudioSpec want;
        SDL_AudioSpec have;
        SDL_AudioDeviceID dev;
        SDLBeep();
        static void audioCallback(void* userdata, uint8_t* stream, int len);
};