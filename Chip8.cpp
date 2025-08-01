#include "Chip8.h"
#include <iostream>
#include "SDL_MainComponents.h"
#include "Configuration.h"
#include <random>
#include <thread>
#include <atomic>
#include <mutex>

//A nibble is 4 bits

void Chip8::updateTimers()
{
    if (delayTimer > 0)
    {
        --delayTimer;
    }
    if (soundTimer > 0)
    {
        SDL_ResumeAudioStreamDevice(beeper.stream); // Unpause audio device if sound timer is active
        --soundTimer;
    }
    else
    {
        SDL_PauseAudioStreamDevice(beeper.stream); // Pause audio device if sound timer is not active
    }
}

void Chip8::handleInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_EVENT_QUIT:
                state = Chip8::STOPPED;
                break;
            case SDL_EVENT_KEY_DOWN:
                switch (event.key.key)
                {
                    case SDLK_ESCAPE:
                        state = Chip8::STOPPED;
                        break;
                    case SDLK_SPACE:
                        if (state == Chip8::RUNNING) 
                        {
                            state = Chip8::PAUSED;
                        }
                        else if (state == Chip8::PAUSED)
                        {
                            state = Chip8::RUNNING;
                        }
                        break;
                    case SDLK_1: keypad[0x1] = true; std::cout << "Key 1 pressed" << std::endl; break;
                    case SDLK_2: keypad[0x2] = true; break;
                    case SDLK_3: keypad[0x3] = true; break;
                    case SDLK_4: keypad[0xC] = true; break;
                    case SDLK_Q: keypad[0x4] = true; break;
                    case SDLK_W: keypad[0x5] = true; break;
                    case SDLK_E: keypad[0x6] = true; break;
                    case SDLK_R: keypad[0xD] = true; break;
                    case SDLK_A: keypad[0x7] = true; break;
                    case SDLK_S: keypad[0x8] = true; break;
                    case SDLK_D: keypad[0x9] = true; break;
                    case SDLK_F: keypad[0xE] = true; break;
                    case SDLK_Z: keypad[0xA] = true; break;
                    case SDLK_X: keypad[0x0] = true; break;
                    case SDLK_C: keypad[0xB] = true; break;
                    case SDLK_V: keypad[0xF] = true; break;
                }
                break;
            case SDL_EVENT_KEY_UP:
                switch (event.key.key)
                {
                    case SDLK_1: keypad[0x1] = false; break;
                    case SDLK_2: keypad[0x2] = false; break;
                    case SDLK_3: keypad[0x3] = false; break;
                    case SDLK_4: keypad[0xC] = false; break;
                    case SDLK_Q: keypad[0x4] = false; break;
                    case SDLK_W: keypad[0x5] = false; break;
                    case SDLK_E: keypad[0x6] = false; break;
                    case SDLK_R: keypad[0xD] = false; break;
                    case SDLK_A: keypad[0x7] = false; break;
                    case SDLK_S: keypad[0x8] = false; break;
                    case SDLK_D: keypad[0x9] = false; break;
                    case SDLK_F: keypad[0xE] = false; break;
                    case SDLK_Z: keypad[0xA] = false; break;
                    case SDLK_X: keypad[0x0] = false; break;
                    case SDLK_C: keypad[0xB] = false; break;
                    case SDLK_V: keypad[0xF] = false; break;
                }
                break;
        }
    }
}

void Chip8::emulateInstruction()
{
    currentInstruction = instruction_t(memory[pc] << 8 | memory[pc + 1]);
    pc += 2;
    uint8_t nibble = (currentInstruction.opcode >> 12) & 0xF;
    opcodeTable[nibble](*this); // Call the appropriate opcode handler

}
void Chip8::loadRom(const std::string &romPath)
{
    std::ifstream romFile(romPath, std::ios::binary | std::ios::ate);
    if (!romFile.is_open()) {
        throw std::runtime_error("Failed to open ROM file: " + romPath);
    }
    std::streamsize romSize = romFile.tellg();
    romFile.seekg(0, std::ios::beg);
    if (romSize > (4096 - 0x200)) {
        throw std::runtime_error("ROM too large to fit in memory");
    }
    romFile.read(reinterpret_cast<char*>(memory + 0x200), romSize);
    romFile.close();
}

