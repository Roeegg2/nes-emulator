#include "../include/ppu.h"

namespace roee_nes {

    PPU::PPU(Bus* bus, NES_Screen* screen)
        : curr_cycle(0), curr_scanline(261), frame_oddness(0), nmi(0), w(0) // NOTE: not sure about these yet!
    {
        this->bus = bus;
        this->screen = screen;
    }

    void PPU::run_ppu(uint8_t cycles) {
        static std::ofstream vlogfile("testr/logs/V_LOG_SCANLINE40.log");
       
        for (uint8_t i = 0; i < cycles; i++) {
            if (Get_rendering_status()) {
                if ((frame_oddness == ODD_FRAME) && (curr_scanline == 0) && (curr_cycle == 0)) // on odd frames we skip the last cycle of the pre-render scanline
                    increment_cycle(1);
                if (curr_cycle == 256) // increment y component of v
                    increment_y();
                if (curr_cycle == 257) { // copy all horizontal bits from t onto v
                    v = (v & 0xFBE0) | (t & 0x41F); // resetting the horizontal position bits in v
                    if (curr_scanline == 40)
                        vlogfile << std::dec << "on scanline: "<< curr_scanline << std::hex << " ppuctrl binary: " << get_binary(ext_regs.ppuctrl, 16) << " ppuctrl hex: " << (int)ext_regs.ppuctrl << std::endl;
                }

                if (Get_rendering_status() && (328 <= curr_cycle || 0 <= curr_cycle <= 256) && (curr_cycle % 8 == 0)) // increment the coarse x component of v
                    increment_coarse_x();
            }

            if (curr_scanline == PRE_RENDER_SCANLINE)
                prerender_scanline();
            else if ((RENDER_START_SCANLINE <= curr_scanline) && (curr_scanline <= RENDER_END_SCANLINE))
                visible_scanline();
            else if ((VBLANK_START_SCANLINE <= curr_scanline) && (curr_scanline <= VBLANK_END_SCANLINE))
                vblank_scanline();
            // otherwise its a post render scanline - ppu doesnt do anything
            
            increment_cycle(1);
            log();
        }
    }

    void PPU::log_nametable(uint64_t frame_number) const {
        static std::ofstream nt_out_file("testr/logs/ROEE_NAMETABLE.log");
        using nt_vram_it = std::array<uint8_t, 0x400>::iterator;

        nt_vram_it it;
        nt_out_file << "\n";
        nt_out_file << "----------- THIS IS OF FRAME NUMBER: " << frame_number << "-----------" << std::endl;
        for (it = bus->nt_vram[1].begin(); it != bus->nt_vram[1].end(); it++) { // for every entry in the nametable (for every tile)
            uint8_t scroll_x, scroll_y;
            if (std::distance(bus->nt_vram[1].begin(), it) % 31 == 0) { // getting the position in the nametable, when visulized as a table
                nt_out_file << "\n";
            }
            nt_out_file << " 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)*it;
        }
    }

    void PPU::log_palette_ram() const {
        static std::ofstream roee_file("testr/logs/ROEE_NES_PALETTES_RAM.log");

        roee_file << "-------- printing palette ram: --------" << std::endl;
        using pr_it = std::array<uint8_t, 32>::iterator;
        pr_it it;
        for (it = bus->palette_vram.begin(); it != bus->palette_vram.end(); it++)
            roee_file << std::hex << std::setw(4) << std::setfill('0') << std::uppercase << static_cast<int>(*it) << std::endl;
    }

    void PPU::log() const {
        static std::ofstream roee_file("testr/logs/ROEE_NES.log");

        roee_file << "\t t: " << std::hex << std::setw(4) << std::setfill('0') << std::uppercase << static_cast<int>(t)
            << ", v: " << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(v)
            << ", x: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(x)
            << ", w: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(w)
            << ", ctrl: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ext_regs.ppuctrl)
            << ", mask: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ext_regs.ppumask)
            << ", status: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ext_regs.ppustatus)
            << ", sl: " << std::dec << static_cast<int>(curr_scanline)
            << ", dot: " << std::dec << static_cast<int>(curr_cycle) << std::endl;

    }

