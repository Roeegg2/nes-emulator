#include "../../include/mappers/mmc1_1.h"

namespace roee_nes {

    MMC1_1::MMC1_1(Cartridge* cart) :
        Mapper(cart) {}

    void MMC1_1::cpu_write(uint16_t addr, uint8_t data) {
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
            reset();
        } else {
            if (shift & 0b0000'0001) { // if this is the 5th write
                //     uint8_t temp = (addr & 0b0111'0000'0000'0000) >> 12; // NOTE note sure about this. maybe i should only check bits 13 and 14?

                if ((0xa000 <= addr) && (addr <= 0xbfff)) {
                    chr_bank.bank_0 = ((data & 0b0000'0001) << 5) | (shift & 0b0001'1110);
                } else if ((0xc000 <= addr) && (addr <= 0xdfff)) {
                    chr_bank.bank_1 = ((data & 0b0000'0001) << 5) | (shift & 0b0001'1110);
                } else if ((0xe000 <= addr) && (addr <= 0xffff)) {
                    prg_bank = ((data & 0b0000'0001) << 5) | (shift & 0b0001'1110);
                }

                shift = 0b0001'0000;
            } else { /// emplace back bit 0 of data
                shift >>= 1;
                shift |= (0b1000'000 & (data << 7)); // masking just in case
            }
        }
    }

    void MMC1_1::reset() {
        shift = 0b0001'0000;
        // set prg rom to bank mode 3 (fixing the last bank at $C000 and allowing the 16 KB bank at $8000 to be switched)
    }

    uint16_t MMC1_1::get_nt_mirrored_addr(uint16_t addr) {
        if (ctrl.comp.mirroring == 0b00)
            return addr % 0x400;
        else if (ctrl.comp.mirroring == 0b00)
                return (addr % 0x400) + 0x400;
        else if (ctrl.comp.mirroring == 0b10) // vertical
            return addr % 0x800;
        else if (ctrl.comp.mirroring == 0b11) { // horizontal
            if (addr >= 0x800)
                return (addr % 0x400) + 0x400;
            else
                return addr % 0x400;
        }
        else 
            std::cerr << "MMC1 MIRRORING PROBLEM!\n";
    }
}