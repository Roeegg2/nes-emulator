#include "../include/cpu.h"
#include "../include/utils.h"

namespace roee_nes {

    CPU::CPU(Bus* bus)
        : S(0xfd), P(0b00110100), PC(0x00), A(0x00), X(0x00), Y(0x00), IR(0x00), bytes(0x0000) {
        /* I know, its ugly, but I'm using it for now (possibly forever) because its very easy and conventient */
        /* Credit to OLC on the map that served as the foundation of this one */
        using a = CPU;
        lookup =
        {
            { "BRK", &a::BRK, IMM, 7 },{ "ORA", &a::ORA, X_IND, 6 },{ "ILL", &a::ILL, IMP, 2 },{ "ILL", &a::ILL, IMP, 8 },{ "ILL", &a::NOP, IMP, 3 },{ "ORA", &a::ORA, ZP, 3 },{ "ASL", &a::ASL, ZP, 5 },{ "ILL", &a::ILL, IMP, 5 },{ "PHP", &a::PHP, IMP, 3 },{ "ORA", &a::ORA, IMM, 2 },{ "ASL", &a::ASL, ACC, 2 },{ "ILL", &a::ILL, IMP, 2 },{ "ILL", &a::NOP, IMP, 4 },{ "ORA", &a::ORA, ABS, 4 },{ "ASL", &a::ASL, ABS, 6 },{ "ILL", &a::ILL, IMP, 6 },
            { "BPL", &a::BPL, REL, 2 },{ "ORA", &a::ORA, IND_Y, 5 },{ "ILL", &a::ILL, IMP, 2 },{ "ILL", &a::ILL, IMP, 8 },{ "ILL", &a::NOP, IMP, 4 },{ "ORA", &a::ORA, ZP_X, 4 },{ "ASL", &a::ASL, ZP_X, 6 },{ "ILL", &a::ILL, IMP, 6 },{ "CLC", &a::CLC, IMP, 2 },{ "ORA", &a::ORA, ABS_Y, 4 },{ "ILL", &a::NOP, IMP, 2 },{ "ILL", &a::ILL, IMP, 7 },{ "ILL", &a::NOP, IMP, 4 },{ "ORA", &a::ORA, ABS_X, 4 },{ "ASL", &a::ASL, ABS_X, 7 },{ "ILL", &a::ILL, IMP, 7 },
            { "JSR", &a::JSR, ABS, 6 },{ "AND", &a::AND, X_IND, 6 },{ "ILL", &a::ILL, IMP, 2 },{ "ILL", &a::ILL, IMP, 8 },{ "BIT", &a::BIT, ZP, 3 },{ "AND", &a::AND, ZP, 3 },{ "ROL", &a::ROL, ZP, 5 },{ "ILL", &a::ILL, IMP, 5 },{ "PLP", &a::PLP, IMP, 4 },{ "AND", &a::AND, IMM, 2 },{ "ROL", &a::ROL, ACC, 2 },{ "ILL", &a::ILL, IMP, 2 },{ "BIT", &a::BIT, ABS, 4 },{ "AND", &a::AND, ABS, 4 },{ "ROL", &a::ROL, ABS, 6 },{ "ILL", &a::ILL, IMP, 6 },
            { "BMI", &a::BMI, REL, 2 },{ "AND", &a::AND, IND_Y, 5 },{ "ILL", &a::ILL, IMP, 2 },{ "ILL", &a::ILL, IMP, 8 },{ "ILL", &a::NOP, IMP, 4 },{ "AND", &a::AND, ZP_X, 4 },{ "ROL", &a::ROL, ZP_X, 6 },{ "ILL", &a::ILL, IMP, 6 },{ "SEC", &a::SEC, IMP, 2 },{ "AND", &a::AND, ABS_Y, 4 },{ "ILL", &a::NOP, IMP, 2 },{ "ILL", &a::ILL, IMP, 7 },{ "ILL", &a::NOP, IMP, 4 },{ "AND", &a::AND, ABS_X, 4 },{ "ROL", &a::ROL, ABS_X, 7 },{ "ILL", &a::ILL, IMP, 7 },
            { "RTI", &a::RTI, IMP, 6 },{ "EOR", &a::EOR, X_IND, 6 },{ "ILL", &a::ILL, IMP, 2 },{ "ILL", &a::ILL, IMP, 8 },{ "ILL", &a::NOP, IMP, 3 },{ "EOR", &a::EOR, ZP, 3 },{ "LSR", &a::LSR, ZP, 5 },{ "ILL", &a::ILL, IMP, 5 },{ "PHA", &a::PHA, IMP, 3 },{ "EOR", &a::EOR, IMM, 2 },{ "LSR", &a::LSR, ACC, 2 },{ "ILL", &a::ILL, IMP, 2 },{ "JMP", &a::JMP, ABS, 3 },{ "EOR", &a::EOR, ABS, 4 },{ "LSR", &a::LSR, ABS, 6 },{ "ILL", &a::ILL, IMP, 6 },
            { "BVC", &a::BVC, REL, 2 },{ "EOR", &a::EOR, IND_Y, 5 },{ "ILL", &a::ILL, IMP, 2 },{ "ILL", &a::ILL, IMP, 8 },{ "ILL", &a::NOP, IMP, 4 },{ "EOR", &a::EOR, ZP_X, 4 },{ "LSR", &a::LSR, ZP_X, 6 },{ "ILL", &a::ILL, IMP, 6 },{ "CLI", &a::CLI, IMP, 2 },{ "EOR", &a::EOR, ABS_Y, 4 },{ "ILL", &a::NOP, IMP, 2 },{ "ILL", &a::ILL, IMP, 7 },{ "ILL", &a::NOP, IMP, 4 },{ "EOR", &a::EOR, ABS_X, 4 },{ "LSR", &a::LSR, ABS_X, 7 },{ "ILL", &a::ILL, IMP, 7 },
            { "RTS", &a::RTS, IMP, 6 },{ "ADC", &a::ADC, X_IND, 6 },{ "ILL", &a::ILL, IMP, 2 },{ "ILL", &a::ILL, IMP, 8 },{ "ILL", &a::NOP, IMP, 3 },{ "ADC", &a::ADC, ZP, 3 },{ "ROR", &a::ROR, ZP, 5 },{ "ILL", &a::ILL, IMP, 5 },{ "PLA", &a::PLA, IMP, 4 },{ "ADC", &a::ADC, IMM, 2 },{ "ROR", &a::ROR, ACC, 2 },{ "ILL", &a::ILL, IMP, 2 },{ "JMP", &a::JMP, IND, 5 },{ "ADC", &a::ADC, ABS, 4 },{ "ROR", &a::ROR, ABS, 6 },{ "ILL", &a::ILL, IMP, 6 },
            { "BVS", &a::BVS, REL, 2 },{ "ADC", &a::ADC, IND_Y, 5 },{ "ILL", &a::ILL, IMP, 2 },{ "ILL", &a::ILL, IMP, 8 },{ "ILL", &a::NOP, IMP, 4 },{ "ADC", &a::ADC, ZP_X, 4 },{ "ROR", &a::ROR, ZP_X, 6 },{ "ILL", &a::ILL, IMP, 6 },{ "SEI", &a::SEI, IMP, 2 },{ "ADC", &a::ADC, ABS_Y, 4 },{ "ILL", &a::NOP, IMP, 2 },{ "ILL", &a::ILL, IMP, 7 },{ "ILL", &a::NOP, IMP, 4 },{ "ADC", &a::ADC, ABS_X, 4 },{ "ROR", &a::ROR, ABS_X, 7 },{ "ILL", &a::ILL, IMP, 7 },
            { "ILL", &a::NOP, IMP, 2 },{ "STA", &a::STA, X_IND, 6 },{ "ILL", &a::NOP, IMP, 2 },{ "ILL", &a::ILL, IMP, 6 },{ "STY", &a::STY, ZP, 3 },{ "STA", &a::STA, ZP, 3 },{ "STX", &a::STX, ZP, 3 },{ "ILL", &a::ILL, IMP, 3 },{ "DEY", &a::DEY, IMP, 2 },{ "ILL", &a::NOP, IMP, 2 },{ "TXA", &a::TXA, IMP, 2 },{ "ILL", &a::ILL, IMP, 2 },{ "STY", &a::STY, ABS, 4 },{ "STA", &a::STA, ABS, 4 },{ "STX", &a::STX, ABS, 4 },{ "ILL", &a::ILL, IMP, 4 },
            { "BCC", &a::BCC, REL, 2 },{ "STA", &a::STA, IND_Y, 6 },{ "ILL", &a::ILL, IMP, 2 },{ "ILL", &a::ILL, IMP, 6 },{ "STY", &a::STY, ZP_X, 4 },{ "STA", &a::STA, ZP_X, 4 },{ "STX", &a::STX, ZP_Y, 4 },{ "ILL", &a::ILL, IMP, 4 },{ "TYA", &a::TYA, IMP, 2 },{ "STA", &a::STA, ABS_Y, 5 },{ "TXS", &a::TXS, IMP, 2 },{ "ILL", &a::ILL, IMP, 5 },{ "ILL", &a::NOP, IMP, 5 },{ "STA", &a::STA, ABS_X, 5 },{ "ILL", &a::ILL, IMP, 5 },{ "ILL", &a::ILL, IMP, 5 },
            { "LDY", &a::LDY, IMM, 2 },{ "LDA", &a::LDA, X_IND, 6 },{ "LDX", &a::LDX, IMM, 2 },{ "ILL", &a::ILL, IMP, 6 },{ "LDY", &a::LDY, ZP, 3 },{ "LDA", &a::LDA, ZP, 3 },{ "LDX", &a::LDX, ZP, 3 },{ "ILL", &a::ILL, IMP, 3 },{ "TAY", &a::TAY, IMP, 2 },{ "LDA", &a::LDA, IMM, 2 },{ "TAX", &a::TAX, IMP, 2 },{ "ILL", &a::ILL, IMP, 2 },{ "LDY", &a::LDY, ABS, 4 },{ "LDA", &a::LDA, ABS, 4 },{ "LDX", &a::LDX, ABS, 4 },{ "ILL", &a::ILL, IMP, 4 },
            { "BCS", &a::BCS, REL, 2 },{ "LDA", &a::LDA, IND_Y, 5 },{ "ILL", &a::ILL, IMP, 2 },{ "ILL", &a::ILL, IMP, 5 },{ "LDY", &a::LDY, ZP_X, 4 },{ "LDA", &a::LDA, ZP_X, 4 },{ "LDX", &a::LDX, ZP_Y, 4 },{ "ILL", &a::ILL, IMP, 4 },{ "CLV", &a::CLV, IMP, 2 },{ "LDA", &a::LDA, ABS_Y, 4 },{ "TSX", &a::TSX, IMP, 2 },{ "ILL", &a::ILL, IMP, 4 },{ "LDY", &a::LDY, ABS_X, 4 },{ "LDA", &a::LDA, ABS_X, 4 },{ "LDX", &a::LDX, ABS_Y, 4 },{ "ILL", &a::ILL, IMP, 4 },
            { "CPY", &a::CPY, IMM, 2 },{ "CMP", &a::CMP, X_IND, 6 },{ "ILL", &a::NOP, IMP, 2 },{ "ILL", &a::ILL, IMP, 8 },{ "CPY", &a::CPY, ZP, 3 },{ "CMP", &a::CMP, ZP, 3 },{ "DEC", &a::DEC, ZP, 5 },{ "ILL", &a::ILL, IMP, 5 },{ "INY", &a::INY, IMP, 2 },{ "CMP", &a::CMP, IMM, 2 },{ "DEX", &a::DEX, IMP, 2 },{ "ILL", &a::ILL, IMP, 2 },{ "CPY", &a::CPY, ABS, 4 },{ "CMP", &a::CMP, ABS, 4 },{ "DEC", &a::DEC, ABS, 6 },{ "ILL", &a::ILL, IMP, 6 },
            { "BNE", &a::BNE, REL, 2 },{ "CMP", &a::CMP, IND_Y, 5 },{ "ILL", &a::ILL, IMP, 2 },{ "ILL", &a::ILL, IMP, 8 },{ "ILL", &a::NOP, IMP, 4 },{ "CMP", &a::CMP, ZP_X, 4 },{ "DEC", &a::DEC, ZP_X, 6 },{ "ILL", &a::ILL, IMP, 6 },{ "CLD", &a::CLD, IMP, 2 },{ "CMP", &a::CMP, ABS_Y, 4 },{ "NOP", &a::NOP, IMP, 2 },{ "ILL", &a::ILL, IMP, 7 },{ "ILL", &a::NOP, IMP, 4 },{ "CMP", &a::CMP, ABS_X, 4 },{ "DEC", &a::DEC, ABS_X, 7 },{ "ILL", &a::ILL, IMP, 7 },
            { "CPX", &a::CPX, IMM, 2 },{ "SBC", &a::SBC, X_IND, 6 },{ "ILL", &a::NOP, IMP, 2 },{ "ILL", &a::ILL, IMP, 8 },{ "CPX", &a::CPX, ZP, 3 },{ "SBC", &a::SBC, ZP, 3 },{ "INC", &a::INC, ZP, 5 },{ "ILL", &a::ILL, IMP, 5 },{ "INX", &a::INX, IMP, 2 },{ "SBC", &a::SBC, IMM, 2 },{ "NOP", &a::NOP, IMP, 2 },{ "ILL", &a::SBC, IMP, 2 },{ "CPX", &a::CPX, ABS, 4 },{ "SBC", &a::SBC, ABS, 4 },{ "INC", &a::INC, ABS, 6 },{ "ILL", &a::ILL, IMP, 6 },
            { "BEQ", &a::BEQ, REL, 2 },{ "SBC", &a::SBC, IND_Y, 5 },{ "ILL", &a::ILL, IMP, 2 },{ "ILL", &a::ILL, IMP, 8 },{ "ILL", &a::NOP, IMP, 4 },{ "SBC", &a::SBC, ZP_X, 4 },{ "INC", &a::INC, ZP_X, 6 },{ "ILL", &a::ILL, IMP, 6 },{ "SED", &a::SED, IMP, 2 },{ "SBC", &a::SBC, ABS_Y, 4 },{ "NOP", &a::NOP, IMP, 2 },{ "ILL", &a::ILL, IMP, 7 },{ "ILL", &a::NOP, IMP, 4 },{ "SBC", &a::SBC, ABS_X, 4 },{ "INC", &a::INC, ABS_X, 7 },{ "ILL", &a::ILL, IMP, 7 },
        };

        this->bus = bus;
    }

