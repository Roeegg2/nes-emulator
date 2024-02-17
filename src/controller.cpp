#include "../include/controller.h"
#include <bitset>
#include <iostream>
#include <fstream>

namespace roee_nes {
    Controller::Controller()
        : strobe(0), frame_controls(0), shift_reg(0), counter(0), ret(0) {}

    uint8_t Controller::read() {
        ret &= 0b1111'1000;
        if (strobe == 1)
            return frame_controls & 0b0000'0001; // return the status of the a button
        else if (counter == 8)
            return 1; // return 1 if all bits were read
        else {
            ret |= (shift_reg & 0b0000'0001);
            shift_reg >>= 1;
            counter++;
        }

        return ret;
    }

    void Controller::write(uint8_t data) {
        if (strobe == data)
            return;

        strobe = data;
        counter = 0;
        if (strobe == 0)
            shift_reg = frame_controls;
    }
}
