#include <thread>
#include <chrono>

#include "../include/cpu.h"
#include "../include/utils.h"

void CPU::ADC(){
    if (inst->mode != IMM)
        bytes = bus->read(bytes);
    
    uint16_t foo = A + bytes + get_flag_status(CARRY_BIT);
    A = (uint8_t)foo;

    set_flag(ZERO_BIT, A == 0);
    set_flag(NEGATIVE_BIT, A & 0b10000000);
    set_flag(CARRY_BIT, foo != A);
    set_flag(OVERFLOW_BIT, A != (int8_t)foo);
}

void CPU::AND(){
    if (inst->mode != IMM)
        bytes = bus->read(bytes);
    
    A &= bytes;

    set_flag(ZERO_BIT, A == 0);
    set_flag(NEGATIVE_BIT, A & 0b10000000);
}

void CPU::ASL(){
    if (inst->mode == ACC){
        set_flag(CARRY_BIT, A & 0b10000000);
        A = A << 1;
        set_flag(ZERO_BIT, A == 0);
        set_flag(NEGATIVE_BIT, A & 0b10000000);
    }
    else{
        uint8_t data = bus->read(bytes);
        set_flag(CARRY_BIT, data & 0b10000000);

        data = data << 1;
        
        set_flag(ZERO_BIT, data == 0);
        set_flag(NEGATIVE_BIT, data & 0b10000000);
        bus->write(bytes, data);
    }
}

void CPU::BCC(){
    if (!get_flag_status(CARRY_BIT))
        PC += (int8_t)bytes;
}

void CPU::BCS(){
    if (get_flag_status(CARRY_BIT))
        PC += (int8_t)bytes;
}

void CPU::BEQ(){
    if (get_flag_status(ZERO_BIT))
        PC += (int8_t)bytes;
}

void CPU::BIT(){
    bytes = bus->read(bytes);
    uint8_t foo = A & bytes;

    set_flag(ZERO_BIT, foo == 0);
    set_flag(NEGATIVE_BIT, bytes & 0b10000000);
    set_flag(OVERFLOW_BIT, bytes & 0b01000000); // dont understand this
}

void CPU::BMI(){
    if (get_flag_status(NEGATIVE_BIT))
        PC += (int8_t)(bytes & 0x00ff);
}

void CPU::BNE(){
    if (!get_flag_status(ZERO_BIT))
        PC += (int8_t)(bytes & 0x00ff);
}

void CPU::BPL(){
    if (!get_flag_status(NEGATIVE_BIT))
        PC += (int8_t)(bytes & 0x00ff);
}

void CPU::BRK(){
    push(P);
    PC++;
    push((PC >> 8) & 0x00ff);
    push(PC & 0x00ff);
    set_flag(BRK_BIT, 1);

    PC = convert_to_2byte(bus->read(0xffff), bus->read(0xfffe)); // order might be wrong
}

void CPU::BVC(){
    if (!get_flag_status(OVERFLOW_BIT))
        PC += (int8_t)(bytes & 0x00ff);
}

void CPU::BVS(){
    if (get_flag_status(OVERFLOW_BIT))
        PC += (int8_t)(bytes & 0x00ff);
}

void CPU::CLC(){
    set_flag(CARRY_BIT, 0);
}

void CPU::CLD(){
    set_flag(DECIMAL_BIT, 0);
}

void CPU::CLI(){
    set_flag(DISINT_BIT, 0);
}

void CPU::CLV(){
    set_flag(OVERFLOW_BIT, 0);
}

// go over this implemented some things wrong
void CPU::CMP(){
    reg_CMP_actual(&A);
}

void CPU::CPX(){
    reg_CMP_actual(&X);
}

void CPU::CPY(){
    reg_CMP_actual(&Y);
}

void CPU::DEC(){
    mem_INCDEC_actual(-1);
}

void CPU::DEX(){
    reg_INCDEC_actual(&X, -1);
}

void CPU::DEY(){
    reg_INCDEC_actual(&Y, -1);
}

void CPU::EOR(){
    if (inst->mode != IMM)
        bytes = bus->read(bytes);
    
    A ^= (uint8_t)bytes;

    set_flag(ZERO_BIT, A == 0);
    set_flag(NEGATIVE_BIT, A & 0b10000000);
}

void CPU::INC(){
    mem_INCDEC_actual(1);
}

