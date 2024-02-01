#include "../include/cpu.h"
#include "../include/utils.h"

namespace roee_nes {

    /* accumulator add with carry */
    void CPU::ADC() {
        if (inst->mode != IMM)
            bytes = bus->cpu_read(bytes);

        uint16_t foo = A + bytes + get_flag_status(CARRY_BIT);
        
        // UNDERSTAND THIS!
        set_flag(OVERFLOW_BIT, (~((uint16_t)A ^ (uint16_t)bytes) & ((uint16_t)A ^ (uint16_t)foo)) & 0x0080);
        A = (uint8_t)foo;

        set_flag(ZERO_BIT, A == 0);
        set_flag(NEGATIVE_BIT, A & 0b10000000);
        set_flag(CARRY_BIT, foo != A);
    }

    /* accumulator bitwise AND */
    void CPU::AND() {
        if (inst->mode != IMM)
            bytes = bus->cpu_read(bytes);

        A &= bytes;

        set_flag(ZERO_BIT, A == 0);
        set_flag(NEGATIVE_BIT, A & 0b10000000);
    }

    /* accumulator shift left */
    void CPU::ASL() {
        if (inst->mode == ACC) {
            set_flag(CARRY_BIT, A & 0b10000000);
            A = A << 1;
            set_flag(ZERO_BIT, A == 0);
            set_flag(NEGATIVE_BIT, A & 0b10000000);
        } else {
            uint8_t data = bus->cpu_read(bytes);
            set_flag(CARRY_BIT, data & 0b10000000);

            data = data << 1;

            set_flag(ZERO_BIT, data == 0);
            set_flag(NEGATIVE_BIT, data & 0b10000000);
            bus->cpu_write(bytes, data);
        }
    }

    /* branch on carry clear */
    void CPU::BCC() {
        if (!get_flag_status(CARRY_BIT))
            PC += (int8_t)bytes;
    }

    /* branch on carry set */
    void CPU::BCS() {
        if (get_flag_status(CARRY_BIT))
            PC += (int8_t)bytes;
    }

    /* branch on equal (zero bit set) */
    void CPU::BEQ() {
        if (get_flag_status(ZERO_BIT))
            PC += (int8_t)bytes;
    }

    /* bit test on accumulator with memory */
    void CPU::BIT() {
        bytes = bus->cpu_read(bytes);
        uint8_t foo = A & bytes;

        set_flag(ZERO_BIT, foo == 0);
        set_flag(NEGATIVE_BIT, bytes & 0b10000000);
        set_flag(OVERFLOW_BIT, bytes & 0b01000000);
    }

    /* branch on minus (negative bit set) */
    void CPU::BMI() {
        if (get_flag_status(NEGATIVE_BIT))
            PC += (int8_t)(bytes & 0x00ff);
    }

    /* branch on not equal (zero bit clear) */
    void CPU::BNE() {
        if (!get_flag_status(ZERO_BIT))
            PC += (int8_t)(bytes & 0x00ff);
    }

    /* branch on plus (negative bit clear) */
    void CPU::BPL() {
        if (!get_flag_status(NEGATIVE_BIT))
            PC += (int8_t)(bytes & 0x00ff);
    }

    /* break - software interrupt request */
    void CPU::BRK() {
        push(P);
        PC++;
        push((PC >> 8) & 0x00ff);
        push(PC & 0x00ff);
        set_flag(BRK_BIT, 1);

        PC = convert_to_2byte(bus->cpu_read(0xffff), bus->cpu_read(0xfffe)); // order might be wrong
    }

    /* branch on overflow clear */
    void CPU::BVC() {
        if (!get_flag_status(OVERFLOW_BIT))
            PC += (int8_t)(bytes & 0x00ff);
    }

    /* branch on overflow set */
    void CPU::BVS() {
        if (get_flag_status(OVERFLOW_BIT))
            PC += (int8_t)(bytes & 0x00ff);
    }

    /* clear carry bit */
    void CPU::CLC() {
        set_flag(CARRY_BIT, 0);
    }

