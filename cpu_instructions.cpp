#include "cpu.hpp"
#include "bus.hpp"



// INSTRUCTION HANDLING


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
        *registerOne = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);
        return 8;
    }
    if (address && registerTwo)
    {
        this->bus->set_memory(address, *registerTwo, MEMORY_ACCESS_TYPE::cpu);
        return 8;
    }
    // if (address && value)

    this->bus->set_memory(address, value, MEMORY_ACCESS_TYPE::cpu);
    return 12;

}

int CPU::ins_LD_r1_nn(Byte* registerOne, const Word address, const int cyclesUsed)
{
    // Cycles used is a param since it can change
    //take value pointed to by nn and put into register one,
    // LD a,(nn), mem byte eg: FA 34 12, ld a,(1234)
    // check memory at address 0x1234, put that data into register one

    *registerOne = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);
    return cyclesUsed;
};
// Take value from registerOne, place at memory location 
int CPU::ins_LD_nn_r1(Word address, Byte* registerOne)
{
    this->bus->set_memory(address, *registerOne, MEMORY_ACCESS_TYPE::cpu);
    return 16;
};

int CPU::ins_LDDI_nn_r1(Word address, const Byte* registerOne, Byte* registerTwo, Byte* registerThree, const int addSubValue)
{
    this->bus->set_memory(address, *registerOne, MEMORY_ACCESS_TYPE::cpu);
    this->registers.set_word(registerTwo, registerThree, (this->registers.get_word(registerTwo, registerThree) + addSubValue));
    return 8;
};

int CPU::ins_LDDI_r1_nn(Byte* registerOne, const Word address, Byte* registerTwo, Byte* registerThree, const int addSubValue)
{
    *registerOne = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);
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

int CPU::ins_LDHL_SP_n(Byte* wordRegisterNibbleHi, Byte* wordRegisterNibbleLo, const Word stackPointerValue, const Byte_s value)
{
    Word sum = stackPointerValue + value;
    this->registers.set_flag(z, 0);
    this->registers.set_flag(n, 0);


    if (value < 0)
    {
        this->registers.set_flag(c, (sum & 0xFF) <= (stackPointerValue & 0xFF));
        this->registers.set_flag(h, (sum & 0xF) <= (stackPointerValue & 0xF));
    }
    else
    {
        this->registers.set_flag(c, ((stackPointerValue & 0xFF) + value) > 0xFF);
        this->registers.set_flag(h, ((stackPointerValue & 0xF) + value) > 0xF);
    }

    this->registers.set_word(wordRegisterNibbleHi, wordRegisterNibbleLo, sum);
    return 12;
}

int CPU::ins_LD_nn_SP(const Word address, const Word stackPointerValue)
{
    //place value of SP into memory at address and address + 1
    this->bus->set_memory_word(address, stackPointerValue, MEMORY_ACCESS_TYPE::cpu);
    return 20;
}

//new push, pushes addr -1 then addr -2
int CPU::ins_PUSH_nn(const Word wordRegisterValue)
{
    // move low byte to higher (sp)
    this->registers.sp--;
    this->bus->set_memory(this->registers.sp, ((wordRegisterValue & 0xff00) >> 8), MEMORY_ACCESS_TYPE::cpu);

    this->registers.sp--;
    // move high byte to lower (sp)
    this->bus->set_memory(this->registers.sp, (wordRegisterValue & 0x00ff), MEMORY_ACCESS_TYPE::cpu);

    return 16;
}

int CPU::ins_POP_nn(Byte* registerOne, Byte* registerTwo)
// a f
{
    *registerTwo = this->bus->get_memory(this->registers.sp, MEMORY_ACCESS_TYPE::cpu);
    if (registerTwo == &this->registers.f)
        *registerTwo &= 0xF0;

    this->registers.sp++;
    *registerOne = this->bus->get_memory(this->registers.sp, MEMORY_ACCESS_TYPE::cpu);
    this->registers.sp++;

    return 12;
}

int CPU::ins_ADD_A_n(const Byte* registerOne, const Byte immediateValue)
{
    if (registerOne)
    {
        this->registers.set_flag(c, this->checkCarry(this->registers.a, *registerOne, 8));
        this->registers.set_flag(h, this->checkCarry(this->registers.a, *registerOne, 4));

        // perform addition
        this->registers.a += *registerOne;

        //evaluate z flag an clear the n flag
        this->registers.set_flag(z, (this->registers.a == 0x0));
        this->registers.set_flag(n, 0);

        return 4;
    }
    // else if immediate value passed
    this->registers.set_flag(c, this->checkCarry(this->registers.a, immediateValue, 8));
    this->registers.set_flag(h, this->checkCarry(this->registers.a, immediateValue, 4));

    // perform addition
    this->registers.a += immediateValue;

    //evaluate z flag an clear the n flag
    this->registers.set_flag(z, (this->registers.a == 0x0));
    this->registers.set_flag(n, 0);


    return 8;
}

