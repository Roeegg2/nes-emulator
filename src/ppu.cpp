#include "../include/ppu.h"

namespace roee_nes {

    PPU::PPU(Bus* bus, NES_Screen* screen)
        : w(0), curr_scanline(261), curr_cycle(0), nmi(0), frame_oddness(0) // NOTE: not sure about these yet!
    {
        this->bus = bus;
        this->screen = screen;
    }

    void PPU::run_ppu(uint8_t cycles) {
        for (uint8_t i = 0; i < cycles; i++) {
            if (curr_scanline == PRE_RENDER_SCANLINE)
                prerender_scanline();
            else if ((RENDER_START_SCANLINE <= curr_scanline) && (curr_scanline <= RENDER_END_SCANLINE))
                visible_scanline();
            else if ((VBLANK_START_SCANLINE <= curr_scanline) && (curr_scanline <= VBLANK_END_SCANLINE))
                vblank_scanline();
            // otherwise its a post render scanline - ppu doesnt do anything

            increment_cycle(1);
            // log();
        }
    }

    void PPU::shared_visible_prerender_scanline() {
        if ((curr_cycle - 1) % 8 == 0) // on cycles 9, 17, 25, etc load the data in the latches into shift registers
            load_shift_regs();

        if ((curr_cycle % 2) == 1) { // if we are on an odd frame, that means we need to fetch something
            if (((1 <= curr_cycle) && (curr_cycle <= 256)) || ((321 <= curr_cycle) && (curr_cycle <= 336)))
                fetch_rendering_data(REGULAR_FETCH);
            else if ((337 == curr_cycle) || (curr_cycle == 339)) // between 337 and 340 we only fetch nt
                fetch_rendering_data(ONLY_NT_FETCH); // fetch nt0, nt1
        }

        if ((1 <= curr_cycle && curr_cycle <= 256) || (321 <= curr_cycle && curr_cycle <= 336)) {
            if (Get_rendering_status())
                shift_shift_regs(); // every cycle we shift the shift registers
            if (Get_rendering_status() && ((curr_cycle % 8) == 0))
                increment_coarse_x();
        }

        if (Get_rendering_status() && (curr_cycle == 256))
            increment_y();

        if (Get_rendering_status() && (curr_cycle == 257)) {// copy all horizontal bits from t onto v
            v.scroll_view.coarse_x = t.scroll_view.coarse_x;
            v.scroll_view.nt = (v.scroll_view.nt & 0b10) | (t.scroll_view.nt & 0b01);
        }

        if ((257 <= curr_cycle) && (curr_cycle <= 320)) // setting oamaddr to 0 during each of these ticks
            ext_regs.oamaddr = 0;
    }

    void PPU::prerender_scanline() {
        if (curr_cycle == 339) {
            screen->update_screen();
            frame_oddness = 1 - frame_oddness;
            frame_counter++;
            screen->handle_events();

            if (Get_rendering_status() && (frame_oddness == EVEN_FRAME)) { // each ODD scanline we skip a frame. I check for EVEN scanline because i switch the scanline value before the 'if'
                increment_cycle(1);
                return;
            }
        }

        if (curr_cycle == 1) {
            ext_regs.ppustatus &= 0b0001'1111; // clearing vblank, sprite 0 hit, and sprite overflow flag
            ext_regs.ppustatus |= sprite_0_next;
        }

        if (Get_rendering_status() && (280 <= curr_cycle) && (curr_cycle <= 304)) {
            v.scroll_view.coarse_y = t.scroll_view.coarse_y;
            v.scroll_view.nt = (v.scroll_view.nt & 0b01) | (t.scroll_view.nt & 0b10);
            v.scroll_view.fine_y = t.scroll_view.fine_y;
        }

        shared_visible_prerender_scanline();
    }

#ifdef DEBUG
    void PPU::print_oam() {
        static std::ofstream a("logs/POAM.log");
        a << "printing primary oam\n";
        int i = 0;
        for (auto it = primary_oam.cbegin(); it != primary_oam.cend(); it++) {
            a << std::hex << (int)*it << " ";
            if ((i % 15) == 0)
                a << "\n";
            i++;
        }
        a << "\n";
        i = 0;
        a << "printing secondary oam\n";
        for (auto it = secondary_oam.cbegin(); it != secondary_oam.cend(); it++) {
            a << std::hex << (int)*it << " ";
            if ((i % 15) == 0)
                a << "\n";
            i++;
        }
        a << "\n";
    }
#endif