    /* clear decimal bit - not used in the NES's 6502 implementation, but decided to include this anyway*/
    void CPU::CLD() {
        set_flag(DECIMAL_BIT, 0);
    }

    /* clear interrupt disable bit */
    void CPU::CLI() {
        set_flag(DISINT_BIT, 0);
    }

    /* clear overflow bit */
    void CPU::CLV() {
        set_flag(OVERFLOW_BIT, 0);
    }

    /* compare accumulator with memory */
    void CPU::CMP() {
        reg_CMP_actual(&A);
    }

    /* compare X register with memory */
    void CPU::CPX() {
        reg_CMP_actual(&X);
    }

    /* compare Y register with memory */
    void CPU::CPY() {
        reg_CMP_actual(&Y);
    }

    /* decrement memory */
    void CPU::DEC() {
        mem_INCDEC_actual(-1);
    }

    /* decrement X register */
    void CPU::DEX() {
        reg_INCDEC_actual(&X, -1);
    }

    /* decrement Y register */
    void CPU::DEY() {
        reg_INCDEC_actual(&Y, -1);
    }

    /* exclusive OR (XOR) accumulator with memory */
    void CPU::EOR() {
        if (inst->mode != IMM)
            bytes = bus->cpu_read(bytes);

        A ^= (uint8_t)bytes;

        set_flag(ZERO_BIT, A == 0);
        set_flag(NEGATIVE_BIT, A & 0b10000000);
    }

    /* increment memory */
    void CPU::INC() {
        mem_INCDEC_actual(1);
    }

    /* increment X register */
    void CPU::INX() {
        reg_INCDEC_actual(&X, 1);
    }

    /* increment Y register */
    void CPU::INY() {
        reg_INCDEC_actual(&Y, 1);
    }

    /* jump to new location */
    void CPU::JMP() {
        // std::cout << "here is PC for ya: " << PC << "\n";
        PC = bytes;
    }

    /* jump to new location saving return address */
    void CPU::JSR() {
        PC--;
        push((PC >> 8) & 0x00ff);
        push(PC & 0x00ff);
        // set_flag(BRK_BIT, 1); REMOVED FROM NESTEST
        PC = bytes;
    }

    /* load accumulator with value */
    void CPU::LDA() {
        reg_LD_actual(&A);
    }

    /* load X register with value */
    void CPU::LDX() {
        reg_LD_actual(&X);
    }

    /* load Y register with value */
    void CPU::LDY() {
        reg_LD_actual(&Y);
    }

    /* logical shift right (wrapper function to avoid code duplication)*/
    void CPU::LSR() {
        if (inst->mode == ACC)
            LSR_actual(&A);
        else {
            uint8_t data = bus->cpu_read(bytes);
            LSR_actual(&data);
            bus->cpu_write(bytes, data);
        }
    }

    /* nop - do nothing */
    void CPU::NOP() {
        return; // do nothing
    }

    /* bitwise OR accumulator with memory */
    void CPU::ORA() {
        if (inst->mode != IMM)
            bytes = bus->cpu_read(bytes);

        A |= (uint8_t)bytes;

        set_flag(ZERO_BIT, A == 0);
        set_flag(NEGATIVE_BIT, A & 0b10000000);
    }

    /* push accumulator on stack */
    void CPU::PHA() {
        push(A);
    }

    /* push processor status flag on stack */
    void CPU::PHP() {
        set_flag(BRK_BIT, 1);
        push(P);
        set_flag(BRK_BIT, 0);
    }

    /* pop accumulator from stack */
    void CPU::PLA() {
        A = pop();
        set_flag(ZERO_BIT, A == 0);
        set_flag(NEGATIVE_BIT, A & 0b10000000);
    }

    /* pop processor status flag from stack */
    void CPU::PLP() {
        P = pop();
        set_flag(BRK_BIT, 0);
        set_flag(UNUSED_BIT, 1);
    }

