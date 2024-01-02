#include "../include/ppu_bus.h"

void PPU_Bus::write(uint16_t addr, uint8_t data) {
    if (addr >= 0x0000 && addr <= 0x1FFF) {
        
    } else if (addr >= 0x2000 && addr <= 0x3FFF) {
        mapper->write(addr, data);
    }
}