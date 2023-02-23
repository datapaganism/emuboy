#pragma once

#include <iostream>

#include "config.hpp"


// forward declaration, bus.hpp is not included here but in cpu.cpp instead
class BUS;

enum eFlags
{
	z = 0b10000000, // Zero
	n = 0b01000000, // Subtraction
	h = 0b00100000, // Half Carry
	c = 0b00010000  // Carry
};

enum eJumpCondition
{
	NZ,
	Z,
	NC,
	C,
	BYPASS
};

enum eInterruptTypes
{
	vblank = 0b00000001,
	lcdstat = 0b00000010,
	timer = 0b00000100,
	serial = 0b00001000,
	joypad = 0b00010000,

};
static const eInterruptTypes eInterruptTypes_all[] = { vblank, lcdstat, timer, serial, joypad };
static const eInterruptTypes eInterruptTypes_all_prioritised[] = { joypad, serial, timer, lcdstat, vblank };


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



	const Word getAF() { return this->getWord(&this->a, &this->f); };
	const Word getBC() { return this->getWord(&this->b, &this->c); };
	const Word getDE() { return this->getWord(&this->d, &this->e); };
	const Word getHL() { return this->getWord(&this->h, &this->l); };

	void setAF(const Word value) { this->setWord(&this->a, &this->f, value); };
	void setBC(const Word value) { this->setWord(&this->b, &this->c, value); };
	void setDE(const Word value) { this->setWord(&this->d, &this->e, value); };
	void setHL(const Word value) { this->setWord(&this->h, &this->l, value); };

	void setLowByte(Word* word_register, const Byte value) { *word_register = (* word_register & 0xFF00) | value; };
	void setHighByte(Word* word_register, const Byte value) { *word_register = (* word_register & 0x00FF) | value << 8; };
	Byte getLowByte(Word* word_register) { return (*word_register & 0xFF00) >> 8; };
	Byte getHighByte(Word* word_register) { return (*word_register & 0x00FF); };


	bool getFlag(const eFlags flag)
	{
		return this->f & flag;
	}


	void setFlag(const eFlags flag, const bool value)
	{
		(value) ? this->f |= flag : this->f &= ~flag;
	}

	const Word getWord(Byte* register_one, Byte* register_two)
	{
		return ((*register_one << 8) | *register_two);
	}

	void setWord(Byte* register_one, Byte* register_two, const Word value)
	{
		*register_one = ((value & 0xFF00) >> 8);
		*register_two = (value & 0xFF);
		if (register_two == &this->f)
			*register_two = (value & 0xF0);
	}
};

struct TimerRegisters
{
	Byte div = 0xAB;
	Byte tima = 0x00;
	Byte tma = 0x00;
	Byte tac = 0xF8;
};


class CPU
{
public:
	bool debug_toggle = false;
	void biosInit();

	CPU();

	Registers registers;
	TimerRegisters timer_registers;

	bool interrupt_master_enable = 0;
	bool is_halted = 0;
	bool halt_bug = 0;
	void connectToBus(BUS* pBus);
	const Byte getByteFromPC();
	void mStepCPU();
	int mStepCPUOneInstruction();
	void haltHandler();
	void updateTimers(const int cycles);
	void requestInterrupt(const eInterruptTypes type);
	
	void DEBUG_printCurrentState(Word pc);
	void DEBUG_printCurrentState();
	void DEBUG_print_IO();

	bool is_instruction_complete = false;

private:
	BUS* bus = nullptr;
	void init();

	Byte getInterruptFlag(const enum eInterruptTypes type, Word address);
	void setInterruptFlag(const enum eInterruptTypes type, const bool value, Word address);
	
	int timer_counter = 0;
	int divtimer_counter = 0;

	Byte getTMCFrequency();
	int getTACFrequency();

	Byte current_running_opcode = 0;
	Byte current_running_cb = 0;
	Byte mcycles_used = 0;
	bool is_executing_instruction = false;
	bool is_executing_cb = false;
	Byte instruction_cache[5] = { 0 };
	Word interrupt_vector = 0;
	bool ei_triggered = false;

