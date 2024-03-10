#ifndef APU_H
#define APU_H

#include <cstdint>

namespace roee_nes {
    // NOTE: i rounded these values down. shouldnt be an issue, since nothing happens during half a cpu cycle that could affect sound. just means sound will be one and a half ppu cycle earlier.
    // TODO move these to enums maybe 
    constexpr uint16_t SEQ_4_STEP_1 = 3728;
    constexpr uint16_t SEQ_4_STEP_2 = 7456;
    constexpr uint16_t SEQ_4_STEP_3 = 11185;
    constexpr uint16_t SEQ_4_STEP_4_1 = 14914;
    constexpr uint16_t SEQ_4_STEP_4_2 = 14914; // was 14914.5
    constexpr uint16_t SEQ_4_STEP_4_3 = 0; // 14915

    constexpr uint16_t SEQ_5_STEP_1 = 3728;
    constexpr uint16_t SEQ_5_STEP_2 = 7456;
    constexpr uint16_t SEQ_5_STEP_3 = 11185;
    constexpr uint16_t SEQ_5_STEP_4 = 14914;
    constexpr uint16_t SEQ_5_STEP_5_1 = 18640;
    constexpr uint16_t SEQ_5_STEP_5_2 = 0; // 18641

    class APU {
        public:
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
        APU();
    };
}

#endif