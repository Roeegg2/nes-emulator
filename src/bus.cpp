#include "../include/bus.h"

namespace roee_nes {
    Bus::Bus(Mapper* mapper, const std::string* palette_path) {
        this->mapper = mapper;
        init_palette(palette_path);
    }

    void Bus::init_palette(const std::string* palette_path) {
        std::ifstream pal_file(*palette_path, std::ios::binary);

        if (!pal_file.is_open())
            std::cerr << "Failed to open palette file" << std::endl;

        for (int i = 0; i < 64; i++) {
            pal_file.read((char*)&palette[i].r, 1);
            pal_file.read((char*)&palette[i].g, 1);
            pal_file.read((char*)&palette[i].b, 1);
        }
    }

    void Bus::cpu_write(uint16_t addr, uint16_t data) {
        if (0 <= addr && addr <= 0x1fff)
            ram[addr % 0x800] = data;
        else if (0x2000 <= addr && addr <= 0x3fff) {
            cpu_write_ppu(addr % 8, data);
        } else if (0x4000 <= addr && addr <= 0x4017)
            return; // didnt implement yet
        else if (0x4018 <= addr && addr <= 0x401f)
            return; // didnt implement yet
        else if (0x4020 <= addr && addr <= 0xffff)
            mapper->cpu_write(addr, data);
    }

    uint8_t Bus::cpu_read(uint16_t addr) {
        if (0 <= addr && addr <= 0x1fff)
            return ram[addr % 0x800];
        else if (0x2000 <= addr && addr <= 0x3fff) {
            return cpu_read_ppu(addr % 8);
        } else if (0x4000 <= addr && addr <= 0x4017) {
            return 0; // didnt implement yet
        } else if (0x4018 <= addr && addr <= 0x401f) {
            return 0; // didnt implement yet
        } else if (0x4020 <= addr && addr <= 0xffff) {
            return mapper->cpu_read(addr);
        }

        return 0;
    }

    void Bus::ppu_write(uint16_t addr, uint8_t data) {
        uint8_t foo;

        if (0x2000 <= addr && addr <= 0x3eff) {
            uint16_t foo = addr;

            addr %= 0x1000;
            addr %= 0x400;

            if (mapper->Get_mirroring() == 'H') {
                if (0 <= foo && foo <= 0x800)
                    vram[0][addr] = data;
                else
                    vram[1][addr] = data;
            } else if (mapper->Get_mirroring() == 'V') {
                if (0 <= addr && addr <= 0x400 || 0x800 <= addr && addr <= 0x2c00)
                    vram[0][addr] = data;
                else
                    vram[1][addr] = data;
            }
        }
    }

    uint8_t Bus::ppu_read(uint16_t addr) {
        if (0 <= addr && addr <= 0x1fff)
            return mapper->ppu_read(addr); // pattern table

        else if (0x2000 <= addr && addr <= 0x3eff) {
            uint16_t foo = addr;

            addr %= 0x1000;
            addr %= 0x400;

            if (mapper->Get_mirroring() == 'H') {
                if (0 <= foo && foo <= 0x800)
                    return vram[0][addr];
                else
                    return vram[1][addr];
            } else if (mapper->Get_mirroring() == 'V') {
                if (0 <= addr && addr <= 0x400 || 0x800 <= addr && addr <= 0x2c00)
                    return vram[0][addr];
                else
                    return vram[1][addr];
            }
        }
        else
            std::cout << "got here!" << std::endl;

        return 0;
    }

    struct Color* Bus::ppu_get_color(uint16_t addr) {
        if (0x3f00 <= addr && addr <= 0x3fff) {
            addr %= 32;
            // TODO: later change to avoid the black screen in super mario
            if (addr == 0x10 || addr == 0x14 || addr == 0x18 || addr == 0x1c)
                addr -= 0x10;

            return &(palette[addr]);
        } else
            return nullptr;
    }

