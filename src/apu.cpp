#include "../include/apu.h"
#include <fstream>

namespace roee_nes {
    APU::APU() {

    }

    void APU::run_apu() {
        // clock frame counter
        triangle.clock_timer();
        // clock pulse channels
        // clock noise channel
        // clock DMC channel
        clock_frame_counter();
    }

    void APU::mix_audio() {
        static std::ofstream file("audio.raw", std::ios::binary);
        file << "triangle output: " << triangle.get_output() << "\n";
    }

    uint8_t Triangle_Channel::get_output() const {
        // std::cout << "output: " << (int)waveform_seq[seq_index] << "\n";
        if ((length_counter == 0) || (linear_counter == 0))
            return 0;
        else
            return waveform_seq[seq_index];
    }

    void Triangle_Channel::clock_timer() {
        if (timer == 0) {
            timer = (timer_reload + 1) & 0b0000'0111'1111'1111; // NOTE not sure about the +1
            if ((linear_counter > 0) && (length_counter > 0))
                seq_index = (seq_index + 1) % 32;

            // static std::ofstream file("audio.raw", std::ios::binary);
            // file << "triangle output: " << (int)get_output() << "\n";
            // clock waveform generator
        } else {
            timer--;
        }
    }

    void Triangle_Channel::clock_linear_counter() {
        if (linear_counter_reload_flag == 1) {
            linear_counter = linear_counter_reload;
        } else if (linear_counter != 0) {
            linear_counter--;
        }
        if (ctrl_halt == 0) {
            linear_counter_reload_flag = 0;
        }
    }

    void Triangle_Channel::clock_length_counter() {
        if (ctrl_halt == 0 && length_counter > 0) {
            length_counter--;
        }
    }

    void APU::clock_frame_counter() {
        if (frame_counter.mode == 1) { // 5-step mode
            switch (frame_counter.counter) {
                case SEQ_5_STEP_1:
                    // clock envelope & triangle linear counter
                    triangle.clock_linear_counter();
                    break;
                case SEQ_5_STEP_2:
                    // clock envelope & triangle linear counter
                    // clock length counter & sweep units
                    triangle.clock_linear_counter();
                    triangle.clock_length_counter();
                    break;
                case SEQ_5_STEP_3:
                    // clock envelope & triangle linear counter
                    triangle.clock_linear_counter();
                    break;
                case SEQ_5_STEP_4:
                    break;
                case SEQ_5_STEP_5_1:
                    // clock envelope & triangle linear counter
                    // clock length counter & sweep units
                    triangle.clock_linear_counter();
                    triangle.clock_length_counter();
                    break;
                case SEQ_5_STEP_5_2:
                    frame_counter.counter = -1; // setting to -1 will make it 0 in the next line
                    break;
            }
        } else { // 4-step mode 
            switch (frame_counter.counter) {
                case SEQ_4_STEP_1:
                    // clock envelope & triangle linear counter
                    triangle.clock_linear_counter();
                    break;
                case SEQ_4_STEP_2:
                    // clock envelope & triangle linear counter
                    // clock length counter & sweep units
                    triangle.clock_linear_counter();
                    triangle.clock_length_counter();
                    break;
                case SEQ_4_STEP_3:
                    // clock envelope & triangle linear counter
                    triangle.clock_linear_counter();
                    break;
                case SEQ_4_STEP_4_1:
                    if (frame_counter.inhibit_frame_interrupt == 0)
                        frame_counter.frame_interrupt = 1;

                    break;
                case SEQ_4_STEP_4_2:
                    // clock envelope & triangle linear counter
                    // clock length counter & sweep units
                    triangle.clock_linear_counter();
                    triangle.clock_length_counter();
                    if (frame_counter.inhibit_frame_interrupt == 0)
                        frame_counter.frame_interrupt = 1;

                    break;
                case SEQ_4_STEP_4_3:
                    if (frame_counter.inhibit_frame_interrupt == 0)
                        frame_counter.frame_interrupt = 1;

                    frame_counter.counter = -1; // setting to -1 will make it 0 in the next line
                    break;
            }
        }
        frame_counter.counter++;
    }

    void APU::cpu_write_apu(const uint8_t addr, const uint8_t data) {
        switch (addr) {
            case 0x0: // pulse 1
                break;
            case 0x1:
                break;
            case 0x2:
                break;
            case 0x3:
                break;
            case 0x4:
                break;
            case 0x5:
                break;
            case 0x6:
                break;
            case 0x7:
                break;
            case 0x8:
                triangle.ctrl_halt = data >> 7;
                triangle.linear_counter_reload = data & 0b0111'1111;
                break;
            case 0x9:
                // unused
                break;
            case 0xa:
                triangle.timer_reload = (triangle.timer_reload & 0b111'0000'0000) | data;
                break;
            case 0xb:
                triangle.timer_reload = (triangle.timer_reload & 0b000'1111'1111) | ((data & 0b111) << 8);
                triangle.linear_counter_reload_flag = 1;
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
                status_reg = data;
                // enable the channels flags

                triangle.ctrl_halt = 0; // NOTE is this correct? enable triangle length counter
                break;
            case 0x17:
                // reset frame counter divider
                // reset frame counter sequencer
                // configure sequencer mode

                // * If the write occurs during an APU cycle, the effects occur 3 CPU cycles after the $4017 write cycle, and if the write occurs between APU cycles, the effects occurs 4 CPU cycles after the write cycle. 
                break;
            case 0x18:
            case 0x19:
            case 0x1a:
                // some rarely used apu functionality
                break;

        }
    }
}