	void instructionHandler();
	void instructionHandlerCB();
	void instructionHandlerSTOP();
	void checkForInterrupts();
	void setupInterruptHandler();
	void setupForNextInstruction();
	void prefetchInstruction();

	
	Byte getNibble(const Byte input, const bool getHi);
	void setNibble(Byte* register_one, const Byte value, const bool setHi);

	bool checkCarry(const int a, const int b, const int shift, const int c = NULL);
	bool checkBorrow(const int a, const int b, const int shift, const int c = NULL);

	bool checkJumpCondition(enum eJumpCondition condition);

	Byte getMemory(const Word address);
	void setMemory(const Word address, const Byte data);


	// instruction declarations

	void ins_LD_XX_u16(Byte* const register_one, Byte* const register_two);
	void ins_LD_SP_u16(Word* const register_word);
	void ins_LD_bXXb_Y(const Word register_word_value, Byte* const register_byte);
	void ins_LD_bHLb_Apm(bool add);
	void ins_INC_XX(Byte* register_one, Byte* register_two);
	void ins_INC_SP();
	void ins_INC_X(Byte* register_byte);
	void ins_INC_bHLb();
	void ins_DEC_X(Byte* register_byte);
	void ins_DEC_bHLb();
	void ins_LD_X_u8(Byte* const register_byte);
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
	void ins_JR_i8(const enum eJumpCondition condition = BYPASS);
	void ins_ADD_HL_XX(Byte* const register_one, Byte* const register_two);
	void ins_ADD_HL_SP();
	void ins_ADD_A_X(Byte* const register_byte);
	void ins_ADD_A_bHLb();
	void ins_ADD_A_u8();
	void ins_LD_X_Y(Byte* const register_one, Byte* const register_two);
	void ins_LD_X_bYYb(Byte* const left_register, Byte* const right_register_one, Byte* const right_register_two, const Byte_s add_to_hl = NULL);
	void ins_DEC_XX(Byte* register_one, Byte* register_two);
	void ins_DEC_SP();
	void ins_ADC_A_X(const Byte* register_byte);
	void ins_ADC_A_bHLb();
	void ins_ADC_A_u8();
	void ins_SUB_A_X(Byte* const register_byte);
	void ins_SUB_A_u8();
	void ins_SUB_A_bHLb();
	void ins_SBC_A_X(Byte* const register_byte);
	void ins_SBC_A_u8();
	void ins_SBC_A_bHLb();
	void ins_AND_A_X(Byte* const register_byte);
	void ins_AND_A_u8();
	void ins_AND_A_bHLb();
	void ins_XOR_A_X(Byte* const register_byte);
	void ins_XOR_A_u8();
	void ins_XOR_A_bHLb();
	void ins_OR_A_X(Byte* const register_byte);
	void ins_OR_A_u8();
	void ins_OR_A_bHLb();
	void ins_CP_A_X(Byte* const register_byte);
	void ins_CP_A_u8();
	void ins_CP_A_bHLb();
	void ins_POP_XX(Byte* register_one, Byte* register_two);
	void ins_PUSH_XX(const Word reigster_word_value);
	void ins_JP_HL();
	void ins_CALL_u16(const enum eJumpCondition condition = BYPASS);
	void ins_JP_u16(const enum eJumpCondition condition = BYPASS);
	void ins_RET_CC(const enum eJumpCondition condition);
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
	void ins_RLC(Byte* register_one);
	void ins_RLC_bHLb();
	void ins_RL(Byte* register_one);
	void ins_RL_bHLb();
	void ins_RRC(Byte* register_one);
	void ins_RRC_bHLb();
	void ins_RR(Byte* register_one);
	void ins_RR_bHLb();
	void ins_SLA(Byte* register_one);
	void ins_SLA_bHLb();
	void ins_SRA(Byte* register_one);
	void ins_SRA_bHLb();
	void ins_SRL(Byte* register_one);
	void ins_SRL_bHLb();
	void ins_SWAP(Byte* register_one);
	void ins_SWAP_bHLb();
	void ins_BIT_b_r(Byte bit, Byte* register_one);
	void ins_BIT_b_r_bHLb(Byte bit);
	void ins_RES_b_r(Byte bit, Byte* register_one);
	void ins_RES_b_r_bHLb(Byte bit);
	void ins_SET_b_r(Byte bit, Byte* register_one);
	void ins_SET_b_r_bHLb(Byte bit);
};