void CPU::INX(){
    reg_INCDEC_actual(&X, 1);
}

void CPU::INY(){
    reg_INCDEC_actual(&Y, 1);
}

void CPU::JMP(){
    if (inst->mode == ABS)
        PC = bytes;
    else
        PC = convert_to_2byte(bus->read(bytes), bus->read(bytes+1)); //WARN: might need to swap the order here
}

void CPU::JSR(){
    PC--; // QUESTION: dont understand why we need to decrement PC. We should point to the next instruction not the current one were executing..
    // push(PC & 0xf00); LATER
    // push(PC & 0x00f);
    PC = bytes;
}

void CPU::LDA(){
    reg_LD_actual(&A);
}

void CPU::LDX(){
    reg_LD_actual(&X);
}

void CPU::LDY(){
    reg_LD_actual(&Y);
}

void CPU::LSR(){ // NOTE: I know there is code duplication here, maybe ill fix later. 
    if (inst->mode == ACC){
        set_flag(CARRY_BIT, A & 0b00000001);
        A = A >> 1;
        set_flag(ZERO_BIT, A == 0);
        set_flag(NEGATIVE_BIT, A & 0b10000000);
    }
    else{
        uint8_t data = bus->read(bytes);
        set_flag(CARRY_BIT, data & 0b00000001);
        data = data >> 1;
        set_flag(ZERO_BIT, data == 0);
        set_flag(NEGATIVE_BIT, data & 0b10000000);
        bus->write(bytes, data);
    }
}

void CPU::NOP(){
    return; // do nothing
}

void CPU::ORA(){
    if (inst->mode != IMM)
        bytes = bus->read(bytes);
    
    A |= (uint8_t)bytes;

    set_flag(ZERO_BIT, A == 0);
    set_flag(NEGATIVE_BIT, A & 0b10000000);
}

void CPU::PHA(){
    push(A);
}

void CPU::PHP(){
    push(P);
}

void CPU::PLA(){
    A = pop();
    set_flag(ZERO_BIT, A == 0);
    set_flag(NEGATIVE_BIT, A & 0b10000000);
}

void CPU::PLP(){
    P = pop();
}

void CPU::ROL(){ // NOTE: I know there is code duplication here, maybe ill fix later. 
    if (inst->mode == ACC){
        uint8_t foo = A & 0b10000000;
        A = A << 1;
        A |= get_flag_status(CARRY_BIT);
        set_flag(CARRY_BIT, foo);
        set_flag(ZERO_BIT, A == 0);
        set_flag(NEGATIVE_BIT, A & 0b10000000);
    }
    else{
        uint8_t data = bus->read(bytes);
        uint8_t foo = data & 0b10000000;
        data = data << 1;
        data |= get_flag_status(CARRY_BIT);
        set_flag(CARRY_BIT, foo);
        set_flag(ZERO_BIT, data == 0);
        set_flag(NEGATIVE_BIT, data & 0b10000000);
        bus->write(bytes, data);
    }
}

void CPU::ROR(){
    if (inst->mode == ACC){
        uint8_t foo = A & 0b00000001;
        A = A >> 1;
        A |= get_flag_status(CARRY_BIT) << 7;
        set_flag(CARRY_BIT, foo);
        set_flag(ZERO_BIT, A == 0);
        set_flag(NEGATIVE_BIT, A & 0b10000000);
    }
    else{
        uint8_t data = bus->read(bytes);
        uint8_t foo = data & 0b00000001;
        data = data >> 1;
        data |= get_flag_status(CARRY_BIT) << 7;
        set_flag(CARRY_BIT, foo);
        set_flag(ZERO_BIT, data == 0);
        set_flag(NEGATIVE_BIT, data & 0b10000000);
        bus->write(bytes, data);
    }

}

void CPU::RTI(){
    P = pop();
    PC = pop();
    PC |= pop() << 8; // might be wrong here!    
}

void CPU::RTS(){
    PC = pop();
    PC |= pop() << 8; // might be wrong here!
    PC++;
}

void CPU::SBC(){
    if (inst->mode != IMM)
        bytes = bus->read(bytes);
    
    uint16_t foo = A - bytes - get_flag_status(CARRY_BIT);
    A = (uint8_t)foo;

    set_flag(ZERO_BIT, A == 0);
    set_flag(NEGATIVE_BIT, A & 0b10000000);
    // PERROR: not sure about the two following implementation
    set_flag(CARRY_BIT, foo != A);
    set_flag(OVERFLOW_BIT, A != (int8_t)foo);
}

