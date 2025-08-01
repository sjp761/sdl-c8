#include "Configuration.h"

namespace configuration
{
    bool vfReset = true;
    bool clipping = true;
    bool jumping = false;
    int mode = 0; // 0 for CHIP-8, 1 for SCHIP-8
}

void configuration::readConfiguration(const char *filename)
{
}