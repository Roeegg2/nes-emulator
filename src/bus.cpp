#include "../include/bus.h"
#include <bitset>

namespace roee_nes {
    Bus::Bus(Mapper* mapper, Controller* controller1, Controller* controller2, const std::string* palette_path) {
        this->mapper = mapper;
        this->controller1 = controller1;
        this->controller2 = controller2;
        init_palette(palette_path);
    }

    void Bus::init_palette(const std::string* palette_path) {
        std::ifstream pal_file(*palette_path, std::ios::binary);

        if (!pal_file.is_open())
            std::cerr << "ERROR: Failed to open palette file" << "\n";

        for (int i = 0; i < 64 * 3; i += 3) {
            for (int j = 0; j < 3; j++) {
                pal_file.read((char*)&color_palette[i + j], 1);
            }
        }
    }

    void Bus::ppu_write(uint16_t addr, uint8_t data, bool came_from_cpu) {
        if (0x2000 <= addr && addr <= 0x3eff) {
            addr %= 0x1000; // removing nametable mirroring
            if (mapper->Get_mirroring() == 'H') {
                if (0 <= addr && addr <= 0x800)
                    nt_vram[0][addr % 0x400] = data;
                else
                    nt_vram[1][addr % 0x400] = data;
            } else if (mapper->Get_mirroring() == 'V') {
                if ((0 <= addr && addr < 0x400) || (0x800 <= addr && addr < 0x2c00))
                    nt_vram[0][addr % 0x400] = data;
                else
                    nt_vram[1][addr % 0x400] = data;
            }
        } else if (0x3f00 <= addr && addr <= 0x3fff) {
            addr %= 0x20; // the actual size of the palette is 0x20

            if (came_from_cpu) {
                if ((addr == 0x10) || (addr == 0x14) || (addr == 0x18) || (addr == 0x1c))
                    addr -= 0x10;
            }

            else if ((addr & 0b11) == 0)
                addr = 0;

            palette_vram[addr] = data;
        }
    }

    uint8_t Bus::ppu_read(uint16_t addr, bool came_from_cpu) {
        if (0 <= addr && addr <= 0x1fff)
            return mapper->ppu_read(addr); // pattern table
        else if (0x2000 <= addr && addr <= 0x3eff) {
            addr %= 0x1000;

            if (mapper->Get_mirroring() == 'H') {
                if (0 <= addr && addr <= 0x800)
                    return nt_vram[0][addr % 0x400];
                else
                    return nt_vram[1][addr % 0x400];
            } else if (mapper->Get_mirroring() == 'V') {
                if ((0 <= addr && addr < 0x400) || (0x800 <= addr && addr < 0x2c00))
                    return nt_vram[0][addr % 0x400];
                else
                    return nt_vram[1][addr % 0x400];
            }
        } else if (0x3f00 <= addr && addr <= 0x3fff) {
            addr %= 0x20; // the actual size of the palette is 0x20
            if (came_from_cpu) {
                if ((addr == 0x10) || (addr == 0x14) || (addr == 0x18) || (addr == 0x1c))
                    addr -= 0x10;
            }
            else if ((addr & 0b11) == 0)
                addr = 0;

            return palette_vram[addr];
        }

        return 0;
    }

