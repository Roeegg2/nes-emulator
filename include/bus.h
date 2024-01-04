#ifndef BUS_H
#define BUS_H

#include <cstdint>

#include "mapper_n_cart.h"
// #include "ppu.h"
#include "cpu.h"

class Bus{
public:
    uint8_t cpu_read(uint16_t addr);
    void cpu_write(uint16_t addr, uint8_t data);

    // uint8_t ppu_read(uint16_t addr);

    Bus(Mapper* mapper);

// cpu stuff
private:
    uint8_t cpu_ram[0x800]; // 0x0000 - 0x07FF, 3 mirrors + real | 0x2000 size

// ppu stuff 
    uint8_t ppu_basic_ram[0x800];

// components
    Mapper* mapper;
    
};

#endif
