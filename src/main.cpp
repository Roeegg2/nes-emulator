#include "../include/bus.h"
#include "../include/cpu.h"
#include "../include/utils.h"

#include <string.h>
#include <iostream>

using namespace roee_nes;

int main(){
    uint8_t cycles;

    Bus bus = Bus(Mapper::create_mapper("testr/gameroms/DK.nes"));
    CPU cpu = CPU(&bus);
    PPU ppu = PPU(&bus);

    cpu.log();

    for (int i = 0; i < 50; i++) {
        cycles = cpu.run_cpu();
        ppu.run_ppu(cycles * 3);
    }

    return 0;
}