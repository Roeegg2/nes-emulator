#ifndef PPU_H
#define PPU_H

#include <cstdint>
#include <iostream>

#include "bus.h"
#include "nes_screen.h"

#define GET_OAM_INDEX(count) ((4*count.counter.n) + count.counter.m)
#define GET_OAM_INDEX_AT(count, m) ((4*count.counter.n) + m)
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

    class Bus;

    enum BG_Fetch_Type : uint8_t {
        FETCH_1 = 1,
        FETCH_2 = 3,
        FETCH_3 = 5,
        FETCH_4 = 7,
    };

    enum FG_Sprite_Eval_Part : uint8_t {
        SPRITE_EVAL = 0,
        SPRITE_OVERFLOW = 1,
        BROKEN_READ = 2
    };
    // enum FG_Fetch_Type : uint8_t {
    //     FETCH_1 = 1,
    //     FETCH_2 = 3,
    //     FETCH_3 = 5,
    //     FETCH_4 = 7,
    // };
    typedef struct {
        uint16_t pt_shift_lsb;
        uint16_t pt_shift_msb;

        uint16_t at_shift_lsb;
        uint16_t at_shift_msb;

        uint8_t nt_latch;
        uint8_t at_latch;
        uint8_t pt_latch_lsb;
        uint8_t pt_latch_msb;
    } background_regs;

    typedef struct {
        uint8_t ppuctrl;
        uint8_t ppumask;
        uint8_t ppustatus;
        uint8_t oamaddr;
        uint8_t oamdata;
    } external_registers;

    typedef union {
        struct {
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
            uint8_t m : 2; // curr byte
            uint8_t n : 6; // curr sprite
        } counter;

        uint8_t raw;
    } sprite_counter;

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
        uint8_t fetch_bg_pt_byte(uint8_t byte_significance);
        
        void shared_visible_prerender_scanline();
        void load_shift_regs();
        void shift_shift_regs();

        void add_bg_render_pixel();
        void fill_fg_rendering_buffer();
        void combine_bg_fg_pixels();
        // void send_pixels_to_render();

        void increment_cycle(uint8_t cycles);
        void increment_y();
        void increment_coarse_x();
// #ifdef DEBUG
        void log() const;
// #endif
        uint8_t fetch_fg_pt_byte(uint8_t byte_significance, uint8_t tile_index, uint8_t y_diff);
    public:
        loopy_reg v;
        loopy_reg t;
        uint8_t x;
        uint8_t w;

        background_regs bg_regs;
        external_registers ext_regs;
        uint8_t oamdma;

        int32_t curr_scanline;
        int32_t curr_cycle;
        uint64_t frame_counter;
        
        uint8_t nmi;
        uint8_t frame_oddness;

        std::array<struct Pixel, 256> data_render_line;
        std::array<struct Sprite_Pixel, 256> fg_data_render_line;
        std::array<uint8_t, 256> primary_oam;
        std::array<uint8_t, 32> secondary_oam;
        // ppu sprites section
        sprite_counter sprite_count;
        sprite_counter sec_oam_count;

        uint8_t sprite_rendering_stage;
        uint8_t oam_write_status;
        uint8_t sprite_0_hit_next_line;
        
    public:
        class Bus* bus;
        NES_Screen* screen;

    private:
        inline uint8_t Get_rendering_status() { return (ext_regs.ppumask & 0b00011000) > 0; } // IMPORTANT: when adding sprite rendering, make sure to check that bit as well!!
    };
}
#endif