#include "../include/bus.h"
#include "../include/cpu.h"
#include "../include/utils.h"

#include <string.h>
#include <iostream>

int main(){
    uint8_t temp[0xbfe0];

    Bus bus = Bus(Mapper::create_mapper("testr/gameroms/DK.nes"));
    CPU cpu = CPU(&bus);

    for (int i = 0; i < 50; i++){
        cpu.log();

        cpu.fetch_decode_inst();
        cpu.execute_inst();

        if (i == 10)
            cpu.nmi();
    }

    return 0;
}