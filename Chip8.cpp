#include "Chip8.h"
#include <iostream>
#include "SDL_MainComponents.h"

void Chip8::emulateInstruction()
{
    currentInstruction = instruction_t(memory[pc] << 8 | memory[pc + 1]); //Combines two bytes into one instruction (16-bit opcode)
    pc += 2; // Move to the next instruction, +2 because each instruction is 2 bytes long

    switch ((currentInstruction.opcode >> 12) & 0xF)
    {
        
        case 0x0:
            switch (currentInstruction.nn & 0x00F)
            {
                case 0x00E0: // Clear the display 
                    memset(display, false, sizeof(display));
                    break;
                case 0x00EE: // Return from subroutine
                    if (!stack.empty()) 
                    {
                        uint16_t returnAddress = stack.back();
                        stack.pop_back();
                        pc = returnAddress; // Set PC to the address popped from the stack
                    } 
                    else 
                    {
                        std::cerr << "Stack underflow on 00EE (return from subroutine)" << std::endl;
                        // Optionally, set state = STOPPED or handle as needed
                    }
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
        
        case 0x03:
            if (V[currentInstruction.x] == currentInstruction.nn) // Skip next instruction if Vx == nn
            {
                pc += 2; //Incrementing PC by 2 skips the next instruction
            }
            break;
        case 0x04:
            if (V[currentInstruction.x] != currentInstruction.nn) // Skip next instruction if Vx != nn
            {
                pc += 2;
            }
            break;
        case 0x05:
            if (V[currentInstruction.x] == V[currentInstruction.y]) // Skip next instruction if Vx == Vy
            {
                pc += 2;
            }
            break;
        case 0x06:
            // Load register Vx with nn
            V[currentInstruction.x] = currentInstruction.nn;
            break;
        case 0x07:
            // Add nn to register Vx
            V[currentInstruction.x] += currentInstruction.nn;
            break;
        case 0x08:
            switch (currentInstruction.n)
            {
                case 0x0:
                    // Load Vx with Vy
                    V[currentInstruction.x] = V[currentInstruction.y];
                    break;
                case 0x1:
                    // Set Vx to Vx OR Vy
                    V[currentInstruction.x] |= V[currentInstruction.y];
                    break;
                case 0x2:
                    // Set Vx to Vx AND Vy
                    V[currentInstruction.x] &= V[currentInstruction.y];
                    break;
                case 0x3:
                    // Set Vx to Vx XOR Vy
                    V[currentInstruction.x] ^= V[currentInstruction.y];
                    break;
                case 0x4:
                    if ((V[currentInstruction.x] + V[currentInstruction.y]) > 255)
                    {
                        V[15] = 1;
                    }
                    else
                    {
                        V[15] = 0;
                    }
                    V[currentInstruction.x] = V[currentInstruction.x] + V[currentInstruction.y];
                    break;
                case 0x5:
                    // Subtract Vy from Vx
                    if (V[currentInstruction.x] > V[currentInstruction.y]) 
                    {
                        V[15] = 1; // Set VF to 1 if no borrow
                    }
                    else
                    {
                        V[15] = 0; // Set VF to 0 if borrow occurs
                    }
                    V[currentInstruction.x] -= V[currentInstruction.y];
                    break;
                case 0x06:
                    // Shift Vx right by 1
                    V[15] = V[currentInstruction.x] & 0x01; // Set VF to the least significant bit before shifting
                    V[currentInstruction.x] >>= 1;
                    break;
                case 0x07:
                    // Set Vx to Vy - Vx
                    if (V[currentInstruction.y] > V[currentInstruction.x]) {
                        V[15] = 0; // Set VF to 0 if borrow occurs
                    } else {
                        V[15] = 1; // Set VF to 1 if no borrow
                    }
                    V[currentInstruction.x] = V[currentInstruction.y] - V[currentInstruction.x];
                    break;
                case 0xE:
                    // Shift Vx left by 1
                    V[15] = (V[currentInstruction.x] & 0x80) >> 7; // Set VF to the most significant bit before shifting
                    V[currentInstruction.x] <<= 1;
                    break;
                default:
                    //Handle unknown opcodes
                    std::cerr << "Unknown opcode: " << std::hex << currentInstruction.opcode << std::dec << std::endl;
                    break;

            }
            break;
        case 0x09:
            if (V[currentInstruction.x] != V[currentInstruction.y]) // Skip next instruction if Vx != Vy
            {
                pc += 2;
            }
            break;
        case 0x0A:
            I = currentInstruction.nnn; // Set index register I to nnn
            break;
        case 0x0B:
            // Jump to address nnn + V0
            pc = currentInstruction.nnn + V[0];
            break;
        case 0x0D:
        {
            updatec8display(); // Update the display with the sprite data
            break;
        }
        default:
            //Handle unknown opcodes
            std::cerr << "Unknown opcode: " << std::hex << currentInstruction.opcode << std::dec << std::endl;
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

inline void Chip8::updatec8display()
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
