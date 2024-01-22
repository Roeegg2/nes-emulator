#include "../include/bus.h"
#include "../include/cpu.h"
#include "../include/utils.h"
#include "../include/nes_screen.h"

using namespace roee_nes;

void print_debugger() {
    // printf("------------- DEBUGGER STARTED --------------\n\n");

    // printf("COMMANDS:\n");
    // printf("c/run - continue\n");
    // printf("s - step\n");
    // printf("b <addr> - add breakpoint at <addr>\n");
    // printf("pc - print cpu registers (NOTE: not implemented yet)\n");
    // printf("pp - print ppu registers (NOTE: not implemented yet)\n");
    // printf("pm <addr> - print memory at <addr> (NOTE: not implemented yet)\n");
    // printf("q - quit\n");
}

int run_emulator(CPU* cpu, PPU* ppu) {
    SDL_Event event;
    uint8_t cycles;

    cycles = cpu->run_cpu();
    ppu->run_ppu(cycles * 3);

    if (ppu->ext_regs.ppustatus & 0b10000000) {
        ppu->ext_regs.ppustatus &= 0b01111111;
        cpu->nmi();
        ppu->nmi = 0;
    }

    if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
        exit(1);

    return 0;
}

int main() {
    Bus bus = Bus(Mapper::create_mapper("testr/gameroms/DK.nes"), "testr/gameroms/palette.pal");
    CPU cpu = CPU(&bus);
    PPU ppu = PPU(&bus, new NES_Screen());
    bus.cpu = &cpu;
    bus.ppu = &ppu;

    ppu.reset();
    cpu.reset();

    while (1) {
        std::cout << "\n>>";
        char command = getchar();

        switch (command) {
            case 'c':
                while (1){
                    run_emulator(&cpu, &ppu);
                    bus.log();
                }
                break;

            case 's':
                run_emulator(&cpu, &ppu);
                bus.log();
                break;

            case 'q':
                return 0;
        }
    }

    return 0;
}