#ifndef MMC3_4_H
#define MMC3_4_H

#include "../cpu.h"
#include "../mapper_n_cart.h"
#include <array>

namespace roee_nes {
union Bank_Select {
  struct {
    struct {
      uint8_t select : 3;
      uint8_t : 2; // unused
      uint8_t : 1; // unused on MMC3
      uint8_t prg_rom_bank_mode : 1;
      uint8_t chr_a12_inversion : 1;
    } even;
    struct {
      uint8_t bank_data;
    } odd;
  } comp;

  uint16_t raw;
};

class MMC3_4 : public Mapper {
public:
  MMC3_4(Cartridge *cart);

  uint8_t cpu_read(uint16_t addr, uint8_t open_bus_data) override;
  void cpu_write(uint16_t addr, uint8_t data) override;
  uint8_t ppu_read(uint16_t addr) override;
  void ppu_write(uint16_t addr, uint8_t data) override;
  uint16_t get_nt_mirrored_addr(const uint16_t addr) const override;
  void clock_irq();
  void save() override;

private:
  void update_chr(uint16_t addr);
  void update_prg(uint16_t addr);

private:
  bool using_chr_ram;
  std::array<uint8_t, 0x2000> save_data;
  std::vector<uint8_t> *chr_read_mem;
  union Bank_Select bank_select;
  uint8_t mirroring;
  uint8_t prg_ram_protect;

  std::array<uint8_t, 2> prg_bank;
  std::array<uint8_t, 6> chr_bank; // 2 * 2kb + 4 * 1kb
  Save_RAM *save_ram;

  uint8_t prg_bank_num;
  uint8_t chr_bank_num;

  uint8_t final_bank;
  uint16_t final_addr;

public:
  uint8_t irq_counter;
  uint8_t irq_latch;
  bool irq_reload;
  bool irq_enabled;
};
} // namespace roee_nes

#endif
