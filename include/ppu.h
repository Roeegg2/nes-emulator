#ifndef PPU_H
#define PPU_H

#include <cstdint>

#include "bus.h"

class PPU {
public:
    PPU(Bus* bus);

private:
    Bus* bus;

    uint8_t v;
    uint8_t t;
    uint8_t x;
    uint8_t w;
};

#endif

