#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <cstdint>

namespace roee_nes {

    typedef union {
        struct {
            uint8_t a : 1;
            uint8_t b : 1;
            uint8_t select : 1;
            uint8_t start : 1;
            uint8_t up : 1;
            uint8_t down : 1;
            uint8_t left : 1;
            uint8_t right : 1;
        } buttons;

        uint8_t raw;
    } controller_reg;


    struct Controller {
        uint8_t ret_buffer; // NOTE: not sure if this should be here or on the bus

        controller_reg live_status_reg;
        controller_reg shift_reg;
    };
}

#endif

