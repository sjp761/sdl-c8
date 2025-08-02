#include <iostream>
#include "Configuration.h"
#include "Opcodes.h"
#include "Chip8.h"

void Opcodes::handle0(Chip8 &chip8)
{
    switch (chip8.nn()) // 0x00nn (grab the 3rd and 4th nibble of the opcode)
    {
        case 0x00E0: // Clear the display
            memset(chip8.display, false, sizeof(chip8.display));
            break;
        case 0x00EE: // Return from subroutine
        {
            uint16_t returnAddress = chip8.stack.back();
            chip8.stack.pop_back();
            chip8.pc = returnAddress; // Set PC to the address popped from the stack
            break;
        }
        case 0x00FD: // Quit the emulator
            chip8.state = Chip8::STOPPED; // Set state to STOPPED
            break;
        case 0x00FF: // High-resolution display mode
            chip8.highResDisplay = true; // Set high-resolution display mode
            break;
        case 0x00FE: // Regular display mode
            chip8.highResDisplay = false; // Set regular display mode
            break;
        default:
            std::cerr << "Unknown opcode: " << chip8.opcode() << std::endl;
    }
}

void Opcodes::handle1(Chip8 &chip8)
{
    chip8.pc = chip8.nnn();
}

void Opcodes::handle2(Chip8 &chip8)
{
    chip8.stack.push_back(chip8.pc);
    chip8.pc = chip8.nnn();
}

void Opcodes::handle3(Chip8 &chip8)
{
    if (chip8.V[chip8.x()] == chip8.nn()) // Skip next instruction if Vx == nn
    {
        chip8.pc += 2; //Incrementing PC by 2 skips the next instruction
    }
}

void Opcodes::handle4(Chip8 &chip8)
{
    if (chip8.V[chip8.x()] != chip8.nn()) // Skip next instruction if Vx != nn
    {
        chip8.pc += 2;
    }
}

void Opcodes::handle5(Chip8 &chip8)
{
    if (chip8.V[chip8.x()] == chip8.V[chip8.y()]) // Skip next instruction if Vx == Vy
    {
        chip8.pc += 2;
    }
}

void Opcodes::handle6(Chip8 &chip8)
{
    chip8.V[chip8.x()] = chip8.nn();
}

void Opcodes::handle7(Chip8 &chip8)
{
    chip8.V[chip8.x()] += chip8.nn();
}

void Opcodes::handle8(Chip8 &chip8)
{
    switch (chip8.n()) //8xyn (grab the fourth nibble (n) from the opcode)
    {
        case 0x0:
            // Load Vx with Vy
            chip8.V[chip8.x()] = chip8.V[chip8.y()];
            break;
        case 0x1:
            // Set Vx to Vx OR Vy
            chip8.V[chip8.x()] |= chip8.V[chip8.y()];
            if (configuration::vfReset)
                chip8.V[0xF] = 0; // QUIRK - configure with vfReset
            break;
        case 0x2:
            // Set Vx to Vx AND Vy
            chip8.V[chip8.x()] &= chip8.V[chip8.y()];
            if (configuration::vfReset)
                chip8.V[0xF] = 0; // QUIRK - configure with vfReset
            break;
        case 0x3:
            // Set Vx to Vx XOR Vy
            chip8.V[chip8.x()] ^= chip8.V[chip8.y()];
            if (configuration::vfReset)
                chip8.V[0xF] = 0; // QUIRK - configure with vfReset
            break;
        case 0x4:
        {
            bool carry = (chip8.V[chip8.x()] + chip8.V[chip8.y()]) > 0xFF; // Check for overflow
            chip8.V[chip8.x()] += chip8.V[chip8.y()];
            chip8.V[0xF] = carry; // Set VF to 1 if there's a carry, 0 otherwise
            break;
        }
        case 0x5:
        {
            bool carry = chip8.V[chip8.x()] >= chip8.V[chip8.y()];
            chip8.V[chip8.x()] -= chip8.V[chip8.y()];
            chip8.V[0xF] = carry;
            break;
        }
        case 0x6:
        {
            chip8.V[chip8.x()] = chip8.V[chip8.y()]; //Quirk - configure with chip mode
            int shiftedBit = chip8.V[chip8.x()] & 0x1; //Get the least significant bit
            chip8.V[chip8.x()] >>= 1;
            chip8.V[0xF] = shiftedBit; // Set VF to the least significant bit before shifting
            break;
        }
        case 0x7:
        {
            bool carry = chip8.V[chip8.y()] >= chip8.V[chip8.x()];
            chip8.V[chip8.x()] = chip8.V[chip8.y()] - chip8.V[chip8.x()];
            chip8.V[0xF] = carry;
            break;
        }
        case 0xE:
        {
            chip8.V[chip8.x()] = chip8.V[chip8.y()]; //Quirk - configure with chip mode
            int shiftedBit = (chip8.V[chip8.x()] & 0x80) >> 7; // Get the most significant bit before shifting
            chip8.V[chip8.x()] <<= 1;
            chip8.V[0xF] = shiftedBit; // Set VF to the most significant bit before shifting
            break;
        }
        default:
            //Handle unknown opcodes
            std::cerr << "Unknown opcode: " << std::hex << chip8.opcode() << std::dec << std::endl;
            break;
    }
}