int CPU::ins_ADC_A_n(const Byte* registerOne, const Byte immediateValue)
{
    Byte a = this->registers.a;
    Byte b = (registerOne) ? *registerOne : immediateValue;
    Byte C = (int)this->registers.get_flag(c);
    Byte cyclesUsed = (registerOne) ? 4 : 8;

    this->registers.a = a + b + C;

    this->registers.set_flag(z, (this->registers.a == 0x0));
    this->registers.set_flag(n, 0);
    this->registers.set_flag(c, this->checkCarry(a, b, 8, C));
    this->registers.set_flag(h, this->checkCarry(a, b, 4, C));

    return cyclesUsed;
}


int CPU::ins_SUB_n(const Byte* registerOne, const Byte immediateValue)
{
    if (registerOne)
    {
        //check carry and half carry flags, I realise this is out of order but it should work the same.
        this->registers.set_flag(c, this->checkBorrow(this->registers.a, *registerOne, 8));
        this->registers.set_flag(h, this->checkBorrow(this->registers.a, *registerOne, 4));

        // perform addition
        this->registers.a -= *registerOne;

        //evaluate z flag an clear the n flag
        this->registers.set_flag(z, (this->registers.a == 0x0));
        this->registers.set_flag(n, 1);

        return 4;
    }
    // else if immediate value passed

    //check carry and half carry flags, I realise this is out of order but it should work the same.
    this->registers.set_flag(c, this->checkBorrow(this->registers.a, immediateValue, 8));
    this->registers.set_flag(h, this->checkBorrow(this->registers.a, immediateValue, 4));

    // perform addition
    this->registers.a -= immediateValue;

    //evaluate z flag an clear the n flag
    this->registers.set_flag(z, (this->registers.a == 0x0));
    this->registers.set_flag(n, 1);

    return 8;
}

int CPU::ins_SBC_A_n(const Byte* registerOne, const Byte immediateValue)
{

    Byte a = this->registers.a;
    Byte b = (registerOne) ? *registerOne : immediateValue;
    Byte C = (int)this->registers.get_flag(c);
    Byte cyclesUsed = (registerOne) ? 4 : 8;

    this->registers.a = a - b - C;

    this->registers.set_flag(z, (this->registers.a == 0x0));
    this->registers.set_flag(n, 1);
    this->registers.set_flag(c, this->checkBorrow(a, b, 8, C));
    this->registers.set_flag(h, this->checkBorrow(a, b, 4, C));

    return cyclesUsed;

}

int CPU::ins_AND_n(const Byte* registerOne, const Byte immediateValue)
{
    if (registerOne)
    {
        this->registers.a &= *registerOne;

        //evaluate z flag an clear the n flag
        this->registers.set_flag(z, (this->registers.a == 0x0));
        this->registers.set_flag(h, 1);
        this->registers.set_flag(n, 0);
        this->registers.set_flag(c, 0);

        return 4;
    }
    // else if immediate value passed

    this->registers.a &= immediateValue;

    //evaluate z flag an clear the n flag
    this->registers.set_flag(z, (this->registers.a == 0x0));
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
        this->registers.set_flag(z, (this->registers.a == 0x0));
        this->registers.set_flag(h, 0);
        this->registers.set_flag(n, 0);
        this->registers.set_flag(c, 0);

        return 4;
    }
    // else if immediate value passed

    this->registers.a |= immediateValue;

    //evaluate z flag an clear the n flag
    this->registers.set_flag(z, (this->registers.a == 0x0));
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
        this->registers.set_flag(z, (this->registers.a == 0x0));
        this->registers.set_flag(h, 0);
        this->registers.set_flag(n, 0);
        this->registers.set_flag(c, 0);

        return 4;
    }
    // else if immediate value passed

    this->registers.a ^= immediateValue;

    //evaluate z flag an clear the n flag
    this->registers.set_flag(z, (this->registers.a == 0x0));
    this->registers.set_flag(h, 0);
    this->registers.set_flag(n, 0);
    this->registers.set_flag(c, 0);


    return 8;
}

