#pragma once

#include <iostream>

#include "config.hpp"


// forward declaration, bus.hpp is not included here but in cpu.cpp instead
class BUS;

enum Flags
{
	z = 0b10000000, // Zero
	n = 0b01000000, // Subtraction
	h = 0b00100000, // Half Carry
	c = 0b00010000  // Carry
};

enum JumpCondition
{
	NZ,
	Z,
	NC,
	C,
	BYPASS
};

enum InterruptTypes
{
	vblank = 0b00000001,
	lcdstat = 0b00000010,
	timer = 0b00000100,
	serial = 0b00001000,
	joypad = 0b00010000,

};
static const InterruptTypes InterruptTypes_all[] = { vblank, lcdstat, timer, serial, joypad };
static const InterruptTypes InterruptTypes_all_prioritised[] = { joypad, serial, timer, lcdstat, vblank };


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

	void set_lowByte(Word* wordRegister, const Byte value) { *wordRegister = (* wordRegister & 0xFF00) | value; };
	void set_highByte(Word* wordRegister, const Byte value) { *wordRegister = (* wordRegister & 0x00FF) | value << 8; };
	Byte get_lowByte(Word* wordRegister) { return (*wordRegister & 0xFF00) >> 8; };
	Byte get_highByte(Word* wordRegister) { return (*wordRegister & 0x00FF); };


	bool get_flag(const Flags flag)
	{
		return this->f & flag;
	}


	void set_flag(const Flags flag, const bool value)
	{
		(value) ? this->f |= flag : this->f &= ~flag;
	}

	const Word get_word(Byte* registerOne, Byte* registerTwo)
	{
		return ((*registerOne << 8) | *registerTwo);
	}

	void set_word(Byte* registerOne, Byte* registerTwo, const Word value)
	{
		*registerOne = ((value & 0xFF00) >> 8);
		*registerTwo = (value & 0xFF);
		if (registerTwo == &this->f)
			*registerTwo = (value & 0xF0);
	}
};


class CPU
{
public:
	bool debug_toggle = false;
	void bios_init();

	CPU();

	Registers registers;
	void DEBUG_print_IO();

	bool interrupt_master_enable = 0;
	bool is_halted = 0;
	bool halt_bug = 0;
	void connect_to_bus(BUS* pBus);
	const Byte get_byte_from_pc();
	void mStepCPU();
	void halt_handler();
	void DEBUG_printCurrentState(Word pc);
	void update_timers();
	void request_interrupt(const InterruptTypes type);


private:
	BUS* bus = nullptr;
	void init();

	Byte get_interrupt_flag(const enum InterruptTypes type, Word address);
	void set_interrupt_flag(const enum InterruptTypes type, const bool value, Word address);
	
	int timerCounter = 0;
	int divtimerCounter = 0;

	Byte get_TMC_frequency();
	int get_TAC_freq();

	Byte currentRunningOpcode = 0;
	Byte currentRunningCB = 0;
	Byte mCyclesUsed = 0;
	bool isExecutingInstruction = false;
	bool isExecutingCB = false;
	Byte instructionCache[5] = { 0 };
	Word interrupt_vector = 0;
	bool EI_triggered = false;

	void instruction_handler();
	void CB_instruction_handler();
	void STOP_instruction_handler();
	void check_for_interrupts();
	void setup_interrupt_handler();
	void setup_for_next_instruction();
	void prefetch_instruction();

	
	Byte get_nibble(const Byte input, const bool getHi);
	void set_nibble(Byte* registerOne, const Byte value, const bool setHi);

	bool checkCarry(const int a, const int b, const int shift, const int c = NULL);
	bool checkBorrow(const int a, const int b, const int shift, const int c = NULL);

	bool checkJumpCondition(enum JumpCondition condition);

	Byte get_memory(const Word address);
	void set_memory(const Word address, const Byte data);


	// instruction declarations

