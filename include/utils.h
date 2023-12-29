#ifndef UTIL_H
#define UTIL_H

#include <cstdint>

void print_binary(uint8_t val);
void copyArray(uint8_t source[], uint8_t destination[], int size);

int get_sleep_time(uint8_t cycles);
uint16_t convert_to_2byte(uint8_t byte1, uint8_t byte2);

#endif
