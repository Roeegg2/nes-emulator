#include "../../include/mappers/nrom_0.h"

namespace roee_nes {

    NROM_0::NROM_0(Cartridge* cart)
        : Mapper(cart) {
            set_irq = false;
            mapper_number = 0;
        }

    uint8_t NROM_0::cpu_read(uint16_t addr, uint8_t open_bus_data) {
        if (0x6000 <= addr && addr <= 0x7FFF) // not sure if its only in famicom
            return cart->prg_rom[addr % 0x4000];
        else if ((0x8000 <= addr) && (addr <= 0xffff)) {
            if (cart->header.prg_rom_size == 1) // 16kb prg rom
                return cart->prg_rom[addr % 0x4000];
            else
                return cart->prg_rom[addr % 0x8000];
        }
        else {
            return open_bus_data;
        }
    }

    void NROM_0::cpu_write(uint16_t addr, uint8_t data) {
        return; // do nothing
    }

    void NROM_0::ppu_write(uint16_t addr, uint8_t data) {
        return; // do nothing
    }

    uint8_t NROM_0::ppu_read(uint16_t addr) {
        return cart->chr_rom[addr]; // pattern table
    }
}