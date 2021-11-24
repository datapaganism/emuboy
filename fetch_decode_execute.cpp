#include "cpu.hpp"
#include "bus.hpp"

#include <iostream>


/// this function is so big it deserves its own file since its painful to navigate
int CPU::fetch_decode_execute()
{
    
    //system("cls");
    //this->DEBUG_print_IO();


    // ala 4.10 from tcagfb.pdf
    if (this->is_halted)
    {
        if (this->interrupt_master_enable)
        {
            if ((this->bus->get_memory(0xFFFF, MEMORY_ACCESS_TYPE::cpu) && this->bus->get_memory(0xFF0F, MEMORY_ACCESS_TYPE::cpu) && 0x1F) != 0)
            {
                this->is_halted = false;
                this->registers.pc++;
                return 0; // system will get cycles from interrupt handling
            }
            return 1; // so system does something whilst halted.
        }
        if ((this->bus->get_memory(0xFFFF, MEMORY_ACCESS_TYPE::cpu) && this->bus->get_memory(0xFF0F, MEMORY_ACCESS_TYPE::cpu) && 0x1F) == 0)
        {
            this->is_halted = false;
            this->registers.pc++;
            return 0; // system will get cycles from interrupt handling
        }
        // HALT BUG, exit halt mode but do not increase pc, next instuction after halt is duplicated
        if ((this->bus->get_memory(0xFFFF, MEMORY_ACCESS_TYPE::cpu) && this->bus->get_memory(0xFF0F, MEMORY_ACCESS_TYPE::cpu) && 0x1F) != 0)
        {
            this->is_halted = false;
            this->registers.pc++;
            this->halt_bug = true;
            return 0; // system will get cycles from interrupt handling
        }
        return 1; // so system does something whilst halted.
           
    }
    // the program counter holds the address in memory for either instruction or data for an instruction.
    
    if (this->halt_bug)
    {
        this->registers.pc--;
        this->halt_bug = false;
    }
    
    Byte opcode = this->get_byte_from_pc();
    
    //std::cout << std::hex << opcode << std::dec <<" ";
   
    int cyclesUsed = 0;

    

    switch (opcode)
    {
    case 0x00: { cyclesUsed = 4; } break; //NOP
    case 0x01: { cyclesUsed = this->ins_LD_n_nn(nullptr, &this->registers.b, &this->registers.c, this->get_word_from_pc_lsbf()); } break;
    case 0x02: { cyclesUsed = this->ins_LD_r1_r2(nullptr, this->registers.get_BC(), &this->registers.a); } break;
    case 0x03: { cyclesUsed = this->ins_INC_nn(&this->registers.b, &this->registers.c); } break;
    case 0x04: { cyclesUsed = this->ins_INC_n(&this->registers.b); } break;
    case 0x05: { cyclesUsed = this->ins_DEC_n(&this->registers.b); } break;
    case 0x06: { cyclesUsed = this->ins_LD_nn_n(&this->registers.b, this->get_byte_from_pc()); } break;
    case 0x07: { cyclesUsed = this->ins_RLCA(); } break;
   
    case 0x08: { cyclesUsed = this->ins_LD_nn_SP(this->get_word_from_pc_lsbf(), this->registers.sp); } break;
    case 0x09: { cyclesUsed = this->ins_ADD_HL_n(this->registers.get_BC()); } break;
    case 0x0A: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, this->registers.get_BC()); } break;
    case 0x0B: { cyclesUsed = this->ins_DEC_nn(&this->registers.b, &this->registers.c); } break;
    case 0x0C: { cyclesUsed = this->ins_INC_n(&this->registers.c); } break;
    case 0x0D: { cyclesUsed = this->ins_DEC_n(&this->registers.c); } break;
    case 0x0E: { cyclesUsed = this->ins_LD_nn_n(&this->registers.c, this->get_byte_from_pc()); } break;
    case 0x0F: { cyclesUsed = this->ins_RRCA(); } break;

    case 0x10: { cyclesUsed = this->STOP_instruction_handler(); } break;
    case 0x11: { cyclesUsed = this->ins_LD_n_nn(nullptr, &this->registers.d, &this->registers.e, this->get_word_from_pc_lsbf()); } break;
    case 0x12: { cyclesUsed = this->ins_LD_r1_r2(nullptr, this->registers.get_DE(), &this->registers.a); } break;
    case 0x13: { cyclesUsed = this->ins_INC_nn(&this->registers.d, &this->registers.e); } break;
    case 0x14: { cyclesUsed = this->ins_INC_n(&this->registers.d); } break;
    case 0x15: { cyclesUsed = this->ins_DEC_n(&this->registers.d); } break;
    case 0x16: { cyclesUsed = this->ins_LD_nn_n(&this->registers.d, this->get_byte_from_pc()); } break;
    case 0x17: { cyclesUsed = this->ins_RLA(); } break;
    
    case 0x18: { cyclesUsed = this->ins_JR_n(this->get_byte_from_pc()); } break;
    case 0x19: { cyclesUsed = this->ins_ADD_HL_n(this->registers.get_DE()); } break;
    case 0x1A: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, this->registers.get_DE()); } break;
    case 0x1B: { cyclesUsed = this->ins_DEC_nn(&this->registers.d, &this->registers.e); } break;
    case 0x1C: { cyclesUsed = this->ins_INC_n(&this->registers.e); } break;
    case 0x1D: { cyclesUsed = this->ins_DEC_n(&this->registers.e); } break;
    case 0x1E: { cyclesUsed = this->ins_LD_nn_n(&this->registers.e, this->get_byte_from_pc()); } break;
    case 0x1F: { cyclesUsed = this->ins_RRA(); } break;

    case 0x20: { cyclesUsed = this->ins_JR_cc_n(NZ, this->get_byte_from_pc()); } break;
    case 0x21: { cyclesUsed = this->ins_LD_n_nn(nullptr, &this->registers.h, &this->registers.l, this->get_word_from_pc_lsbf()); } break;
    case 0x22: { cyclesUsed = this->ins_LDDI_nn_r1(this->registers.get_HL(), &this->registers.a, &this->registers.h, &this->registers.l, +1); } break;
    case 0x23: { cyclesUsed = this->ins_INC_nn(&this->registers.h, &this->registers.l); } break;
    case 0x24: { cyclesUsed = this->ins_INC_n(&this->registers.h); } break;
    case 0x25: { cyclesUsed = this->ins_DEC_n(&this->registers.h); } break;
    case 0x26: { cyclesUsed = this->ins_LD_nn_n(&this->registers.h, this->get_byte_from_pc()); } break;
    case 0x27: { cyclesUsed = this->ins_DAA(); } break;
    
    case 0x28: { cyclesUsed = this->ins_JR_cc_n(Z, this->get_byte_from_pc()); } break;
    case 0x29: { cyclesUsed = this->ins_ADD_HL_n(this->registers.get_HL()); } break;
    case 0x2A: { cyclesUsed = this->ins_LDDI_r1_nn(&this->registers.a, this->registers.get_HL(), &this->registers.h, &this->registers.l, +1); } break;
    case 0x2B: { cyclesUsed = this->ins_DEC_nn(&this->registers.h, &this->registers.l); } break;
    case 0x2C: { cyclesUsed = this->ins_INC_n(&this->registers.l); } break;
    case 0x2D: { cyclesUsed = this->ins_DEC_n(&this->registers.l); } break;
    case 0x2E: { cyclesUsed = this->ins_LD_nn_n(&this->registers.l, this->get_byte_from_pc()); } break;
    case 0x2F: { cyclesUsed = this->ins_CPL(); } break;

    case 0x30: { cyclesUsed = this->ins_JR_cc_n(NC, this->get_byte_from_pc()); } break;
    case 0x31: { cyclesUsed = this->ins_LD_n_nn(&this->registers.sp, nullptr, nullptr, this->get_word_from_pc_lsbf()); } break;
    case 0x32: { cyclesUsed = this->ins_LDDI_nn_r1(this->registers.get_HL(), &this->registers.a, &this->registers.h, &this->registers.l, -1); } break;
    case 0x33: { cyclesUsed = this->ins_INC_nn(nullptr, nullptr, &this->registers.sp); } break;
    case 0x34: { cyclesUsed = this->ins_INC_n(nullptr, this->registers.get_HL()); } break;
    case 0x35: { cyclesUsed = this->ins_DEC_n(nullptr, this->registers.get_HL()); } break;
    case 0x36: { cyclesUsed = this->ins_LD_r1_r2(nullptr, this->registers.get_HL(), nullptr, this->get_byte_from_pc()); } break;
    case 0x37: { cyclesUsed = this->ins_SCF(); } break;
    
    case 0x38: { cyclesUsed = this->ins_JR_cc_n(C, this->get_byte_from_pc()); } break;
    case 0x39: { cyclesUsed = this->ins_ADD_HL_n(this->registers.sp); } break;
    case 0x3A: { cyclesUsed = this->ins_LDDI_r1_nn(&this->registers.a, this->registers.get_HL(), &this->registers.h, &this->registers.l, -1); } break;
    case 0x3B: { cyclesUsed = this->ins_DEC_nn(nullptr, nullptr, &this->registers.sp); } break;
    case 0x3C: { cyclesUsed = this->ins_INC_n(&this->registers.a); } break;
    case 0x3D: { cyclesUsed = this->ins_DEC_n(&this->registers.a); } break;
    case 0x3E: { cyclesUsed = this->ins_LD_nn_n(&this->registers.a, this->get_byte_from_pc()); } break;
    case 0x3F: { cyclesUsed = this->ins_CCF(); } break;

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

    case 0x70: { cyclesUsed = this->ins_LD_r1_r2(nullptr, this->registers.get_HL(), &this->registers.b); } break;
    case 0x71: { cyclesUsed = this->ins_LD_r1_r2(nullptr, this->registers.get_HL(), &this->registers.c); } break;
    case 0x72: { cyclesUsed = this->ins_LD_r1_r2(nullptr, this->registers.get_HL(), &this->registers.d); } break;
    case 0x73: { cyclesUsed = this->ins_LD_r1_r2(nullptr, this->registers.get_HL(), &this->registers.e); } break;
    case 0x74: { cyclesUsed = this->ins_LD_r1_r2(nullptr, this->registers.get_HL(), &this->registers.h); } break;
    case 0x75: { cyclesUsed = this->ins_LD_r1_r2(nullptr, this->registers.get_HL(), &this->registers.l); } break;
    case 0x76: { this->is_halted = true; return 0; } break; //HALT
    case 0x77: { cyclesUsed = this->ins_LD_r1_r2(nullptr, this->registers.get_HL(), &this->registers.a); } break;
    
    case 0x78: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, NULL, &this->registers.b); } break;
    case 0x79: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, NULL, &this->registers.c); } break;
    case 0x7A: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, NULL, &this->registers.d); } break;
    case 0x7B: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, NULL, &this->registers.e); } break;
    case 0x7C: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, NULL, &this->registers.h); } break;
    case 0x7D: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, NULL, &this->registers.l); } break;
    case 0x7E: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, this->registers.get_HL()); } break;
    case 0x7F: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, NULL, &this->registers.a); } break;

    case 0x80: { cyclesUsed = this->ins_ADD_A_n(&this->registers.b); } break;
    case 0x81: { cyclesUsed = this->ins_ADD_A_n(&this->registers.c); } break;
    case 0x82: { cyclesUsed = this->ins_ADD_A_n(&this->registers.d); } break;
    case 0x83: { cyclesUsed = this->ins_ADD_A_n(&this->registers.e); } break;
    case 0x84: { cyclesUsed = this->ins_ADD_A_n(&this->registers.h); } break;
    case 0x85: { cyclesUsed = this->ins_ADD_A_n(&this->registers.l); } break;
    case 0x86: { cyclesUsed = this->ins_ADD_A_n(nullptr, this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu)); } break;
    case 0x87: { cyclesUsed = this->ins_ADD_A_n(&this->registers.a); } break;
   
    case 0x88: { cyclesUsed = this->ins_ADC_A_n(&this->registers.b); } break;
    case 0x89: { cyclesUsed = this->ins_ADC_A_n(&this->registers.c); } break;
    case 0x8A: { cyclesUsed = this->ins_ADC_A_n(&this->registers.d); } break;
    case 0x8B: { cyclesUsed = this->ins_ADC_A_n(&this->registers.e); } break;
    case 0x8C: { cyclesUsed = this->ins_ADC_A_n(&this->registers.h); } break;
    case 0x8D: { cyclesUsed = this->ins_ADC_A_n(&this->registers.l); } break;
    case 0x8E: { cyclesUsed = this->ins_ADC_A_n(nullptr, this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu)); } break;
    case 0x8F: { cyclesUsed = this->ins_ADC_A_n(&this->registers.a); } break;

    case 0x90: { cyclesUsed = this->ins_SUB_n(&this->registers.b); } break;
    case 0x91: { cyclesUsed = this->ins_SUB_n(&this->registers.c); } break;
    case 0x92: { cyclesUsed = this->ins_SUB_n(&this->registers.d); } break;
    case 0x93: { cyclesUsed = this->ins_SUB_n(&this->registers.e); } break;
    case 0x94: { cyclesUsed = this->ins_SUB_n(&this->registers.h); } break;
    case 0x95: { cyclesUsed = this->ins_SUB_n(&this->registers.l); } break;
    case 0x96: { cyclesUsed = this->ins_SUB_n(nullptr, this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu)); } break;
    case 0x97: { cyclesUsed = this->ins_SUB_n(&this->registers.a); } break;
    
    case 0x98: { cyclesUsed = this->ins_SBC_A_n(&this->registers.b); } break;
    case 0x99: { cyclesUsed = this->ins_SBC_A_n(&this->registers.c); } break;
    case 0x9A: { cyclesUsed = this->ins_SBC_A_n(&this->registers.d); } break;
    case 0x9B: { cyclesUsed = this->ins_SBC_A_n(&this->registers.e); } break;
    case 0x9C: { cyclesUsed = this->ins_SBC_A_n(&this->registers.h); } break;
    case 0x9D: { cyclesUsed = this->ins_SBC_A_n(&this->registers.l); } break;
    case 0x9E: { cyclesUsed = this->ins_SBC_A_n(nullptr, this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu)); } break;
    case 0x9F: { cyclesUsed = this->ins_SBC_A_n(&this->registers.a); } break;

    case 0xA0: { cyclesUsed = this->ins_AND_n(&this->registers.b); } break;
    case 0xA1: { cyclesUsed = this->ins_AND_n(&this->registers.c); } break;
    case 0xA2: { cyclesUsed = this->ins_AND_n(&this->registers.d); } break;
    case 0xA3: { cyclesUsed = this->ins_AND_n(&this->registers.e); } break;
    case 0xA4: { cyclesUsed = this->ins_AND_n(&this->registers.h); } break;
    case 0xA5: { cyclesUsed = this->ins_AND_n(&this->registers.l); } break;
    case 0xA6: { cyclesUsed = this->ins_AND_n(nullptr, this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu)); } break;
    case 0xA7: { cyclesUsed = this->ins_AND_n(&this->registers.a); } break;
    
    case 0xA8: { cyclesUsed = this->ins_XOR_n(&this->registers.b); } break;
    case 0xA9: { cyclesUsed = this->ins_XOR_n(&this->registers.c); } break;
    case 0xAA: { cyclesUsed = this->ins_XOR_n(&this->registers.d); } break;
    case 0xAB: { cyclesUsed = this->ins_XOR_n(&this->registers.e); } break;
    case 0xAC: { cyclesUsed = this->ins_XOR_n(&this->registers.h); } break;
    case 0xAD: { cyclesUsed = this->ins_XOR_n(&this->registers.l); } break;
    case 0xAE: { cyclesUsed = this->ins_XOR_n(nullptr, this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu)); } break;
    case 0xAF: { cyclesUsed = this->ins_XOR_n(&this->registers.a); } break;

    case 0xB0: { cyclesUsed = this->ins_OR_n(&this->registers.b); } break;
    case 0xB1: { cyclesUsed = this->ins_OR_n(&this->registers.c); } break;
    case 0xB2: { cyclesUsed = this->ins_OR_n(&this->registers.d); } break;
    case 0xB3: { cyclesUsed = this->ins_OR_n(&this->registers.e); } break;
    case 0xB4: { cyclesUsed = this->ins_OR_n(&this->registers.h); } break;
    case 0xB5: { cyclesUsed = this->ins_OR_n(&this->registers.l); } break;
    case 0xB6: { cyclesUsed = this->ins_OR_n(nullptr, this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu)); } break;
    case 0xB7: { cyclesUsed = this->ins_OR_n(&this->registers.a); } break;
    
    case 0xB8: { cyclesUsed = this->ins_CP_n(&this->registers.b); } break;
    case 0xB9: { cyclesUsed = this->ins_CP_n(&this->registers.c); } break;
    case 0xBA: { cyclesUsed = this->ins_CP_n(&this->registers.d); } break;
    case 0xBB: { cyclesUsed = this->ins_CP_n(&this->registers.e); } break;
    case 0xBC: { cyclesUsed = this->ins_CP_n(&this->registers.h); } break;
    case 0xBD: { cyclesUsed = this->ins_CP_n(&this->registers.l); } break;
    case 0xBE: { cyclesUsed = this->ins_CP_n(nullptr, this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu)); } break;
    case 0xBF: { cyclesUsed = this->ins_CP_n(&this->registers.a); } break;

    case 0xC0: { cyclesUsed = this->ins_RET_cc(NZ); } break;
    case 0xC1: { cyclesUsed = this->ins_POP_nn(&this->registers.b, &this->registers.c); } break;
    case 0xC2: { cyclesUsed = this->ins_JP_cc_nn(NZ,this->get_word_from_pc_lsbf()); } break;
    case 0xC3: { cyclesUsed = this->ins_JP_nn(this->get_word_from_pc_lsbf()); } break;
    case 0xC4: { cyclesUsed = this->ins_CALL_cc_nn(NZ, this->get_word_from_pc_lsbf()); } break;
    case 0xC5: { cyclesUsed = this->ins_PUSH_nn(this->registers.get_BC()); } break;
    case 0xC6: { cyclesUsed = this->ins_ADD_A_n(nullptr, this->get_byte_from_pc()); } break;
    case 0xC7: { cyclesUsed = this->ins_RST_n(0x00); } break;
    
    case 0xC8: { cyclesUsed = this->ins_RET_cc(Z); } break;
    case 0xC9: { cyclesUsed = this->ins_RET(); } break;
    case 0xCA: { cyclesUsed = this->ins_JP_cc_nn(Z, this->get_word_from_pc_lsbf()); } break;
    case 0xCB: { cyclesUsed = this->CB_instruction_handler(); } break;
    case 0xCC: { cyclesUsed = this->ins_CALL_cc_nn(Z, this->get_word_from_pc_lsbf()); } break;
    case 0xCD: { cyclesUsed = this->ins_CALL_nn(this->get_word_from_pc_lsbf()); } break;
    case 0xCE: { cyclesUsed = this->ins_ADC_A_n(nullptr, this->get_byte_from_pc()); } break;
    case 0xCF: { cyclesUsed = this->ins_RST_n(0x08); } break;

    case 0xD0: { cyclesUsed = this->ins_RET_cc(NC); } break;
    case 0xD1: { cyclesUsed = this->ins_POP_nn(&this->registers.d, &this->registers.e); } break;
    case 0xD2: { cyclesUsed = this->ins_JP_cc_nn(NC, this->get_word_from_pc_lsbf()); } break;
    //case 0xD3: { cyclesUsed = this->ins_SBC_A_n(nullptr, this->get_byte_from_pc()); } break;
    case 0xD4: { cyclesUsed = this->ins_CALL_cc_nn(NC, this->get_word_from_pc_lsbf()); } break;
    case 0xD5: { cyclesUsed = this->ins_PUSH_nn(this->registers.get_DE()); } break;
    case 0xD6: { cyclesUsed = this->ins_SUB_n(nullptr, this->get_byte_from_pc());  } break;
    case 0xD7: { cyclesUsed = this->ins_RST_n(0x10); } break;
    
    case 0xD8: { cyclesUsed = this->ins_RET_cc(C); } break; //35
    case 0xD9: { cyclesUsed = this->ins_RETI(); } break;
    case 0xDA: { cyclesUsed = this->ins_JP_cc_nn(C, this->get_word_from_pc_lsbf()); } break;
    //case 0xDB: { } break;
    case 0xDC: { cyclesUsed = this->ins_CALL_cc_nn(C, this->get_word_from_pc_lsbf()); } break;
    //case 0xDD: { } break;
    case 0xDE: { cyclesUsed = this->ins_SBC_A_n(nullptr, this->get_byte_from_pc()); } break;
    case 0xDF: { cyclesUsed = this->ins_RST_n(0x18); } break;

    case 0xE0: { cyclesUsed = this->ins_LD_r1_r2(nullptr, 0xFF00 + this->get_byte_from_pc(), nullptr, this->registers.a); } break;
    case 0xE1: { cyclesUsed = this->ins_POP_nn(&this->registers.h, &this->registers.l); } break;
    case 0xE2: { cyclesUsed = this->ins_LD_r1_r2(nullptr, 0xFF00 + this->registers.c, &this->registers.a); } break;
    //case 0xE3: { } break;
    //case 0xE4: { } break;
    case 0xE5: { cyclesUsed = this->ins_PUSH_nn(this->registers.get_HL()); } break;
    case 0xE6: { cyclesUsed = this->ins_AND_n(nullptr, this->get_byte_from_pc()); } break;
    case 0xE7: { cyclesUsed = this->ins_RST_n(0x20); } break;
   
    case 0xE8: { cyclesUsed = this->ins_ADD_SP_n((Byte_s)this->get_byte_from_pc()); } break;
    case 0xE9: { cyclesUsed = this->ins_JP_HL(); } break;
    case 0xEA: { cyclesUsed = this->ins_LD_nn_r1(this->get_word_from_pc_lsbf(), &this->registers.a); } break;
    //case 0xEB: { } break;
    //case 0xEC: { } break;
    //case 0xED: { } break;
    case 0xEE: { cyclesUsed = this->ins_XOR_n(nullptr, this->get_byte_from_pc()); } break;
    case 0xEF: { cyclesUsed = this->ins_RST_n(0x28); } break;

    case 0xF0: { cyclesUsed = this->ins_LD_r1_nn(&this->registers.a, 0xFF00 + this->get_byte_from_pc(), 12); } break;
    case 0xF1: { cyclesUsed = this->ins_POP_nn(&this->registers.a, &this->registers.f); } break;
    case 0xF2: { cyclesUsed = this->ins_LD_r1_r2(&this->registers.a, 0xFF00 + this->registers.c); } break;
    case 0xF3: { this->DI_triggered = true; return 4; } break; //DI is set to trigger, finish instruction
    //case 0xF4: { } break;
    case 0xF5: { cyclesUsed = this->ins_PUSH_nn(this->registers.get_AF()); } break;
    case 0xF6: { cyclesUsed = this->ins_OR_n(nullptr, this->get_byte_from_pc()); } break;
    case 0xF7: { cyclesUsed = this->ins_RST_n(0x30); } break;
    
    case 0xF8: { cyclesUsed = this->ins_LDHL_SP_n(&this->registers.h, &this->registers.l, this->registers.sp, this->get_byte_from_pc()); } break;
    case 0xF9: { cyclesUsed = this->ins_LD_nn_nn(&this->registers.sp, this->registers.get_HL()); } break;
    case 0xFA: { cyclesUsed = this->ins_LD_r1_nn(&this->registers.a, this->get_word_from_pc_lsbf(), 16); } break;
    case 0xFB: { cyclesUsed = this->EI_triggered = true; return 4; } break; // EI is set to trigger, finish instruction
    //case 0xFC: { } break;
    //case 0xFD: { } break;
    case 0xFE: { cyclesUsed = this->ins_CP_n(nullptr, this->get_byte_from_pc()); } break;
    case 0xFF: { cyclesUsed = this->ins_RST_n(0x38); } break;
    default:
    {
        printf("ILLEGAL OPCODE CALL %0.2X \n", opcode);
        cyclesUsed = 4;
    }
    }

    this->interrupt_DI_EI_handler();

