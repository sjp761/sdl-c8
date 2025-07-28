#pragma once
#include "SDL_SmartPointer.h"
#include <SDL2/SDL.h>
#include "Chip8.h"
#include <tuple>

class SDL_MainComponents
{
    public:
        static SDL_Window* window;
        static SDL_Renderer* renderer;
        static SDL_SmartTexture display;
        static void renderUpdate();
        static void init();
        static std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> extractRGBA();

};