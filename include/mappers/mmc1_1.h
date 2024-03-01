#ifndef MMC1_1_H
#define MMC1_1_H

#include <cstdint>

#include "../mapper_n_cart.h"

namespace roee_nes {
    class MMC1_1 : public Mapper {
        public:
        MMC1_1(Cartridge* cart);

        uint8_t cpu_read(uint16_t addr) override;
        void cpu_write(uint16_t addr, uint8_t data) override;
        uint8_t ppu_read(uint16_t addr) override;
        void ppu_write(uint16_t addr, uint8_t data) override;

        uint16_t get_nt_mirrored_addr(uint16_t addr) override;
        void reset();

        private:
        uint8_t shift;
        uint8_t load; // NOTE not sure i need this reg

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
            uint8_t bank_0 : 4;
            uint8_t bank_1 : 4;
        } chr_bank;

        uint8_t prg_bank;

    };
}

#endif