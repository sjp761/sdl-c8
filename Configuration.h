#include <cstdint>
#include <ini.h>
#include <INIReader.h>
namespace configuration
{
    constexpr int WINDOW_WIDTH = 64;
    constexpr int WINDOW_HEIGHT = 32;
    constexpr uint32_t DEFAULT_COLOR = 0x00000000; // Black in RGBA format
    constexpr int SCALE_FACTOR = 20;
    extern bool vfReset;
    extern bool clipping;
    extern bool jumping;
    extern int mode;
    void readConfiguration(const char* filename);
}