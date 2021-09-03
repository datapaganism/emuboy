#include "cpu.hpp"
#include "bus.hpp"

#include <iostream>
#include <bitset>

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
    printf("%s:","0xFF00");
    std::cout << std::bitset<8>(this->bus->io[0]);

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
//
//void CPU::DEBUG_init()
//{
//    this->registers.pc = 0x100;
//    this->registers.a = 0x11;
//    this->registers.b = 0x00;
//    this->registers.c = 0x00;
//    this->registers.d = 0xff;
//    this->registers.e = 0x56;
//    this->registers.f = 0x80;
//    this->registers.h = 0x00;
//    this->registers.l = 0x0d;
//    this->registers.sp = 0xFFFE;
//}



CPU::CPU()
{
    this->init();
#if DEBUG 1
    //this->DEBUG_init();
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
        this->interrupt_master_enable = 0;
        this->DI_triggered = false;
        return;
    }
    if (this->EI_triggered)
    {
        this->interrupt_master_enable = 1;
        this->EI_triggered = false;
        return;
    }


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

void CPU::checkHalfCarryWord(int a, int b)
{
    //make sum of lower bytes
    int sum = (a & 0xFF) + (b & 0xFF);

        if ((sum & 0x1000) == 0x1000)
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

    if ((sum & 0x100) == 0x100)
    {
        this->registers.set_flag(c, 1);
        return;
    }
    this->registers.set_flag(c, 0);
    return;
}