    void PPU::sprite_evaluation() {
    static uint8_t byte_0;
#ifdef DEBUG
        static std::ofstream selog("logs/SELOG.log");
        selog << "soam is " << std::dec << (int)soam_count.counter.n
            << "\n poam is " << std::dec << (int)poam_count.counter.n << "\n";
#endif
        if ((curr_cycle % 2) == 1) // if cycle is odd
            byte_0 = primary_oam[INDEX_OAM_AT(poam_count, 0)]; // read from primary oam
        else {
            if (soam_count.counter.n < 8) {
                secondary_oam[INDEX_OAM_AT(soam_count, 0)] = byte_0;
                uint8_t y_diff = curr_scanline - byte_0 - 1;
                if ((0 <= y_diff) && (y_diff <= 7)) {
                    secondary_oam[INDEX_OAM_AT(soam_count, 1)] = primary_oam[INDEX_OAM_AT(poam_count, 1)]; // write all of them to secondary oam
                    secondary_oam[INDEX_OAM_AT(soam_count, 2)] = primary_oam[INDEX_OAM_AT(poam_count, 2)]; // write all of them to secondary oam
                    secondary_oam[INDEX_OAM_AT(soam_count, 3)] = primary_oam[INDEX_OAM_AT(poam_count, 3)]; // write all of them to secondary oam                    
                }
                soam_count.counter.n++;
            }
            poam_count.counter.n++;

            if (soam_count.counter.n == 8) {
                sprite_rendering_stage = SPRITE_OVERFLOW;
            } // if more than 8 sprites have been found 
            else if (poam_count.counter.n == 0)
                sprite_rendering_stage = BROKEN_READ;
        }
    }

    void PPU::sprite_overflow_check() {
        uint8_t byte_0 = primary_oam[INDEX_OAM(poam_count)];
        if (byte_0 == (curr_scanline + 1)) { // if y is in range
            ext_regs.ppustatus |= 0b0010'0000; // setting sprite overflow flag
            for (poam_count.counter.m = 1; poam_count.counter.m != 0; soam_count.raw++, poam_count.raw++);
        } else {
            // poam_count foo;
            // foo.raw = poam_count.raw;
            // poam_count.counter.n += 1;
            // poam_count.counter.m += 1;
            // if (poam_count.raw < foo.raw) // that means an overflow occured
            //     sprite_rendering_stage = BROKEN_READ; 
        }

    }

    void PPU::visible_scanline() {
        if (curr_cycle == 0) { // NOTE i know its wrong doing that here
            sprite_rendering_stage = SPRITE_EVAL;
            for (auto it = secondary_oam.begin(); it != secondary_oam.end(); it++)
                *it = 0xff;
            return;
        }

        if ((1 <= curr_cycle) && (curr_cycle <= 256)) {
            if (65 <= curr_cycle) {
                if (curr_cycle == 65) {
                    poam_count.raw = 0;
                    soam_count.raw = 0;
                }

                switch (sprite_rendering_stage) {
                    case SPRITE_EVAL:
                        sprite_evaluation();
                        break;
                    case SPRITE_OVERFLOW:
                        sprite_overflow_check();
                        break;
                    case BROKEN_READ:
                        break;
                    default:
                        std::cerr << "UNKNOWN FG 1-256 PART!\n";
                        break;
                }
            }
            add_render_pixel();
        }

        if (curr_cycle == 256) {
            soam_count.raw = 0;
#ifdef DEBUG
            print_oam();
#endif
            for (auto it = fg_data_render_line.begin(); it != fg_data_render_line.end(); it++) {
                *it = 0;
            }
            fill_fg_render_line();
            merge_bg_fg_render_line();
            screen->draw_pixel_line(&data_render_line, curr_scanline);
        }

        shared_visible_prerender_scanline();
    }

