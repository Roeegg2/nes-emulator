#ifndef CPU_H
#define CPU_H

#include "bus.h"
#include "ppu.h"

#include <cstdint>
#include <vector>

namespace roee_nes {
    constexpr double CPU_CLOCK_SPEED = 0.5586592178771;
    constexpr uint8_t IRQ_CYCLES = 7;
    constexpr uint8_t NMI_CYCLES = 8;
    constexpr uint8_t RST_CYCLES = 7;
    constexpr uint16_t STACK_BASE = 256;

    enum StatusFlag : uint8_t {
        CARRY_BIT = 0b00000001,
        ZERO_BIT = 0b00000010,
        DISINT_BIT = 0b00000100, // hardware interrupt flag
        DECIMAL_BIT = 0b00001000, // decimal mode flag - unused in the NES's 6502 implementation
        BRK_BIT = 0b00010000, // software (brk) interrupt flag.
        UNUSED_BIT = 0b00100000, // unused flag in 6502
        OVERFLOW_BIT = 0b01000000,
        NEGATIVE_BIT = 0b10000000
    };

    enum AddressingMode : uint8_t { 
        IMP = 0, 
        ACC = 1, 
        IMM = 2, 
        ABS = 3, 
        ZP = 4, 
        ZP_X = 5, 
        ZP_Y = 6, 
        REL = 7, 
        ABS_X = 8, 
        ABS_Y = 9, 
        IND = 10, 
        IND_Y = 11, 
        X_IND = 12,
    };

    // forward declaration to avoid circular dependency
    class Bus; 
    class CPU; 
    struct Instruction;

    struct Instruction {
        std::string name;
        void (CPU::* exec)(); // pointer to the instruction to execute

        AddressingMode mode;
        uint8_t cycles;
    };

    class CPU {
    public:
        uint8_t S;
        uint8_t P;
        uint16_t PC;
        uint8_t A;
        uint8_t X;
        uint8_t Y;
        uint8_t IR;

        Bus* bus;
        Instruction* inst;
        uint16_t bytes;
        std::vector<Instruction> lookup;

// #ifdef DEBUG
        uint16_t log_PC;
        uint16_t log_bytes;
        uint8_t log_A;
        uint8_t log_P;
        uint8_t log_S;
        uint8_t log_X;
        uint8_t log_Y;
// #endif

    public:
        uint8_t run_cpu();
        void fetch_decode_inst();
        CPU();

        /* interrupts */
        void nmi(); // non maskable - triggered by the PPU
        void irq(); // maskable
        void reset(); // reset

    private:
        void set_flag(const StatusFlag flag, const uint8_t res);
        uint8_t get_flag_status(const StatusFlag flag) const;
        uint8_t pop();
        void push(const uint8_t value);

        /* the functions called for each instruction */
        void ADC();	void AND();	void ASL();	void BCC();
        void BCS();	void BEQ();	void BIT();	void BMI();
        void BNE();	void BPL();	void BRK();	void BVC();
        void BVS();	void CLC();	void CLD();	void CLI();
        void CLV();	void CMP();	void CPX();	void CPY();
        void DEC();	void DEX();	void DEY();	void EOR();
        void INC();	void INX();	void INY();	void JMP();
        void JSR();	void LDA();	void LDX();	void LDY();
        void LSR();	void NOP();	void ORA();	void PHA();
        void PHP();	void PLA();	void PLP();	void ROL();
        void ROR();	void RTI();	void RTS();	void SBC();
        void SEC();	void SED();	void SEI();	void STA();
        void STX();	void STY();	void TAX();	void TAY();
        void TSX();	void TXA();	void TXS();	void TYA();
        void ILL(); // illegal opcode

        // to avoid code duplications in some instructions, i added these functions (see implementation)
        void reg_CMP_actual(const uint8_t& reg);
        void reg_INCDEC_actual(uint8_t& reg, const uint8_t val);
        void mem_INCDEC_actual(const int8_t val);
        void reg_LD_actual(uint8_t& reg);
        void reg_T_actual(uint8_t& dst_reg, const uint8_t& src_reg) ;
        void LSR_actual(uint8_t& reg);
        void ROL_actual(uint8_t& target);
        void ROR_actual(uint8_t& target);
    };

}
#endif