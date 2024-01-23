#ifndef PPU_H
#define PPU_H

#include <cstdint>
#include <iostream>

#include "bus.h"
#include "nes_screen.h"

namespace roee_nes {

    constexpr uint8_t LSB = 0b00001000;
    constexpr uint8_t MSB = 0b00000000;

    constexpr uint8_t PRE_RENDER_SCANLINE = -1;
    constexpr uint8_t RENDER_START_SCANLINE = 0;
    constexpr uint8_t RENDER_END_SCANLINE = 239;
    constexpr uint8_t POST_RENDER_SCANLINE = 240;
    constexpr uint8_t VBLANK_START_SCANLINE = 241;
    constexpr uint8_t VBLANK_END_SCANLINE = 260;

    constexpr uint8_t ODD_FRAME = 1;
    constexpr uint8_t EVEN_FRAME = 0;

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
        PPU(Bus* bus, NES_Screen* screen);

        void run_ppu(uint8_t cycles);
    //     void reset();

    // private:
    //     void prerender_and_visible_scanline();
    //     void vblank_scanline();

    //     void increment_counters(uint8_t cycles);

    //     void increment_v_x();
    //     void increment_v_y();

    //     void load_bg_shift_regs();

    //     uint16_t fetch_pt_byte(uint8_t byte_significance);
    //     void fetch_rendering_data(uint8_t next_fetch);

    //     struct Color* get_pixel_to_render();
    private:
        void prerender_scanline();
        void render_scanline();
        void vblank_scanline();
        void fetch_rendering_data();
        void render_pixel();

    public:
        uint16_t v;
        uint16_t t;
        uint8_t x;
        uint8_t w;

        Background_Regs bg_regs;

        External_Registers ext_regs;

        int32_t curr_scanline; // why does static cause an error here?
        int32_t curr_cycle;

        uint8_t nmi;
        uint8_t frame_oddness;

    public:
        Bus* bus;
        NES_Screen* screen;

    private:
        inline uint8_t Get_rendering_status() { return (ext_regs.ppumask & 0b00011000) > 0; } // IMPORTANT: when adding sprite rendering, make sure to check that bit as well!!
    };
}
#endif