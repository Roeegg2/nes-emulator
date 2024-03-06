#include "../include/bus.h"
#include "../include/cpu.h"
#include "../include/utils.h"
#include "../include/nes_screen.h"
#include <chrono>

using namespace roee_nes;

// extern "C" void debugger_run(std::array<std::array<uint8_t, 0x400>, 4>* nt_vram);

uint16_t emulator_tick(CPU* cpu, PPU* ppu, Bus* bus, NES_Screen* screen, uint8_t val) {
    uint8_t cycles;
    if (val % 100 == 0)
        screen->handle_events();

    if (bus->cpu_sleep_dma_counter == 0)
        cycles = cpu->run_cpu();
    else {
        bus->cpu_sleep_dma_counter--;
        cycles = 2; // takes 2 cycles to transfer one byte
    }
    // if ((ppu->ext_regs.ppumask.raw & 0b0001'1000))
    ppu->run_ppu(cycles * 3);

    if (ppu->nmi == 1) {
        ppu->nmi = 0;
        cpu->nmi();
    }

    return cpu->PC;
}

int main() {
    const std::string rom_path = "roms/LOZ.nes";
    const std::string palette_path = "ntscpalette.pal";

    Controller* controller_1 = new Controller();
    Controller* controller_2 = new Controller();
    NES_Screen* screen = new NES_Screen(controller_1, controller_2);
    Bus* bus = new Bus(Mapper::create_mapper(&rom_path), controller_1, controller_2, &palette_path);
    CPU* cpu = new CPU(bus);
    PPU* ppu = new PPU(bus, screen);
    bus->cpu = cpu;
    bus->ppu = ppu;


    ppu->reset();
    cpu->reset();

    uint8_t val = 0;
    while (1) {
        emulator_tick(cpu, ppu, bus, screen, val);
#ifdef DEBUG        
        bus->full_log();
#endif
        val++;
    }

    return 0;
}