#include "../include/bus.h"
#include "../include/mappers/nrom_0.h"

#include <iostream>

Bus::Bus(Mapper* mapper) {
    this->mapper = mapper;
}

void Bus::cpu_write(uint16_t addr, uint8_t data) {
    uint16_t temp = data;

    if (0 <= addr && addr <= 0x1fff)
        ram[addr % 0x800] = data;
    else if (0x2000 <= addr && addr <= 0x3fff) {
        switch (addr % 8) {
        case PPUCTRL:
            // ppu->t = ppu->t & (0b00000011 & data);
            break;
        case PPUMASK:
            break;
        case PPUSTATUS:
            break;
        case OAMADDR:
            break;
        case OAMDATA:
            break;
        case PPUSCROLL:
            // if (ppu->w == 0) {
            //     ppu->t = ((ppu->t >> 5) << 5) & (temp >> 3);
            //     ppu->x = temp & 0b00000111;
            //     ppu->w = 1; // NOTE: move this out of the if statement
            // }
            // else {
            //     ppu->t = (ppu->t & 0b0000110000011111); // note: i set another 0 at the start - because the register is 16 bits, and not 15 bits like it should be
            //     ppu->t = ppu->t | (temp << 12);
            //     ppu->t = ppu->t | ((temp >> 3) << 5);
            //     ppu->w = 0; // NOTE: move this out of the if statement
            // }
            break;
        case PPUADDR:
            // if (ppu->w == 0) {
            //     temp = temp & 0b00111111;
            //     ppu->t = (ppu->t & 0b0000000011111111) | (data << 8);
            //     ppu->w = 1; // NOTE: move this out of the if statement
            // }
            // else {
            //     ppu->t = (ppu->t & 0b1111111100000000) | temp;
            //     ppu->v = ppu->t;
            //     ppu->w = 0; // NOTE: move this out of the if statement
            // }
            break;
        case PPUDATA:
            break;

        }
    }
    else if (0x4000 <= addr && addr <= 0x4017)
        return; // apu related - didnt implement yet
    else if (0x4018 <= addr && addr <= 0x401f)
        return; // apu related - didnt implement yet
    else if (0x4020 <= addr && addr <= 0xffff)
        mapper->cpu_write(addr, data);
}

uint8_t Bus::cpu_read(uint16_t addr) {
    if (0 <= addr && addr <= 0x1fff) {
        return ram[addr % 0x800];
    }
    else if (0x2000 <= addr && addr <= 0x3fff) {
        switch (addr % 8) {
        case PPUCTRL:
            break;
        case PPUMASK:
            break;
        case PPUSTATUS:
            // ppu->w = 0;
            // ppu->ppustatus = ppu->ppustatus & 0b01111111;
            break;
        case OAMADDR:
            break;
        case OAMDATA:
            break;
        case PPUSCROLL:
            break;
        case PPUADDR:
            break;
        case PPUDATA:
            break;

        }

        return 1;
    }
    else if (0x4000 <= addr && addr <= 0x4017) {
        return 0; // apu related - didnt implement yet
    }
    else if (0x4018 <= addr && addr <= 0x401f) {
        return 0; // apu related - didnt implement yet
    }
    else if (0x4020 <= addr && addr <= 0xffff) {
        return mapper->cpu_read(addr);
    }

    return 0;
}

// uint8_t Bus::ppu_read(uint16_t addr) {
//     if (0 <= addr && addr <= 0x1fff)
//         return mapper->ppu_read(addr);
//     else if (0x2000 <= addr && addr <= 0x3eff)
//         return mapper->ppu_read(addr);
//     else if (0x3f00 <= addr && addr <= 0x3fff)
//         return 0;

//     return 0;
// }