void CPU::checkCarryWord(const int a, const int b)
{
    uint32_t sum = a + b;

    
    if ((sum & 0x10000) == 0x10000)
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
    
}void CPU::checkHalfBorrowWord(const int a, const int b)
{
    if ((a & 0xff) >= (b & 0xff))
    {
        this->registers.set_flag(h, 0);
        return;
    }
    this->registers.set_flag(h, 1);
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

/// <summary>
/// This is the master interrupt function, we need to emulate interrupts, There is a master switch for interrupt enable which takes prority over all interrupts, this is being modelled as a boolean inside the CPU class.
/// The only way this bool is ever modified is during the DI and EI instructions of the processor.
/// 
/// interrupts need to be requested externally to the CPU, this is done with request_interrupt function, there are only 5 types of interrupts that can be requested, so when we enter an interrupt period in a different part of hardware we write to a special register stating the type of interrupt.
/// Interrupts per type can be enabled or disabled accordingly modifiying using the IE register, if the master is enabled, then we need to check through each type of interrupt to see if it has been requested (and enabled, since if it has been requested but disabled then that interrupt will need to wait the next cpu cycle)
/// if it has been requested then we aknowledge the interrupt by turning off the request flag and disabling the master interrupt switch, at that point we push the current program counter onto the stack and depending on the interrupt type set the program counter to its corresponding interrupt vector.
/// </summary>
/// <returns></returns>
int CPU::do_interrupts()
{
    int cyclesUsed = 0;
    //check master to see if interrupts are enabled
    if (this->interrupt_master_enable == true)
    {
        // iterate through all types, this allows us to check each interrupt and also service them in order of priority
        for (const auto type : InterruptTypes_all)
        {
            
            // if it has been requested and its corresponding flag is enabled
            if (this->get_interrupt_flag(type, IF_REGISTER) && this->get_interrupt_flag(type, IE_REGISTER))
            {
                //disable interrupts in general and for this type
                this->set_interrupt_flag(type, 0, IF_REGISTER);
                this->interrupt_master_enable = 0;

                //push the current pc to the stack
                this->ins_PUSH_nn(this->registers.pc);

                // jump
                switch (type)
                {
                case InterruptTypes::vblank  : { this->registers.pc = 0x0040; } break;
                case InterruptTypes::lcdstat : { this->registers.pc = 0x0048; } break;
                case InterruptTypes::timer   : { this->registers.pc = 0x0050; } break;
                case InterruptTypes::serial  : { this->registers.pc = 0x0058; } break;
                case InterruptTypes::joypad  : { this->registers.pc = 0x0060; } break;
                }

                cyclesUsed += 20;
            }
        }
    }
    return cyclesUsed;
}

/// <summary>
/// This is the master function for handling the CPU timers, timers do not increment every CPU cycle but after a specified amount of cycles.
/// Since there are two types of timers (normal timer and div timer) we need to store a rolling counter of how many cycles have been executed before the timer needs to advance.
/// this is done in the divTimerCounter and timerCounter respectively, each hold the amount of cycles it takes for the counter to increment.
/// 
/// the div counter increments once every 256 counters, we remove the mount of cycles done this CPU 'tick' from divTimerCounter and when that reaches 0 or less then we increment the register at position DIV, however the Gameboy cannot directly write to postion DIV since every write will reset the register.
/// so we access the io regsiters directly and increment the register.
/// divCounter is reset back to its initial value and we can continue all over again.
/// 
/// the TAC, timer controller is responsible for two things, enabling the timer and setting the rate of increment, the 2nd bit of the TAC shows if it is enabled. we modify timerCounter and check if the register needs incrementing, of the value of the TIMA register is 0xFF (TIMA is like DIV) then we need to overfllow
/// and when the TIMA overflows we set to the value of TMA and request an interrupt of type timer.
/// </summary>
/// <param name="cycles"></param>
void CPU::update_timers(const int cycles)
{
    //run the DIV timer, every 256cpu cycles we need to increment the DIV register once,
    this->divTimerCounter -= cycles;
    if (this->divTimerCounter <= 0)
    {
        this->divTimerCounter = DIVinit;
        //register is read only to gameboy
        this->bus->io.at(DIV - IOOFFSET)++;
    }

    // if TMC bit 2 is set, this means that the timer is enabled
    if (this->bus->get_memory(TAC) & (0b00000100))
    {
        //the timer increment holds the amount of cycles needed to tick TIMA register up one, we can decrement it and see if it has hit 0
        this->timerCounter -= cycles;

        // if the TIMA needs updating
        if (this->timerCounter <= 0)
        {
            if (this->bus->get_memory(TIMA) == 0xFF)
            {
                this->bus->set_memory(TIMA, this->bus->get_memory(TMA));
                this->request_interrupt(timer);
            }

            this->bus->set_memory(TIMA, this->bus->get_memory(TIMA)+1);
        }
    }
}

Byte CPU::get_TMC_frequency()
{
    return this->bus->get_memory(TAC) & 0x3;
}

void CPU::update_timerCounter()
{
    switch (this->get_TMC_frequency())
    {
    case 0: { this->timerCounter = GB_CLOCKSPEED / 4096; } break;
    case 1: { this->timerCounter = GB_CLOCKSPEED / 262144; } break;
    case 2: { this->timerCounter = GB_CLOCKSPEED / 65536; } break;
    case 3: { this->timerCounter = GB_CLOCKSPEED / 16382; } break;
    }
}


void CPU::request_interrupt(const InterruptTypes type)
{
    this->set_interrupt_flag(type, 1, IF_REGISTER);
}

Byte CPU::get_interrupt_flag(const enum InterruptTypes type, Word address)
{
    return this->bus->get_memory(address) & type;
}

void CPU::set_interrupt_flag(const enum InterruptTypes type, const bool value, Word address)
{
    (value) ? this->bus->set_memory(address, this->bus->get_memory(address) | type) : this->bus->set_memory(address, this->bus->get_memory(address) & ~type);
}





//INSTRUCTION HANDLING
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
    this->checkCarryWord(stackPointerValue, value);
    // this is weird, wrong behaviour when using checkHalfCarryWord
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
    this->bus->set_memory(this->registers.sp, ((wordRegisterValue & 0xff00) >> 8));
    this->registers.sp--;
    this->bus->set_memory(this->registers.sp, (wordRegisterValue & 0x00ff));
    this->registers.sp--;

    return 16;
}

