#ifndef PPU_BUS_H
#define PPU_BUS_H

#include "mapper_n_cart.h"
#include "bus.h"

#include <cstdint>

class PPU_Bus : Bus{
public:
    void write(uint16_t addr, uint8_t data);
    uint8_t read(uint16_t addr); 

private:
    Mapper* mapper;

    uint8_t nametables[0x400 * 4];
};

#endif