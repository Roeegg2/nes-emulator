#ifndef MAIN_BUS_H
#define MAIN_BUS_H

#include <cstdint>
#include <array>

#include "mapper_n_cart.h"
#include "ppu.h"
#include "cpu.h"

namespace roee_nes {
    class PPU;
    class CPU;

    enum PPU_Regs : uint16_t {
        PPUCTRL = 0x2000,
        PPUMASK = 0x2001,
        PPUSTATUS = 0x2002,
        OAMADDR = 0x2003,
        OAMDATA = 0x2004,
        PPUSCROLL = 0x2005,
        PPUADDR = 0x2006,
        PPUDATA = 0x2007
    };


    class Bus {
    public:
        std::array<uint8_t, 0x800> ram; // 0x0000 - 0x07FF, 3 mirrors + real | 0x2000 size

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