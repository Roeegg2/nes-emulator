#ifndef PPU_H
#define PPU_H

#include <cstdint>
#include <iostream>

#include "bus.h"
#include "nes_screen.h"

namespace roee_nes {

    constexpr uint8_t PT_MSB = 0b00001000;
    constexpr uint8_t PT_LSB = 0b00000000;

    constexpr uint8_t ODD_FRAME = 1;
    constexpr uint8_t EVEN_FRAME = 0;

    enum Scanline_Ranges : int16_t {
        PRE_RENDER_SCANLINE = -1,
        RENDER_START_SCANLINE = 0,
        RENDER_END_SCANLINE = 239,
        POST_RENDER_SCANLINE = 240,
        VBLANK_START_SCANLINE = 241,
        VBLANK_END_SCANLINE = 260
    };

    enum Fetch_Modes : uint8_t {
        REGULAR_FETCH,
        GARBAGE_NT_FETCH,
        ONLY_NT_FETCH
    };


    class Bus;

    enum Fetch_Type {
        FETCH_1 = 2,
        FETCH_2 = 4,
        FETCH_3 = 0,
        FETCH_4 = 6,
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
        void reset();
        
    private:
        void prerender_scanline();
        void visible_scanline();
        void vblank_scanline();
        void fetch_rendering_data(Fetch_Modes fetch_mode);

        void add_render_pixel();
        void send_pixels_to_render();

        void increment_cycle(uint8_t cycles);
        void increment_y();
        void increment_coarse_x();

        uint8_t fetch_pt_byte(uint8_t byte_significance);

        void load_attr_shift_regs();
        void log_palette_ram() const;
        void log() const;
        void log_nametable(uint64_t frame_number) const;

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

        std::array<struct Pixel, 256> data_render_line;

    public:
        Bus* bus;
        NES_Screen* screen;

    private:
        inline uint8_t Get_rendering_status() { return (ext_regs.ppumask & 0b00011000) > 0; } // IMPORTANT: when adding sprite rendering, make sure to check that bit as well!!
    };
}
#endif