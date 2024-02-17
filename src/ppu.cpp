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
            v.scroll.coarse_x = t.scroll.coarse_x;
            v.scroll.nt = (v.scroll.nt & 0b10) | (t.scroll.nt & 0b01);
        }
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
            ext_regs.ppustatus &= 0b0001'1111; // clearing sprite overflow, sprite 0 hit, vblank start
        }

        if (Get_rendering_status() && (280 <= curr_cycle) && (curr_cycle <= 304)) {
            v.scroll.coarse_y = t.scroll.coarse_y;
            v.scroll.nt = (v.scroll.nt & 0b01) | (t.scroll.nt & 0b10);
            v.scroll.fine_y = t.scroll.fine_y;
        }

        shared_visible_prerender_scanline();
    }

    void PPU::evaluate_sprite() {
        uint8_t y_byte = primary_oam[(4 * curr_sprite.counter.n) + 0];
        if (y_byte == curr_scanline + 1) {
            for (curr_sprite.counter.m = 1; curr_sprite.counter.m != 0; curr_sprite.raw++)
                secondary_oam[secondary_oam_counter] = primary_oam[(4 * curr_sprite.counter.n) + curr_sprite.counter.m];
            secondary_oam_counter += 1;
        } else
            curr_sprite.counter.n += 1; // on the other if, n gets incremented in the for loop

        if (curr_sprite.counter.n == 0) // if n overflowed to 0
            sprite_rendering_stage = STAGE_4;
        else if (secondary_oam_counter > 8) {
            sprite_rendering_stage = CHECK_OVERFLOW;
            curr_sprite.counter.m = 0; // starting from m = 0 for next stage
        }
        // else if (secondary_oam_counter == 8)
        //     disable rendering
    }

    void PPU::check_for_overflow() {
        uint8_t y_byte = primary_oam[(4 * curr_sprite.counter.n) + curr_sprite.counter.m];
        curr_sprite.counter.n += 1;
        if (y_byte == curr_scanline + 1) // if sprite is not in range
            ext_regs.ppustatus |= 0b0010'0000;
        else { // these 2 ifs are similar to check if the sprite counter overflowed
            if (curr_sprite.raw == 0)
                sprite_rendering_stage = STAGE_4;
            curr_sprite.counter.m += 1;
            if (curr_sprite.raw == 0)
                sprite_rendering_stage = STAGE_4;
        }
    }

    void PPU::visible_scanline() {
        if (curr_cycle == 0)
            return; // ppu idles during this cycle

        if ((1 <= curr_cycle) && (curr_cycle <= 256)) {
            switch (sprite_rendering_stage) {
                case SPRITE_EVAL:
                    evaluate_sprite();
                    break;
                case CHECK_OVERFLOW:
                    check_for_overflow();
                    break;
                case STAGE_4:
                    break;
            }

            add_render_pixel();
            if (curr_cycle == 256) {
                screen->draw_pixel_line(&data_render_line, curr_scanline);
            }
        } else if ((257 <= curr_cycle) && (curr_cycle <= 320)) {
            fetch_sprite_rendering_data();
        }

        shared_visible_prerender_scanline();
    }

    void PPU::fetch_sprite_rendering_data() {
        switch (sprite_rendering_stage) {
            case BYTE_0_FETCH:
                scanline_sprites[(4 * curr_sprite.counter.n) + curr_sprite.counter.m].y_byte_0
                    = secondary_oam[(4 * secondary_oam_counter.counter.n) + secondary_oam_counter.counter.m];
                break;
            case BYTE_1_FETCH:
                scanline_sprites[(4 * curr_sprite.counter.n) + curr_sprite.counter.m].pt_byte_1
                    = secondary_oam[(4 * secondary_oam_counter.counter.n) + secondary_oam_counter.counter.m];
                break;
            case BYTE_2_FETCH:
                scanline_sprites[(4 * curr_sprite.counter.n) + curr_sprite.counter.m].at_byte_2
                    = secondary_oam[(4 * secondary_oam_counter.counter.n) + secondary_oam_counter.counter.m];
                break;
            case BYTE_3_FETCH:
                scanline_sprites[(4 * curr_sprite.counter.n) + curr_sprite.counter.m].x_byte_4
                    = secondary_oam[(4 * secondary_oam_counter.counter.n) + secondary_oam_counter.counter.m];
                break;
            default:
                std::cerr << "NO FETCHING RIGHT!\n";
        }

    }

    void PPU::vblank_scanline() {
        if ((curr_scanline == VBLANK_START_SCANLINE) && (curr_cycle == 1)) {
            ext_regs.ppustatus |= 0b1000'0000; // set vblank flag
            if (ext_regs.ppuctrl & 0b1000'0000)
                nmi = 1;
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
                bg_regs.at_latch = bus->ppu_read(0x23C0 | (v.scroll.nt << 10) | ((v.scroll.coarse_y >> 2) << 3) | (v.scroll.coarse_x >> 2));

                if (v.scroll.coarse_y & 0x02)
                    bg_regs.at_latch >>= 4;
                if (v.scroll.coarse_x & 0x02)
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
        uint16_t fetch_addr = v.scroll.fine_y; // setting bits 0,1,2
        fetch_addr |= byte_significance; // setting bit 3
        fetch_addr |= bg_regs.nt_latch << 4; // setting bits 4,5,6,7,8,9,10,11,12
        fetch_addr |= (ext_regs.ppuctrl & 0b0001'0000) << 8; // setting bit 13
        fetch_addr &= 0b0011'1111'1111'1111; // bit 14 should always be set to 0

        return bus->ppu_read(fetch_addr);
        // return bus->ppu_read(((ext_regs.ppuctrl & 0b0001'0000) << 8) + ((uint16_t)bg_regs.nt_latch << 4) + v.scroll.fine_y + byte_significance);
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
        if (v.scroll.fine_y < 7) // if fine y < 7 (normal incrementing)
            v.scroll.fine_y += 1;
        else {
            v.scroll.fine_y = 0;
            uint8_t coarse_y = v.scroll.coarse_y;
            if (coarse_y == 29) {
                coarse_y = 0;
                v.scroll.nt ^= 0b10; // switching vertical nametable
            } else if (coarse_y == 31)
                coarse_y = 0;
            else
                coarse_y += 1;

            v.scroll.coarse_y = coarse_y; // placing coarse y back onto v
        }
    }

    void PPU::increment_coarse_x() {
        if (v.scroll.coarse_x == 31) { // if coarse x = 31 (the limit)
            v.scroll.coarse_x = 0; // coarse x = 0
            v.scroll.nt ^= 0b01; // switching horizontal nametable
        } else
            v.scroll.coarse_x += 1; // otherwise, increment v normally
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