#ifndef MMC1_1_H
#define MMC1_1_H

#include <cstdint>
#include <array>
#include "../mapper_n_cart.h"

namespace roee_nes {
    class MMC1_1 : public Mapper {
    public:
        MMC1_1(Cartridge* cart);

        uint8_t cpu_read(uint16_t addr, uint8_t open_bus_data) override;
        void cpu_write(uint16_t addr, uint8_t data) override;
        uint8_t ppu_read(uint16_t addr) override;
        void ppu_write(uint16_t addr, uint8_t data) override;
        uint16_t get_nt_mirrored_addr(const uint16_t addr) const override;
        void save() override;
        
        void reset();

    private:
        void update_chr(uint16_t addr);
        void update_prg(uint16_t addr);

    private:
        bool using_chr_ram;
        uint8_t shift_reg;
        std::vector<uint8_t>* chr_read_mem;
        std::array<uint8_t, 0x2000> save_data;
        std::string save_file_path;
        union {
            struct {
                uint8_t mirroring : 2;
                uint8_t prg_rom_mode : 2;
                uint8_t chr_rom_mode : 1;
                uint8_t : 3;
            } comp;
            uint8_t raw;
        } ctrl;
        struct {
            uint8_t bank_0 : 5;
            uint8_t : 3;
            uint16_t bank_1 : 5;
            uint8_t : 3;
        } chr_bank;
        struct {
            uint8_t bank : 4;
            uint8_t ext : 1;
            uint8_t : 3;
        } prg_bank;
        uint8_t prg_bank_num;
        uint8_t chr_bank_num;
        uint8_t final_bank;
        uint16_t final_addr;
    };
}

#endif