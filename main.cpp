#include <SDL2/SDL.h>
#include <iostream>
#include "SDL_MainComponents.h"
#include "SDL_SmartPointer.h"
#include "Chip8.h"

SDL_Window* SDL_MainComponents::window = nullptr;
SDL_Renderer* SDL_MainComponents::renderer = nullptr;

int main(int argc, char* argv[]) 
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    Chip8 c8machine("test.ch8"); // Pass ROM path directly if Chip8 expects std::string or const char*
    SDL_MainComponents::init();
    SDL_ShowWindow(SDL_MainComponents::window);
    while (c8machine.state != Chip8::STOPPED)
    {
        SDL_MainComponents::renderUpdate();
        SDL_Delay(16); // Simulate frame delay
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            SDL_MainComponents::handleEvent(event, c8machine);
        }
    }
    SDL_Quit();
    return 0;
}