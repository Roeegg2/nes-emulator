#include "../../include/mappers/nrom_0.h"

NROM_0::NROM_0(Cartridge* cart) {
    this->cart = cart;

    cart->vram.resize(0x800);
}

uint8_t NROM_0::cpu_read(uint16_t addr) {
    if (0x6000 <= addr && addr <= 0x7FFF) // not sure if its only in famicom
        return cart->prg_rom[addr - 0x6000];
    else if (cart->header.total_size == 40)
        return cart->prg_rom[addr - 0x8000];
    else
        return cart->prg_rom[(addr - 0x8000) % 0x4000];
}

void NROM_0::cpu_write(uint16_t addr, uint8_t data) {
    return; // do nothing
}

uint8_t NROM_0::ppu_read(uint16_t addr) {
    if (0x0000 <= addr && addr <= 0x1fff)
        return cart->chr_rom[addr]; // pattern table
    else if (0x2000 <= addr && addr <= 0x3eff)
        return cart->vram[addr % 0x800]; // dependent on mirroring i think
    else if (0x3f00 <= addr && addr <= 0x3fff)
        return 1; // palette

    return 0;
}