int CPU::ins_CP_n(const Byte* registerOne, const Byte immediateValue)
{
    if (registerOne)
    {

        this->registers.set_flag(c, this->checkBorrow(this->registers.a, *registerOne, 8));
        this->registers.set_flag(h, this->checkBorrow(this->registers.a, *registerOne, 4));

        //evaluate z flag an clear the n flag
        (this->registers.a == *registerOne) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
        this->registers.set_flag(n, 1);

        return 4;
    }
    // else if immediate value passed
    this->registers.set_flag(c, this->checkBorrow(this->registers.a, immediateValue, 8));
    this->registers.set_flag(h, this->checkBorrow(this->registers.a, immediateValue, 4));

    //evaluate z flag an clear the n flag
    (this->registers.a == immediateValue) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
    this->registers.set_flag(n, 1);

    return 8;
}

int CPU::ins_INC_n(Byte* registerOne, Word address)
{
    if (registerOne)
    {
        this->registers.set_flag(h, this->checkCarry(*registerOne, 1, 4));

        // perform addition
        (*registerOne)++;

        //evaluate z flag an clear the n flag
        (*registerOne == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
        this->registers.set_flag(n, 0);

        return 4;
    }
    // else if immediate value passed

    //check carry and half carry flags, I realise this is out of order but it should work the same.

    auto temp = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);
    this->registers.set_flag(h, this->checkCarry(temp, 1, 4));

    // perform addition
    this->bus->set_memory(address, (temp + 1), MEMORY_ACCESS_TYPE::cpu);
    //evaluate z flag an clear the n flag
    (this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu) == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
    this->registers.set_flag(n, 0);

    return 12;
}

int CPU::ins_DEC_n(Byte* registerOne, Word address)
{
    if (registerOne)
    {
        this->registers.set_flag(h, this->checkBorrow(*registerOne, 1, 4));
        // perform addition
        (*registerOne)--;

        //evaluate z flag an clear the n flag
        (*registerOne == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
        this->registers.set_flag(n, 1);

        return 4;
    }
    // else if immediate value passed
    auto temp = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);
    this->registers.set_flag(h, this->checkBorrow(temp, 1, 4));

    // perform addition
    this->bus->set_memory(address, (temp - 1), MEMORY_ACCESS_TYPE::cpu);
    //evaluate z flag an clear the n flag
    (this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu) == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
    this->registers.set_flag(n, 1);

    return 12;
}

int CPU::ins_ADD_HL_n(const Word value)
{
    Word HLvalue = this->registers.get_HL();


    this->registers.set_flag(c, this->checkCarry(HLvalue, value, 16));
    this->registers.set_flag(h, this->checkCarry(HLvalue, value, 12));

    this->registers.set_HL(HLvalue + value);

    this->registers.set_flag(n, 0);
    return 8;
}

int CPU::ins_ADD_SP_n(const Byte_s value)
{
    //if negative

    if (value < 0)
    {
        this->registers.set_flag(c, ((this->registers.sp + value) & 0xFF) <= (this->registers.sp & 0xFF));
        this->registers.set_flag(h, ((this->registers.sp + value) & 0xF) <= (this->registers.sp & 0xF));
    }
    else
    {
        this->registers.set_flag(c, ((this->registers.sp & 0xFF) + value) > 0xFF);
        this->registers.set_flag(h, ((this->registers.sp & 0xF) + value) > 0xF);
    }

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
    this->registers.set_word(registerOne, registerTwo, (this->registers.get_word(registerOne, registerTwo) + 1));
    return 8;
}

int CPU::ins_DEC_nn(Byte* registerOne, Byte* registerTwo, Word* stackPointer)
{
    if (stackPointer)
    {
        this->registers.sp--;
        return 8;
    }
    this->registers.set_word(registerOne, registerTwo, (this->registers.get_word(registerOne, registerTwo) - 1));
    return 8;
}