#define DEBUG 0
#if DEBUG 1
//#define BREAKPOINTPC 0xC241
#define BREAKPOINTPC 0xDEF8
#define BREAKPOINTDE 0xC242

    // the mission, get past 239

    //0x020f frame 1, tick 61172

    //this->DEBUG_printCurrentState();
    auto c242 = this->bus->work_ram[0x0242];
    auto pc = this->registers.pc;
    auto af = this->registers.get_AF();
    auto bc = this->registers.get_BC();
    auto de = this->registers.get_DE();
    auto hl = this->registers.get_HL();
    auto work_ram_ptr = this->bus->work_ram.get() + 0x0242;

    //if (this->bus->DEBUG_PC_breakpoint_hit)
    //{
    //    /*this->DEBUG_print_IO();*/
    //    this->DEBUG_printCurrentState();
    //}
    if (this->registers.pc == BREAKPOINTPC)
    {
         this->registers.pc = BREAKPOINTPC;
        this->bus->DEBUG_PC_breakpoint_hit = true;

        //TILE tile0(this->bus, this->bus->ppu.get_tile_address_from_number(25, PPU::background));
        //tile0.consolePrint();
        
    }
    if (this->bus->DEBUG_PC_breakpoint_hit)
    {
        /*this->DEBUG_print_IO();*/
        this->DEBUG_printCurrentState();
    }
    this->bus->DEBUG_PC_breakpoint_hit = false;

    if (this->registers.pc == BREAKPOINTPC && this->bus->get_memory(this->registers.pc, MEMORY_ACCESS_TYPE::cpu) == 0xff)
    {
        this->registers.pc = BREAKPOINTPC;
        //TILE tile0(this->bus, this->bus->ppu.get_tile_address_from_number(25, PPU::background));
        //tile0.consolePrint();

    }


    if (this->registers.get_DE() == BREAKPOINTDE && this->registers.pc == BREAKPOINTPC)
    {
        this->registers.get_DE();

    }

   /* if (this->bus->video_ram[0x8010 - 0x8000] != 0){
        int i = 0;
        TILE tile0(this->bus, 0x8010 + (16 * i));
        tile0.consolePrint();
        std::cout << "\n";
    }*/

