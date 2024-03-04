#include "../../include/mappers/mmc1_1.h"
#include <bitset>
namespace roee_nes {

    MMC1_1::MMC1_1(Cartridge* cart) :
        Mapper(cart), shift_reg(0b0001'0000), ctrl({ 0 }), chr_bank({ 0 }), prg_bank({ 0 }) {
        if (cart->chr_rom.size() == 0) {
            cart->chr_ram.resize(8 * KILOBYTE);
            chr_bank_num = 2;
            chr_read_mem = &cart->chr_ram;
            using_chr_ram = true;
        } else {
            using_chr_ram = false;
            chr_read_mem = &cart->chr_rom;
            chr_bank_num = (cart->chr_rom.size() / (4 * KILOBYTE));
        }

        prg_bank_num = (cart->prg_rom.size() / (16 * KILOBYTE));
        // if (cart->header.flag_6.parsed.prg_ram == 1) {
        //     save_data.open("AAAA", std::ios::in | std::ios::out | std::ios::app | std::ios::binary);
        //     std::cout << "hjere!\n";
        //     if (save_data.is_open())
        //         std::cout << "yesh\n"; 
        // }
    }

    void MMC1_1::cpu_write(uint16_t addr, uint8_t data) {
        if ((0x6000 <= addr) && (addr <= 0x7fff)) {
            // save_data.seekg(addr % 0x6000);
            // save_data.write(reinterpret_cast<const char*>(&data), sizeof(data));
            save_data[addr % 0x6000] = data;
        } else if (data & 0b1000'0000) {
            shift_reg = 0b0001'0000;
            ctrl.comp.prg_rom_mode = 3;
        } else {
            if (shift_reg & 0b0000'0001) { // if this is the 5th write
                uint8_t target_reg = (addr >> 13) & 0b0000'0000'0000'0011;
                uint8_t foo = (((data & 0b0000'0001) << 4) | ((shift_reg & 0b0001'1110) >> 1));

                switch (target_reg) {
                    case 0:
                        ctrl.comp.mirroring = foo & 0b00011;
                        ctrl.comp.prg_rom_mode = (foo & 0b01100) >> 2;
                        ctrl.comp.chr_rom_mode = (foo & 0b10000) >> 4;
                        break;
                    case 1:
                        chr_bank.bank_0 = foo;
                        break;
                    case 2:
                        chr_bank.bank_1 = foo;
                        break;
                    case 3:
                        prg_bank.bank = foo & 0b0000'1111;
                        prg_bank.ext = (foo >> 4) & 0b0000'0001;
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
        // std::cout << "prg mode "
        //     << std::hex << " chr rom mode " << (int)ctrl.comp.chr_rom_mode
        //     << std::hex << " prg rom mode " << (int)ctrl.comp.prg_rom_mode
        //     << std::hex << " mirroring " << (int)ctrl.comp.mirroring
        //     << std::hex << " prg bank " << (int)prg_bank.bank
        //     << std::hex << " chr bank 0 " << (int)chr_bank.bank_0
        //     << std::hex << " chr bank 1 " << (int)chr_bank.bank_1
        //     << "\n";
        if ((0x6000 <= addr) && (addr <= 0x7fff))
            return save_data[addr % 0x6000];
        
        update_prg(addr);
        return cart->prg_rom[(final_bank * 16 * KILOBYTE) + final_addr];
    }

    void MMC1_1::update_chr(uint16_t addr) {
        if (ctrl.comp.chr_rom_mode == 0) {
            final_addr = addr % 0x2000;
            final_bank = (chr_bank.bank_0 % (chr_bank_num / 2)) & 0b0000'1110;
        } else {
            final_addr = addr % 0x1000;
            if ((0x0000 <= addr) && (addr <= 0x0fff))
                final_bank = chr_bank.bank_0 % chr_bank_num;
            else // if ((0x1000 <= addr) && (addr <= 0x1fff)) {
                final_bank = chr_bank.bank_1 % chr_bank_num;
        }
    }

    void MMC1_1::update_prg(uint16_t addr) {
        switch (ctrl.comp.prg_rom_mode) {
            case 0:
            case 1: // switch 32 KB at $8000
                final_bank = (prg_bank.bank % (prg_bank_num)) & 0b0000'1110;
                final_addr = addr % 0x8000;
                break;
            case 2: // fix first bank at $8000 and switch 16 KB bank at $C000
                final_addr = addr % 0x4000;
                if ((0x8000 <= addr) && (addr <= 0xbfff))
                    final_bank = 0;
                else // addr is 0xc000 and over
                    final_bank = (prg_bank.bank % prg_bank_num);
                break;
            case 3:
                final_addr = addr % 0x4000;
                if ((0x8000 <= addr) && (addr <= 0xbfff))
                    final_bank = prg_bank.bank % prg_bank_num;
                else // addr is 0xc000 and over
                    final_bank = prg_bank_num - 1;
                break;
        }
    }

    uint8_t MMC1_1::ppu_read(uint16_t addr) {
        update_chr(addr);
        return (*chr_read_mem)[(final_bank * 4 * KILOBYTE) + final_addr];
    }

    void MMC1_1::ppu_write(uint16_t addr, uint8_t data) {
        if (using_chr_ram == true) {
            update_chr(addr);
            (*chr_read_mem)[(final_bank * 4 * KILOBYTE) + final_addr] = data;
        } else
            std::cerr << "ERR trying to write to chr rom!\n";
    }


    void MMC1_1::reset() {
        shift_reg = 0b0001'0000;
        ctrl.comp.prg_rom_mode = 3;
        // set prg rom to bank mode 3 (fixing the last bank at $C000 and allowing the 16 KB bank at $8000 to be switched)
    }

    uint16_t MMC1_1::get_nt_mirrored_addr(uint16_t addr) {
        switch (ctrl.comp.mirroring) {
            case 0: // single nt, first one
                return addr % 0x400;
            case 1: // single nt, second one
                return (addr % 0x400) + 0x400;
            case 2: // horizontal mirroring
                return addr % 0x800;
            default: // vertical mirroring
                if (addr >= 0x800)
                    return addr % 0x400;
                else
                    return (addr % 0x400) + 0x400;
        }
    }
}