#ifndef UTIL_H
#define UTIL_H

#include <cstdint>
namespace roee_nes {
    int get_sleep_time(const uint8_t cycles); 
    uint16_t convert_to_2byte(const uint8_t low, const uint8_t high);
}
#endif