#endif // DEBUG

    
    return cyclesUsed;
}

int CPU::CB_instruction_handler()
{
    Byte opcode = this->get_byte_from_pc();
    int cyclesUsed = 0;

    
    

    switch (opcode)
    {
    case 0x00: { cyclesUsed = this->ins_RLC(&this->registers.b); } break;
    case 0x01: { cyclesUsed = this->ins_RLC(&this->registers.c); } break;
    case 0x02: { cyclesUsed = this->ins_RLC(&this->registers.d); } break;
    case 0x03: { cyclesUsed = this->ins_RLC(&this->registers.e); } break;
    case 0x04: { cyclesUsed = this->ins_RLC(&this->registers.h); } break;
    case 0x05: { cyclesUsed = this->ins_RLC(&this->registers.l); } break;
    case 0x06: { cyclesUsed = this->ins_RLC(nullptr, this->registers.get_HL()); } break;
    case 0x07: { cyclesUsed = this->ins_RLC(&this->registers.a); } break;

    case 0x08: { cyclesUsed = this->ins_RRC(&this->registers.b); } break;
    case 0x09: { cyclesUsed = this->ins_RRC(&this->registers.c); } break;
    case 0x0A: { cyclesUsed = this->ins_RRC(&this->registers.d); } break;
    case 0x0B: { cyclesUsed = this->ins_RRC(&this->registers.e); } break;
    case 0x0C: { cyclesUsed = this->ins_RRC(&this->registers.h); } break;
    case 0x0D: { cyclesUsed = this->ins_RRC(&this->registers.l); } break;
    case 0x0E: { cyclesUsed = this->ins_RRC(nullptr, this->registers.get_HL()); } break;
    case 0x0F: { cyclesUsed = this->ins_RRC(&this->registers.a); } break;
    
    case 0x10: { cyclesUsed = this->ins_RL(&this->registers.b); } break;
    case 0x11: { cyclesUsed = this->ins_RL(&this->registers.c); } break;
    case 0x12: { cyclesUsed = this->ins_RL(&this->registers.d); } break;
    case 0x13: { cyclesUsed = this->ins_RL(&this->registers.e); } break;
    case 0x14: { cyclesUsed = this->ins_RL(&this->registers.h); } break;
    case 0x15: { cyclesUsed = this->ins_RL(&this->registers.l); } break;
    case 0x16: { cyclesUsed = this->ins_RL(nullptr, this->registers.get_HL()); } break;
    case 0x17: { cyclesUsed = this->ins_RL(&this->registers.a); } break;
        
    case 0x18: { cyclesUsed = this->ins_RR(&this->registers.b); } break;
    case 0x19: { cyclesUsed = this->ins_RR(&this->registers.c); } break;
    case 0x1A: { cyclesUsed = this->ins_RR(&this->registers.d); } break;
    case 0x1B: { cyclesUsed = this->ins_RR(&this->registers.e); } break;
    case 0x1C: { cyclesUsed = this->ins_RR(&this->registers.h); } break;
    case 0x1D: { cyclesUsed = this->ins_RR(&this->registers.l); } break;
    case 0x1E: { cyclesUsed = this->ins_RR(nullptr, this->registers.get_HL()); } break;
    case 0x1F: { cyclesUsed = this->ins_RR(&this->registers.a); } break;

    case 0x20: { cyclesUsed = this->ins_SLA(&this->registers.b); } break;
    case 0x21: { cyclesUsed = this->ins_SLA(&this->registers.c); } break;
    case 0x22: { cyclesUsed = this->ins_SLA(&this->registers.d); } break;
    case 0x23: { cyclesUsed = this->ins_SLA(&this->registers.e); } break;
    case 0x24: { cyclesUsed = this->ins_SLA(&this->registers.h); } break;
    case 0x25: { cyclesUsed = this->ins_SLA(&this->registers.l); } break;
    case 0x26: { cyclesUsed = this->ins_SLA(nullptr, this->registers.get_HL()); } break;
    case 0x27: { cyclesUsed = this->ins_SLA(&this->registers.a); } break;

    case 0x28: { cyclesUsed = this->ins_SRA_n(&this->registers.b); } break;
    case 0x29: { cyclesUsed = this->ins_SRA_n(&this->registers.c); } break;
    case 0x2A: { cyclesUsed = this->ins_SRA_n(&this->registers.d); } break;
    case 0x2B: { cyclesUsed = this->ins_SRA_n(&this->registers.e); } break;
    case 0x2C: { cyclesUsed = this->ins_SRA_n(&this->registers.h); } break;
    case 0x2D: { cyclesUsed = this->ins_SRA_n(&this->registers.l); } break;
    case 0x2E: { cyclesUsed = this->ins_SRA_n(nullptr, this->registers.get_HL()); } break;
    case 0x2F: { cyclesUsed = this->ins_SRA_n(&this->registers.a); } break;
    
    case 0x30: { cyclesUsed = this->ins_SWAP_nn(&this->registers.b); } break;
    case 0x31: { cyclesUsed = this->ins_SWAP_nn(&this->registers.c); } break;
    case 0x32: { cyclesUsed = this->ins_SWAP_nn(&this->registers.d); } break;
    case 0x33: { cyclesUsed = this->ins_SWAP_nn(&this->registers.e); } break;
    case 0x34: { cyclesUsed = this->ins_SWAP_nn(&this->registers.h); } break;
    case 0x35: { cyclesUsed = this->ins_SWAP_nn(&this->registers.l); } break;
    case 0x36: { cyclesUsed = this->ins_SWAP_nn(nullptr, this->registers.get_HL()); } break;
    case 0x37: { cyclesUsed = this->ins_SWAP_nn(&this->registers.a); } break;

    case 0x38: { cyclesUsed = this->ins_SRL_n(&this->registers.b); } break;
    case 0x39: { cyclesUsed = this->ins_SRL_n(&this->registers.c); } break;
    case 0x3A: { cyclesUsed = this->ins_SRL_n(&this->registers.d); } break;
    case 0x3B: { cyclesUsed = this->ins_SRL_n(&this->registers.e); } break;
    case 0x3C: { cyclesUsed = this->ins_SRL_n(&this->registers.h); } break;
    case 0x3D: { cyclesUsed = this->ins_SRL_n(&this->registers.l); } break;
    case 0x3E: { cyclesUsed = this->ins_SRL_n(nullptr, this->registers.get_HL()); } break;
    case 0x3F: { cyclesUsed = this->ins_SRL_n(&this->registers.a); } break;

    case 0x40: { cyclesUsed = this->ins_BIT_b_r(0, &this->registers.b); } break;
    case 0x41: { cyclesUsed = this->ins_BIT_b_r(0, &this->registers.c); } break;
    case 0x42: { cyclesUsed = this->ins_BIT_b_r(0, &this->registers.d); } break;
    case 0x43: { cyclesUsed = this->ins_BIT_b_r(0, &this->registers.e); } break;
    case 0x44: { cyclesUsed = this->ins_BIT_b_r(0, &this->registers.h); } break;
    case 0x45: { cyclesUsed = this->ins_BIT_b_r(0, &this->registers.l); } break;
    case 0x46: { cyclesUsed = this->ins_BIT_b_r(0, nullptr, this->registers.get_HL()); } break;
    case 0x47: { cyclesUsed = this->ins_BIT_b_r(0, &this->registers.a); } break;
    
    case 0x48: { cyclesUsed = this->ins_BIT_b_r(1, &this->registers.b); } break;
    case 0x49: { cyclesUsed = this->ins_BIT_b_r(1, &this->registers.c); } break;
    case 0x4A: { cyclesUsed = this->ins_BIT_b_r(1, &this->registers.d); } break;
    case 0x4B: { cyclesUsed = this->ins_BIT_b_r(1, &this->registers.e); } break;
    case 0x4C: { cyclesUsed = this->ins_BIT_b_r(1, &this->registers.h); } break;
    case 0x4D: { cyclesUsed = this->ins_BIT_b_r(1, &this->registers.l); } break;
    case 0x4E: { cyclesUsed = this->ins_BIT_b_r(1, nullptr, this->registers.get_HL()); } break;
    case 0x4F: { cyclesUsed = this->ins_BIT_b_r(1, &this->registers.a); } break;
    
    case 0x50: { cyclesUsed = this->ins_BIT_b_r(2, &this->registers.b); } break;
    case 0x51: { cyclesUsed = this->ins_BIT_b_r(2, &this->registers.c); } break;
    case 0x52: { cyclesUsed = this->ins_BIT_b_r(2, &this->registers.d); } break;
    case 0x53: { cyclesUsed = this->ins_BIT_b_r(2, &this->registers.e); } break;
    case 0x54: { cyclesUsed = this->ins_BIT_b_r(2, &this->registers.h); } break;
    case 0x55: { cyclesUsed = this->ins_BIT_b_r(2, &this->registers.l); } break;
    case 0x56: { cyclesUsed = this->ins_BIT_b_r(2, nullptr, this->registers.get_HL()); } break;
    case 0x57: { cyclesUsed = this->ins_BIT_b_r(2, &this->registers.a); } break;
    
    case 0x58: { cyclesUsed = this->ins_BIT_b_r(3, &this->registers.b); } break;
    case 0x59: { cyclesUsed = this->ins_BIT_b_r(3, &this->registers.c); } break;
    case 0x5A: { cyclesUsed = this->ins_BIT_b_r(3, &this->registers.d); } break;
    case 0x5B: { cyclesUsed = this->ins_BIT_b_r(3, &this->registers.e); } break;
    case 0x5C: { cyclesUsed = this->ins_BIT_b_r(3, &this->registers.h); } break;
    case 0x5D: { cyclesUsed = this->ins_BIT_b_r(3, &this->registers.l); } break;
    case 0x5E: { cyclesUsed = this->ins_BIT_b_r(3, nullptr, this->registers.get_HL()); } break;
    case 0x5F: { cyclesUsed = this->ins_BIT_b_r(3, &this->registers.a); } break;

    case 0x60: { cyclesUsed = this->ins_BIT_b_r(4, &this->registers.b); } break;
    case 0x61: { cyclesUsed = this->ins_BIT_b_r(4, &this->registers.c); } break;
    case 0x62: { cyclesUsed = this->ins_BIT_b_r(4, &this->registers.d); } break;
    case 0x63: { cyclesUsed = this->ins_BIT_b_r(4, &this->registers.e); } break;
    case 0x64: { cyclesUsed = this->ins_BIT_b_r(4, &this->registers.h); } break;
    case 0x65: { cyclesUsed = this->ins_BIT_b_r(4, &this->registers.l); } break;
    case 0x66: { cyclesUsed = this->ins_BIT_b_r(4, nullptr, this->registers.get_HL()); } break;
    case 0x67: { cyclesUsed = this->ins_BIT_b_r(4, &this->registers.a); } break;

    case 0x68: { cyclesUsed = this->ins_BIT_b_r(5, &this->registers.b); } break;
    case 0x69: { cyclesUsed = this->ins_BIT_b_r(5, &this->registers.c); } break;
    case 0x6A: { cyclesUsed = this->ins_BIT_b_r(5, &this->registers.d); } break;
    case 0x6B: { cyclesUsed = this->ins_BIT_b_r(5, &this->registers.e); } break;
    case 0x6C: { cyclesUsed = this->ins_BIT_b_r(5, &this->registers.h); } break;
    case 0x6D: { cyclesUsed = this->ins_BIT_b_r(5, &this->registers.l); } break;
    case 0x6E: { cyclesUsed = this->ins_BIT_b_r(5, nullptr, this->registers.get_HL()); } break;
    case 0x6F: { cyclesUsed = this->ins_BIT_b_r(5, &this->registers.a); } break;

    case 0x70: { cyclesUsed = this->ins_BIT_b_r(6, &this->registers.b); } break;
    case 0x71: { cyclesUsed = this->ins_BIT_b_r(6, &this->registers.c); } break;
    case 0x72: { cyclesUsed = this->ins_BIT_b_r(6, &this->registers.d); } break;
    case 0x73: { cyclesUsed = this->ins_BIT_b_r(6, &this->registers.e); } break;
    case 0x74: { cyclesUsed = this->ins_BIT_b_r(6, &this->registers.h); } break;
    case 0x75: { cyclesUsed = this->ins_BIT_b_r(6, &this->registers.l); } break;
    case 0x76: { cyclesUsed = this->ins_BIT_b_r(6, nullptr, this->registers.get_HL()); } break;
    case 0x77: { cyclesUsed = this->ins_BIT_b_r(6, &this->registers.a); } break;

    case 0x78: { cyclesUsed = this->ins_BIT_b_r(7, &this->registers.b); } break;
    case 0x79: { cyclesUsed = this->ins_BIT_b_r(7, &this->registers.c); } break;
    case 0x7A: { cyclesUsed = this->ins_BIT_b_r(7, &this->registers.d); } break;
    case 0x7B: { cyclesUsed = this->ins_BIT_b_r(7, &this->registers.e); } break;
    case 0x7C: { cyclesUsed = this->ins_BIT_b_r(7, &this->registers.h); } break;
    case 0x7D: { cyclesUsed = this->ins_BIT_b_r(7, &this->registers.l); } break;
    case 0x7E: { cyclesUsed = this->ins_BIT_b_r(7, nullptr, this->registers.get_HL()); } break;
    case 0x7F: { cyclesUsed = this->ins_BIT_b_r(7, &this->registers.a); } break;

    case 0x80: { cyclesUsed = this->ins_RES_b_r(0, &this->registers.b); } break;
    case 0x81: { cyclesUsed = this->ins_RES_b_r(0, &this->registers.c); } break;
    case 0x82: { cyclesUsed = this->ins_RES_b_r(0, &this->registers.d); } break;
    case 0x83: { cyclesUsed = this->ins_RES_b_r(0, &this->registers.e); } break;
    case 0x84: { cyclesUsed = this->ins_RES_b_r(0, &this->registers.h); } break;
    case 0x85: { cyclesUsed = this->ins_RES_b_r(0, &this->registers.l); } break;
    case 0x86: { cyclesUsed = this->ins_RES_b_r(0, nullptr, this->registers.get_HL()); } break;
    case 0x87: { cyclesUsed = this->ins_RES_b_r(0, &this->registers.a); } break;

    case 0x88: { cyclesUsed = this->ins_RES_b_r(1, &this->registers.b); } break;
    case 0x89: { cyclesUsed = this->ins_RES_b_r(1, &this->registers.c); } break;
    case 0x8A: { cyclesUsed = this->ins_RES_b_r(1, &this->registers.d); } break;
    case 0x8B: { cyclesUsed = this->ins_RES_b_r(1, &this->registers.e); } break;
    case 0x8C: { cyclesUsed = this->ins_RES_b_r(1, &this->registers.h); } break;
    case 0x8D: { cyclesUsed = this->ins_RES_b_r(1, &this->registers.l); } break;
    case 0x8E: { cyclesUsed = this->ins_RES_b_r(1, nullptr, this->registers.get_HL()); } break;
    case 0x8F: { cyclesUsed = this->ins_RES_b_r(1, &this->registers.a); } break;

    case 0x90: { cyclesUsed = this->ins_RES_b_r(2, &this->registers.b); } break;
    case 0x91: { cyclesUsed = this->ins_RES_b_r(2, &this->registers.c); } break;
    case 0x92: { cyclesUsed = this->ins_RES_b_r(2, &this->registers.d); } break;
    case 0x93: { cyclesUsed = this->ins_RES_b_r(2, &this->registers.e); } break;
    case 0x94: { cyclesUsed = this->ins_RES_b_r(2, &this->registers.h); } break;
    case 0x95: { cyclesUsed = this->ins_RES_b_r(2, &this->registers.l); } break;
    case 0x96: { cyclesUsed = this->ins_RES_b_r(2, nullptr, this->registers.get_HL()); } break;
    case 0x97: { cyclesUsed = this->ins_RES_b_r(2, &this->registers.a); } break;

    case 0x98: { cyclesUsed = this->ins_RES_b_r(3, &this->registers.b); } break;
    case 0x99: { cyclesUsed = this->ins_RES_b_r(3, &this->registers.c); } break;
    case 0x9A: { cyclesUsed = this->ins_RES_b_r(3, &this->registers.d); } break;
    case 0x9B: { cyclesUsed = this->ins_RES_b_r(3, &this->registers.e); } break;
    case 0x9C: { cyclesUsed = this->ins_RES_b_r(3, &this->registers.h); } break;
    case 0x9D: { cyclesUsed = this->ins_RES_b_r(3, &this->registers.l); } break;
    case 0x9E: { cyclesUsed = this->ins_RES_b_r(3, nullptr, this->registers.get_HL()); } break;
    case 0x9F: { cyclesUsed = this->ins_RES_b_r(3, &this->registers.a); } break;

    case 0xA0: { cyclesUsed = this->ins_RES_b_r(4, &this->registers.b); } break;
    case 0xA1: { cyclesUsed = this->ins_RES_b_r(4, &this->registers.c); } break;
    case 0xA2: { cyclesUsed = this->ins_RES_b_r(4, &this->registers.d); } break;
    case 0xA3: { cyclesUsed = this->ins_RES_b_r(4, &this->registers.e); } break;
    case 0xA4: { cyclesUsed = this->ins_RES_b_r(4, &this->registers.h); } break;
    case 0xA5: { cyclesUsed = this->ins_RES_b_r(4, &this->registers.l); } break;
    case 0xA6: { cyclesUsed = this->ins_RES_b_r(4, nullptr, this->registers.get_HL()); } break;
    case 0xA7: { cyclesUsed = this->ins_RES_b_r(4, &this->registers.a); } break;

    case 0xA8: { cyclesUsed = this->ins_RES_b_r(5, &this->registers.b); } break;
    case 0xA9: { cyclesUsed = this->ins_RES_b_r(5, &this->registers.c); } break;
    case 0xAA: { cyclesUsed = this->ins_RES_b_r(5, &this->registers.d); } break;
    case 0xAB: { cyclesUsed = this->ins_RES_b_r(5, &this->registers.e); } break;
    case 0xAC: { cyclesUsed = this->ins_RES_b_r(5, &this->registers.h); } break;
    case 0xAD: { cyclesUsed = this->ins_RES_b_r(5, &this->registers.l); } break;
    case 0xAE: { cyclesUsed = this->ins_RES_b_r(5, nullptr, this->registers.get_HL()); } break;
    case 0xAF: { cyclesUsed = this->ins_RES_b_r(5, &this->registers.a); } break;

    case 0xB0: { cyclesUsed = this->ins_RES_b_r(6, &this->registers.b); } break;
    case 0xB1: { cyclesUsed = this->ins_RES_b_r(6, &this->registers.c); } break;
    case 0xB2: { cyclesUsed = this->ins_RES_b_r(6, &this->registers.d); } break;
    case 0xB3: { cyclesUsed = this->ins_RES_b_r(6, &this->registers.e); } break;
    case 0xB4: { cyclesUsed = this->ins_RES_b_r(6, &this->registers.h); } break;
    case 0xB5: { cyclesUsed = this->ins_RES_b_r(6, &this->registers.l); } break;
    case 0xB6: { cyclesUsed = this->ins_RES_b_r(6, nullptr, this->registers.get_HL()); } break;
    case 0xB7: { cyclesUsed = this->ins_RES_b_r(6, &this->registers.a); } break;

    case 0xB8: { cyclesUsed = this->ins_RES_b_r(7, &this->registers.b); } break;
    case 0xB9: { cyclesUsed = this->ins_RES_b_r(7, &this->registers.c); } break;
    case 0xBA: { cyclesUsed = this->ins_RES_b_r(7, &this->registers.d); } break;
    case 0xBB: { cyclesUsed = this->ins_RES_b_r(7, &this->registers.e); } break;
    case 0xBC: { cyclesUsed = this->ins_RES_b_r(7, &this->registers.h); } break;
    case 0xBD: { cyclesUsed = this->ins_RES_b_r(7, &this->registers.l); } break;
    case 0xBE: { cyclesUsed = this->ins_RES_b_r(7, nullptr, this->registers.get_HL()); } break;
    case 0xBF: { cyclesUsed = this->ins_RES_b_r(7, &this->registers.a); } break;

    case 0xC0: { cyclesUsed = this->ins_SET_b_r(0, &this->registers.b); } break;
    case 0xC1: { cyclesUsed = this->ins_SET_b_r(0, &this->registers.c); } break;
    case 0xC2: { cyclesUsed = this->ins_SET_b_r(0, &this->registers.d); } break;
    case 0xC3: { cyclesUsed = this->ins_SET_b_r(0, &this->registers.e); } break;
    case 0xC4: { cyclesUsed = this->ins_SET_b_r(0, &this->registers.h); } break;
    case 0xC5: { cyclesUsed = this->ins_SET_b_r(0, &this->registers.l); } break;
    case 0xC6: { cyclesUsed = this->ins_SET_b_r(0, nullptr, this->registers.get_HL()); } break;
    case 0xC7: { cyclesUsed = this->ins_SET_b_r(0, &this->registers.a); } break;
    
    case 0xC8: { cyclesUsed = this->ins_SET_b_r(1, &this->registers.b); } break;
    case 0xC9: { cyclesUsed = this->ins_SET_b_r(1, &this->registers.c); } break;
    case 0xCA: { cyclesUsed = this->ins_SET_b_r(1, &this->registers.d); } break;
    case 0xCB: { cyclesUsed = this->ins_SET_b_r(1, &this->registers.e); } break;
    case 0xCC: { cyclesUsed = this->ins_SET_b_r(1, &this->registers.h); } break;
    case 0xCD: { cyclesUsed = this->ins_SET_b_r(1, &this->registers.l); } break;
    case 0xCE: { cyclesUsed = this->ins_SET_b_r(1, nullptr, this->registers.get_HL()); } break;
    case 0xCF: { cyclesUsed = this->ins_SET_b_r(1, &this->registers.a); } break;
    
    case 0xD0: { cyclesUsed = this->ins_SET_b_r(2, &this->registers.b); } break;
    case 0xD1: { cyclesUsed = this->ins_SET_b_r(2, &this->registers.c); } break;
    case 0xD2: { cyclesUsed = this->ins_SET_b_r(2, &this->registers.d); } break;
    case 0xD3: { cyclesUsed = this->ins_SET_b_r(2, &this->registers.e); } break;
    case 0xD4: { cyclesUsed = this->ins_SET_b_r(2, &this->registers.h); } break;
    case 0xD5: { cyclesUsed = this->ins_SET_b_r(2, &this->registers.l); } break;
    case 0xD6: { cyclesUsed = this->ins_SET_b_r(2, nullptr, this->registers.get_HL()); } break;
    case 0xD7: { cyclesUsed = this->ins_SET_b_r(2, &this->registers.a); } break;

    case 0xD8: { cyclesUsed = this->ins_SET_b_r(3, &this->registers.b); } break;
    case 0xD9: { cyclesUsed = this->ins_SET_b_r(3, &this->registers.c); } break;
    case 0xDA: { cyclesUsed = this->ins_SET_b_r(3, &this->registers.d); } break;
    case 0xDB: { cyclesUsed = this->ins_SET_b_r(3, &this->registers.e); } break;
    case 0xDC: { cyclesUsed = this->ins_SET_b_r(3, &this->registers.h); } break;
    case 0xDD: { cyclesUsed = this->ins_SET_b_r(3, &this->registers.l); } break;
    case 0xDE: { cyclesUsed = this->ins_SET_b_r(3, nullptr, this->registers.get_HL()); } break;
    case 0xDF: { cyclesUsed = this->ins_SET_b_r(3, &this->registers.a); } break;

    case 0xE0: { cyclesUsed = this->ins_SET_b_r(4, &this->registers.b); } break;
    case 0xE1: { cyclesUsed = this->ins_SET_b_r(4, &this->registers.c); } break;
    case 0xE2: { cyclesUsed = this->ins_SET_b_r(4, &this->registers.d); } break;
    case 0xE3: { cyclesUsed = this->ins_SET_b_r(4, &this->registers.e); } break;
    case 0xE4: { cyclesUsed = this->ins_SET_b_r(4, &this->registers.h); } break;
    case 0xE5: { cyclesUsed = this->ins_SET_b_r(4, &this->registers.l); } break;
    case 0xE6: { cyclesUsed = this->ins_SET_b_r(4, nullptr, this->registers.get_HL()); } break;
    case 0xE7: { cyclesUsed = this->ins_SET_b_r(4, &this->registers.a); } break;

    case 0xE8: { cyclesUsed = this->ins_SET_b_r(5, &this->registers.b); } break;
    case 0xE9: { cyclesUsed = this->ins_SET_b_r(5, &this->registers.c); } break;
    case 0xEA: { cyclesUsed = this->ins_SET_b_r(5, &this->registers.d); } break;
    case 0xEB: { cyclesUsed = this->ins_SET_b_r(5, &this->registers.e); } break;
    case 0xEC: { cyclesUsed = this->ins_SET_b_r(5, &this->registers.h); } break;
    case 0xED: { cyclesUsed = this->ins_SET_b_r(5, &this->registers.l); } break;
    case 0xEE: { cyclesUsed = this->ins_SET_b_r(5, nullptr, this->registers.get_HL()); } break;
    case 0xEF: { cyclesUsed = this->ins_SET_b_r(5, &this->registers.a); } break;

    case 0xF0: { cyclesUsed = this->ins_SET_b_r(6, &this->registers.b); } break;
    case 0xF1: { cyclesUsed = this->ins_SET_b_r(6, &this->registers.c); } break;
    case 0xF2: { cyclesUsed = this->ins_SET_b_r(6, &this->registers.d); } break;
    case 0xF3: { cyclesUsed = this->ins_SET_b_r(6, &this->registers.e); } break;
    case 0xF4: { cyclesUsed = this->ins_SET_b_r(6, &this->registers.h); } break;
    case 0xF5: { cyclesUsed = this->ins_SET_b_r(6, &this->registers.l); } break;
    case 0xF6: { cyclesUsed = this->ins_SET_b_r(6, nullptr, this->registers.get_HL()); } break;
    case 0xF7: { cyclesUsed = this->ins_SET_b_r(6, &this->registers.a); } break;

    case 0xF8: { cyclesUsed = this->ins_SET_b_r(7, &this->registers.b); } break;
    case 0xF9: { cyclesUsed = this->ins_SET_b_r(7, &this->registers.c); } break;
    case 0xFA: { cyclesUsed = this->ins_SET_b_r(7, &this->registers.d); } break;
    case 0xFB: { cyclesUsed = this->ins_SET_b_r(7, &this->registers.e); } break;
    case 0xFC: { cyclesUsed = this->ins_SET_b_r(7, &this->registers.h); } break;
    case 0xFD: { cyclesUsed = this->ins_SET_b_r(7, &this->registers.l); } break;
    case 0xFE: { cyclesUsed = this->ins_SET_b_r(7, nullptr, this->registers.get_HL()); } break;
    case 0xFF: { cyclesUsed = this->ins_SET_b_r(7, &this->registers.a); } break;
    }

    
    return cyclesUsed;
}

int CPU::STOP_instruction_handler()
{
    Byte opcode = this->get_byte_from_pc();
    int cyclesUsed = 0;

    switch (opcode)
    {
    case 0x00: { cyclesUsed = 4; } // STOP ins but there might be something more important needed to be done here
    }

    return cyclesUsed;
}

