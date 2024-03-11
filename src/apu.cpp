#include "../include/apu.h"

namespace roee_nes {
    APU::APU()
        : some_seq_flag({ 0 }), apu_cycle_counter(0), pulse_1({ 0 }), pulse_2({ 0 }) {
    }

    void APU::clock_pulse_envelope(Pulse_Channel* pulse) {
        /*
        if (start flag is clear) {
            clock divider
            if (divider is zero) {
                reload divider with V
                clock decay level counter
                if (decay level counter is not zero) {
                    decrement it
                }
                else if (loop flag is set) {
                    load decay level counter with 15
                }
            }
        }
        else {
            clear start flag
            load decay level counter with 15
            reload divider 
        }
        */
    }

    void APU::step_sequencer(uint8_t cycles) {
        if (some_seq_flag.comp.seq_mode == 1) { // 5-step mode
            switch (apu_cycle_counter) {
                case SEQ_5_STEP_1:
                    clock_pulse_envelope(&pulse_1);
                    clock_pulse_envelope(&pulse_2);
                    // clock envelope & triangle linear counter
                    break;
                case SEQ_5_STEP_2:
                    // clock envelope & triangle linear counter
                    // clock length counter & sweep units
                    break;
                case SEQ_5_STEP_3:
                    // clock envelope & triangle linear counter
                    break;
                case SEQ_5_STEP_4:
                    break;
                case SEQ_5_STEP_5_1:
                    // clock envelope & triangle linear counter
                    // clock length counter & sweep units
                    apu_cycle_counter = 0;
                    break;
                case SEQ_5_STEP_5_2:
                    break;
            }
        } else { // 4-step mode 
            switch (apu_cycle_counter) {
                case SEQ_4_STEP_1:
                    // clock envelope & triangle linear counter
                    break;
                case SEQ_4_STEP_2:
                    // clock envelope & triangle linear counter
                    // clock length counter & sweep units
                    break;
                case SEQ_4_STEP_3:
                    // clock envelope & triangle linear counter
                    break;
                case SEQ_4_STEP_4_1:
                    if (some_seq_flag.comp.inhibit_int == 0) {
                        // set frame interrupt flag
                    }
                    break;
                case SEQ_4_STEP_4_2:
                    // clock envelope & triangle linear counter
                    // clock length counter & sweep units
                    if (some_seq_flag.comp.inhibit_int == 0) {
                        // set frame interrupt flag
                    }
                    apu_cycle_counter = 0;
                    break;
                case SEQ_4_STEP_4_3:
                    if (some_seq_flag.comp.inhibit_int == 0) {
                        // set frame interrupt flag
                    }
                    break;
            }
        }
    }

    void APU::cpu_write_apu(uint8_t addr, uint8_t data) {

        auto handle_pulse_write = [](Pulse_Channel* pulse, uint8_t data) {
            pulse->duty_cycle = (data & 0b11000000) >> 6;
            pulse->length_counter_halt = (data & 0b00100000) >> 5;
            pulse->constant_volume = (data & 0b00010000) >> 4;
            pulse->volume_envelope = data & 0b00001111;
            };

        // please complete all the switch case until 0x401a
        switch (addr) {
            case 0x0: // pulse 1
                handle_pulse_write(&pulse_1, data);
                break;
            case 0x1:
                pulse_1.apu_sweep = data;
                break;
            case 0x2:
                pulse_1.timer = (pulse_1.timer & 0b111'0000'0000) | data;
                break;
            case 0x3:
                pulse_1.timer = (pulse_1.timer & 0b000'1111'1111) | (data << 8);
                pulse_1.length_counter_load = data >> 3;
                break;
            case 0x4: // pulse 2
                handle_pulse_write(&pulse_2, data);
                break;
            case 0x5:
                pulse_2.apu_sweep = data;
                break;
            case 0x6:
                pulse_2.timer = (pulse_2.timer & 0b111'0000'0000) | data;
                break;
            case 0x7:
                pulse_2.timer = (pulse_2.timer & 0b000'1111'1111) | (data << 8);
                pulse_2.length_counter_load = data >> 3;
                break;
            case 0x8:
                break;
            case 0x9:
                break;
            case 0xa:
                break;
            case 0xb:
                break;
            case 0xc:
                break;
            case 0xd:
                break;
            case 0xe:
                break;
            case 0xf:
                break;
            case 0x10:
                break;
            case 0x11:
                break;
            case 0x12:
                break;
            case 0x13:
                break;
            case 0x14:
                break;
            case 0x15:
                break;
            case 0x17:
                break;
            case 0x18:
            case 0x19:
            case 0x1a:
                // some rarely used apu functionality
                break;

        }
    }
}
