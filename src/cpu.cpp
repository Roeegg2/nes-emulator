#include "../include/cpu.h"
#include "../include/utils.h"

namespace roee_nes {

    CPU::CPU(Bus* bus)
        : S(0xfd), P(0b00110100), A(0x00), X(0x00), Y(0x00), IR(0x00), bytes(0x0000) {
        /* I know, its ugly, but I'm using it for now (possibly forever) because its very easy and conventient */
        /* Credit to OLC on the map that served as the foundation of this one */
        using a = CPU;
        lookup = 
        {
            { "brk", &a::BRK, IMM, 7 },{ "ora", &a::ORA, X_IND, 6 },{ "ill", &a::ILL, IMP, 2 },{ "ill", &a::ILL, IMP, 8 },{ "ill", &a::NOP, IMP, 3 },{ "ora", &a::ORA, ZP, 3 },{ "asl", &a::ASL, ZP, 5 },{ "ill", &a::ILL, IMP, 5 },{ "php", &a::PHP, IMP, 3 },{ "ora", &a::ORA, IMM, 2 },{ "asl", &a::ASL, IMP, 2 },{ "ill", &a::ILL, IMP, 2 },{ "ill", &a::NOP, IMP, 4 },{ "ora", &a::ORA, ABS, 4 },{ "asl", &a::ASL, ABS, 6 },{ "ill", &a::ILL, IMP, 6 },
            { "bpl", &a::BPL, REL, 2 },{ "ora", &a::ORA, IND_Y, 5 },{ "ill", &a::ILL, IMP, 2 },{ "ill", &a::ILL, IMP, 8 },{ "ill", &a::NOP, IMP, 4 },{ "ora", &a::ORA, ZP_X, 4 },{ "asl", &a::ASL, ZP_X, 6 },{ "ill", &a::ILL, IMP, 6 },{ "clc", &a::CLC, IMP, 2 },{ "ora", &a::ORA, ABS_Y, 4 },{ "ill", &a::NOP, IMP, 2 },{ "ill", &a::ILL, IMP, 7 },{ "ill", &a::NOP, IMP, 4 },{ "ora", &a::ORA, ABS_X, 4 },{ "asl", &a::ASL, ABS_X, 7 },{ "ill", &a::ILL, IMP, 7 },
            { "jsr", &a::JSR, ABS, 6 },{ "and", &a::AND, X_IND, 6 },{ "ill", &a::ILL, IMP, 2 },{ "ill", &a::ILL, IMP, 8 },{ "bit", &a::BIT, ZP, 3 },{ "and", &a::AND, ZP, 3 },{ "rol", &a::ROL, ZP, 5 },{ "ill", &a::ILL, IMP, 5 },{ "plp", &a::PLP, IMP, 4 },{ "and", &a::AND, IMM, 2 },{ "rol", &a::ROL, IMP, 2 },{ "ill", &a::ILL, IMP, 2 },{ "bit", &a::BIT, ABS, 4 },{ "and", &a::AND, ABS, 4 },{ "rol", &a::ROL, ABS, 6 },{ "ill", &a::ILL, IMP, 6 },
            { "bmi", &a::BMI, REL, 2 },{ "and", &a::AND, IND_Y, 5 },{ "ill", &a::ILL, IMP, 2 },{ "ill", &a::ILL, IMP, 8 },{ "ill", &a::NOP, IMP, 4 },{ "and", &a::AND, ZP_X, 4 },{ "rol", &a::ROL, ZP_X, 6 },{ "ill", &a::ILL, IMP, 6 },{ "sec", &a::SEC, IMP, 2 },{ "and", &a::AND, ABS_Y, 4 },{ "ill", &a::NOP, IMP, 2 },{ "ill", &a::ILL, IMP, 7 },{ "ill", &a::NOP, IMP, 4 },{ "and", &a::AND, ABS_X, 4 },{ "rol", &a::ROL, ABS_X, 7 },{ "ill", &a::ILL, IMP, 7 },
            { "rti", &a::RTI, IMP, 6 },{ "eor", &a::EOR, X_IND, 6 },{ "ill", &a::ILL, IMP, 2 },{ "ill", &a::ILL, IMP, 8 },{ "ill", &a::NOP, IMP, 3 },{ "eor", &a::EOR, ZP, 3 },{ "lsr", &a::LSR, ZP, 5 },{ "ill", &a::ILL, IMP, 5 },{ "pha", &a::PHA, IMP, 3 },{ "eor", &a::EOR, IMM, 2 },{ "lsr", &a::LSR, IMP, 2 },{ "ill", &a::ILL, IMP, 2 },{ "jmp", &a::JMP, ABS, 3 },{ "eor", &a::EOR, ABS, 4 },{ "lsr", &a::LSR, ABS, 6 },{ "ill", &a::ILL, IMP, 6 },
            { "bvc", &a::BVC, REL, 2 },{ "eor", &a::EOR, IND_Y, 5 },{ "ill", &a::ILL, IMP, 2 },{ "ill", &a::ILL, IMP, 8 },{ "ill", &a::NOP, IMP, 4 },{ "eor", &a::EOR, ZP_X, 4 },{ "lsr", &a::LSR, ZP_X, 6 },{ "ill", &a::ILL, IMP, 6 },{ "cli", &a::CLI, IMP, 2 },{ "eor", &a::EOR, ABS_Y, 4 },{ "ill", &a::NOP, IMP, 2 },{ "ill", &a::ILL, IMP, 7 },{ "ill", &a::NOP, IMP, 4 },{ "eor", &a::EOR, ABS_X, 4 },{ "lsr", &a::LSR, ABS_X, 7 },{ "ill", &a::ILL, IMP, 7 },
            { "rts", &a::RTS, IMP, 6 },{ "adc", &a::ADC, X_IND, 6 },{ "ill", &a::ILL, IMP, 2 },{ "ill", &a::ILL, IMP, 8 },{ "ill", &a::NOP, IMP, 3 },{ "adc", &a::ADC, ZP, 3 },{ "ror", &a::ROR, ZP, 5 },{ "ill", &a::ILL, IMP, 5 },{ "pla", &a::PLA, IMP, 4 },{ "adc", &a::ADC, IMM, 2 },{ "ror", &a::ROR, IMP, 2 },{ "ill", &a::ILL, IMP, 2 },{ "jmp", &a::JMP, IND, 5 },{ "adc", &a::ADC, ABS, 4 },{ "ror", &a::ROR, ABS, 6 },{ "ill", &a::ILL, IMP, 6 },
            { "bvs", &a::BVS, REL, 2 },{ "adc", &a::ADC, IND_Y, 5 },{ "ill", &a::ILL, IMP, 2 },{ "ill", &a::ILL, IMP, 8 },{ "ill", &a::NOP, IMP, 4 },{ "adc", &a::ADC, ZP_X, 4 },{ "ror", &a::ROR, ZP_X, 6 },{ "ill", &a::ILL, IMP, 6 },{ "sei", &a::SEI, IMP, 2 },{ "adc", &a::ADC, ABS_Y, 4 },{ "ill", &a::NOP, IMP, 2 },{ "ill", &a::ILL, IMP, 7 },{ "ill", &a::NOP, IMP, 4 },{ "adc", &a::ADC, ABS_X, 4 },{ "ror", &a::ROR, ABS_X, 7 },{ "ill", &a::ILL, IMP, 7 },
            { "ill", &a::NOP, IMP, 2 },{ "sta", &a::STA, X_IND, 6 },{ "ill", &a::NOP, IMP, 2 },{ "ill", &a::ILL, IMP, 6 },{ "sty", &a::STY, ZP, 3 },{ "sta", &a::STA, ZP, 3 },{ "stx", &a::STX, ZP, 3 },{ "ill", &a::ILL, IMP, 3 },{ "dey", &a::DEY, IMP, 2 },{ "ill", &a::NOP, IMP, 2 },{ "txa", &a::TXA, IMP, 2 },{ "ill", &a::ILL, IMP, 2 },{ "sty", &a::STY, ABS, 4 },{ "sta", &a::STA, ABS, 4 },{ "stx", &a::STX, ABS, 4 },{ "ill", &a::ILL, IMP, 4 },
            { "bcc", &a::BCC, REL, 2 },{ "sta", &a::STA, IND_Y, 6 },{ "ill", &a::ILL, IMP, 2 },{ "ill", &a::ILL, IMP, 6 },{ "sty", &a::STY, ZP_X, 4 },{ "sta", &a::STA, ZP_X, 4 },{ "stx", &a::STX, ZP_Y, 4 },{ "ill", &a::ILL, IMP, 4 },{ "tya", &a::TYA, IMP, 2 },{ "sta", &a::STA, ABS_Y, 5 },{ "txs", &a::TXS, IMP, 2 },{ "ill", &a::ILL, IMP, 5 },{ "ill", &a::NOP, IMP, 5 },{ "sta", &a::STA, ABS_X, 5 },{ "ill", &a::ILL, IMP, 5 },{ "ill", &a::ILL, IMP, 5 },
            { "ldy", &a::LDY, IMM, 2 },{ "lda", &a::LDA, X_IND, 6 },{ "ldx", &a::LDX, IMM, 2 },{ "ill", &a::ILL, IMP, 6 },{ "ldy", &a::LDY, ZP, 3 },{ "lda", &a::LDA, ZP, 3 },{ "ldx", &a::LDX, ZP, 3 },{ "ill", &a::ILL, IMP, 3 },{ "tay", &a::TAY, IMP, 2 },{ "lda", &a::LDA, IMM, 2 },{ "tax", &a::TAX, IMP, 2 },{ "ill", &a::ILL, IMP, 2 },{ "ldy", &a::LDY, ABS, 4 },{ "lda", &a::LDA, ABS, 4 },{ "ldx", &a::LDX, ABS, 4 },{ "ill", &a::ILL, IMP, 4 },
            { "bcs", &a::BCS, REL, 2 },{ "lda", &a::LDA, IND_Y, 5 },{ "ill", &a::ILL, IMP, 2 },{ "ill", &a::ILL, IMP, 5 },{ "ldy", &a::LDY, ZP_X, 4 },{ "lda", &a::LDA, ZP_X, 4 },{ "ldx", &a::LDX, ZP_Y, 4 },{ "ill", &a::ILL, IMP, 4 },{ "clv", &a::CLV, IMP, 2 },{ "lda", &a::LDA, ABS_Y, 4 },{ "tsx", &a::TSX, IMP, 2 },{ "ill", &a::ILL, IMP, 4 },{ "ldy", &a::LDY, ABS_X, 4 },{ "lda", &a::LDA, ABS_X, 4 },{ "ldx", &a::LDX, ABS_Y, 4 },{ "ill", &a::ILL, IMP, 4 },
            { "cpy", &a::CPY, IMM, 2 },{ "cmp", &a::CMP, X_IND, 6 },{ "ill", &a::NOP, IMP, 2 },{ "ill", &a::ILL, IMP, 8 },{ "cpy", &a::CPY, ZP, 3 },{ "cmp", &a::CMP, ZP, 3 },{ "dec", &a::DEC, ZP, 5 },{ "ill", &a::ILL, IMP, 5 },{ "iny", &a::INY, IMP, 2 },{ "cmp", &a::CMP, IMM, 2 },{ "dex", &a::DEX, IMP, 2 },{ "ill", &a::ILL, IMP, 2 },{ "cpy", &a::CPY, ABS, 4 },{ "cmp", &a::CMP, ABS, 4 },{ "dec", &a::DEC, ABS, 6 },{ "ill", &a::ILL, IMP, 6 },
            { "bne", &a::BNE, REL, 2 },{ "cmp", &a::CMP, IND_Y, 5 },{ "ill", &a::ILL, IMP, 2 },{ "ill", &a::ILL, IMP, 8 },{ "ill", &a::NOP, IMP, 4 },{ "cmp", &a::CMP, ZP_X, 4 },{ "dec", &a::DEC, ZP_X, 6 },{ "ill", &a::ILL, IMP, 6 },{ "cld", &a::CLD, IMP, 2 },{ "cmp", &a::CMP, ABS_Y, 4 },{ "nop", &a::NOP, IMP, 2 },{ "ill", &a::ILL, IMP, 7 },{ "ill", &a::NOP, IMP, 4 },{ "cmp", &a::CMP, ABS_X, 4 },{ "dec", &a::DEC, ABS_X, 7 },{ "ill", &a::ILL, IMP, 7 },
            { "cpx", &a::CPX, IMM, 2 },{ "sbc", &a::SBC, X_IND, 6 },{ "ill", &a::NOP, IMP, 2 },{ "ill", &a::ILL, IMP, 8 },{ "cpx", &a::CPX, ZP, 3 },{ "sbc", &a::SBC, ZP, 3 },{ "inc", &a::INC, ZP, 5 },{ "ill", &a::ILL, IMP, 5 },{ "inx", &a::INX, IMP, 2 },{ "sbc", &a::SBC, IMM, 2 },{ "nop", &a::NOP, IMP, 2 },{ "ill", &a::SBC, IMP, 2 },{ "cpx", &a::CPX, ABS, 4 },{ "sbc", &a::SBC, ABS, 4 },{ "inc", &a::INC, ABS, 6 },{ "ill", &a::ILL, IMP, 6 },
            { "beq", &a::BEQ, REL, 2 },{ "sbc", &a::SBC, IND_Y, 5 },{ "ill", &a::ILL, IMP, 2 },{ "ill", &a::ILL, IMP, 8 },{ "ill", &a::NOP, IMP, 4 },{ "sbc", &a::SBC, ZP_X, 4 },{ "inc", &a::INC, ZP_X, 6 },{ "ill", &a::ILL, IMP, 6 },{ "sed", &a::SED, IMP, 2 },{ "sbc", &a::SBC, ABS_Y, 4 },{ "nop", &a::NOP, IMP, 2 },{ "ill", &a::ILL, IMP, 7 },{ "ill", &a::NOP, IMP, 4 },{ "sbc", &a::SBC, ABS_X, 4 },{ "inc", &a::INC, ABS_X, 7 },{ "ill", &a::ILL, IMP, 7 },
        };

        this->bus = bus;

        reset();
    }

