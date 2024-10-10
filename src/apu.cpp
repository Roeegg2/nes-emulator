
#include "../include/apu.h"

namespace roee_nes {

void APU::cpu_write_apu(const uint16_t addr, const uint8_t data) {
  switch (addr) {
  case 0x4017:
    frame_counter.mode = (data >> 7) & 0x01;
    frame_counter.irq_inhibit = (data >> 6) & 0x01;
    frame_counter.frame_interrupt_flag = 0;
    frame_counter.cycle_counter = 0;
    break;
  }
}

void APU::clock_frame_counter(void) {
  if (frame_counter.post_4017_write == 1) {
    frame_counter.post_4017_write = 0;
    if (frame_counter.mode == 1) {
      // envelopes
      // triangle linear counter
      // length counters
      // sweep units
    }
    frame_counter.cycle_counter = 0;
  }

  if (frame_counter.mode == 0) { // 4-step sequence
    switch (frame_counter.cycle_counter) {
    case 7457: // 3728.5
               // quarter frame
      break;
    case 14913: // 7456.5
                // quarter frame
                // half frame
      break;
    case 22371: // 11185.5
                // quarter frame
      break;
    case 29828: // 14914.5
                // quarter frame
                // half frame
      __attribute__((fallthrough));
    case 0:
      if (frame_counter.irq_inhibit == 0) {
        frame_counter.frame_interrupt_flag = 1;
      }
      break;
    case 29829:
      if (frame_counter.irq_inhibit == 0) {
        frame_counter.frame_interrupt_flag = 1;
      }
      frame_counter.cycle_counter = 0;
      return;
    }
  } else { // 5-step sequence
    switch (frame_counter.cycle_counter) {
    case 7457: // 3728.5
               // quarter frame
      break;
    case 14913: // 7456.5
                // quarter frame
                // half frame
      break;
    case 22371: // 11185.5
                // quarter frame
      break;
    case 29829: // 14914.5
      break;
    case 37281: // 18640.5
                // quarter frame
                // half frame
      frame_counter.cycle_counter = 0;
      return;
    case 0:
      break;
    }
  }

  frame_counter.cycle_counter++;
}
} // namespace roee_nes
