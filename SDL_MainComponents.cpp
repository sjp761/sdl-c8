#include "SDL_MainComponents.h"

void SDL_MainComponents::renderUpdate()
{
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void SDL_MainComponents::init()
{
    window = SDL_CreateWindow("SDL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 640, 480);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
}