int CPU::ins_SWAP_nn(Byte* registerOne, Word address)
{
    int cyclesUsed = 0;
    if (registerOne)
    {
        *registerOne = (this->get_nibble(*registerOne, false) << 4) + this->get_nibble(*registerOne, true);
        (*registerOne == 0x0) ? this->registers.set_flag(z, true) : this->registers.set_flag(z, false);
        cyclesUsed = 8;
    }
    else
    {
        this->bus->set_memory(address, (this->get_nibble(this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu), false) << 4) + this->get_nibble(this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu), true), MEMORY_ACCESS_TYPE::cpu);
        (this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu) == 0x0) ? this->registers.set_flag(z, true) : this->registers.set_flag(z, false);
        cyclesUsed = 16;
    }
    this->registers.set_flag(n, false);
    this->registers.set_flag(h, false);
    this->registers.set_flag(c, false);

    return cyclesUsed;
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


    //DAA is used after addition or subtraction to fix the BCD value. So if we take the decimal numbers 42 and add 9, we expect to get 51. But if we do this with BCD values, we'll get $4B instead. Executing DAA after this addition will adjust $4B to $51 as expected. The first if determines whether we need to ...
    Byte adjust = 0x0;

    bool flagZ = this->registers.get_flag(z);
    bool flagN = this->registers.get_flag(n);
    bool flagH = this->registers.get_flag(h);
    bool flagC = this->registers.get_flag(c);

    // temp store of value of the carry bit, set to reset
    bool need_carry = false;

    if (!flagN)
    {
        if (flagH || ((this->registers.a & 0xF) > 0x9))
            adjust += 0x6;

        if (flagC || ((this->registers.a) > 0x9F))
        {
            adjust += 0x60;
            need_carry = true;
        }
    }
    else
    {
        if (flagH)
            adjust -= 0x6;
        if (flagC)
            adjust -= 0x60;
    }

    (need_carry) ? this->registers.set_flag(c, 1) : this->registers.set_flag(c, 0);

    this->registers.set_flag(h, 0);

    this->registers.a += adjust;

    (this->registers.a == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);


    return 4;

    // check if lower nibble is bigger than 9 or flag H is set
    if (((this->registers.a) > 9) || flagH)
    {
        adjust += 0x6;
    }

    // check if higher nibble is bigger than 90, set carry to true
    if (((this->registers.a & 0xF0) > 90) || flagC)
    {
        adjust += 0x60;
        need_carry = true;
    }

    if (flagN && flagC)
    {

        need_carry = true;
        adjust = 0xA0;
    }
    if (flagH && flagC)
    {

        need_carry = true;
        adjust = 0x66;
    }
    if (flagH && flagN)
        adjust = 0xFA;

    if (flagH && flagN && flagC)
    {
        need_carry = true;
        adjust = 0x9A;
    }


    //this->checkCarry(this->registers.a, adjust);
    (need_carry) ? this->registers.set_flag(c, 1) : this->registers.set_flag(c, 0);

    this->registers.set_flag(h, 0);

    this->registers.a += adjust;

    (this->registers.a == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);



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
    this->registers.set_flag(n, 0);
    this->registers.set_flag(h, 0);

    (this->registers.get_flag(c) == true) ? this->registers.set_flag(c, 0) : this->registers.set_flag(c, 1);
    return 4;
}

int CPU::ins_SCF()
{
    this->registers.set_flag(n, 0);
    this->registers.set_flag(h, 0);
    this->registers.set_flag(c, 1);
    return 4;
}

int CPU::ins_RLCA()
{
    //set c flag to whatever is the value of the leftest bit from register a
    this->registers.set_flag(c, (this->registers.a & 0x80));
    this->registers.set_flag(n, 0);
    this->registers.set_flag(h, 0);

    // move everything to the left by one, toggle bit 0 with bit 7 shifted right 7 places
    this->registers.a = (this->registers.a << 1) | (this->registers.a >> (7));

    // z is reset
    this->registers.set_flag(z, 0);

    return 4;
}

int CPU::ins_RLA()
{
    // swap leftest most bit with the carry flag, then rotate to the left

    Byte flagCarry = this->registers.get_flag(c);
    bool registerCarry = ((this->registers.a & 0x80) >> 7);

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
    this->registers.set_flag(n, 0);
    this->registers.set_flag(h, 0);

    if (registerOne)
    {
        this->registers.set_flag(c, ((*registerOne & 0x80) >> 7));
        *registerOne = ((*registerOne << 1) | (*registerOne >> 7));

        (*registerOne == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);

        return 8;
    }

    Byte temp = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);
    Byte result = ((temp << 1) | (temp >> 7));

    this->registers.set_flag(c, ((temp & 0x80) >> 7));

    this->bus->set_memory(address, result, MEMORY_ACCESS_TYPE::cpu);


    (result == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);


    return 16;
}

