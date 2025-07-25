#include <SDL2/SDL.h>
#include <iostream>
#include "SDL_MainComponents.h"
#include "SDL_SmartPointer.h"

SDL_Window* SDL_MainComponents::window = nullptr;
SDL_Renderer* SDL_MainComponents::renderer = nullptr;

int main(int argc, char* argv[]) 
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_MainComponents::init();
    SDL_ShowWindow(SDL_MainComponents::window);
    while (true) 
    {
        SDL_MainComponents::renderUpdate();
        SDL_Delay(16); // Simulate frame delay
    }

    SDL_Quit();
}