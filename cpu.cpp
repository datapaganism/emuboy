#include "cpu.hpp"
#include "bus.hpp"

#include <iostream>

/// <summary>
/// sets the registers of the cpu to their initial power up state, program counter is set to skip the bios program by default
/// </summary>
void CPU::init()
{
    this->registers.pc = 0x100;
    this->registers.a = 0x01;
    this->registers.b = 0x00;
    this->registers.c = 0x13;
    this->registers.d = 0x00;
    this->registers.e = 0xd8;
    this->registers.f = 0xb0;
    this->registers.h = 0x01;
    this->registers.l = 0x4d;
    this->registers.sp = 0xFFFE;
}

CPU::CPU()
{
    this->init();
}


const Byte CPU::get_byte_from_pc()
{
    return this->bus->get_memory(this->registers.pc++);
}



const Word CPU::get_word_from_pc()
{
    Byte temp = this->get_byte_from_pc();
    return (temp << 8) | this->get_byte_from_pc();
}

const Word CPU::get_word_from_pc_lsbf()
{
    return (this->get_byte_from_pc()) | (this->get_byte_from_pc() << 8);
}

void CPU::connect_to_bus(BUS *pBus)
{
    this->bus = pBus;
}

void CPU::checkHalfCarry(int a, int b)
{
    //add only the lower nibbles of the sum to each other
    int sum = (a & 0xf) + (b & 0xf);

    // if the 5th bit is set then we have had a carry over the nibble
    if ((sum & 0x10) == 0x10)
    {
        this->registers.set_flag(h, 1);
        return;
    }
    this->registers.set_flag(h, 0);
    return;   
}

void CPU::checkCarry(const int a, const int b)
{
    
    uint32_t sum = a + b;

    //check if we have Word addition
    if (a > 0xff || b > 0xff)
    {
        if ((sum & 0x10000) == 0x10000)
        {
            this->registers.set_flag(c, 1);
            return;
        }
        this->registers.set_flag(c, 0);
        return;
    }


    if ((sum & 0x100) == 0x100)
    {
        this->registers.set_flag(c, 1);
        return;
    }
    this->registers.set_flag(c, 0);
    return;
}

void CPU::checkHalfBorrow(const int a, const int b)
{
    if ((a & 0xf) >= (b & 0xf))
    {
        this->registers.set_flag(h, 0);
        return;
    }
    this->registers.set_flag(h, 1);
    return;
}

void CPU::checkBorrow(const int a, const int b)
{

    if (a >= b)
    {
        this->registers.set_flag(c, 0);
        return;
    }
    this->registers.set_flag(c, 1);
    return;
    
}

int CPU::ins_LD_nn_n(Byte* registerOne, Byte value)
{
    //LD nn,n
    *registerOne = value;
    return 8;
}

int CPU::ins_LD_r1_r2(Byte* registerOne, Word address, Byte* registerTwo, Byte value)
{
    if (registerOne && registerTwo)
    {
        *registerOne = *registerTwo;
        return 4;
    }
    if (registerOne && address)
    {
        *registerOne = this->bus->get_memory(address);
        return 8;
    }
    if (address && registerTwo)
    {
        this->bus->set_memory(address, *registerTwo);
        return 8;
    }
    // if (address && value)
    
    this->bus->set_memory(address, value);
    return 12;
    
}

int CPU::ins_LD_r1_nn(Byte* registerOne, const Word address, const int cyclesUsed)
{
    // Cycles used is a param since it can change
    //take value pointed to by nn and put into register one,
    // LD a,(nn), mem byte eg: FA 34 12, ld a,(1234)
    // check memory at address 0x1234, put that data into register one

    *registerOne = this->bus->get_memory(address);
    return cyclesUsed;
};


