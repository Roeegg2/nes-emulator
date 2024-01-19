#include "../include/bus.h"
#include "../include/cpu.h"
#include "../include/utils.h"
#include "../include/nes_screen.h"

using namespace roee_nes;

int main(){
    uint8_t cycles;

    Bus bus = Bus(Mapper::create_mapper("testr/gameroms/DK.nes"));
    CPU cpu = CPU(&bus);
    PPU ppu = PPU(&bus, new NES_Screen());

    cpu.log();

    while(true) {
        cycles = cpu.run_cpu();
        ppu.run_ppu(cycles * 3);

        if (ppu.nmi)
            cpu.nmi();
        
        std::cout << "scanline: " << (int)ppu.curr_scanline << " cycle: " << (int)ppu.curr_cycle << std::endl;
        
    }
    return 0;
}