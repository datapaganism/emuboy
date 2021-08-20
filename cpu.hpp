#pragma once

#include "config.h"
#include <iostream>



// forward declaration, bus.hpp is not included here but in cpu.cpp instead
class BUS;

enum Flags 
{
    z = 0b01000000, // Zero
    n = 0b00100000, // Subtraction
    h = 0b00010000, // Half Carry
    c = 0b00001000, // Carry
};


// The registers of the CPU, including SP and PC, access single registers directly, use functions to get combinations of registers as a word, nibble manipulation of single bytes as well.
struct Registers
{
public:
    Byte a = 0;
    Byte b = 0;
    Byte c = 0;
    Byte d = 0;
    Byte e = 0;
    Byte f = 0;
    Byte h = 0;
    Byte l = 0;
    Word sp = 0;
    Word pc = 0;



    const Word get_AF() { return this->get_word(&this->a, &this->f); };
    const Word get_BC() { return this->get_word(&this->b, &this->c); };
    const Word get_DE() { return this->get_word(&this->d, &this->e); };
    const Word get_HL() { return this->get_word(&this->h, &this->l); };

    void set_AF(const Word value) { this->set_word(&this->a, &this->f, value); };
    void set_BC(const Word value) { this->set_word(&this->b, &this->c, value); };
    void set_DE(const Word value) { this->set_word(&this->d, &this->e, value); };
    void set_HL(const Word value) { this->set_word(&this->h, &this->l, value); };

    bool get_flag(const Flags flag)
    {
        return this->f & flag;
    }

    void set_flag(const Flags flag, const bool value)
    {
        (value) ? this->f |= flag : this->f &= ~flag;
    }

    Byte get_nibble(Byte* registerOne, const bool getHi)
    {
        Byte result = 0;
        (getHi) ? result = (*registerOne & 0xF0) >> 4 : result = *registerOne & 0x0F;
        return result;
    }

    void set_nibble(Byte* registerOne, const Byte value, const bool setHi)
    {
        (setHi) ? *registerOne = ((*registerOne & 0x0F) | (value << 4)) : *registerOne = ((*registerOne & 0xF0) | value);
    }

    const Word get_word(Byte* registerOne, Byte* registerTwo)
    {
        return ((*registerOne << 8) | *registerTwo);
    }

    void set_word(Byte* registerOne, Byte* registerTwo, const Word value)
    {
        *registerOne = ((value & 0xFF00) >> 8);
        *registerTwo = (value & 0xFF);
    }
};


class CPU
{

    private:
        
        BUS* bus = nullptr;

        
        
        /// <summary>
        /// sets up the CPU registers to fresh boot state.
        /// </summary>
        void init();
        
        // The way I want the system to be emulated is to have master class where all the components are accessed, the address bus is how are going to achieve this,
        // the address bus has a cpu attached to it but the cpu itself needs to be connected to the bus to access other devices.
        
    public:
        Registers registers;
        CPU();



        /// <summary>
        /// Allows a CPU to access any device on the BUS
        /// </summary>
        /// <param name="pBus">pointer to a BUS obj</param>
        void connect_to_bus(BUS *pBus);
        

        /// <summary>
        /// Gets Byte from memory at address pointed to by the PC, also increments PC by a byte, better to make it an atomic operation
        /// </summary>
        /// <returns>One byte in memory</returns>
        const Byte get_byte_from_pc();
        
        const Word get_word_from_pc();

        /// <summary>
        /// returns two bytes from memory pointed to by PC, returned in least significant byte first (lsbf)
        /// </summary>
        const Word get_word_from_pc_lsbf();


        /// <summary>
        /// A function that emulates the execution of the CPU in one go
        /// </summary>
        /// <returns>The number of CPU cycles used</returns>
        int fetch_decode_execute();

private:
        /// <summary>
        /// Takes two integers, evaluates whether the sum of the lower nibble carries over to the high nibble, sets flag accordingly.
        /// </summary>
        void checkHalfCarry(const int a, const int b);
        void checkCarry(const int a, const int b);
        void checkHalfBorrow(const int a, const int b);
        void checkBorrow(const int a, const int b);

        int ins_LD_nn_n(Byte* registerOne, const Byte value);
        int ins_LD_r1_r2(Byte* registerOne = nullptr, const Word address = NULL, Byte* registerTwo = nullptr, const Byte value = NULL);

        int ins_LD_r1_nn(Byte* registerOne, const Word address, const int cyclesUsed);
        int ins_LD_nn_r1(const Word address, Byte* registerOne);

        int ins_LDDI_nn_r1(const Word address, const Byte* registerOne, Byte* registerTwo, Byte* registerThree,const int addSubValue);
        int ins_LDDI_r1_nn(Byte* registerOne, const Word address, Byte* registerTwo, Byte* registerThree,const int addSubValue);

        int ins_LD_n_nn(Word* wordRegister = nullptr, Byte* registerOne = nullptr, Byte* registerTwo = nullptr, const Word value = NULL);
        int ins_LD_nn_nn(Word* wordRegisterOne, const Word value);

        int ins_LDHL_SP_n(Byte* wordRegisterNibbleHi, Byte* wordRegisterNibbleLo, const Word stackPointerValue, const Byte value);
        int ins_LD_nn_SP(const Word address, const Word stackPointerValue);

        int ins_PUSH_nn(const Word wordRegisterValue);
        int ins_POP_nn(Byte* registerOne, Byte* registerTwo);

        int ins_ADD_A_n(const Byte* registerOne = nullptr, const Byte immediateValue = NULL);
        int ins_ADC_A_n(const Byte* registerOne = nullptr, const Byte immediateValue = NULL);

        int ins_SUB_n(const Byte* registerOne = nullptr, const Byte immediateValue = NULL);
        int ins_SBC_A_n(const Byte* registerOne = nullptr, const Byte immediateValue = NULL);

        int ins_AND_n(const Byte* registerOne = nullptr, const Byte immediateValue = NULL);
        int ins_OR_n(const Byte* registerOne = nullptr, const Byte immediateValue = NULL);

        int ins_XOR_n(const Byte* registerOne = nullptr, const Byte immediateValue = NULL);
        int ins_CP_n(const Byte* registerOne = nullptr, const Byte immediateValue = NULL);

        int ins_INC_n(Byte* registerOne = nullptr, Word address = NULL);
        int ins_DEC_n(Byte* registerOne = nullptr, Word address = NULL);
};