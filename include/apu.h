#ifndef APU_H
#define APU_H

#include <cstdint>

namespace roee_nes {

    // measured in CPU cycles!
    enum Seq_Mode_4 : uint16_t {
        SEQ_4_STEP_1 = 7457, // 3728.5
        SEQ_4_STEP_2 = 14913, // 7456.5
        SEQ_4_STEP_3 = 22371, // 11185.5
        SEQ_4_STEP_4_1 = 29828, // 14914
        SEQ_4_STEP_4_2 = 29829, // 14914.5
        SEQ_4_STEP_4_3 = 0, // 14915
    };

    enum Seq_Mode_4 : uint16_t {
        SEQ_5_STEP_1 = 7457, // 3728.5
        SEQ_5_STEP_2 = 14913, // 7456.5
        SEQ_5_STEP_3 = 22371, // 11185.5
        SEQ_5_STEP_4 = 29829, // 14914.5
        SEQ_5_STEP_5_1 = 37281, // 18640.5
        SEQ_5_STEP_5_2 = 0, // 18641
    };

    struct Pulse_Channel {
        uint8_t volume_envelope : 4;
        uint8_t constant_volume : 1;
        uint8_t length_counter_halt : 1;
        uint8_t duty_cycle : 2;

        uint8_t apu_sweep;
        
        uint16_t timer : 11; // low + high
        uint16_t length_counter_load : 5;
        // uint8_t timer_low;
        // uint8_t timer_high : 3;
    };


    class APU {
        public:
        // APU pulse components
        Pulse_Channel pulse_1;
        Pulse_Channel pulse_2;


        union {
            struct {
                uint8_t seq_mode : 1; // sequencer mode
                uint8_t inhibit_int : 1; // inhibit IRQ interrupt
                uint8_t : 6;
            } comp;
            uint8_t raw;
        } some_seq_flag;

        uint16_t apu_cycle_counter;

        public:
        void step_sequencer(uint8_t cycles);
        void cpu_write_apu(uint8_t addr, uint8_t data);
        APU();
    };
}

#endif

/**
 * some important notes for later:
 *
 * 1. frame counter: If the write occurs during an APU cycle, the effects occur 3 CPU cycles
 * after the $4017 write cycle, and if the write occurs between APU cycles, the effects occurs
 * 4 CPU cycles after the write cycle.
*/