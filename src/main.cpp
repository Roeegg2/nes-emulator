#include "../include/bus.h"
#include "../include/cpu.h"
#include "../include/utils.h"
#include "../include/nes_screen.h"

using namespace roee_nes;

// extern "C" void debugger_run(std::array<std::array<uint8_t, 0x400>, 4>* nt_vram);

uint16_t emulator_tick(CPU* cpu, PPU* ppu) {
    uint8_t cycles;

    cycles = cpu->run_cpu();
    ppu->run_ppu(cycles * 3);

    if (ppu->nmi == 1) {
        ppu->nmi = 0;
        cpu->nmi();
    }

    return cpu->PC;
}

int main() {
        const std::string rom_path = "roms/DD.nes";
    const std::string palette_path = "ntscpalette.pal";

    // Controller controller1 = {.ret_buffer = 0, .live_status_reg = 0, .shift_reg = 0}; // might move these to heap
    // Controller controller2 = {.ret_buffer = 0, .live_status_reg = 0, .shift_reg = 0};
    Controller* controller1 = new Controller();
    Controller* controller2 = new Controller();
    NES_Screen* screen = new NES_Screen(controller1, controller2);
    Bus* bus = new Bus(Mapper::create_mapper(&rom_path), controller1, controller2, &palette_path);
    CPU* cpu = new CPU(bus);
    PPU* ppu = new PPU(bus, screen);
    bus->cpu = cpu;
    bus->ppu = ppu;

    ppu->reset();
    cpu->reset();

    while (1) {
        emulator_tick(cpu, ppu);
    }

    // bus->find_difference();

    return 0;
}