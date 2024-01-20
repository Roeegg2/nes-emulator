#ifndef PPU_H
#define PPU_H

#include <cstdint>
#include <iostream>

#include "bus.h"
#include "nes_screen.h"

namespace roee_nes
{

    constexpr uint8_t lsb = 0b00001000;
    constexpr uint8_t msb = 0b00000000;

    class Bus;

    const std::array<uint32_t, 64> palette = 
    {
        0x666666ff, 0x002a88ff, 0x1412a7ff, 0x3b00a4ff, 0x5c007eff, 0x6e0040ff, 0x6c0600ff, 0x561d00ff,
        0x333500ff, 0x0b4800ff, 0x005200ff, 0x004f08ff, 0x00404dff, 0x000000ff, 0x000000ff, 0x000000ff,
        0xadadadff, 0x155fd9ff, 0x4240ffff, 0x7527feff, 0xa01accff, 0xb71e7bff, 0xb53120ff, 0x994e00ff,
        0x6b6d00ff, 0x388700ff, 0x0c9300ff, 0x008f32ff, 0x007c8dff, 0x000000ff, 0x000000ff, 0x000000ff,
        0xfffeffff, 0x64b0ffff, 0x9290ffff, 0xc676ffff, 0xf36affff, 0xfe6eccff, 0xfe8170ff, 0xea9e22ff,
        0xbcbe00ff, 0x88d800ff, 0x5ce430ff, 0x45e082ff, 0x48cddeff, 0x4f4f4fff, 0x000000ff, 0x000000ff,
        0xfffeffff, 0xc0dfffff, 0xd3d2ffff, 0xe8c8ffff, 0xfbc2ffff, 0xfec4eaff, 0xfeccc5ff, 0xf7d8a5ff,
        0xe4e594ff, 0xcfef96ff, 0xbdf4abff, 0xb3f3ccff, 0xb5ebf2ff, 0xb8b8b8ff, 0x000000ff, 0x000000ff,
    };
    
    enum PPU_State
    {
        FETCH_NT,
        FETCH_AT,
        FETCH_PT_LSB,
        FETCH_PT_MSB,
        LOAD_SHIFT_REGS
    };

    struct Background_Regs
    {
        uint16_t pt_shift_lsb;
        uint16_t pt_shift_msb;

        uint16_t attr_shift_lsb;
        uint16_t attr_shift_msb;

        uint8_t nt_latch;
        uint8_t at_latch;
        uint8_t pt_latch_lsb;
        uint8_t pt_latch_msb;
    };

    struct External_Registers
    {
        uint8_t ppuctrl;
        uint8_t ppumask;
        uint8_t ppustatus;
        uint8_t oamaddr;
    };

    class PPU
    {
    public:
        PPU(Bus *bus, NES_Screen *screen);

        void run_ppu(uint8_t cycles);
        void reset();

    private:
        void prerender_and_visible_scanline();
        void vblank_scanline();

        void increment_counters(uint8_t cycles);

        void increment_coarse_x();
        void increment_y();

        void load_bg_shift_regs();

        uint16_t fetch_pt_byte(uint8_t byte_significance);
        void fetch_rendering_data(uint8_t next_fetch);

        void render_pixel();

    public:
        uint16_t v;
        uint16_t t;
        uint8_t x;
        uint8_t w;

        Background_Regs bg_regs;

        External_Registers ext_regs;

        int curr_scanline; // why does static cause an error here?
        int curr_cycle;
        uint8_t odd_even_frame; // for pre-render scanline
        
        uint8_t nmi;

    public:
        Bus *bus;
        NES_Screen *screen;
    };
}
#endif