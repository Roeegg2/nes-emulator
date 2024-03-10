#include "../include/apu.h"

namespace roee_nes {
    APU::APU()
        : some_seq_flag({ 0 }), apu_cycle_counter(0) {

    }

    void APU::step_sequencer(uint8_t cycles) {
        if (some_seq_flag.comp.seq_mode == 1) { // 5-step mode
            switch (apu_cycle_counter) {
                case SEQ_5_STEP_1:
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
                    // case SEQ_4_STEP_4_1:
                    //     break;
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
}
