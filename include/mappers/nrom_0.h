#ifndef NROM_0_H
#define NROM_0_H
#include "../mapper_n_cart.h"

class NROM_0 : public Mapper{
public:
    uint8_t cpu_read(uint16_t addr) override;
    void cpu_write(uint16_t addr, uint8_t data) override;

    NROM_0(Cartridge* cart);
};

#endif