    void PPU::prerender_scanline() {
        static uint64_t frame_counter = 0;
        if (curr_cycle == 0) {
            log_nametable(frame_counter);
            log_palette_ram();
            
            screen->update_screen();
            frame_counter++;
        }
        if (curr_cycle == 1) {
            ext_regs.ppustatus &= 0b01111111; // clearing vblank flag
        }
        if (Get_rendering_status() && (280 <= curr_cycle) && (curr_cycle <= 304)) {
            v &= 0b0000010000011111; // reset coarse y and fine y in v
            v = (v & 0x041F) | (t & 0x7BE0); // setting v to t's y values
        }
        if (((2 <= curr_cycle) && (curr_cycle <= 256)) || ((321 <= curr_cycle) && (curr_cycle <= 336)))
            fetch_rendering_data(REGULAR_FETCH);
    }

    void PPU::visible_scanline() {
        if (((1 <= curr_cycle) && (curr_cycle <= 256)) || ((321 <= curr_cycle) && (curr_cycle <= 336))) {    
            add_render_pixel();
            if (curr_cycle == 256)
                screen->draw_pixel_line(&data_render_line, curr_scanline);

            fetch_rendering_data(REGULAR_FETCH);
        } 
        else if ((257 <= curr_cycle) && (curr_cycle <= 320))
            fetch_rendering_data(GARBAGE_NT_FETCH); // garbage nt0, nt1, pt low, pt high 

        else if ((337 <= curr_cycle) && (curr_cycle <= 340))
            fetch_rendering_data(ONLY_NT_FETCH); // fetch nt0, nt1
        if ((curr_cycle - 1) % 8 == 0) { // on cycles 9, 17, 25, etc load latches into shift registers
            bg_regs.pt_shift_lsb = (bg_regs.pt_shift_lsb & 0xFF00) | bg_regs.pt_latch_lsb;
            bg_regs.pt_shift_msb = (bg_regs.pt_shift_msb & 0xFF00) | bg_regs.pt_latch_msb;

            load_attr_shift_regs();
        }
    }

    void PPU::vblank_scanline() {
        if ((curr_scanline == VBLANK_START_SCANLINE) && (curr_cycle == 1)) {
            ext_regs.ppustatus |= 0b10000000; // set vblank flag
            if (ext_regs.ppuctrl & 0b10000000)
                nmi = 1;
        }
    }

    void PPU::fetch_rendering_data(Fetch_Modes fetch_mode) {
        if ((curr_cycle % 2 == 1) || (curr_cycle == 0))
            return; // if cycles are not multiples of 2, we return, since memory fetching actually takes 2 ppu cycles. 

        uint8_t fetch_type = curr_cycle % 8; // getting 2, 4, 6, or 0 representing what do we need to fetch

        if (fetch_mode == ONLY_NT_FETCH) {
            bg_regs.nt_latch = bus->ppu_read(0x2000 | (v & 0x0FFF)); // fetch nt
            return;
        }

        switch (fetch_type) {
            case FETCH_1: // fetch nt
                bg_regs.nt_latch = bus->ppu_read(0x2000 | (v & 0x0FFF));
                break;
            case FETCH_2: // if its a GARBAGE_NT_FETCH, fetch nt. otherwise its a regular fetch, fetch attr
                if (fetch_mode == GARBAGE_NT_FETCH)
                    bg_regs.nt_latch = bus->ppu_read(0x2000 | (v & 0x0FFF));
                else
                    bg_regs.at_latch = bus->ppu_read(0x23C0 | (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07));
                break;
            case FETCH_3: // fetch pt msb
                bg_regs.pt_latch_lsb = fetch_pt_byte(PT_LSB);
                break;
            case FETCH_4: // fetch pt lsb
                bg_regs.pt_latch_msb = fetch_pt_byte(PT_MSB);
                break;
            default:
                std::cerr << "error!!" << std::endl;
                break; // return error
        }
    }

