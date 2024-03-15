#ifndef APU_H
#define APU_H

#include <cstdint>
#include <array>
#include <iostream>
#include <SDL2/SDL.h>

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

    enum Seq_Mode_5 : uint16_t {
        SEQ_5_STEP_1 = 7457, // 3728.5
        SEQ_5_STEP_2 = 14913, // 7456.5
        SEQ_5_STEP_3 = 22371, // 11185.5
        SEQ_5_STEP_4 = 29829, // 14914.5
        SEQ_5_STEP_5_1 = 37281, // 18640.5
        SEQ_5_STEP_5_2 = 0, // 18641
    };

    class Triangle_Channel {
        public:
        uint16_t timer;
        uint8_t length_counter;
        uint8_t linear_counter;
        uint8_t linear_counter_reload_flag;
        uint8_t ctrl_halt; // length counter halt/linear counter control
        uint8_t linear_counter_reload;
        uint16_t timer_reload;
        uint8_t seq_index;

        const std::array<uint8_t, 32> waveform_seq = { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

        public:
        void clock_length_counter();
        void clock_linear_counter();
        void clock_timer();
        uint8_t get_output() const;

        // double get_tnd_out() {
        //     // std::cout << "output: " << (int)get_output() << "\n";
        //     return (159.79 / ((1 / (get_output() / 8227) + (0 / 12241) + (0 / 22638)) + 100));
        // }
    };

    class APU {
        public:
        Triangle_Channel triangle;

        struct {
            uint16_t counter;
            uint8_t mode : 1;
            uint8_t inhibit_frame_interrupt : 1;
            uint8_t frame_interrupt : 1;
            uint8_t : 5;
        } frame_counter;


        uint8_t status_reg;

        public:
        APU();
        void clock_frame_counter();
        void cpu_write_apu(uint8_t addr, uint8_t data);
        void run_apu();
        void mix_audio();

        // double get_tnd_out() {
        //     return (159.79 / ((1 / (triangle.get_output() / 8227) + (0 / 12241) + (0 / 22638)) + 100));
        // }
    };
}

#endif

/**
 * some important notes for later:
 *
 * 1. frame counter: If the write occurs during an APU cycle, the effects occur 3 CPU cycles
 *    after the $4017 write cycle, and if the write occurs between APU cycles, the effects occurs
 *    4 CPU cycles after the write cycle.
 *
 * 2. triangle channel: At the lowest two periods ($400B = 0 and $400A = 0 or 1), the resulting
 *    frequency is so high that the DAC effectively outputs a value half way between 7 and 8.
 *
 *
 *
*/