void CPU::SEC(){
    set_flag(CARRY_BIT, 1);
}

void CPU::SED(){
    set_flag(DECIMAL_BIT, 1);
}

void CPU::SEI(){
    set_flag(DISINT_BIT, 1);
}

void CPU::STA(){
    bus->write(bytes, A);
}

void CPU::STX(){
    bus->write(bytes, X);
}

void CPU::STY(){
    bus->write(bytes, Y);
}

void CPU::TAX(){
    reg_T_actual(&X, &A);
}

void CPU::TAY(){
    reg_T_actual(&Y, &A);
}

void CPU::TSX(){
    reg_T_actual(&X, &S);
}

void CPU::TXA(){
    reg_T_actual(&A, &X);
}

void CPU::TXS(){
    S = X;
}

void CPU::TYA(){
    reg_T_actual(&A, &Y);
}

void CPU::ILLEGAL(){
    return; // do nothing (for now...)
}

/* Helper functions to avoid code dupilication */

void CPU::reg_CMP_actual(uint8_t* reg){
    if (inst->mode != IMM)
        bytes = bus->read(bytes);

    set_flag(ZERO_BIT, *(reg) == (uint8_t)bytes);
    set_flag(NEGATIVE_BIT, (*(reg) - ((uint8_t)bytes)) & 0b10000000);
    set_flag(CARRY_BIT, *(reg) >= (uint8_t)bytes);
    // set_flag(OVERFLOW_BIT, (int8_t)foo != A);
}

void CPU::reg_INCDEC_actual(uint8_t* reg, uint8_t val){
    (*reg) += val;
    set_flag(ZERO_BIT, (*reg) == 0);
    set_flag(NEGATIVE_BIT, (*reg) & 0b10000000);
}

void CPU::mem_INCDEC_actual(int8_t val){
    uint8_t data = bus->read(bytes);
    bus->write(bytes, data + val);

    set_flag(ZERO_BIT, data == 0);
    set_flag(NEGATIVE_BIT, data & 0b10000000);
    // QUESTION: why there's no need to set the overflow and the carry bit?
}

void CPU::reg_LD_actual(uint8_t* reg){
    if (inst->mode != IMM)
        bytes = bus->read(bytes);
    
    *(reg) = (uint8_t)bytes;

    set_flag(ZERO_BIT, *(reg) == 0);
    set_flag(NEGATIVE_BIT, *(reg) & 0b10000000);
}

void CPU::reg_T_actual(uint8_t* dst_reg, uint8_t* src_reg){
    *(dst_reg) = *(src_reg);
    set_flag(ZERO_BIT, *(dst_reg) == 0);
    set_flag(NEGATIVE_BIT, *(dst_reg) & 0b10000000);
}

/* Interrupts */
// there is code duplication in nmi and irq right now, but later they will be different
void CPU::nmi(){
    push(P);
    push((PC >> 8) & 0x00ff);
    push(PC & 0x00ff);

    PC = convert_to_2byte(bus->read(0xfffa), bus->read(0xfffb));

    set_flag(DISINT_BIT, 1);
    set_flag(BRK_BIT, 0);

    std::chrono::microseconds sleepDuration(get_sleep_time(NMI_CYCLES));
    std::this_thread::sleep_for(sleepDuration);
}

void CPU::reset(){
    PC = convert_to_2byte(bus->read(0xfffc), bus->read(0xfffd));
    S = S-3;

    set_flag(DISINT_BIT, 1);

    std::chrono::microseconds sleepDuration(get_sleep_time(RST_CYCLES));
    std::this_thread::sleep_for(sleepDuration);
}

void CPU::irq(){
    push(P);
    push((PC >> 8) & 0x00ff);
    push(PC & 0x00ff);

    PC = convert_to_2byte(bus->read(0xfffe), bus->read(0xffff));

    set_flag(DISINT_BIT, 1);
    set_flag(BRK_BIT, 0);

    std::chrono::microseconds sleepDuration(get_sleep_time(IRQ_CYCLES));
    std::this_thread::sleep_for(sleepDuration);
}
    