#ifndef PPU_H
#define PPU_H

#include <cstdint>

#include "ppu_bus.h"

enum PPU_Reg {
    PPUCTRL = 0x2000, // Various flags controlling PPU operation 
    PPUMASK = 0x2001, // This register controls the rendering of sprites and backgrounds, as well as colour effects. 
    PPUSTATUS = 0x2002, // This register reflects the state of various functions inside the PPU.
    OAMADDR = 0x2003, // This register is used to select the address to access in OAM memory.
    OAMDATA = 0x2004, // Write OAM data here. Writes will increment OAMADDR after the write; reads do not. Reads during vertical or forced blanking return the value from OAM at that address. 
    PPUSCROLL = 0x2005, // 
    PPUADDR = 0x2006,
    PPUDATA = 0x2007,
    OAMDMA = 0x4014
};

class PPU {
public:
    uint8_t regs[8];
    
private:
    uint8_t reg_v; 
    uint8_t reg_t;
    uint8_t reg_x;
    uint8_t reg_w;
    

    PPU_Bus* bus;
};

#endif

