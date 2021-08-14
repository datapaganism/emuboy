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


Byte CPU::get_byte()
{
    Byte opcode = this->bus->get_memory(this->registers.pc);
    std::cout << "opcode " << opcode << "\n";
    this->registers.pc++;
    return opcode;
}

void CPU::connect_to_bus(BUS *pBus)
{
    this->bus = pBus;
}


int CPU::fetch_decode_execute()
{
    // the program counter holds the address in memory for either instruction or data for an instruction.
    this->get_byte();
    return 0;
}