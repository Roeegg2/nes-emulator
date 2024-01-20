#ifndef MAIN_BUS_H
#define MAIN_BUS_H

#include <cstdint>
#include <array>

#include <iostream>

#include "mapper_n_cart.h"
#include "mappers/nrom_0.h"
#include "ppu.h"
#include "cpu.h"

namespace roee_nes {

    class PPU;
    class CPU;

    enum PPU_Regs : uint8_t {
        PPUCTRL = 0,
        PPUMASK = 1,
        PPUSTATUS = 2,
        OAMADDR = 3,
        OAMDATA = 4,
        PPUSCROLL = 5,
        PPUADDR = 6,
        PPUDATA = 7
    };

    class Bus {
    public:
        std::array<uint8_t, 0x800> ram; // 0x0000 - 0x07FF, 3 mirrors + real | 0x2000 size
        std::array<uint8_t, 0x1000> vram;

        Mapper* mapper;
        PPU* ppu;
        CPU* cpu;

    public:
        Bus(Mapper* mapper);

        uint8_t cpu_read(uint16_t addr);
        void cpu_write(uint16_t addr, uint8_t data);

        uint8_t ppu_read(uint16_t addr);
    };

}
#endif