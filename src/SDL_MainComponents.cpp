#include "SDL_MainComponents.h"
#include "Configuration.h"
#include <tuple>


SDL_SmartPointer<SDL_Texture> SDL_MainComponents::display;

void SDL_MainComponents::renderUpdate()
{
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, display.get(), NULL, NULL);
    SDL_RenderPresent(renderer);
}

void SDL_MainComponents::init()
{
    window = SDL_CreateWindow("SDL Window", configuration::WINDOW_WIDTH * configuration::SCALE_FACTOR, configuration::WINDOW_HEIGHT * configuration::SCALE_FACTOR, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, nullptr);
    auto [r, g, b, a] = extractRGBA();
    SDL_SetRenderDrawColor(renderer, r, g, b, a); // Set draw color to yellow
}

std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> SDL_MainComponents::extractRGBA()
{
    uint8_t r = (configuration::DEFAULT_COLOR >> 24) & 0xFF; // Shifts the bits by 24 then only grabs the first 8 bits to get the color
    uint8_t g = (configuration::DEFAULT_COLOR >> 16) & 0xFF;
    uint8_t b = (configuration::DEFAULT_COLOR >> 8) & 0xFF;
    uint8_t a = configuration::DEFAULT_COLOR & 0xFF;
    return std::make_tuple(r, g, b, a);
}