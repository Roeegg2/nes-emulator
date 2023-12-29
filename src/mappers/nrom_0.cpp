#include "../../include/mappers/nrom_0.h"

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

NROM_0::NROM_0(Cartridge* cart){
    this->cart = cart;
}