    void Bus::cpu_write_ppu(uint16_t addr, uint8_t data) {
        switch (addr % 8) {
            case OAMADDR:
                ppu->ext_regs.oamaddr = data;
                break;
            case OAMDATA:
                if (((RENDER_START_SCANLINE <= ppu->curr_scanline) && (ppu->curr_scanline <= RENDER_END_SCANLINE)) || (ppu->curr_scanline == PRE_RENDER_SCANLINE))
                    return; // if we are not in vblank, we do nothing. TODO implement the weird increment of oamaddr here
                ppu->primary_oam[ppu->ext_regs.oamaddr] = data;
                ppu->ext_regs.oamaddr += 1;
                break;
            case PPUCTRL:
                // if (ppu-> <= 30000) return; // but not really important
                ppu->ext_regs.ppuctrl = data;
                ppu->t.scroll_view.nt = data & 0b0000'0011;
                break;
            case PPUMASK:
                ppu->ext_regs.ppumask = data;
                break;
            case PPUSCROLL:
                if (ppu->w == 0) {
                    ppu->t.scroll_view.coarse_x = data >> 3; // setting coarse x
                    ppu->x = data & 0b0000'0111; // setting fine x
                } else { //setting coarse y and fine y
                    ppu->t.scroll_view.fine_y = data & 0b0000'0111;
                    ppu->t.scroll_view.coarse_y = data >> 3;
                }

                ppu->w = 1 - ppu->w; // changing w from 1 to 0 and vise versa
                break;
            case PPUADDR:
                if (ppu->w == 0) { // setting upper 6 bits data
                    ppu->t.raw = (ppu->t.raw & 0x00ff) | ((data & 0x3F) << 8); // bit 14 is set to 0 in this case and the register is 15 bits wide, so bit 15 is not set
                } else { // setting lower byte data
                    ppu->t.raw = (ppu->t.raw & 0x7f00) | data;
                    ppu->v.raw = ppu->t.raw; // TODO: do this every 3 cycles to make more games compatible
                }

                ppu->w = 1 - ppu->w; // changing w from 1 to 0 and vise versa
                break;
            case PPUDATA: // TODO: implement $2007 reads and writes during rendering (incremnting both x and y)
                ppu_write(ppu->v.raw, data, true);
                ppu->v.raw += (ppu->ext_regs.ppuctrl & 0b00000100) ? 32 : 1;
                break;
        }
    }

    uint8_t Bus::cpu_read_ppu(uint16_t addr) {
        uint8_t ret = 0;
        switch (addr) {
            case OAMDATA:
                // TODO take care of reading OAMDATA during rendering
                ret = ppu->primary_oam[ppu->ext_regs.oamaddr];
                break;
            case PPUSTATUS:
                ppu->w = 0;
                ret = (ppu->ext_regs.ppustatus & 0b11100000) | (ppu_stupid_buffer & 0b00011111); // returning the last 5 bits of the latch and 3 top bits of the status register
                ppu->ext_regs.ppustatus &= 0b01111111; // clearing vblank flag
                break;
            case PPUDATA:
                ret = ppu_stupid_buffer;
                ppu_stupid_buffer = ppu_read(ppu->v.raw, true);
                if (addr >= 0x3f00)
                    ret = ppu_read(ppu->v.raw, true);
                ppu->v.raw += (ppu->ext_regs.ppuctrl & 0b00000100) ? 32 : 1;
                break;
        }

        return ret;
    }

