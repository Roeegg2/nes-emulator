#include "../../include/mappers/mmc1_1.h"
#include <bitset>
#include <fstream>
#include <iostream>

namespace roee_nes {

MMC1_1::MMC1_1(Cartridge *cart) : Mapper(cart), shift_reg(0b0001'0000) {
  set_irq = false;
  mapper_number = 1;

  if (cart->chr_rom.size() == 0) {
    cart->chr_ram.resize(8 * KILOBYTE);
    using_chr_ram = true;
    chr_read_mem = &cart->chr_ram;
  } else {
    using_chr_ram = false;
    chr_read_mem = &cart->chr_rom;
  }

  chr_bank_num = (chr_read_mem->size() / (4 * KILOBYTE));
  prg_bank_num = (cart->prg_rom.size() / (16 * KILOBYTE));

  ctrl.comp.prg_rom_mode = 3;
  ctrl.comp.chr_rom_mode = 1;
  ctrl.comp.mirroring = 0;

  if (cart->header.flag_6.parsed.prg_ram == 1)
    save_ram = new Save_RAM(cart->rom_path + ".sav");
  else
    save_ram = nullptr;
}

void MMC1_1::cpu_write(uint16_t addr, uint8_t data) {
  if ((0x6000 <= addr) && (addr <= 0x7fff)) {
    if (save_ram != nullptr)
      save_ram->mapper_write(addr, data);
  } else if (data & 0b1000'0000) {
    shift_reg = 0b0001'0000;
    ctrl.comp.prg_rom_mode = 3;
  } else if ((0x8000 <= addr) && (addr <= 0xffff)) {
    if (shift_reg & 0b0000'0001) { // if this is the 5th write
      uint8_t foo = (((data & 0b0000'0001) << 4) | (shift_reg >> 1));

      if ((0x8000 <= addr) && (addr <= 0x9fff)) {
        ctrl.comp.mirroring = foo & 0b0'0011;
        ctrl.comp.prg_rom_mode = (foo & 0b0'1100) >> 2;
        ctrl.comp.chr_rom_mode = (foo & 0b1'0000) >> 4;
      } else if ((0xa000 <= addr) && (addr <= 0xbfff))
        chr_bank.bank_0 = foo & 0b0001'1111;
      else if ((0xc000 <= addr) && (addr <= 0xdfff))
        chr_bank.bank_1 = foo & 0b0001'1111;
      else if ((0xe000 <= addr) && (addr <= 0xffff)) {
        prg_bank.bank = foo & 0b0000'1111;
        prg_bank.ext = (foo >> 4) & 0b0000'0001;
      }

      shift_reg = 0b0001'0000;
    } else {
      shift_reg >>= 1;
      shift_reg |= (0b0001'0000 & (data << 4));
    }
  }
}

uint8_t MMC1_1::cpu_read(uint16_t addr, uint8_t open_bus_data) {
  if ((0x6000 <= addr) && (addr <= 0x7fff)) {
    if (save_ram != nullptr)
      return save_ram->mapper_read(addr);
    else
      return open_bus_data;
  } else if ((0x8000 <= addr) && (addr <= 0xffff)) {
    update_prg(addr);
    return cart->prg_rom[(final_bank * 16 * KILOBYTE) + final_addr];
  } else {
    return open_bus_data;
  }
}

void MMC1_1::update_chr(uint16_t addr) {
  if (ctrl.comp.chr_rom_mode == 0) {
    final_addr = addr % 0x2000;
    final_bank = ((chr_bank.bank_0 & 0b0001'1110) % chr_bank_num);
  } else {
    final_addr = addr % 0x1000;
    if ((0x0000 <= addr) && (addr <= 0x0fff))
      final_bank = chr_bank.bank_0 % chr_bank_num;
    else // if ((0x1000 <= addr) && (addr <= 0x1fff)) {
      final_bank = chr_bank.bank_1 % chr_bank_num;
  }
  if (using_chr_ram == true)
    final_bank &= 1;
}

void MMC1_1::update_prg(uint16_t addr) {
  switch (ctrl.comp.prg_rom_mode) {
  case 0:
  case 1: // switch 32 KB at $8000
    final_bank = ((prg_bank.bank & 0b0000'1110) % prg_bank_num);
    final_addr = addr % 0x8000;
    break;
  case 2: // fix first bank at $8000 and switch 16 KB bank at $C000
    final_addr = addr % 0x4000;
    if ((0x8000 <= addr) && (addr <= 0xbfff))
      final_bank = 0;
    else // addr is 0xc000 and over
      final_bank = (prg_bank.bank % prg_bank_num);
    break;
  case 3:
    final_addr = addr % 0x4000;
    if ((0x8000 <= addr) && (addr <= 0xbfff))
      final_bank = prg_bank.bank % prg_bank_num;
    else // addr is 0xc000 and over
      final_bank = prg_bank_num - 1;
    break;
  }
}

uint8_t MMC1_1::ppu_read(uint16_t addr) {
  update_chr(addr);
  return (*chr_read_mem)[(final_bank * 4 * KILOBYTE) + final_addr];
}

void MMC1_1::ppu_write(uint16_t addr, uint8_t data) {
  if (using_chr_ram == true) {
    update_chr(addr);
    (*chr_read_mem)[addr] = data;
  } else
    std::cerr << "ERR trying to write to chr rom!\n";
}

void MMC1_1::reset() {
  shift_reg = 0b0001'0000;
  ctrl.comp.prg_rom_mode = 3;
  // set prg rom to bank mode 3 (fixing the last bank at $C000 and allowing the
  // 16 KB bank at $8000 to be switched)
}

uint16_t MMC1_1::get_nt_mirrored_addr(const uint16_t addr) const {
  switch (ctrl.comp.mirroring) {
  case 0: // single nt, first one
    return addr % 0x400;
  case 1: // single nt, second one
    return (addr % 0x400) + 0x400;
  case 2: // vertical mirroring
    return addr % 0x800;
  default: // case 3: // horizontal mirroring
    if (addr < 0x0800)
      return addr % 0x400;
    else
      return (addr % 0x400) + 0x400;
  }
}

void MMC1_1::save() {
  if (save_ram != nullptr)
    save_ram->~Save_RAM();
}
} // namespace roee_nes
