#include "../../include/mappers/unrom_2.h"

// later generelize this to UxROM nad have UNROM inherit from it
namespace roee_nes {
    UNROM_2::UNROM_2(Cartridge* cart) :
        Mapper(cart), prg_bank_select(0) {
        last_bank = cart->prg_rom.size() - 0x4000;
        cart->chr_ram.resize(8 * KILOBYTE);
    }

    uint8_t UNROM_2::cpu_read(uint16_t addr) {
        uint32_t addr_32;

        if ((0x6000 <= addr) && (addr <= 0x7fff))
            std::cerr << "reading from PRG RAM\n";
        else if ((0x8000 <= addr) && (addr <= 0xbfff))
            addr_32 = (prg_bank_select * 0x4000) + (addr - 0x8000);
        else if ((0xc000 <= addr) && (addr <= 0xffff))
            addr_32 = (last_bank) + (addr % 0xc000);

        return cart->prg_rom[addr_32];
    }

    void UNROM_2::cpu_write(uint16_t addr, uint8_t data) {
        if ((0x6000 <= addr) && (addr <= 0x7fff))
            std::cerr << "writing to PRG RAM\n";
        else if ((0x8000 <= addr) && (addr <= 0xffff))
            prg_bank_select = 0b0000'0111 & data;
        else
            std::cerr << "WARN: cpu writing not in range\n";
    }

    uint8_t UNROM_2::ppu_read(uint16_t addr) {
        // std::cout << "printing chr_ram:\n";
        // for (auto it = cart->chr_ram.cbegin(); it != cart->chr_ram.cend(); it++)
        //     std::cout << (int)*it << ",";
        // std::cout << "\n";
        // exit(1);
        return cart->chr_ram[addr]; // pattern table
    }

    void UNROM_2::ppu_write(uint16_t addr, uint8_t data) {
        cart->chr_ram[addr] = data;
    }
}