    void Bus::cpu_write(uint16_t addr, uint8_t data) {
        if (0 <= addr && addr <= 0x1fff)
            ram[addr % 0x800] = data;
        else if (0x2000 <= addr && addr <= 0x3fff)
            cpu_write_ppu(addr % 8, data);
        else if (0x4000 <= addr && addr <= 0x4015) {
            if (addr == OAMDMA) {
                cpu_sleep_dma_counter = 513; // TODO takes more sometimes
                uint16_t start_addr = data;
                start_addr <<= 8;
                // start_addr |= (0x00ff & ppu->ext_regs.oamaddr);
                for (int i = 0; i < 256; i++) {
                    ppu->primary_oam[i] = ram[start_addr + i];
                }
            }
        } else if (addr == 0x4016)
            controller1->write(0b0000'0001 & data);
        else if (addr == 0x4017)
            controller2->write(0b0000'0001 & data);
        else if (0x4018 <= addr && addr <= 0x401f)
            return; // didnt implement yet
        else if (0x4020 <= addr && addr <= 0xffff)
            mapper->cpu_write(addr, data);
    }

    uint8_t Bus::cpu_read(uint16_t addr) {
        if (0 <= addr && addr <= 0x1fff)
            return ram[addr % 0x800];
        else if (0x2000 <= addr && addr <= 0x3fff)
            return cpu_read_ppu(addr % 8);
        else if (0x4000 <= addr && addr <= 0x4015)
            return 0; // didnt implement yet
        else if (addr == 0x4016)
            return controller1->read();
        else if (addr == 0x4017)
            return controller2->read();
        else if (0x4018 <= addr && addr <= 0x401f)
            return 0; // didnt implement yet
        else if (0x4020 <= addr && addr <= 0xffff)
            return mapper->cpu_read(addr);


        return 0;
    }

#ifdef DEBUG
    void Bus::full_log() const {
        static std::ofstream roee_file("ROEE_NES_MAIN.log");

        roee_file << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << static_cast<uint32_t>(cpu->log_PC) << " "
            << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cpu->IR) << " "
            << std::hex << std::setw(2) << std::setfill('0') << (cpu->log_bytes & 0x00ff) << " "
            << std::hex << std::setw(2) << std::setfill('0') << (cpu->log_bytes >> 8) << " "
            << cpu->inst->name;

        roee_file << "\t A:" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(cpu->log_A)
            << " X:" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cpu->log_X)
            << " Y:" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cpu->log_Y)
            << " P:" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cpu->log_P)
            << " SP:" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cpu->log_S) << std::dec;

        roee_file << "\t t:" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(ppu->t.raw)
            << " v:" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ppu->v.raw)
            << " x:" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ppu->x)
            << " w:" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ppu->w)
            << " ppuctrl:" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ppu->ext_regs.ppuctrl)
            << " ppumask:" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ppu->ext_regs.ppumask)
            << " ppustatus:" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ppu->ext_regs.ppustatus) << std::dec;

        roee_file << " PPU: " << ppu->curr_scanline << ", " << ppu->curr_cycle << ", CYC:" << ppu->curr_cycle / 3 << "\n";
    }

    void Bus::find_difference() const {
        std::ifstream roee_file("logs/ROEE_NES_PPU.log");
        std::ifstream nestest_file("logs/dk-binjnes.txt");

        if (!roee_file.is_open() || !nestest_file.is_open()) {
            std::cerr << "Error opening files." << "\n";
            return;
        }

        uint64_t line_cnt = 0;
        std::string roee_line, nestest_line;
        std::string roee_token, nestest_token;
        bool error_found = false;
        std::string error;
        while (std::getline(roee_file, roee_line) && std::getline(nestest_file, nestest_line)) {
            line_cnt++;

            std::istringstream roee_ss(roee_line);
            std::istringstream nestest_ss(nestest_line);

            roee_ss >> roee_token;
            nestest_ss >> nestest_token;

            roee_ss >> roee_token;
            nestest_ss >> nestest_token;

            if (roee_token != nestest_token && !error_found) {
                std::cout << "roee string:" << roee_token << "\n";
                std::cout << "nestest string:" << nestest_token << "\n";
                error = "t reg!";
                error_found = true;
            }

            roee_ss >> roee_token;
            nestest_ss >> nestest_token;

            roee_ss >> roee_token;
            nestest_ss >> nestest_token;

            if (roee_token != nestest_token && !error_found) {
                error = "v reg!";
                error_found = true;
            }

            roee_ss >> roee_token;
            nestest_ss >> nestest_token;

            roee_ss >> roee_token;
            nestest_ss >> nestest_token;

            if (roee_token != nestest_token && !error_found) {
                error = "x reg!";
                error_found = true;
            }

            roee_ss >> roee_token;
            nestest_ss >> nestest_token;

            roee_ss >> roee_token;
            nestest_ss >> nestest_token;

            if (roee_token != nestest_token && !error_found) {
                error = "w reg!";
                error_found = true;
            }

            roee_ss >> roee_token;
            nestest_ss >> nestest_token;

            roee_ss >> roee_token;
            nestest_ss >> nestest_token;

            if (roee_token != nestest_token && !error_found) {
                error = "ctrl reg!";
                error_found = true;
            }

            roee_ss >> roee_token;
            nestest_ss >> nestest_token;

            roee_ss >> roee_token;
            nestest_ss >> nestest_token;

            if (roee_token != nestest_token && !error_found) {
                error = "mask reg!";
                error_found = true;
            }

            roee_ss >> roee_token;
            nestest_ss >> nestest_token;

            roee_ss >> roee_token;
            nestest_ss >> nestest_token;

            if (roee_token != nestest_token && !error_found) {
                error = "status reg!";
                error_found = true;
            }

            roee_ss >> roee_token;
            nestest_ss >> nestest_token;

            roee_ss >> roee_token;
            nestest_ss >> nestest_token;

            if (roee_token != nestest_token && !error_found) {
                error = "scanline!";
                error_found = true;
            }

            roee_ss >> roee_token;
            nestest_ss >> nestest_token;

            roee_ss >> roee_token;
            nestest_ss >> nestest_token;

            if (roee_token != nestest_token && !error_found) {
                error = "cycle!";
                error_found = true;
            }

            if (error_found == true) {
                std::cerr << "Difference found in " << error << " Line: " << line_cnt << "\n";
                return;
            }
        }

        std::cout << "all goodie!" << "\n";
    }
#endif
    }
