#include "../include/ppu.h"

namespace roee_nes {
    PPU::PPU(Bus* bus, NES_Screen* screen)
        // write the initilizer list in order please
        : v({ 0 }), t({ 0 }), x(0), w(0), bg_regs({ 0 }), ext_regs({ 0 }), oamdma(0), curr_scanline(0), curr_cycle(0),
        nmi(0), frame_oddness(0), frame_counter(0), sprite_rendering_stage(SPRITE_EVAL),
        data_render_buffer({ 0 }), sprites({ 0 }), primary_oam({ 0 }), secondary_oam({ 0 }), next_sprite_0(nullptr),
        curr_sprite_0(nullptr), pri_oam_cnt(0), sec_oam_cnt(0), y_diff(0)
        // NOTE: not sure about these yet!
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

            increment_cycle(1);
        }
    }

    void PPU::shared_visible_prerender_scanline() {
        if ((curr_cycle - 1) % 8 == 0)
            load_shift_regs(); // on cycles 9, 17, 25, etc load the data in the latches into shift registers

        if ((curr_cycle % 2) == 1) { // if we are on an odd frame, that means we need to fetch something
            if (((1 <= curr_cycle) && (curr_cycle <= 256)) || ((321 <= curr_cycle) && (curr_cycle <= 336)))
                fetch_rendering_data(REGULAR_FETCH);
            else if ((337 == curr_cycle) || (curr_cycle == 339)) // between 337 and 340 we only fetch nt
                fetch_rendering_data(ONLY_NT_FETCH); // fetch nt0, nt1
        }

        if (((1 <= curr_cycle) && (curr_cycle <= 256)) || ((321 <= curr_cycle) && (curr_cycle <= 336))) {
            if (Get_rendering_status())
                shift_regs(); // every cycle we shift the shift registers
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

        if (curr_cycle == 1)
            ext_regs.ppustatus &= 0b0001'1111; // clearing vblank, sprite 0 hit, and sprite overflow flag


        if (Get_rendering_status() && (280 <= curr_cycle) && (curr_cycle <= 304)) {
            v.scroll_view.coarse_y = t.scroll_view.coarse_y;
            v.scroll_view.nt = (v.scroll_view.nt & 0b01) | (t.scroll_view.nt & 0b10);
            v.scroll_view.fine_y = t.scroll_view.fine_y;
        }

        shared_visible_prerender_scanline();
    }

    void PPU::visible_scanline() {
        if (curr_cycle == 0) {
            curr_sprite_0 = next_sprite_0;
            next_sprite_0 = nullptr;
            return; // idle cycle; do nothing
        }

        if ((1 <= curr_cycle) && (curr_cycle <= 256)) {
            add_render_pixel(); // get bg pixel color; get sprite pixel color; decide which pixel to add to render line

            if (65 <= curr_cycle) { // sprite evaluation 
                if (curr_cycle == 65) {
                    sprite_rendering_stage = SPRITE_EVAL;
                    sec_oam_cnt = 0;
                    pri_oam_cnt = 0;
                    for (auto it = secondary_oam.begin(); it != secondary_oam.end(); it++)
                        *it = 0xff;
                    // print_palette();
                    // sprite_evaluation();
                }

                if ((ext_regs.ppumask & 0b0001'1000)) { // if either sprites or background rendering is enabled
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
            }
        } else if (curr_cycle == 257)
            x_to_sprite_map.clear();

        if ((257 <= curr_cycle) && (curr_cycle <= 320))
            fill_sprites_render_data();
        else if (curr_cycle == 321) {
            screen->draw_pixel_line(&data_render_buffer, curr_scanline);
#ifdef DEBUG
            print_oam();
#endif
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

    void PPU::print_palette() {
        static std::ofstream a("logs/PALETTE.log");
        for (int i = 0; i < 32; i++) {
            if (((i % 4) == 0))
                a << "\n";
            a << std::hex << (int)bus->ppu_read(0x3f00 + i) << " ";
        }
        a << "\nfinished \n";
    }

    void PPU::sprite_overflow_check() {
        static uint8_t m = 0;
        uint8_t y = primary_oam[(4 * pri_oam_cnt) + m];
        if ((0 <= (curr_scanline - y - 1)) && ((curr_scanline - y - 1) <= 7)) {
            ext_regs.ppustatus |= 0b0010'0000;
            sprite_rendering_stage = BROKEN_READ;
            pri_oam_cnt++;
        } else {
            m++;
            pri_oam_cnt++;
            if (m == 4) {
                m = 0;
                pri_oam_cnt++;
            }
        }
        if (pri_oam_cnt == 64)
            sprite_rendering_stage = BROKEN_READ;
    }

    void PPU::sprite_evaluation() {
        // int pri_oam_cnt = 0;
        // int sec_oam_cnt = 0;
        // while (sec_oam_cnt < 8) {
        //     if ((0 <= (curr_scanline - primary_oam[(pri_oam_cnt * 4) + 0] - 1)) && ((curr_scanline - primary_oam[(pri_oam_cnt * 4) + 0] - 1) <= 7)) {
        //         secondary_oam[(sec_oam_cnt * 4) + 0] = primary_oam[(pri_oam_cnt * 4) + 0];
        //         secondary_oam[(sec_oam_cnt * 4) + 1] = primary_oam[(pri_oam_cnt * 4) + 1];
        //         secondary_oam[(sec_oam_cnt * 4) + 2] = primary_oam[(pri_oam_cnt * 4) + 2];
        //         secondary_oam[(sec_oam_cnt * 4) + 3] = primary_oam[(pri_oam_cnt * 4) + 3];
        //         sec_oam_cnt++;
        //     }
        //     pri_oam_cnt++;
        //     if (pri_oam_cnt > 63)
        //         return;
        // }
        //  -----------------------------------------------------

        static uint8_t byte_0;
        if ((curr_cycle % 2) == 1) // if cycle is odd
            byte_0 = primary_oam[(4 * pri_oam_cnt) + 0]; // read from primary oam
        else {
            if (sec_oam_cnt < 8) {
                int32_t diff = curr_scanline - byte_0 - 1;
                if ((0 <= diff) && (diff <= 7)) {
                    secondary_oam[(4 * sec_oam_cnt) + 0] = byte_0;
                    secondary_oam[(4 * sec_oam_cnt) + 1] = primary_oam[(4 * pri_oam_cnt) + 1]; // write all of them to secondary oam
                    secondary_oam[(4 * sec_oam_cnt) + 2] = primary_oam[(4 * pri_oam_cnt) + 2]; // write all of them to secondary oam
                    secondary_oam[(4 * sec_oam_cnt) + 3] = primary_oam[(4 * pri_oam_cnt) + 3]; // write all of them to secondary oam                    
                    sec_oam_cnt++;
                }
            }
            pri_oam_cnt++;
        }
        if (sec_oam_cnt == 8) {
            sprite_rendering_stage = SPRITE_OVERFLOW;
        } // if more than 8 sprites have been found 
        else if (pri_oam_cnt == 64)
            sprite_rendering_stage = BROKEN_READ;
    }

    void PPU::fill_sprites_render_data() {
        static uint8_t n;
        if (curr_cycle == 257) {
            n = 0;
        };

        switch ((curr_cycle - 1) % 8) {
            case Y_BYTE_0: // case this is 0
                sprites[n].y = secondary_oam[(4 * n) + 0];
                y_diff = curr_scanline - sprites[n].y - 1;

                if ((n == 0))
                    next_sprite_0 = &sprites.data()[0];

                break;
            case TILE_BYTE_1: // case this is 1
                sprites[n].tile = secondary_oam[(4 * n) + 1];
                break;
            case AT_BYTE_2: // case this is 2
                sprites[n].at = secondary_oam[(4 * n) + 2];
                break;
            case X_BYTE_3: // case this is 3
                sprites[n].x = secondary_oam[(4 * n) + 3];
                break;
            case FILL_BUFFER: // case this is 4
                fill_sprite_pixels(n);
                n = (n + 1);
                break;
            default: // if its 5,6,7
                // the PPU should fetch here X byte again 4 times, but for emulation this is unessecary, so do nothing
                break;
        }
    }

    void PPU::fill_sprite_pixels(uint8_t n) {
        // static std::ofstream a("logs/ll.log");

        // if (ext_regs.ppustatus & 0b0001'0000)
        //     std::cerr << "fill sprite pixels rendering not enableded\n";

        uint8_t pt_low = fetch_fg_pt_byte(PT_LSB, sprites[n].tile);
        uint8_t pt_high = fetch_fg_pt_byte(PT_MSB, sprites[n].tile);

        for (int i = 0; i < 8; i++) { // you were in the middle of checking the shifting here
            uint8_t pt_data = (((pt_high >> (7 - i)) & 0b0000'0001) << 1) | ((pt_low >> (7 - i)) & 0b0000'0001);
            if (sprites[n].at & 0b0100'0000)
                sprites[n].palette_indices[7 - i] = ((4 * (sprites[n].at & 0b0000'0011)) + pt_data);
            else
                sprites[n].palette_indices[i] = ((4 * (sprites[n].at & 0b0000'0011)) + pt_data);


            // a << "palette index " << std::dec << (int)sprites[n].palette_indices[i] << "\n";

            x_to_sprite_map.emplace(sprites[n].x + i, &(sprites[n]));
        }
        // a << "\n";
    }

    uint8_t PPU::get_bg_palette_index() {
        auto get_color_bit = [](uint16_t shift_reg_lsb, uint16_t shift_reg_msb, uint8_t x) -> uint8_t {
            uint8_t data = 0;
            data |= (0b0000'0001 & (shift_reg_lsb >> (15 - x)));
            data |= (0b0000'0010 & (shift_reg_msb >> (14 - x)));

            return data;
            };

        uint8_t pt_data = get_color_bit(bg_regs.pt_shift_lsb, bg_regs.pt_shift_msb, x);
        uint8_t at_data = get_color_bit(bg_regs.at_shift_lsb, bg_regs.at_shift_msb, x);

        return (0x3f00 + (at_data * 4) + pt_data);
    }

#ifdef DEBUG
    void PPU::print_oam() {
        static std::ofstream a("logs/POAM.log");
        // a << "printing primary oam\n";
        int i = 0;
        // for (auto it = primary_oam.cbegin(); it != primary_oam.cend(); it++) {
        //     a << std::hex << (int)*it << " ";
        //     if ((i % 15) == 0)
        //         a << "\n";
        //     i++;
        // }
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

    uint8_t PPU::fetch_fg_pt_byte(uint16_t priority, uint16_t tile) {
        uint16_t addr = y_diff & 0b0000'0000'0000'0111; // bits 0,1,2 masking just in case
        addr |= priority; // bits 3 setting msb/lsb
        addr |= (tile << 4); // bits 4-11 setting the tile to select from

        if (ext_regs.ppuctrl & 0b0010'0000) { // bit 12 if this is a 8x16 sprite
            if (tile & 0b0000'0001)
                addr |= 0b0001'0000'0000'0000;
            else
                addr |= 0b0000'0000'0000'0000;
            // tile >>= 1; // NOTE not sure about this shift, should i cancel bit 0?
        } else {
            if (ext_regs.ppuctrl & 0b0000'1000)
                addr |= 0b0001'0000'0000'0000;
            else
                addr |= 0b0000'0000'0000'0000;
        }

        return bus->ppu_read(addr);
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
                bg_regs.pt_latch_lsb = fetch_bg_pt_byte(PT_LSB);
                break;
            case FETCH_4: // fetch pt lsb
                bg_regs.pt_latch_msb = fetch_bg_pt_byte(PT_MSB);
                break;
        }
    }

    void PPU::add_render_pixel() {
        /**
     * get bg palette index
     * get bg pixel
     * if (such fg pixel)
     *      get fg pixel
     * if (fg should go infront)
     *      place fg
     * else
     *      place bg
    */
        uint8_t bg_palette_index = get_bg_palette_index();

        struct Pixel fg_pixel;
        struct Pixel bg_pixel;

        bg_pixel.r = bus->color_palette[(bus->ppu_read(0x3f00 + bg_palette_index) * 3) + 0];
        bg_pixel.g = bus->color_palette[(bus->ppu_read(0x3f00 + bg_palette_index) * 3) + 1];
        bg_pixel.b = bus->color_palette[(bus->ppu_read(0x3f00 + bg_palette_index) * 3) + 2];

        using iterator = std::unordered_map<uint8_t, struct Sprite*>::iterator;
        iterator sprite = x_to_sprite_map.find(curr_cycle - 1);

        if (sprite != x_to_sprite_map.end()) {
            // fg_pixel.r = bus->color_palette[(bus->ppu_read(0x3f10 + sprite->second->palette_indices[curr_cycle - 1 - sprite->second->x]) * 3) + 0];
            // fg_pixel.g = bus->color_palette[(bus->ppu_read(0x3f10 + sprite->second->palette_indices[curr_cycle - 1 - sprite->second->x]) * 3) + 1];
            // fg_pixel.b = bus->color_palette[(bus->ppu_read(0x3f10 + sprite->second->palette_indices[curr_cycle - 1 - sprite->second->x]) * 3) + 2];

            fg_pixel.r = 0xff;
            fg_pixel.g = 0xff;
            fg_pixel.b = 0x00;

            if ((ext_regs.ppustatus & 0b0100'0000) == 0b0000'0000) {
                if ((curr_cycle - 1) != 255)
                    if (curr_sprite_0 == sprite->second) {
                        if (((sprite->second->palette_indices[curr_cycle - 1 - sprite->second->x] != 0))) {
                            if ((bg_palette_index != 0)) {
                                ext_regs.ppustatus |= 0b0100'0000;
                            }
                        }
                    }
            }

        }

        if ((sprite != x_to_sprite_map.end()) && // if sprite should appear before background
            (ext_regs.ppumask & 0b0001'0000) &&
            ((sprite->second->at & 0b0010'0000) == 0) &&
            (curr_scanline != 0) &&
            ((fg_pixel.r != 0) || (fg_pixel.g != 0) || (fg_pixel.b != 0))){
            // a << "palette index " << std::dec << (int)sprite->second->palette_indices[curr_cycle - 1 - sprite->second->x] << "\n";

            data_render_buffer[curr_cycle - 1].r = fg_pixel.r;
            data_render_buffer[curr_cycle - 1].g = fg_pixel.g;
            data_render_buffer[curr_cycle - 1].b = fg_pixel.b;
        } else { // otherwise, place bg_pixel
            data_render_buffer[curr_cycle - 1].r = bg_pixel.r;
            data_render_buffer[curr_cycle - 1].g = bg_pixel.g;
            data_render_buffer[curr_cycle - 1].b = bg_pixel.b;
        }


        // -----------------------------------------------------------

                // static std::ofstream a("logs/OUT.log");
                // uint8_t bg_palette_index = get_bg_palette_index();

                // using iterator = std::unordered_map<uint8_t, struct Sprite*>::iterator;
                // iterator sprite = x_to_sprite_map.find(curr_cycle - 1);

                // // if sprite is in range
                // if ((sprite != x_to_sprite_map.end()) &&
                //     // ((0 <= (curr_scanline - sprite->second->y - 1)) && ((curr_scanline - sprite->second->y - 1) <= 7)) &&
                //     (ext_regs.ppumask & 0b0001'0000) &&
                //     ((sprite->second->at & 0b0010'0000) == 0) &&
                //     (curr_scanline != 0) &&
                //     ((bus->color_palette[(sprite->second->palette_indices[curr_cycle - 1 - sprite->second->x] * 3) + 0] != 0) ||
                //         (bus->color_palette[(sprite->second->palette_indices[curr_cycle - 1 - sprite->second->x] * 3) + 1] != 0) ||
                //         (bus->color_palette[(sprite->second->palette_indices[curr_cycle - 1 - sprite->second->x] * 3) + 2] != 0))) {

                //     data_render_buffer[curr_cycle - 1].r = bus->color_palette[(sprite->second->palette_indices[curr_cycle - 1 - sprite->second->x] * 3) + 0];
                //     data_render_buffer[curr_cycle - 1].g = bus->color_palette[(sprite->second->palette_indices[curr_cycle - 1 - sprite->second->x] * 3) + 1];
                //     data_render_buffer[curr_cycle - 1].b = bus->color_palette[(sprite->second->palette_indices[curr_cycle - 1 - sprite->second->x] * 3) + 2];

                // } else {
                //     data_render_buffer[curr_cycle - 1].r = bus->color_palette[(bg_palette_index * 3) + 0];
                //     data_render_buffer[curr_cycle - 1].g = bus->color_palette[(bg_palette_index * 3) + 1];
                //     data_render_buffer[curr_cycle - 1].b = bus->color_palette[(bg_palette_index * 3) + 2];
                // }
    }

    uint8_t PPU::fetch_bg_pt_byte(uint8_t byte_significance) {
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

    void PPU::shift_regs() {
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
        v.raw = 0;
        t.raw = 0;
        x = 0;
        w = 0;
        curr_cycle = 0;
        curr_scanline = 0;
        frame_oddness = 0;
        nmi = 0;
        bg_regs = { 0 };
        ext_regs = { 0 };
        frame_counter = 0;
        oamdma = 0;
        sprite_rendering_stage = SPRITE_EVAL;
        sec_oam_cnt = 0;
        pri_oam_cnt = 0;
        y_diff = 0;
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