#include "../include/cpu.h"
#include "../include/utils.h"

#include <iostream>

CPU::CPU(CPU_Bus* bus){
    /* I know, its ugly, but I'm using it for now (possibly forever) */
    /* Credit to OLC on the array that served as the foundation of this one */
    using a = CPU;
    lookup = 
	{
		{ &a::BRK, IMM, 7 },{ &a::ORA, X_IND, 6 },{ &a::XXX, IMP, 2 },{ &a::XXX, IMP, 8 },{ &a::NOP, IMP, 3 },{ &a::ORA, ZP, 3 },{ &a::ASL, ZP, 5 },{ &a::XXX, IMP, 5 },{ &a::PHP, IMP, 3 },{ &a::ORA, IMM, 2 },{ &a::ASL, IMP, 2 },{ &a::XXX, IMP, 2 },{ &a::NOP, IMP, 4 },{ &a::ORA, ABS, 4 },{ &a::ASL, ABS, 6 },{ &a::XXX, IMP, 6 },
		{ &a::BPL, REL, 2 },{ &a::ORA, IND_Y, 5 },{ &a::XXX, IMP, 2 },{ &a::XXX, IMP, 8 },{ &a::NOP, IMP, 4 },{ &a::ORA, ZP_X, 4 },{ &a::ASL, ZP_X, 6 },{ &a::XXX, IMP, 6 },{ &a::CLC, IMP, 2 },{ &a::ORA, ABS_Y, 4 },{ &a::NOP, IMP, 2 },{ &a::XXX, IMP, 7 },{ &a::NOP, IMP, 4 },{ &a::ORA, ABS_X, 4 },{ &a::ASL, ABS_X, 7 },{ &a::XXX, IMP, 7 },
		{ &a::JSR, ABS, 6 },{ &a::AND, X_IND, 6 },{ &a::XXX, IMP, 2 },{ &a::XXX, IMP, 8 },{ &a::BIT, ZP, 3 },{ &a::AND, ZP, 3 },{ &a::ROL, ZP, 5 },{ &a::XXX, IMP, 5 },{ &a::PLP, IMP, 4 },{ &a::AND, IMM, 2 },{ &a::ROL, IMP, 2 },{ &a::XXX, IMP, 2 },{ &a::BIT, ABS, 4 },{ &a::AND, ABS, 4 },{ &a::ROL, ABS, 6 },{ &a::XXX, IMP, 6 },
		{ &a::BMI, REL, 2 },{ &a::AND, IND_Y, 5 },{ &a::XXX, IMP, 2 },{ &a::XXX, IMP, 8 },{ &a::NOP, IMP, 4 },{ &a::AND, ZP_X, 4 },{ &a::ROL, ZP_X, 6 },{ &a::XXX, IMP, 6 },{ &a::SEC, IMP, 2 },{ &a::AND, ABS_Y, 4 },{ &a::NOP, IMP, 2 },{ &a::XXX, IMP, 7 },{ &a::NOP, IMP, 4 },{ &a::AND, ABS_X, 4 },{ &a::ROL, ABS_X, 7 },{ &a::XXX, IMP, 7 },
		{ &a::RTI, IMP, 6 },{ &a::EOR, X_IND, 6 },{ &a::XXX, IMP, 2 },{ &a::XXX, IMP, 8 },{ &a::NOP, IMP, 3 },{ &a::EOR, ZP, 3 },{ &a::LSR, ZP, 5 },{ &a::XXX, IMP, 5 },{ &a::PHA, IMP, 3 },{ &a::EOR, IMM, 2 },{ &a::LSR, IMP, 2 },{ &a::XXX, IMP, 2 },{ &a::JMP, ABS, 3 },{ &a::EOR, ABS, 4 },{ &a::LSR, ABS, 6 },{ &a::XXX, IMP, 6 },
		{ &a::BVC, REL, 2 },{ &a::EOR, IND_Y, 5 },{ &a::XXX, IMP, 2 },{ &a::XXX, IMP, 8 },{ &a::NOP, IMP, 4 },{ &a::EOR, ZP_X, 4 },{ &a::LSR, ZP_X, 6 },{ &a::XXX, IMP, 6 },{ &a::CLI, IMP, 2 },{ &a::EOR, ABS_Y, 4 },{ &a::NOP, IMP, 2 },{ &a::XXX, IMP, 7 },{ &a::NOP, IMP, 4 },{ &a::EOR, ABS_X, 4 },{ &a::LSR, ABS_X, 7 },{ &a::XXX, IMP, 7 },
		{ &a::RTS, IMP, 6 },{ &a::ADC, X_IND, 6 },{ &a::XXX, IMP, 2 },{ &a::XXX, IMP, 8 },{ &a::NOP, IMP, 3 },{ &a::ADC, ZP, 3 },{ &a::ROR, ZP, 5 },{ &a::XXX, IMP, 5 },{ &a::PLA, IMP, 4 },{ &a::ADC, IMM, 2 },{ &a::ROR, IMP, 2 },{ &a::XXX, IMP, 2 },{ &a::JMP, IND, 5 },{ &a::ADC, ABS, 4 },{ &a::ROR, ABS, 6 },{ &a::XXX, IMP, 6 },
		{ &a::BVS, REL, 2 },{ &a::ADC, IND_Y, 5 },{ &a::XXX, IMP, 2 },{ &a::XXX, IMP, 8 },{ &a::NOP, IMP, 4 },{ &a::ADC, ZP_X, 4 },{ &a::ROR, ZP_X, 6 },{ &a::XXX, IMP, 6 },{ &a::SEI, IMP, 2 },{ &a::ADC, ABS_Y, 4 },{ &a::NOP, IMP, 2 },{ &a::XXX, IMP, 7 },{ &a::NOP, IMP, 4 },{ &a::ADC, ABS_X, 4 },{ &a::ROR, ABS_X, 7 },{ &a::XXX, IMP, 7 },
		{ &a::NOP, IMP, 2 },{ &a::STA, X_IND, 6 },{ &a::NOP, IMP, 2 },{ &a::XXX, IMP, 6 },{ &a::STY, ZP, 3 },{ &a::STA, ZP, 3 },{ &a::STX, ZP, 3 },{ &a::XXX, IMP, 3 },{ &a::DEY, IMP, 2 },{ &a::NOP, IMP, 2 },{ &a::TXA, IMP, 2 },{ &a::XXX, IMP, 2 },{ &a::STY, ABS, 4 },{ &a::STA, ABS, 4 },{ &a::STX, ABS, 4 },{ &a::XXX, IMP, 4 },
		{ &a::BCC, REL, 2 },{ &a::STA, IND_Y, 6 },{ &a::XXX, IMP, 2 },{ &a::XXX, IMP, 6 },{ &a::STY, ZP_X, 4 },{ &a::STA, ZP_X, 4 },{ &a::STX, ZP_Y, 4 },{ &a::XXX, IMP, 4 },{ &a::TYA, IMP, 2 },{ &a::STA, ABS_Y, 5 },{ &a::TXS, IMP, 2 },{ &a::XXX, IMP, 5 },{ &a::NOP, IMP, 5 },{ &a::STA, ABS_X, 5 },{ &a::XXX, IMP, 5 },{ &a::XXX, IMP, 5 },
		{ &a::LDY, IMM, 2 },{ &a::LDA, X_IND, 6 },{ &a::LDX, IMM, 2 },{ &a::XXX, IMP, 6 },{ &a::LDY, ZP, 3 },{ &a::LDA, ZP, 3 },{ &a::LDX, ZP, 3 },{ &a::XXX, IMP, 3 },{ &a::TAY, IMP, 2 },{ &a::LDA, IMM, 2 },{ &a::TAX, IMP, 2 },{ &a::XXX, IMP, 2 },{ &a::LDY, ABS, 4 },{ &a::LDA, ABS, 4 },{ &a::LDX, ABS, 4 },{ &a::XXX, IMP, 4 },
		{ &a::BCS, REL, 2 },{ &a::LDA, IND_Y, 5 },{ &a::XXX, IMP, 2 },{ &a::XXX, IMP, 5 },{ &a::LDY, ZP_X, 4 },{ &a::LDA, ZP_X, 4 },{ &a::LDX, ZP_Y, 4 },{ &a::XXX, IMP, 4 },{ &a::CLV, IMP, 2 },{ &a::LDA, ABS_Y, 4 },{ &a::TSX, IMP, 2 },{ &a::XXX, IMP, 4 },{ &a::LDY, ABS_X, 4 },{ &a::LDA, ABS_X, 4 },{ &a::LDX, ABS_Y, 4 },{ &a::XXX, IMP, 4 },
		{ &a::CPY, IMM, 2 },{ &a::CMP, X_IND, 6 },{ &a::NOP, IMP, 2 },{ &a::XXX, IMP, 8 },{ &a::CPY, ZP, 3 },{ &a::CMP, ZP, 3 },{ &a::DEC, ZP, 5 },{ &a::XXX, IMP, 5 },{ &a::INY, IMP, 2 },{ &a::CMP, IMM, 2 },{ &a::DEX, IMP, 2 },{ &a::XXX, IMP, 2 },{ &a::CPY, ABS, 4 },{ &a::CMP, ABS, 4 },{ &a::DEC, ABS, 6 },{ &a::XXX, IMP, 6 },
		{ &a::BNE, REL, 2 },{ &a::CMP, IND_Y, 5 },{ &a::XXX, IMP, 2 },{ &a::XXX, IMP, 8 },{ &a::NOP, IMP, 4 },{ &a::CMP, ZP_X, 4 },{ &a::DEC, ZP_X, 6 },{ &a::XXX, IMP, 6 },{ &a::CLD, IMP, 2 },{ &a::CMP, ABS_Y, 4 },{ &a::NOP, IMP, 2 },{ &a::XXX, IMP, 7 },{ &a::NOP, IMP, 4 },{ &a::CMP, ABS_X, 4 },{ &a::DEC, ABS_X, 7 },{ &a::XXX, IMP, 7 },
		{ &a::CPX, IMM, 2 },{ &a::SBC, X_IND, 6 },{ &a::NOP, IMP, 2 },{ &a::XXX, IMP, 8 },{ &a::CPX, ZP, 3 },{ &a::SBC, ZP, 3 },{ &a::INC, ZP, 5 },{ &a::XXX, IMP, 5 },{ &a::INX, IMP, 2 },{ &a::SBC, IMM, 2 },{ &a::NOP, IMP, 2 },{ &a::SBC, IMP, 2 },{ &a::CPX, ABS, 4 },{ &a::SBC, ABS, 4 },{ &a::INC, ABS, 6 },{ &a::XXX, IMP, 6 },
		{ &a::BEQ, REL, 2 },{ &a::SBC, IND_Y, 5 },{ &a::XXX, IMP, 2 },{ &a::XXX, IMP, 8 },{ &a::NOP, IMP, 4 },{ &a::SBC, ZP_X, 4 },{ &a::INC, ZP_X, 6 },{ &a::XXX, IMP, 6 },{ &a::SED, IMP, 2 },{ &a::SBC, ABS_Y, 4 },{ &a::NOP, IMP, 2 },{ &a::XXX, IMP, 7 },{ &a::NOP, IMP, 4 },{ &a::SBC, ABS_X, 4 },{ &a::INC, ABS_X, 7 },{ &a::XXX, IMP, 7 },
	};

    this->bus = bus;
    PC = 0x8000;
    S = 0xfd;
    P = 0b00110100;
    A = X = Y = 0x00;
    IR = 0x00;
    bytes = 0x0000;
}

