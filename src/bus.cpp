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

        for (int i = 0; i < 64 * 3; i += 3) {
            for (int j = 0; j < 3; j++) {
                pal_file.read((char*)&color_palette[i+j], 1);
            }
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

            if (mapper->Get_mirroring() == 'H') {
                if (0 <= foo && foo <= 0x800)
                    nt_vram[0][addr % 0x400] = data;
                else
                    nt_vram[1][addr % 0x400] = data;
            } else if (mapper->Get_mirroring() == 'V') {
                if (0 <= addr && addr <= 0x400 || 0x800 <= addr && addr <= 0x2c00)
                    nt_vram[0][addr % 0x400] = data;
                else
                    nt_vram[1][addr % 0x400] = data;
            }
        }
        else if (0x3f00 <= addr && addr <= 0x3fff) {
            addr %= 0x20;
            if ((addr & 0b11) == 0)
                addr = 0;
            
            palette_vram[addr] = data;
        }
    }

    uint8_t Bus::ppu_read(uint16_t addr) {
        if (0 <= addr && addr <= 0x1fff)
            return mapper->ppu_read(addr); // pattern table
        else if (0x2000 <= addr && addr <= 0x3eff) {
            uint16_t foo = addr;

            addr %= 0x1000;

            if (mapper->Get_mirroring() == 'H') {
                if (0 <= foo && foo <= 0x800)
                    return nt_vram[0][addr % 0x400];
                else
                    return nt_vram[1][addr % 0x400];
            } else if (mapper->Get_mirroring() == 'V') {
                if (0 <= addr && addr <= 0x400 || 0x800 <= addr && addr <= 0x2c00)
                    return nt_vram[0][addr % 0x400];
                else
                    return nt_vram[1][addr % 0x400];
            }
        }
        else if (0x3f00 <= addr && addr <= 0x3fff) {
            addr %= 0x20;
            if ((addr & 0b11) == 0)
                addr = 0;
            
            return palette_vram[addr];
        }
        else
            std::cout << "got here!" << std::endl;

        return 0;
    }

    void Bus::cpu_write_ppu(uint16_t addr, uint8_t data) {
        uint16_t data16 = data;

        switch (addr % 8) {
            case PPUCTRL:
                // if (ppu-> <= 30000) return; // but not really important
                ppu->ext_regs.ppuctrl = data;
                ppu->t = (ppu->t & 0111001111111111) | ((0b00000011 & data) << 10);
                break;
            case PPUMASK:
                ppu->ext_regs.ppumask = data;
                break;
            case PPUSCROLL:
                if (ppu->w == 0) {
                    ppu->t = ((ppu->t & 0x7fe0) | (data >> 3)); // setting coarse x
                    ppu->x = data & 0b0000000000000111; // setting fine x
                    ppu->w = 1;
                } else { //setting coarse y and fine y
                    ppu->t = (ppu->t & 0x0C1F) | ((data & 0xF8) << 2) | ((data & 7) << 12);
                    ppu->w = 0;
                }

                break;
            case PPUADDR:
                if (ppu->w == 0) {
                    // ppu->v &= 0b0011111111111111; // clearing bit 14 on first write.
                    ppu->t = (ppu->t & 0x00ff) | ((data & 0x3F) << 8); // bit 14 is set to 0 in this case and the register is 15 bits wide, so bit 15 is not set
                    ppu->w = 1;
                } else {
                    ppu->t = (ppu->t & 0x7f00) | data;
                    ppu->v = ppu->t; // TODO: do this every 3 cycles to make more games compatible
                    ppu->w = 0;
                }

                // ppu->w = 1 - ppu->w; // changing w from 1 to 0 and vise versa
                break;
            case PPUDATA: // TODO: implement $2007 reads and writes during rendering (incremnting both x and y)
                ppu_write(ppu->v, data);
                ppu->v += (ppu->ext_regs.ppuctrl & 0b00000100) ? 32 : 1;
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
                ppu_stupid_buffer = ppu_read(ppu->v);
                if (addr >= 0x3f00)
                    ret = ppu_read(ppu->v);
                ppu->v += (ppu->ext_regs.ppuctrl & 0b00000100) ? 32 : 1;
                break;
        }

        return ret;
    }

    void Bus::log() const {
        static std::ofstream roee_file("logs/ROEE_NES_CPU.log");

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
        std::ifstream roee_file("logs/ROEE_NES.log");
        std::ifstream nestest_file("logs/dk_ppu_log.txt");

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
