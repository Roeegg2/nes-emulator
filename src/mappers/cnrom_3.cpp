#include "../../include/mappers/cnrom_3.h"

namespace roee_nes {
    CNROM_3::CNROM_3(Cartridge* cart) :
        Mapper(cart), chr_bank_select(0) {}

    uint8_t CNROM_3::cpu_read(uint16_t addr) {
        if ((0x8000 <= addr) && (addr <= 0xffff)) {
            if (cart->header.prg_rom_size == 1) // 16 kib
                addr %= 0x4000;
            else if (cart->header.prg_rom_size == 2) // 32 kib
                addr %= 0x8000;

            return cart->prg_rom[addr];
        } else {
            std::cerr << "WARNING! cpu reading not in range\n";
            return 0;
        }
    }

    void CNROM_3::cpu_write(uint16_t addr, uint8_t data) {
        if ((0x8000 <= addr) && (addr <= 0xffff))
            chr_bank_select = 0b0000'0011 & data;
        else
            std::cerr << "WARNING! cpu writing not in range\n";
    }

    uint8_t CNROM_3::ppu_read(uint16_t addr) {
        return cart->chr_rom[(chr_bank_select * 0x2000) + addr];
    }

    void CNROM_3::ppu_write(uint16_t addr, uint8_t data) {
        return; // do nothing
    }
}