    void PPU::add_render_pixel() {
        static std::ofstream pattern_log("testr/logs/ROEE_NES_PALETTE_TABLE.log");
        static std::ofstream attribute_log("testr/logs/ROEE_NES_PALETTE_TABLE.log");
        static std::ofstream address_log("testr/logs/ADDRESS_LOG.log");
        
        auto get_color_bit = [](uint16_t shift_reg_lsb, uint16_t shift_reg_msb, uint8_t x) -> uint8_t {
            uint8_t data = 0;
            data |= (0b00000001 & (shift_reg_lsb >> (15 - x)));
            data |= (0b00000010 & (shift_reg_msb >> (14 - x)));

            return data;
        };

        uint8_t pt_data = get_color_bit(bg_regs.pt_shift_lsb, bg_regs.pt_shift_msb, x);
        pattern_log << std::hex << "pt value: " << (int)pt_data << std::endl;

        uint8_t attr_data = get_color_bit(bg_regs.attr_shift_lsb, bg_regs.attr_shift_msb, x);
        attribute_log << std::hex << "at value: " << (int)attr_data << std::endl;

        uint8_t palette_index = bus->ppu_read((0x3f00 + (attr_data * 4) + pt_data));

        address_log << std::dec << "scanline: " << curr_scanline << std::hex 
            << " nt data: " << std::setw(4) << std::setfill('0') << (int)pt_data            
            << " pt data: " << std::setw(4) << std::setfill('0') << (int)pt_data
            << " attr data: " << std::setw(4) << std::setfill('0') << (int)attr_data << std::endl;

        data_render_line[curr_cycle-1].r = bus->color_palette[(palette_index * 3) + 0];
        data_render_line[curr_cycle-1].g = bus->color_palette[(palette_index * 3) + 1];
        data_render_line[curr_cycle-1].b = bus->color_palette[(palette_index * 3) + 2];

        bg_regs.pt_shift_lsb <<= 1;
        bg_regs.pt_shift_msb <<= 1;
        bg_regs.attr_shift_lsb <<= 1;
        bg_regs.attr_shift_msb <<= 1;
    }

    /* function to increment the fine y and coarse y component of v */
    void PPU::increment_y() {
        if ((v & 0b0111000000000000) != 0b0111000000000000) // if fine y < 7 (normal incrementing)
            v += 0b0001000000000000;
        else {
            v &= ~0b0111000000000000;
            uint8_t coarse_y = (v & 0b0000001111100000) >> 5; // extracting coarse y out of v
            if (coarse_y == 29) {
                coarse_y = 0;
                v ^= 0b0001000000000000; // switching vertical nametable
            } else if (coarse_y == 31)
                coarse_y = 0;
            else
                coarse_y++;

            v = (v & ~0b0000001111100000) | (coarse_y << 5); // placing coarse y back onto v
        }
    }

    void PPU::increment_coarse_x() {
        if ((v & 0b0000000000011111) == 31) { // if coarse x = 31 (the limit)
            v &= ~0b0000000000011111; // coarse x = 0
            v ^= 0b0000010000000000;
        } else
            v++; // otherwise, increment v normally
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

    uint8_t PPU::fetch_pt_byte(uint8_t byte_significance) {
        uint16_t pt_byte_addr = bg_regs.nt_latch; // 0b00000000dddddddd
        pt_byte_addr <<= 4;                       // 0b0000dddddddd0000
        pt_byte_addr |= v >> 12;                  // 0b0000ddddddddvvvv
        pt_byte_addr |= byte_significance;        // fetching msb/lsb // 0b0000ddddddddsvvv
        pt_byte_addr |= (ext_regs.ppuctrl & 0b00010000) << 8;

        return bus->ppu_read(pt_byte_addr);
    }

    void PPU::load_attr_shift_regs() {
        static std::ofstream attr_file("testr/logs/ROEE_NES_ATTRIBUTE_BYTES.log");

        uint8_t coarse_x = v & 0b0000000000011111;
        uint8_t coarse_y = (v >> 5) & 0b0000000000011111;

        uint8_t at_byte;
        if ((coarse_y & 0x02) != 0) {
            bg_regs.at_latch >>= 4;
        }
        if ((coarse_x & 0x02) != 0) {
            bg_regs.at_latch >>= 2;
        }
        
        //  if sb_data = 0b1 we get 0b11111111, if sb_data = 0b0 we get 0b00000000
        bg_regs.attr_shift_lsb |= (bg_regs.at_latch & 0x1) * 0b11111111;
        bg_regs.attr_shift_msb |= (bg_regs.at_latch & 0x2) * 0b11111111;

        attr_file << "lsb: " << get_binary(bg_regs.attr_shift_lsb, 8) << " msb: " << get_binary(bg_regs.attr_shift_msb, 8) << std::endl;
    }

    void PPU::reset() { // change later!
        x = 0;
        w = 0;
        v = 0;
        t = 0;
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
}