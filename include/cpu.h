#ifndef CPU_H
#define CPU_H

#include "../include/cpu_bus.h"

#include <cstdint>
#include <vector>

#define CARRY_BIT 0b00000001
#define ZERO_BIT 0b00000010
#define DISINT_BIT 0b00000100 // hardware interrupt flag
#define DECIMAL_BIT 0b00001000 // unused in NES's 6502 implementation
#define BRK_BIT 0b00010000 // software (brk) interrupt flag
#define UNUSED_BIT 0b00100000
#define OVERFLOW_BIT 0b01000000
#define NEGATIVE_BIT 0b10000000

#define STACK_BASE 256

constexpr double NTSC_ClockSpeed = 1.789773;

constexpr uint8_t IRQCycles = 7;
constexpr uint8_t NMICycles = 8;
constexpr uint8_t ResetCycles = 7;

enum AddressingModes {IMP, ACC, IMM, ABS, ZP, ZP_X, ZP_Y, REL, ABS_X, ABS_Y, IND, IND_Y, X_IND, XXX};

struct Instruction;
class CPU;

struct Instruction {
    // uint8_t opcode;
    void (CPU::*exec)();
    uint8_t mode;
    uint8_t cycles;
};

class CPU {
private:
    uint8_t A;
    uint8_t P;
    uint16_t PC;
    uint8_t S;
    uint8_t X;
    uint8_t Y;
    uint8_t IR; 
    CPU_Bus* bus;

    Instruction* inst;
    uint16_t bytes;

    std::vector<Instruction> lookup;

public:
    uint8_t fetch(uint8_t offset);
    void fetch_decode_inst();
    void execute_inst();

    void print_state(uint8_t* temp);

    CPU(CPU_Bus* bus);

    void nmi();
    void irq();
    void reset();

private:
    void set_flag(uint8_t flag, uint8_t res);
    uint8_t get_flag_status(uint8_t flag);

    uint8_t pop();
    void push(uint8_t data);

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
    void XXX();

    void reg_CMP_actual(uint8_t* reg);
    void reg_INCDEC_actual(uint8_t* reg, uint8_t val);
    void mem_INCDEC_actual(int8_t val);
    void reg_LD_actual(uint8_t* reg);
    void reg_T_actual(uint8_t* dst_reg, uint8_t* src_reg);
};


#endif