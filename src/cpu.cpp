#include "../include/cpu.h"
#include "../include/utils.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

CPU::CPU(Bus* bus){
    /* I know, its ugly, but I'm using it for now (possibly forever) because its very easy and conventient */
    /* Credit to OLC on the map that served as the foundation of this one */
    using a = CPU;
    lookup = 
	{
		{ &a::BRK, IMM, 7 },{ &a::ORA, X_IND, 6 },{ &a::ILLEGAL, IMP, 2 },{ &a::ILLEGAL, IMP, 8 },{ &a::NOP, IMP, 3 },{ &a::ORA, ZP, 3 },{ &a::ASL, ZP, 5 },{ &a::ILLEGAL, IMP, 5 },{ &a::PHP, IMP, 3 },{ &a::ORA, IMM, 2 },{ &a::ASL, IMP, 2 },{ &a::ILLEGAL, IMP, 2 },{ &a::NOP, IMP, 4 },{ &a::ORA, ABS, 4 },{ &a::ASL, ABS, 6 },{ &a::ILLEGAL, IMP, 6 },
		{ &a::BPL, REL, 2 },{ &a::ORA, IND_Y, 5 },{ &a::ILLEGAL, IMP, 2 },{ &a::ILLEGAL, IMP, 8 },{ &a::NOP, IMP, 4 },{ &a::ORA, ZP_X, 4 },{ &a::ASL, ZP_X, 6 },{ &a::ILLEGAL, IMP, 6 },{ &a::CLC, IMP, 2 },{ &a::ORA, ABS_Y, 4 },{ &a::NOP, IMP, 2 },{ &a::ILLEGAL, IMP, 7 },{ &a::NOP, IMP, 4 },{ &a::ORA, ABS_X, 4 },{ &a::ASL, ABS_X, 7 },{ &a::ILLEGAL, IMP, 7 },
		{ &a::JSR, ABS, 6 },{ &a::AND, X_IND, 6 },{ &a::ILLEGAL, IMP, 2 },{ &a::ILLEGAL, IMP, 8 },{ &a::BIT, ZP, 3 },{ &a::AND, ZP, 3 },{ &a::ROL, ZP, 5 },{ &a::ILLEGAL, IMP, 5 },{ &a::PLP, IMP, 4 },{ &a::AND, IMM, 2 },{ &a::ROL, IMP, 2 },{ &a::ILLEGAL, IMP, 2 },{ &a::BIT, ABS, 4 },{ &a::AND, ABS, 4 },{ &a::ROL, ABS, 6 },{ &a::ILLEGAL, IMP, 6 },
		{ &a::BMI, REL, 2 },{ &a::AND, IND_Y, 5 },{ &a::ILLEGAL, IMP, 2 },{ &a::ILLEGAL, IMP, 8 },{ &a::NOP, IMP, 4 },{ &a::AND, ZP_X, 4 },{ &a::ROL, ZP_X, 6 },{ &a::ILLEGAL, IMP, 6 },{ &a::SEC, IMP, 2 },{ &a::AND, ABS_Y, 4 },{ &a::NOP, IMP, 2 },{ &a::ILLEGAL, IMP, 7 },{ &a::NOP, IMP, 4 },{ &a::AND, ABS_X, 4 },{ &a::ROL, ABS_X, 7 },{ &a::ILLEGAL, IMP, 7 },
		{ &a::RTI, IMP, 6 },{ &a::EOR, X_IND, 6 },{ &a::ILLEGAL, IMP, 2 },{ &a::ILLEGAL, IMP, 8 },{ &a::NOP, IMP, 3 },{ &a::EOR, ZP, 3 },{ &a::LSR, ZP, 5 },{ &a::ILLEGAL, IMP, 5 },{ &a::PHA, IMP, 3 },{ &a::EOR, IMM, 2 },{ &a::LSR, IMP, 2 },{ &a::ILLEGAL, IMP, 2 },{ &a::JMP, ABS, 3 },{ &a::EOR, ABS, 4 },{ &a::LSR, ABS, 6 },{ &a::ILLEGAL, IMP, 6 },
		{ &a::BVC, REL, 2 },{ &a::EOR, IND_Y, 5 },{ &a::ILLEGAL, IMP, 2 },{ &a::ILLEGAL, IMP, 8 },{ &a::NOP, IMP, 4 },{ &a::EOR, ZP_X, 4 },{ &a::LSR, ZP_X, 6 },{ &a::ILLEGAL, IMP, 6 },{ &a::CLI, IMP, 2 },{ &a::EOR, ABS_Y, 4 },{ &a::NOP, IMP, 2 },{ &a::ILLEGAL, IMP, 7 },{ &a::NOP, IMP, 4 },{ &a::EOR, ABS_X, 4 },{ &a::LSR, ABS_X, 7 },{ &a::ILLEGAL, IMP, 7 },
		{ &a::RTS, IMP, 6 },{ &a::ADC, X_IND, 6 },{ &a::ILLEGAL, IMP, 2 },{ &a::ILLEGAL, IMP, 8 },{ &a::NOP, IMP, 3 },{ &a::ADC, ZP, 3 },{ &a::ROR, ZP, 5 },{ &a::ILLEGAL, IMP, 5 },{ &a::PLA, IMP, 4 },{ &a::ADC, IMM, 2 },{ &a::ROR, IMP, 2 },{ &a::ILLEGAL, IMP, 2 },{ &a::JMP, IND, 5 },{ &a::ADC, ABS, 4 },{ &a::ROR, ABS, 6 },{ &a::ILLEGAL, IMP, 6 },
		{ &a::BVS, REL, 2 },{ &a::ADC, IND_Y, 5 },{ &a::ILLEGAL, IMP, 2 },{ &a::ILLEGAL, IMP, 8 },{ &a::NOP, IMP, 4 },{ &a::ADC, ZP_X, 4 },{ &a::ROR, ZP_X, 6 },{ &a::ILLEGAL, IMP, 6 },{ &a::SEI, IMP, 2 },{ &a::ADC, ABS_Y, 4 },{ &a::NOP, IMP, 2 },{ &a::ILLEGAL, IMP, 7 },{ &a::NOP, IMP, 4 },{ &a::ADC, ABS_X, 4 },{ &a::ROR, ABS_X, 7 },{ &a::ILLEGAL, IMP, 7 },
		{ &a::NOP, IMP, 2 },{ &a::STA, X_IND, 6 },{ &a::NOP, IMP, 2 },{ &a::ILLEGAL, IMP, 6 },{ &a::STY, ZP, 3 },{ &a::STA, ZP, 3 },{ &a::STX, ZP, 3 },{ &a::ILLEGAL, IMP, 3 },{ &a::DEY, IMP, 2 },{ &a::NOP, IMP, 2 },{ &a::TXA, IMP, 2 },{ &a::ILLEGAL, IMP, 2 },{ &a::STY, ABS, 4 },{ &a::STA, ABS, 4 },{ &a::STX, ABS, 4 },{ &a::ILLEGAL, IMP, 4 },
		{ &a::BCC, REL, 2 },{ &a::STA, IND_Y, 6 },{ &a::ILLEGAL, IMP, 2 },{ &a::ILLEGAL, IMP, 6 },{ &a::STY, ZP_X, 4 },{ &a::STA, ZP_X, 4 },{ &a::STX, ZP_Y, 4 },{ &a::ILLEGAL, IMP, 4 },{ &a::TYA, IMP, 2 },{ &a::STA, ABS_Y, 5 },{ &a::TXS, IMP, 2 },{ &a::ILLEGAL, IMP, 5 },{ &a::NOP, IMP, 5 },{ &a::STA, ABS_X, 5 },{ &a::ILLEGAL, IMP, 5 },{ &a::ILLEGAL, IMP, 5 },
		{ &a::LDY, IMM, 2 },{ &a::LDA, X_IND, 6 },{ &a::LDX, IMM, 2 },{ &a::ILLEGAL, IMP, 6 },{ &a::LDY, ZP, 3 },{ &a::LDA, ZP, 3 },{ &a::LDX, ZP, 3 },{ &a::ILLEGAL, IMP, 3 },{ &a::TAY, IMP, 2 },{ &a::LDA, IMM, 2 },{ &a::TAX, IMP, 2 },{ &a::ILLEGAL, IMP, 2 },{ &a::LDY, ABS, 4 },{ &a::LDA, ABS, 4 },{ &a::LDX, ABS, 4 },{ &a::ILLEGAL, IMP, 4 },
		{ &a::BCS, REL, 2 },{ &a::LDA, IND_Y, 5 },{ &a::ILLEGAL, IMP, 2 },{ &a::ILLEGAL, IMP, 5 },{ &a::LDY, ZP_X, 4 },{ &a::LDA, ZP_X, 4 },{ &a::LDX, ZP_Y, 4 },{ &a::ILLEGAL, IMP, 4 },{ &a::CLV, IMP, 2 },{ &a::LDA, ABS_Y, 4 },{ &a::TSX, IMP, 2 },{ &a::ILLEGAL, IMP, 4 },{ &a::LDY, ABS_X, 4 },{ &a::LDA, ABS_X, 4 },{ &a::LDX, ABS_Y, 4 },{ &a::ILLEGAL, IMP, 4 },
		{ &a::CPY, IMM, 2 },{ &a::CMP, X_IND, 6 },{ &a::NOP, IMP, 2 },{ &a::ILLEGAL, IMP, 8 },{ &a::CPY, ZP, 3 },{ &a::CMP, ZP, 3 },{ &a::DEC, ZP, 5 },{ &a::ILLEGAL, IMP, 5 },{ &a::INY, IMP, 2 },{ &a::CMP, IMM, 2 },{ &a::DEX, IMP, 2 },{ &a::ILLEGAL, IMP, 2 },{ &a::CPY, ABS, 4 },{ &a::CMP, ABS, 4 },{ &a::DEC, ABS, 6 },{ &a::ILLEGAL, IMP, 6 },
		{ &a::BNE, REL, 2 },{ &a::CMP, IND_Y, 5 },{ &a::ILLEGAL, IMP, 2 },{ &a::ILLEGAL, IMP, 8 },{ &a::NOP, IMP, 4 },{ &a::CMP, ZP_X, 4 },{ &a::DEC, ZP_X, 6 },{ &a::ILLEGAL, IMP, 6 },{ &a::CLD, IMP, 2 },{ &a::CMP, ABS_Y, 4 },{ &a::NOP, IMP, 2 },{ &a::ILLEGAL, IMP, 7 },{ &a::NOP, IMP, 4 },{ &a::CMP, ABS_X, 4 },{ &a::DEC, ABS_X, 7 },{ &a::ILLEGAL, IMP, 7 },
		{ &a::CPX, IMM, 2 },{ &a::SBC, X_IND, 6 },{ &a::NOP, IMP, 2 },{ &a::ILLEGAL, IMP, 8 },{ &a::CPX, ZP, 3 },{ &a::SBC, ZP, 3 },{ &a::INC, ZP, 5 },{ &a::ILLEGAL, IMP, 5 },{ &a::INX, IMP, 2 },{ &a::SBC, IMM, 2 },{ &a::NOP, IMP, 2 },{ &a::SBC, IMP, 2 },{ &a::CPX, ABS, 4 },{ &a::SBC, ABS, 4 },{ &a::INC, ABS, 6 },{ &a::ILLEGAL, IMP, 6 },
		{ &a::BEQ, REL, 2 },{ &a::SBC, IND_Y, 5 },{ &a::ILLEGAL, IMP, 2 },{ &a::ILLEGAL, IMP, 8 },{ &a::NOP, IMP, 4 },{ &a::SBC, ZP_X, 4 },{ &a::INC, ZP_X, 6 },{ &a::ILLEGAL, IMP, 6 },{ &a::SED, IMP, 2 },{ &a::SBC, ABS_Y, 4 },{ &a::NOP, IMP, 2 },{ &a::ILLEGAL, IMP, 7 },{ &a::NOP, IMP, 4 },{ &a::SBC, ABS_X, 4 },{ &a::INC, ABS_X, 7 },{ &a::ILLEGAL, IMP, 7 },
	};

    this->bus = bus;
    S = 0xfd;
    P = 0b00110100;
    A = X = Y = 0x00;
    IR = 0x00;
    bytes = 0x0000;

    reset();
}

