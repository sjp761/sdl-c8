#pragma once
#include <cstdint>
#include <fstream>
#include <cstring>
#include <vector>
#include <SDL2/SDL_render.h>
#include <atomic>
#include <thread>
#include <mutex>

struct instruction_t
{
    uint16_t opcode; // 2-byte opcode
    uint8_t x; // First nibble of the opcode
    uint8_t y; // Second nibble of the opcode
    uint8_t n; // Third nibble of the opcode
    uint8_t nn; // Last byte of the opcode
    uint16_t nnn; // Last three bytes of the opcode


    instruction_t()
        : opcode(0), x(0), y(0), n(0), nn(0), nnn(0)
        {}
    instruction_t(uint16_t op)
        : opcode(op),
          x((op & 0x0F00) >> 8),
          y((op & 0x00F0) >> 4),
          n(op & 0x000F),
          nn(op & 0x00FF),
          nnn(op & 0x0FFF)
        {}
};

class Chip8
{
    public:
        uint8_t memory[4096]; // 4KB of memory
        bool display[64 * 32]; // Chip8 has a 64x32 pixel monochrome display, bool because on or off
        std::vector<uint16_t> stack; // Chip8 stack using vector
        uint8_t V[16]; // 16 registers (V0 to VF)
        uint16_t I; // Index register
        uint8_t delayTimer; // Delay timer
        uint8_t soundTimer; // Sound timer
        uint16_t pc; // Program counter - current instruction address
        std::string currentRom; // Current ROM being executed
        bool keypad[16]; // 16 keys for input (0x0 to 0xF)
        void updateTimers();
        void handleInput();
        void emulateInstruction();
        void loadRom(const std::string& romPath);
        inline void updatec8display();
        SDL_Texture* getDisplayTexture() const;
        enum emulationState { RUNNING, PAUSED, STOPPED };
        emulationState state;
        instruction_t currentInstruction;

        const uint8_t font[80] = 
        {
            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
            0x20, 0x60, 0x20, 0x20, 0x70, // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80  // F

        };

        

        Chip8(const std::string& romPath) : state(RUNNING), currentRom(romPath)
        {
            memset(display, 0, sizeof(display));
            memset(memory, 0, sizeof(memory));
            memset(V, 0, sizeof(V));
            memset(keypad, 0, sizeof(keypad));
            I = 0;
            currentRom = romPath;
            loadRom(romPath);
            pc = 0x200; // Program starts at 0x200
            memcpy(memory + 0x50, font, sizeof(font)); // Load font into memory starting at 0x50
        }



};