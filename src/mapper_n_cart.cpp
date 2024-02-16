#include "../include/mapper_n_cart.h"
#include "../include/mappers/nrom_0.h"
#include "../include/mappers/unrom_2.h"
#include "../include/mappers/cnrom_3.h"

namespace roee_nes {
    Mapper* Mapper::create_mapper(const std::string* rom_path) {
        Cartridge* cart = new Cartridge(rom_path);

        uint8_t mapper_number = ((cart->header.flags_7 >> 4) << 4) | (cart->header.flags_6 >> 4);

        switch (mapper_number) {
            case 0: // mapper number 0 (NROM)
                return new NROM_0(cart);
            case 2: // mapper number 2 (UNROM)
                return new UNROM_2(cart);
            case 3: // mapper number 3 (CNROM)
                return new CNROM_3(cart);
            default:
                std::cerr << "ERROR: Mapper number " << std::dec << (int)mapper_number << "not implemented yet/does not exist\n";
                exit(1);
        }
    }

    // need also to take care of the case there is a trainer section
    Cartridge::Cartridge(const std::string* rom_path) {
        std::ifstream rom_file(*rom_path, std::ios::binary);

        if (!rom_file.is_open()) {
            std::cerr << "ERROR: Error opening file" << "\n";
            exit(1);
        }

        {
            char signature[5];
            rom_file.read(signature, 4);
            if (strcmp(signature, "NES\x1A") != 0) {
                std::cerr << "ERROR: Not a valid NES ROM" << "\n";
                exit(1);
            }
        }

        rom_file.read((char*)&header.prg_rom_size, 1);
        rom_file.read((char*)&header.chr_rom_size, 1);
        rom_file.read((char*)&header.flags_6, 1);
        rom_file.read((char*)&header.flags_7, 1);
        rom_file.read((char*)&header.flags_8, 1);
        rom_file.read((char*)&header.flags_9, 1);
        rom_file.read((char*)&header.flags_10, 1);

        header.total_size = ((header.prg_rom_size * 16) + (header.chr_rom_size * 8)); // the prg and chr banks are in 16kb and 8kb respectively

        rom_file.seekg(16);
        prg_rom.resize(header.prg_rom_size * 16 * KILOBYTE);
        rom_file.read((char*)prg_rom.data(), prg_rom.size());

        if (header.chr_rom_size == 0) {
            chr_ram.resize(1 * 8 * KILOBYTE); // NOTE: this is specific for UNROM! change that later
            rom_file.read((char*)chr_ram.data(), chr_ram.size());
        } else {
            chr_rom.resize(header.chr_rom_size * 8 * KILOBYTE);
            rom_file.read((char*)chr_rom.data(), chr_rom.size());
        }


        if (header.flags_10 & 0b00000010) // if this bit in flag 10 is set then there is prg ram
            prg_ram.resize(header.flags_8 * 8 * KILOBYTE); // flag 8 is the size of the prg ram in 8kb units
        else
            prg_ram.resize(0);
    }
}