// Take value from registerOne, place at memory location 
int CPU::ins_LD_nn_r1(Word address, Byte* registerOne)
{
    this->bus->set_memory(address, *registerOne);
    return 16;
};
int CPU::ins_LDDI_nn_r1(Word address, const Byte* registerOne, Byte* registerTwo, Byte* registerThree, const int addSubValue)
{
    this->bus->set_memory(address, *registerOne);
    this->registers.set_word(registerTwo, registerThree, (this->registers.get_word(registerTwo, registerThree) + addSubValue));
    return 8;
};
int CPU::ins_LDDI_r1_nn(Byte* registerOne, const Word address, Byte* registerTwo, Byte* registerThree, const int addSubValue)
{
    *registerOne = this->bus->get_memory(address);
    this->registers.set_word(registerTwo, registerThree, (this->registers.get_word(registerTwo, registerThree) + addSubValue));
    return 8;
}
int CPU::ins_LD_n_nn(Word* wordRegister, Byte* registerOne, Byte* registerTwo, const Word value)
{
    if (wordRegister)
    {
        *wordRegister = value;
        return 12;
    }
    
    this->registers.set_word(registerOne, registerTwo, value);
    return 12;
}
int CPU::ins_LD_nn_nn(Word* wordRegisterOne, const Word value)
{
    *wordRegisterOne = value;
    return 8;
}
int CPU::ins_LDHL_SP_n(Byte* wordRegisterNibbleHi, Byte* wordRegisterNibbleLo, const Word stackPointerValue, const Byte value)
{
    Word sum = stackPointerValue + value;
    this->registers.set_flag(z, 0);
    this->registers.set_flag(n, 0);
    this->checkCarry(stackPointerValue, value);
    this->checkHalfCarry(stackPointerValue, value);

    this->registers.set_word(wordRegisterNibbleHi, wordRegisterNibbleLo, sum);
    return 12;
}

int CPU::ins_LD_nn_SP(const Word address, const Word stackPointerValue)
{
    //place value of SP into memory at address and address + 1
    this->bus->set_memory_word(address, stackPointerValue);
    return 20;
}

int CPU::ins_PUSH_nn(const Word wordRegisterValue)
{
    this->bus->set_memory_word(this->registers.sp, wordRegisterValue);
    this->registers.sp -= 2;
    return 16;
}

int CPU::ins_POP_nn(Byte* registerOne, Byte* registerTwo)
{
    *registerOne = (this->registers.sp & 0x00ff);
    *registerTwo = ((this->registers.sp & 0xff00) >> 8);
    this->registers.sp += 2;
    return 12;
}

