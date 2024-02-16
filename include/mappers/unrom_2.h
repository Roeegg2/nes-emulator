#ifndef UNROM_2_H
#define UNROM_2_H

#include <cstdint>

#include "../mapper_n_cart.h"

namespace roee_nes {
    class UNROM_2 : public Mapper {
        public:
        UNROM_2(Cartridge* cart);

        uint8_t cpu_read(uint16_t addr) override;
        void cpu_write(uint16_t addr, uint8_t data) override;
        uint8_t ppu_read(uint16_t addr) override;
        void ppu_write(uint16_t addr, uint8_t data) override;

        private:
        uint8_t prg_bank_select;
        uint8_t last_bank;
    };
}

#endif