int CPU::ins_POP_nn(Byte* registerOne, Byte* registerTwo)
{
    this->registers.sp++;
    *registerTwo = this->bus->get_memory(this->registers.sp);
    this->registers.sp++;
    *registerOne = this->bus->get_memory(this->registers.sp);

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
        this->ins_SUB_n(nullptr, *registerOne);
        this->registers.a -= +this->registers.get_flag(c);
        return 4;
    }
    this->ins_SUB_n(nullptr, immediateValue);
    this->registers.a -= +this->registers.get_flag(c);
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
    (this->bus->get_memory(address) == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
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
    (this->bus->get_memory(address) == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
    this->registers.set_flag(n, 1);

    return 12;
}

int CPU::ins_ADD_HL_n(const Word value)
{
    Word HLvalue = this->registers.get_HL();
    this->checkCarryWord(HLvalue, value);
    this->checkHalfCarryWord(HLvalue, value);

    this->registers.set_HL(HLvalue + value);
    
    this->registers.set_flag(n, 0);
    return 8;
}

int CPU::ins_ADD_SP_n(const Byte_s value)
{
    //if negative
    if (value < 0)
    {
        this->checkBorrow((this->registers.sp & 0xff), value);
        this->checkHalfCarry(this->registers.sp, value);
    }
    else
    {
        this->checkCarry((Byte)(this->registers.sp & 0xff), value);
        this->checkHalfCarry(this->registers.sp, value);
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
    int cyclesUsed = 0;
    if (registerOne)
    {
        *registerOne = (this->get_nibble(*registerOne, false) << 4) + this->get_nibble(*registerOne, true);
        (*registerOne == 0x0) ? this->registers.set_flag(z, true) : this->registers.set_flag(z, false);
        cyclesUsed = 8;
    }
    else
    {
        this->bus->set_memory(address, (this->get_nibble(this->bus->get_memory(address), false) << 4) + this->get_nibble(this->bus->get_memory(address), true));
        (this->bus->get_memory(address) == 0x0) ? this->registers.set_flag(z, true) : this->registers.set_flag(z, false);
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
    
    // temp store of value of the carry bit, set to reset
    bool carry = false;

    bool flagZ = this->registers.get_flag(z);
    bool flagN = this->registers.get_flag(n);
    bool flagH = this->registers.get_flag(h);
    bool flagC = this->registers.get_flag(c);

    if (flagC)
        adjust += 0x60;
    
    if (((this->registers.a & 0xf) > 9) | flagH)
        adjust += 0x06;

    if (((this->registers.a & 0xff) > 90) && (flagH) && !(flagC) && !(flagN))
    {
        adjust = 0x66;
        carry = true;
    }

    if ((this->registers.a & 0xff) > 99)
    {
        adjust = 0x66;
        carry = true;
    }

    if (flagC && flagN)
    {
        adjust = 0xA0;
        //carry = true;
    }

    if (flagH && flagC)
    {
        adjust = 0x66;
        carry = true;
    }

    if (flagH && flagN)
    {
        adjust = 0xFA;
    }
    if (flagH && flagN && flagC)
    {
        adjust = 0x9A;
        carry = true;
    }

    (carry) ? this->registers.set_flag(c, 1) : this->registers.set_flag(c, 0);
    this->registers.a += adjust;
    (this->registers.a == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
    this->registers.set_flag(h, 0);

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

int CPU::ins_JP_cc_nn(const enum JumpCondition condition, Word address)
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

int CPU::ins_JR_cc_n(const enum JumpCondition condition, Byte_s jumpOffset)
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
    this->interrupt_master_enable = 1;
    return 12;
}

int CPU::ins_RET_cc(const enum JumpCondition condition)
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









