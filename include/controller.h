#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <cstdint>

namespace roee_nes {
    class Controller {
    friend class NES_Screen;
    private:
        uint8_t strobe;
        uint8_t frame_controls;
        uint8_t shift_reg;
        uint8_t counter;
        uint8_t ret;
        
    public:
        void write(uint8_t data);
        uint8_t read();

        Controller();
    };
}

#endif

