#include "../../include/mappers/mmc1_1.h"
#include <bitset>
namespace roee_nes {

    MMC1_1::MMC1_1(Cartridge* cart) :
        Mapper(cart), shift(0b1'0000), ctrl({ 0 }), chr_bank({ 0 }), prg_bank({ 0 }) {
        if (cart->chr_rom.size() == 0) {
            cart->chr_ram.resize(8 * KILOBYTE);
            chr_read_mem = &cart->chr_ram;
            using_chr_ram = true;
        } else {
            using_chr_ram = false;
            chr_read_mem = &cart->chr_rom;
        }
    }

    void MMC1_1::cpu_write(uint16_t addr, uint8_t data) {
        static std::ofstream templog("logs/TEMP.log");
        /**
         * NOTE: the initial state of shift reg is 10000. 1 is used to detect 5th write
         * if (data has bit 7 set)
         *      reset shift reg into initial state
         * else { // were trying to write data into the shift reg
         *      emplace back bit 0 of data
         *      if (this is the 5th write)
         *          copy shift reg into internal register, selected by bits 14 and 13
         *          clear shift reg
         * }
        */
        if (data & 0b1000'000) {
            shift = 0b0001'0000;
            ctrl.comp.prg_rom_mode = 3;
        } else {
            if (shift & 0b0000'0001) { // if this is the 5th write
                //     uint8_t temp = (addr & 0b0111'0000'0000'0000) >> 12; // NOTE note sure about this. maybe i should only check bits 13 and 14?
                // if ((0x8000 <= addr) && (addr <= 0x9fff)) {

                // }
                std::cout << "DATA: " << std::bitset<sizeof(data) * 8>(data) << std::endl;
                if ((0xa000 <= addr) && (addr <= 0xbfff)) {
                    chr_bank.bank_0 = ((data & 0b0000'0001) << 5) | ((shift & 0b0001'1110) >> 1);
                    templog << "chr bank 0 is " << (int)prg_bank.bank << "\n";
                } else if ((0xc000 <= addr) && (addr <= 0xdfff)) {
                    chr_bank.bank_1 = ((data & 0b0000'0001) << 5) | ((shift & 0b0001'1110) >> 1);
                    templog << "chr bank 1 is " << (int)prg_bank.bank << "\n";
                } else {
                // if ((0xe000 <= addr) && (addr <= 0xffff)) {
                    prg_bank.bank = (shift & 0b0001'1110) >> 1;
                    prg_bank.ext = (data & 0b0000'0001) << 5;
                    templog << "prg_bank is " << (int)prg_bank.bank << "\n";
                }
                // else {
                //     std::cerr << "BAD CPU WRITE! address is " << std::hex << (int)addr << "\n";
                // }

                shift = 0b0001'0000;
            } else { /// emplace back bit 0 of data
                std::cout << "data: " << std::bitset<sizeof(data) * 8>(data) << std::endl;
                shift >>= 1;
                shift |= (0b0001'0000 & (data << 5));
                std::cout << "shift: " << std::bitset<sizeof(shift) * 8>(shift) << std::endl;
            }
        }
    }

    uint8_t MMC1_1::cpu_read(uint16_t addr) {
        switch (ctrl.comp.prg_rom_mode) {
            case 0:
            case 1: // switch 32 KB at $8000
                return cart->prg_rom[((prg_bank.bank & 0b1110) * 32 * KILOBYTE) + addr - 0x8000];
            case 2: // fix first bank at $8000 and switch 16 KB bank at $C000
                if ((0x8000 <= addr) && (addr <= 0xbfff))
                    return cart->prg_rom[addr - 0x8000]; // get first bank
                else // addr is 0xc000 and over
                    return cart->prg_rom[(prg_bank.bank * 16 * KILOBYTE) + addr - 0x8000];
            case 3:
                if ((0x8000 <= addr) && (addr <= 0xbfff))
                    return cart->prg_rom[(prg_bank.bank * 16 * KILOBYTE) + addr - 0x8000];
                else
                    return cart->prg_rom[addr - 0x8000]; // get first bank
                // NOTE WHEN YOU GET BACK!
                // you were in the middle of implementing the prg_rom reading, see:
                /*
                |++--- PRG ROM bank mode (0, 1: switch 32 KB at $8000, ignoring low bit of bank number;
    |                         2: fix first bank at $8000 and switch 16 KB bank at $C000;
    |                         3: fix last bank at $C000 and switch 16 KB bank at $8000)
                */
        }
    }

    uint8_t MMC1_1::ppu_read(uint16_t addr) {
        static std::ofstream templog("logs/TEMP.log");
        switch (ctrl.comp.chr_rom_mode) {
            case 0:
                return (*chr_read_mem)[(chr_bank.bank_0 * 8 * KILOBYTE) + addr];
            default: // case 1:
                if ((0x000 <= addr) && (addr <= 0x0fff))
                    return (*chr_read_mem)[(chr_bank.bank_0 * 4 * KILOBYTE) + addr];
                else // if ((0x1000 <= addr) && (addr <= 0x1fff))
                    return (*chr_read_mem)[(chr_bank.bank_1 * 4 * KILOBYTE) + addr - 0xf000];
        }
    }

    void MMC1_1::ppu_write(uint16_t addr, uint8_t data) {
        if (using_chr_ram == true) {
            switch (ctrl.comp.chr_rom_mode) {
                case 0:
                    (*chr_read_mem)[(chr_bank.bank_0 * 8 * KILOBYTE) + addr] = data;
                case 1:
                    if ((0x000 <= addr) && (addr <= 0x0fff))
                        (*chr_read_mem)[(chr_bank.bank_0 * 4 * KILOBYTE) + addr] = data;
                    else // if ((0x1000 <= addr) && (addr <= 0x1fff))
                        (*chr_read_mem)[(chr_bank.bank_1 * 4 * KILOBYTE) + addr - 0xf000] = data;

                    // std::cerr << "MMC1 BAD PPU READ!\n";
                    // return 0;
            }
        }
    }

    void MMC1_1::reset() {
        shift = 0b0001'0000;
        ctrl.comp.prg_rom_mode = 3;
        // set prg rom to bank mode 3 (fixing the last bank at $C000 and allowing the 16 KB bank at $8000 to be switched)
    }

    uint16_t MMC1_1::get_nt_mirrored_addr(uint16_t addr) {
        if (ctrl.comp.mirroring == 0b00)
            return addr % 0x400;
        else if (ctrl.comp.mirroring == 0b00)
            return (addr % 0x400) + 0x400;
        else if (ctrl.comp.mirroring == 0b10) // vertical
            return addr % 0x800;
        else //if (ctrl.comp.mirroring == 0b11) 
        { // horizontal
            if (addr >= 0x800)
                return (addr % 0x400) + 0x400;
            else
                return addr % 0x400;
        }
        // else
        //     std::cerr << "MMC1 MIRRORING PROBLEM!\n";
    }
}