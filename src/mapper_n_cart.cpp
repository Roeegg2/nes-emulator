#include "../include/mapper_n_cart.h"
#include "../include/mappers/nrom_0.h"

#include <iostream>
#include <stdio.h>
#include <string.h>

Mapper* Mapper::create_mapper(std::string rom_path){
    Cartridge* cart = new Cartridge(rom_path);

    uint8_t mapper_number = (cart->header.flags_6 & 0xf0) | (cart->header.flags_7 >> 4);

    switch(mapper_number){
        case 0:
            return new NROM_0(cart);
        default:
            std::cout << "Mapper not implemented yet!";
            return nullptr;
    }
}

// need also to take care of the case there is a trainer section
// i had some trouble with the STL file io so i used the c one for now. I might switch to the STL one later
Cartridge::Cartridge(std::string rom_path){
    char signature[5];
    signature[4] = '\0';

    FILE *rom_file = fopen("testr/Donkey_Kong.nes", "rb");

    if (rom_file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    fread(signature, 1, 4, rom_file);
    if (strcmp(signature, "NES\x1A") != 0) {
        printf("Not a valid NES ROM\n");
        exit(1);
    }

    fread(&header.prg_bank_size, 1, 1, rom_file);
    fread(&header.chr_bank_size, 1, 1, rom_file);
    fread(&header.flags_6, 1, 1, rom_file);
    fread(&header.flags_7, 1, 1, rom_file);
    fread(&header.flags_8, 1, 1, rom_file);
    fread(&header.flags_9, 1, 1, rom_file);
    fread(&header.flags_10, 1, 1, rom_file);

    header.total_size = (header.prg_bank_size * 16 + header.chr_bank_size * 8);
    prg_rom.resize(header.prg_bank_size * 16 * KILOBYTE);
    chr_rom.resize(header.chr_bank_size * 8 * KILOBYTE);

    fseek(rom_file, 16, SEEK_SET);
    fread(prg_rom.data(), 1, prg_rom.size(), rom_file);
    fread(chr_rom.data(), 1, chr_rom.size(), rom_file);

    prg_ram.resize(header.prg_ram_size * 8 * KILOBYTE);

    if (header.flags_10 & 0b00000010)
        prg_ram.resize(header.prg_ram_size * 8 * KILOBYTE);
    else
        prg_ram.resize(0);
}

uint8_t Mapper::cpu_read(uint16_t addr){
    return 0;
}

void Mapper::cpu_write(uint16_t addr, uint8_t data){
    return;
}