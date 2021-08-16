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


Byte CPU::get_byte_from_pc()
{
    return this->bus->get_memory(this->registers.pc++);
}

Word CPU::get_word_from_pc()
{
    Byte temp = this->get_byte_from_pc();
    return (temp << 8) | this->get_byte_from_pc();
}

void CPU::connect_to_bus(BUS *pBus)
{
    this->bus = pBus;
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
    if (address && value)
    {
        this->bus->set_memory(address, value);
        return 12;
    }
    return -1;
}




int CPU::fetch_decode_execute()
{
    // the program counter holds the address in memory for either instruction or data for an instruction.
    Byte opcode = this->get_byte_from_pc();
    int cyclesUsed = 0;

    //skipped
    // 0xfa lda a,(nn) : 16
    // 0xea lda (nn),a : 16

    switch (opcode) 
    {
        case 0x00:{ } break;
        case 0x01:{ } break;
        case 0x02:{ this->ins_LD_r1_r2(nullptr, this->registers.get_BC(), &this->registers.a); } break;
        case 0x03:{ } break;
        case 0x04:{ } break;
        case 0x05:{ } break;
        case 0x06:{ cyclesUsed = this->ins_LD_nn_n(&this->registers.b, this->get_byte_from_pc()); } break;
        case 0x07:{ } break;
        case 0x08:{ } break;
        case 0x09:{ } break;
        case 0x0A:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, this->registers.get_BC()); } break;
        case 0x0B:{ } break;
        case 0x0C:{ } break;
        case 0x0D:{ } break;
        case 0x0E:{ cyclesUsed = this->ins_LD_nn_n(&this->registers.c, this->get_byte_from_pc()); } break;
        case 0x0F:{ } break;
        
        case 0x10:{ } break;
        case 0x11:{ } break;
        case 0x12:{ this->ins_LD_r1_r2(nullptr, this->registers.get_DE(), &this->registers.a); } break;
        case 0x13:{ } break;
        case 0x14:{ } break;
        case 0x15:{ } break;
        case 0x16:{ cyclesUsed = this->ins_LD_nn_n(&this->registers.d, this->get_byte_from_pc()); } break;
        case 0x17:{ } break;
        case 0x18:{ } break;
        case 0x19:{ } break;
        case 0x1A:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, this->registers.get_DE()); } break;
        case 0x1B:{ } break;
        case 0x1C:{ } break;
        case 0x1D:{ } break;
        case 0x1E:{ cyclesUsed = this->ins_LD_nn_n(&this->registers.e, this->get_byte_from_pc()); } break;
        case 0x1F:{ } break;

        case 0x20:{ } break;
        case 0x21:{ } break;
        case 0x22:{ } break;
        case 0x23:{ } break;
        case 0x24:{ } break;
        case 0x25:{ } break;
        case 0x26:{ cyclesUsed = this->ins_LD_nn_n(&this->registers.h, this->get_byte_from_pc()); } break;
        case 0x27:{ } break;
        case 0x28:{ } break;
        case 0x29:{ } break;
        case 0x2A:{ } break;
        case 0x2B:{ } break;
        case 0x2C:{ } break;
        case 0x2D:{ } break;
        case 0x2E:{ cyclesUsed = this->ins_LD_nn_n(&this->registers.l, this->get_byte_from_pc()); } break;
        case 0x2F:{ } break;
        case 0x30:{ } break;
        case 0x31:{ } break;
        case 0x32:{ } break;
        case 0x33:{ } break;
        case 0x34:{ } break;
        case 0x35:{ } break;
        case 0x36:{ cyclesUsed = this->ins_LD_r1_r2(nullptr, this->registers.get_HL(), nullptr, this->get_byte_from_pc()); } break;
        case 0x37:{ } break;
        case 0x38:{ } break;
        case 0x39:{ } break;
        case 0x3A:{ } break;
        case 0x3B:{ } break;
        case 0x3C:{ } break;
        case 0x3D:{ } break;
        case 0x3E:{ cyclesUsed = this->ins_LD_nn_n(&this->registers.a, this->get_byte_from_pc()); } break;
        case 0x3F:{ } break;

        case 0x40:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.b, NULL, &this->registers.b); } break;
        case 0x41:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.b, NULL, &this->registers.c); } break;
        case 0x42:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.b, NULL, &this->registers.d); } break;
        case 0x43:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.b, NULL, &this->registers.e); } break;
        case 0x44:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.b, NULL, &this->registers.h); } break;
        case 0x45:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.b, NULL, &this->registers.l); } break;
        case 0x46:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.b, this->registers.get_HL()); } break;
        case 0x47:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.b, NULL, &this->registers.a); } break;
        case 0x48:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.c, NULL, &this->registers.b); } break;
        case 0x49:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.c, NULL, &this->registers.c); } break;
        case 0x4A:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.c, NULL, &this->registers.d); } break;
        case 0x4B:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.c, NULL, &this->registers.e); } break;
        case 0x4C:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.c, NULL, &this->registers.h); } break;
        case 0x4D:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.c, NULL, &this->registers.l); } break;
        case 0x4E:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.c, this->registers.get_HL()); } break;
        case 0x4F:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.c, NULL, &this->registers.a); } break;

        case 0x50:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.d, NULL, &this->registers.b); } break;
        case 0x51:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.d, NULL, &this->registers.c); } break;
        case 0x52:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.d, NULL, &this->registers.d); } break;
        case 0x53:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.d, NULL, &this->registers.e); } break;
        case 0x54:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.d, NULL, &this->registers.h); } break;
        case 0x55:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.d, NULL, &this->registers.l); } break;
        case 0x56:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.d, this->registers.get_HL()); } break;
        case 0x57:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.d, NULL, &this->registers.a); } break;
        case 0x58:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.e, NULL, &this->registers.b); } break;
        case 0x59:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.e, NULL, &this->registers.c); } break;
        case 0x5A:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.e, NULL, &this->registers.d); } break;
        case 0x5B:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.e, NULL, &this->registers.e); } break;
        case 0x5C:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.e, NULL, &this->registers.h); } break;
        case 0x5D:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.e, NULL, &this->registers.l); } break;
        case 0x5E:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.e, this->registers.get_HL()); } break;
        case 0x5F:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.e, NULL, &this->registers.a); } break;
        case 0x60:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.h, NULL, &this->registers.b); } break;
        case 0x61:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.h, NULL, &this->registers.c); } break;
        case 0x62:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.h, NULL, &this->registers.d); } break;
        case 0x63:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.h, NULL, &this->registers.e); } break;
        case 0x64:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.h, NULL, &this->registers.h); } break;
        case 0x65:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.h, NULL, &this->registers.l); } break;
        case 0x66:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.h, this->registers.get_HL()); } break;
        case 0x67:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.h, NULL, &this->registers.a); } break;
        case 0x68:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.l, NULL, &this->registers.b); } break;
        case 0x69:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.l, NULL, &this->registers.c); } break;
        case 0x6A:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.l, NULL, &this->registers.d); } break;
        case 0x6B:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.l, NULL, &this->registers.e); } break;
        case 0x6C:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.l, NULL, &this->registers.h); } break;
        case 0x6D:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.l, NULL, &this->registers.l); } break;
        case 0x6E:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.l, this->registers.get_HL()); } break;
        case 0x6F:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.l, NULL, &this->registers.a); } break;

        case 0x70:{ cyclesUsed = this->ins_LD_r1_r2(nullptr,this->registers.get_HL(), &this->registers.b); } break;
        case 0x71:{ cyclesUsed = this->ins_LD_r1_r2(nullptr,this->registers.get_HL(), &this->registers.c); } break;
        case 0x72:{ cyclesUsed = this->ins_LD_r1_r2(nullptr,this->registers.get_HL(), &this->registers.d); } break;
        case 0x73:{ cyclesUsed = this->ins_LD_r1_r2(nullptr,this->registers.get_HL(), &this->registers.e); } break;
        case 0x74:{ cyclesUsed = this->ins_LD_r1_r2(nullptr,this->registers.get_HL(), &this->registers.h); } break;
        case 0x75:{ cyclesUsed = this->ins_LD_r1_r2(nullptr,this->registers.get_HL(), &this->registers.l); } break;
        case 0x76:{ } break;
        case 0x77:{ this->ins_LD_r1_r2(nullptr,this->registers.get_HL(), &this->registers.a); } break;
        case 0x78:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, NULL, &this->registers.b); } break;
        case 0x79:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, NULL, &this->registers.c); } break;
        case 0x7A:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, NULL, &this->registers.d); } break;
        case 0x7B:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, NULL, &this->registers.e); } break;
        case 0x7C:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, NULL, &this->registers.h); } break;
        case 0x7D:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, NULL, &this->registers.l); } break;
        case 0x7E:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.a,this->registers.get_HL()); } break;
        case 0x7F:{ cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, NULL, &this->registers.a); } break;

        case 0x80:{ } break;
        case 0x81:{ } break;
        case 0x82:{ } break;
        case 0x83:{ } break;
        case 0x84:{ } break;
        case 0x85:{ } break;
        case 0x86:{ } break;
        case 0x87:{ } break;
        case 0x88:{ } break;
        case 0x89:{ } break;
        case 0x8A:{ } break;
        case 0x8B:{ } break;
        case 0x8C:{ } break;
        case 0x8D:{ } break;
        case 0x8E:{ } break;
        case 0x8F:{ } break;

        case 0x90:{ } break;
        case 0x91:{ } break;
        case 0x92:{ } break;
        case 0x93:{ } break;
        case 0x94:{ } break;
        case 0x95:{ } break;
        case 0x96:{ } break;
        case 0x97:{ } break;
        case 0x98:{ } break;
        case 0x99:{ } break;
        case 0x9A:{ } break;
        case 0x9B:{ } break;
        case 0x9C:{ } break;
        case 0x9D:{ } break;
        case 0x9E:{ } break;
        case 0x9F:{ } break;

        case 0xA0:{ } break;
        case 0xA1:{ } break;
        case 0xA2:{ } break;
        case 0xA3:{ } break;
        case 0xA4:{ } break;
        case 0xA5:{ } break;
        case 0xA6:{ } break;
        case 0xA7:{ } break;
        case 0xA8:{ } break;
        case 0xA9:{ } break;
        case 0xAA:{ } break;
        case 0xAB:{ } break;
        case 0xAC:{ } break;
        case 0xAD:{ } break;
        case 0xAE:{ } break;
        case 0xAF:{ } break;

        case 0xB0:{ } break;
        case 0xB1:{ } break;
        case 0xB2:{ } break;
        case 0xB3:{ } break;
        case 0xB4:{ } break;
        case 0xB5:{ } break;
        case 0xB6:{ } break;
        case 0xB7:{ } break;
        case 0xB8:{ } break;
        case 0xB9:{ } break;
        case 0xBA:{ } break;
        case 0xBB:{ } break;
        case 0xBC:{ } break;
        case 0xBD:{ } break;
        case 0xBE:{ } break;
        case 0xBF:{ } break;

        case 0xC0:{ } break;
        case 0xC1:{ } break;
        case 0xC2:{ } break;
        case 0xC3:{ } break;
        case 0xC4:{ } break;
        case 0xC5:{ } break;
        case 0xC6:{ } break;
        case 0xC7:{ } break;
        case 0xC8:{ } break;
        case 0xC9:{ } break;
        case 0xCA:{ } break;
        case 0xCB:{ } break;
        case 0xCC:{ } break;
        case 0xCD:{ } break;
        case 0xCE:{ } break;
        case 0xCF:{ } break;

        case 0xD0:{ } break;
        case 0xD1:{ } break;
        case 0xD2:{ } break;
        case 0xD3:{ } break;
        case 0xD4:{ } break;
        case 0xD5:{ } break;
        case 0xD6:{ } break;
        case 0xD7:{ } break;
        case 0xD8:{ } break;
        case 0xD9:{ } break;
        case 0xDA:{ } break;
        case 0xDB:{ } break;
        case 0xDC:{ } break;
        case 0xDD:{ } break;
        case 0xDE:{ } break;
        case 0xDF:{ } break;

        case 0xE0:{ } break;
        case 0xE1:{ } break;
        case 0xE2:{ } break;
        case 0xE3:{ } break;
        case 0xE4:{ } break;
        case 0xE5:{ } break;
        case 0xE6:{ } break;
        case 0xE7:{ } break;
        case 0xE8:{ } break;
        case 0xE9:{ } break;
        case 0xEA:{ } break;
        case 0xEB:{ } break;
        case 0xEC:{ } break;
        case 0xED:{ } break;
        case 0xEE:{ } break;
        case 0xEF:{ } break;

        case 0xF0:{ } break;
        case 0xF1:{ } break;
        case 0xF2:{ } break;
        case 0xF3:{ } break;
        case 0xF4:{ } break;
        case 0xF5:{ } break;
        case 0xF6:{ } break;
        case 0xF7:{ } break;
        case 0xF8:{ } break;
        case 0xF9:{ } break;
        case 0xFA:{ } break;
        case 0xFB:{ } break;
        case 0xFC:{ } break;
        case 0xFD:{ } break;
        case 0xFE:{ } break;
        case 0xFF:{ } break;
  
    }

    return cyclesUsed;
}


