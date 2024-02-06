#include "../include/bus.h"
#include "../include/cpu.h"
#include "../include/utils.h"
#include "../include/nes_screen.h"

using namespace roee_nes;

// extern void debugger_run(std::array<std::array<uint8_t, 0x400>, 4>* nt_vram);

Bus* bus;
CPU* cpu;
PPU* ppu;

uint16_t emulator_tick() {
    SDL_Event event;
    uint8_t cycles;

    cycles = cpu->run_cpu();
    ppu->run_ppu(cycles * 3);

    if (ppu->nmi == 1) {
        ppu->nmi = 0;
        cpu->nmi();
    }

    if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
        exit(1);

    return cpu->PC;
}

int main() {
    const std::string rom_path = "roms/dk3.nes";
    const std::string palette_path = "ntscpalette.pal";

    bus = new Bus(Mapper::create_mapper(&rom_path), &palette_path);
    cpu = new CPU(bus);
    ppu = new PPU(bus, new NES_Screen());
    bus->cpu = cpu;
    bus->ppu = ppu;

    ppu->reset();
    cpu->reset();

    while (1) {
        emulator_tick();
    }

    // bus->find_difference();
    
    return 0;
}