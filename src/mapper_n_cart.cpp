#include "../include/mapper_n_cart.h"
#include "../include/mappers/nrom_0.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

Mapper* Mapper::create_mapper(std::string rom_path){
    Cartridge* cart = new Cartridge(rom_path);

    uint8_t mapper_number = (cart->header.flags_6 & 0xf0) | (cart->header.flags_7 >> 4);

    switch(mapper_number){
        case 0:
            return new NROM_0(cart);
        default:
            std::cerr << "Mapper not implemented yet!" << std::endl;
            return nullptr;
    }
}

// need also to take care of the case there is a trainer section
Cartridge::Cartridge(std::string rom_path){
    static std::ifstream rom_file(rom_path, std::ios::binary);

    if (!rom_file.is_open()){
        std::cerr << "Error opening file" << std::endl;
        exit(1);
    }

    char signature[5];
    rom_file.read(signature, 4);
    if (strcmp(signature, "NES\x1A") != 0) {
        std::cerr << "Not a valid NES ROM" << std::endl;
        exit(1);
    }

    rom_file.read((char*)&header.prg_bank_size, 1);
    rom_file.read((char*)&header.chr_bank_size, 1);
    rom_file.read((char*)&header.flags_6, 1);
    rom_file.read((char*)&header.flags_7, 1);
    rom_file.read((char*)&header.flags_8, 1);
    rom_file.read((char*)&header.flags_9, 1);
    rom_file.read((char*)&header.flags_10, 1);

    header.total_size = ((header.prg_bank_size * 16) + (header.chr_bank_size * 8)); // the prg and chr banks are in 16kb and 8kb respectively

    prg_rom.resize(header.prg_bank_size * 16 * KILOBYTE);
    chr_rom.resize(header.chr_bank_size * 8 * KILOBYTE);

    rom_file.seekg(16);
    rom_file.read((char*)prg_rom.data(), prg_rom.size());
    std::cout << std::hex << (int)prg_rom[0] << std::endl;
    rom_file.read((char*)chr_rom.data(), chr_rom.size());

    if (header.flags_10 & 0b00000010) // if this bit in flag 10 is set then there is prg ram
        prg_ram.resize(header.flags_8 * 8 * KILOBYTE); // flag 8 is the size of the prg ram in 8kb units
    else
        prg_ram.resize(0);
}

uint8_t Mapper::cpu_read(uint16_t addr){
    return 0;
}

void Mapper::cpu_write(uint16_t addr, uint8_t data){
    return;
}