void Chip8::updatec8display()
{
    uint8_t cX = V[currentInstruction.x] % 64; // X coordinate - starting (wraps around with modulo)
    uint8_t cY = V[currentInstruction.y] % 32; // Y coordinate - starting (wraps around with modulo)
    V[0xF] = 0; // Clear VF register (collision flag)
    for (int i = 0; i < currentInstruction.n; ++i) // Loop through each row of the sprite
    {
        uint8_t spriteRow = memory[I + i];
        for (int j = 0; j < 8; ++j) // Loop through the column
        {
            bool pixel = (spriteRow & (0x80 >> j)) != 0; // Check if the pixel is set
            int x = (cX + j); // X coordinate of the pixel to be drawn
            if ((x >= 64 || x < 0) && configuration::clipping) continue; // QUIRK - stop drawing if out of bounds - configure with Clipping
            int y = (cY + i); // Y coordinate of the pixel to be drawn
            if ((y >= 32 || y < 0) && configuration::clipping) continue; // QUIRK - stop drawing if out of bounds - configure with Clipping
            int displayIndex = y * 64 + x;
            if (display[displayIndex] && pixel) // Collision detection - XOR operation
            {
                V[0xF] = 1; // Set VF to indicate collision
            }
            display[displayIndex] ^= pixel; // XOR operation to draw the sprite
        }
    }
}

SDL_Texture *Chip8::getDisplayTexture() const
{
    SDL_Texture *texture = nullptr;
    /*
    if (configuration::mode == 1 || configuration::mode == 2) // High-resolution display for Super Chip-8
    {
        texture = SDL_CreateTexture(SDL_MainComponents::renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, configuration::WINDOW_WIDTH * 2, configuration::WINDOW_HEIGHT * 2);
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
        uint32_t pixels[configuration::WINDOW_WIDTH * configuration::WINDOW_HEIGHT * 4];
        for (int i = 0; i < configuration::WINDOW_WIDTH * configuration::WINDOW_HEIGHT * 4; i++)
        {
            pixels[i] = highResDisplay[i] ? 0xFFFFFFFF : 0x00000000; // White for on, black for off
        }
        SDL_UpdateTexture(texture, nullptr, pixels, configuration::WINDOW_WIDTH * sizeof(uint32_t));
    }
    else //Regular Chip-8 display
    {

    }
    */

    texture = SDL_CreateTexture(SDL_MainComponents::renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, configuration::WINDOW_WIDTH, configuration::WINDOW_HEIGHT);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    uint32_t pixels[configuration::WINDOW_WIDTH * configuration::WINDOW_HEIGHT];
    for (int i = 0; i < configuration::WINDOW_WIDTH * configuration::WINDOW_HEIGHT; i++)
    {
        pixels[i] = display[i] ? 0xFFFFFFFF : 0x00000000; // White for on, black for off
    }
    SDL_UpdateTexture(texture, nullptr, pixels, configuration::WINDOW_WIDTH * sizeof(uint32_t));
    return texture;
}

Chip8::Chip8(const std::string &romPath)
{
    beeper = SDLBeep();
    memset(display, 0, sizeof(display));
    memset(memory, 0, sizeof(memory));
    memset(V, 0, sizeof(V));
    memset(keypad, 0, sizeof(keypad));
    I = 0;
    currentRom = romPath;
    loadRom(romPath);
    pc = 0x200; // Program starts at 0x200
    // Load font into memory starting at 0x50
    memcpy(memory + 0x50, font, sizeof(font));
}
