#include "../include/bus.h"
#include "../include/cpu.h"
#include "../include/utils.h"
#include "../include/nes_screen.h"
#include <chrono>

using namespace roee_nes;

// extern "C" void debugger_run(std::array<std::array<uint8_t, 0x400>, 4>* nt_vram);

uint16_t emulator_tick(Bus* bus, NES_Screen* screen, uint8_t val) {
    uint8_t cycles;
    if (val % 100 == 0)
        screen->handle_events();

    if (bus->cpu_sleep_dma_counter == 0)
        cycles = bus->cpu->run_cpu();
    else {
        bus->cpu_sleep_dma_counter--;
        cycles = 2; // takes 2 cycles to transfer one byte
    }
    
    bus->ppu->run_ppu(cycles * 3);

    if (bus->ppu->nmi == 1) {
        bus->ppu->nmi = 0;
        bus->cpu->nmi();
    }

    return bus->cpu->PC;
}

int main() {
    const std::string rom_path = "roms/DK.nes";
    const std::string palette_path = "ntscpalette.pal";

    Controller* controller_1 = new Controller();
    Controller* controller_2 = new Controller();
    Mapper* mapper = Mapper::create_mapper(&rom_path);
    NES_Screen* screen = new NES_Screen(mapper, controller_1, controller_2);
    Bus* bus = new Bus(mapper, controller_1, controller_2, &palette_path);
    CPU* cpu = new CPU(bus);
    PPU* ppu = new PPU(bus, screen);
    APU* apu = new APU();
    bus->cpu = cpu;
    bus->ppu = ppu;
    bus->apu = apu;

    ppu->reset();
    cpu->reset();

    uint8_t val = 0;
    while (1) {
        emulator_tick(bus, screen, val);
#ifdef DEBUG        
        bus->full_log();
#endif
        val++;
    }

    return 0;
}