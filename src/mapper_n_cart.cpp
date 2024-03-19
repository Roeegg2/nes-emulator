#include <iostream>
#include <cstring>
#include <fstream>
#include <filesystem>

#include "../include/mapper_n_cart.h"
#include "../include/mappers/nrom_0.h"
#include "../include/mappers/unrom_2.h"
#include "../include/mappers/cnrom_3.h"
#include "../include/mappers/mmc1_1.h"
#include "../include/mappers/mmc3_4.h"
namespace roee_nes {
    Mapper* Mapper::create_mapper(const std::string& rom_path) {
        Cartridge* cart = new Cartridge(rom_path);

        cart->header.mapper_number = (((uint8_t)cart->header.flag_7.parsed.mapper_num_high) << 4) | cart->header.flag_6.parsed.mapper_num_low;
        switch (cart->header.mapper_number) {
            case 0: // mapper number 0 (NROM)
                std::cout << "USER INFO: Game mapper is NROM (iNES 0)\n";
                return new NROM_0(cart);
            case 1:
                std::cout << "USER INFO: Game mapper is MMC1 (iNES 1)\n";
                return new MMC1_1(cart);
            case 2: // mapper number 2 (UNROM)
                std::cout << "USER INFO: Game mapper is UNROM (iNES 2)\n";
                return new UNROM_2(cart);
            case 3: // mapper number 3 (CNROM)
                std::cout << "USER INFO: Game mapper is CNROM (iNES 3)\n";
                return new CNROM_3(cart);
            case 4: // mapper number 4 (MMC3)
                std::cout << "USER INFO: Game mapper is MMC3 (iNES 4)\n";
                return new MMC3_4(cart);
            default:
                std::cerr << "ERROR: Mapper number " << std::dec << (int)cart->header.mapper_number << " not implemented yet\n";
                exit(1);
        }
    }

    // need also to take care of the case there is a trainer section
    Cartridge::Cartridge(const std::string& rom_path) {
        std::ifstream rom_file(rom_path, std::ios::binary);

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

        this->rom_path = rom_path;
        rom_file.read((char*)&header.prg_rom_size, 1);
        rom_file.read((char*)&header.chr_rom_size, 1);
        rom_file.read((char*)&header.flag_6.raw, 1);
        rom_file.read((char*)&header.flag_7.raw, 1);
        rom_file.read((char*)&header.flag_8.prg_ram_size, 1);
        rom_file.read((char*)&header.flag_9.tv_system, 1);
        rom_file.read((char*)&header.flag_10.raw, 1);

        if (header.flag_6.parsed.trainer == 1) { // if trainer is present
            trainer.resize(512);
            rom_file.read((char*)trainer.data(), trainer.size());
        }

        if (header.flag_6.parsed.prg_ram == 1) { // if there is prg ram
            if (header.flag_8.prg_ram_size == 0) // size 0 means 8kb
                prg_ram.resize(8 * KILOBYTE);
            else
                prg_ram.resize(header.flag_8.prg_ram_size * 8 * KILOBYTE);
        }

        if ((header.flag_9.tv_system == 1) || (header.flag_10.parsed.tv_system == 2)) // game is designed for PAL and not NTSC
            std::cerr << "WARNING: This game was designed for the PAL console, this emulator is for the NTSC console\n";
        if (header.flag_7.parsed.uu_vsunisystem == 1)
            std::cerr << "WARNING: This game is recommended to play with a different palette than the standard one.\n";
        if (header.flag_7.parsed.uu_playchoice == 1)
            std::cerr << "WARNING: This game handles color emphasis differently than the standard NES, and this has not been implemented yet\n";

        header.total_size = ((header.prg_rom_size * 16) + (header.chr_rom_size * 8)); // the prg and chr banks are in 16kb and 8kb respectively

        rom_file.seekg(16);
        prg_rom.resize(header.prg_rom_size * 16 * KILOBYTE);
        rom_file.read((char*)prg_rom.data(), prg_rom.size());
        chr_rom.resize(header.chr_rom_size * 8 * KILOBYTE);
        rom_file.read((char*)chr_rom.data(), chr_rom.size());
    }

    uint16_t Mapper::get_nt_mirrored_addr(const uint16_t addr) const {
        if (cart->header.flag_6.parsed.nt_layout == 1) { // horizontal mirroring
            return addr % 0x800;
        } else { // vertical mirroring
            if (addr >= 0x800)
                return (addr % 0x400) + 0x400;
            else
                return addr % 0x400;
        }
    }

    void Mapper::save() {
        return; // do nothing!
    }

    Save_RAM::Save_RAM(const std::string& save_file_path) 
    : save_data({ 0 }), path(save_file_path){
        if (std::filesystem::exists(path)) {
            std::cout << "USER INFO: Save game data found. Reading from save data\n";

            std::ifstream input_save_file(path, std::ios::in | std::ios::binary);
            input_save_file.read(reinterpret_cast<char*>(save_data.data()), 0x2000);

            input_save_file.close();
        } else
            std::cout << "USER INFO: No past game save data found\n";
    }

    Save_RAM::~Save_RAM() {
        std::ofstream output_save_file(path, std::ios::out | std::ios::binary);
        std::cout << "USER INFO: Saving game...\n";
        for (uint8_t num : save_data) {
            output_save_file << num;
        }
        output_save_file.close();
        std::cout << "USER INFO: Saving done!\n";
    }

    uint8_t Save_RAM::mapper_read(const uint16_t addr) {
        return save_data[addr % 0x2000];
    }

    void Save_RAM::mapper_write(const uint16_t addr, const uint8_t data) {
        save_data[addr % 0x2000] = data;
    }


}