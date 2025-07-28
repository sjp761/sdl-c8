#include <SDL2/SDL.h>
#include <iostream>
#include "SDL_MainComponents.h"
#include "SDL_SmartPointer.h"
#include "Chip8.h"

SDL_Window* SDL_MainComponents::window = nullptr;
SDL_Renderer* SDL_MainComponents::renderer = nullptr;

int main(int argc, char* argv[])
{
    srand(static_cast<unsigned int>(time(0)));
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    Chip8 c8machine("/home/user/Documents/sdl-c8/test.ch8"); // Pass ROM path directly if Chip8 expects std::string or const char*
    SDL_MainComponents::init();
    SDL_ShowWindow(SDL_MainComponents::window);
    while (c8machine.state != Chip8::STOPPED)
    {
        c8machine.handleInput();
        SDL_Delay(16); // Roughly 60 FPS
        for (int i = 0; i < 700; ++i) 
        {
        c8machine.emulateInstruction();
        }
        SDL_MainComponents::display.reset(c8machine.getDisplayTexture());
        SDL_MainComponents::renderUpdate();
    }
    SDL_Quit();
    return 0;
}