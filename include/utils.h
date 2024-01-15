#ifndef UTIL_H
#define UTIL_H

#include <cstdint>

namespace roee_nes {

    int get_sleep_time(uint8_t cycles);
    uint16_t convert_to_2byte(uint8_t low, uint8_t high);
}
#endif
