#include "cpu.hpp"
#include "bus.hpp"

#include <iostream>

void CPU::DEBUG_printCurrentState()
{

    printf("op:0x%.2X | ", this->bus->get_memory(this->registers.pc));
    printf("%s:0x%.2X%.2X  ","AF", this->registers.a,this->registers.f);
    printf("%s:0x%.2X%.2X  ","BC", this->registers.b,this->registers.c);
    printf("%s:0x%.2X%.2X  ","DE", this->registers.d,this->registers.e);
    printf("%s:0x%.2X%.2X  ","HL", this->registers.h,this->registers.l);
    printf("%s:0x%.4X  ","SP", this->registers.sp);
    printf("%s:0x%.4X  ","pc", this->registers.pc);
    printf("%s:%i  ","z", this->registers.get_flag(z));
    printf("%s:%i  ","n", this->registers.get_flag(n));
    printf("%s:%i  ","h", this->registers.get_flag(h));
    printf("%s:%i  ","c", this->registers.get_flag(c));

    printf("\n");
}

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

void CPU::init2()
{
    this->registers.pc = 0x100;
    this->registers.a = 0x11;
    this->registers.b = 0x00;
    this->registers.c = 0x00;
    this->registers.d = 0xff;
    this->registers.e = 0x56;
    this->registers.f = 0x80;
    this->registers.h = 0x00;
    this->registers.l = 0x0d;
    this->registers.sp = 0xFFFE;
}



