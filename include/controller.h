#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <cstdint>

namespace roee_nes {

    struct Output_Reg {
        uint8_t val : 3;
        uint8_t : 5;
    };

    struct Input_Reg {
        uint8_t val : 5;
        uint8_t : 3;
    };

    union Buttons {
        struct {
            uint8_t a : 1;
            uint8_t b : 1;
            uint8_t select : 1;
            uint8_t start : 1;
            uint8_t up : 1;
            uint8_t down : 1;
            uint8_t left : 1;
            uint8_t right : 1;
        } comp;
        uint8_t raw;
    };

    class Controller {
        friend class NES_Screen;
        public:
        // union {
        //     struct {
        //         uint8_t a : 1;
        //         uint8_t b : 1;
        //         uint8_t select : 1;
        //         uint8_t start : 1;
        //         uint8_t up : 1;
        //         uint8_t down : 1;
        //         uint8_t left : 1;
        //         uint8_t right : 1;
        //     } comp;
        //     uint8_t raw;
        // } controller_status;
        // uint8_t strobe;
        // uint8_t frame_controls;
        // uint8_t shift_reg;
        // uint8_t counter;
        // uint8_t ret;

        struct Output_Reg out_1;
        struct Output_Reg out_2;
        struct Input_Reg in;

        union Buttons buttons;
        union Buttons shift_reg;

        struct {
            uint8_t strobe : 1;
            uint8_t counter : 3;
            uint8_t : 4;
        } internal_regs;

        public:
        void write(uint8_t data);
        uint8_t read();

        Controller();
    };
};

#endif

