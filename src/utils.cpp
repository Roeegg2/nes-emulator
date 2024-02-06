#include "../include/utils.h"

#include <iostream>

namespace roee_nes {

    constexpr float NTSC_CPU_CLOCKSPEED = 1.789773; // in nano seconds

    int get_sleep_time(uint8_t cycles) { // for now its in cpu cycles, but later it would be in ppu cycles since they are shorter.
        return (int)((cycles / NTSC_CPU_CLOCKSPEED) * 1000);
    }

    uint16_t convert_to_2byte(uint8_t low, uint8_t high) {
        return (high << 8) | low;
    }

    std::string get_binary(int val, uint8_t size){
        std::string str = "";

        for (uint8_t i = 0; i < size; i++){
            if ((val & 0b1) >= 1)
                str += '1';
            else
                str += '0';
            
            val = val >> 1;
        }
        
        return str;
    }
}