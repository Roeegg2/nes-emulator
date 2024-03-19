
#include <iostream>

#include "../../include/mappers/mmc3_4.h"

namespace roee_nes {
    MMC3_4::MMC3_4(Cartridge* cart) :
        Mapper(cart) {
        set_irq = false;
        mapper_number = 4;

        if (cart->chr_rom.size() == 0) {
            cart->chr_ram.resize(8 * KILOBYTE);
            using_chr_ram = true;
            chr_read_mem = &cart->chr_ram;
        } else {
            using_chr_ram = false;
            chr_read_mem = &cart->chr_rom;
        }

        chr_bank_num = (chr_read_mem->size() / (1 * KILOBYTE)) - 2;
        prg_bank_num = (cart->prg_rom.size() / (8 * KILOBYTE));

        if (cart->header.flag_6.parsed.prg_ram == 1)
            save_ram = new Save_RAM(cart->rom_path + ".sav");
        else
            save_ram = nullptr;

        prg_bank[0] = 0;
        prg_bank[1] = 0;
        chr_bank[0] = 0;
        chr_bank[1] = 0;
        chr_bank[2] = 0;
        chr_bank[3] = 0;
        chr_bank[4] = 0;
        chr_bank[5] = 0;
    }

    void MMC3_4::cpu_write(uint16_t addr, uint8_t data) {
        if ((0x6000 <= addr) && (addr <= 0x7fff)) {
            save_ram->mapper_write(addr, data);
        } else if ((0x8000 <= addr) && (addr <= 0x9ffe)) {
            if ((addr % 2) == 0) { // write to low reg
                bank_select.comp.even.select = data & 0b0000'0111;
                bank_select.comp.even.prg_rom_bank_mode = (data & 0b0100'0000) >> 6;
                bank_select.comp.even.chr_a12_inversion = (data & 0b1000'0000) >> 7;
            } else {
                switch (bank_select.comp.even.select) {
                    case 0b000:
                        chr_bank[0] = data & 0b1111'1110;
                        break;
                    case 0b001:
                        chr_bank[1] = data & 0b1111'1110;
                        break;
                    case 0b010:
                        chr_bank[2] = data;
                        break;
                    case 0b011:
                        chr_bank[3] = data;
                        break;
                    case 0b100:
                        chr_bank[4] = data;
                        break;
                    case 0b101:
                        chr_bank[5] = data;
                        break;
                    case 0b110:
                        prg_bank[0] = data & 0b0011'1111;
                        break;
                    case 0b111:
                        prg_bank[1] = data & 0b0011'1111;
                        break;
                }
            }
        } else if ((0xa000 <= addr) && (addr <= 0xbfff)) {
            if ((addr % 2) == 0)
                mirroring = data & 0b0000'0001;
            else
                prg_ram_protect = data & 0b1100'0000;
        } else if ((0xc000 <= addr) && (addr <= 0xdffe)) {
            if ((addr % 2) == 0)
                irq_latch = data;
            else {
                irq_counter = 0;
            }
        } else if ((0xe000 <= addr) && (addr <= 0xffff)) {
            if ((addr % 2) == 0) {
                irq_enabled = false;
                set_irq = false;
            } else
                irq_enabled = true;
        }
    }

    uint8_t MMC3_4::cpu_read(uint16_t addr, uint8_t open_bus_data) {
        // std::cout << std::hex
        // << " current reg: " << (int)bank_select.comp.even.select << " \n"
        // << " prg banking mode: " << (int)bank_select.comp.even.prg_rom_bank_mode << " \n"
        // << " chr a12: " << (int)bank_select.comp.even.chr_a12_inversion << " \n"
        // << " mirroring: " << (int)mirroring << " \n"
        // << " prg ram protect: " << (int)(prg_ram_protect & 0b0100'0000) << " \n"
        // << " prg ram enable: " << (int)(prg_ram_protect & 0b1000'0000) << " \n"
        // << " chr bank 0: " << (int)chr_bank_2kb[0] << " \n"
        // << " chr bank 1: " << (int)chr_bank_2kb[1] << " \n"
        // << " chr bank 0: " << (int)chr_bank_1kb[0] << " \n"
        // << " chr bank 1: " << (int)chr_bank_1kb[1] << " \n"
        // << " chr bank 2: " << (int)chr_bank_1kb[2] << " \n"
        // << " chr bank 3: " << (int)chr_bank_1kb[3] << " \n"
        // << " prg bank 0: " << (int)prg_bank[0] << " \n"
        // << " prg bank 1: " << (int)prg_bank[1] << " \n"
        // << " irq latch: " << (int)irq_latch << " \n"
        // << " irq counter: " << (int)irq_counter << " \n"
        // << " irq enabled: " << (int)irq_enabled << " \n"
        // << " irq reload: " << (int)irq_reload << " \n"
        // << " set irq: " << (int)set_irq << " \n"
        // << "-------------------\n";
        if ((0x6000 <= addr) && (addr <= 0x7fff)) {
            return save_ram->mapper_read(addr);
        } else if ((0x6000 <= addr) && (addr <= 0x7fff)) {
            if ((prg_ram_protect & 0b1000'0000) == 0)
                return open_bus_data;
            else
                return save_data[addr % 0x6000]; // handle prg_ram
        } else if ((0x8000 <= addr) && (addr <= 0xffff)) {
            update_prg(addr);
            return cart->prg_rom[(final_bank * 8 * KILOBYTE) + final_addr];
        } else {
            return open_bus_data;
        }
    }

    void MMC3_4::ppu_write(uint16_t addr, uint8_t data) {
        if (using_chr_ram == true) {
            update_chr(addr);
            (*chr_read_mem)[(final_bank * 1 * KILOBYTE) + final_addr] = data;
        }
        // else
        //     std::cerr << "ERR trying to write to chr rom!\n";
    }

    uint8_t MMC3_4::ppu_read(uint16_t addr) {
        update_chr(addr);
        return (*chr_read_mem)[(final_bank * 1 * KILOBYTE) + final_addr];
    }

    uint16_t MMC3_4::get_nt_mirrored_addr(const uint16_t addr) const {
        if ((mirroring & 0b0000'0001) == 1) { // horizontal mirroring
            if (0x800 <= addr)
                return (addr % 0x400) + 0x400;
            else
                return addr % 0x400;
        } else { // vertical mirroring
            return addr % 0x800;
        }
    }

    void MMC3_4::update_prg(uint16_t addr) {
        if ((0xa000 <= addr) && (addr <= 0xbfff)) {
            final_bank = prg_bank[1];
        } else if ((0xe000 <= addr) && (addr <= 0xffff)) {
            final_bank = prg_bank_num - 1;
        } else {
            uint16_t foo = addr ^ (bank_select.comp.even.prg_rom_bank_mode << 14);
            if ((0x8000 <= foo) && (foo <= 0x9fff)) {
                final_bank = prg_bank[0];
            } else if ((0xc000 <= foo) && (foo <= 0xdfff)) {
                final_bank = prg_bank_num - 2;
            } else {
                std::cout << "addr: " << std::hex << addr << " \n";
                std::cerr << "ERR: MMC3_4::update_prg() - invalid addr\n";
            }
        }

        final_addr = addr % 0x2000;
    }

    void MMC3_4::update_chr(uint16_t addr) {
        uint16_t foo = addr ^ (bank_select.comp.even.chr_a12_inversion << 12);
        if ((0x0000 <= foo) && (foo <= 0x03ff)) {
            final_addr = foo % 0x0800;
            final_bank = chr_bank[0] & 0b1111'1110;
        } else if ((0x0400 <= foo) && (foo <= 0x07ff)) {
            final_addr = foo % 0x0800;
            final_bank = chr_bank[0] & 0b1111'1110;
        } else if ((0x0800 <= foo) && (foo <= 0x0bff)) {
            final_addr = foo % 0x0800;
            final_bank = chr_bank[1];
        } else if ((0x0c00 <= foo) && (foo <= 0x0fff)) {
            final_addr = foo % 0x0800;
            final_bank = chr_bank[1];
        } else if ((0x1000 <= foo) && (foo <= 0x13ff)) {
            final_addr = foo % 0x0400;
            final_bank = chr_bank[2];
        } else if ((0x1400 <= foo) && (foo <= 0x17ff)) {
            final_addr = foo % 0x0400;
            final_bank = chr_bank[3];
        } else if ((0x1800 <= foo) && (foo <= 0x1bff)) {
            final_addr = foo % 0x0400;
            final_bank = chr_bank[4];
        } else if ((0x1c00 <= foo) && (foo <= 0x1fff)) {
            final_addr = foo % 0x0400;
            final_bank = chr_bank[5];
        }
    }

    void MMC3_4::clock_irq() {
        if (irq_counter == 0) {
            irq_counter = irq_latch;
        } else
            irq_counter--;

        if ((irq_counter == 0) && (irq_enabled == true)) {
            set_irq = true;
        }
    }

    void MMC3_4::save() {
        if (save_ram != nullptr)
            save_ram->~Save_RAM();
    }
}