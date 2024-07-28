#ifndef MAIN_BUS_H
#define MAIN_BUS_H

#include <cstdint>
#include <array>
#include <string>

#include "mapper_n_cart.h"
#include "ppu.h"
#include "cpu.h"
#include "controller.h"

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
        Controller* controller_1;
        Controller* controller_2;
        
        uint8_t cpu_dma_controllers_open_bus;
        uint16_t cpu_sleep_dma_counter;

    public:
        Bus(CPU* cpu, PPU* ppu, Mapper* mapper, Controller* controller_1, Controller* controller_2, const std::string& palette_path);
        uint8_t cpu_read(uint16_t addr);
        void cpu_write(uint16_t addr, uint8_t data);
        uint8_t ppu_read(uint16_t addr, bool came_from_cpu = false);
        void ppu_write(uint16_t addr, uint8_t data, bool came_from_cpu = false);
        struct Color* ppu_get_color(uint16_t addr);

#ifdef DEBUG
        void full_log() const;
#endif
    private:
        void init_palette(const std::string& palette_path);
    };

}
#endif