uint8_t CPU::fetch(uint8_t offset){
    uint8_t data = bus->read(PC) + offset;
    PC++;
    return data;
}

void CPU::execute_inst(){
    (this->*inst->exec)();
    sleep_ns((inst->cycles)/NTSC_ClockSpeed);
}

void CPU::fetch_decode_inst(){
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
            bytes = bus->read(bytes); // make sure this is indeed used as the least significant byte
            break;
        case X_IND:
            bytes = convert_to_2byte(fetch(0) + X, 0);
            bytes = bus->read(bytes);
            break;
        case IND_Y:
            bytes = convert_to_2byte(fetch(0), 0);
            bytes = bus->read(bytes); // make sure this is indeed used as the least significant byte
            bytes += Y;
            break;
        default:
            break;
    }
}


/* P register helper functions */
void CPU::set_flag(uint8_t flag, uint8_t res){
    if (res)
        P |= flag;
    else
        P &= ~flag;
}

uint8_t CPU::get_flag_status(uint8_t flag){
    return (P & flag) ? 1 : 0;
}


/* Stack helper functions */
uint8_t CPU::pop() {
    S++;
    uint8_t data = bus->read(STACK_BASE + S);
    if (S >= 0xff)
        std::cout << "WARNING: Stack underflowing! Could be affecting game memory.";

    return data;
}

void CPU::push(uint8_t value){
    bus->write(STACK_BASE + S, value); // could be passing arguments in the wrong order; check that
    if (S <= 0)
        std::cout << "%s\n", "WARNING: Stack overflowing into zero page!";
        
    S--;
}

// DELETE LATER!

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
void CPU::print_state(uint8_t* old) {
    printf("A: %d\n", A);
    printf("X: %d\n", X);
    printf("Y: %d\n", Y);
    printf("S: %d\n", S);
    printf("PC: %d\n", PC);
    print_binary(P);
    printf("IR: %d\n", IR);

    check_array_equal(old, bus->ram);
    printf("----------------------------------------------\n");
}