int CPU::ins_ADD_A_n(const Byte* registerOne, const Byte immediateValue)
{
    if (registerOne)
    {
        //check carry and half carry flags, I realise this is out of order but it should work the same.
        this->checkHalfCarry(this->registers.a, *registerOne);
        this->checkCarry(this->registers.a, *registerOne);

        // perform addition
        this->registers.a += *registerOne;

        //evaluate z flag an clear the n flag
        (this->registers.a == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
        this->registers.set_flag(n, 0);
       
        return 4;
    }
    // else if immediate value passed

    //check carry and half carry flags, I realise this is out of order but it should work the same.
    this->checkHalfCarry(this->registers.a, immediateValue);
    this->checkCarry(this->registers.a, immediateValue);

    // perform addition
    this->registers.a += immediateValue;

    //evaluate z flag an clear the n flag
    (this->registers.a == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
    this->registers.set_flag(n, 0);

    return 8;
}

int CPU::ins_ADC_A_n(const Byte* registerOne, const Byte immediateValue)
{
    // this function is a bit of cheat there is no point rewriting logic,
    // we just call the normal addition instruction but pass the register as an immediate value + the carry flag
    // the add ins would return 8 since we are passing an immediate but we can disregard that and return 4;
    if (registerOne)
    {
        this->ins_ADD_A_n(nullptr, *registerOne + this->registers.get_flag(c));
        return 4;
    }
    this->ins_ADD_A_n(nullptr, immediateValue + this->registers.get_flag(c));
    // while the call above would return 8 anyways, it is clearer to be explicit
    return 8;
}

int CPU::ins_SUB_n(const Byte* registerOne, const Byte immediateValue)
{
    if (registerOne)
    {
        //check carry and half carry flags, I realise this is out of order but it should work the same.
        this->checkHalfBorrow(this->registers.a, *registerOne);
        this->checkBorrow(this->registers.a, *registerOne);

        // perform addition
        this->registers.a -= *registerOne;

        //evaluate z flag an clear the n flag
        (this->registers.a == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
        this->registers.set_flag(n, 1);

        return 4;
    }
    // else if immediate value passed

    //check carry and half carry flags, I realise this is out of order but it should work the same.
    this->checkHalfBorrow(this->registers.a, immediateValue);
    this->checkBorrow(this->registers.a, immediateValue);

    // perform addition
    this->registers.a -= immediateValue;

    //evaluate z flag an clear the n flag
    (this->registers.a == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
    this->registers.set_flag(n, 1);

    return 8;
}

int CPU::ins_SBC_A_n(const Byte* registerOne, const Byte immediateValue)
{
    if (registerOne)
    {
        this->ins_SUB_n(nullptr, *registerOne + this->registers.get_flag(c));
        return 4;
    }
    this->ins_SUB_n(nullptr, immediateValue + this->registers.get_flag(c));
    return 8;
}

int CPU::ins_AND_n(const Byte* registerOne, const Byte immediateValue)
{
    if (registerOne)
    {
        this->registers.a &= *registerOne;

        //evaluate z flag an clear the n flag
        (this->registers.a == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
        this->registers.set_flag(h, 1);
        this->registers.set_flag(n, 0);
        this->registers.set_flag(c, 0);

        return 4;
    }
    // else if immediate value passed

    this->registers.a &= immediateValue;

    //evaluate z flag an clear the n flag
    (this->registers.a == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
    this->registers.set_flag(h, 1);
    this->registers.set_flag(n, 0);
    this->registers.set_flag(c, 0);


    return 8;
}

int CPU::ins_OR_n(const Byte* registerOne, const Byte immediateValue)
{
    if (registerOne)
    {
        this->registers.a |= *registerOne;

        //evaluate z flag an clear the n flag
        (this->registers.a == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
        this->registers.set_flag(h, 0);
        this->registers.set_flag(n, 0);
        this->registers.set_flag(c, 0);

        return 4;
    }
    // else if immediate value passed

    this->registers.a |= immediateValue;

    //evaluate z flag an clear the n flag
    (this->registers.a == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
    this->registers.set_flag(h, 0);
    this->registers.set_flag(n, 0);
    this->registers.set_flag(c, 0);


    return 8;
}
int CPU::ins_XOR_n(const Byte* registerOne, const Byte immediateValue)
{
    if (registerOne)
    {
        this->registers.a ^= *registerOne;

        //evaluate z flag an clear the n flag
        (this->registers.a == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
        this->registers.set_flag(h, 0);
        this->registers.set_flag(n, 0);
        this->registers.set_flag(c, 0);

        return 4;
    }
    // else if immediate value passed

    this->registers.a ^= immediateValue;

    //evaluate z flag an clear the n flag
    (this->registers.a == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
    this->registers.set_flag(h, 0);
    this->registers.set_flag(n, 0);
    this->registers.set_flag(c, 0);


    return 8;
}

int CPU::ins_CP_n(const Byte* registerOne, const Byte immediateValue)
{
    if (registerOne)
    {
        //check carry and half carry flags, I realise this is out of order but it should work the same.
        this->checkHalfBorrow(this->registers.a, *registerOne);
        this->checkBorrow(this->registers.a, *registerOne);


        //evaluate z flag an clear the n flag
        (this->registers.a == *registerOne) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
        this->registers.set_flag(n, 1);

        return 4;
    }
    // else if immediate value passed

    //check carry and half carry flags, I realise this is out of order but it should work the same.
    this->checkHalfBorrow(this->registers.a, immediateValue);
    this->checkBorrow(this->registers.a, immediateValue);

    //evaluate z flag an clear the n flag
    (this->registers.a == immediateValue) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
    this->registers.set_flag(n, 1);

    return 8;
}

int CPU::ins_INC_n(Byte* registerOne, Word address)
{
    if (registerOne)
    {
        //check carry and half carry flags, I realise this is out of order but it should work the same.
        this->checkHalfCarry(*registerOne, 1);
        
        // perform addition
        (*registerOne)++;

        //evaluate z flag an clear the n flag
        (this->registers.a == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
        this->registers.set_flag(n, 0);

        return 4;
    }
    // else if immediate value passed

    //check carry and half carry flags, I realise this is out of order but it should work the same.
    this->checkHalfCarry(this->bus->get_memory(address), 1);
    
    // perform addition
    this->bus->set_memory(address, (this->bus->get_memory(address) + 1));
    //evaluate z flag an clear the n flag
    (this->registers.a == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
    this->registers.set_flag(n, 0);

    return 12;
}

int CPU::ins_DEC_n(Byte* registerOne, Word address)
{
    if (registerOne)
    {
        //check carry and half carry flags, I realise this is out of order but it should work the same.
        this->checkHalfBorrow(*registerOne, 1);

        // perform addition
        (*registerOne)--;

        //evaluate z flag an clear the n flag
        (this->registers.a == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
        this->registers.set_flag(n, 1);

        return 4;
    }
    // else if immediate value passed

    //check carry and half carry flags, I realise this is out of order but it should work the same.
    this->checkHalfBorrow(this->bus->get_memory(address), 1);

    // perform addition
    this->bus->set_memory(address, (this->bus->get_memory(address) - 1 ));
    //evaluate z flag an clear the n flag
    (this->registers.a == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
    this->registers.set_flag(n, 1);

    return 12;
}




int CPU::fetch_decode_execute()
{
    // the program counter holds the address in memory for either instruction or data for an instruction.
    Byte opcode = this->get_byte_from_pc();
    int cyclesUsed = 0;

    //skipped
    // 0xea lda (nn),a : 16

    switch (opcode) 
    {
        case 0x00:{ } break;
        case 0x01: { cyclesUsed = this->ins_LD_n_nn(nullptr, &this->registers.b, &this->registers.c, this->get_word_from_pc_lsbf()); } break;
        case 0x02: { cyclesUsed = this->ins_LD_r1_r2(nullptr, this->registers.get_BC(), &this->registers.a); } break;
        case 0x03:{ } break;
        case 0x04:{ cyclesUsed = this->ins_INC_n(&this->registers.b); } break;
        case 0x05:{ cyclesUsed = this->ins_DEC_n(&this->registers.b); } break;
        case 0x06: { cyclesUsed = this->ins_LD_nn_n(&this->registers.b, this->get_byte_from_pc()); } break;
        case 0x07:{ } break;
        case 0x08: { cyclesUsed = this->ins_LD_nn_SP(this->get_word_from_pc_lsbf(), this->registers.sp); } break;
        case 0x09:{ } break;
        case 0x0A: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, this->registers.get_BC()); } break;
        case 0x0B:{ } break;
        case 0x0C:{ cyclesUsed = this->ins_INC_n(&this->registers.c); } break;
        case 0x0D:{ cyclesUsed = this->ins_DEC_n(&this->registers.c); } break;
        case 0x0E: { cyclesUsed = this->ins_LD_nn_n(&this->registers.c, this->get_byte_from_pc()); } break;
        case 0x0F:{ } break;
        
        case 0x10:{ } break;
        case 0x11: { cyclesUsed = this->ins_LD_n_nn(nullptr, &this->registers.d, &this->registers.e, this->get_word_from_pc_lsbf()); } break;
        case 0x12: { cyclesUsed = this->ins_LD_r1_r2(nullptr, this->registers.get_DE(), &this->registers.a); } break;
        case 0x13:{ } break;
        case 0x14:{ cyclesUsed = this->ins_INC_n(&this->registers.d); } break;
        case 0x15:{ cyclesUsed = this->ins_DEC_n(&this->registers.d); } break;
        case 0x16: { cyclesUsed = this->ins_LD_nn_n(&this->registers.d, this->get_byte_from_pc()); } break;
        case 0x17:{ } break;
        case 0x18:{ } break;
        case 0x19:{ } break;
        case 0x1A: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, this->registers.get_DE()); } break;
        case 0x1B:{ } break;
        case 0x1C:{ cyclesUsed = this->ins_INC_n(&this->registers.e); } break;
        case 0x1D:{ cyclesUsed = this->ins_DEC_n(&this->registers.e); } break;
        case 0x1E: { cyclesUsed = this->ins_LD_nn_n(&this->registers.e, this->get_byte_from_pc()); } break;
        case 0x1F:{ } break;
          
        case 0x20:{ } break;
        case 0x21: { cyclesUsed = this->ins_LD_n_nn(nullptr, &this->registers.h, &this->registers.l, this->get_word_from_pc_lsbf()); } break;
        case 0x22: { cyclesUsed = this->ins_LDDI_nn_r1(this->registers.get_HL(), &this->registers.a, &this->registers.h, &this->registers.l, +1); } break;
        case 0x23:{ } break;
        case 0x24: { cyclesUsed = this->ins_INC_n(&this->registers.h); } break;
        case 0x25:{ cyclesUsed = this->ins_DEC_n(&this->registers.h); } break;
        case 0x26: { cyclesUsed = this->ins_LD_nn_n(&this->registers.h, this->get_byte_from_pc()); } break;
        case 0x27:{ } break;
        case 0x28:{ } break;
        case 0x29:{ } break;
        case 0x2A: { cyclesUsed = this->ins_LDDI_r1_nn(&this->registers.a, this->registers.get_HL(), &this->registers.h, &this->registers.l, +1); } break;
        case 0x2B:{ } break;
        case 0x2C:{ cyclesUsed = this->ins_INC_n(&this->registers.l); } break;
        case 0x2D:{ cyclesUsed = this->ins_DEC_n(&this->registers.l); } break;
        case 0x2E: { cyclesUsed = this->ins_LD_nn_n(&this->registers.l, this->get_byte_from_pc()); } break;
        case 0x2F:{ } break;

        case 0x30:{ } break;
        case 0x31: { cyclesUsed = this->ins_LD_n_nn(&this->registers.sp, nullptr,nullptr, this->get_word_from_pc_lsbf()); } break;
        case 0x32: { cyclesUsed = this->ins_LDDI_nn_r1(this->registers.get_HL(), &this->registers.a, &this->registers.h, &this->registers.l, -1); } break;
        case 0x33:{ } break;
        case 0x34:{ cyclesUsed = this->ins_INC_n(nullptr, this->registers.get_HL()); } break;
        case 0x35:{ cyclesUsed = this->ins_DEC_n(nullptr, this->registers.get_HL()); } break;
        case 0x36: { cyclesUsed = this->ins_LD_r1_r2(nullptr, this->registers.get_HL(), nullptr, this->get_byte_from_pc()); } break;
        case 0x37:{ } break;
        case 0x38:{ } break;
        case 0x39:{ } break;
        case 0x3A:{ } break;
        case 0x3B:{ } break;
        case 0x3C: { cyclesUsed = this->ins_INC_n(&this->registers.a); } break;
        case 0x3D: { cyclesUsed = this->ins_DEC_n(&this->registers.a); } break;
        case 0x3E: { cyclesUsed = this->ins_LD_nn_n(&this->registers.a, this->get_byte_from_pc()); } break;
        case 0x3F:{ } break;
         
        case 0x40: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.b, NULL, &this->registers.b); } break;
        case 0x41: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.b, NULL, &this->registers.c); } break;
        case 0x42: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.b, NULL, &this->registers.d); } break;
        case 0x43: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.b, NULL, &this->registers.e); } break;
        case 0x44: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.b, NULL, &this->registers.h); } break;
        case 0x45: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.b, NULL, &this->registers.l); } break;
        case 0x46: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.b, this->registers.get_HL()); } break;
        case 0x47: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.b, NULL, &this->registers.a); } break;
        case 0x48: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.c, NULL, &this->registers.b); } break;
        case 0x49: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.c, NULL, &this->registers.c); } break;
        case 0x4A: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.c, NULL, &this->registers.d); } break;
        case 0x4B: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.c, NULL, &this->registers.e); } break;
        case 0x4C: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.c, NULL, &this->registers.h); } break;
        case 0x4D: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.c, NULL, &this->registers.l); } break;
        case 0x4E: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.c, this->registers.get_HL()); } break;
        case 0x4F: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.c, NULL, &this->registers.a); } break;

        case 0x50: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.d, NULL, &this->registers.b); } break;
        case 0x51: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.d, NULL, &this->registers.c); } break;
        case 0x52: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.d, NULL, &this->registers.d); } break;
        case 0x53: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.d, NULL, &this->registers.e); } break;
        case 0x54: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.d, NULL, &this->registers.h); } break;
        case 0x55: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.d, NULL, &this->registers.l); } break;
        case 0x56: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.d, this->registers.get_HL()); } break;
        case 0x57: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.d, NULL, &this->registers.a); } break;
        case 0x58: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.e, NULL, &this->registers.b); } break;
        case 0x59: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.e, NULL, &this->registers.c); } break;
        case 0x5A: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.e, NULL, &this->registers.d); } break;
        case 0x5B: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.e, NULL, &this->registers.e); } break;
        case 0x5C: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.e, NULL, &this->registers.h); } break;
        case 0x5D: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.e, NULL, &this->registers.l); } break;
        case 0x5E: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.e, this->registers.get_HL()); } break;
        case 0x5F: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.e, NULL, &this->registers.a); } break;
        
        case 0x60: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.h, NULL, &this->registers.b); } break;
        case 0x61: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.h, NULL, &this->registers.c); } break;
        case 0x62: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.h, NULL, &this->registers.d); } break;
        case 0x63: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.h, NULL, &this->registers.e); } break;
        case 0x64: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.h, NULL, &this->registers.h); } break;
        case 0x65: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.h, NULL, &this->registers.l); } break;
        case 0x66: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.h, this->registers.get_HL()); } break;
        case 0x67: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.h, NULL, &this->registers.a); } break;
        case 0x68: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.l, NULL, &this->registers.b); } break;
        case 0x69: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.l, NULL, &this->registers.c); } break;
        case 0x6A: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.l, NULL, &this->registers.d); } break;
        case 0x6B: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.l, NULL, &this->registers.e); } break;
        case 0x6C: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.l, NULL, &this->registers.h); } break;
        case 0x6D: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.l, NULL, &this->registers.l); } break;
        case 0x6E: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.l, this->registers.get_HL()); } break;
        case 0x6F: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.l, NULL, &this->registers.a); } break;

        case 0x70: { cyclesUsed = this->ins_LD_r1_r2(nullptr,this->registers.get_HL(), &this->registers.b); } break;
        case 0x71: { cyclesUsed = this->ins_LD_r1_r2(nullptr,this->registers.get_HL(), &this->registers.c); } break;
        case 0x72: { cyclesUsed = this->ins_LD_r1_r2(nullptr,this->registers.get_HL(), &this->registers.d); } break;
        case 0x73: { cyclesUsed = this->ins_LD_r1_r2(nullptr,this->registers.get_HL(), &this->registers.e); } break;
        case 0x74: { cyclesUsed = this->ins_LD_r1_r2(nullptr,this->registers.get_HL(), &this->registers.h); } break;
        case 0x75: { cyclesUsed = this->ins_LD_r1_r2(nullptr,this->registers.get_HL(), &this->registers.l); } break;
        case 0x76:{ } break;
        case 0x77: { cyclesUsed = this->ins_LD_r1_r2(nullptr,this->registers.get_HL(), &this->registers.a); } break;
        case 0x78: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, NULL, &this->registers.b); } break;
        case 0x79: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, NULL, &this->registers.c); } break;
        case 0x7A: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, NULL, &this->registers.d); } break;
        case 0x7B: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, NULL, &this->registers.e); } break;
        case 0x7C: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, NULL, &this->registers.h); } break;
        case 0x7D: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, NULL, &this->registers.l); } break;
        case 0x7E: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.a,this->registers.get_HL()); } break;
        case 0x7F: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, NULL, &this->registers.a); } break;
            
        case 0x80: { cyclesUsed = this->ins_ADD_A_n(&this->registers.b); } break;
        case 0x81: { cyclesUsed = this->ins_ADD_A_n(&this->registers.c); } break;
        case 0x82: { cyclesUsed = this->ins_ADD_A_n(&this->registers.d); } break;
        case 0x83: { cyclesUsed = this->ins_ADD_A_n(&this->registers.e); } break;
        case 0x84: { cyclesUsed = this->ins_ADD_A_n(&this->registers.h); } break;
        case 0x85: { cyclesUsed = this->ins_ADD_A_n(&this->registers.l); } break;
        case 0x86: { cyclesUsed = this->ins_ADD_A_n(nullptr, this->bus->get_memory(this->registers.get_HL())); } break;
        case 0x87: { cyclesUsed = this->ins_ADD_A_n(&this->registers.a); } break;
        case 0x88: { cyclesUsed = this->ins_ADC_A_n(&this->registers.b); } break;
        case 0x89: { cyclesUsed = this->ins_ADC_A_n(&this->registers.c); } break;
        case 0x8A: { cyclesUsed = this->ins_ADC_A_n(&this->registers.d); } break;
        case 0x8B: { cyclesUsed = this->ins_ADC_A_n(&this->registers.e); } break;
        case 0x8C: { cyclesUsed = this->ins_ADC_A_n(&this->registers.h); } break;
        case 0x8D: { cyclesUsed = this->ins_ADC_A_n(&this->registers.l); } break;
        case 0x8E: { cyclesUsed = this->ins_ADC_A_n(nullptr, this->bus->get_memory(this->registers.get_HL())); } break;
        case 0x8F: { cyclesUsed = this->ins_ADC_A_n(&this->registers.a); } break;

        case 0x90: { cyclesUsed = this->ins_SUB_n(&this->registers.b); } break;
        case 0x91: { cyclesUsed = this->ins_SUB_n(&this->registers.c); } break;
        case 0x92: { cyclesUsed = this->ins_SUB_n(&this->registers.d); } break;
        case 0x93: { cyclesUsed = this->ins_SUB_n(&this->registers.e); } break;
        case 0x94: { cyclesUsed = this->ins_SUB_n(&this->registers.h); } break;
        case 0x95: { cyclesUsed = this->ins_SUB_n(&this->registers.l); } break;
        case 0x96: { cyclesUsed = this->ins_SUB_n(nullptr, this->bus->get_memory(this->registers.get_HL())); } break;
        case 0x97: { cyclesUsed = this->ins_SUB_n(&this->registers.a); } break;
        case 0x98: { cyclesUsed = this->ins_SBC_A_n(&this->registers.b); } break;
        case 0x99: { cyclesUsed = this->ins_SBC_A_n(&this->registers.c); } break;
        case 0x9A: { cyclesUsed = this->ins_SBC_A_n(&this->registers.d); } break;
        case 0x9B: { cyclesUsed = this->ins_SBC_A_n(&this->registers.e); } break;
        case 0x9C: { cyclesUsed = this->ins_SBC_A_n(&this->registers.h); } break;
        case 0x9D: { cyclesUsed = this->ins_SBC_A_n(&this->registers.l); } break;
        case 0x9E: { cyclesUsed = this->ins_SBC_A_n(nullptr, this->bus->get_memory(this->registers.get_HL())); } break;
        case 0x9F: { cyclesUsed = this->ins_SBC_A_n(&this->registers.a); } break;

        case 0xA0: { cyclesUsed = this->ins_AND_n(&this->registers.b); } break;
        case 0xA1: { cyclesUsed = this->ins_AND_n(&this->registers.c); } break;
        case 0xA2: { cyclesUsed = this->ins_AND_n(&this->registers.d); } break;
        case 0xA3: { cyclesUsed = this->ins_AND_n(&this->registers.e); } break;
        case 0xA4: { cyclesUsed = this->ins_AND_n(&this->registers.h); } break;
        case 0xA5: { cyclesUsed = this->ins_AND_n(&this->registers.l); } break;
        case 0xA6: { cyclesUsed = this->ins_AND_n(nullptr, this->bus->get_memory(this->registers.get_HL())); } break;
        case 0xA7: { cyclesUsed = this->ins_AND_n(&this->registers.a); } break;
        case 0xA8: { cyclesUsed = this->ins_XOR_n(&this->registers.b); } break;
        case 0xA9: { cyclesUsed = this->ins_XOR_n(&this->registers.c); } break;
        case 0xAA: { cyclesUsed = this->ins_XOR_n(&this->registers.d); } break;
        case 0xAB: { cyclesUsed = this->ins_XOR_n(&this->registers.e); } break;
        case 0xAC: { cyclesUsed = this->ins_XOR_n(&this->registers.h); } break;
        case 0xAD: { cyclesUsed = this->ins_XOR_n(&this->registers.l); } break;
        case 0xAE: { cyclesUsed = this->ins_XOR_n(nullptr, this->bus->get_memory(this->registers.get_HL())); } break;
        case 0xAF: { cyclesUsed = this->ins_XOR_n(&this->registers.a); } break;

        case 0xB0: { cyclesUsed = this->ins_OR_n(&this->registers.b); } break;
        case 0xB1: { cyclesUsed = this->ins_OR_n(&this->registers.c); } break;
        case 0xB2: { cyclesUsed = this->ins_OR_n(&this->registers.d); } break;
        case 0xB3: { cyclesUsed = this->ins_OR_n(&this->registers.e); } break;
        case 0xB4: { cyclesUsed = this->ins_OR_n(&this->registers.h); } break;
        case 0xB5: { cyclesUsed = this->ins_OR_n(&this->registers.l); } break;
        case 0xB6: { cyclesUsed = this->ins_OR_n(nullptr, this->bus->get_memory(this->registers.get_HL())); } break;
        case 0xB7: { cyclesUsed = this->ins_OR_n(&this->registers.a); } break;
        case 0xB8: { cyclesUsed = this->ins_CP_n(&this->registers.b); } break;
        case 0xB9: { cyclesUsed = this->ins_CP_n(&this->registers.c); } break;
        case 0xBA: { cyclesUsed = this->ins_CP_n(&this->registers.d); } break;
        case 0xBB: { cyclesUsed = this->ins_CP_n(&this->registers.e); } break;
        case 0xBC: { cyclesUsed = this->ins_CP_n(&this->registers.h); } break;
        case 0xBD: { cyclesUsed = this->ins_CP_n(&this->registers.l); } break;
        case 0xBE: { cyclesUsed = this->ins_CP_n(nullptr, this->bus->get_memory(this->registers.get_HL())); } break;
        case 0xBF: { cyclesUsed = this->ins_CP_n(&this->registers.a); } break;

        case 0xC0:{ } break;
        case 0xC1:{ cyclesUsed = this->ins_POP_nn(&this->registers.b, &this->registers.c); } break;
        case 0xC2:{ } break;
        case 0xC3:{ } break;
        case 0xC4:{ } break;
        case 0xC5:{ cyclesUsed = this->ins_PUSH_nn(this->registers.get_BC()); } break;
        case 0xC6:{ cyclesUsed = this->ins_ADD_A_n(nullptr, this->get_byte_from_pc()); } break;
        case 0xC7:{ } break;
        case 0xC8:{ } break;
        case 0xC9:{ } break;
        case 0xCA:{ } break;
        case 0xCB:{ } break;
        case 0xCC:{ } break;
        case 0xCD:{ } break;
        case 0xCE: { cyclesUsed = this->ins_ADC_A_n(nullptr, this->get_byte_from_pc()); } break;
        case 0xCF:{ } break;

        case 0xD0:{ } break;
        case 0xD1: { cyclesUsed = this->ins_POP_nn(&this->registers.d, &this->registers.e); } break;
        case 0xD2:{ } break;
        case 0xD3: {  cyclesUsed = this->ins_SBC_A_n(nullptr, this->get_byte_from_pc()); } break;
        case 0xD4:{ } break;
        case 0xD5: { cyclesUsed = this->ins_PUSH_nn(this->registers.get_DE()); } break;
        case 0xD6:{ cyclesUsed = this->ins_SUB_n(nullptr, this->get_byte_from_pc());  } break;
        case 0xD7:{ } break;
        case 0xD8:{ } break;
        case 0xD9:{ } break;
        case 0xDA:{ } break;
        case 0xDB:{ } break;
        case 0xDC:{ } break;
        case 0xDD:{ } break;
        case 0xDE:{ } break;
        case 0xDF:{ } break;

        case 0xE0: { cyclesUsed = this->ins_LD_r1_r2(nullptr, 0xFF00 + this->get_byte_from_pc(), nullptr, this->registers.a); } break;
        case 0xE1:{  cyclesUsed = this->ins_POP_nn(&this->registers.h, &this->registers.l); } break;
        case 0xE2: { cyclesUsed = this->ins_LD_r1_r2(nullptr, 0xFF00 + this->registers.c, &this->registers.a); } break;
        case 0xE3:{ } break;
        case 0xE4:{ } break;
        case 0xE5: { cyclesUsed = this->ins_PUSH_nn(this->registers.get_HL()); } break;
        case 0xE6: { cyclesUsed = this->ins_AND_n(nullptr, this->get_byte_from_pc()); } break;
        case 0xE7:{ } break;
        case 0xE8:{ } break;
        case 0xE9:{ } break;
        case 0xEA: { cyclesUsed = this->ins_LD_nn_r1(this->get_word_from_pc_lsbf(), &this->registers.a); } break;
        case 0xEB:{ } break;
        case 0xEC:{ } break;
        case 0xED:{ } break;
        case 0xEE:{ } break;
        case 0xEF:{ } break;
      
        case 0xF0: { cyclesUsed = this->ins_LD_r1_nn(&this->registers.a, 0xFF00 + this->get_byte_from_pc(), 12); } break;
        case 0xF1: { cyclesUsed = this->ins_POP_nn(&this->registers.a, &this->registers.f); } break;
        case 0xF2: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, 0xFF00+this->registers.c); } break;
        case 0xF3:{ } break;
        case 0xF4:{ } break;
        case 0xF5: { cyclesUsed = this->ins_PUSH_nn(this->registers.get_AF()); } break;
        case 0xF6:{ cyclesUsed = this->ins_OR_n(nullptr, this->get_byte_from_pc()); } break;
        case 0xF7:{ } break;
        case 0xF8: { cyclesUsed = this->ins_LDHL_SP_n(&this->registers.h, &this->registers.l, this->registers.sp, this->get_byte_from_pc()); } break;
        case 0xF9: { cyclesUsed = this->ins_LD_nn_nn(&this->registers.sp, this->registers.get_HL()); } break;
        case 0xFA: { cyclesUsed = this->ins_LD_r1_nn(&this->registers.a, this->get_word_from_pc_lsbf(), 16); } break;
        case 0xFB:{ } break;
        case 0xFC:{ } break;
        case 0xFD:{ } break;
        case 0xFE:{ } break;
        case 0xFF:{ } break;

    }

    return cyclesUsed;
}