uint8_t CPU::fetch(uint8_t offset){
    uint8_t data = bus->cpu_read(PC) + offset;
    PC++;
    return data;
}

void CPU::execute_inst(){
    (this->*inst->exec)();
    
    std::chrono::microseconds sleepDuration(get_sleep_time(inst->cycles));
    std::this_thread::sleep_for(sleepDuration);
}

void CPU::fetch_decode_inst(){ // this is also ugly, might rewrite it in the future
    IR = fetch(0);
    inst = &lookup[IR];

    switch(inst->mode){
        case REL:
        case IMM:
            bytes = fetch(0);
            break;
        case ACC:
        case IMP:
            break;
        case ABS:
            bytes = convert_to_2byte(fetch(0), fetch(0));
            break;
        case ZP:
            bytes = convert_to_2byte(fetch(0), 0);
            break;
        case ZP_X:
            bytes = convert_to_2byte(fetch(0), 0);
            bytes += X;
            break;
        case ZP_Y:
            bytes = convert_to_2byte(fetch(0), 0);
            bytes += Y;
            break;
        case ABS_X:
            bytes = convert_to_2byte(fetch(0), fetch(0));
            bytes += X;
            break;
        case ABS_Y:
            bytes = convert_to_2byte(fetch(0), fetch(0));
            bytes += Y;
            break;
        case IND:
            bytes = convert_to_2byte(fetch(0), fetch(0));
            bytes = bus->cpu_read(bytes);
            break;
        case X_IND:
            bytes = convert_to_2byte(fetch(0) + X, 0);
            bytes = bus->cpu_read(bytes);
            break;
        case IND_Y:
            bytes = convert_to_2byte(fetch(0), 0);
            bytes = bus->cpu_read(bytes);
            bytes += Y;
            break;
        default:
            std::cout << "ERROR: Invalid addressing mode!" << std::endl;
            break;
    }
}


