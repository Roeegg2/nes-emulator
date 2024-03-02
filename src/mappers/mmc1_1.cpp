#include "../../include/mappers/mmc1_1.h"
#include <bitset>
namespace roee_nes {

    MMC1_1::MMC1_1(Cartridge* cart) :
        Mapper(cart), shift_reg(0b0001'0000), ctrl({ 0 }), chr_bank({ 0 }), prg_bank({ 0 }) {
        if (cart->chr_rom.size() == 0) {
            cart->chr_ram.resize(8 * KILOBYTE);
            chr_read_mem = &cart->chr_ram;
            using_chr_ram = true;
        } else {
            using_chr_ram = false;
            chr_read_mem = &cart->chr_rom;
        }

        last_prg_rom_bank = cart->chr_rom.size() - (16 * KILOBYTE);
    }

    void MMC1_1::cpu_write(uint16_t addr, uint8_t data) {
        if ((0x6000 <= addr) && (addr <= 0x7fff)) {
            std::cout << "exit!\n";
            return;
        }
        if (data & 0b1000'0000) {
            shift_reg = 0b0001'0000;
            ctrl.comp.prg_rom_mode = 3;
        } else {
            if (shift_reg & 0b0000'0001) { // if this is the 5th write
                uint8_t target_reg = (addr >> 13) & 0b0000'0000'0000'0011;

                switch (target_reg) {
                    case 0:
                        ctrl.raw = ((data & 0b0000'0001) << 4) | ((shift_reg & 0b0001'1110) >> 1);
                    case 1:
                        chr_bank.bank_0 = ((data & 0b0000'0001) << 4) | ((shift_reg & 0b0001'1110) >> 1);
                        break;
                    case 2:
                        chr_bank.bank_1 = ((data & 0b0000'0001) << 4) | ((shift_reg & 0b0001'1110) >> 1);
                        break;
                    case 3:
                        prg_bank.bank = (shift_reg & 0b0001'1110) >> 1;
                        prg_bank.ext = (data & 0b0000'0001) << 4;
                        break;
                }
                shift_reg = 0b0001'0000;
            } else { /// emplace back bit 0 of data
                shift_reg >>= 1;
                shift_reg |= (0b0001'0000 & (data << 4));
            }
        }
    }

    uint8_t MMC1_1::cpu_read(uint16_t addr) {
        if ((0x6000 <= addr) && (addr <= 0x7fff)) {
            std::cout << "exit!!!\n";
            return 0;
        }

        switch (ctrl.comp.prg_rom_mode) {
            case 0:
            case 1: // switch 32 KB at $8000
                return cart->prg_rom[((prg_bank.bank & 0b1110) * 32 * KILOBYTE) + (addr % 0x8000)];
            case 2: // fix first bank at $8000 and switch 16 KB bank at $C000
                if ((0x8000 <= addr) && (addr <= 0xbfff))
                    return cart->prg_rom[(addr % 0x8000)]; // get first bank
                else // addr is 0xc000 and over
                    return cart->prg_rom[(prg_bank.bank * 16 * KILOBYTE) + (addr % 0x8000)];
            case 3:
                if ((0x8000 <= addr) && (addr <= 0xbfff))
                    return cart->prg_rom[(prg_bank.bank * 16 * KILOBYTE) + (addr % 0x8000)];
                else // addr is 0xc000 and over
                    return cart->prg_rom[last_prg_rom_bank + (addr % 0x8000)]; // get last bank
        }
    }

    uint8_t MMC1_1::ppu_read(uint16_t addr) {
        switch (ctrl.comp.chr_rom_mode) {
            case 0:
                return (*chr_read_mem)[(chr_bank.bank_0 * 8 * KILOBYTE) + addr];
            default: // case 1:
                if ((0x000 <= addr) && (addr <= 0x0fff))
                    return (*chr_read_mem)[(chr_bank.bank_0 * 4 * KILOBYTE) + addr];
                else // if ((0x1000 <= addr) && (addr <= 0x1fff))
                    return (*chr_read_mem)[(chr_bank.bank_1 * 4 * KILOBYTE) + (addr % 0x1000)];
        }
    }

    void MMC1_1::ppu_write(uint16_t addr, uint8_t data) {
        if (using_chr_ram == true) {
            switch (ctrl.comp.chr_rom_mode) {
                case 0:
                    (*chr_read_mem)[(chr_bank.bank_0 * 8 * KILOBYTE) + addr] = data;
                case 1:
                    if ((0x000 <= addr) && (addr <= 0x0fff))
                        (*chr_read_mem)[(chr_bank.bank_0 * 4 * KILOBYTE) + addr] = data;
                    else // if ((0x1000 <= addr) && (addr <= 0x1fff))
                        (*chr_read_mem)[(chr_bank.bank_1 * 4 * KILOBYTE) + (addr % 0x1000)] = data;

                    // std::cerr << "MMC1 BAD PPU READ!\n";
                    // return 0;
            }
        }
    }

    void MMC1_1::reset() {
        shift_reg = 0b0001'0000;
        ctrl.comp.prg_rom_mode = 3;
        // set prg rom to bank mode 3 (fixing the last bank at $C000 and allowing the 16 KB bank at $8000 to be switched)
    }

    uint16_t MMC1_1::get_nt_mirrored_addr(uint16_t addr) {
        if (ctrl.comp.mirroring == 0b00)
            return addr % 0x400;
        else if (ctrl.comp.mirroring == 0b01)
            return (addr % 0x400) + 0x400;
        else if (ctrl.comp.mirroring == 0b10) // vertical
            return addr % 0x800;
        else //if (ctrl.comp.mirroring == 0b11) 
        { // horizontal
            if (addr >= 0x800)
                return (addr % 0x400) + 0x400;
            else
                return addr % 0x400;
        }
        // else
        //     std::cerr << "MMC1 MIRRORING PROBLEM!\n";
    }
}