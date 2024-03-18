#include "../include/bus.h"
#include "../include/cpu.h"
#include "../include/utils.h"
#include "../include/nes_screen.h"
#include <chrono>
#include <iostream>

using namespace roee_nes;

uint16_t emulator_tick(Bus* bus) {
    uint8_t cycles = bus->cpu->run_cpu();
    bus->ppu->run_ppu(cycles * 3);
    bus->apu->run_apu();

    if (bus->mapper->set_irq) {
        bus->cpu->irq();
        bus->mapper->set_irq = false;
    }

    if (bus->ppu->nmi == 1) {
        bus->ppu->nmi = 0;
        bus->cpu->nmi();
    }

    return bus->cpu->PC;
}

int main() {
    const std::string rom_path = "roms/SMB2.nes";
    const std::string palette_path = "ntscpalette.pal";

    Controller* const controller_1 = new Controller();
    Controller* const controller_2 = new Controller();
    Mapper* const mapper = Mapper::create_mapper(rom_path);
    NES_Screen* const screen = new NES_Screen(mapper, controller_1, controller_2);
    CPU* const cpu = new CPU();
    PPU* const ppu = new PPU(screen);
    APU* const apu = new APU();
    Bus* const bus = new Bus(cpu, ppu, apu, mapper, controller_1, controller_2, palette_path);

    uint8_t val = 0;
    while (true) {
        if (val % 100 == 0)
            screen->handle_events();
        val++;

        emulator_tick(bus);
#ifdef DEBUG
        bus->full_log();
#endif
    }

    return 0;
}