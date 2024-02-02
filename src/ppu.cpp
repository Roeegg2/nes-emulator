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
            if (Get_rendering_status())
                rendering_enabled_actions();

            if (curr_scanline == PRE_RENDER_SCANLINE)
                prerender_scanline();
            else if ((RENDER_START_SCANLINE <= curr_scanline) && (curr_scanline <= RENDER_END_SCANLINE))
                visible_scanline();
            else if ((VBLANK_START_SCANLINE <= curr_scanline) && (curr_scanline <= VBLANK_END_SCANLINE))
                vblank_scanline();
            // otherwise its a post render scanline - ppu doesnt do anything

            increment_cycle(1);
#ifdef DEBUG
            log();
#endif
        }
    }

    void PPU::handle_shift_regs() {
        shift_shift_regs(); // every cycle we shift the shift registers

        if ((curr_cycle-1) % 8 == 0) { // on cycles 9, 17, 25, etc load the data in the latches into shift registers
            bg_regs.pt_shift_lsb = (bg_regs.pt_shift_lsb & 0xFF00) | bg_regs.pt_latch_lsb;
            bg_regs.pt_shift_msb = (bg_regs.pt_shift_msb & 0xFF00) | bg_regs.pt_latch_msb;

            load_attr_shift_regs();
        }

        if ((curr_cycle % 2) == 1) { // if we are on an odd frame, that means we need to fetch something
            if (((1 <= curr_cycle) && (curr_cycle <= 256)) || ((321 <= curr_cycle) && (curr_cycle <= 336)))
                fetch_rendering_data(REGULAR_FETCH);
            else if ((257 <= curr_cycle) && (curr_cycle <= 320)) // For some reason this fetch causes a problem, and it doesnt really matter, sooo
                fetch_rendering_data(GARBAGE_NT_FETCH); // garbage nt0, nt1, pt low, pt high 
            else if ((337 <= curr_cycle) && (curr_cycle <= 340))
                fetch_rendering_data(ONLY_NT_FETCH); // fetch nt0, nt1
        }
    }

    void PPU::prerender_scanline() {
        handle_shift_regs();

        static uint64_t frame_counter = 0;
        if (curr_cycle == 0) {
            screen->update_screen();
            frame_counter++;
        }
        if (curr_cycle == 1)
            ext_regs.ppustatus &= 0b0111'1111; // clearing vblank flag
        if (Get_rendering_status() && (280 <= curr_cycle) && (curr_cycle <= 304)) {
            v.scroll_view.coarse_y = t.scroll_view.coarse_y;
            v.scroll_view.nt = t.scroll_view.nt;
            v.scroll_view.fine_y = t.scroll_view.fine_y;
        }
    }

    void PPU::visible_scanline() {
        if (curr_cycle == 0)
            return;

        handle_shift_regs();

        if ((1 <= curr_cycle) && (curr_cycle <= 256))
            add_render_pixel();

        if (curr_cycle == 256)
            screen->draw_pixel_line(&data_render_line, curr_scanline);
    }

    void PPU::vblank_scanline() {
        if ((curr_scanline == VBLANK_START_SCANLINE) && (curr_cycle == 1)) {
            ext_regs.ppustatus |= 0b1000'0000; // set vblank flag
            if (ext_regs.ppuctrl & 0b1000'0000)
                nmi = 1;
        }
    }

    void PPU::rendering_enabled_actions() {
        if ((curr_scanline == 0) && (curr_cycle == 0) && (frame_oddness == ODD_FRAME)) { // on odd frames we skip the last cycle of the pre-render scanline
            increment_cycle(1);
            frame_oddness = 1 - frame_oddness;
        }
        if (curr_cycle == 256) // increment y component of v
            increment_y();
        if (curr_cycle == 257) {// copy all horizontal bits from t onto v
            v.scroll_view.coarse_x = t.scroll_view.coarse_x;
            v.scroll_view.nt = t.scroll_view.nt;
        }

        if (((328 <= curr_cycle) || ((1 <= curr_cycle) && (curr_cycle <= 256))) && (curr_cycle % 8 == 0)) // increment the coarse x component of v
            increment_coarse_x();
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
            case FETCH_2: // if its a GARBAGE_NT_FETCH, fetch nt. otherwise its a regular fetch, fetch attr
                if (fetch_mode == GARBAGE_NT_FETCH)
                    bg_regs.nt_latch = bus->ppu_read(0x2000 | (v.raw & 0x0FFF));
                else
                    bg_regs.at_latch = bus->ppu_read(0x23C0 | (v.raw & 0x0C00) | ((v.raw >> 4) & 0x38) | ((v.raw >> 2) & 0x07));
                break;
            case FETCH_3: // fetch pt msb
                bg_regs.pt_latch_lsb = fetch_pt_byte(PT_LSB);
                break;
            case FETCH_4: // fetch pt lsb
                bg_regs.pt_latch_msb = fetch_pt_byte(PT_MSB);
                break;
            default:
                std::cerr << "error!!" << "\n";
                break; // return error
        }
    }

    void PPU::add_render_pixel() {
#ifdef DEBUG
        static std::ofstream sl_file("logs/SCANLINE.log");
        sl_file << std::dec << curr_scanline << "\n";
#endif
        auto get_color_bit = [](uint16_t shift_reg_lsb, uint16_t shift_reg_msb, uint8_t x) -> uint8_t {
            uint8_t data = 0;
            data |= (0b0000'0001 & (shift_reg_lsb >> (15 - x)));
            data |= (0b0000'0010 & (shift_reg_msb >> (14 - x)));

            return data;
            };

        uint8_t pt_data = get_color_bit(bg_regs.pt_shift_lsb, bg_regs.pt_shift_msb, x);
        uint8_t attr_data = get_color_bit(bg_regs.attr_shift_lsb, bg_regs.attr_shift_msb, x);

        uint8_t palette_index = bus->ppu_read((0x3f00 + (attr_data * 4) + pt_data));

        data_render_line[curr_cycle - 1].r = bus->color_palette[(palette_index * 3) + 0];
        data_render_line[curr_cycle - 1].g = bus->color_palette[(palette_index * 3) + 1];
        data_render_line[curr_cycle - 1].b = bus->color_palette[(palette_index * 3) + 2];
    }

    uint8_t PPU::fetch_pt_byte(uint8_t byte_significance) {
        uint16_t fetch_addr = v.scroll_view.fine_y; // setting bits 0,1,2
        fetch_addr |= byte_significance; // setting bit 3
        fetch_addr |= bg_regs.nt_latch << 4; // setting bits 4,5,6,7,8,9,10,11,12
        fetch_addr |= (ext_regs.ppuctrl & 0b00010000) << 8; // setting bit 13
        fetch_addr &= 0b0011'1111'1111'1111; // bit 14 should always be set to 0

        return bus->ppu_read(fetch_addr);
    }

    void PPU::load_attr_shift_regs() {
        if (v.scroll_view.coarse_y & 0x02)
            bg_regs.at_latch >>= 4;
        if (v.scroll_view.coarse_y & 0x02)
            bg_regs.at_latch >>= 2;

        bg_regs.at_latch &= 0x03;

        bg_regs.attr_shift_lsb = (bg_regs.attr_shift_lsb & 0xff00) | ((bg_regs.at_latch & 0b01) ? 0xFF : 0x00);
        bg_regs.attr_shift_msb = (bg_regs.attr_shift_msb & 0xff00) | ((bg_regs.at_latch & 0b10) ? 0xFF : 0x00);
    }

    void PPU::shift_shift_regs() {
        if (Get_rendering_status()) {
            bg_regs.pt_shift_lsb <<= 1;
            bg_regs.pt_shift_msb <<= 1;
            bg_regs.attr_shift_lsb <<= 1;
            bg_regs.attr_shift_msb <<= 1;
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
            curr_scanline++;
            if (curr_scanline == 261) // i mark pre render scanline as -1 instead of 261 for convinience.
                curr_scanline = -1;
        }

        curr_cycle %= 341; // wrapping over if its bigger than 340
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
        bg_regs.attr_shift_lsb = 0;
        bg_regs.attr_shift_msb = 0;
        bg_regs.nt_latch = 0;
        bg_regs.pt_latch_lsb = 0;
        bg_regs.pt_latch_msb = 0;
        bg_regs.pt_shift_lsb = 0;
        bg_regs.pt_shift_msb = 0;
        ext_regs.ppuctrl = 0;
        ext_regs.ppumask = 0;
        ext_regs.ppustatus = 0;
        ext_regs.oamaddr = 0;
    }

#ifdef DEBUG

    void PPU::log() const {
        static std::ofstream roee_file("logs/ROEE_NES_PPU.log");

        roee_file << "\t t: " << std::hex << std::setw(4) << std::setfill('0') << std::uppercase << static_cast<int>(t.raw)
            << ", v: " << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(v.raw)
            << ", x: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(x)
            << ", w: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(w)
            << ", ctrl: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ext_regs.ppuctrl)
            << ", mask: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ext_regs.ppumask)
            << ", status: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ext_regs.ppustatus)
            << ", sl: " << std::dec << static_cast<int>(curr_scanline)
            << ", dot: " << std::dec << static_cast<int>(curr_cycle) << std::endl;

    }
#endif
}