#include "../include/cpu_bus.h"
#include "../include/mappers/nrom_0.h"

#include <iostream>

void CPU_Bus::write(uint16_t addr, uint8_t data) {
    if (0 <= addr && addr <= 0x1fff)
        ram[addr % 0x800] = data;
    else if (0x2000 <= addr && addr <= 0x3fff)
        ppu->regs[addr % 0x8] = data; 
    else if (0x4000 <= addr && addr <= 0x4017)
        return; // apu related - didnt implement yet
    else if (0x4018 <= addr && addr <= 0x401f)
        return; // apu related - didnt implement yet
    else if (0x4020 <= addr && addr <= 0xffff)
        mapper->cpu_write(addr, data);
}

uint8_t CPU_Bus::read(uint16_t addr) {
    if (0 <= addr && addr <= 0x1fff)
        return ram[addr % 0x800];
    else if (0x2000 <= addr && addr <= 0x3fff)
        return ppu->regs[addr % 0x8];
    else if (0x4000 <= addr && addr <= 0x4017)
        return 0; // apu related - didnt implement yet
    else if (0x4018 <= addr && addr <= 0x401f)
        return 0; // apu related - didnt implement yet
    else if (0x4020 <= addr && addr <= 0xffff)
        return mapper->cpu_read(addr);

    return 0;
}