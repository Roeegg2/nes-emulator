#include "../include/bus.h"
#include "../include/mappers/mmc3_4.h"
#include "../include/utils.h"
#include <fstream>
#include <iomanip>
#include <iostream>

namespace roee_nes {
Bus::Bus(CPU *cpu, PPU *ppu, Mapper *mapper, Controller *controller_1,
         Controller *controller_2, const std::string &palette_path) {
  this->cpu = cpu;
  this->ppu = ppu;
  this->mapper = mapper;
  this->controller_1 = controller_1;
  this->controller_2 = controller_2;
  init_palette(palette_path);

  cpu->bus = this;
  ppu->bus = this;

  cpu->reset();
  ppu->reset();
}

void Bus::init_palette(const std::string &palette_path) {
  std::ifstream pal_file(palette_path, std::ios::binary);

  if (!pal_file.is_open())
    std::cerr << "ERROR: Failed to open palette file" << "\n";

  for (int i = 0; i < 64 * 3; i += 3) {
    for (int j = 0; j < 3; j++) {
      pal_file.read((char *)&color_palette[i + j], 1);
    }
  }
}

void Bus::ppu_write(uint16_t addr, const uint8_t data,
                    const bool came_from_cpu) {
  if (0 <= addr && addr <= 0x1fff) {
    // std::cout << "addr is: " << std::hex << addr << std::dec << "\n";
    // std::cout << "pc is: " << std::hex << cpu->PC << std::dec << "\n";
    // std::cout << "instruction is " << std::hex << static_cast<int>(cpu->IR)
    // << std::dec << "\n";
    mapper->ppu_write(addr, data);
  } else if (0x2000 <= addr && addr <= 0x3eff) {
    addr %= 0x1000;
    addr = mapper->get_nt_mirrored_addr(addr);

    nt_vram[addr] = data;
  } else if (0x3f00 <= addr && addr <= 0x3fff) {
    addr %= 0x20; // the actual size of the palette is 0x20

    if (came_from_cpu) {
      if ((addr == 0x10) || (addr == 0x14) || (addr == 0x18) || (addr == 0x1c))
        addr -= 0x10;
    } else if ((addr & 0b11) == 0)
      addr = 0;

    palette_vram[addr] = data;
  }
}

uint8_t Bus::ppu_read(uint16_t addr, const bool came_from_cpu) {
  if (0 <= addr && addr <= 0x1fff) {
    return mapper->ppu_read(addr); // pattern table
  } else if (0x2000 <= addr && addr <= 0x3eff) {
    addr %= 0x1000;
    addr = mapper->get_nt_mirrored_addr(addr);

    return nt_vram[addr];
  } else if (0x3f00 <= addr && addr <= 0x3fff) {
    addr %= 0x20; // the actual size of the palette is 0x20
    if (came_from_cpu) {
      if ((addr == 0x10) || (addr == 0x14) || (addr == 0x18) || (addr == 0x1c))
        addr -= 0x10;
    } else if ((addr & 0b11) == 0)
      addr = 0;

    if (ppu->ext_regs.ppumask.comp.grayscale) {
      return palette_vram[addr] & 0x30;
    }

    return palette_vram[addr];
  }

  return 0;
}

void Bus::cpu_write(const uint16_t addr, const uint8_t data) {
  if (0 <= addr && addr <= 0x1fff)
    ram[addr % 0x800] = data;
  else if (0x2000 <= addr && addr <= 0x3fff)
    ppu->cpu_write_ppu(addr % 0x2000, data);
  else if (addr == 0x4014) {     // OAMDMA
    cpu_sleep_dma_counter = 513; // TODO takes more sometimes
    uint16_t start_addr = data;
    start_addr <<= 8;
    // start_addr |= (0x00ff & ppu->ext_regs.oamaddr);
    for (int i = 0; i < 256; i++) {
      ppu->primary_oam[i] = ram[start_addr + i];
    }
  } else if (addr == 0x4016) {
    controller_1->write(data);
    controller_2->write(data);
  } else if (0x4020 <= addr && addr <= 0xffff)
    mapper->cpu_write(addr, data);
  else if (addr == 0x4017) {
    apu->cpu_write_apu(addr, data);
  }

  // else if ((0x401c <= addr) && (addr <= 0x401f)) // unfinished IRQ timer
  // functionality
  //     return; // do nothing!
}

uint8_t Bus::cpu_read(const uint16_t addr) {
  if (0 <= addr && addr <= 0x1fff)
    cpu_dma_controllers_open_bus = ram[addr % 0x800];
  else if (0x2000 <= addr && addr <= 0x3fff)
    cpu_dma_controllers_open_bus = ppu->cpu_read_ppu(addr % 8);
  else if (0x4000 <= addr && addr <= 0x4014)
    cpu_dma_controllers_open_bus = 0; // didnt implement yet
  else if (addr == 0x4016)
    cpu_dma_controllers_open_bus = controller_1->read();
  else if (addr == 0x4017)
    cpu_dma_controllers_open_bus = controller_2->read();
  else if (0x4020 <= addr && addr <= 0xffff)
    cpu_dma_controllers_open_bus =
        mapper->cpu_read(addr, cpu_dma_controllers_open_bus);

  return cpu_dma_controllers_open_bus;
}

#ifdef DEBUG
void Bus::full_log() const {
  static std::ofstream roee_file("logs/ROEE_NES_MAIN.log");

  roee_file << std::hex << std::uppercase << std::setw(4) << std::setfill('0')
            << static_cast<uint32_t>(cpu->log_PC) << " " << std::hex
            << std::setw(2) << std::setfill('0') << static_cast<int>(cpu->IR)
            << " " << std::hex << std::setw(2) << std::setfill('0')
            << (cpu->log_bytes & 0x00ff) << " " << std::hex << std::setw(2)
            << std::setfill('0') << (cpu->log_bytes >> 8) << " "
            << cpu->inst->name;

  roee_file << "\t A:" << std::hex << std::uppercase << std::setw(2)
            << std::setfill('0') << static_cast<int>(cpu->log_A)
            << " X:" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(cpu->log_X) << " Y:" << std::hex << std::setw(2)
            << std::setfill('0') << static_cast<int>(cpu->log_Y)
            << " P:" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(cpu->log_P) << " SP:" << std::hex
            << std::setw(2) << std::setfill('0') << static_cast<int>(cpu->log_S)
            << std::dec;

  roee_file << "\t t:" << std::hex << std::uppercase << std::setw(2)
            << std::setfill('0') << static_cast<int>(ppu->t.raw)
            << " v:" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(ppu->v.raw) << " x:" << std::hex << std::setw(2)
            << std::setfill('0') << static_cast<int>(ppu->x)
            << " w:" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(ppu->w) << " ppuctrl:" << std::hex
            << std::setw(2) << std::setfill('0')
            << static_cast<int>(ppu->ext_regs.ppuctrl.raw)
            << " ppumask:" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(ppu->ext_regs.ppumask.raw)
            << " ppustatus:" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(ppu->ext_regs.ppustatus.raw) << std::dec;

  roee_file << " PPU: " << ppu->curr_scanline << ", " << ppu->curr_cycle
            << ", CYC:" << ppu->curr_cycle / 3 << "\n";
}
#endif
} // namespace roee_nes
