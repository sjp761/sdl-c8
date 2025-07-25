#include <cstdint>

class Chip8
{
    public:
        uint8_t memory[4096]; // 4KB of memory
        enum emulationState { RUNNING, PAUSED, STOPPED };
        emulationState state;
        Chip8() : state(RUNNING) 
        {
            // Initialize memory and other components if necessary
            for (int i = 0; i < 4096; ++i) {
                memory[i] = 0;
            }
        }
};