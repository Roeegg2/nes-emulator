#include "../include/bus.h"
#include "../include/cpu.h"
#include "../include/utils.h"

#include <string.h>
#include <iostream>

int main(){
    uint8_t temp[0xbfe0];

    Bus bus = Bus();
    bus.mapper = Mapper::create_mapper("testr/gameroms/Donkey Kong (U) (PRG1) [!p](1).nes");
    CPU cpu = CPU(&bus);

    for (int i = 0; i < 50; i++){
        cpu.log();

        cpu.fetch_decode_inst();
        cpu.execute_inst();

        /* if (i == 10)
            cpu.nmi(); */
    }

    return 0;
}