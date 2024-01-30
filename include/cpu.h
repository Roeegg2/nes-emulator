#ifndef CPU_H
#define CPU_H

#include "bus.h"
#include "ppu.h"
#include "utils.h"

#include <cstdint>
#include <vector>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

constexpr double CPU_CLOCK_SPEED = 0.5586592178771;

constexpr uint8_t IRQ_CYCLES = 7;
constexpr uint8_t NMI_CYCLES = 8;
constexpr uint8_t RST_CYCLES = 7;

constexpr uint16_t STACK_BASE = 256;

namespace roee_nes {

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

    enum AddressingMode : uint8_t { IMP, ACC, IMM, ABS, ZP, ZP_X, ZP_Y, REL, ABS_X, ABS_Y, IND, IND_Y, X_IND };

    class Bus; // forward declaration to avoid circular dependency
    class CPU; // forward declaration to avoid circular dependency
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

        uint16_t log_PC;
        uint16_t log_bytes;
        uint8_t log_A;
        uint8_t log_P;
        uint8_t log_S;
        uint8_t log_X;
        uint8_t log_Y;
    public:
        uint8_t run_cpu();

        void fetch_decode_inst();

        CPU(Bus* bus);

        /* interrupts */
        void nmi(); // non maskable - triggered by the PPU
        void irq(); // maskable
        void reset(); // reset

    private:
        uint8_t Get_from_op(uint8_t offset) const;
        
        void set_flag(StatusFlag flag, uint8_t res);
        uint8_t get_flag_status(StatusFlag flag) const;

        // stack functions
        uint8_t pop();
        void push(uint8_t value);

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
        // this one is a nop which i call for each illegal instruction. I plan to add support for most of them (because some are a pain in the ass to implement) later.
        void ILL();

        // to avoid code duplications in some instructions, i added these functions (see implementation)
        void reg_CMP_actual(uint8_t* reg);
        void reg_INCDEC_actual(uint8_t* reg, uint8_t val);
        void mem_INCDEC_actual(int8_t val);
        void reg_LD_actual(uint8_t* reg);
        void reg_T_actual(uint8_t* dst_reg, uint8_t* src_reg);
        void LSR_actual(uint8_t* reg);
        void ROL_actual(uint8_t* val);
        void ROR_actual(uint8_t* val);
    };

}
#endif