#include "Chip8.h"

void Chip8::emulateInstruction()
{
    currentInstruction = instruction_t(memory[pc] << 8 | memory[pc + 1]); //Combines two bytes into one instruction (16-bit opcode)
    pc += 2; // Move to the next instruction
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