#ifndef CPU_BUS_H
#define CPU_BUS_H

#include "mapper_n_cart.h"
#include "ppu.h"
#include "bus.h"

#include <cstdint>

class CPU_Bus : Bus{
public:
    uint8_t ram[0x800]; // 0x0000 - 0x07FF, 3 mirrors + real | 0x2000 size
    uint8_t apu_regs[0x18];

    Mapper* mapper;
    PPU* ppu;
    
public:
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);
};

#endif