int CPU::ins_RL(Byte* registerOne, Word address)
{
    Byte flagCarry = this->registers.get_flag(c);
    bool newCarry = 0;

    this->registers.set_flag(n, 0);
    this->registers.set_flag(h, 0);

    if (registerOne)
    {
        newCarry = ((*registerOne & 0x80) >> 7);
        *registerOne = ((*registerOne << 1) | (flagCarry));

        this->registers.set_flag(c, newCarry);

        (*registerOne == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);

        return 8;
    }

    Byte temp = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);
    Byte result = ((temp << 1) | (flagCarry));
    newCarry = ((temp & 0x80) >> 7);
    this->bus->set_memory(address, result, MEMORY_ACCESS_TYPE::cpu);

    this->registers.set_flag(c, newCarry);

    (result == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);

    return 16;
}

int CPU::ins_RRC(Byte* registerOne, Word address)
{

    this->registers.set_flag(n, 0);
    this->registers.set_flag(h, 0);

    if (registerOne)
    {
        this->registers.set_flag(c, (*registerOne & 0x1));
        *registerOne = ((*registerOne >> 1) | (*registerOne << 7));


        (*registerOne == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);

        return 8;
    }

    Byte temp = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);
    Byte result = ((temp >> 1) | (temp << 7));

    this->registers.set_flag(c, (temp & 0x1));
    this->bus->set_memory(address, result, MEMORY_ACCESS_TYPE::cpu);

    (result == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);

    return 16;
}

int CPU::ins_RR(Byte* registerOne, Word address)
{
    Byte flagCarry = this->registers.get_flag(c);
    bool newCarry = 0;

    this->registers.set_flag(n, 0);
    this->registers.set_flag(h, 0);

    if (registerOne)
    {
        newCarry = (*registerOne & 0x1);
        *registerOne = ((*registerOne >> 1) | (flagCarry << 7));

        this->registers.set_flag(c, newCarry);

        (*registerOne == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);

        return 8;
    }

    Byte temp = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);
    Byte result = ((temp >> 1) | (flagCarry << 7));
    newCarry = (temp & 0x01);
    this->bus->set_memory(address, result, MEMORY_ACCESS_TYPE::cpu);

    this->registers.set_flag(c, newCarry);
    (result == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
    return 16;
}

int CPU::ins_SLA(Byte* registerOne, Word address)
{
    this->registers.set_flag(n, 0);
    this->registers.set_flag(h, 0);

    if (registerOne)
    {
        this->registers.set_flag(c, (*registerOne & 0x80) >> 7);
        *registerOne = *registerOne << 1;

        (*registerOne == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);

        return 8;
    }
    Byte temp = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);
    Byte result = temp << 1;

    this->registers.set_flag(c, (temp & 0x80) >> 7);
    this->bus->set_memory(address, result, MEMORY_ACCESS_TYPE::cpu);

    (result == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);

    return 16;
}

int CPU::ins_SRA_n(Byte* registerOne, Word address)
{
    this->registers.set_flag(n, 0);
    this->registers.set_flag(h, 0);

    Byte bit7 = 0;
    // shift right into carry, msb does not change
    if (registerOne)
    {
        this->registers.set_flag(c, *registerOne & 0x1);
        bit7 = *registerOne >> 7;

        *registerOne = (*registerOne >> 1) | (bit7 << 7);
        (*registerOne == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
        return 8;
    }

    Byte temp = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);

    this->registers.set_flag(c, temp & 0x1);
    bit7 = temp >> 7;

    Byte result = (temp >> 1) | (bit7 << 7);
    this->bus->set_memory(address, result, MEMORY_ACCESS_TYPE::cpu);

    (result == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);

    return 16;
}

int CPU::ins_SRL_n(Byte* registerOne, Word address)
{
    this->registers.set_flag(n, 0);
    this->registers.set_flag(h, 0);

    // shift right into carry, msb does not change
    if (registerOne)
    {
        this->registers.set_flag(c, *registerOne & 0x1);
        *registerOne = *registerOne >> 1;
        (*registerOne == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
        return 8;
    }
    Byte temp = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);
    Byte result = temp >> 1;

    this->registers.set_flag(c, temp & 0x1);

    this->bus->set_memory(address, result, MEMORY_ACCESS_TYPE::cpu);
    (result == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
    return 16;
}

int CPU::ins_BIT_b_r(Byte bit, Byte* registerOne, Word address)
{
    this->registers.set_flag(n, 0);
    this->registers.set_flag(h, 1);

    if (registerOne)
    {
        ((*registerOne & (1 << bit)) == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
        return 8;
    }

    ((this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu) & (1 << bit)) == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
    return 16;
}

int CPU::ins_SET_b_r(Byte bit, Byte* registerOne, Word address)
{
    if (registerOne)
    {
        *registerOne |= (1 << bit);
        return 8;
    }

    this->bus->set_memory(address, this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu) | (1 << bit), MEMORY_ACCESS_TYPE::cpu);
    return 16;
}

int CPU::ins_RES_b_r(Byte bit, Byte* registerOne, Word address)
{
    if (registerOne)
    {
        *registerOne &= ~(1 << bit);
        return 8;
    }

    this->bus->set_memory(address, this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu) & ~(1 << bit), MEMORY_ACCESS_TYPE::cpu);
    return 16;
}

