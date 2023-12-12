#include "../include/bus.h"

void Bus::write(uint16_t addr, uint8_t data) {
    if (0 <= addr && addr <= 0x1fff){
        ram[addr % 0x800] = data;
    }
    else if (0x2000 <= addr && addr <= 0x3fff){
        return; // ppu
    }
    else if (0x4000 <= addr && addr <= 0x4017){
        return; // apu and other io registers
    }
    else if (0x4018 <= addr && addr <= 0x401f){
        return; // APU and I/O functionality that is normally disabled
    }
    else if (0x4020 <= addr && addr <= 0xffff){
        PRG_rom[addr] = data; // cartridge stuff
    }
}

uint8_t Bus::read(uint16_t addr) {
    if (0 <= addr && addr <= 0x1fff){
        return ram[addr % 0x800];
    }
    else if (0x2000 <= addr && addr <= 0x3fff){
        return 0; // ppu
    }
    else if (0x4000 <= addr && addr <= 0x4017){
        return 0; // apu and other io registers
    }
    else if (0x4018 <= addr && addr <= 0x401f){
        return 0; // APU and I/O functionality that is normally disabled
    }
    else if (0x4020 <= addr && addr <= 0xffff){
        return PRG_rom[addr - 0x4020]; // cartridge stuff
    }

    return 0;
}
