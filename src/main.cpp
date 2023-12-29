#include "../include/cpu_bus.h"
#include "../include/cpu.h"
#include "../include/utils.h"

#include <string.h>
#include <iostream>

int main(){
    uint8_t temp[0xbfe0];

    CPU_Bus cpu_bus = CPU_Bus();
    cpu_bus.mapper = Mapper::create_mapper("testr/Donkey_Kong_Classics.nes");
    CPU cpu = CPU(&cpu_bus);
    // cpu.reset();
    
    for (int i = 0; i < 1000; i++){
        copyArray(cpu_bus.ram, temp, 0x800);

        cpu.fetch_decode_inst();
        cpu.execute_inst();

        cpu.print_state(temp);
    }

    return 0;
}