int CPU::ins_JP_nn(Word address)
{
    this->registers.pc = address;
    return 12;
}

int CPU::ins_JP_cc_nn(const enum JumpCondition condition, Word address)
{
    switch (condition)
    {
    case NZ:
    {
        if (this->registers.get_flag(z) == 0)
        {
            this->ins_JP_nn(address);
            return 16;
        }
    } break;

    case Z:
    {
        if (this->registers.get_flag(z) == 1)
        {
            this->ins_JP_nn(address);
            return 16;
        }
    } break;

    case NC:
    {
        if (this->registers.get_flag(c) == 0)
        {
            this->ins_JP_nn(address);
            return 16;
        }
    } break;

    case C:
    {
        if (this->registers.get_flag(c) == 1)
        {
            this->ins_JP_nn(address);
            return 16;
        }
    } break;

    default: throw "Unreachable Jump condition"; break;
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

    return 12;
}

int CPU::ins_JR_cc_n(const enum JumpCondition condition, Byte_s jumpOffset)
{
    switch (condition)
    {
    case NZ:
    {
        if (!this->registers.get_flag(z))
        {
            this->ins_JR_n(jumpOffset);
            return 12;
        }
    } break;

    case Z:
    {
        if (this->registers.get_flag(z))
        {
            this->ins_JR_n(jumpOffset);
            return 12;
        }
    } break;

    case NC:
    {
        if (!this->registers.get_flag(c))
        {
            this->ins_JR_n(jumpOffset);
            return 12;
        }
    } break;

    case C:
    {
        if (this->registers.get_flag(c))
        {
            this->ins_JR_n(jumpOffset);
            return 12;
        }
    } break;
    default: throw "Unreachable Jump condition"; break;
    }
    return 8;
}

int CPU::ins_CALL_nn(Word address)
{
    this->ins_PUSH_nn(this->registers.pc);
    this->ins_JP_nn(address);
    return 12;
}

int CPU::ins_CALL_cc_nn(enum JumpCondition condition, Word address)
{

    switch (condition)
    {
    case NZ:
    {
        if (!this->registers.get_flag(z))
        {
            this->ins_CALL_nn(address);
            return 24;
        }
    } break;

    case Z:
    {
        if (this->registers.get_flag(z))
        {
            this->ins_CALL_nn(address);
            return 24;
        }
    } break;

    case NC:
    {
        if (!this->registers.get_flag(c))
        {
            this->ins_CALL_nn(address);
            return 24;
        }
    } break;

    case C:
    {
        if (this->registers.get_flag(c))
        {
            this->ins_CALL_nn(address);
            return 24;
        }
    } break;
    default: throw "Unreachable Jump condition"; break;
    }
    return 12;
}

int CPU::ins_RST_n(const Byte addrOffset)
{
    this->ins_CALL_nn(0x0000 + addrOffset);
    return 16;
}


int CPU::ins_RET()
{
    Word SP = this->registers.sp;
    this->registers.pc = this->bus->get_memory_word_lsbf(SP, MEMORY_ACCESS_TYPE::cpu);
    this->registers.sp += 2;
    return 16;
}

int CPU::ins_RETI()
{
    this->interrupt_master_enable = 1;
    this->ins_RET();
    return 16;
}

int CPU::ins_RET_cc(const enum JumpCondition condition)
{
    switch (condition)
    {
    case NZ:
    {
        if (!this->registers.get_flag(z))
        {
            this->ins_RET();
            return 20;
        }
    } break;

    case Z:
    {
        if (this->registers.get_flag(z))
        {
            this->ins_RET();
            return 20;
        }
    } break;

    case NC:
    {
        if (!this->registers.get_flag(c))
        {
            this->ins_RET();
            return 20;
        }
    } break;

    case C:
    {
        if (this->registers.get_flag(c))
        {
            this->ins_RET();
            return 20;
        }
    } break;
    default: throw "Unreachable Jump condition"; break;
    }
    // If we don't require jumping
    return 8;
}