    /* rotate left */
    void CPU::ROL() { // NOTE: I know there is code duplication here, maybe ill fix later.
        if (inst->mode == ACC)
            ROL_actual(&A);
        else {
            uint8_t data = bus->cpu_read(bytes);
            ROL_actual(&data);
            bus->cpu_write(bytes, data);
        }
    }

    /* rotate right */
    void CPU::ROR() {
        if (inst->mode == ACC)
            ROR_actual(&A);
        else {
            uint8_t data = bus->cpu_read(bytes);
            ROR_actual(&data);
            bus->cpu_write(bytes, data);
        }
    }

    /* return from interrupt */
    void CPU::RTI() {
        P = pop();
        set_flag(BRK_BIT, 0);
        set_flag(UNUSED_BIT, 1);

        PC = (uint16_t)pop();
        PC |= (uint16_t)(pop() << 8);
    }

    /* return from subroutine */
    void CPU::RTS() {
        PC = pop();
        PC |= (pop() << 8);
        PC++;
    }

    /* subtract with carry */
    void CPU::SBC() { // NOTE: REWRITE ON YOUR OWN LATER!
        if (inst->mode != IMM)
            bytes = bus->cpu_read(bytes);

        uint16_t val = ((uint16_t)bytes) ^ 0x00FF;

        uint32_t foo = (uint16_t)A + val + (uint16_t)get_flag_status(CARRY_BIT);
        set_flag(CARRY_BIT, foo & 0xFF00 || foo == 0); // the || is my addition
	    set_flag(ZERO_BIT, ((foo & 0x00FF) == 0));
	    set_flag(OVERFLOW_BIT, (foo ^ (uint16_t)A) & (foo ^ val) & 0x0080);
	    set_flag(NEGATIVE_BIT, foo & 0x0080);
	    A = foo & 0x00FF;
    }

    /* set carry bit */
    void CPU::SEC() {
        set_flag(CARRY_BIT, 1);
    }

    /* set decimal bit - not used in the NES's 6502 implementation, but decided to include this anyway*/
    void CPU::SED() {
        set_flag(DECIMAL_BIT, 1);
    }

    /* set interrupt disable bit */
    void CPU::SEI() {
        set_flag(DISINT_BIT, 1);
    }

    /* store accumulator in memory */
    void CPU::STA() {
        bus->cpu_write(bytes, A);
    }

    /* store X register in memory */
    void CPU::STX() {
        bus->cpu_write(bytes, X);
    }

    /* store Y register in memory */
    void CPU::STY() {
        bus->cpu_write(bytes, Y);
    }

    /* transfer accumulator to X register */
    void CPU::TAX() {
        reg_T_actual(&X, &A);
    }

    /* transfer accumulator to Y register */
    void CPU::TAY() {
        reg_T_actual(&Y, &A);
    }

    /* transfer stack pointer to X register */
    void CPU::TSX() {
        reg_T_actual(&X, &S);
    }

    /* transfer X register to accumulator */
    void CPU::TXA() {
        reg_T_actual(&A, &X);
    }

    /* transfer X register to stack pointer */
    void CPU::TXS() {
        S = X; // we dont need to set the flags here, so im not calling reg_T_actual
    }

    /* transfer Y register to accumulator */
    void CPU::TYA() {
        reg_T_actual(&A, &Y);
    }

    /* unofficial instructions */
    void CPU::ILL() {
        return; // do nothing (for now...)
    }

    /* Helper functions to avoid code dupilication */

    /* the actual CMP implementation */
    void CPU::reg_CMP_actual(uint8_t* reg) {
        if (inst->mode != IMM)
            bytes = bus->cpu_read(bytes);

        set_flag(ZERO_BIT, *(reg) == (uint8_t)bytes);
        set_flag(NEGATIVE_BIT, (*(reg)-((uint8_t)bytes)) & 0b10000000);
        set_flag(CARRY_BIT, *(reg) >= (uint8_t)bytes);
        // set_flag(OVERFLOW_BIT, (int8_t)foo != A);
    }

