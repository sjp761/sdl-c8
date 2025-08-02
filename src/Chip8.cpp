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
    if (highResDisplay)
    {
        int x = V[currentInstruction.x] & 0x7F; // Mask to 0-127
        int y = V[currentInstruction.y] & 0x3F; // Mask to 0-63
        V[0xF] = 0; // Clear collision flag

        if (currentInstruction.n == 0) 
        {
            for (int row = 0; row < 16; row++) 
            {
                uint8_t a1 = memory[I + row * 2];
                uint8_t a2 = memory[I + row * 2 + 1];
                uint16_t merged = (a1 << 8) | a2;
                bool rowCollision = false;
                for (int col = 0; col < 16; col++)
                {
                    int pixel = (merged >> (15 - col)) & 1;
                    int displayX = (x + col); // wrap if needed
                    int displayY = (y + row);  // wrap if needed
                    if (pixel == 0 || displayX >= 128 || displayY >= 64) continue; // Skip if pixel is off or values are out of bounds

                    if (display[displayX][displayY] && pixel) rowCollision = true;
                    display[displayX][displayY] ^= pixel;
                }
                
                if (rowCollision) V[0xF]++;
            }
        }
        else
        {
            for (int row = 0; row < currentInstruction.n; row++) 
            {
                uint8_t byte = memory[I + row];
                bool rowCollision = false;
                for (int col = 0; col < 8; col++) 
                {
                    int pixel = (byte >> (7 - col)) & 1;
                    int displayX = (x + col);
                    int displayY = (y + row);
                    if (pixel == 0 || displayX >= 128 || displayY >= 64) continue; // Skip if pixel is off or values are out of bounds
                    if (pixel && display[displayX][displayY]) rowCollision = true; // Set collision flag
                    display[displayX][displayY] ^= pixel;
                }
                if (rowCollision) V[0xF]++; // Increment collision flag if any pixel was toggled off
            }
        }
    }
    else
    {
        int x = V[currentInstruction.x] & 0x3F; // Mask to 0-63
        int y = V[currentInstruction.y] & 0x1F; // Mask to 0-31
        V[0xF] = 0; // Clear collision flag

        for (int row = 0; row < currentInstruction.n; row++) 
        {
            uint8_t byte = memory[I + row];
            for (int col = 0; col < 8; col++) 
            {
                int pixel = (byte >> (7 - col)) & 1;
                int baseX = (x + col);
                int baseY = (y + row);
                if (baseX >= 64 || baseX < 0) continue;
                if (baseY >= 32 || baseY < 0) continue;
                // Scale low-res pixel to 2x2 in high-res display
                for (int dy = 0; dy < 2; dy++)
                {
                    for (int dx = 0; dx < 2; dx++)
                    {
                        int displayX = (baseX * 2 + dx) % 128;
                        int displayY = (baseY * 2 + dy) % 64;
                        
                        // Toggle pixel
                        if (pixel && display[displayX][displayY] && dy == 0) 
                        {
                            V[0xF] = 1; // Set collision flag if pixel was already on in the top row
                        }
                        display[displayX][displayY] ^= pixel;
                    }
                }
            }
        }
    }
}

SDL_Texture *Chip8::getDisplayTexture() const
{
    int width = 128;
    int height = 64;
    SDL_Texture *texture = SDL_CreateTexture(SDL_MainComponents::renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, width, height);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);           
    uint32_t pixels[width * height];
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int i = y * width + x;
            pixels[i] = display[x][y] ? 0xFFFFFFFF : 0x00000000; // White for on, black for off
        }
    }
    SDL_UpdateTexture(texture, nullptr, pixels, width * sizeof(uint32_t));
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
    highResDisplay = false;
    loadRom(romPath);
    pc = 0x200; // Program starts at 0x200
    // Load font into memory starting at 0x50
    memcpy(memory + 0x50, font, sizeof(font));
    memcpy(memory + 0x50 + sizeof(font), superFont, sizeof(superFont)); // Load Super Chip-8 font
}