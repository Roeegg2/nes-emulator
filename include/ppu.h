#ifndef PPU_H
#define PPU_H

#include <cstdint>
#include "bus.h"

class Bus; // forward declaration to avoid circular dependency

class PPU {
public:
    uint8_t regs[8];
    
    void print_yay();
private:
    uint8_t reg_v; 
    uint8_t reg_t;
    uint8_t reg_x;
    uint8_t reg_w;

    Bus* bus;
};

#endif

