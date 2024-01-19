#ifndef PPU_H
#define PPU_H

#include <cstdint>
#include <iostream>

#include "bus.h"

namespace roee_nes {

    constexpr uint8_t lsb = 0b00001000;
    constexpr uint8_t msb = 0b00000000;

    class Bus;

    enum PPU_State {
        FETCH_NT,
        FETCH_AT,
        FETCH_PT_LSB,
        FETCH_PT_MSB,
        LOAD_SHIFT_REGS
    };

    struct Background_Regs {
        uint16_t pt_shift_lsb;
        uint16_t pt_shift_msb;

        uint16_t attr_shift_lsb;
        uint16_t attr_shift_msb;

        uint8_t nt_latch;
        uint8_t at_latch;
        uint8_t pt_latch_lsb;
        uint8_t pt_latch_msb;
    };

    struct External_Registers {
        uint8_t ppuctrl;
        uint8_t ppumask;
        uint8_t ppustatus;
        uint8_t oamaddr;
    };

    class PPU {
    public:
        PPU(Bus* bus);

        void run_ppu(uint8_t cycles);

    private:
        void prerender_and_visible_scanline(uint8_t cycles);
        void vblank_scanline(uint8_t cycles);

        void increment_counters(uint8_t cycles);

        void increment_coarse_x();
        void increment_y();

        void PPU::load_bg_shift_regs(); 

        uint16_t fetch_pt_byte(uint8_t byte_significance);

    public:
        uint16_t v;
        uint16_t t;
        uint8_t x;
        uint8_t w;

        Background_Regs bg_regs;

        External_Registers ext_regs;

        uint8_t nmi_occurred;

        int32_t curr_scanline; // why does static cause an error here?
        int32_t curr_cycle;
        uint8_t odd_even_frame; // for pre-render scanline

    // private:
    //     const std::array<uint8_t, 64> palette;

    public:
        Bus* bus;

    };
}
#endif