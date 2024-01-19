#include "../include/ppu.h"

namespace roee_nes {

    PPU::PPU(Bus* bus)
        : curr_cycle(0), curr_scanline(261), odd_even_frame(0), nmi_occurred(0), w(0)
    {
        this->bus = bus;

        // curr_scanline = 261; // NOTE: not sure about this yet!
        // curr_cycle = 0;

        // w = 0;
        // odd_even_frame = 0; // NOTE: not sure about this yet!
        // nmi_occurred = 0; // NOTE: not sure about this yet!
    }

    void PPU::run_ppu(uint8_t cycles) {
        if (-1 <= curr_scanline <= 239) // pre-render and visible scanlines
            prerender_and_visible_scanline(cycles);
        else // vblank scanlines
            vblank_scanline(cycles);
    }

    /* I am a bit simplfying this, usually there is an insertion and fetch in different address*/
    void PPU::prerender_and_visible_scanline(uint8_t cycles) {
        static uint8_t run_cycles = 0;
        static PPU_State where_did_i_stop = FETCH_NT;

        if (curr_cycle == 0) {
            // do later, but pretty much just idle
        }

        run_cycles += cycles;

        for (uint8_t i = 0; i < cycles; i++) {
            auto increment_mod_eight = [](uint8_t* foo) -> void { *foo = (*foo + 1) % 8; };
            increment_mod_eight(&x);

            if (1 <= curr_cycle <= 256) {
                bg_regs.pt_shift_lsb <<= 1;
                bg_regs.pt_shift_msb <<= 1;
                // render pixel
            }
            if (where_did_i_stop == LOAD_SHIFT_REGS) {
                load_bg_shift_regs();
                where_did_i_stop = static_cast<PPU_State>(static_cast<int>(where_did_i_stop) + 1);
            }
            if (run_cycles > 1) {
                switch (where_did_i_stop) {
                case FETCH_NT:
                    bg_regs.nt_latch = bus->ppu_read(0x2000 | (v & 0x0FFF));
                    break;
                case FETCH_AT:
                    bg_regs.at_latch = bus->ppu_read(0010001111000000 | (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07));
                    break;
                case FETCH_PT_LSB:
                    bg_regs.pt_latch_lsb = fetch_pt_byte(lsb);
                    break;
                case FETCH_PT_MSB:
                    bg_regs.pt_latch_msb = fetch_pt_byte(msb);
                    increment_coarse_x();
                    break;
                }

                where_did_i_stop = static_cast<PPU_State>(static_cast<int>(where_did_i_stop) + 1);
                run_cycles -= 2;
            }
            if (run_cycles == 256)
                increment_y();

            if (odd_even_frame == 1 && curr_scanline == -1 && curr_cycle == 339)
                increment_counters(1);

            increment_counters(1);
        }
    }

    void PPU::vblank_scanline(uint8_t cycles) {
        if (curr_scanline == 241 && curr_cycle == 1) {
            nmi_occurred = 1;
            ext_regs.ppustatus |= 0b10000000;
            nmi_occurred = 0;
            ext_regs.ppuctrl |= 0b10000000;
            // bus->cpu->nmi();
        }
        if (curr_scanline == 260)
            curr_scanline = -1;
    }


    /* helper functions*/

    void PPU::increment_counters(uint8_t cycles) {
        curr_cycle += cycles;

        if (curr_cycle >= 341)
            curr_scanline++;
        if (curr_scanline == 261)
            curr_scanline = -1;

        curr_cycle %= 340;
    }

    void PPU::increment_coarse_x() { // pseudocode taken from the nesDEV wiki
        if (v & 0b00011111 == 31) {
            v &= 0b0111111111100000; // setting coarse x to 0
            v ^= 0b0000010000000000; // moving to the next nametable
        }
        else
            v += 1;
    }

    void PPU::increment_y() { // pseudocode taken from the nesDEV wiki
        if (v & 0b011100000000000 != 0b011100000000000)
            v += 0b0000100000000000;
        else {
            v &= ~0b011100000000000; // setting fine scroll y to 0
            uint8_t y = (v & 0b0000001111100000) >> 5;// extracting coarse y
            if (y == 29) {
                y = 0;
                v ^= 0b0000100000000000; // switching vertical nametable
            }
            else if (y == 31)
                y = 0; // got to the end of the screen
            else {
                y += 1; // increment coarse y
                v = (v & ~0b0000001111100000) | (y << 5); // putting coarse y back in v

            }
        }
    }

    uint16_t PPU::fetch_pt_byte(uint8_t byte_significance) {
        uint16_t pt_byte = bg_regs.nt_latch;
        pt_byte <<= 4;
        pt_byte |= v >> 12;
        pt_byte |= byte_significance; // fetching msb/lsb
        pt_byte |= (ext_regs.ppuctrl & 0b00010000) << 8;

        return pt_byte;
    }

    void PPU::load_bg_shift_regs() {
        uint8_t attr_loading_shift;

        bg_regs.pt_shift_lsb = (bg_regs.pt_shift_lsb & 0xFF00) | bg_regs.pt_latch_lsb;
        bg_regs.pt_shift_msb = (bg_regs.pt_shift_msb & 0xFF00) | bg_regs.pt_latch_msb;

        auto constexpr get_coarse_x = [](uint16_t v) -> uint8_t { return v & 0b00011111; };
        auto constexpr get_coarse_y = [](uint16_t v) -> uint8_t { return (v & 0b11111000000) >> 5; };

        // 0, 0 -> 2|, 3
        // 0, 1 -> 4|, 5
        // 1, 0 -> 0|, 1
        // 1, 1 -> 6|, 7
        auto get_first_bit = [](uint8_t coarse_x, uint8_t coarse_y) -> uint8_t { // will rewrite this lambda later
            if (coarse_x % 2 == 0) {
                if (coarse_y % 2 == 0)
                    return 2; // fetch bits for top right
                else
                    return 4; // fetch bits for bottom right
            }
            else {
                if (coarse_y % 2 == 0)
                    return 0; // fetch bits for top left
                else
                    return 6; // fetch bits for bottom left
            }
        };

        attr_loading_shift = get_first_bit(get_coarse_x(v), get_coarse_y(v));

        bg_regs.attr_shift_msb = (bg_regs.at_latch >> attr_loading_shift) && 0b00000001;
        bg_regs.attr_shift_lsb = (bg_regs.at_latch >> (attr_loading_shift +1)) && 0b00000001;

        // if reg = 0b01 -> 0b11111111, if reg = 0b00 -> 0b00000000
        bg_regs.attr_shift_lsb *= 0b11111111;
        bg_regs.attr_shift_msb *= 0b11111111;

    }
}