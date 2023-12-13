#include "../include/cpu_bus.h"
#include "../include/cpu.h"
#include "../include/utils.h"

#include <string.h>
#include <iostream>

int main(){
    uint8_t temp[0xbfe0];

    CPU_Bus bus = CPU_Bus();
    bus.mapper = Mapper::create_mapper("testr/Donkey_Kong_Classics.nes");
    CPU cpu = CPU(&bus);
    cpu.reset();

    while (true){
        copyArray(bus.ram, temp, 0x800);

        cpu.fetch_decode_inst();
        cpu.execute_inst();

        cpu.print_state(temp);
    }

    return 0;
}