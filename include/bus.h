#ifndef MAIN_BUS_H
#define MAIN_BUS_H

#include <cstdint>
#include <array>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>

#include "mapper_n_cart.h"
#include "mappers/nrom_0.h"
#include "ppu.h"
#include "cpu.h"
#include "controller.h"

namespace roee_nes {
    constexpr uint16_t OAMDMA = 0x4014;

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
        PPUDATA = 7,
    };

    class Bus {
    public:
        std::array<uint8_t, 0x800> ram; // 0x0000 - 0x07FF, 3 mirrors + real | 0x2000 size
        std::array<uint8_t, 0x800> nt_vram; // ram used for the nametables
        std::array<uint8_t, 32> palette_vram; // ram used for the palettes
        std::array<uint8_t, 64 * 3> color_palette;

        Mapper* mapper;
        PPU* ppu;
        CPU* cpu;
        Controller* controller1;
        Controller* controller2;
        
        uint8_t ppu_stupid_buffer;
        uint16_t cpu_sleep_dma_counter;
    public:
        Bus(Mapper* mapper, Controller* controller1, Controller* controller2, const std::string* palette_path);
        uint8_t cpu_read(uint16_t addr);
        void cpu_write(uint16_t addr, uint8_t data);
        uint8_t ppu_read(uint16_t addr, bool came_from_cpu = false);
        void ppu_write(uint16_t addr, uint8_t data, bool came_from_cpu = false);
        struct Color* ppu_get_color(uint16_t addr);
        uint8_t cpu_read_ppu(uint16_t addr);
        void cpu_write_ppu(uint16_t addr, uint8_t data);
        uint8_t cpu_read_controller(Controller* controller);
        
#ifdef DEBUG
        void full_log() const;
        void find_difference() const;
#endif
    private:
        void init_palette(const std::string* palette_path);
    };

}
#endif