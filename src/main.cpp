#include "../include/bus.h"
#include "../include/cpu.h"
#include "../include/utils.h"

#include <string.h>
#include <iostream>

int main(){
    uint8_t temp[0xbfe0];

    Bus bus = Bus();
    CPU cpu = CPU(&bus);

    for (auto i = 0; i < 100; i++){
        copyArray(bus.ram, temp, 0x800);
        cpu.IR = cpu.fetch(0);
        cpu.decode_inst();
        cpu.execute_inst();

        cpu.print_state(temp);
    }

    return 0;
}