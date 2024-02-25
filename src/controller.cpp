#include "../include/controller.h"
#include <bitset>
#include <iostream>
#include <fstream>

namespace roee_nes {
    Controller::Controller()
        : out_1{0}, out_2{0}, in{0}, buttons{0}, shift_reg{0}, internal_regs{0} {}

    // uint8_t Controller::read() {
    //     ret &= 0b1111'1000;
    //     if (strobe == 1)
    //         return frame_controls & 0b0000'0001; // return the status of the a button
    //     else if (counter == 8)
    //         return 1; // return 1 if all bits were read
    //     else {
    //         ret |= (shift_reg & 0b0000'0001);
    //         shift_reg >>= 1;
    //         counter++;
    //     }

    //     return ret;
    // }

    // void Controller::write(uint8_t data) {
    //     if (strobe == data)
    //         return;

    //     strobe = data;
    //     counter = 0;
    //     if (strobe == 0)
    //         shift_reg = frame_controls;
    // }

    void Controller::write(uint8_t data) {
        if (internal_regs.strobe == data)
            return;
        
        internal_regs.strobe = data & 0x01;

        if (internal_regs.strobe == 1) {
            shift_reg.raw = buttons.raw;
            internal_regs.counter = 0;
            return;
        }
    }

    uint8_t Controller::read() {
        if (internal_regs.strobe == 1)
            return buttons.comp.a;
        
        if (internal_regs.counter == 8)
            return 1;
        
        uint8_t ret = shift_reg.raw & 0x01;
        internal_regs.counter++;
        shift_reg.raw >>= 1;

        return ret;
    }
}

