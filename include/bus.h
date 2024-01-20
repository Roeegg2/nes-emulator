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

    struct Color {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };
    class Bus {
        public:
        std::array<uint8_t, 0x800> ram; // 0x0000 - 0x07FF, 3 mirrors + real | 0x2000 size
        std::array<uint8_t, 0x1000> vram;

        std::array<Color, 64> palette;

        Mapper* mapper;
        PPU* ppu;
        CPU* cpu;

        public:
        Bus(Mapper* mapper, std::string palette_path);

        uint8_t cpu_read(uint16_t addr);
        void cpu_write(uint16_t addr, uint16_t data);

        uint32_t ppu_read(uint16_t addr);
        Color* ppu_get_color(uint16_t addr);

        private:
        void init_palette(std::string palette_path);
    };

}
#endif