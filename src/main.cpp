#include "../include/bus.h"
#include "../include/cpu.h"
#include "../include/utils.h"
#include "../include/nes_screen.h"

using namespace roee_nes;

int main(){
    SDL_Event event;
    uint8_t cycles;

    Bus bus = Bus(Mapper::create_mapper("testr/gameroms/DK.nes"));
    CPU cpu = CPU(&bus);
    PPU ppu = PPU(&bus, new NES_Screen());
    
    while(true) {
        cycles = cpu.run_cpu();
        ppu.run_ppu(cycles * 3);

        if (ppu.nmi)
            cpu.nmi();
        
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            exit(1);
        
    }
    return 0;
}