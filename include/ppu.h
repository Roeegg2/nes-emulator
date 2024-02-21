#ifndef PPU_H
#define PPU_H

#include <cstdint>
#include <iostream>

#include "bus.h"
#include "nes_screen.h"

#define INDEX_OAM(index) ((4*index.counter.n) + index.counter.m)
#define INDEX_OAM_AT(index, m) ((4*index.counter.n) + m)
namespace roee_nes {
    constexpr uint8_t PT_MSB = 0b00001000;
    constexpr uint8_t PT_LSB = 0b00000000;
    constexpr uint8_t ODD_FRAME = 1;
    constexpr uint8_t EVEN_FRAME = 0;

    enum Scanline_Ranges : int16_t {
        PRE_RENDER_SCANLINE = 261,
        RENDER_START_SCANLINE = 0,
        RENDER_END_SCANLINE = 239,
        POST_RENDER_SCANLINE = 240,
        VBLANK_START_SCANLINE = 241,
        VBLANK_END_SCANLINE = 260
    };

    enum Fetch_Modes : uint8_t {
        REGULAR_FETCH,
        ONLY_NT_FETCH
    };

    enum Sprite_Rendering_Modes {
        SPRITE_EVAL,
        SPRITE_OVERFLOW,
        BROKEN_READ
    };

    class Bus;

    enum Fetch_Type {
        FETCH_1 = 1,
        FETCH_2 = 3,
        FETCH_3 = 5,
        FETCH_4 = 7,
    };

    struct Background_Regs {
        uint16_t pt_shift_lsb;
        uint16_t pt_shift_msb;

        uint16_t at_shift_lsb;
        uint16_t at_shift_msb;

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

    typedef union {
        struct Scroll_View {
            uint16_t coarse_x : 5;
            uint16_t coarse_y : 5;
            uint16_t nt : 2;
            uint16_t fine_y : 3;
            uint16_t unused : 1;
        } scroll_view;

        uint16_t raw;
    } loopy_reg;

    typedef union {
        struct {
            uint8_t m : 2;
            uint8_t n : 6;
        } counter;
        uint8_t raw;
    } oam_counter;
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
        uint8_t fetch_pt_byte(uint8_t byte_significance);
        
        void shared_visible_prerender_scanline();
        void load_shift_regs();
        void shift_shift_regs();

        void add_render_pixel();
        void send_pixels_to_render();

        void increment_cycle(uint8_t cycles);
        void increment_y();
        void increment_coarse_x();
#ifdef DEBUG
        void log() const;
        void print_oam();
#endif

        uint8_t fetch_fg_pt_byte(uint16_t priority, uint8_t y_diff, uint16_t tile, uint8_t at_byte_2);
        void merge_bg_fg_render_line();
        void fill_fg_render_line();
        void sprite_evaluation();
        void sprite_overflow_check();
    public:
        loopy_reg v;
        loopy_reg t;
        uint8_t x;
        uint8_t w;

        struct Background_Regs bg_regs;
        struct External_Registers ext_regs;
        uint8_t oamdma;

        oam_counter poam_count;
        oam_counter soam_count;

        int32_t curr_scanline; // why does static cause an error here?
        int32_t curr_cycle;
        uint64_t frame_counter;
        uint8_t sprite_rendering_stage;
        
        uint8_t nmi;
        uint8_t frame_oddness;
        uint8_t sprite_0_next;

        std::array<struct Pixel, 256> data_render_line;
        std::array<struct Entity_Pixel, 256> fg_data_render_line;
        std::array<uint8_t, 256> primary_oam;
        std::array<uint8_t, 32> secondary_oam;

    public:
        class Bus* bus;
        NES_Screen* screen;

    private:
        inline uint8_t Get_rendering_status() { return (ext_regs.ppumask & 0b00011000) > 0; } // IMPORTANT: when adding sprite rendering, make sure to check that bit as well!!
    };
}
#endif