#include "../include/ppu.h"

namespace roee_nes {

    PPU::PPU(Bus* bus, NES_Screen* screen)
        : curr_cycle(0), curr_scanline(261), frame_oddness(0), nmi(0), w(0) // NOTE: not sure about these yet!
    {
        this->bus = bus;
        this->screen = screen;
    }

    void PPU::run_ppu(uint8_t cycles) {
        for (uint8_t i = 0; i < cycles; i++) {
            // in this situation we need to skip a cycle 
            if (Get_rendering_status()) {
                if (frame_oddness == ODD_FRAME && curr_scanline == PRE_RENDER_SCANLINE && curr_cycle == 339) // on odd frames we skip the last cycle of the pre-render scanline
                    increment_counters(1);
                if (curr_cycle == 256) // increment y component of v
                    increment_y();
                if (curr_cycle == 257)
                    // copy all horizontal bits from t onto v;
                if ((328 <= curr_cycle || curr_cycle <= 256) && curr_cycle & 8 == 0)
                    // increment the coarse x component of v
            }

            if (curr_scanline == PRE_RENDER_SCANLINE)
                prerender_scanline();
            else if (RENDER_START_SCANLINE <= curr_scanline && curr_scanline <= RENDER_END_SCANLINE)
                render_scanline();
            else if (VBLANK_START_SCANLINE <= curr_scanline && curr_scanline <= VBLANK_END_SCANLINE)
                vblank_scanline();
            else if (curr_scanline == POST_RENDER_SCANLINE)
                // do nothing!
        }
    }

    void PPU::prerender_scanline() {
        if (curr_cycle == 1)
            ext_regs.ppustatus &= 0b01111111; // clearing vblank flag
        else if (Get_rendering_status() && 280 <= curr_cycle && curr_cycle <= 304)
            // copy vertical bits from t onto v;
        /**
         * if (curr_cycle is between 1-256 or between 321-336)
         *   fetch_bytes()
        */
    }

    void PPU::render_scanline() {
        /**
         * if (curr_cycle is between 1-256 or between 321-336){
         *   fetch_bytes()
         *   render_pixel()
         * }
         * else if (curr_cycle is between 257 and 320) {
         *   fetch garbage nt byte0
         *   fetch garbage nt byte1
         *   fetch pattern table low
         *   fetch pattern table high
         * }
         * else if (curr_cycle is between 337 and 340)
         *   fetch nt byte0
         *   fetch nt byte1
         * if (curr cycle is 8th cycle)
         *   transfer attr 2 bits (01, 23, 45, or 67) into the *high bits* of attr_shift_reg
         *   transfer pt0 into the *high bits* of pt0_shift_reg
         *   increment scroll x component of v
         *
        */
    }

    void PPU::vblank_scanline() {
        if (curr_scanline == VBLANK_START_SCANLINE && curr_cycle == 1)
            ext_regs.ppustatus |= 0b10000000; // set blank flag

        /**
         * set scroll position at the end of vblank
         * NOTE: Set the scroll last. After using PPUADDR ($2006), the program must always set PPUSCROLL again. They have a shared internal register and using PPUADDR will overwrite the scroll position.
         *
        */
    }

    void PPU::fetch_rendering_data() {
        /**
         * fetch nt, then attr, then pt0, then pt1
         * {
         * if (fetch_num == 1)
         *  place data on the ppu bus
         * else
         *  read the data from the ppu bus
         * }
        */
    }

    void PPU::render_pixel() {
        /**
         * get pt bit high using x reg
         * get pt bit low using x reg
         * get attr bit high using x reg
         * get attr bit low using x reg
        */
    }

    // /* I am a bit simplfying this, usually there is an insertion and fetch in different address*/
    // void PPU::prerender_and_visible_scanline() {
    //     static uint8_t fetching_cycle_counter = 0;
    //     static PPU_State next_fetch = FETCH_NT;

    //     fetching_cycle_counter += 1;

