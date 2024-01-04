#include <iostream>

#include "../include/utils.h"

constexpr _Float16 NTSC_CPU_CLOCKSPEED = 1.789773; // in nano seconds

int get_sleep_time(uint8_t cycles) { // for now its in cpu cycles, but later it would be in ppu cycles since they are shorter.
    return (int)((cycles/NTSC_CPU_CLOCKSPEED)*1000);
}

uint16_t convert_to_2byte(uint8_t low, uint8_t high){
    return (high << 8) | low;
}

void print_binary(uint8_t val) {
    char binary[9]; // Use 9 to accommodate the null terminator

    // Start from the least significant bit
    for (int i = 7; i >= 0; i--) {
        binary[7 - i] = '0' + ((val >> i) & 1);
    }

    binary[8] = '\0'; // Null-terminate the string

    std::cout << "P:" << binary << std::endl;
}