CPU::CPU()
{
    this->init();
#ifdef DEBUG
    this->init2();
#endif

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

void CPU::interrupt_DI_EI_handler()
{
    if (this->DI_triggered)
    {
        this->bus->set_memory(0xffff, 0);
        this->DI_triggered = false;
        return;
    }
    if (this->EI_triggered)
    {
        this->bus->set_memory(0xffff, 1);
        this->EI_triggered = false;
        return;
    }


}

void CPU::checkHalfCarry(int a, int b)
{
    int sum = 0;

    //16 bit half carry check
    if (a > 0xff || b > 0xff)
    {  
        //make sum of lower bytes
         sum = (a & 0xFF) + (b & 0xFF);

         if ((sum & 0x100) == 0x100)
         {
             this->registers.set_flag(h, 1);
             return;
         }
         this->registers.set_flag(h, 0);
         return;
    }
  
    //add only the lower nibbles of the sum to each other
     sum = (a & 0xf) + (b & 0xf);

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

Byte CPU::get_nibble(const Byte input, const bool getHi)
{
    Byte result = 0;
    (getHi) ? result = (input & 0xF0) >> 4 : result = input & 0x0F;
    return result;
}

void CPU::set_nibble(Byte* registerOne, const Byte value, const bool setHi)
 {
            (setHi) ? *registerOne = ((*registerOne & 0x0F) | (value << 4)) : *registerOne = ((*registerOne & 0xF0) | value);
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
        (*registerOne == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
        this->registers.set_flag(n, 0);

        return 4;
    }
    // else if immediate value passed

    //check carry and half carry flags, I realise this is out of order but it should work the same.
    this->checkHalfCarry(this->bus->get_memory(address), 1);
    
    // perform addition
    this->bus->set_memory(address, (this->bus->get_memory(address) + 1));
    //evaluate z flag an clear the n flag
    (*registerOne == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
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
        (*registerOne == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
        this->registers.set_flag(n, 1);

        return 4;
    }
    // else if immediate value passed

    //check carry and half carry flags, I realise this is out of order but it should work the same.
    this->checkHalfBorrow(this->bus->get_memory(address), 1);

    // perform addition
    this->bus->set_memory(address, (this->bus->get_memory(address) - 1 ));
    //evaluate z flag an clear the n flag
    (*registerOne == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
    this->registers.set_flag(n, 1);

    return 12;
}

int CPU::ins_ADD_HL_n(const Word value)
{
    this->checkCarry(this->registers.get_HL(), value);
    this->checkHalfCarry(this->registers.get_HL(), value);

    this->registers.set_HL(value);
    
    this->registers.set_flag(n, 0);
    return 8;
}

int CPU::ins_ADD_SP_n(const Byte_s value)
{
    //if negative
    if (value < 0)
    {
        this->checkBorrow(this->registers.sp, value);
        this->checkHalfBorrow(this->registers.sp, value);
        this->registers.sp += value;
        this->registers.set_flag(z, 0);
        this->registers.set_flag(n, 0);
        return 16;
    }
    this->checkCarry(this->registers.sp, value);
    this->checkHalfCarry(this->registers.sp, value);
    this->registers.sp += value;
    this->registers.set_flag(z, 0);
    this->registers.set_flag(n, 0);
    return 16;

}



int CPU::ins_INC_nn(Byte* registerOne, Byte* registerTwo, Word* stackPointer)
{
    if (stackPointer)
    {
        this->registers.sp++;
        return 8;
    }
    this->registers.set_word(registerOne, registerTwo, (this->registers.get_word(registerOne, registerTwo)+1));
    return 8;
}

int CPU::ins_DEC_nn(Byte* registerOne, Byte* registerTwo, Word* stackPointer)
{
    if (stackPointer)
    {
        this->registers.sp--;
        return 8;
    }
    this->registers.set_word(registerOne, registerTwo, (this->registers.get_word(registerOne, registerTwo)-1));
    return 8;
}

int CPU::ins_SWAP_nn(Byte* registerOne, Word address)
{
    if (registerOne)
    {
        *registerOne = (this->get_nibble(*registerOne, false) << 4) + this->get_nibble(*registerOne, true);
        return 8;
    }
    this->bus->set_memory(address, (this->get_nibble(this->bus->get_memory(address), false) << 4) + this->get_nibble(this->bus->get_memory(address), true));
    return 16;
}

int CPU::ins_DAA()
{
    // this is a weird one, take whatever is in register a and re shuffle it to a packed binary coded decimal
    // since we only have 1 byte of space to work with then we can only display upto 99 using BCD
    // which would require to set the overflow flag.

    // to actually convert binary to binary coded decimal, its fairly simple
    // an order of magnitude is represented by a nibble, if a nibble in binary is larger than 9, then
    // we just add 0x06 to register A and it will be converted
    // the same is true for the 10s nibble, if it is over 9 then we add 0x60 to A and set carry.
    // however we need to check if the value in register A actually means a negative which we check N flag and then subract 6 instead of adding it
    
    // temp store of offset we will use
    Byte nibbleOfset = 0x0;
    
    // temp store of value of the carry bit, set to reset
    bool carry = false;

    // check if lower nibble is bigger than 9
    if ((this->registers.a & 0x0F) > 9)
    {
        nibbleOfset += 6;
    }

    // check if higher nibble is bigger than 90, set carry to true
    if ((this->registers.a & 0xF0) > 90)
    {
        nibbleOfset += 60;
        carry = true;
    }

    // check state of the n flag
    if (this->registers.get_flag(n))
        nibbleOfset *= -1;

    // perfrom bcd conversion
    this->registers.a += nibbleOfset;

    // set carry flag and half to 0
    this->registers.set_flag(c,carry);
    this->registers.set_flag(h, 0);

    // check z flag status
    if (this->registers.a == 0)
        this->registers.set_flag(z, 1);


    return 4;
}

int CPU::ins_CPL()
{
    this->registers.a = ~this->registers.a;

    this->registers.set_flag(n, true);
    this->registers.set_flag(h, true);
    return 4;
}

int CPU::ins_CCF()
{
    this->registers.set_flag(n,0);
    this->registers.set_flag(h,0);

    (this->registers.get_flag(c))? this->registers.set_flag(c,0) : this->registers.set_flag(c, 1);
    return 4;
}

int CPU::ins_SCF()
{
    this->registers.set_flag(n,0);
    this->registers.set_flag(h,0);
    this->registers.set_flag(c,1);
    return 4;
}

int CPU::ins_RCLA()
{
    //set c flag to whatever is the value of the leftest bit from register a
    this->registers.set_flag(c, (this->registers.a & 0x80 >> 7));
    this->registers.set_flag(n, 0);
    this->registers.set_flag(h, 0);
    this->registers.set_flag(z, 0);

    // move everything to the left by one, toggle bit 0 with bit 7 shifted right 7 places
    this->registers.a = (this->registers.a << 1) | (this->registers.a >> (7));


    return 4;
}

int CPU::ins_RLA()
{
    // swap leftest most bit with the carry flag, then rotate to the left
    
    Byte flagCarry = this->registers.get_flag(c);
    bool registerCarry = (this->registers.a & 0x80 >> 7);

    this->registers.a = (this->registers.a << 1) | (flagCarry);

    this->registers.set_flag(c, registerCarry);
    this->registers.set_flag(n, 0);
    this->registers.set_flag(h, 0);
    this->registers.set_flag(z, 0);
    
    return 4;
}

int CPU::ins_RRCA()
{
    this->registers.set_flag(c, this->registers.a & 0x1);

    this->registers.a = (this->registers.a >> 1) | (this->registers.a << (7));

    this->registers.set_flag(n, 0);
    this->registers.set_flag(h, 0);
    this->registers.set_flag(z, 0);

    return 4;
}

int CPU::ins_RRA()
{
    Byte flagCarry = this->registers.get_flag(c);
    bool registerCarry = (this->registers.a & 0x1);

    this->registers.a = (this->registers.a >> 1) | (flagCarry << (7));

    this->registers.set_flag(c, registerCarry);
    this->registers.set_flag(n, 0);
    this->registers.set_flag(h, 0);
    this->registers.set_flag(z, 0);

    return 4;
}

int CPU::ins_RLC(Byte* registerOne, Word address)
{
    if (registerOne)
    {
        this->registers.set_flag(c, (*registerOne & 0x80 >> 7));
        *registerOne = ((*registerOne << 1) | (*registerOne >> 7));

        this->registers.set_flag(n,0);
        this->registers.set_flag(h,0);

        if (*registerOne == 0x0)
            this->registers.set_flag(z, 1);

        return 8;
    }

    Byte temp = this->bus->get_memory(address);

    this->registers.set_flag(c, (temp & 0x80 >> 7));
    this->bus->set_memory(address,((temp << 1) | (temp >> 7)));

    this->registers.set_flag(n, 0);
    this->registers.set_flag(h, 0);

    if (temp == 0x0)
        this->registers.set_flag(z, 1);

    return 16;
}

int CPU::ins_RL(Byte* registerOne, Word address)
{
    Byte flagCarry = this->registers.get_flag(c);
    bool newCarry = 0;

    if (registerOne)
    {
        newCarry = (*registerOne & 0x80 >> 7);
        *registerOne = ((*registerOne << 1) | (flagCarry));

        this->registers.set_flag(c, newCarry);
        this->registers.set_flag(n, 0);
        this->registers.set_flag(h, 0);

        if (*registerOne == 0x0)
            this->registers.set_flag(z, 1);

        return 8;
    }

    Byte temp = this->bus->get_memory(address);

    newCarry = (temp & 0x80 >> 7);
    this->bus->set_memory(address, ((temp << 1) | (flagCarry)));

    this->registers.set_flag(c, newCarry);
    this->registers.set_flag(n, 0);
    this->registers.set_flag(h, 0);

    if (temp == 0x0)
        this->registers.set_flag(z, 1);

    return 16;
}

int CPU::ins_RRC(Byte* registerOne, Word address)
{
    if (registerOne)
    {
        this->registers.set_flag(c, (*registerOne & 0x1));
        *registerOne = ((*registerOne >> 1) | (*registerOne << 7));

        this->registers.set_flag(n, 0);
        this->registers.set_flag(h, 0);

        if (*registerOne == 0x0)
            this->registers.set_flag(z, 1);

        return 8;
    }

    Byte temp = this->bus->get_memory(address);

    this->registers.set_flag(c, (temp & 0x1));
    this->bus->set_memory(address, ((temp >> 1) | (temp << 7)));

    this->registers.set_flag(n, 0);
    this->registers.set_flag(h, 0);

    if (temp == 0x0)
        this->registers.set_flag(z, 1);

    return 16;
}

int CPU::ins_RR(Byte* registerOne, Word address)
{
    Byte flagCarry = this->registers.get_flag(c);
    bool newCarry = 0;

    if (registerOne)
    {
        newCarry = (*registerOne & 0x1);
        *registerOne = ((*registerOne >> 1) | (flagCarry << 7 ));

        this->registers.set_flag(c, newCarry);
        this->registers.set_flag(n, 0);
        this->registers.set_flag(h, 0);

        if (*registerOne == 0x0)
            this->registers.set_flag(z, 1);

        return 8;
    }

    Byte temp = this->bus->get_memory(address);

    newCarry = (temp & 0x01);
    this->bus->set_memory(address, ((temp >> 1) | (flagCarry << 7)));

    this->registers.set_flag(c, newCarry);
    this->registers.set_flag(n, 0);
    this->registers.set_flag(h, 0);

    if (temp == 0x0)
        this->registers.set_flag(z, 1);

    return 16;
}

int CPU::ins_SLA(Byte* registerOne, Word address)
{
    if (registerOne)
    {
        this->registers.set_flag(c,*registerOne & 0x80 >> 7);
        *registerOne = *registerOne << 1;

        this->registers.set_flag(n, 0);
        this->registers.set_flag(h, 0);

        if (*registerOne == 0x0)
            this->registers.set_flag(z, 1);

        return 8;
    }
    Byte temp = this->bus->get_memory(address);

    this->registers.set_flag(c, temp & 0x80 >> 7);
    this->bus->set_memory(address, temp << 1);

    this->registers.set_flag(n, 0);
    this->registers.set_flag(h, 0);

    if (temp == 0x0)
        this->registers.set_flag(z, 1);

    return 16;
}

int CPU::ins_SRA_n(Byte* registerOne, Word address)
{
    Byte bit7 = 0;
    // shift right into carry, msb does not change
    if (registerOne)
    {
        this->registers.set_flag(c, *registerOne & 0x1);
        bit7 = *registerOne >> 7;

        *registerOne = *registerOne >> 1 | (bit7 << 7);

        this->registers.set_flag(n, 0);
        this->registers.set_flag(h, 0);

        if (*registerOne == 0x0)
            this->registers.set_flag(z, 1);

        return 8;
    }
    Byte temp = this->bus->get_memory(address);

    this->registers.set_flag(c, temp & 0x1);
    bit7 = temp >> 7;
    this->bus->set_memory(address, temp >> 1 | (bit7 << 7));

    this->registers.set_flag(n, 0);
    this->registers.set_flag(h, 0);

    if (temp == 0x0)
        this->registers.set_flag(z, 1);

    return 16;
}

int CPU::ins_SRL_n(Byte* registerOne, Word address)
{
    // shift right into carry, msb does not change
    if (registerOne)
    {
        this->registers.set_flag(c, *registerOne & 0x1);

        *registerOne = *registerOne >> 1;

        this->registers.set_flag(n, 0);
        this->registers.set_flag(h, 0);

        if (*registerOne == 0x0)
            this->registers.set_flag(z, 1);

        return 8;
    }
    Byte temp = this->bus->get_memory(address);

    this->registers.set_flag(c, temp & 0x1);

    this->bus->set_memory(address, temp >> 1);

    this->registers.set_flag(n, 0);
    this->registers.set_flag(h, 0);

    if (temp == 0x0)
        this->registers.set_flag(z, 1);

    return 16;
}

int CPU::ins_BIT_b_r(Byte bit, Byte* registerOne, Word address)
{
    if (registerOne)
    {
        if ((*registerOne & (1 << bit)) == 0)
            this->registers.set_flag(n, 1);

        this->registers.set_flag(n,0);
        this->registers.set_flag(h,1);

        return 8;
    }

    if ((this->bus->get_memory(address) & (1 << bit)) == 0)
        this->registers.set_flag(n, 1);

    this->registers.set_flag(n, 0);
    this->registers.set_flag(h, 1);

    return 16;
}

int CPU::ins_SET_b_r(Byte bit, Byte* registerOne, Word address)
{
    if (registerOne)
    {
        *registerOne |=  (1 << bit);
        return 8;
    }

    this->bus->set_memory(address, this->bus->get_memory(address) | (1 << bit));
    return 16;
}

int CPU::ins_RES_b_r(Byte bit, Byte* registerOne, Word address)
{
    if (registerOne)
    {
        *registerOne &= ~(1 << bit);
        return 8;
    }

    this->bus->set_memory(address, this->bus->get_memory(address) & ~(1 << bit));
    return 16;
}

int CPU::ins_JP_nn(Word address)
{
    this->registers.pc = address;
    return 12;
}

int CPU::ins_JP_cc_nn(enum JumpCondition condition, Word address)
{
    switch (condition)
    {
        case NZ:
        {
            if (this->registers.get_flag(z) == 0)
                this->ins_JP_nn(address);
        } break;
    
        case Z:
        {
            if (this->registers.get_flag(z) == 1)
                this->ins_JP_nn(address);
        } break;
    
        case NC:
        {
            if (this->registers.get_flag(c) == 0)
                this->ins_JP_nn(address);
        } break;
    
        case C:
        {
            if (this->registers.get_flag(c) == 1)
                this->ins_JP_nn(address);
        } break;
    }
    return 12;
}

int CPU::ins_JP_HL()
{
    this->registers.pc = this->registers.get_HL();
    return 4;
}

int CPU::ins_JR_n(Byte_s jumpOffset)
{
    this->registers.pc += jumpOffset;

    return 8;
}

int CPU::ins_JR_cc_n(JumpCondition condition, Byte_s jumpOffset)
{
    switch (condition)
    {
    case NZ:
    {
        if (this->registers.get_flag(z) == 0)
            this->ins_JR_n(jumpOffset);
    } break;

    case Z:
    {
        if (this->registers.get_flag(z) == 1)
            this->ins_JR_n(jumpOffset);
    } break;

    case NC:
    {
        if (this->registers.get_flag(c) == 0)
            this->ins_JR_n(jumpOffset);
    } break;

    case C:
    {
        if (this->registers.get_flag(c) == 1)
            this->ins_JR_n(jumpOffset);
    } break;
    }
    return 8;
}


int CPU::ins_CALL_nn(Word address)
{
    this->ins_PUSH_nn(this->registers.pc+1);
    this->ins_JP_nn(address);
    return 12;
}

int CPU::ins_CALL_cc_nn(enum JumpCondition condition, Word address)
{
    switch (condition)
    {
    case NZ:
    {
        if (this->registers.get_flag(z) == 0)
            this->ins_CALL_nn(address);
    } break;

    case Z:
    {
        if (this->registers.get_flag(z) == 1)
            this->ins_CALL_nn(address);
    } break;

    case NC:
    {
        if (this->registers.get_flag(c) == 0)
            this->ins_CALL_nn(address);
    } break;

    case C:
    {
        if (this->registers.get_flag(c) == 1)
            this->ins_CALL_nn(address);
    } break;
    }
    return 12;
}

int CPU::ins_RST_n(const Byte addrOffset)
{
    this->ins_PUSH_nn(this->registers.pc);
    this->ins_JP_nn(0x0000 + addrOffset);
    return 32;
}

int CPU::ins_RET()
{
    this->registers.sp -= 2;
    this->registers.pc = this->registers.sp;
    return 8;
}

int CPU::ins_RETI()
{
    this->ins_RET();
    this->bus->set_memory(0xffff, 1);
    return 12;
}

int CPU::ins_RET_cc(const JumpCondition condition)
{
    switch (condition)
    {
    case NZ:
    {
        if (this->registers.get_flag(z) == 0)
            this->ins_RET();
    } break;

    case Z:
    {
        if (this->registers.get_flag(z) == 1)
            this->ins_RET();
    } break;

    case NC:
    {
        if (this->registers.get_flag(c) == 0)
            this->ins_RET();
    } break;

    case C:
    {
        if (this->registers.get_flag(c) == 1)
            this->ins_RET();
    } break;
    }
    return 12;
}