    uint8_t CPU::run_cpu() {
        fetch_decode_inst();
        (this->*inst->exec)();
        // sleep?
        // std::this_thread::sleep_for(std::chrono::nanoseconds(1));

        return inst->cycles;
    }

    uint8_t CPU::Get_from_op(uint8_t offset) const { 
        return bus->cpu_read(PC + offset); 
    }

    void CPU::fetch_decode_inst() { // this is also ugly, might rewrite it in the future
        uint16_t foo; // helper variable to 
        bytes = 0; // clearing operands from last operations
        IR = Get_from_op(0);
        inst = &lookup[IR];
        
        log_PC = PC;
        log_A = A;
        log_X = X;
        log_Y = Y;
        log_P = P;
        log_S = S;

        switch (inst->mode) {
            case REL:
            case IMM:
                log_bytes = bytes = Get_from_op(1);
                PC += 2;
                break;
            case ACC:
            case IMP:
                PC++;
                break;
            case ABS:
                log_bytes = bytes = convert_to_2byte(Get_from_op(1), Get_from_op(2));
                PC += 3;
                break;
            case ZP:
                log_bytes = bytes = convert_to_2byte(Get_from_op(1), 0);
                PC += 2;
                break;
            case ZP_X:
                log_bytes = bytes = convert_to_2byte(Get_from_op(1), 0);
                bytes += X;
                PC += 2;
                bytes &= 0x00ff;
                break;
            case ZP_Y:
                log_bytes = bytes = convert_to_2byte(Get_from_op(1), 0);
                bytes += Y;
                PC += 2;
                bytes &= 0x00ff;
                break;
            case ABS_X:
                log_bytes = bytes = convert_to_2byte(Get_from_op(1), Get_from_op(2));
                bytes += X;
                PC += 3;
                break;
            case ABS_Y:
                log_bytes = bytes = convert_to_2byte(Get_from_op(1), Get_from_op(2));
                bytes += Y;
                PC += 3;
                break;
            case IND:
                log_bytes = foo = convert_to_2byte(Get_from_op(1), Get_from_op(2)); // get the 2 next operands (JMP $op1op2)
                bytes = 0x00ff & bus->cpu_read(foo); // get first part
                foo = ((foo + 1) & 0x00ff) | (foo & 0xff00); // getting the second one (notice the wrap around)
                bytes |= ((0x00ff & bus->cpu_read(foo)) << 8); // getting the final address
                PC += 3; // incrementing PC by 3 (1 opcode, 2 operands)
                break;
            case X_IND:
                log_bytes = foo = convert_to_2byte(Get_from_op(1), 0);
                foo += X;
                bytes = 0x00ff & bus->cpu_read((uint8_t)foo);
                bytes |= ((0x00ff & bus->cpu_read((uint8_t)(foo+1))) << 8);
                PC += 2;
                break;
            case IND_Y:
                log_bytes = foo = convert_to_2byte(Get_from_op(1), 0);
                bytes = 0x00ff & bus->cpu_read((uint8_t)foo);
                bytes |= ((0x00ff & bus->cpu_read((uint8_t)(foo+1))) << 8);
                bytes += Y;
                PC += 2;
                break;
            default:
                PC++;
        }
        
    }


    /* P register helper functions */

    void CPU::set_flag(StatusFlag flag, uint8_t res) {
        if (res)
            P |= flag;
        else
            P &= ~flag;
    }

    uint8_t CPU::get_flag_status(StatusFlag flag) const {
        return ((P & flag) > 0) ? 1 : 0;
    }


    /* Stack helper functions */

    uint8_t CPU::pop() {
        S++;
        uint8_t data = bus->cpu_read(STACK_BASE + S);

        return data;
    }

    void CPU::push(uint8_t value) {
        bus->cpu_write(STACK_BASE + S, value); // could be passing arguments in the wrong order; check that
        S--;
    }
}