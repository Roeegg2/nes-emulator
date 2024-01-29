#ifndef MAPPER_H
#define MAPPER_H

#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>

namespace roee_nes {

    constexpr uint16_t KILOBYTE = 1024;

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
        Cartridge(const std::string* rom_path);

    private:
        Cartridge_Header header;

        std::vector<uint8_t> prg_rom;
        std::vector<uint8_t> chr_rom;
        std::vector<uint8_t> prg_ram;

        std::vector<uint8_t> nt_vram;
    };

    class Mapper {
    public:
        Mapper(Cartridge* cart) : cart(cart) {}
        
        static Mapper* create_mapper(const std::string* rom_path);

        virtual uint8_t cpu_read(uint16_t addr) = 0;
        virtual void cpu_write(uint16_t addr, uint8_t data) = 0;
        virtual uint8_t ppu_read(uint16_t addr) = 0;

        inline char Get_mirroring() { return (cart->header.flags_6 & 0b00000001) ? 'V' : 'H'; } // NOTE: might need to rewrite this when implementing more mappers

    protected:
        Cartridge* cart;
    };

}
#endif