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
        SDL_PauseAudioDevice(beeper.dev); // Unpause audio device if sound timer is active
        --soundTimer;
    }
    else
    {
        SDL_ResumeAudioDevice(beeper.dev); // Pause audio device if sound timer is not active
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
    currentInstruction = instruction_t(memory[pc] << 8 | memory[pc + 1]); //Combines two bytes into one instruction (16-bit opcode)
    pc += 2; // Move to the next instruction, +2 because each instruction is 2 bytes long
    switch ((currentInstruction.opcode >> 12) & 0xF)
    {
        case 0x0:
           switch (currentInstruction.nn) // 0x00nn (grab the 3rd and 4th nibble of the opcode)
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
            switch (currentInstruction.n) //8xyn (grab the fourth nibble (n) from the opcode)
            {
                case 0x0:
                    // Load Vx with Vy
                    V[currentInstruction.x] = V[currentInstruction.y];
                    break;
                case 0x1:
                    // Set Vx to Vx OR Vy
                    V[currentInstruction.x] |= V[currentInstruction.y];
                    V[0xF] = 0; // QUIRK
                    break;
                case 0x2:
                    // Set Vx to Vx AND Vy
                    V[currentInstruction.x] &= V[currentInstruction.y];
                    V[0xF] = 0; // QUIRK
                    break;
                case 0x3:
                    // Set Vx to Vx XOR Vy
                    V[currentInstruction.x] ^= V[currentInstruction.y];
                    V[0xF] = 0; // QUIRK
                    break;
                case 0x4:
                {
                    bool carry = (V[currentInstruction.x] + V[currentInstruction.y]) > 0xFF; // Check for overflow
                    V[currentInstruction.x] += V[currentInstruction.y];
                    V[15] = carry; // Set VF to 1 if there's a carry, 0 otherwise
                    break;

                }
                case 0x5:
                {
                    bool carry = V[currentInstruction.x] >= V[currentInstruction.y];
                    V[currentInstruction.x] -= V[currentInstruction.y];
                    V[15] = carry;
                    break;

                }
                case 0x06:
                {
                    V[currentInstruction.x] = V[currentInstruction.y];
                    int shiftedBit = V[currentInstruction.x] & 0x1; //Get the least significant bit
                    V[currentInstruction.x] >>= 1;
                    V[15] = shiftedBit; // Set VF to the least significant bit before shifting
                    break;
                }
                case 0x07:
                {
                    bool carry = V[currentInstruction.y] >= V[currentInstruction.x];
                    V[currentInstruction.x] = V[currentInstruction.y] - V[currentInstruction.x];
                    V[15] = carry;
                    break;
                }
                case 0xE:
                {
                    V[currentInstruction.x] = V[currentInstruction.y];
                    int shiftedBit = (V[currentInstruction.x] & 0x80) >> 7; // Get the most significant bit before shifting
                    V[currentInstruction.x] <<= 1;
                    V[15] = shiftedBit; // Set VF to the most significant bit before shifting
                    break;
                }
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
        case 0x0C:
            V[currentInstruction.x] = (rand() % 256) & currentInstruction.nn; // Set Vx to a random number
            break;
        case 0x0D:
        {
            updatec8display(); // Update the display with the sprite data
            break;
        }
        case 0x0E:
            switch (currentInstruction.nn) // 0xExnn (grab the last byte of the opcode)
            {
                case 0x9E:
                    if (keypad[V[currentInstruction.x]]) // Skip next instruction if key Vx is pressed
                    {
                        pc += 2;
                    }
                    break;
                case 0xA1:
                    if (!keypad[V[currentInstruction.x]]) // Skip next instruction if key Vx is not pressed
                    {
                        pc += 2;
                    }
                    break;
                default:
                    std::cerr << "Unknown opcode: " << currentInstruction.opcode << std::endl;
                    break;
            }
            break;
        
        case 0x0F:
            switch (currentInstruction.nn) // 0xFXNN (grab the 3rd and fourth nibble of the opcode)
            {
                case 0x0A:
                {
                    bool keyPressed = false;
                    for (int i = 0; i < 16; ++i)
                    {
                        if (keypad[i]) {
                            V[currentInstruction.x] = i; // Set Vx to the key pressed
                            keyPressed = true;
                            break;
                        }
                    }
                    if (!keyPressed) {
                        pc -= 2; // Repeat this instruction until a key is pressed
                    }
                    break;
                }
                case 0x1E:
                    I += V[currentInstruction.x]; // Add Vx to I
                    break;
                case 0x07:  
                    V[currentInstruction.x] = delayTimer; // Set VX to the value of the delay timer
                    break;
                case 0x15:
                    delayTimer = V[currentInstruction.x]; // Set the delay timer to the value of VX
                    break;
                case 0x18:
                    soundTimer = V[currentInstruction.x]; // Set the sound timer to the value of VX
                    break;
                case 0x29:
                    I = 0x50 + (V[currentInstruction.x] * 5); // Set I to the address of the font sprite for Vx
                    break;
                case 0x33:
                    // Store BCD representation of Vx in memory at I, I+1, I+2
                    memory[I] = V[currentInstruction.x] / 100; // Hundreds place
                    memory[I + 1] = (V[currentInstruction.x] / 10) % 10; // Tens place
                    memory[I + 2] = V[currentInstruction.x] % 10; // Ones place
                    break;
                case 0x55:
                    // Store registers V0 to Vx in memory starting at address I
                    for (int i = 0; i <= currentInstruction.x; ++i) 
                    {
                        memory[I + i] = V[i];
                    }
                    I += 1 + currentInstruction.x; // QUIRK - Increment I by the number of registers stored + 1
                    break;
                case 0x65:
                    // Read registers V0 to Vx from memory starting at address I
                    for (int i = 0; i <= currentInstruction.x; ++i)
                    {
                        V[i] = memory[I + i];
                    }
                    // Some interpreters increment I, some don't. Choose one for compatibility.
                    I += 1 + currentInstruction.x; // QUIRK - Increment I by the number of registers read + 1
                    break;
                default:
                    std::cerr << "Unknown opcode: " << currentInstruction.opcode << std::endl;
                    break;
            }
            break;
        
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
    SDL_Texture *texture = SDL_CreateTexture(SDL_MainComponents::renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, configuration::WINDOW_WIDTH, configuration::WINDOW_HEIGHT);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    if (!texture) {
        std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
        return nullptr;
    }
    uint32_t pixels[configuration::WINDOW_WIDTH * configuration::WINDOW_HEIGHT];
    for (int i = 0; i < configuration::WINDOW_WIDTH * configuration::WINDOW_HEIGHT; i++)
    {
        pixels[i] = display[i] ? 0xFFFFFFFF : 0x00000000; // White for on, black for off
    }
    SDL_UpdateTexture(texture, nullptr, pixels, configuration::WINDOW_WIDTH * sizeof(uint32_t));
    return texture;
}