    /* the actual INC/DEC implementation */
    void CPU::reg_INCDEC_actual(uint8_t* reg, uint8_t val) {
        (*reg) += val;
        set_flag(ZERO_BIT, (*reg) == 0);
        set_flag(NEGATIVE_BIT, (*reg) & 0b10000000);
    }

    /* the actual INC/DEC implementation */
    void CPU::mem_INCDEC_actual(int8_t val) {
        uint8_t data = bus->cpu_read(bytes);
        data += val;
        bus->cpu_write(bytes, data);

        set_flag(ZERO_BIT, data == 0);
        set_flag(NEGATIVE_BIT, data & 0b10000000);
        // QUESTION: why there's no need to set the overflow and the carry bit?
    }

    /* the actual LD implementation */
    void CPU::reg_LD_actual(uint8_t* reg) {
        if (inst->mode != IMM)
            bytes = bus->cpu_read(bytes);

        *(reg) = (uint8_t)bytes;

        set_flag(ZERO_BIT, *(reg) == 0);
        set_flag(NEGATIVE_BIT, *(reg) & 0b10000000);
    }

    /* the actual registers transfers implementation */
    void CPU::reg_T_actual(uint8_t* dst_reg, uint8_t* src_reg) {
        *(dst_reg) = *(src_reg);
        set_flag(ZERO_BIT, *(dst_reg) == 0);
        set_flag(NEGATIVE_BIT, *(dst_reg) & 0b10000000);
    }

    /* the actual LSR implementation */
    void CPU::LSR_actual(uint8_t* reg) {
        set_flag(CARRY_BIT, *(reg) & 0b00000001);
        *(reg) = *(reg) >> 1;
        set_flag(ZERO_BIT, *(reg) == 0);
        set_flag(NEGATIVE_BIT, *(reg) & 0b10000000);
    }

    /* the actual ROL implementation */
    void CPU::ROL_actual(uint8_t* val) {
        uint8_t foo = *(val) & 0b10000000;
        *(val) = *(val) << 1;
        *(val) |= get_flag_status(CARRY_BIT);
        set_flag(CARRY_BIT, foo);
        set_flag(ZERO_BIT, *(val) == 0);
        set_flag(NEGATIVE_BIT, *(val) & 0b10000000);
    }

    /* the actual ROR implementation */
    void CPU::ROR_actual(uint8_t* val) {
        uint8_t foo = *(val) & 0b00000001;

        *(val) = *(val) >> 1;
        *(val) |= (get_flag_status(CARRY_BIT) << 7);

        set_flag(CARRY_BIT, foo);
        set_flag(ZERO_BIT, *(val) == 0);
        set_flag(NEGATIVE_BIT, *(val) & 0b10000000);
    }

    /* Interrupts */
    // there is code duplication in nmi and irq right now, but later they will be different

    /* non maskable - triggered by the PPU */
    void CPU::nmi() {
        push((PC >> 8) & 0x00ff);
        push(PC & 0x00ff);

        set_flag(DISINT_BIT, 1);
        set_flag(BRK_BIT, 0);
        set_flag(UNUSED_BIT, 1);
        push(P);

        PC = convert_to_2byte(bus->cpu_read(0xfffa), bus->cpu_read(0xfffb));
    }

    /* reset */
    void CPU::reset() {
        PC = convert_to_2byte(bus->cpu_read(0xfffc), bus->cpu_read(0xfffd));
        S = S - 3;

        set_flag(DISINT_BIT, 1);
    }

    /* maskable interrupt*/
    void CPU::irq() {
        if (get_flag_status(DISINT_BIT) == 0) {
            push((PC >> 8) & 0x00ff);
            push(PC & 0x00ff);


            set_flag(DISINT_BIT, 1);
            set_flag(BRK_BIT, 0);
            set_flag(UNUSED_BIT, 1);
            push(P);

            PC = convert_to_2byte(bus->cpu_read(0xfffe), bus->cpu_read(0xffff));
        }
    }

}