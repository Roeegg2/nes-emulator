#ifndef NROM_0_H
#define NROM_0_H

#include "../mapper_n_cart.h"

namespace roee_nes {
class NROM_0 : public Mapper {
public:
  NROM_0(Cartridge *cart);

  uint8_t cpu_read(uint16_t addr, uint8_t open_bus_data) override;
  void cpu_write(uint16_t addr, uint8_t data) override;
  uint8_t ppu_read(uint16_t addr) override;
  void ppu_write(uint16_t addr, uint8_t data) override;
};
} // namespace roee_nes
#endif
