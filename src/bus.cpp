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
            switch (addr % 8) {
                case PPUCTRL:
                ppu->ext_regs.ppuctrl = data;
                ppu->t = ppu->t | ((0b00000011 & data) << 10);
                break;
                case PPUMASK:
                ppu->ext_regs.ppumask = data;
                break;
                case PPUSTATUS:
                break;
                case OAMADDR:
                break;
                case OAMDATA:
                break;
                case PPUSCROLL:
                if (ppu->w == 0) {
                    ppu->t = ((ppu->t >> 5) << 5) & (data >> 3);
                    ppu->x = data & 0b00000111;
                    ppu->w = 1; // NOTE: move this out of the if statement
                } else {
                    ppu->t = (ppu->t & 0b0000110000011111); // note: i set another 0 at the start - because the register is 16 bits, and not 15 bits like it should be
                    ppu->t = ppu->t | (data << 12);
                    ppu->t = ppu->t | ((data >> 3) << 5);
                    ppu->w = 0; // NOTE: move this out of the if statement
                }
                break;
                case PPUADDR:
                if (ppu->w == 0) {
                    data = data & 0b00111111;
                    ppu->t = (ppu->t & 0b0000000011111111) | (data << 8);
                    ppu->w = 1; // NOTE: move this out of the if statement
                } else {
                    ppu->t = (ppu->t & 0b1111111100000000) | data;
                    ppu->v = ppu->t; // TODO: do this every 3 cycles to make more games compatible
                    ppu->w = 0;      // NOTE: move this out of the if statement
                }
                break;
                case PPUDATA:
                vram[ppu->v] = data;                                     // not sure about this
                ppu->v += (ppu->ext_regs.ppuctrl & 0b00000100) ? 32 : 1; // not sure about this
                break;
            }
        } else if (0x4000 <= addr && addr <= 0x4017)
            return; // apu related - didnt implement yet
        else if (0x4018 <= addr && addr <= 0x401f)
            return; // apu related - didnt implement yet
        else if (0x4020 <= addr && addr <= 0xffff)
            mapper->cpu_write(addr, data);
    }

    uint8_t Bus::cpu_read(uint16_t addr) {
        uint16_t tee;
        if (0 <= addr && addr <= 0x1fff)
            return ram[addr % 0x800];
        else if (0x2000 <= addr && addr <= 0x3fff) {
            switch (addr % 8) {
                case PPUCTRL:
                case PPUMASK:
                break;
                case PPUSTATUS: // TODO: implement something with games using the ppustatus register for valid data
                tee = ppu->ext_regs.ppustatus;
                ppu->w = 0;
                ppu->ext_regs.ppustatus = ppu->ext_regs.ppustatus & 0b01111111;
                return tee;
                case OAMADDR:
                case OAMDATA:
                case PPUSCROLL:
                case PPUADDR:
                break;
                case PPUDATA:
                uint8_t temp = vram[ppu->v];
                ppu->v += (ppu->ext_regs.ppuctrl & 0b00000100) ? 32 : 1; // not sure about this
                return temp;
            }
        } else if (0x4000 <= addr && addr <= 0x4017) {
            return 0; // apu related - didnt implement yet
        } else if (0x4018 <= addr && addr <= 0x401f) {
            return 0; // apu related - didnt implement yet
        } else if (0x4020 <= addr && addr <= 0xffff) {
            return mapper->cpu_read(addr);
        }

        return 0;
    }

    uint32_t Bus::ppu_read(uint16_t addr) {
        if (0 <= addr && addr <= 0x1fff) {
            return mapper->ppu_read(addr); // pattern table
        } else if (0x2000 <= addr && addr <= 0x3eff) {
            return mapper->ppu_read(addr); // nametable and attribute table
        }

        return 0;
    }

    Color* Bus::ppu_get_color(uint16_t addr) {
        if (0x3f00 <= addr && addr <= 0x3fff) {
            addr %= 32;
            // TODO: later change to avoid the black screen in super mario
            if (addr == 0x10 || addr == 0x14 || addr == 0x18 || addr == 0x1c)
                addr -= 0x10;

            return &(palette[addr]);
        } else
            return nullptr;
    }
}