    void Bus::cpu_write_ppu(uint16_t addr, uint8_t data) {
        uint16_t data16 = data;

        switch (addr % 8) {
            case PPUCTRL:
                // if (total_frames <= 30000) return; // but not really important
                ppu->ext_regs.ppuctrl = data;
                ppu->t &= 0b0111001111111111;
                ppu->t |= ((0b00000011 & data) << 10);
                break;
            case PPUMASK:
                ppu->ext_regs.ppumask = data;
                break;
            case PPUSCROLL:
                if (ppu->w == 0) {
                    ppu->t = ((ppu->t >> 5) << 5) | (data16 >> 3); // setting coarse x
                    ppu->x = data16 & 0b0000000000000111; // setting fine x
                } else { //setting coarse y and fine y
                    ppu->t = (ppu->t & 0b0000110000011111); // note: i set another 0 at the start - because the register is 16 bits, and not 15 bits like it should be
                    ppu->t |= (data16 << 12);
                    ppu->t |= ((data16 >> 3) << 5);
                }

                ppu->w = 1 - ppu->w;
                break;
            case PPUADDR:
                if (ppu->w == 0) {
                    ppu->v &= 0b0011111111111111; // clearing bit 14 on first write.
                    data16 = data16 & 0b00111111;
                    ppu->t = (ppu->t & 0b0000000011111111) | (data16 << 8); // bit 14 is set to 0 in this case and the register is 15 bits wide, so bit 15 is not set
                } else {
                    ppu->t = (ppu->t & 0b0011111100000000) | data;
                    ppu->v = ppu->t; // TODO: do this every 3 cycles to make more games compatible
                }

                ppu->w = 1 - ppu->w; // changing w from 1 to 0 and vise versa
                break;
            case PPUDATA: // TODO: implement $2007 reads and writes during rendering (incremnting both x and y)
                ppu_write(ppu->v, data);
                ppu->v += (ppu->ext_regs.ppuctrl & 0b00000100) ? 32 : 1; // not sure about this
                break;
        }
    }

    uint8_t Bus::cpu_read_ppu(uint16_t addr) {
        uint8_t ret = 0;

        switch (addr) {
            case PPUSTATUS:
                ppu->w = 0;
                ret = (ppu->ext_regs.ppustatus & 0b11100000) | (ppu_stupid_buffer & 0b00011111); // returning the last 5 bits of the latch and 3 top bits of the status register
                ppu->ext_regs.ppustatus &= 0b01111111; // clearing vblank flag
                break;
            case PPUDATA:
                ret = ppu_stupid_buffer;
                ppu_stupid_buffer = ppu_read(ppu->v); // not sure about this
                if (addr >= 0x3f00)
                    ret = ppu_read(ppu->v);
                ppu->v += (ppu->ext_regs.ppuctrl & 0b00000100) ? 32 : 1; // MIGHT CAUSE PROBLEM
                break;
        }

        return ret;
    }

    void Bus::log() const {
        static std::ofstream roee_file("testr/logs/ROEE_NES_CPU.log");

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

        roee_file << "\t t:" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(ppu->t)
            << " v:" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ppu->v)
            << " x:" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ppu->x)
            << " w:" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ppu->w)
            << " ppuctrl:" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ppu->ext_regs.ppuctrl)
            << " ppumask:" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ppu->ext_regs.ppumask)
            << " ppustatus:" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ppu->ext_regs.ppustatus) << std::dec;

        roee_file << " PPU: " << ppu->curr_scanline << ", " << ppu->curr_cycle << ", CYC:" << ppu->curr_cycle / 3 << std::endl;
    }


    void Bus::find_difference() const {
        std::ifstream roee_file("testr/logs/ROEE_NES.log");
        std::ifstream nestest_file("testr/logs/dk_ppu_log.txt");

        if (!roee_file.is_open() || !nestest_file.is_open()) {
            std::cerr << "Error opening files." << std::endl;
            return;
        }

        int line_cnt = 0;
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
                std::cerr << "Difference found in " << error << " Line: " << line_cnt << std::endl;
                return;
            }
        }

        std::cout << "all goodie!" << std::endl;
    }
}
