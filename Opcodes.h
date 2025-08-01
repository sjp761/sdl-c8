#pragma once
class Chip8; //Using forward declaration to avoid circular dependency

class Opcodes
{
    public:
        static void handle0(Chip8& chip8);
        static void handle1(Chip8& chip8);
        static void handle2(Chip8& chip8);
        static void handle3(Chip8& chip8);
        static void handle4(Chip8& chip8);
        static void handle5(Chip8& chip8);
        static void handle6(Chip8& chip8);
        static void handle7(Chip8& chip8);
        static void handle8(Chip8& chip8);
        static void handle9(Chip8& chip8);
        static void handleA(Chip8& chip8);
        static void handleB(Chip8& chip8);
        static void handleC(Chip8& chip8);
        static void handleD(Chip8& chip8);
        static void handleE(Chip8& chip8);
        static void handleF(Chip8& chip8);
};