/* P register helper functions */

void CPU::set_flag(StatusFlag flag, uint8_t res){
    if (res)
        P |= flag;
    else
        P &= ~flag;
}

uint8_t CPU::get_flag_status(StatusFlag flag) const{
    return (P & flag) ? 1 : 0;
}


/* Stack helper functions */

uint8_t CPU::pop() {
    S++;
    uint8_t data = bus->cpu_read(STACK_BASE + S);
    if (S >= 0xff)
        std::cerr << "WARNING: Stack underflowing! Could be affecting game memory." << std::endl;

    return data;
}

void CPU::push(uint8_t value){
    bus->cpu_write(STACK_BASE + S, value); // could be passing arguments in the wrong order; check that
    if (S <= 0)
        std::cerr << "WARNING: Stack overflowing into zero page!" << std::endl;
        
    S--;
}

/* ----- util functions for testing - DELETE LATER! */

void check_array_equal(uint8_t* old, uint8_t* changed){
    for (int i = 0; i < 0x800; i++){
        if (old[i] != changed[i]){
            std::cout << "| Index " << i << " is different!" << std::endl;
            std::cout << "| Old: " << (int)old[i] << std::endl;
            std::cout << "| New: " << (int)changed[i] << std::endl;
        }
    }
}

/* Printing util */
void CPU::log() {
    static std::ofstream log("testr/nestest/roeenes.log", std::ios::out | std::ios::trunc);

    log << std::hex << std::uppercase << PC << " ";
    log << std::hex << std::uppercase << (uint16_t)IR << " ";
    log << std::hex << std::uppercase << (uint16_t)(bytes >> 8) << " ";
    log << std::hex << std::uppercase << (uint16_t)(bytes << 8) << " \t";
    log << std::hex << std::uppercase << "A:" << (uint16_t)A << " ";
    log << std::hex << std::uppercase << "X:" << (uint16_t)X << " ";
    log << std::hex << std::uppercase << "Y:" << (uint16_t)Y << " ";
    log << std::hex << std::uppercase << "P:" << (uint16_t)P << " ";
    log << std::hex << std::uppercase << "SP:" << (uint16_t)S << " ";
    log << std::endl;
}