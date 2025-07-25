#include "Chip8.h"
#include <iostream>
#include "SDL_MainComponents.h"

void Chip8::emulateInstruction()
{
    currentInstruction = instruction_t(memory[pc] << 8 | memory[pc + 1]); //Combines two bytes into one instruction (16-bit opcode)
    pc += 2; // Move to the next instruction

    switch ((currentInstruction.opcode >> 12) & 0xF)
    {
        
        case 0x0:
            switch (currentInstruction.nn & 0x00F)
            {
                case 0x00E0: // Clear the display 
                    memset(display, false, sizeof(display));
                    break;
                case 0x00EE: // Return from subroutine
                    pc = stack.back();
                    stack.pop_back();
                    break;
            }
            break;
        case 0x01:
            // Jump to address nnn
            pc = currentInstruction.nnn;
            break;
        case 0x02:
            // Call subroutine at nnn
            stack.push_back(pc); // Push current PC onto stack
            pc = currentInstruction.nnn; // Set PC to nnn
            break;
        case 0x0A:
            I = currentInstruction.nnn; // Set index register I to nnn
            break;
        case 0x06:
            // Load register Vx with nn
            V[currentInstruction.x] = currentInstruction.nn;
            break;
        case 0x07:
            // Add nn to register Vx
            V[currentInstruction.x] += currentInstruction.nn;
            break;
        case 0x0D:
        {
            uint8_t cX = V[currentInstruction.x] % 64; // X coordinate
            uint8_t cY = V[currentInstruction.y] % 32; // Y coordinate
            V[0xF] = 0; // Clear VF register (collision flag)
            for (int i = 0; i < currentInstruction.n; ++i)
            {
                uint8_t spriteRow = memory[I + i];
                for (int j = 0; j < 8; ++j)
                {
                    bool pixel = (spriteRow & (0x80 >> j)) != 0; // Check if the pixel is set
                    int x = (cX + j) % 64;
                    int y = (cY + i) % 32;
                    int displayIndex = y * 64 + x;
                    if (display[displayIndex] && pixel) // Collision detection
                    {
                        V[0xF] = 1; // Set VF to indicate collision
                    }
                    display[displayIndex] ^= pixel; // XOR operation to draw the sprite
                }
            }
            break;

        }
        default:
            // Handle unknown opcodes
            //std::cerr << "Unknown opcode: " << std::hex << currentInstruction.opcode << std::dec << std::endl;
            break;
    }
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

SDL_Texture *Chip8::getDisplayTexture() const
{
    SDL_Texture *texture = SDL_CreateTexture(SDL_MainComponents::renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, 64, 32);
    if (!texture) {
        std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
        return nullptr;
    }
    uint32_t pixels[64 * 32];
    for (int i = 0; i < 64 * 32; ++i)
    {
        pixels[i] = display[i] ? 0xFFFFFFFF : 0x00000000; // White for on, black for off
    }
    if (SDL_UpdateTexture(texture, nullptr, pixels, 64 * sizeof(uint32_t)) < 0) {
        std::cerr << "Failed to update texture: " << SDL_GetError() << std::endl;
        SDL_DestroyTexture(texture);
        return nullptr;
    }
    return texture;
}
