#ifndef MAPPER_H
#define MAPPER_H

#define KILOBYTE 1024

#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>

#include "../include/mappers/nrom_0.h"

namespace roee_nes {

    struct Cartridge_Header {
        uint16_t total_size; // in kb

        uint8_t prg_bank_size;
        uint8_t chr_bank_size;

        uint8_t prg_ram_size;

        uint8_t flags_6;
        uint8_t flags_7;
        uint8_t flags_8;
        uint8_t flags_9;
        uint8_t flags_10;

        // might add other stuff later if i add support for iNES 2.0
    };

    class Cartridge {
        friend class Mapper;
        friend class NROM_0;

    public:
        Cartridge(std::string rom_path);

    private:
        Cartridge_Header header;

        std::vector<uint8_t> chr_rom;
        std::vector<uint8_t> prg_rom;
        std::vector<uint8_t> prg_ram;

        std::vector<uint8_t> vram;
    };

    class Mapper {
    public:
        virtual uint8_t cpu_read(uint16_t addr) = 0;
        virtual void cpu_write(uint16_t addr, uint8_t data) = 0;

        virtual uint8_t ppu_read(uint16_t addr) = 0;

        static Mapper* create_mapper(std::string rom_path);

    protected:
        Cartridge* cart;
    };

}

#endif