    //     if (curr_cycle == 0 && 0 <= curr_scanline <= 239)
    //         return; // do later, but pretty much just idle.

    //     if (1 <= curr_cycle <= 256 && curr_scanline != -1) {// rendering cycle
    //         Color* color = get_pixel_to_render();
    //         screen->draw_pixel(curr_cycle, curr_scanline, color->r, color->g, color->b);
    //     }

    //     if (next_fetch == LOAD_SHIFT_REGS) // loading takes one cycle, so if we need to load, we always can
    //     {
    //         load_bg_shift_regs();
    //         next_fetch = static_cast<PPU_State>(static_cast<int>(next_fetch) + 1);
    //     }
    //     if (fetching_cycle_counter > 1) // making sure we have at least 2 cycles in the bank
    //     {
    //         fetch_rendering_data(next_fetch);

    //         next_fetch = static_cast<PPU_State>(static_cast<int>(next_fetch) + 1);
    //         fetching_cycle_counter -= 2;
    //     }
    // }

    // void PPU::vblank_scanline() {
    //     if (curr_scanline == 241 && curr_cycle == 1 /*&& ext_regs.ppuctrl & 0b10000000 */) {
    //         nmi = 1;
    //         ext_regs.ppustatus |= 0b10000000;
    //     }

    // }

    // uint16_t PPU::fetch_pt_byte(uint8_t byte_significance) {
    //     uint16_t pt_byte_addr = bg_regs.nt_latch; // 0b00000000dddddddd
    //     pt_byte_addr <<= 4;                       // 0b0000dddddddd0000
    //     pt_byte_addr |= v >> 12;                  // 0b0000ddddddddvvvv
    //     pt_byte_addr |= byte_significance;        // fetching msb/lsb // 0b0000ddddddddsvvv
    //     pt_byte_addr |= (ext_regs.ppuctrl & 0b00010000) << 8;

    //     return bus->ppu_read(pt_byte_addr);
    // }

    // void PPU::load_bg_shift_regs() {
    //     uint8_t attr_loading_shift;

    //     bg_regs.pt_shift_lsb = (bg_regs.pt_shift_lsb & 0xFF00) | bg_regs.pt_latch_lsb;
    //     bg_regs.pt_shift_msb = (bg_regs.pt_shift_msb & 0xFF00) | bg_regs.pt_latch_msb;

    //     auto get_coarse_x = [](uint16_t v) -> uint8_t { return v & 0b00011111; };
    //     auto get_coarse_y = [](uint16_t v) -> uint8_t { return (v & 0b11111000000) >> 5; };

    //     // 0, 0 -> 2|, 3
    //     // 0, 1 -> 4|, 5
    //     // 1, 0 -> 0|, 1
    //     // 1, 1 -> 6|, 7
    //     // getting the attribute bits for the palette table
    //     auto get_first_bit = [](uint8_t coarse_x, uint8_t coarse_y) -> uint8_t { // will rewrite this lambda later
    //         if (coarse_x % 2 == 0) {
    //             if (coarse_y % 2 == 0)
    //                 return 2; // fetch bits for top right
    //             else
    //                 return 4; // fetch bits for bottom right
    //         } else {
    //             if (coarse_y % 2 == 0)
    //                 return 0; // fetch bits for top left
    //             else
    //                 return 6; // fetch bits for bottom left
    //         }
    //         };

    //     attr_loading_shift = get_first_bit(get_coarse_x(v), get_coarse_y(v));

    //     bg_regs.attr_shift_msb = (bg_regs.at_latch >> attr_loading_shift) && 0b00000001;
    //     bg_regs.attr_shift_lsb = (bg_regs.at_latch >> (attr_loading_shift + 1)) && 0b00000001;

    //     // if reg = 0b01 => 0b11111111, if reg = 0b00 => 0b00000000
    //     bg_regs.attr_shift_lsb *= 0b11111111;
    //     bg_regs.attr_shift_msb *= 0b11111111;
    // }

