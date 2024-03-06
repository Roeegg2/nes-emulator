#ifndef CNROM_3_H
#define CNROM_3_H

#include <cstdint>

#include "../mapper_n_cart.h"

namespace roee_nes { 
    class CNROM_3 : public Mapper {
    public:
        CNROM_3(Cartridge* cart);

        uint8_t cpu_read(uint16_t addr, uint8_t open_bus_data) override;
        void cpu_write(uint16_t addr, uint8_t data) override;
        uint8_t ppu_read(uint16_t addr) override;
        void ppu_write(uint16_t addr, uint8_t data) override;
    
    private:
        uint8_t chr_bank_select;
    };
}

#endif