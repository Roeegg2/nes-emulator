#include <iostream>

#include "../include/bus.h"
#include "../include/mappers/nrom_0.h"

Bus::Bus(Mapper* mapper) {
    this->mapper = mapper;
}

void Bus::cpu_write(uint16_t addr, uint8_t data) {
    if (0 <= addr && addr <= 0x1fff)
        cpu_ram[addr % 0x800] = data;
    else if (0x2000 <= addr && addr <= 0x3fff){
        std::cout << "PPU REG WRITE" << std::endl; // for debugging
        // something here
    }
    else if (0x4000 <= addr && addr <= 0x4017)
        return; // apu related - didnt implement yet
    else if (0x4018 <= addr && addr <= 0x401f)
        return; // apu related - didnt implement yet
    else if (0x4020 <= addr && addr <= 0xffff)
        mapper->cpu_write(addr, data);
}

uint8_t Bus::cpu_read(uint16_t addr) {
    if (0 <= addr && addr <= 0x1fff)
        return cpu_ram[addr % 0x800];
    else if (0x2000 <= addr && addr <= 0x3fff)
        return 1; // something here
    else if (0x4000 <= addr && addr <= 0x4017)
        return 0; // apu related - didnt implement yet
    else if (0x4018 <= addr && addr <= 0x401f)
        return 0; // apu related - didnt implement yet
    else if (0x4020 <= addr && addr <= 0xffff)
        return mapper->cpu_read(addr);

    return 0;
}