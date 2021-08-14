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

    const bool get_flag(enum Flags flag)
    {
        return this->f & flag;
    };

    void set_flag(enum Flags flag, bool value)
    {
        (value) ? this->f |= flag : this->f &= ~flag;
    };

    const Word get_word(Byte *registerOne, Byte *registerTwo)
    {
        return ((*registerOne << 8) | *registerTwo);
    };

    void set_word(Byte *registerOne, Byte *registerTwo, Word value)
    {
        *registerOne = ((value & 0xFF00) >> 8);
        *registerTwo = (value & 0xFF);
    };

    
    const Byte get_nibble(Byte *registerOne, bool getHi)
    {
        Byte result = 0;
        (getHi) ? result = (*registerOne & 0xF0) >> 4 : result = *registerOne & 0x0F;
        return result;
    };

    void set_nibble(Byte *registerOne, Byte value, bool setHi)
    {
        (setHi) ? *registerOne = ((*registerOne & 0x0F) | (value << 4)) : *registerOne = ((*registerOne & 0xF0) | value);
    };
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
        Byte get_byte();


        /// <summary>
        /// A function that emulates the execution of the CPU in one go
        /// </summary>
        /// <returns>The number of CPU cycles used</returns>
        int fetch_decode_execute();
        



};