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

    class Cartridge {
        friend class Mapper;
        friend class NROM_0;
        friend class CNROM_3;
        friend class UNROM_2;
        public:
        Cartridge(const std::string* rom_path);

        private:
        struct Cartridge_Header {
            uint16_t total_size; // in kb

            uint8_t prg_rom_size;
            uint8_t chr_rom_size;
            // uint8_t chr_ram_size;
            union {
                struct {
                    uint8_t nt_layout : 1; // 1 if horizontal, 0 if vertical
                    uint8_t prg_ram : 1; // 1 if present, 0 if not
                    uint8_t trainer : 1; // 1 if present, 0 if not
                    uint8_t alt_nt_layout : 1;
                    uint8_t mapper_num_low : 4;
                } parsed;
                uint8_t raw;
            } flag_6;

            union {
                struct {
                    uint8_t uu_vsunisystem : 1;
                    uint8_t uu_playchoice : 1;
                    uint8_t uu_ines2_sig : 2;
                    uint8_t mapper_num_high : 4;
                } parsed;
                uint8_t raw;
            } flag_7;

            struct {
                uint8_t prg_ram_size;
            } flag_8;

            struct {
                uint8_t tv_system;
            } flag_9;

            union {
                struct {
                    uint8_t tv_system : 2;
                    uint8_t : 2;
                    uint8_t prg_ram : 1;
                    uint8_t bus_conflicts : 1;
                } parsed;
                uint8_t raw;
            } flag_10;

            // TODO add other stuff later if i add support for iNES 2.0
        } header;

        std::vector<uint8_t> prg_rom;
        std::vector<uint8_t> chr_rom;
        std::vector<uint8_t> prg_ram;
        std::vector<uint8_t> chr_ram;

        std::vector<uint8_t> trainer;
        // TODO make all writes/reads to nt go through the cartridge
    };

    class Mapper {
        public:
        Mapper(Cartridge* cart) : cart(cart) {}
        static Mapper* create_mapper(const std::string* rom_path);

        virtual uint8_t cpu_read(uint16_t addr) = 0;
        virtual void cpu_write(uint16_t addr, uint8_t data) = 0;
        virtual uint8_t ppu_read(uint16_t addr) = 0;
        virtual void ppu_write(uint16_t addr, uint8_t data) = 0;
        
        virtual uint16_t get_nt_mirrored_addr(uint16_t addr);

        protected:
        Cartridge* cart;
    };

}
#endif