    void PPU::vblank_scanline() {
        if ((curr_scanline == VBLANK_START_SCANLINE) && (curr_cycle == 1)) {
            ext_regs.ppustatus |= 0b1000'0000; // set vblank flag
            if (ext_regs.ppuctrl & 0b1000'0000)
                nmi = 1;
        }
    }

    uint8_t PPU::fetch_fg_pt_byte(uint16_t priority, uint8_t y_diff, uint16_t tile, uint8_t at_byte_2) {
        uint16_t addr = y_diff & 0b0000'0000'0000'0111; // masking just in case
        addr |= priority;  // setting msb/lsb

        if (ext_regs.ppuctrl & 0b0010'0000) { // if this is a 8x16 sprite
            addr |= tile & 0b0000'0001 ? 0b0001'0000'0000'0000 : 0b0000'0000'0000'0000;
            tile >>= 1; // NOTE not sure about this shift, should i cancel bit 0?
        } else {
            addr |= ext_regs.ppuctrl & 0b0000'1000 ? 0b0001'0000'0000'0000 : 0b0000'0000'0000'0000;
        }

        // addr |= get_pt_side(ext_regs.ppuctrl); // setting the pattern table side
        addr |= (tile << 4); // setting the tile to select from
        addr &= ~0b1100'0000'0000'0000; // setting the rest of the bits to 0

        return bus->ppu_read(addr);
    }

    void PPU::fill_fg_render_line() {
        int cnt = 7;
        while (cnt > -1) { // going over the sprites in reversed order, to have lower indexed sprites override the higher indexed ones
            uint8_t y_byte_0 = secondary_oam[(4 * cnt) + 0];
            uint8_t y_diff = curr_scanline - y_byte_0 - 1;

            auto get_color_bit = [](uint8_t shift_reg_lsb, uint8_t shift_reg_msb, uint8_t x) -> uint8_t {
                uint8_t data = 0;
                data |= (0b0000'0001 & (shift_reg_lsb >> (7 - x)));
                data |= (0b0000'0010 & (shift_reg_msb >> (6 - x)));

                return data;
                };

            if ((0 <= y_diff) && (y_diff <= 7)) { // if the sprite is in current scanline range
                uint8_t tile_byte_1 = secondary_oam[(4 * cnt) + 1];
                uint8_t at_byte_2 = secondary_oam[(4 * cnt) + 2];
                uint8_t x_byte_3 = secondary_oam[(4 * cnt) + 3];

                uint8_t pt_low = fetch_fg_pt_byte(PT_LSB, y_diff, tile_byte_1, at_byte_2);
                uint8_t pt_high = fetch_fg_pt_byte(PT_MSB, y_diff, tile_byte_1, at_byte_2);

                for (int i = 0; i < 8; i++) {
                    uint8_t pt_data = get_color_bit(pt_low, pt_high, i);
                    uint8_t palette_index = bus->ppu_read(0x3f10 + (4 * (at_byte_2 & 0b0000'0011)) + pt_data);
                    fg_data_render_line[x_byte_3 + i].r = bus->color_palette[(palette_index * 3) + 0];
                    fg_data_render_line[x_byte_3 + i].g = bus->color_palette[(palette_index * 3) + 1];
                    fg_data_render_line[x_byte_3 + i].b = bus->color_palette[(palette_index * 3) + 2];

                    fg_data_render_line[x_byte_3 + i].pt_data = pt_data;
                    fg_data_render_line[x_byte_3 + i].priority = ((at_byte_2 & 0b0001'0000) >> 4);
                    
                    if (cnt == 0)
                        fg_data_render_line[x_byte_3 + i].im_sprite_0 = 1;
                }
            }
            cnt--;
        }
    }

    void PPU::merge_bg_fg_render_line() {
        for (int i = 0; i < 256; i++) {
            if ((fg_data_render_line[i].im_sprite_0 == 1) &&
                (fg_data_render_line[i].pt_data != 0) &&
                (data_render_line[i].pt_data != 0) &&
                ((ext_regs.ppumask & 0b0001'1000) == 0b0001'1000) &&
                (i != 255)) {
                // check left side clipping as well!!
                sprite_0_next = 0b0100'0000;
            }
            if ((ext_regs.ppumask & 0b0001'0000) && ((fg_data_render_line[i].priority == 0) || (data_render_line[i].pt_data == 0))) {
                data_render_line[i].r = fg_data_render_line[i].r;
                data_render_line[i].g = fg_data_render_line[i].g;
                data_render_line[i].b = fg_data_render_line[i].b;
            }
        }
    }

    void PPU::fetch_rendering_data(Fetch_Modes fetch_mode) {
        if (fetch_mode == ONLY_NT_FETCH) { // only fetching nametable byte
            bg_regs.nt_latch = bus->ppu_read(0x2000 | (v.raw & 0x0FFF));
            return;
        }

        switch (curr_cycle % 8) { // getting 1, 3, 5, or 7 representing what do we need to fetch
            case FETCH_1: // fetch nt
                bg_regs.nt_latch = bus->ppu_read(0x2000 | (v.raw & 0x0FFF));
                break;
            case FETCH_2: // its fetch at
                bg_regs.at_latch = bus->ppu_read(0x23C0 | (v.scroll_view.nt << 10) | ((v.scroll_view.coarse_y >> 2) << 3) | (v.scroll_view.coarse_x >> 2));

                if (v.scroll_view.coarse_y & 0x02)
                    bg_regs.at_latch >>= 4;
                if (v.scroll_view.coarse_x & 0x02)
                    bg_regs.at_latch >>= 2;

                bg_regs.at_latch &= 0b11; // removing the rest of the bits, which are unnessecary to get only bit 0, 1
                break;
            case FETCH_3: // fetch pt msb
                bg_regs.pt_latch_lsb = fetch_pt_byte(PT_LSB);
                break;
            case FETCH_4: // fetch pt lsb
                bg_regs.pt_latch_msb = fetch_pt_byte(PT_MSB);
                break;
        }
    }
    // pt: 3 at: 2 palette index: f color: 000000 frame: 34 scanline: 24 cycle: 92
    void PPU::add_render_pixel() {
        uint8_t palette_index = 0;

        auto get_color_bit = [](uint16_t shift_reg_lsb, uint16_t shift_reg_msb, uint8_t x) -> uint8_t {
            uint8_t data = 0;
            data |= (0b0000'0001 & (shift_reg_lsb >> (15 - x)));
            data |= (0b0000'0010 & (shift_reg_msb >> (14 - x)));

            return data;
            };

        uint8_t pt_data = get_color_bit(bg_regs.pt_shift_lsb, bg_regs.pt_shift_msb, x);
        uint8_t at_data = get_color_bit(bg_regs.at_shift_lsb, bg_regs.at_shift_msb, x);

        palette_index = bus->ppu_read((0x3f00 + (at_data * 4) + pt_data));

        data_render_line[curr_cycle - 1].r = bus->color_palette[(palette_index * 3) + 0];
        data_render_line[curr_cycle - 1].g = bus->color_palette[(palette_index * 3) + 1];
        data_render_line[curr_cycle - 1].b = bus->color_palette[(palette_index * 3) + 2];
    }

    uint8_t PPU::fetch_pt_byte(uint8_t byte_significance) {
        uint16_t fetch_addr = v.scroll_view.fine_y; // setting bits 0,1,2
        fetch_addr |= byte_significance; // setting bit 3
        fetch_addr |= bg_regs.nt_latch << 4; // setting bits 4,5,6,7,8,9,10,11,12
        fetch_addr |= (ext_regs.ppuctrl & 0b0001'0000) << 8; // setting bit 13
        fetch_addr &= 0b0011'1111'1111'1111; // bit 14 should always be set to 0

        return bus->ppu_read(fetch_addr);
        // return bus->ppu_read(((ext_regs.ppuctrl & 0b0001'0000) << 8) + ((uint16_t)bg_regs.nt_latch << 4) + v.scroll_view.fine_y + byte_significance);
    }

    void PPU::load_shift_regs() {
        // fill pattern table shift regs
        bg_regs.pt_shift_lsb = (bg_regs.pt_shift_lsb & 0xFF00) | bg_regs.pt_latch_lsb;
        bg_regs.pt_shift_msb = (bg_regs.pt_shift_msb & 0xFF00) | bg_regs.pt_latch_msb;

        // deciding which 2 bits i need, depending on which nametable we are currently at
        auto fill_shift_reg = [](uint8_t at_latch, uint16_t at_shift_reg, uint8_t significance) {
            if ((at_latch & significance) == significance)
                return (at_shift_reg & 0xff00) | 0x00ff;
            else
                return (at_shift_reg & 0xff00);
            };

        // filling the shift regs
        bg_regs.at_shift_lsb = fill_shift_reg(bg_regs.at_latch, bg_regs.at_shift_lsb, 0b01);
        bg_regs.at_shift_msb = fill_shift_reg(bg_regs.at_latch, bg_regs.at_shift_msb, 0b10);
    }

    void PPU::shift_shift_regs() {
        if (Get_rendering_status()) {
            bg_regs.pt_shift_lsb <<= 1;
            bg_regs.pt_shift_msb <<= 1;
            bg_regs.at_shift_lsb <<= 1;
            bg_regs.at_shift_msb <<= 1;
        }
    }

    /* function to increment the fine y and coarse y component of v */
    void PPU::increment_y() {
        if (v.scroll_view.fine_y < 7) // if fine y < 7 (normal incrementing)
            v.scroll_view.fine_y += 1;
        else {
            v.scroll_view.fine_y = 0;
            uint8_t coarse_y = v.scroll_view.coarse_y;
            if (coarse_y == 29) {
                coarse_y = 0;
                v.scroll_view.nt ^= 0b10; // switching vertical nametable
            } else if (coarse_y == 31)
                coarse_y = 0;
            else
                coarse_y += 1;

            v.scroll_view.coarse_y = coarse_y; // placing coarse y back onto v
        }
    }

    void PPU::increment_coarse_x() {
        if (v.scroll_view.coarse_x == 31) { // if coarse x = 31 (the limit)
            v.scroll_view.coarse_x = 0; // coarse x = 0
            v.scroll_view.nt ^= 0b01; // switching horizontal nametable
        } else
            v.scroll_view.coarse_x += 1; // otherwise, increment v normally
    }

    /* function to increment the curr_cycle internal counter (and curr_scanline accordingly) */
    void PPU::increment_cycle(uint8_t cycles) {
        curr_cycle += cycles;

        if (curr_cycle > 340) {
            curr_cycle = 0;
            curr_scanline++;
            curr_scanline %= 262; // wrapping around 261 -> 0
        }
    }

    void PPU::reset() { // change later!
        x = 0;
        w = 0;
        v.raw = 0;
        t.raw = 0;
        curr_cycle = 0;
        curr_scanline = 0;
        frame_oddness = 0;
        nmi = 0;
        bg_regs.at_latch = 0;
        bg_regs.at_shift_lsb = 0;
        bg_regs.at_shift_msb = 0;
        bg_regs.nt_latch = 0;
        bg_regs.pt_latch_lsb = 0;
        bg_regs.pt_latch_msb = 0;
        bg_regs.pt_shift_lsb = 0;
        bg_regs.pt_shift_msb = 0;
        ext_regs.ppuctrl = 0;
        ext_regs.ppumask = 0;
        ext_regs.ppustatus = 0;
        ext_regs.oamaddr = 0;
        frame_counter = 0;
    }

#ifdef DEBUG
    void PPU::log() const {
        // static uint16_t p_v, p_t, p_w, p_x, p_ppuctrl, p_ppumask, p_ppustatus;
        static std::ofstream roee_file("logs/ROEE_NES_PPU.log");

        // auto is_diff = [](uint16_t old_val, uint16_t new_val) -> bool {
        //     if (old_val != new_val)
        //         return true;

        //     return false;
        //     };

        // if (is_diff(p_t, t.raw) || is_diff(p_v, v.raw) || is_diff(p_w, w) || is_diff(p_x, x) || is_diff(p_ppuctrl, ext_regs.ppuctrl) || is_diff(p_ppumask, ext_regs.ppumask) || is_diff(p_ppustatus, ext_regs.ppustatus)) {
        roee_file << "\tt: " << std::hex << std::setw(4) << std::setfill('0') << std::uppercase << static_cast<int>(t.raw)
            << ", v: " << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(v.raw)
            << ", x: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(x)
            << ", w: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(w)
            << ", ctrl: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ext_regs.ppuctrl)
            << ", mask: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ext_regs.ppumask)
            << ", status: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ext_regs.ppustatus)
            << ", sl: " << std::dec << static_cast<int>(curr_scanline)
            << ", dot: " << std::dec << static_cast<int>(curr_cycle) << "\n";
        // }

        // p_v = v.raw;
        // p_t = t.raw;
        // p_w = w;
        // p_x = x;
        // p_ppuctrl = ext_regs.ppuctrl;
        // p_ppumask = ext_regs.ppumask;
        // p_ppustatus = ext_regs.ppustatus;
    }
#endif
}