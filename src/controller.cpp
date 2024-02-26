#include "../include/controller.h"
#include <bitset>
#include <iostream>
#include <fstream>

namespace roee_nes {
    Controller::Controller()
        : buttons{0}, shift_reg{0}, internal_regs{0} {}

    void Controller::write(uint8_t data) {
        if ((internal_regs.strobe == data))
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

