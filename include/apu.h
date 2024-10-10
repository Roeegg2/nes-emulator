#ifndef APU_H
#define APU_H

#include <cstdint>

namespace roee_nes {
class APU {
public:
  void clock_frame_counter(void);
  void cpu_write_apu(const uint16_t addr, const uint8_t data);

private:
  struct {
    uint32_t cycle_counter; // IN CPU CYCLES!!!
    uint8_t mode : 1;
    uint8_t irq_inhibit : 1;
    uint8_t frame_interrupt_flag : 1;
    uint8_t post_4017_write : 1;
  } frame_counter;
};

} // namespace roee_nes

#endif