    // void PPU::fetch_rendering_data(uint8_t next_fetch) {
    //     switch (next_fetch) {
    //         case FETCH_NT:
    //             bg_regs.nt_latch = bus->ppu_read(0x2000 | (v & 0x0FFF)); // CHECK THAT!
    //             break;
    //         case FETCH_AT:
    //             bg_regs.at_latch = bus->ppu_read(/*0010001111000000 | */ (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07));
    //             break;
    //         case FETCH_PT_LSB:
    //             bg_regs.pt_latch_lsb = fetch_pt_byte(LSB);
    //             break;
    //         case FETCH_PT_MSB:
    //             bg_regs.pt_latch_msb = fetch_pt_byte(MSB);
    //             break;
    //     }
    // }

    // void PPU::increment_counters(uint8_t cycles) {
    //     curr_cycle += cycles;

    //     if (curr_cycle >= 340) {
    //         curr_cycle %= 340;
    //         curr_scanline++;
    //     }
    //     if (curr_scanline == 261) {
    //         curr_scanline = -1;
    //         odd_even_frame = 1 - odd_even_frame; // toggeling odd_even_frame
    //     }
    // }

    // void PPU::increment_v_x() { // pseudocode taken from the nesDEV wiki
    //     if (v & 0b00011111 == 31) {
    //         v &= 0b0111111111100000; // setting coarse x to 0
    //         v ^= 0b0000010000000000; // moving to the next nametable
    //     } else
    //         v += 1;
    // }

    // void PPU::increment_v_y() { // pseudocode taken from the nesDEV wiki
    //     if (v & 0b011100000000000 != 0b011100000000000)
    //         v += 0b0000100000000000;
    //     else {
    //         v &= ~0b011100000000000;                   // setting fine scroll y to 0
    //         uint8_t y = (v & 0b0000001111100000) >> 5; // extracting coarse y
    //         if (y == 29) {
    //             y = 0;
    //             v ^= 0b0000100000000000; // switching vertical nametable
    //         } else if (y == 31)
    //             y = 0; // got to the end of the screen
    //         else {
    //             y += 1;                                   // increment coarse y
    //             v = (v & ~0b0000001111100000) | (y << 5); // putting coarse y back in v
    //         }
    //     }
    // }

    // void PPU::reset() { // change later!
    //     x = 0;
    //     w = 0;
    //     v = 0;
    //     t = 0;
    //     curr_cycle = 0;
    //     curr_scanline = 0;
    //     odd_even_frame = 0;
    //     nmi = 0;
    //     bg_regs.at_latch = 0;
    //     bg_regs.attr_shift_lsb = 0;
    //     bg_regs.attr_shift_msb = 0;
    //     bg_regs.nt_latch = 0;
    //     bg_regs.pt_latch_lsb = 0;
    //     bg_regs.pt_latch_msb = 0;
    //     bg_regs.pt_shift_lsb = 0;
    //     bg_regs.pt_shift_msb = 0;
    //     ext_regs.ppuctrl = 0;
    //     ext_regs.ppumask = 0;
    //     ext_regs.ppustatus = 0;
    //     ext_regs.oamaddr = 0;
    // }

    // struct Color* PPU::get_pixel_to_render() {
    //     uint8_t pt_data = bg_regs.pt_shift_lsb >> (7 - x);
    //     pt_data |= (bg_regs.pt_shift_msb >> (7 - x)) << 1;
    //     pt_data &= 0b00000011;

    //     uint8_t attr_data = bg_regs.attr_shift_lsb >> (7 - x);
    //     attr_data |= (bg_regs.attr_shift_msb >> (7 - x)) << 1;
    //     attr_data &= 0b00000011;

    //     return bus->ppu_get_color(0x3f00 + (bus->ppu_read(0x3F00 + (attr_data * 4) + pt_data)));
    // }
}