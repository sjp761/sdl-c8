#include "SDL_MainComponents.h"
#include <tuple>
SDL_SmartPointer<SDL_Texture> SDL_MainComponents::display;

namespace
{
    constexpr int WINDOW_WIDTH = 64;
    constexpr int WINDOW_HEIGHT = 32;
    constexpr uint32_t DEFAULT_COLOR = 0xFFFF00FF; // Yellow in RGBA format
    constexpr int SCALE_FACTOR = 20;
}

void SDL_MainComponents::renderUpdate()
{
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, display.get(), nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

void SDL_MainComponents::init()
{
    window = SDL_CreateWindow("SDL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH * SCALE_FACTOR, WINDOW_HEIGHT * SCALE_FACTOR, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    auto [r, g, b, a] = extractRGBA();
    SDL_SetRenderDrawColor(renderer, r, g, b, a); // Set draw color to yellow
}

std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> SDL_MainComponents::extractRGBA()
{
    uint8_t r = (DEFAULT_COLOR >> 24) & 0xFF; // Shifts the bits by 24 then only grabs the first 8 bits to get the color
    uint8_t g = (DEFAULT_COLOR >> 16) & 0xFF;
    uint8_t b = (DEFAULT_COLOR >> 8) & 0xFF;
    uint8_t a = DEFAULT_COLOR & 0xFF;
    return std::make_tuple(r, g, b, a);
}