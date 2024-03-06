#include "../../include/mappers/unrom_2.h"

// later generelize this to UxROM nad have UNROM inherit from it
namespace roee_nes {
    UNROM_2::UNROM_2(Cartridge* cart) :
        Mapper(cart), prg_bank_select(0) {
        if (cart->chr_rom.size() == 0) {
            using_chr_ram = true;
            cart->chr_ram.resize(8 * KILOBYTE);
        }
        else
            using_chr_ram = false;

        last_bank = cart->prg_rom.size() - 0x4000;
    }

    uint8_t UNROM_2::cpu_read(uint16_t addr, uint8_t open_bus_data) {
        if ((0x8000 <= addr) && (addr <= 0xbfff))
            return cart->prg_rom[(prg_bank_select * 0x4000) + (addr % 0x4000)];
        else if ((0xc000 <= addr) && (addr <= 0xffff))
            return cart->prg_rom[last_bank + (addr % 0x4000)];
        else {
            return open_bus_data;
        }
    }

    void UNROM_2::cpu_write(uint16_t addr, uint8_t data) {
        prg_bank_select = 0b0000'0111 & data;
    }

    uint8_t UNROM_2::ppu_read(uint16_t addr) {
        if (using_chr_ram)
            return cart->chr_ram[addr];
        else
            return cart->chr_rom[addr];
    }

    void UNROM_2::ppu_write(uint16_t addr, uint8_t data) {
        if (using_chr_ram)
            cart->chr_ram[addr % 0x4000] = data;
        else
            std::cerr << "WARNING: trying to write to CHR ROM\n";
    }
}