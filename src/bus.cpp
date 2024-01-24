#include "../include/bus.h"

namespace roee_nes {
    Bus::Bus(Mapper* mapper, std::string palette_path) {
        this->mapper = mapper;
        init_palette(palette_path);
    }

    void Bus::init_palette(std::string palette_path) {
        static std::ifstream pal_file(palette_path, std::ios::binary);

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
        } /* else if (0x4000 <= addr && addr <= 0x4017)
            return; // apu related - didnt implement yet
        else if (0x4018 <= addr && addr <= 0x401f)
            return; // apu related - didnt implement yet
        */ else if (0x4020 <= addr && addr <= 0xffff)
            mapper->cpu_write(addr, data);
    }

    uint8_t Bus::cpu_read(uint16_t addr) {
        if (0 <= addr && addr <= 0x1fff)
            return ram[addr % 0x800];
        else if (0x2000 <= addr && addr <= 0x3fff) {
            return cpu_read_ppu(addr % 8);
        } /* else if (0x4000 <= addr && addr <= 0x4017) {
            return 0; // apu related - didnt implement yet
        } else if (0x4018 <= addr && addr <= 0x401f) {
            return 0; // apu related - didnt implement yet
        } */ else if (0x4020 <= addr && addr <= 0xffff) {
            return mapper->cpu_read(addr);
        }

        return 0;
    }

    void Bus::ppu_write(uint16_t addr, uint8_t data) {
        ppu_bus_latch = data;

        if (0x2000 <= addr && addr <= 0x3eff) {
            uint16_t foo = addr;

            addr %= 0x1000;
            addr %= 0x400;

            if (mapper->Get_mirroring() == 'H') {
                if (0 <= foo && foo <= 0x800)
                    vram[0][addr] = data;
                else
                    vram[1][addr] = data;
            } 
            else if (mapper->Get_mirroring() == 'V') {
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
            } 
            else if (mapper->Get_mirroring() == 'V') {
                if (0 <= addr && addr <= 0x400 || 0x800 <= addr && addr <= 0x2c00)
                    return vram[0][addr];
                else
                    return vram[1][addr];
            }
        }

        return 0;
    }

    struct Color* Bus::ppu_get_color(uint16_t addr) {
        if (0x3f00 <= addr && addr <= 0x3fff) {
            addr %= 32;
            // TODO: later change to avoid the black screen in super mario
            if (addr == 0x10 || addr == 0x14 || addr == 0x18 || addr == 0x1c)
                addr -= 0x10;

            return &(palette[addr]);
        } 
        else
            return nullptr;
    }

    void Bus::cpu_write_ppu(uint16_t addr, uint8_t data) {
        ppu_bus_latch = data;
        uint16_t data16 = data;

        switch (addr % 8) {
            case PPUCTRL:
                // if (total_frames <= 30000) return; // but not really important
                ppu->ext_regs.ppuctrl = data;
                ppu->t = ppu->t | ((0b00000011 & data) << 10);
                break;
            case PPUMASK:
                ppu->ext_regs.ppumask = data;
                break;
            case PPUSCROLL:
                if (ppu->w == 0) {
                    ppu->t = ((ppu->t >> 5) << 5) | (data16 >> 3); // setting coarse x
                    ppu->x = data16 & 0b0000000000000111; // setting fine x
                }
                else { //setting coarse y and fine y
                    ppu->t = (ppu->t & 0b0000110000011111); // note: i set another 0 at the start - because the register is 16 bits, and not 15 bits like it should be
                    ppu->t = ppu->t | (data16 << 12);
                    ppu->t = ppu->t | ((data16 >> 3) << 5);
                }

                ppu->w = 1 - ppu->w;
                break;
            case PPUADDR:
                if (ppu->w == 0) {
                    ppu->v &= 0b0011111111111111; // clearing bit 14 on first write.
                    data16 = data16 & 0b00111111;
                    ppu->t = (ppu->t & 0b0000000011111111) | (data16 << 8); // bit 14 is set to 0 in this case and the register is 15 bits wide, so bit 15 is not set
                } 
                else {
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
                ret = (ppu->ext_regs.ppustatus & 0b11100000) | (ppu_bus_latch & 0b00011111); // returning the last 5 bits of the latch and 3 top bits of the status register
                ppu->ext_regs.ppustatus &= 0b01111111; // clearing vblank flag
                break;
            case PPUDATA:
                ret = ppu_bus_latch;
                ppu_bus_latch = ppu_read(ppu->v); // not sure about this
                if (addr >= 0x3f00)
                    ret = ppu_read(ppu->v);
                // else return palette data
                ppu->v += (ppu->ext_regs.ppuctrl & 0b00000100) ? 32 : 1; // MIGHT CAUSE PROBLEM
                break;
        }

        return ret;
    }

    void Bus::log() const {
        uint16_t index = 0x10;
        
        static std::ofstream log("testr/logs/ROEE_NES.log", std::ios::out | std::ios::trunc);

        log << "------------- PPU SECTION -------------" << std::endl;
        // log << std::hex << std::uppercase << "Memory at: " << index << "is: " << (int)ram[index] << std::endl;

        log << "frame oddness: " << std::hex << std::uppercase << (int)ppu->frame_oddness << std::endl;
        log << "scanline: " << std::dec << ppu->curr_scanline << std::endl;
        log << "cycle: " << std::dec << ppu->curr_cycle << std::endl;
        log << "t: " << std::hex << std::uppercase << (int)ppu->v << std::endl;
        log << "v: " << std::hex << std::uppercase << (int)ppu->t << std::endl;
        log << "x: " << std::hex << std::uppercase << (int)ppu->x << std::endl;
        log << "w: " << std::hex << std::uppercase << (int)ppu->w << std::endl;
        log << "ppuctrl: " << std::hex << std::uppercase << (int)ppu->ext_regs.ppuctrl << std::endl;
        log << "ppumask: " << std::hex << std::uppercase << (int)ppu->ext_regs.ppumask << std::endl;
        log << "ppustatus: " << std::hex << std::uppercase << (int)ppu->ext_regs.ppustatus << std::endl;
        log << "---------------------------------------" << std::endl;

        log << "------------- CPU SECTION -------------" << std::endl;
        log << std::hex << std::uppercase << "PC: $" << cpu->log_PC << std::endl;
        log << std::hex << std::uppercase << "Opcode: " << (int)cpu->IR << std::endl;
        log << "Operation: " << cpu->inst->name << std::endl;
        log << std::hex << std::uppercase << "Params " << (int)(cpu->log_bytes) << std::endl;
        log << std::hex << std::uppercase << "A: " << (int)cpu->A << std::endl;
        log << std::hex << std::uppercase << "X: " << (int)cpu->X << std::endl;
        log << std::hex << std::uppercase << "Y: " << (int)cpu->Y << std::endl;
        log << std::uppercase << "P: " << get_binary(cpu->P, 8) << " (" << std::dec << (int)cpu->P << ")" << std::endl;
        log << std::hex << std::uppercase << "SP: " << (int)cpu->S << std::endl;
        log << "---------------------------------------" << std::endl;
    }
}

