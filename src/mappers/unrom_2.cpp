#include "../../include/mappers/unrom_2.h"

// later generelize this to UxROM nad have UNROM inherit from it
namespace roee_nes {
    UNROM_2::UNROM_2(Cartridge* cart) :
        Mapper(cart), prg_bank_select(0) {}

    uint8_t UNROM_2::cpu_read(uint16_t addr) {
        uint32_t addr_32;

        if ((0x8000 <= addr) && (addr <= 0xbfff))
            addr_32 = (prg_bank_select * 0x4000) + (addr - 0x8000);
        else if ((0xc000 <= addr) && (addr <= 0xffff))
            addr_32 = (cart->prg_rom.size() - 0x4000) + (addr % 0xc000);

        return cart->prg_rom[addr_32];
    }

    void UNROM_2::cpu_write(uint16_t addr, uint8_t data) {
        if ((0x8000 <= addr) && (addr <= 0xffff))
            prg_bank_select = 0b0000'0111 & data;
        else
            std::cerr << "WARNING! cpu writing not in range\n";
    }

    uint8_t UNROM_2::ppu_read(uint16_t addr) {
        return cart->chr_ram[addr]; // pattern table
    }

    void UNROM_2::ppu_write(uint16_t addr, uint8_t data) {
        cart->chr_ram[addr] = data;
    }
}