	void ins_LD_XX_u16(Byte* const registerOne, Byte* const registerTwo);
	void ins_LD_SP_u16(Word* const registerWord);
	void ins_LD_bXXb_Y(const Word registerWordvalue, Byte* const registerByte);
	void ins_LD_bHLb_Apm(bool add);
	void ins_INC_XX(Byte* registerOne, Byte* registerTwo);
	void ins_INC_SP();
	void ins_INC_X(Byte* registerByte);
	void ins_INC_bHLb();
	void ins_DEC_X(Byte* registerByte);
	void ins_DEC_bHLb();
	void ins_LD_X_u8(Byte* const registerByte);
	void ins_LD_bHLb_u8();
	void ins_DAA();
	void ins_CPL();
	void ins_CCF();
	void ins_SCF();
	void ins_RLCA();
	void ins_RLA();
	void ins_RRCA();
	void ins_RRA();
	void ins_LD_bu16b_SP();
	void ins_JR_i8(const enum JumpCondition condition = BYPASS);
	void ins_ADD_HL_XX(Byte* const registerOne, Byte* const registerTwo);
	void ins_ADD_HL_SP();
	void ins_ADD_A_X(Byte* const registerByte);
	void ins_ADD_A_bHLb();
	void ins_ADD_A_u8();
	void ins_LD_X_Y(Byte* const registerOne, Byte* const registerTwo);
	void ins_LD_X_bYYb(Byte* const leftRegister, Byte* const rightRegisterOne, Byte* const rightRegisterTwo, const Byte_s addToHL = NULL);
	void ins_DEC_XX(Byte* registerOne, Byte* registerTwo);
	void ins_DEC_SP();
	void ins_ADC_A_X(const Byte* registerByte);
	void ins_ADC_A_bHLb();
	void ins_ADC_A_u8();
	void ins_SUB_A_X(Byte* const registerByte);
	void ins_SUB_A_u8();
	void ins_SUB_A_bHLb();
	void ins_SBC_A_X(Byte* const registerByte);
	void ins_SBC_A_u8();
	void ins_SBC_A_bHLb();
	void ins_AND_A_X(Byte* const registerByte);
	void ins_AND_A_u8();
	void ins_AND_A_bHLb();
	void ins_XOR_A_X(Byte* const registerByte);
	void ins_XOR_A_u8();
	void ins_XOR_A_bHLb();
	void ins_OR_A_X(Byte* const registerByte);
	void ins_OR_A_u8();
	void ins_OR_A_bHLb();
	void ins_CP_A_X(Byte* const registerByte);
	void ins_CP_A_u8();
	void ins_CP_A_bHLb();
	void ins_POP_XX(Byte* registerOne, Byte* registerTwo);
	void ins_PUSH_XX(const Word wordRegisterValue);
	void ins_JP_HL();
	void ins_CALL_u16(const enum JumpCondition condition = BYPASS);
	void ins_JP_u16(const enum JumpCondition condition = BYPASS);
	void ins_RET_CC(const enum JumpCondition condition);
	void ins_RET();
	void ins_RETI();
	void ins_RST(Byte jumpVector);
	void ins_LD_bFF00_u8b_A();
	void ins_LD_A_bFF00_u8b();
	void ins_LD_A_bFF00_Cb();
	void ins_ADD_SP_i8();
	void ins_LD_HL_SP_i8();
	void ins_LD_SP_HL();
	void ins_LD_bu16b_A();
	void ins_LD_A_bu16b();
	void ins_RLC(Byte* registerOne);
	void ins_RLC_bHLb();
	void ins_RL(Byte* registerOne);
	void ins_RL_bHLb();
	void ins_RRC(Byte* registerOne);
	void ins_RRC_bHLb();
	void ins_RR(Byte* registerOne);
	void ins_RR_bHLb();
	void ins_SLA(Byte* registerOne);
	void ins_SLA_bHLb();
	void ins_SRA(Byte* registerOne);
	void ins_SRA_bHLb();
	void ins_SRL(Byte* registerOne);
	void ins_SRL_bHLb();
	void ins_SWAP(Byte* registerOne);
	void ins_SWAP_bHLb();
	void ins_BIT_b_r(Byte bit, Byte* registerOne);
	void ins_BIT_b_r_bHLb(Byte bit);
	void ins_RES_b_r(Byte bit, Byte* registerOne);
	void ins_RES_b_r_bHLb(Byte bit);
	void ins_SET_b_r(Byte bit, Byte* registerOne);
	void ins_SET_b_r_bHLb(Byte bit);
};