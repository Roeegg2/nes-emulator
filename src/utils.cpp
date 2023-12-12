#ifndef UTIL_H
#define UTIL_H

#include "../include/utils.h"

#include <time.h>
#include <iostream>

void sleep_ns(long long ns){
    struct timespec req, rem;

    req.tv_sec = ns / 1000000000;
    req.tv_nsec = ns % 1000000000;

    while (nanosleep(&req, &rem) == -1)
        req = rem;
}

uint16_t convert_to_2byte(uint8_t byte1, uint8_t byte2){
    return (byte1 << 8) | byte2;
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

void copyArray(uint8_t source[], uint8_t destination[], int size) {
    for (int i = 0; i < size; i++) {
        destination[i] = source[i];
    }
}

#endif