    uint8_t CPU::run_cpu() {
        fetch_decode_inst();
        (this->*inst->exec)();
        // sleep?

        return inst->cycles;
    }

    uint8_t CPU::Get_from_op(uint8_t offset) const { 
        return bus->cpu_read(PC + offset); 
    }

    void CPU::fetch_decode_inst() { // this is also ugly, might rewrite it in the future
        bytes = 0; // clearing operands from last operations
        IR = Get_from_op(0);
        inst = &lookup[IR];

        log_PC = PC;
        switch (inst->mode) {
            case REL:
            case IMM:
                bytes = Get_from_op(1);
                PC += 2;
                break;
            case ACC:
            case IMP:
                PC++;
                break;
            case ABS:
                bytes = convert_to_2byte(Get_from_op(1), Get_from_op(2));
                PC += 3;
                break;
            case ZP:
                bytes = convert_to_2byte(Get_from_op(1), 0);
                PC += 2;
                break;
            case ZP_X:
                bytes = convert_to_2byte(Get_from_op(1), 0);
                bytes += X;
                PC += 2;
                break;
            case ZP_Y:
                bytes = convert_to_2byte(Get_from_op(1), 0);
                bytes += Y;
                PC += 2;
                break;
            case ABS_X:
                bytes = convert_to_2byte(Get_from_op(1), Get_from_op(2));
                bytes += X;
                PC += 3;
                break;
            case ABS_Y:
                bytes = convert_to_2byte(Get_from_op(1), Get_from_op(2));
                bytes += Y;
                PC += 3;
                break;
            case IND:
                bytes = convert_to_2byte(Get_from_op(1), Get_from_op(2));
                bytes = convert_to_2byte(bus->cpu_read(bytes), bus->cpu_read(bytes + 1)); // might be the other way around
                PC += 3;
                break;
            case X_IND:
                bytes = convert_to_2byte(Get_from_op(1), 0);
                bytes = bus->cpu_read(bytes);
                PC += 2;
                break;
            case IND_Y:
                bytes = convert_to_2byte(Get_from_op(1), 0);
                bytes = bus->cpu_read(bytes);
                bytes += Y;
                PC += 2;
                break;
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