#include <SDL3/SDL.h>
#include <iostream>
#include "SDL_MainComponents.h"
#include "SDL_SmartPointer.h"
#include "Chip8.h"
#include <filesystem>

SDL_Window* SDL_MainComponents::window = nullptr;
SDL_Renderer* SDL_MainComponents::renderer = nullptr;

int main(int argc, char* argv[])
{
    std::cout << std::filesystem::current_path() << std::endl;
    srand(static_cast<unsigned int>(time(0)));
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == false) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    std::string romPath;
    if (argc < 2) {
        std::cerr << "Rom path empty: " << std::endl;
        romPath = "/home/user/Documents/sdl-c8/roms/beep.ch8";
    }
    else {
        romPath = std::string(argv[1]);
    }
    Chip8 c8machine(romPath); // Pass ROM path directly if Chip8 expects std::string or const char*
    SDL_MainComponents::init();
    SDL_ShowWindow(SDL_MainComponents::window);
    while (c8machine.state != Chip8::STOPPED)
    {
        c8machine.handleInput();
        uint64_t startTime = SDL_GetPerformanceCounter();
        for (int i = 0; i < 700/60; ++i) 
        {
        c8machine.emulateInstruction();
        }
        uint64_t endTime = SDL_GetPerformanceCounter();
        uint64_t elapsedTime = endTime - startTime;
        uint64_t delayTime = (SDL_GetPerformanceFrequency() / 60) - elapsedTime;
        if (delayTime > 0) {
            SDL_Delay(delayTime * 1000 / SDL_GetPerformanceFrequency());
        }
        c8machine.updateTimers();
        SDL_MainComponents::display.reset(c8machine.getDisplayTexture());
        SDL_MainComponents::renderUpdate();
    }
    SDL_Quit();
    return 0;
}