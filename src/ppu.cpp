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
            v.scroll_view.coarse_y = t.scroll_view.coarse_y;
            v.scroll_view.nt = (v.scroll_view.nt & 0b01) | (t.scroll_view.nt & 0b10);
            v.scroll_view.fine_y = t.scroll_view.fine_y;
        }

        shared_visible_prerender_scanline();
    }

    void PPU::visible_scanline() {
        if (curr_cycle == 0)
            return; // ppu idles during this cycle

        if ((1 <= curr_cycle) && (curr_cycle <= 256)) {
            if (65 <= curr_cycle) {
                if (curr_cycle == 65) {
                    sprite_count.raw = 0;
                    ext_regs.ppustatus |= 0b0010'0000; // setting on the sprite evaluation flag
                }
                switch (current_eval_part) {
                    case PART_1_2:
                        fg_part_1_2();
                        if (sprite_count.counter.n == 0)
                            current_eval_part = PART_4;
                        else if (open_secondary_oam_slot > 8) // if more than 8 sprites have been found
                            current_eval_part = PART_3;
                        break;
                    case PART_3:
                        fg_part_3();
                        break;
                    case PART_4:
                        fg_part_4();
                        break;
                    // default:
                    //     std::cerr << "UNKNOWN FG 1-256 PART!\n";
                    //     break;
                }
            }

            add_render_pixel();
            if (curr_cycle == 256) {
                add_sprites_to_render_line(); // FG decide if you should display sprites or background
                screen->draw_pixel_line(&data_render_line, curr_scanline);
            }
        }

        else if ((257 <= curr_cycle) && (curr_cycle <= 320)) {
            static uint8_t y_byte_0, tile_byte_1, at_byte_2, x_byte_3;
            // set sprite_count to 0
            if (curr_cycle == 257)
                sprite_count.raw = 0;
            switch (curr_cycle % 8) {
                case 1:
                    y_byte_0 = read_secondary_oam((4 * sprite_count.counter.n) + sprite_count.counter.m);
                    sprite_count.raw += 1;
                    break;
                case 2:
                    tile_byte_1 = read_secondary_oam((4 * sprite_count.counter.n) + sprite_count.counter.m);
                    sprite_count.raw += 1;
                    break;
                case 3:
                    at_byte_2 = read_secondary_oam((4 * sprite_count.counter.n) + sprite_count.counter.m);
                    sprite_count.raw += 1;
                    break;
                case 4:
                    x_byte_3 = read_secondary_oam((4 * sprite_count.counter.n) + sprite_count.counter.m);
                    break;
                default: // for cases 5,6,7,0
                    x_byte_3 = read_secondary_oam((4 * sprite_count.counter.n) + sprite_count.counter.m);
                    if ((curr_cycle % 8) == 0) {
                        sprite_count.raw += 1;
                        // FG add_to_sprite_buffer();
                    }
                    // read x coordinate
                    break;
            }

            // NOTE have no idea what to do with this value
        }
        else if ((321 <= curr_cycle) && (curr_cycle <= 340)) {
            // read the first byte in secondary oam
            // FG set the sprite rendering for next line
        }

        shared_visible_prerender_scanline();
    }

    void PPU::fg_part_1_2() {
        static uint8_t y_byte_0;
        if ((curr_cycle % 2) == 1) // if cycle is odd
            y_byte_0 = read_primary_oam((4 * sprite_count.counter.n) + sprite_count.counter.m); // read from primary oam
        else if (open_secondary_oam_slot < 8) { // if cycle is even and there is still an open slot in secondary oam
            write_secondary_oam(open_secondary_oam_slot, y_byte_0); // write to secondary oam
            open_secondary_oam_slot += 1;

            if (y_byte_0 == (curr_scanline + 1)) // if the next scanline is in range
                for (sprite_count.counter.m = 1; sprite_count.counter.m != 0; open_secondary_oam_slot++, sprite_count.raw++) // go over the rest of the 3 bytes
                    write_secondary_oam(open_secondary_oam_slot, read_primary_oam((4 * sprite_count.counter.n) + sprite_count.counter.m)); // write all of them to secondary oam
            // NOTE: m should be 0 and n should be incremented by one by the end of this
        } else { // if cycle is even but there is no open slot in secondary oam
            y_byte_0 = read_secondary_oam((4 * sprite_count.counter.n) + sprite_count.counter.m); // i dont know what to do with this read lol
        }
    }

    void PPU::fg_part_3() {
        // NOTE start from m = 0
        uint8_t dumb_read;
        uint8_t y_byte_0 = read_primary_oam((4 * sprite_count.counter.n) + sprite_count.counter.m);
        if (y_byte_0 == (curr_scanline + 1)) { // if y is in range
            ext_regs.ppustatus |= 0b0010'0000; // setting sprite overflow flag
            for (sprite_count.counter.m = 1; sprite_count.counter.m != 0; open_secondary_oam_slot++, sprite_count.raw++) // go over the rest of the 3 bytes
                dumb_read = read_primary_oam((4 * sprite_count.counter.n) + sprite_count.counter.m); // dummy read them NOTE: not sure about this, maybe this should a read incrementing oamaddr?
        } else {
            sprite_counter foo;
            foo.raw = sprite_count.raw;
            sprite_count.counter.n += 1;
            sprite_count.counter.m += 1;
            if (sprite_count.raw < foo.raw) // that means an overflow occured
                current_eval_part = PART_4; // NOTE move that to the switch-case statement instead of this function
        }

    }

    void PPU::fg_part_4() {
        // Attempt (and fail) to copy OAM[n][0] into the next free slot in secondary OAM
        sprite_count.counter.n += 1;

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
    
    uint8_t PPU::read_primary_oam(uint8_t entry) {
        return primary_oam[entry];
    }

    uint8_t PPU::read_secondary_oam(uint8_t entry) {
        return secondary_oam[entry];
    }

    void PPU::write_primary_oam(uint8_t entry, uint8_t data) {
        primary_oam[entry] = data;
    }

    void PPU::write_secondary_oam(uint8_t entry, uint8_t data) {
        secondary_oam[entry] = data;
    }

    // void PPU::add_sprites_to_render_line() {
    //     std::array<struct >
    // }

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