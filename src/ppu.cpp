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

        uint8_t pixel;

        if (curr_cycle == 0) {
            // do later, but pretty much just idle
        }

        run_cycles += cycles;

        for (uint8_t i = 0; i < cycles; i++) {
            auto increment_mod_eight = [](uint8_t* foo) -> void { *foo = (*foo + 1) % 8; };
            increment_mod_eight(&x);

            if (1 <= curr_cycle <= 256) {
                // render pixel
            }
            if (where_did_i_stop == LOAD_SHIFT_REGS) {
                // load into shift registers
                where_did_i_stop = static_cast<PPU_State>(static_cast<int>(where_did_i_stop) + 1);
            }
            if (run_cycles > 1) {
                switch (where_did_i_stop) {
                case FETCH_NT:
                    bg_regs.nt_latch = bus->ppu_read(0x2000 | (v & 0x0FFF));
                    break;
                case FETCH_AT:
                    bg_regs.at_latch = bus->ppu_read(0x23C0 | (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07));
                    break;
                case FETCH_PT1:
                    bg_regs.pt_latch1 = fetch_pt_byte(msb);
                    break;
                case FETCH_PT2:
                    bg_regs.pt_latch1 = fetch_pt_byte(lsb);
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

        if (curr_cycle >= 340)
            curr_scanline++;
        if (curr_scanline == 261)
            curr_scanline = -1;

        curr_cycle %= 340;
    }

    void PPU::increment_coarse_x() { // pseudocode taken from the nesDEV wiki
        if (v & 0b00011111 == 31) {
            v &= 0b11100000; // setting coarse x to 0
            v ^= 0000010000000000; // moving to the next nametable
        }
        else
            v += 1;
    }

    void PPU::increment_y() { // pseudocode taken from the nesDEV wiki
        if (v & 0x7000 != 0x7000)
            v += 0x1000;
        else {
            v &= ~0x7000;
            uint8_t y = (v & 0x03E0) >> 5;
            if (y == 29) {
                y = 0;
                v ^= 0x0800;
            }
            else if (y == 31)
                y = 0;
            else {
                y += 1;
                v = (v & ~0x03E0) | (y << 5);
            }
        }
    }

    uint16_t PPU::fetch_pt_byte(uint8_t byte_significance) {
        uint16_t pt_byte = bg_regs.nt_latch;
        pt_byte <<= 5;
        pt_byte |= v >> 12;
        pt_byte |= (ext_regs.ppuctrl & 0b00010000) << 8;
        pt_byte |= 0b1000000000000000; // pt is only between 0x0000 and 0x1FFF
        pt_byte |= byte_significance; // fetching msb/lsb

        return pt_byte;
    }
}