void Opcodes::handle9(Chip8 &chip8)
{
    if (chip8.V[chip8.x()] != chip8.V[chip8.y()]) // Skip next instruction if Vx != Vy
    {
        chip8.pc += 2;
    }
}

void Opcodes::handleA(Chip8 &chip8)
{
    chip8.I = chip8.nnn();
}

void Opcodes::handleB(Chip8 &chip8)
{
    if (!configuration::jumping)
        chip8.pc = chip8.nnn() + chip8.V[0]; // QUIRK - configure with Jumping - 0 if off, X if on
    else
        chip8.pc = chip8.nnn() + chip8.V[chip8.x()]; // QUIRK - configure with Jumping - 0 if off, X if on
}

void Opcodes::handleC(Chip8 &chip8)
{
    chip8.V[chip8.x()] = (rand() % 256) & chip8.nn(); // Set Vx to a random number
}

void Opcodes::handleD(Chip8 &chip8)
{
    chip8.updatec8display();
}

void Opcodes::handleE(Chip8 &chip8)
{
    switch (chip8.nn()) // 0xExnn (grab the last byte of the opcode)
    {
        case 0x9E:
            if (chip8.keypad[chip8.V[chip8.x()]]) // Skip next instruction if key Vx is pressed
            {
                chip8.pc += 2;
            }
            break;
        case 0xA1:
            if (!chip8.keypad[chip8.V[chip8.x()]]) // Skip next instruction if key Vx is not pressed
            {
                chip8.pc += 2;
            }
            break;
        default:
            std::cerr << "Unknown opcode: " << chip8.opcode() << std::endl;
            break;
    }
}

void Opcodes::handleF(Chip8 &chip8)
{
    switch (chip8.nn()) // 0xFXNN (grab the 3rd and fourth nibble of the opcode)
    {
        case 0x0A:
        {
            bool keyPressed = false;
            for (int i = 0; i < 16; ++i)
            {
                if (chip8.keypad[i]) {
                    chip8.V[chip8.x()] = i; // Set Vx to the key pressed
                    keyPressed = true;
                    break;
                }
            }
            if (!keyPressed) {
                chip8.pc -= 2; // Repeat this instruction until a key is pressed
            }
            break;
        }
        case 0x1E:
            chip8.I += chip8.V[chip8.x()]; // Add Vx to I
            break;
        case 0x07:  
            chip8.V[chip8.x()] = chip8.delayTimer; // Set VX to the value of the delay timer
            break;
        case 0x15:
            chip8.delayTimer = chip8.V[chip8.x()]; // Set the delay timer to the value of VX
            break;
        case 0x18:
            chip8.soundTimer = chip8.V[chip8.x()]; // Set the sound timer to the value of VX
            break;
        case 0x29:
            chip8.I = 0x50 + (chip8.V[chip8.x()] * 5); // Set I to the address of the font sprite for Vx
            break;
        case 0x33:
            // Store BCD representation of Vx in memory at I, I+1, I+2
            chip8.memory[chip8.I] = chip8.V[chip8.x()] / 100; // Hundreds place
            chip8.memory[chip8.I + 1] = (chip8.V[chip8.x()] / 10) % 10; // Tens place
            chip8.memory[chip8.I + 2] = chip8.V[chip8.x()] % 10; // Ones place
            break;
        case 0x55:
            // Store registers V0 to Vx in memory starting at address I
            for (int i = 0; i <= chip8.x(); ++i) 
            {
                chip8.memory[chip8.I + i] = chip8.V[i];
            }
            chip8.I += 1 + chip8.x(); // QUIRK - Increment I by the number of registers stored + 1 - Configure with chip mode
            break;
        case 0x65:
            // Read registers V0 to Vx from memory starting at address I
            for (int i = 0; i <= chip8.x(); ++i)
            {
                chip8.V[i] = chip8.memory[chip8.I + i];
            }
            chip8.I += 1 + chip8.x(); // QUIRK - Increment I by the number of registers read + 1 - Configure with chip mode
            break;
        default:
            std::cerr << "Unknown opcode: " << chip8.opcode() << std::endl;
            break;
    }
}
