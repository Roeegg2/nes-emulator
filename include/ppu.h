#ifndef PPU_H
#define PPU_H

#include <cstdint>
#include <iostream>

#include "bus.h"

class Bus;

enum PPU_STATE {
    FETCH_NT,
    FETCH_AT,
    FETCH_PT1,
    FETCH_PT2
};

struct Background_Regs {
    uint16_t pt_shift1;
    uint16_t pt_shift2;

    uint8_t attr_shift1;
    uint8_t attr_shift2;

    uint8_t nt_latch;
    uint8_t at_latch;
    uint8_t pt_latch1;
    uint8_t pt_latch2;
};

class PPU {
public:
    PPU(Bus* bus);

    void run_ppu(uint8_t cycles);

private:
    void prerender_and_visible_scanline(uint8_t cycles);
    void vblank_scanline(uint8_t cycles);

    void increment_counters(uint8_t cycles);

    void increment_x();
    void increment_y();

public:
    uint16_t v;
    uint16_t t;
    uint8_t x;
    uint8_t w;

    Background_Regs bg_regs;

    uint8_t ppustatus;
    uint8_t ppuctrl;

    uint8_t nmi_occurred;

    int32_t curr_scanline; // why does static cause an error here?
    int32_t curr_cycle;
    uint8_t odd_even_frame; // for pre-render scanline

public:
    Bus* bus;
};
#endif