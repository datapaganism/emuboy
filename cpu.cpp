#include "cpu.hpp"
#include "bus.hpp"

#include <iostream>
#include <bitset>
#include <sstream>

#define BREAKPOINT 0xCb15

void CPU::mStepCPU()
{

	if (this->isExecutingInstruction)
	{
		//decode and execute instruction // takes a cycle

		if (this->registers.pc == 0xC6c8)
		{
			throw "";
		}

		this->instruction_handler();
		//if finished instruction after this execution, prefetch
		if (!this->isExecutingInstruction)
		{
			//this->DEBUG_printCurrentState(BREAKPOINT);
			/*if (this->bus->work_ram[0xdffd - WORKRAMOFFSET] == 0xc2 && this->bus->work_ram[0xdffe - WORKRAMOFFSET] == 0x4c)
				throw "";*/

			this->prefetch_instruction();
			this->check_for_interrupts();
		}
		return;
	}

	// if we havent prefetched anything, fetch instruction // takes a cycle
	this->currentRunningOpcode = this->get_byte_from_pc(); // read opcode
	this->setup_for_next_instruction();
};


void CPU::DEBUG_printCurrentState(Word pc)
{
	if (this->registers.pc == pc)
		this->debug_toggle = true;
	if (this->debug_toggle)
	{
		printf("%s:0x%.4X  ", "pc", this->registers.pc);
		printf("%s:0x%.2X  ", "cyclesused", this->mCyclesUsed);
		printf("op:0x%.2X | ", this->bus->get_memory(this->registers.pc, MEMORY_ACCESS_TYPE::cpu));
		printf("%s:0x%.2X%.2X  ", "AF", this->registers.a, this->registers.f);
		printf("%s:0x%.2X%.2X  ", "BC", this->registers.b, this->registers.c);
		printf("%s:0x%.2X%.2X  ", "DE", this->registers.d, this->registers.e);
		printf("%s:0x%.2X%.2X  ", "HL", this->registers.h, this->registers.l);
		printf("%s:0x%.4X  ", "SP", this->registers.sp);
		printf("%s:0x%.4X  ", "STAT", this->bus->get_memory(STAT, MEMORY_ACCESS_TYPE::cpu));
		printf("%s:%i  ", "IME", this->interrupt_master_enable);

		/*printf("%s:%i  ","z", this->registers.get_flag(z));
		printf("%s:%i  ","n", this->registers.get_flag(n));
		printf("%s:%i  ","h", this->registers.get_flag(h));
		printf("%s:%i  ","c", this->registers.get_flag(c));
		printf("%s:","0xFF00");
		std::cout << std::bitset<8>(this->bus->io[0]);*/

		printf("\n");
	}
	if (this->registers.pc == pc || this->registers.pc == 0xFFFF)
		this->debug_toggle = true;
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

void CPU::bios_init()
{
	this->registers.pc = 0;
	this->registers.a = 0;
	this->registers.b = 0;
	this->registers.c = 0;
	this->registers.d = 0;
	this->registers.e = 0;
	this->registers.f = 0;
	this->registers.h = 0;
	this->registers.l = 0;
	this->registers.sp = 0;
}


CPU::CPU()
{
	this->init();
}


const Byte CPU::get_byte_from_pc()
{
	return this->bus->get_memory(this->registers.pc++, MEMORY_ACCESS_TYPE::cpu);
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

void CPU::DEBUG_print_IO()
{
	std::cout << "\t0xFF00  [JOYP]: ";  printf("%.2X\n", this->bus->get_memory(0xFF00, MEMORY_ACCESS_TYPE::cpu));
	std::cout << "\t0xFF01    [SB]: ";  printf("%.2X\n", this->bus->get_memory(0xFF01, MEMORY_ACCESS_TYPE::cpu));
	std::cout << "\t0xFF02    [SC]: ";  printf("%.2X\n", this->bus->get_memory(0xFF02, MEMORY_ACCESS_TYPE::cpu));
	std::cout << "\t0xFF04   [DIV]: ";  printf("%.2X\n", this->bus->get_memory(0xFF04, MEMORY_ACCESS_TYPE::cpu));
	std::cout << "\t0xFF05  [TIMA]: ";  printf("%.2X\n", this->bus->get_memory(0xFF05, MEMORY_ACCESS_TYPE::cpu));
	std::cout << "\t0xFF06   [TMA]: ";  printf("%.2X\n", this->bus->get_memory(0xFF06, MEMORY_ACCESS_TYPE::cpu));
	std::cout << "\t0xFF07   [TAC]: ";  printf("%.2X\n", this->bus->get_memory(0xFF07, MEMORY_ACCESS_TYPE::cpu));
	std::cout << "\t0xFF0F    [IF]: ";  printf("%.2X\n", this->bus->get_memory(0xFF0F, MEMORY_ACCESS_TYPE::cpu));
	std::cout << "\t0xFF40  [LCDC]: ";  printf("%.2X\n", this->bus->get_memory(0xFF40, MEMORY_ACCESS_TYPE::cpu));
	std::cout << "\t0xFF41  [STAT]: ";  printf("%.2X\n", this->bus->get_memory(0xFF41, MEMORY_ACCESS_TYPE::cpu));
	std::cout << "\t0xFF42   [SCY]: ";  printf("%.2X\n", this->bus->get_memory(0xFF42, MEMORY_ACCESS_TYPE::cpu));
	std::cout << "\t0xFF43   [SCX]: ";  printf("%.2X\n", this->bus->get_memory(0xFF43, MEMORY_ACCESS_TYPE::cpu));
	std::cout << "\t0xFF44    [LY]: ";  printf("%.2X\n", this->bus->get_memory(0xFF44, MEMORY_ACCESS_TYPE::cpu));
	std::cout << "\t0xFF45   [LYC]: ";  printf("%.2X\n", this->bus->get_memory(0xFF45, MEMORY_ACCESS_TYPE::cpu));
	std::cout << "\t0xFF47   [BGP]: ";  printf("%.2X\n", this->bus->get_memory(0xFF47, MEMORY_ACCESS_TYPE::cpu));
	std::cout << "\t0xFF48  [OPB0]: ";  printf("%.2X\n", this->bus->get_memory(0xFF48, MEMORY_ACCESS_TYPE::cpu));
	std::cout << "\t0xFF49  [OPB1]: ";  printf("%.2X\n", this->bus->get_memory(0xFF49, MEMORY_ACCESS_TYPE::cpu));
	std::cout << "\t0xFF4A    [WY]: ";  printf("%.2X\n", this->bus->get_memory(0xFF4A, MEMORY_ACCESS_TYPE::cpu));
	std::cout << "\t0xFF4B    [WX]: ";  printf("%.2X\n", this->bus->get_memory(0xFF4B, MEMORY_ACCESS_TYPE::cpu));
	std::cout << "\t0xFFFF    [IE]: ";  printf("%.2X\n", this->bus->interrupt_enable_register);
}

void CPU::connect_to_bus(BUS* pBus)
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

bool CPU::checkCarry(const int a, const int b, const int shift, const int c)
{
	int carries = (a + b + c) ^ (a ^ b ^ c);
	return carries & (1 << shift);
}

bool CPU::checkBorrow(const int a, const int b, const int shift, const int c)
{
	int borrows = (a - b - c) ^ (a ^ b ^ c);
	return borrows & (1 << shift);
}

bool CPU::checkJumpCondition(enum JumpCondition condition)
{
	switch (condition)
	{
	case BYPASS:
		return true;
		break;
	case NZ:
		if (!this->registers.get_flag(z))
			return true;
		break;
	case Z:
		if (this->registers.get_flag(z))
			return true;
		break;
	case NC:
		if (!this->registers.get_flag(c))
			return true;
		break;
	case C:
		if (this->registers.get_flag(c))
			return true;
		break;
	default: throw "Unreachable Jump condition"; break;
	}
	return false;
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

//int CPU::do_interrupts()
//{
//	int cyclesUsed = 0;
//	//check master to see if interrupts are enabled
//	if (this->interrupt_master_enable == true)
//	{
//		// iterate through all types, this allows us to check each interrupt and also service them in order of priority
//		for (const auto type : InterruptTypes_all)
//		{
//			// if it has been requested and its corresponding flag is enabled
//			if (this->get_interrupt_flag(type, IF_REGISTER) && this->get_interrupt_flag(type, IE_REGISTER))
//			{
//				//disable interrupts in general and for this type
//				this->set_interrupt_flag(type, 0, IF_REGISTER);
//				this->interrupt_master_enable = 0;
//
//				//push the current pc to the stack
//				this->ins_PUSH_nn(this->registers.pc);
//
//				// jump
//				switch (type)
//				{
//				case InterruptTypes::vblank: { this->registers.pc = 0x0040; } break;
//				case InterruptTypes::lcdstat: { this->registers.pc = 0x0048; } break;
//				case InterruptTypes::timer: { this->registers.pc = 0x0050; } break;
//				case InterruptTypes::serial: { this->registers.pc = 0x0058; } break;
//				case InterruptTypes::joypad: { this->registers.pc = 0x0060; } break;
//				default: throw "Unreachable interrupt type"; break;
//				}
//
//				//cyclesUsed += 20;
//			}
//		}
//	}
//	return cyclesUsed;
//}

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
		this->bus->io[DIV - IOOFFSET]++;
	}

	// if TMC bit 2 is set, this means that the timer is enabled
	if (this->bus->get_memory(TAC, MEMORY_ACCESS_TYPE::cpu) & (0b00000100))
	{
		//the timer increment holds the amount of cycles needed to tick TIMA register up one, we can decrement it and see if it has hit 0
		this->timerCounter -= cycles;

		// if the TIMA needs updating
		if (this->timerCounter <= 0)
		{
			if (this->bus->get_memory(TIMA, MEMORY_ACCESS_TYPE::cpu) == 0xFF)
			{
				this->bus->set_memory(TIMA, this->bus->get_memory(TMA, MEMORY_ACCESS_TYPE::cpu), MEMORY_ACCESS_TYPE::cpu);
				this->request_interrupt(timer);
			}

			this->bus->set_memory(TIMA, this->bus->get_memory(TIMA, MEMORY_ACCESS_TYPE::cpu) + 1, MEMORY_ACCESS_TYPE::cpu);
		}
	}
}

Byte CPU::get_TMC_frequency()
{
	return this->bus->get_memory(TAC, MEMORY_ACCESS_TYPE::cpu) & 0x3;
}

void CPU::update_timerCounter()
{
	switch (this->get_TMC_frequency())
	{
	case 0: { this->timerCounter = GB_CLOCKSPEED / 4096; } break;
	case 1: { this->timerCounter = GB_CLOCKSPEED / 262144; } break;
	case 2: { this->timerCounter = GB_CLOCKSPEED / 65536; } break;
	case 3: { this->timerCounter = GB_CLOCKSPEED / 16382; } break;
	default: throw "Unreachable timer frequency"; break;
	}
}


void CPU::request_interrupt(const InterruptTypes type)
{
	this->set_interrupt_flag(type, 1, IF_REGISTER);
}

void CPU::dma_transfer(Byte data)
{
	for (int i = 0; i < 0xA0; i++)
	{
		this->bus->set_memory(OAM + i, this->bus->get_memory((data << 8) + i, MEMORY_ACCESS_TYPE::cpu), MEMORY_ACCESS_TYPE::cpu);
	}
}

Byte CPU::get_interrupt_flag(const enum InterruptTypes type, Word address)
{
	return this->bus->get_memory(address, MEMORY_ACCESS_TYPE::interrupt_handler) & type;
}

void CPU::set_interrupt_flag(const enum InterruptTypes type, const bool value, Word address)
{
	auto mem_type = MEMORY_ACCESS_TYPE::interrupt_handler;
	auto original_value = this->bus->get_memory(address, mem_type);
	auto new_value = (value) ? (original_value | type) : (original_value & ~type);
	this->bus->set_memory(address, new_value, mem_type);
}

// some instructions only take a single cycle, peforming the fetch decode and execute in one mCycle

// need to implement the prefetcher:
// on the last mCycle of the execution of an instruction, the next opcode is prefetched and ready to run
// therefore next cycle we just execute it, this is why it may looks like f d e is performed in one go


void CPU::check_for_interrupts()
{
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
				//this->ins_PUSH_nn(this->registers.pc);

				// set jump vector
				switch (type)
				{
				case InterruptTypes::vblank: { this->interrupt_vector = 0x0040; } break;
				case InterruptTypes::lcdstat: { this->interrupt_vector = 0x0048; } break;
				case InterruptTypes::timer: { this->interrupt_vector = 0x0050; } break;
				case InterruptTypes::serial: { this->interrupt_vector = 0x0058; } break;
				case InterruptTypes::joypad: { this->interrupt_vector = 0x0060; } break;
				default: throw "Unreachable interrupt type"; break;
				}
			}
		}
	}
}
void CPU::setup_interrupt_handler()
{
	switch (this->mCyclesUsed)
	{
	case 0:
		this->mCyclesUsed++;
		// undocumented_internal_operation
		break;
	case 1:
		this->mCyclesUsed++;

		this->registers.sp--;
		this->bus->set_memory(this->registers.sp, ((this->registers.pc & 0xff00) >> 8), MEMORY_ACCESS_TYPE::cpu);

		break;
	case 2:
		this->mCyclesUsed++;

		this->registers.sp--;
		this->bus->set_memory(this->registers.sp, (this->registers.pc & 0x00ff), MEMORY_ACCESS_TYPE::cpu);

		break;

	case 3:

		this->registers.pc = this->interrupt_vector;

		this->interrupt_vector = 0;
		this->isExecutingInstruction = false;
		break;
	}
}

void CPU::setup_for_next_instruction()
{
	this->mCyclesUsed = 0;
	this->isExecutingInstruction = true;
}

void CPU::prefetch_instruction()
{
	this->setup_for_next_instruction();
	this->currentRunningOpcode = this->get_byte_from_pc();
}

void CPU::instruction_handler()
{
	if (this->interrupt_vector != 0)
	{
		this->setup_interrupt_handler();
		return;
	}
	switch (this->currentRunningOpcode)
	{
	case 0x00: { this->isExecutingInstruction = false; } break; //NOP
	case 0x01: { this->ins_LD_XX_u16(&this->registers.b, &this->registers.c); } break;
	case 0x02: { this->ins_LD_bXXb_Y(this->registers.get_BC(), &this->registers.a); } break;
	case 0x03: { this->ins_INC_XX(&this->registers.b, &this->registers.c); } break;
	case 0x04: { this->ins_INC_X(&this->registers.b); } break;
	case 0x05: { this->ins_DEC_X(&this->registers.b); } break;
	case 0x06: { this->ins_LD_X_u8(&this->registers.b); } break;
	case 0x07: { this->ins_RLCA(); } break;

	case 0x08: { this->ins_LD_bu16b_SP(); } break;
	case 0x09: { this->ins_ADD_HL_XX(&this->registers.b, &this->registers.c); } break;
	case 0x0A: { this->ins_LD_X_bYYb(&this->registers.a, &this->registers.b, &this->registers.c);  } break;
	case 0x0B: { this->ins_DEC_XX(&this->registers.b, &this->registers.c); } break;
	case 0x0C: { this->ins_INC_X(&this->registers.c); } break;
	case 0x0D: { this->ins_DEC_X(&this->registers.c); } break;
	case 0x0E: { this->ins_LD_X_u8(&this->registers.c); } break;
	case 0x0F: { this->ins_RRCA(); } break;

	case 0x10: { this->STOP_instruction_handler(); } break;
	case 0x11: { this->ins_LD_XX_u16(&this->registers.d, &this->registers.e); } break;
	case 0x12: { this->ins_LD_bXXb_Y(this->registers.get_DE(), &this->registers.a); } break;
	case 0x13: { this->ins_INC_XX(&this->registers.d, &this->registers.e); } break;
	case 0x14: { this->ins_INC_X(&this->registers.d); } break;
	case 0x15: { this->ins_DEC_X(&this->registers.d); } break;
	case 0x16: { this->ins_LD_X_u8(&this->registers.d); } break;
	case 0x17: { this->ins_RLA(); } break;

	case 0x18: { this->ins_JR_i8(); } break;
	case 0x19: { this->ins_ADD_HL_XX(&this->registers.d, &this->registers.e); } break;
	case 0x1A: { this->ins_LD_X_bYYb(&this->registers.a, &this->registers.d, &this->registers.e);  } break;
	case 0x1B: { this->ins_DEC_XX(&this->registers.d, &this->registers.e); } break;
	case 0x1C: { this->ins_INC_X(&this->registers.e); } break;
	case 0x1D: { this->ins_DEC_X(&this->registers.e); } break;
	case 0x1E: { this->ins_LD_X_u8(&this->registers.e); } break;
	case 0x1F: { this->ins_RRA(); } break;

	case 0x20: { this->ins_JR_i8(NZ); } break;
	case 0x21: { this->ins_LD_XX_u16(&this->registers.h, &this->registers.l); } break;
	case 0x22: { this->ins_LD_bHLb_Apm(true); } break;
	case 0x23: { this->ins_INC_XX(&this->registers.h, &this->registers.l); } break;
	case 0x24: { this->ins_INC_X(&this->registers.h); } break;
	case 0x25: { this->ins_DEC_X(&this->registers.h); } break;
	case 0x26: { this->ins_LD_X_u8(&this->registers.h); } break;
	case 0x27: { this->ins_DAA(); } break;

	case 0x28: { this->ins_JR_i8(Z); } break;
	case 0x29: { this->ins_ADD_HL_XX(&this->registers.h, &this->registers.l); } break;
	case 0x2A: { this->ins_LD_X_bYYb(&this->registers.a, &this->registers.h, &this->registers.l, +1);  } break;
	case 0x2B: { this->ins_DEC_XX(&this->registers.h, &this->registers.l); } break;
	case 0x2C: { this->ins_INC_X(&this->registers.l); } break;
	case 0x2D: { this->ins_DEC_X(&this->registers.l); } break;
	case 0x2E: { this->ins_LD_X_u8(&this->registers.l); } break;
	case 0x2F: { this->ins_CPL(); } break;

	case 0x30: { this->ins_JR_i8(NC); } break;
	case 0x31: { this->ins_LD_SP_u16(&this->registers.sp); } break; //sp is a word
	case 0x32: { this->ins_LD_bHLb_Apm(false); } break;
	case 0x33: { this->ins_INC_SP(); } break;
	case 0x34: { this->ins_INC_bHLb(); } break;
	case 0x35: { this->ins_DEC_bHLb(); } break;
	case 0x36: { this->ins_LD_bHLb_u8(); } break;
	case 0x37: { this->ins_SCF(); } break;

	case 0x38: { this->ins_JR_i8(C); } break;
	case 0x39: { this->ins_ADD_HL_SP(); } break;
	case 0x3A: { this->ins_LD_X_bYYb(&this->registers.l, &this->registers.h, &this->registers.l, -1);  } break;
	case 0x3B: { this->ins_DEC_SP(); } break;
	case 0x3C: { this->ins_INC_X(&this->registers.a); } break;
	case 0x3D: { this->ins_DEC_X(&this->registers.a); } break;
	case 0x3E: { this->ins_LD_X_u8(&this->registers.a); } break;
	case 0x3F: { this->ins_CCF(); } break;

	case 0x40: { this->ins_LD_X_Y(&this->registers.b, &this->registers.b); } break;
	case 0x41: { this->ins_LD_X_Y(&this->registers.b, &this->registers.c); } break;
	case 0x42: { this->ins_LD_X_Y(&this->registers.b, &this->registers.d); } break;
	case 0x43: { this->ins_LD_X_Y(&this->registers.b, &this->registers.e); } break;
	case 0x44: { this->ins_LD_X_Y(&this->registers.b, &this->registers.h); } break;
	case 0x45: { this->ins_LD_X_Y(&this->registers.b, &this->registers.l); } break;
	case 0x46: { this->ins_LD_X_bYYb(&this->registers.b, &this->registers.h, &this->registers.l); } break;
	case 0x47: { this->ins_LD_X_Y(&this->registers.b, &this->registers.a); } break;

	case 0x48: { this->ins_LD_X_Y(&this->registers.c, &this->registers.b); } break;
	case 0x49: { this->ins_LD_X_Y(&this->registers.c, &this->registers.c); } break;
	case 0x4A: { this->ins_LD_X_Y(&this->registers.c, &this->registers.d); } break;
	case 0x4B: { this->ins_LD_X_Y(&this->registers.c, &this->registers.e); } break;
	case 0x4C: { this->ins_LD_X_Y(&this->registers.c, &this->registers.h); } break;
	case 0x4D: { this->ins_LD_X_Y(&this->registers.c, &this->registers.l); } break;
	case 0x4E: { this->ins_LD_X_bYYb(&this->registers.c, &this->registers.h, &this->registers.l); } break;
	case 0x4F: { this->ins_LD_X_Y(&this->registers.c, &this->registers.a); } break;

	case 0x50: { this->ins_LD_X_Y(&this->registers.d, &this->registers.b); } break;
	case 0x51: { this->ins_LD_X_Y(&this->registers.d, &this->registers.c); } break;
	case 0x52: { this->ins_LD_X_Y(&this->registers.d, &this->registers.d); } break;
	case 0x53: { this->ins_LD_X_Y(&this->registers.d, &this->registers.e); } break;
	case 0x54: { this->ins_LD_X_Y(&this->registers.d, &this->registers.h); } break;
	case 0x55: { this->ins_LD_X_Y(&this->registers.d, &this->registers.l); } break;
	case 0x56: { this->ins_LD_X_bYYb(&this->registers.d, &this->registers.h, &this->registers.l); } break;
	case 0x57: { this->ins_LD_X_Y(&this->registers.d, &this->registers.a); } break;

	case 0x58: { this->ins_LD_X_Y(&this->registers.e, &this->registers.b); } break;
	case 0x59: { this->ins_LD_X_Y(&this->registers.e, &this->registers.c); } break;
	case 0x5A: { this->ins_LD_X_Y(&this->registers.e, &this->registers.d); } break;
	case 0x5B: { this->ins_LD_X_Y(&this->registers.e, &this->registers.e); } break;
	case 0x5C: { this->ins_LD_X_Y(&this->registers.e, &this->registers.h); } break;
	case 0x5D: { this->ins_LD_X_Y(&this->registers.e, &this->registers.l); } break;
	case 0x5E: { this->ins_LD_X_bYYb(&this->registers.e, &this->registers.h, &this->registers.l); } break;
	case 0x5F: { this->ins_LD_X_Y(&this->registers.e, &this->registers.a); } break;

	case 0x60: { this->ins_LD_X_Y(&this->registers.h, &this->registers.b); } break;
	case 0x61: { this->ins_LD_X_Y(&this->registers.h, &this->registers.c); } break;
	case 0x62: { this->ins_LD_X_Y(&this->registers.h, &this->registers.d); } break;
	case 0x63: { this->ins_LD_X_Y(&this->registers.h, &this->registers.e); } break;
	case 0x64: { this->ins_LD_X_Y(&this->registers.h, &this->registers.h); } break;
	case 0x65: { this->ins_LD_X_Y(&this->registers.h, &this->registers.l); } break;
	case 0x66: { this->ins_LD_X_bYYb(&this->registers.h, &this->registers.h, &this->registers.l); } break;
	case 0x67: { this->ins_LD_X_Y(&this->registers.h, &this->registers.a); } break;

	case 0x68: { this->ins_LD_X_Y(&this->registers.l, &this->registers.b); } break;
	case 0x69: { this->ins_LD_X_Y(&this->registers.l, &this->registers.c); } break;
	case 0x6A: { this->ins_LD_X_Y(&this->registers.l, &this->registers.d); } break;
	case 0x6B: { this->ins_LD_X_Y(&this->registers.l, &this->registers.e); } break;
	case 0x6C: { this->ins_LD_X_Y(&this->registers.l, &this->registers.h); } break;
	case 0x6D: { this->ins_LD_X_Y(&this->registers.l, &this->registers.l); } break;
	case 0x6E: { this->ins_LD_X_bYYb(&this->registers.l, &this->registers.h, &this->registers.l); } break;
	case 0x6F: { this->ins_LD_X_Y(&this->registers.l, &this->registers.a); } break;

	case 0x70: { this->ins_LD_bXXb_Y(this->registers.get_HL(), &this->registers.b); } break;
	case 0x71: { this->ins_LD_bXXb_Y(this->registers.get_HL(), &this->registers.c); } break;
	case 0x72: { this->ins_LD_bXXb_Y(this->registers.get_HL(), &this->registers.d); } break;
	case 0x73: { this->ins_LD_bXXb_Y(this->registers.get_HL(), &this->registers.e); } break;
	case 0x74: { this->ins_LD_bXXb_Y(this->registers.get_HL(), &this->registers.h); } break;
	case 0x75: { this->ins_LD_bXXb_Y(this->registers.get_HL(), &this->registers.l); } break;
	case 0x76: { this->is_halted = true; } break; //HALT
	case 0x77: { this->ins_LD_bXXb_Y(this->registers.get_HL(), &this->registers.a); } break;

	case 0x78: { this->ins_LD_X_Y(&this->registers.a, &this->registers.b); } break;
	case 0x79: { this->ins_LD_X_Y(&this->registers.a, &this->registers.c); } break;
	case 0x7A: { this->ins_LD_X_Y(&this->registers.a, &this->registers.d); } break;
	case 0x7B: { this->ins_LD_X_Y(&this->registers.a, &this->registers.e); } break;
	case 0x7C: { this->ins_LD_X_Y(&this->registers.a, &this->registers.h); } break;
	case 0x7D: { this->ins_LD_X_Y(&this->registers.a, &this->registers.l); } break;
	case 0x7E: { this->ins_LD_X_bYYb(&this->registers.a, &this->registers.h, &this->registers.l); } break;
	case 0x7F: { this->ins_LD_X_Y(&this->registers.a, &this->registers.a); } break;

	case 0x80: { this->ins_ADD_A_X(&this->registers.b); } break;
	case 0x81: { this->ins_ADD_A_X(&this->registers.c); } break;
	case 0x82: { this->ins_ADD_A_X(&this->registers.d); } break;
	case 0x83: { this->ins_ADD_A_X(&this->registers.e); } break;
	case 0x84: { this->ins_ADD_A_X(&this->registers.h); } break;
	case 0x85: { this->ins_ADD_A_X(&this->registers.l); } break;
	case 0x86: { this->ins_ADD_A_bHLb(); } break;
	case 0x87: { this->ins_ADD_A_X(&this->registers.a); } break;

	case 0x88: { this->ins_ADC_A_X(&this->registers.b); } break;
	case 0x89: { this->ins_ADC_A_X(&this->registers.c); } break;
	case 0x8A: { this->ins_ADC_A_X(&this->registers.d); } break;
	case 0x8B: { this->ins_ADC_A_X(&this->registers.e); } break;
	case 0x8C: { this->ins_ADC_A_X(&this->registers.h); } break;
	case 0x8D: { this->ins_ADC_A_X(&this->registers.l); } break;
	case 0x8E: { this->ins_ADC_A_bHLb(); } break;
	case 0x8F: { this->ins_ADC_A_X(&this->registers.a); } break;

	case 0x90: { this->ins_SUB_A_X(&this->registers.b); } break;
	case 0x91: { this->ins_SUB_A_X(&this->registers.c); } break;
	case 0x92: { this->ins_SUB_A_X(&this->registers.d); } break;
	case 0x93: { this->ins_SUB_A_X(&this->registers.e); } break;
	case 0x94: { this->ins_SUB_A_X(&this->registers.h); } break;
	case 0x95: { this->ins_SUB_A_X(&this->registers.l); } break;
	case 0x96: { this->ins_SUB_A_bHLb(); } break;
	case 0x97: { this->ins_SUB_A_X(&this->registers.a); } break;

	case 0x98: { this->ins_SBC_A_X(&this->registers.b); } break;
	case 0x99: { this->ins_SBC_A_X(&this->registers.c); } break;
	case 0x9A: { this->ins_SBC_A_X(&this->registers.d); } break;
	case 0x9B: { this->ins_SBC_A_X(&this->registers.e); } break;
	case 0x9C: { this->ins_SBC_A_X(&this->registers.h); } break;
	case 0x9D: { this->ins_SBC_A_X(&this->registers.l); } break;
	case 0x9E: { this->ins_SBC_A_bHLb(); } break;
	case 0x9F: { this->ins_SBC_A_X(&this->registers.a); } break;

	case 0xA0: { this->ins_AND_A_X(&this->registers.b); } break;
	case 0xA1: { this->ins_AND_A_X(&this->registers.c); } break;
	case 0xA2: { this->ins_AND_A_X(&this->registers.d); } break;
	case 0xA3: { this->ins_AND_A_X(&this->registers.e); } break;
	case 0xA4: { this->ins_AND_A_X(&this->registers.h); } break;
	case 0xA5: { this->ins_AND_A_X(&this->registers.l); } break;
	case 0xA6: { this->ins_AND_A_bHLb(); } break;
	case 0xA7: { this->ins_AND_A_X(&this->registers.a); } break;

	case 0xA8: { this->ins_XOR_A_X(&this->registers.b); } break;
	case 0xA9: { this->ins_XOR_A_X(&this->registers.c); } break;
	case 0xAA: { this->ins_XOR_A_X(&this->registers.d); } break;
	case 0xAB: { this->ins_XOR_A_X(&this->registers.e); } break;
	case 0xAC: { this->ins_XOR_A_X(&this->registers.h); } break;
	case 0xAD: { this->ins_XOR_A_X(&this->registers.l); } break;
	case 0xAE: { this->ins_XOR_A_bHLb(); } break;
	case 0xAF: { this->ins_XOR_A_X(&this->registers.a); } break;

	case 0xB0: { this->ins_OR_A_X(&this->registers.b); } break;
	case 0xB1: { this->ins_OR_A_X(&this->registers.c); } break;
	case 0xB2: { this->ins_OR_A_X(&this->registers.d); } break;
	case 0xB3: { this->ins_OR_A_X(&this->registers.e); } break;
	case 0xB4: { this->ins_OR_A_X(&this->registers.h); } break;
	case 0xB5: { this->ins_OR_A_X(&this->registers.l); } break;
	case 0xB6: { this->ins_OR_A_bHLb(); } break;
	case 0xB7: { this->ins_OR_A_X(&this->registers.a); } break;

	case 0xB8: { this->ins_CP_A_X(&this->registers.b); } break;
	case 0xB9: { this->ins_CP_A_X(&this->registers.c); } break;
	case 0xBA: { this->ins_CP_A_X(&this->registers.d); } break;
	case 0xBB: { this->ins_CP_A_X(&this->registers.e); } break;
	case 0xBC: { this->ins_CP_A_X(&this->registers.h); } break;
	case 0xBD: { this->ins_CP_A_X(&this->registers.l); } break;
	case 0xBE: { this->ins_CP_A_bHLb(); } break;
	case 0xBF: { this->ins_CP_A_X(&this->registers.a); } break;

	case 0xC0: { this->ins_RET_CC(NZ); } break;
	case 0xC1: { this->ins_POP_XX(&this->registers.b, &this->registers.c); } break;
	case 0xC2: { this->ins_JP_u16(NZ); } break;
	case 0xC3: { this->ins_JP_u16(); } break;
	case 0xC4: { this->ins_CALL_u16(NZ); } break;
	case 0xC5: { this->ins_PUSH_XX(this->registers.get_BC()); } break;
	case 0xC6: { this->ins_AND_A_u8(); } break;
	case 0xC7: { this->ins_RST(0x00); } break;

	case 0xC8: { this->ins_RET_CC(Z); } break;
	case 0xC9: { this->ins_RET(); } break;
	case 0xCA: { this->ins_JP_u16(Z); } break;
	case 0xCB: { this->CB_instruction_handler(); } break;
	case 0xCC: { this->ins_CALL_u16(Z); } break;
	case 0xCD: { this->ins_CALL_u16(); } break;
	case 0xCE: { this->ins_ADC_A_u8(); } break;
	case 0xCF: { this->ins_RST(0x08); } break;

	case 0xD0: { this->ins_RET_CC(NC); } break;
	case 0xD1: { this->ins_POP_XX(&this->registers.d, &this->registers.e); } break;
	case 0xD2: { this->ins_JP_u16(NC); } break;
		//case 0xD3: { this->ins_SBC_A_n(nullptr, this->get_byte_from_pc()); } break;
	case 0xD4: { this->ins_CALL_u16(NC); } break;
	case 0xD5: { this->ins_PUSH_XX(this->registers.get_DE()); } break;
	case 0xD6: { this->ins_SUB_A_u8();  } break;
	case 0xD7: { this->ins_RST(0x10); } break;

	case 0xD8: { this->ins_RET_CC(C); } break; //35
	case 0xD9: { this->ins_RETI(); } break;
	case 0xDA: { this->ins_JP_u16(C); } break;
		//case 0xDB: { } break;
	case 0xDC: { this->ins_CALL_u16(C); } break;
		//case 0xDD: { } break;
	case 0xDE: { this->ins_SBC_A_u8(); } break;
	case 0xDF: { this->ins_RST(0x18); } break;

	case 0xE0: { ins_LD_bFF00_u8b_A(); } break;
	case 0xE1: { this->ins_POP_XX(&this->registers.h, &this->registers.l); } break;
	case 0xE2: { this->ins_LD_bXXb_Y(0xFF00 + this->registers.c, &this->registers.a); } break;
		//case 0xE3: { } break;
		//case 0xE4: { } break;
	case 0xE5: { this->ins_PUSH_XX(this->registers.get_HL()); } break;
	case 0xE6: { this->ins_AND_A_u8(); } break;
	case 0xE7: { this->ins_RST(0x20); } break;

	case 0xE8: { this->ins_ADD_SP_i8(); } break;
	case 0xE9: { this->ins_JP_HL(); } break;
	case 0xEA: { this->ins_LD_bu16u_A(); } break;
		//case 0xEB: { } break;
		//case 0xEC: { } break;
		//case 0xED: { } break;
	case 0xEE: { this->ins_XOR_A_u8(); } break;
	case 0xEF: { this->ins_RST(0x28); } break;

	case 0xF0: { ins_LD_A_bFF00_u8b(); } break;
	case 0xF1: { this->ins_POP_XX(&this->registers.a, &this->registers.f); } break;
	case 0xF2: { this->ins_LD_A_bFF00_u8b(); } break;
	case 0xF3: { this->interrupt_master_enable = 0; this->EI_triggered = false; this->isExecutingInstruction = false; } break; //DIsable interrupts immediately, if EI is set to trigger, we can disable it so we can keep the behaviour of EI then DI never enabling interrupts
	//case 0xF4: { } break;
	case 0xF5: { this->ins_PUSH_XX(this->registers.get_AF()); } break;
	case 0xF6: { this->ins_OR_A_u8(); } break;
	case 0xF7: { this->ins_RST(0x30); } break;

	case 0xF8: { this->ins_LD_HL_SP_i8(); } break;
	case 0xF9: { this->ins_LD_SP_HL(); } break;
	case 0xFA: { this->ins_LD_A_bu16u(); } break;
	case 0xFB: { this->EI_triggered = true; this->isExecutingInstruction = false; return; } break; // EI is set to trigger, only activates after next instruction finishes note the early return
	//case 0xFC: { } break;
	//case 0xFD: { } break;
	case 0xFE: { this->ins_CP_A_u8(); } break;
	case 0xFF: { this->ins_RST(0x38); } break;
	default: { printf("ILLEGAL OPCODE CALL %0.2X \n", this->currentRunningOpcode); }
	}

	if (this->EI_triggered)
	{
		this->interrupt_master_enable = 1;
		this->EI_triggered = false;
	}
}
void CPU::CB_instruction_handler()
{
	//fetch cb instruction
	if (!this->isExecutingCB)
	{
		this->currentRunningCB = this->get_byte_from_pc();
		this->isExecutingCB = true;
		//return;
	}

	switch (this->currentRunningCB)
	{
	case 0x00: { this->ins_RLC(&this->registers.b); } break;
	case 0x01: { this->ins_RLC(&this->registers.c); } break;
	case 0x02: { this->ins_RLC(&this->registers.d); } break;
	case 0x03: { this->ins_RLC(&this->registers.e); } break;
	case 0x04: { this->ins_RLC(&this->registers.h); } break;
	case 0x05: { this->ins_RLC(&this->registers.l); } break;
	case 0x06: { this->ins_RLC_bHLb(); } break;
	case 0x07: { this->ins_RLC(&this->registers.a); } break;

	case 0x08: { this->ins_RRC(&this->registers.b); } break;
	case 0x09: { this->ins_RRC(&this->registers.c); } break;
	case 0x0A: { this->ins_RRC(&this->registers.d); } break;
	case 0x0B: { this->ins_RRC(&this->registers.e); } break;
	case 0x0C: { this->ins_RRC(&this->registers.h); } break;
	case 0x0D: { this->ins_RRC(&this->registers.l); } break;
	case 0x0E: { this->ins_RRC_bHLb(); } break;
	case 0x0F: { this->ins_RRC(&this->registers.a); } break;

	case 0x10: { this->ins_RL(&this->registers.b); } break;
	case 0x11: { this->ins_RL(&this->registers.c); } break;
	case 0x12: { this->ins_RL(&this->registers.d); } break;
	case 0x13: { this->ins_RL(&this->registers.e); } break;
	case 0x14: { this->ins_RL(&this->registers.h); } break;
	case 0x15: { this->ins_RL(&this->registers.l); } break;
	case 0x16: { this->ins_RL_bHLb(); } break;
	case 0x17: { this->ins_RL(&this->registers.a); } break;

	case 0x18: { this->ins_RR(&this->registers.b); } break;
	case 0x19: { this->ins_RR(&this->registers.c); } break;
	case 0x1A: { this->ins_RR(&this->registers.d); } break;
	case 0x1B: { this->ins_RR(&this->registers.e); } break;
	case 0x1C: { this->ins_RR(&this->registers.h); } break;
	case 0x1D: { this->ins_RR(&this->registers.l); } break;
	case 0x1E: { this->ins_RR_bHLb(); } break;
	case 0x1F: { this->ins_RR(&this->registers.a); } break;

	case 0x20: { this->ins_SLA(&this->registers.b); } break;
	case 0x21: { this->ins_SLA(&this->registers.c); } break;
	case 0x22: { this->ins_SLA(&this->registers.d); } break;
	case 0x23: { this->ins_SLA(&this->registers.e); } break;
	case 0x24: { this->ins_SLA(&this->registers.h); } break;
	case 0x25: { this->ins_SLA(&this->registers.l); } break;
	case 0x26: { this->ins_SLA_bHLb(); } break;
	case 0x27: { this->ins_SLA(&this->registers.a); } break;

	case 0x28: { this->ins_SRA(&this->registers.b); } break;
	case 0x29: { this->ins_SRA(&this->registers.c); } break;
	case 0x2A: { this->ins_SRA(&this->registers.d); } break;
	case 0x2B: { this->ins_SRA(&this->registers.e); } break;
	case 0x2C: { this->ins_SRA(&this->registers.h); } break;
	case 0x2D: { this->ins_SRA(&this->registers.l); } break;
	case 0x2E: { this->ins_SRA_bHLb(); } break;
	case 0x2F: { this->ins_SRA(&this->registers.a); } break;

	case 0x30: { this->ins_SWAP(&this->registers.b); } break;
	case 0x31: { this->ins_SWAP(&this->registers.c); } break;
	case 0x32: { this->ins_SWAP(&this->registers.d); } break;
	case 0x33: { this->ins_SWAP(&this->registers.e); } break;
	case 0x34: { this->ins_SWAP(&this->registers.h); } break;
	case 0x35: { this->ins_SWAP(&this->registers.l); } break;
	case 0x36: { this->ins_SWAP_bHLb(); } break;
	case 0x37: { this->ins_SWAP(&this->registers.a); } break;

	case 0x38: { this->ins_SRL(&this->registers.b); } break;
	case 0x39: { this->ins_SRL(&this->registers.c); } break;
	case 0x3A: { this->ins_SRL(&this->registers.d); } break;
	case 0x3B: { this->ins_SRL(&this->registers.e); } break;
	case 0x3C: { this->ins_SRL(&this->registers.h); } break;
	case 0x3D: { this->ins_SRL(&this->registers.l); } break;
	case 0x3E: { this->ins_SRL_bHLb(); } break;
	case 0x3F: { this->ins_SRL(&this->registers.a); } break;

	case 0x40: { this->ins_BIT_b_r(0, &this->registers.b); } break;
	case 0x41: { this->ins_BIT_b_r(0, &this->registers.c); } break;
	case 0x42: { this->ins_BIT_b_r(0, &this->registers.d); } break;
	case 0x43: { this->ins_BIT_b_r(0, &this->registers.e); } break;
	case 0x44: { this->ins_BIT_b_r(0, &this->registers.h); } break;
	case 0x45: { this->ins_BIT_b_r(0, &this->registers.l); } break;
	case 0x46: { this->ins_BIT_b_r_bHLb(0); } break;
	case 0x47: { this->ins_BIT_b_r(0, &this->registers.a); } break;

	case 0x48: { this->ins_BIT_b_r(1, &this->registers.b); } break;
	case 0x49: { this->ins_BIT_b_r(1, &this->registers.c); } break;
	case 0x4A: { this->ins_BIT_b_r(1, &this->registers.d); } break;
	case 0x4B: { this->ins_BIT_b_r(1, &this->registers.e); } break;
	case 0x4C: { this->ins_BIT_b_r(1, &this->registers.h); } break;
	case 0x4D: { this->ins_BIT_b_r(1, &this->registers.l); } break;
	case 0x4E: { this->ins_BIT_b_r_bHLb(1); } break;
	case 0x4F: { this->ins_BIT_b_r(1, &this->registers.a); } break;

	case 0x50: { this->ins_BIT_b_r(2, &this->registers.b); } break;
	case 0x51: { this->ins_BIT_b_r(2, &this->registers.c); } break;
	case 0x52: { this->ins_BIT_b_r(2, &this->registers.d); } break;
	case 0x53: { this->ins_BIT_b_r(2, &this->registers.e); } break;
	case 0x54: { this->ins_BIT_b_r(2, &this->registers.h); } break;
	case 0x55: { this->ins_BIT_b_r(2, &this->registers.l); } break;
	case 0x56: { this->ins_BIT_b_r_bHLb(2); } break;
	case 0x57: { this->ins_BIT_b_r(2, &this->registers.a); } break;

	case 0x58: { this->ins_BIT_b_r(3, &this->registers.b); } break;
	case 0x59: { this->ins_BIT_b_r(3, &this->registers.c); } break;
	case 0x5A: { this->ins_BIT_b_r(3, &this->registers.d); } break;
	case 0x5B: { this->ins_BIT_b_r(3, &this->registers.e); } break;
	case 0x5C: { this->ins_BIT_b_r(3, &this->registers.h); } break;
	case 0x5D: { this->ins_BIT_b_r(3, &this->registers.l); } break;
	case 0x5E: { this->ins_BIT_b_r_bHLb(3); } break;
	case 0x5F: { this->ins_BIT_b_r(3, &this->registers.a); } break;

	case 0x60: { this->ins_BIT_b_r(4, &this->registers.b); } break;
	case 0x61: { this->ins_BIT_b_r(4, &this->registers.c); } break;
	case 0x62: { this->ins_BIT_b_r(4, &this->registers.d); } break;
	case 0x63: { this->ins_BIT_b_r(4, &this->registers.e); } break;
	case 0x64: { this->ins_BIT_b_r(4, &this->registers.h); } break;
	case 0x65: { this->ins_BIT_b_r(4, &this->registers.l); } break;
	case 0x66: { this->ins_BIT_b_r_bHLb(4); } break;
	case 0x67: { this->ins_BIT_b_r(4, &this->registers.a); } break;

	case 0x68: { this->ins_BIT_b_r(5, &this->registers.b); } break;
	case 0x69: { this->ins_BIT_b_r(5, &this->registers.c); } break;
	case 0x6A: { this->ins_BIT_b_r(5, &this->registers.d); } break;
	case 0x6B: { this->ins_BIT_b_r(5, &this->registers.e); } break;
	case 0x6C: { this->ins_BIT_b_r(5, &this->registers.h); } break;
	case 0x6D: { this->ins_BIT_b_r(5, &this->registers.l); } break;
	case 0x6E: { this->ins_BIT_b_r_bHLb(5); } break;
	case 0x6F: { this->ins_BIT_b_r(5, &this->registers.a); } break;

	case 0x70: { this->ins_BIT_b_r(6, &this->registers.b); } break;
	case 0x71: { this->ins_BIT_b_r(6, &this->registers.c); } break;
	case 0x72: { this->ins_BIT_b_r(6, &this->registers.d); } break;
	case 0x73: { this->ins_BIT_b_r(6, &this->registers.e); } break;
	case 0x74: { this->ins_BIT_b_r(6, &this->registers.h); } break;
	case 0x75: { this->ins_BIT_b_r(6, &this->registers.l); } break;
	case 0x76: { this->ins_BIT_b_r_bHLb(6); } break;
	case 0x77: { this->ins_BIT_b_r(6, &this->registers.a); } break;

	case 0x78: { this->ins_BIT_b_r(7, &this->registers.b); } break;
	case 0x79: { this->ins_BIT_b_r(7, &this->registers.c); } break;
	case 0x7A: { this->ins_BIT_b_r(7, &this->registers.d); } break;
	case 0x7B: { this->ins_BIT_b_r(7, &this->registers.e); } break;
	case 0x7C: { this->ins_BIT_b_r(7, &this->registers.h); } break;
	case 0x7D: { this->ins_BIT_b_r(7, &this->registers.l); } break;
	case 0x7E: { this->ins_BIT_b_r_bHLb(7); } break;
	case 0x7F: { this->ins_BIT_b_r(7, &this->registers.a); } break;

	case 0x80: { this->ins_RES_b_r(0, &this->registers.b); } break;
	case 0x81: { this->ins_RES_b_r(0, &this->registers.c); } break;
	case 0x82: { this->ins_RES_b_r(0, &this->registers.d); } break;
	case 0x83: { this->ins_RES_b_r(0, &this->registers.e); } break;
	case 0x84: { this->ins_RES_b_r(0, &this->registers.h); } break;
	case 0x85: { this->ins_RES_b_r(0, &this->registers.l); } break;
	case 0x86: { this->ins_RES_b_r_bHLb(0); } break;
	case 0x87: { this->ins_RES_b_r(0, &this->registers.a); } break;

	case 0x88: { this->ins_RES_b_r(1, &this->registers.b); } break;
	case 0x89: { this->ins_RES_b_r(1, &this->registers.c); } break;
	case 0x8A: { this->ins_RES_b_r(1, &this->registers.d); } break;
	case 0x8B: { this->ins_RES_b_r(1, &this->registers.e); } break;
	case 0x8C: { this->ins_RES_b_r(1, &this->registers.h); } break;
	case 0x8D: { this->ins_RES_b_r(1, &this->registers.l); } break;
	case 0x8E: { this->ins_RES_b_r_bHLb(1); } break;
	case 0x8F: { this->ins_RES_b_r(1, &this->registers.a); } break;

	case 0x90: { this->ins_RES_b_r(2, &this->registers.b); } break;
	case 0x91: { this->ins_RES_b_r(2, &this->registers.c); } break;
	case 0x92: { this->ins_RES_b_r(2, &this->registers.d); } break;
	case 0x93: { this->ins_RES_b_r(2, &this->registers.e); } break;
	case 0x94: { this->ins_RES_b_r(2, &this->registers.h); } break;
	case 0x95: { this->ins_RES_b_r(2, &this->registers.l); } break;
	case 0x96: { this->ins_RES_b_r_bHLb(2); } break;
	case 0x97: { this->ins_RES_b_r(2, &this->registers.a); } break;

	case 0x98: { this->ins_RES_b_r(3, &this->registers.b); } break;
	case 0x99: { this->ins_RES_b_r(3, &this->registers.c); } break;
	case 0x9A: { this->ins_RES_b_r(3, &this->registers.d); } break;
	case 0x9B: { this->ins_RES_b_r(3, &this->registers.e); } break;
	case 0x9C: { this->ins_RES_b_r(3, &this->registers.h); } break;
	case 0x9D: { this->ins_RES_b_r(3, &this->registers.l); } break;
	case 0x9E: { this->ins_RES_b_r_bHLb(3); } break;
	case 0x9F: { this->ins_RES_b_r(3, &this->registers.a); } break;

	case 0xA0: { this->ins_RES_b_r(4, &this->registers.b); } break;
	case 0xA1: { this->ins_RES_b_r(4, &this->registers.c); } break;
	case 0xA2: { this->ins_RES_b_r(4, &this->registers.d); } break;
	case 0xA3: { this->ins_RES_b_r(4, &this->registers.e); } break;
	case 0xA4: { this->ins_RES_b_r(4, &this->registers.h); } break;
	case 0xA5: { this->ins_RES_b_r(4, &this->registers.l); } break;
	case 0xA6: { this->ins_RES_b_r_bHLb(4); } break;
	case 0xA7: { this->ins_RES_b_r(4, &this->registers.a); } break;

	case 0xA8: { this->ins_RES_b_r(5, &this->registers.b); } break;
	case 0xA9: { this->ins_RES_b_r(5, &this->registers.c); } break;
	case 0xAA: { this->ins_RES_b_r(5, &this->registers.d); } break;
	case 0xAB: { this->ins_RES_b_r(5, &this->registers.e); } break;
	case 0xAC: { this->ins_RES_b_r(5, &this->registers.h); } break;
	case 0xAD: { this->ins_RES_b_r(5, &this->registers.l); } break;
	case 0xAE: { this->ins_RES_b_r_bHLb(5); } break;
	case 0xAF: { this->ins_RES_b_r(5, &this->registers.a); } break;

	case 0xB0: { this->ins_RES_b_r(6, &this->registers.b); } break;
	case 0xB1: { this->ins_RES_b_r(6, &this->registers.c); } break;
	case 0xB2: { this->ins_RES_b_r(6, &this->registers.d); } break;
	case 0xB3: { this->ins_RES_b_r(6, &this->registers.e); } break;
	case 0xB4: { this->ins_RES_b_r(6, &this->registers.h); } break;
	case 0xB5: { this->ins_RES_b_r(6, &this->registers.l); } break;
	case 0xB6: { this->ins_RES_b_r_bHLb(6); } break;
	case 0xB7: { this->ins_RES_b_r(6, &this->registers.a); } break;

	case 0xB8: { this->ins_RES_b_r(7, &this->registers.b); } break;
	case 0xB9: { this->ins_RES_b_r(7, &this->registers.c); } break;
	case 0xBA: { this->ins_RES_b_r(7, &this->registers.d); } break;
	case 0xBB: { this->ins_RES_b_r(7, &this->registers.e); } break;
	case 0xBC: { this->ins_RES_b_r(7, &this->registers.h); } break;
	case 0xBD: { this->ins_RES_b_r(7, &this->registers.l); } break;
	case 0xBE: { this->ins_RES_b_r_bHLb(7); } break;
	case 0xBF: { this->ins_RES_b_r(7, &this->registers.a); } break;

	case 0xC0: { this->ins_SET_b_r(0, &this->registers.b); } break;
	case 0xC1: { this->ins_SET_b_r(0, &this->registers.c); } break;
	case 0xC2: { this->ins_SET_b_r(0, &this->registers.d); } break;
	case 0xC3: { this->ins_SET_b_r(0, &this->registers.e); } break;
	case 0xC4: { this->ins_SET_b_r(0, &this->registers.h); } break;
	case 0xC5: { this->ins_SET_b_r(0, &this->registers.l); } break;
	case 0xC6: { this->ins_SET_b_r_bHLb(0); } break;
	case 0xC7: { this->ins_SET_b_r(0, &this->registers.a); } break;

	case 0xC8: { this->ins_SET_b_r(1, &this->registers.b); } break;
	case 0xC9: { this->ins_SET_b_r(1, &this->registers.c); } break;
	case 0xCA: { this->ins_SET_b_r(1, &this->registers.d); } break;
	case 0xCB: { this->ins_SET_b_r(1, &this->registers.e); } break;
	case 0xCC: { this->ins_SET_b_r(1, &this->registers.h); } break;
	case 0xCD: { this->ins_SET_b_r(1, &this->registers.l); } break;
	case 0xCE: { this->ins_SET_b_r_bHLb(1); } break;
	case 0xCF: { this->ins_SET_b_r(1, &this->registers.a); } break;

	case 0xD0: { this->ins_SET_b_r(2, &this->registers.b); } break;
	case 0xD1: { this->ins_SET_b_r(2, &this->registers.c); } break;
	case 0xD2: { this->ins_SET_b_r(2, &this->registers.d); } break;
	case 0xD3: { this->ins_SET_b_r(2, &this->registers.e); } break;
	case 0xD4: { this->ins_SET_b_r(2, &this->registers.h); } break;
	case 0xD5: { this->ins_SET_b_r(2, &this->registers.l); } break;
	case 0xD6: { this->ins_SET_b_r_bHLb(2); } break;
	case 0xD7: { this->ins_SET_b_r(2, &this->registers.a); } break;

	case 0xD8: { this->ins_SET_b_r(3, &this->registers.b); } break;
	case 0xD9: { this->ins_SET_b_r(3, &this->registers.c); } break;
	case 0xDA: { this->ins_SET_b_r(3, &this->registers.d); } break;
	case 0xDB: { this->ins_SET_b_r(3, &this->registers.e); } break;
	case 0xDC: { this->ins_SET_b_r(3, &this->registers.h); } break;
	case 0xDD: { this->ins_SET_b_r(3, &this->registers.l); } break;
	case 0xDE: { this->ins_SET_b_r_bHLb(3); } break;
	case 0xDF: { this->ins_SET_b_r(3, &this->registers.a); } break;

	case 0xE0: { this->ins_SET_b_r(4, &this->registers.b); } break;
	case 0xE1: { this->ins_SET_b_r(4, &this->registers.c); } break;
	case 0xE2: { this->ins_SET_b_r(4, &this->registers.d); } break;
	case 0xE3: { this->ins_SET_b_r(4, &this->registers.e); } break;
	case 0xE4: { this->ins_SET_b_r(4, &this->registers.h); } break;
	case 0xE5: { this->ins_SET_b_r(4, &this->registers.l); } break;
	case 0xE6: { this->ins_SET_b_r_bHLb(4); } break;
	case 0xE7: { this->ins_SET_b_r(4, &this->registers.a); } break;

	case 0xE8: { this->ins_SET_b_r(5, &this->registers.b); } break;
	case 0xE9: { this->ins_SET_b_r(5, &this->registers.c); } break;
	case 0xEA: { this->ins_SET_b_r(5, &this->registers.d); } break;
	case 0xEB: { this->ins_SET_b_r(5, &this->registers.e); } break;
	case 0xEC: { this->ins_SET_b_r(5, &this->registers.h); } break;
	case 0xED: { this->ins_SET_b_r(5, &this->registers.l); } break;
	case 0xEE: { this->ins_SET_b_r_bHLb(5); } break;
	case 0xEF: { this->ins_SET_b_r(5, &this->registers.a); } break;

	case 0xF0: { this->ins_SET_b_r(6, &this->registers.b); } break;
	case 0xF1: { this->ins_SET_b_r(6, &this->registers.c); } break;
	case 0xF2: { this->ins_SET_b_r(6, &this->registers.d); } break;
	case 0xF3: { this->ins_SET_b_r(6, &this->registers.e); } break;
	case 0xF4: { this->ins_SET_b_r(6, &this->registers.h); } break;
	case 0xF5: { this->ins_SET_b_r(6, &this->registers.l); } break;
	case 0xF6: { this->ins_SET_b_r_bHLb(6); } break;
	case 0xF7: { this->ins_SET_b_r(6, &this->registers.a); } break;

	case 0xF8: { this->ins_SET_b_r(7, &this->registers.b); } break;
	case 0xF9: { this->ins_SET_b_r(7, &this->registers.c); } break;
	case 0xFA: { this->ins_SET_b_r(7, &this->registers.d); } break;
	case 0xFB: { this->ins_SET_b_r(7, &this->registers.e); } break;
	case 0xFC: { this->ins_SET_b_r(7, &this->registers.h); } break;
	case 0xFD: { this->ins_SET_b_r(7, &this->registers.l); } break;
	case 0xFE: { this->ins_SET_b_r_bHLb(7); } break;
	case 0xFF: { this->ins_SET_b_r(7, &this->registers.a); } break;
	}
}
void CPU::STOP_instruction_handler()
{

	switch (this->get_byte_from_pc())
	{
	case 0x00: {  4; } // STOP ins but there might be something more important needed to be done here
	}
}


//implemented from
// https://izik1.github.io/gbops/
// for timings of each instruction

// prefetcher info from
// https://c-sp.github.io/posts/game-boy-cpu-instruction-timing/ and
// https://gekkio.fi/files/gb-docs/gbctr.pdf

/* template
void X(const Byte* register_val, const Word data)
{
	switch (this->mCyclesUsed)
	{
	case 0:
	{
		//logic

		this->mCyclesUsed++;
		break;
	}

	case 1:
	{
		//logic

		this->mCyclesUsed++;
		break;
	}

	case 2:
	{
		//logic

		this->isExecutingInstruction = false;
		break;
	}

	}
*/

// Instructions here skip the fetch cycle as it is prefetched.

// If an instruction is 1 mCycle(s)
// This means that 
// opcode is prefetched in previous mCycle
// in mCycle 1 we perform the instruction, 
// and flag instruction as finished to start next prefetch.

// If an instruction is 2 mCycle(s)
// This means that 
// opcode is prefetched in previous mCycle
// in mCycle 1 we perform a part of the instruction,
// in mCycle 2 we perform the final of the instruction,
// and flag instruction as finished to start next prefetch.

// If an instruction is 3 mCycle(s)
// This means that 
// opcode is prefetched in previous mCycle
// in mCycle 1 we perform a part of the instruction,
// in mCycle 2 we perform a part of the instruction,
// in mCycle 3 we perform the final of the instruction,
// and flag instruction as finished to start next prefetch.


// 0x01,0x11,0x21
void CPU::ins_LD_XX_u16(Byte* const registerOne, Byte* const registerTwo)
{
	switch (this->mCyclesUsed)
	{
	case 0: this->mCyclesUsed++; break;
	case 1: *registerTwo = this->get_byte_from_pc(); this->mCyclesUsed++; break;
	case 2: *registerOne = this->get_byte_from_pc(); this->isExecutingInstruction = false;
	}
}

// 0x031
void CPU::ins_LD_SP_u16(Word* const registerWord)
{
	switch (this->mCyclesUsed)
	{
	case 0: this->mCyclesUsed++; break;
	case 1: *registerWord = (*registerWord & 0xFF00) | this->get_byte_from_pc(); this->mCyclesUsed++; break;
	case 2: *registerWord = (*registerWord & 0x00FF) | (this->get_byte_from_pc() << 0x8); this->isExecutingInstruction = false;
	}
}

// 0x02, 0x12, 0x70 - 0x75, 0x77, 0xE2
void CPU::ins_LD_bXXb_Y(const Word registerWordvalue, Byte* const registerByte)
{
	switch (this->mCyclesUsed)
	{
	case 0: this->mCyclesUsed++; break;
	case 1: this->bus->set_memory(registerWordvalue, *registerByte, MEMORY_ACCESS_TYPE::cpu); this->isExecutingInstruction = false;
	}
}

// 0x22, 0x32
void CPU::ins_LD_bHLb_Apm(bool add)
{

	switch (this->mCyclesUsed)
	{
	case 0: this->mCyclesUsed++; break;
	case 1:
	{
		Word hl = this->registers.get_HL();
		this->bus->set_memory(hl, this->registers.a, MEMORY_ACCESS_TYPE::cpu);
		add ? this->registers.set_HL(hl + 1) : this->registers.set_HL(hl - 1);
		this->isExecutingInstruction = false;
	}
	}
}

void CPU::ins_INC_XX(Byte* registerOne, Byte* registerTwo)
{
	switch (this->mCyclesUsed)
	{
	case 0:
	{
		Word newValue = this->registers.get_word(registerOne, registerTwo) + 1;
		this->instructionCache[0] = (newValue & 0xFF00) >> 8;
		*registerTwo = newValue & 0x00FF;
		this->mCyclesUsed++;
		break;
	}
	case 1:
		*registerOne = (this->instructionCache[0]);
		this->isExecutingInstruction = false;
	}
}

void CPU::ins_INC_SP()
{
	switch (this->mCyclesUsed)
	{
	case 0: this->mCyclesUsed++; break;
	case 1: this->registers.sp++; this->isExecutingInstruction = false;
	}
}

void CPU::ins_INC_X(Byte* registerByte)
{
	this->registers.set_flag(h, this->checkCarry(*registerByte, 1, 4));

	(*registerByte)++;

	(*registerByte == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
	this->registers.set_flag(n, 0);

	this->isExecutingInstruction = false;
}

void CPU::ins_INC_bHLb()
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1:  this->instructionCache[0] = this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu); this->mCyclesUsed++; break;
	case 2:
		Word address = this->registers.get_HL();
		Byte temp = this->instructionCache[0];

		this->bus->set_memory(address, (temp + 1), MEMORY_ACCESS_TYPE::cpu);

		this->registers.set_flag(h, this->checkCarry(temp, 1, 4));
		this->registers.set_flag(z, (this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu) == 0x0));
		this->registers.set_flag(n, 0);

		this->isExecutingInstruction = false;
	}
}

void CPU::ins_DEC_X(Byte* registerByte)
{
	this->registers.set_flag(h, this->checkBorrow(*registerByte, 1, 4));
	(*registerByte)--;

	this->registers.set_flag(z, (*registerByte == 0x0));
	this->registers.set_flag(n, 1);

	this->isExecutingInstruction = false;
}

void CPU::ins_DEC_bHLb()
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1:  this->mCyclesUsed++; break;
	case 2:
		Word address = this->registers.get_HL();
		Byte temp = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);

		this->registers.set_flag(h, this->checkBorrow(temp, 1, 4));
		this->bus->set_memory(address, (temp - 1), MEMORY_ACCESS_TYPE::cpu);
		this->registers.set_flag(z, (this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu) == 0x0));
		this->registers.set_flag(n, 1);

		this->isExecutingInstruction = false;
	}
}

void CPU::ins_LD_X_u8(Byte* const registerByte)
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1: *registerByte = this->get_byte_from_pc(); this->isExecutingInstruction = false;
	}
}

// 0x36
void CPU::ins_LD_bHLb_u8()
{
	switch (this->mCyclesUsed)
	{
	case 0: this->mCyclesUsed++; break;
	case 1: this->mCyclesUsed++; break;
	case 2:
		this->bus->set_memory(this->registers.get_HL(), this->get_byte_from_pc(), MEMORY_ACCESS_TYPE::cpu);
		this->isExecutingInstruction = false;
	}
}


void CPU::ins_CPL()
{
	this->registers.a = ~this->registers.a;
	this->registers.set_flag(n, true);
	this->registers.set_flag(h, true);
	this->isExecutingInstruction = false;
}

void CPU::ins_CCF()
{
	this->registers.set_flag(n, 0);
	this->registers.set_flag(h, 0);
	this->registers.set_flag(c, !this->registers.get_flag(c));
	this->isExecutingInstruction = false;
}

void CPU::ins_SCF()
{
	this->registers.set_flag(n, 0);
	this->registers.set_flag(h, 0);
	this->registers.set_flag(c, 1);
	this->isExecutingInstruction = false;
}

void CPU::ins_RLCA()
{
	//set c flag to whatever is the value of the leftest bit from register a
	this->registers.set_flag(c, (this->registers.a & 0x80));
	this->registers.set_flag(n, 0);
	this->registers.set_flag(h, 0);

	// move everything to the left by one, toggle bit 0 with bit 7 shifted right 7 places
	this->registers.a = (this->registers.a << 1) | (this->registers.a >> (7));

	// z is reset
	this->registers.set_flag(z, 0);

	this->isExecutingInstruction = false;
}

void CPU::ins_RLA()
{
	// swap leftest most bit with the carry flag, then rotate to the left

	Byte flagCarry = this->registers.get_flag(c);
	bool registerCarry = ((this->registers.a & 0x80) >> 7);

	this->registers.a = (this->registers.a << 1) | (flagCarry);

	this->registers.set_flag(c, registerCarry);
	this->registers.set_flag(n, 0);
	this->registers.set_flag(h, 0);
	this->registers.set_flag(z, 0);


	this->isExecutingInstruction = false;
}

void CPU::ins_RRCA()
{
	this->registers.set_flag(c, this->registers.a & 0x1);

	this->registers.a = (this->registers.a >> 1) | (this->registers.a << (7));

	this->registers.set_flag(n, 0);
	this->registers.set_flag(h, 0);
	this->registers.set_flag(z, 0);


	this->isExecutingInstruction = false;
}

void CPU::ins_RRA()
{
	Byte flagCarry = this->registers.get_flag(c);
	bool registerCarry = (this->registers.a & 0x1);

	this->registers.a = (this->registers.a >> 1) | (flagCarry << (7));

	this->registers.set_flag(c, registerCarry);
	this->registers.set_flag(n, 0);
	this->registers.set_flag(h, 0);
	this->registers.set_flag(z, 0);


	this->isExecutingInstruction = false;
}

//0x08
void CPU::ins_LD_bu16b_SP()
{
	switch (this->mCyclesUsed)
	{
	case 0: this->mCyclesUsed++; break;
	case 1: this->instructionCache[0] = this->get_byte_from_pc(); this->mCyclesUsed++; break;
	case 2: this->instructionCache[1] = this->get_byte_from_pc(); this->mCyclesUsed++; break;
	case 3:
	{
		Word address = (this->instructionCache[1] << 8) | this->instructionCache[0];
		this->bus->set_memory(address, (this->registers.sp & 0x00FF), MEMORY_ACCESS_TYPE::cpu);
		this->mCyclesUsed++; break;
	}
	case 4:
	{
		Word address = (this->instructionCache[1] << 8) | this->instructionCache[0];
		this->bus->set_memory(address + 1, (this->registers.sp & 0xFF00 >> 8), MEMORY_ACCESS_TYPE::cpu);
		this->isExecutingInstruction = false;
	}
	}
}

// 0x18
void CPU::ins_JR_i8(const enum JumpCondition condition)
{
	// DEBUG
	if (this->registers.get_flag(z))
		printf("");
	//
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1:
		this->instructionCache[0] = this->get_byte_from_pc();
		checkJumpCondition(condition) ? this->mCyclesUsed++ : this->isExecutingInstruction = false;
		break;
	case 2:
		this->registers.pc += (Byte_s)this->instructionCache[0];
		this->isExecutingInstruction = false;
	}
}

void CPU::ins_ADD_HL_XX(Byte* const registerOne, Byte* const registerTwo)
{
	switch (this->mCyclesUsed)
	{
	case 0:
	{
		Word HLvalue = this->registers.get_HL();
		Word registerWord = (*registerOne << 8) | *registerTwo;
		this->registers.set_flag(c, this->checkCarry(HLvalue, registerWord, 16));
		this->registers.set_flag(h, this->checkCarry(HLvalue, registerWord, 12));
		this->registers.set_flag(n, 0);
		this->registers.l = *registerTwo;

		this->mCyclesUsed++;
		break;
	}
	case 1:
		this->registers.h = *registerOne;
		this->isExecutingInstruction = false;
	}
}

void CPU::ins_ADD_HL_SP()
{
	switch (this->mCyclesUsed)
	{
	case 0:
	{
		Word HLvalue = this->registers.get_HL();
		this->registers.set_flag(c, this->checkCarry(HLvalue, this->registers.sp, 16));
		this->registers.set_flag(h, this->checkCarry(HLvalue, this->registers.sp, 12));
		this->registers.set_flag(n, 0);
		this->registers.l = this->registers.sp & 0x00FF;

		this->mCyclesUsed++; break;
	}
	case 1:
		this->registers.h = (this->registers.sp & 0xFF00) >> 8;
		this->isExecutingInstruction = false;
	}
}

void CPU::ins_ADD_A_X(Byte* const registerByte)
{
	this->registers.set_flag(c, this->checkCarry(this->registers.a, *registerByte, 8));
	this->registers.set_flag(h, this->checkCarry(this->registers.a, *registerByte, 4));

	// perform addition
	this->registers.a += *registerByte;

	//evaluate z flag an clear the n flag
	this->registers.set_flag(z, (this->registers.a == 0x0));
	this->registers.set_flag(n, 0);

	this->isExecutingInstruction = false;
}

void CPU::ins_ADD_A_bHLb()
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1:
		Byte immediateValue = this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu);

		this->registers.set_flag(c, this->checkCarry(this->registers.a, immediateValue, 8));
		this->registers.set_flag(h, this->checkCarry(this->registers.a, immediateValue, 4));

		// perform addition
		this->registers.a += immediateValue;

		//evaluate z flag an clear the n flag
		this->registers.set_flag(z, (this->registers.a == 0x0));
		this->registers.set_flag(n, 0);

		this->isExecutingInstruction = false;
	}
}

void CPU::ins_LD_X_Y(Byte* const registerOne, Byte* const registerTwo)
{
	*registerOne = *registerTwo;
	this->isExecutingInstruction = false;
}

void CPU::ins_LD_X_bYYb(Byte* const leftRegister, Byte* const rightRegisterOne, Byte* const rightRegisterTwo, const Byte_s addToHL)
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1:
		*leftRegister = this->bus->get_memory((*rightRegisterOne << 8) | *rightRegisterTwo, MEMORY_ACCESS_TYPE::cpu);
		if (addToHL != NULL)
			this->registers.set_HL(this->registers.get_HL() + addToHL);
		this->isExecutingInstruction = false;
	}
}

void CPU::ins_DEC_XX(Byte* registerOne, Byte* registerTwo)
{
	switch (this->mCyclesUsed)
	{
	case 0:
		this->registers.set_word(registerOne, registerTwo, (this->registers.get_word(registerOne, registerTwo) - 1));
		this->mCyclesUsed++; break;
	case 1: this->isExecutingInstruction = false;
	}
}

void CPU::ins_DEC_SP()
{
	switch (this->mCyclesUsed)
	{
	case 0: this->registers.sp--; this->mCyclesUsed++; break;
	case 1:  this->isExecutingInstruction = false;
	}
}

void CPU::ins_ADC_A_X(const Byte* registerByte)
{
	Byte a = this->registers.a;
	Byte b = *registerByte;
	Byte C = (int)this->registers.get_flag(c);

	this->registers.a = a + b + C;

	this->registers.set_flag(z, (this->registers.a == 0x0));
	this->registers.set_flag(n, 0);
	this->registers.set_flag(c, this->checkCarry(a, b, 8, C));
	this->registers.set_flag(h, this->checkCarry(a, b, 4, C));

	this->isExecutingInstruction = false;
}

void CPU::ins_ADC_A_bHLb()
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1:
		Byte a = this->registers.a;
		Byte b = this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu);
		Byte C = (int)this->registers.get_flag(c);

		this->registers.a = a + b + C;

		this->registers.set_flag(z, (this->registers.a == 0x0));
		this->registers.set_flag(n, 0);
		this->registers.set_flag(c, this->checkCarry(a, b, 8, C));
		this->registers.set_flag(h, this->checkCarry(a, b, 4, C));

		this->isExecutingInstruction = false;
	}
}

void CPU::ins_ADC_A_u8()
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1:
		Byte a = this->registers.a;
		Byte b = this->get_byte_from_pc();
		Byte C = (int)this->registers.get_flag(c);

		this->registers.a = a + b + C;

		this->registers.set_flag(z, (this->registers.a == 0x0));
		this->registers.set_flag(n, 0);
		this->registers.set_flag(c, this->checkCarry(a, b, 8, C));
		this->registers.set_flag(h, this->checkCarry(a, b, 4, C));

		this->isExecutingInstruction = false;
	}
}


void CPU::ins_SUB_A_X(Byte* const registerByte)
{
	this->registers.set_flag(c, this->checkBorrow(this->registers.a, *registerByte, 8));
	this->registers.set_flag(h, this->checkBorrow(this->registers.a, *registerByte, 4));

	// perform addition
	this->registers.a -= *registerByte;

	//evaluate z flag an clear the n flag
	this->registers.set_flag(z, (this->registers.a == 0x0));
	this->registers.set_flag(n, 1);

	this->isExecutingInstruction = false;
}

void CPU::ins_SUB_A_u8()
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1:

		Byte immediateByte = this->get_byte_from_pc();

		this->registers.set_flag(c, this->checkBorrow(this->registers.a, immediateByte, 8));
		this->registers.set_flag(h, this->checkBorrow(this->registers.a, immediateByte, 4));

		// perform addition
		this->registers.a -= immediateByte;

		//evaluate z flag an clear the n flag
		this->registers.set_flag(z, (this->registers.a == 0x0));
		this->registers.set_flag(n, 1);

		this->isExecutingInstruction = false;
	}
}


void CPU::ins_SUB_A_bHLb()
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1:

		Byte immediateByte = this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu);

		this->registers.set_flag(c, this->checkBorrow(this->registers.a, immediateByte, 8));
		this->registers.set_flag(h, this->checkBorrow(this->registers.a, immediateByte, 4));

		// perform addition
		this->registers.a -= immediateByte;

		//evaluate z flag an clear the n flag
		this->registers.set_flag(z, (this->registers.a == 0x0));
		this->registers.set_flag(n, 1);
		this->isExecutingInstruction = false;
	}
}


void CPU::ins_SBC_A_X(Byte* const registerByte)
{
	Byte a = this->registers.a;
	Byte b = *registerByte;
	Byte C = (int)this->registers.get_flag(c);

	this->registers.a = a - b - C;

	this->registers.set_flag(z, (this->registers.a == 0x0));
	this->registers.set_flag(n, 1);
	this->registers.set_flag(c, this->checkBorrow(a, b, 8, C));
	this->registers.set_flag(h, this->checkBorrow(a, b, 4, C));


	this->isExecutingInstruction = false;
}

void CPU::ins_SBC_A_u8()
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1:
		Byte a = this->registers.a;
		Byte b = this->get_byte_from_pc();
		Byte C = (int)this->registers.get_flag(c);

		this->registers.a = a - b - C;

		this->registers.set_flag(z, (this->registers.a == 0x0));
		this->registers.set_flag(n, 1);
		this->registers.set_flag(c, this->checkBorrow(a, b, 8, C));
		this->registers.set_flag(h, this->checkBorrow(a, b, 4, C));
		this->isExecutingInstruction = false;
	}
}


void CPU::ins_SBC_A_bHLb()
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1:
		Byte a = this->registers.a;
		Byte b = this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu);
		Byte C = (int)this->registers.get_flag(c);

		this->registers.a = a - b - C;

		this->registers.set_flag(z, (this->registers.a == 0x0));
		this->registers.set_flag(n, 1);
		this->registers.set_flag(c, this->checkBorrow(a, b, 8, C));
		this->registers.set_flag(h, this->checkBorrow(a, b, 4, C));

		this->isExecutingInstruction = false;
	}
}

void CPU::ins_AND_A_X(Byte* const registerByte)
{
	this->registers.a &= *registerByte;

	//evaluate z flag an clear the n flag
	this->registers.set_flag(z, (this->registers.a == 0x0));
	this->registers.set_flag(h, 1);
	this->registers.set_flag(n, 0);
	this->registers.set_flag(c, 0);

	this->isExecutingInstruction = false;
}

void CPU::ins_AND_A_u8()
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1:
		this->registers.a &= this->get_byte_from_pc();

		//evaluate z flag an clear the n flag
		this->registers.set_flag(z, (this->registers.a == 0x0));
		this->registers.set_flag(h, 1);
		this->registers.set_flag(n, 0);
		this->registers.set_flag(c, 0);

		this->isExecutingInstruction = false;
	}
}


void CPU::ins_AND_A_bHLb()
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1:
		this->registers.a &= this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu);

		//evaluate z flag an clear the n flag
		this->registers.set_flag(z, (this->registers.a == 0x0));
		this->registers.set_flag(h, 1);
		this->registers.set_flag(n, 0);
		this->registers.set_flag(c, 0);

		this->isExecutingInstruction = false;
	}
}

void CPU::ins_XOR_A_X(Byte* const registerByte)
{
	this->registers.a ^= *registerByte;

	//evaluate z flag an clear the n flag
	this->registers.set_flag(z, (this->registers.a == 0x0));
	this->registers.set_flag(h, 0);
	this->registers.set_flag(n, 0);
	this->registers.set_flag(c, 0);

	this->isExecutingInstruction = false;
}

void CPU::ins_XOR_A_u8()
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1:
		this->registers.a ^= this->get_byte_from_pc();

		//evaluate z flag an clear the n flag
		this->registers.set_flag(z, (this->registers.a == 0x0));
		this->registers.set_flag(h, 0);
		this->registers.set_flag(n, 0);
		this->registers.set_flag(c, 0);

		this->isExecutingInstruction = false;
	}
}


void CPU::ins_XOR_A_bHLb()
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1:
		this->registers.a ^= this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu);

		//evaluate z flag an clear the n flag
		this->registers.set_flag(z, (this->registers.a == 0x0));
		this->registers.set_flag(h, 0);
		this->registers.set_flag(n, 0);
		this->registers.set_flag(c, 0);

		this->isExecutingInstruction = false;
	}
}

void CPU::ins_OR_A_X(Byte* const registerByte)
{
	this->registers.a |= *registerByte;

	//evaluate z flag an clear the n flag
	this->registers.set_flag(z, (this->registers.a == 0x0));
	this->registers.set_flag(h, 0);
	this->registers.set_flag(n, 0);
	this->registers.set_flag(c, 0);


	this->isExecutingInstruction = false;
}

void CPU::ins_OR_A_u8()
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1:
		this->registers.a |= this->get_byte_from_pc();

		//evaluate z flag an clear the n flag
		this->registers.set_flag(z, (this->registers.a == 0x0));
		this->registers.set_flag(h, 0);
		this->registers.set_flag(n, 0);
		this->registers.set_flag(c, 0);

		this->isExecutingInstruction = false;
	}
}


void CPU::ins_OR_A_bHLb()
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1:
		this->registers.a |= this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu);

		//evaluate z flag an clear the n flag
		this->registers.set_flag(z, (this->registers.a == 0x0));
		this->registers.set_flag(h, 0);
		this->registers.set_flag(n, 0);
		this->registers.set_flag(c, 0);

		this->isExecutingInstruction = false;
	}
}

void CPU::ins_CP_A_X(Byte* const registerByte)
{
	this->registers.set_flag(c, this->checkBorrow(this->registers.a, *registerByte, 8));
	this->registers.set_flag(h, this->checkBorrow(this->registers.a, *registerByte, 4));

	//evaluate z flag an clear the n flag
	(this->registers.a == *registerByte) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
	this->registers.set_flag(n, 1);


	this->isExecutingInstruction = false;
}

void CPU::ins_CP_A_u8()
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1:
		Byte immediateValue = this->get_byte_from_pc();
		this->registers.set_flag(c, this->checkBorrow(this->registers.a, immediateValue, 8));
		this->registers.set_flag(h, this->checkBorrow(this->registers.a, immediateValue, 4));
		(this->registers.a == immediateValue) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
		this->registers.set_flag(n, 1);

		this->isExecutingInstruction = false;
	}
}


void CPU::ins_CP_A_bHLb()
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1:
		Byte immediateValue = this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu);
		this->registers.set_flag(c, this->checkBorrow(this->registers.a, immediateValue, 8));
		this->registers.set_flag(h, this->checkBorrow(this->registers.a, immediateValue, 4));
		(this->registers.a == immediateValue) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
		this->registers.set_flag(n, 1);

		this->isExecutingInstruction = false;
	}
}


void CPU::ins_POP_XX(Byte* registerOne, Byte* registerTwo)
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1:
		*registerTwo = this->bus->get_memory(this->registers.sp++, MEMORY_ACCESS_TYPE::cpu);
		if (registerTwo == &this->registers.f)
			*registerTwo &= 0xF0;


		this->mCyclesUsed++; break;
	case 2:
		*registerOne = this->bus->get_memory(this->registers.sp++, MEMORY_ACCESS_TYPE::cpu);
		if (registerOne == &this->registers.f)
			*registerOne &= 0xF0;

		this->isExecutingInstruction = false;
	}
}


void CPU::ins_PUSH_XX(const Word wordRegisterValue)
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1:  this->mCyclesUsed++; break;
	case 2:
		// move low byte to higher (sp)
		this->bus->set_memory(--this->registers.sp, (wordRegisterValue >> 8), MEMORY_ACCESS_TYPE::cpu);

		this->mCyclesUsed++; break;
	case 3:
		// move high byte to lower (sp)
		this->bus->set_memory(--this->registers.sp, (wordRegisterValue & 0xff), MEMORY_ACCESS_TYPE::cpu);

		//for (int i = 0; i > -10; i-=2)
		//{
		//	printf("%x %x%x\n", (0xE000 + i) -1, this->bus->get_memory(0xE000 + i, MEMORY_ACCESS_TYPE::debug), this->bus->get_memory((0xE000 + i) -1, MEMORY_ACCESS_TYPE::debug));
		//}
		//printf("%x\n", this->bus->work_ram[0xdffe - WORKRAMOFFSET]);
		//printf("%x\n", this->bus->work_ram[0xdffd - WORKRAMOFFSET]);

		this->isExecutingInstruction = false;
	}
}



void CPU::ins_JP_u16(const enum JumpCondition condition)
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1: this->instructionCache[0] = this->get_byte_from_pc(); this->mCyclesUsed++; break;
	case 2: this->instructionCache[1] = this->get_byte_from_pc();
		checkJumpCondition(condition) ? this->mCyclesUsed++ : this->isExecutingInstruction = false;
		break;
	case 3:
		this->registers.pc = (instructionCache[1] << 8) | instructionCache[0];
		this->isExecutingInstruction = false;
	}
}

//this and standard RET (and by extension RETI) are completely different, one would expect it to be 3m-4m but instead it is 2m-5m, which is longer than RET's 4m, for this reason I will have to treat it as a different instruction.
void CPU::ins_RET_CC(const enum JumpCondition condition)
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1:  checkJumpCondition(condition) ? this->mCyclesUsed++ : this->isExecutingInstruction = false; break;
	case 2: this->instructionCache[0] = this->bus->get_memory(this->registers.sp++, MEMORY_ACCESS_TYPE::cpu); this->mCyclesUsed++; break;
	case 3: this->instructionCache[1] = this->bus->get_memory(this->registers.sp++, MEMORY_ACCESS_TYPE::cpu); this->mCyclesUsed++; break;
	case 4: this->registers.pc = (instructionCache[1] << 8) | instructionCache[0]; this->isExecutingInstruction = false;
	}
}

void CPU::ins_RET()
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1: this->instructionCache[0] = this->bus->get_memory(this->registers.sp++, MEMORY_ACCESS_TYPE::cpu); this->mCyclesUsed++; break;
	case 2: this->instructionCache[1] = this->bus->get_memory(this->registers.sp++, MEMORY_ACCESS_TYPE::cpu); this->mCyclesUsed++; break;
	case 3: this->registers.pc = (instructionCache[1] << 8) | instructionCache[0]; this->isExecutingInstruction = false;
	}
}

void CPU::ins_RETI()
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1: this->instructionCache[0] = this->bus->get_memory(this->registers.sp++, MEMORY_ACCESS_TYPE::cpu); this->mCyclesUsed++; break;
	case 2: this->instructionCache[1] = this->bus->get_memory(this->registers.sp++, MEMORY_ACCESS_TYPE::cpu); this->mCyclesUsed++; break;
	case 3: this->registers.pc = (instructionCache[1] << 8) | instructionCache[0]; this->interrupt_master_enable = 1; this->isExecutingInstruction = false;
	}
}



void CPU::ins_JP_HL()
{
	this->registers.pc = this->registers.get_HL();
	this->isExecutingInstruction = false;
}

// call is correct
void CPU::ins_CALL_u16(const enum JumpCondition condition)
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1: this->instructionCache[0] = this->get_byte_from_pc(); this->mCyclesUsed++; break;
	case 2: this->instructionCache[1] = this->get_byte_from_pc(); this->mCyclesUsed++; break;
	case 3: checkJumpCondition(condition) ? this->mCyclesUsed++ : this->isExecutingInstruction = false; break;
	case 4:
		this->bus->set_memory(--this->registers.sp, this->registers.get_lowByte(&this->registers.pc), MEMORY_ACCESS_TYPE::cpu);
		this->mCyclesUsed++; break;
	case 5:
		this->bus->set_memory(--this->registers.sp, this->registers.get_highByte(&this->registers.pc), MEMORY_ACCESS_TYPE::cpu);
		this->registers.pc = (instructionCache[1] << 8) | instructionCache[0];
		this->isExecutingInstruction = false;
	}
}


void CPU::ins_RST(Byte jumpVector)
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1:  this->mCyclesUsed++; break;
	case 2:
		this->bus->set_memory(--this->registers.sp, this->registers.get_lowByte(&this->registers.pc), MEMORY_ACCESS_TYPE::cpu);
		this->mCyclesUsed++; break;
	case 3:
		this->bus->set_memory(--this->registers.sp, this->registers.get_highByte(&this->registers.pc), MEMORY_ACCESS_TYPE::cpu);
		this->registers.pc = 0x0000 + jumpVector;
		this->isExecutingInstruction = false;
	}
}

void CPU::ins_LD_bFF00_u8b_A()
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1: this->instructionCache[0] = this->get_byte_from_pc(); this->mCyclesUsed++; break;
	case 2:
		this->bus->set_memory(0xFF00 + this->instructionCache[0], this->registers.a, MEMORY_ACCESS_TYPE::cpu);
		this->isExecutingInstruction = false;
	}
}

void CPU::ins_LD_A_bFF00_u8b()
{
	switch (this->mCyclesUsed)
	{
	case 0: this->mCyclesUsed++; break;
	case 1: this->registers.a = this->bus->get_memory(0xFF00 + this->get_byte_from_pc(), MEMORY_ACCESS_TYPE::cpu); this->isExecutingInstruction = false;
	}
}

void CPU::ins_ADD_SP_i8()
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1: this->instructionCache[0] = this->get_byte_from_pc(); this->mCyclesUsed++; break;
	case 2:
	{
		Byte_s value = (Byte_s)this->instructionCache[0];
		if (value < 0)
		{
			this->registers.set_flag(c, ((this->registers.sp + value) & 0xFF) <= (this->registers.sp & 0xFF));
			this->registers.set_flag(h, ((this->registers.sp + value) & 0xF) <= (this->registers.sp & 0xF));
		}
		else
		{
			this->registers.set_flag(c, ((this->registers.sp & 0xFF) + value) > 0xFF);
			this->registers.set_flag(h, ((this->registers.sp & 0xF) + (value & 0xF)) > 0xF);
		}
		this->registers.set_flag(z, 0);
		this->registers.set_flag(n, 0);

		instructionCache[1] = (this->registers.sp + value) >> 8;
		this->registers.set_lowByte(&this->registers.sp, instructionCache[1]);

		this->mCyclesUsed++; break;
	}
	case 3:
		this->registers.set_highByte(&this->registers.sp, instructionCache[1]);
		this->isExecutingInstruction = false;
	}
}

void CPU::ins_LD_HL_SP_i8()
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1: this->instructionCache[0] = this->get_byte_from_pc(); this->mCyclesUsed++; break;
	case 2:
		Byte_s value = (Byte_s)this->instructionCache[0];
		Word sum = this->registers.sp + value;
		this->registers.set_flag(z, 0);
		this->registers.set_flag(n, 0);
		if (value < 0)
		{
			this->registers.set_flag(c, (sum & 0xFF) <= (this->registers.sp & 0xFF));
			this->registers.set_flag(h, (sum & 0xF) <= (this->registers.sp & 0xF));
		}
		else
		{
			this->registers.set_flag(c, ((this->registers.sp & 0xFF) + value) > 0xFF);
			this->registers.set_flag(h, ((this->registers.sp & 0xF) + (value & 0xF)) > 0xF);
		}
		this->registers.set_HL(sum);

		this->isExecutingInstruction = false;
	}
}

void CPU::ins_LD_SP_HL()
{
	switch (this->mCyclesUsed)
	{
	case 0: this->mCyclesUsed++; break;
	case 1: this->registers.sp = this->registers.get_HL(); this->isExecutingInstruction = false;
	}
}

void CPU::ins_LD_bu16u_A()
{
	switch (this->mCyclesUsed)
	{
	case 0: this->mCyclesUsed++; break;
	case 1: this->instructionCache[0] = this->get_byte_from_pc(); this->mCyclesUsed++; break;
	case 2: this->instructionCache[1] = this->get_byte_from_pc(); this->mCyclesUsed++; break;
	case 3:
		this->bus->set_memory((this->instructionCache[1] << 8) | this->instructionCache[0], this->registers.a, MEMORY_ACCESS_TYPE::cpu);
		this->isExecutingInstruction = false;
	}
}

void CPU::ins_LD_A_bu16u()
{
	switch (this->mCyclesUsed)
	{
	case 0: this->mCyclesUsed++; break;
	case 1: this->instructionCache[0] = this->get_byte_from_pc(); this->mCyclesUsed++; break;
	case 2: this->instructionCache[1] = this->get_byte_from_pc(); this->mCyclesUsed++; break;
	case 3:
	{
		Word address = (this->instructionCache[1] << 8) | this->instructionCache[0];
		this->registers.a = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);
		this->isExecutingInstruction = false;
	}
	}
}


void CPU::ins_RLC(Byte* registerOne)
{
	this->registers.set_flag(n, 0);
	this->registers.set_flag(h, 0);
	this->registers.set_flag(c, ((*registerOne & 0x80) >> 7));
	*registerOne = ((*registerOne << 1) | (*registerOne >> 7));

	this->registers.set_flag(z, (*registerOne == 0));

	this->isExecutingCB = false;
	this->isExecutingInstruction = false;

}

void CPU::ins_RLC_bHLb()
{
	switch (this->mCyclesUsed)
	{
	case 0: this->mCyclesUsed++; break;
	case 1: this->instructionCache[0] = this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu); this->mCyclesUsed++; break;
	case 2:

		this->registers.set_flag(n, 0);
		this->registers.set_flag(h, 0);
		Byte temp = this->instructionCache[0];
		Byte result = ((temp << 1) | (temp >> 7));
		this->registers.set_flag(c, ((temp & 0x80) >> 7));
		this->bus->set_memory(this->registers.get_HL(), result, MEMORY_ACCESS_TYPE::cpu);
		this->registers.set_flag(z, (result == 0));

		this->isExecutingCB = false;
		this->isExecutingInstruction = false;
	}
}

void CPU::ins_RL(Byte* registerOne)
{
	Byte flagCarry = this->registers.get_flag(c);
	bool newCarry = 0;

	this->registers.set_flag(n, 0);
	this->registers.set_flag(h, 0);

	newCarry = ((*registerOne & 0x80) >> 7);
	*registerOne = ((*registerOne << 1) | (flagCarry));

	this->registers.set_flag(c, newCarry);
	this->registers.set_flag(z, (*registerOne == 0));

	this->isExecutingCB = false;
	this->isExecutingInstruction = false;

}

void CPU::ins_RL_bHLb()
{
	switch (this->mCyclesUsed)
	{
	case 0: this->mCyclesUsed++; break;
	case 1: this->instructionCache[0] = this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu); this->mCyclesUsed++; break;
	case 2:

		Byte flagCarry = this->registers.get_flag(c);
		bool newCarry = 0;

		this->registers.set_flag(n, 0);
		this->registers.set_flag(h, 0);

		Byte temp = this->instructionCache[0];
		Byte result = ((temp << 1) | (flagCarry));
		newCarry = ((temp & 0x80) >> 7);
		this->bus->set_memory(this->registers.get_HL(), result, MEMORY_ACCESS_TYPE::cpu);

		this->registers.set_flag(c, newCarry);
		this->registers.set_flag(z, (result == 0));

		this->isExecutingCB = false;
		this->isExecutingInstruction = false;
	}
}


void CPU::ins_RRC(Byte* registerOne)
{
	this->registers.set_flag(n, 0);
	this->registers.set_flag(h, 0);
	this->registers.set_flag(c, (*registerOne & 0x1));
	*registerOne = ((*registerOne >> 1) | (*registerOne << 7));

	this->registers.set_flag(z, (*registerOne == 0));

	this->isExecutingCB = false;
	this->isExecutingInstruction = false;

}

void CPU::ins_RRC_bHLb()
{
	switch (this->mCyclesUsed)
	{
	case 0: this->mCyclesUsed++; break;
	case 1: this->instructionCache[0] = this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu); this->mCyclesUsed++; break;
	case 2:

		this->registers.set_flag(n, 0);
		this->registers.set_flag(h, 0);
		Byte temp = this->instructionCache[0];
		Byte result = ((temp >> 1) | (temp << 7));
		this->registers.set_flag(c, (temp & 0x1));
		this->bus->set_memory(this->registers.get_HL(), result, MEMORY_ACCESS_TYPE::cpu);
		this->registers.set_flag(z, (result == 0));

		this->isExecutingCB = false;
		this->isExecutingInstruction = false;
	}
}

void CPU::ins_RR(Byte* registerOne)
{
	Byte flagCarry = this->registers.get_flag(c);
	bool newCarry = 0;

	this->registers.set_flag(n, 0);
	this->registers.set_flag(h, 0);

	newCarry = (*registerOne & 0x1);
	*registerOne = ((*registerOne >> 1) | (flagCarry << 7));

	this->registers.set_flag(c, newCarry);
	this->registers.set_flag(z, (*registerOne == 0));

	this->isExecutingCB = false;
	this->isExecutingInstruction = false;

}

void CPU::ins_RR_bHLb()
{
	switch (this->mCyclesUsed)
	{
	case 0: this->mCyclesUsed++; break;
	case 1: this->instructionCache[0] = this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu); this->mCyclesUsed++; break;
	case 2:

		Byte flagCarry = this->registers.get_flag(c);
		bool newCarry = 0;

		this->registers.set_flag(n, 0);
		this->registers.set_flag(h, 0);

		Byte temp = this->instructionCache[0];
		Byte result = ((temp >> 1) | (flagCarry << 7));
		newCarry = (temp & 0x01);
		this->bus->set_memory(this->registers.get_HL(), result, MEMORY_ACCESS_TYPE::cpu);

		this->registers.set_flag(c, newCarry);
		this->registers.set_flag(z, (result == 0));

		this->isExecutingCB = false;
		this->isExecutingInstruction = false;
	}
}

void CPU::ins_SLA(Byte* registerOne)
{
	this->registers.set_flag(n, 0);
	this->registers.set_flag(h, 0);

	this->registers.set_flag(c, (*registerOne & 0x80) >> 7);
	*registerOne = *registerOne << 1;
	this->registers.set_flag(z, (*registerOne == 0));

	this->isExecutingCB = false;
	this->isExecutingInstruction = false;

}

void CPU::ins_SLA_bHLb()
{
	switch (this->mCyclesUsed)
	{
	case 0: this->mCyclesUsed++; break;
	case 1: this->instructionCache[0] = this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu); this->mCyclesUsed++; break;
	case 2:

		this->registers.set_flag(n, 0);
		this->registers.set_flag(h, 0);

		Byte temp = this->instructionCache[0];
		Byte result = temp << 1;

		this->registers.set_flag(c, (temp & 0x80) >> 7);
		this->bus->set_memory(this->registers.get_HL(), result, MEMORY_ACCESS_TYPE::cpu);

		this->registers.set_flag(z, (result == 0));

		this->isExecutingCB = false;
		this->isExecutingInstruction = false;
	}
}

void CPU::ins_SRA(Byte* registerOne)
{
	this->registers.set_flag(n, 0);
	this->registers.set_flag(h, 0);
	this->registers.set_flag(c, *registerOne & 0x1);
	Byte bit7 = *registerOne >> 7;

	*registerOne = (*registerOne >> 1) | (bit7 << 7);
	this->registers.set_flag(z, (*registerOne == 0));

	this->isExecutingCB = false;
	this->isExecutingInstruction = false;

}

void CPU::ins_SRA_bHLb()
{
	switch (this->mCyclesUsed)
	{
	case 0: this->mCyclesUsed++; break;
	case 1: this->instructionCache[0] = this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu); this->mCyclesUsed++; break;
	case 2:

		this->registers.set_flag(n, 0);
		this->registers.set_flag(h, 0);

		Byte temp = this->instructionCache[0];
		this->registers.set_flag(c, temp & 0x1);
		Byte bit7 = temp >> 7;

		Byte result = (temp >> 1) | (bit7 << 7);
		this->bus->set_memory(this->registers.get_HL(), result, MEMORY_ACCESS_TYPE::cpu);

		this->registers.set_flag(z, (result == 0));

		this->isExecutingCB = false;
		this->isExecutingInstruction = false;
	}
}

void CPU::ins_SRL(Byte* registerOne)
{
	this->registers.set_flag(n, 0);
	this->registers.set_flag(h, 0);
	this->registers.set_flag(c, *registerOne & 0x1);
	*registerOne = *registerOne >> 1;
	this->registers.set_flag(z, (*registerOne == 0));

	this->isExecutingCB = false;
	this->isExecutingInstruction = false;

}

void CPU::ins_SRL_bHLb()
{
	switch (this->mCyclesUsed)
	{
	case 0: this->mCyclesUsed++; break;
	case 1: this->instructionCache[0] = this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu); this->mCyclesUsed++; break;
	case 2:

		this->registers.set_flag(n, 0);
		this->registers.set_flag(h, 0);
		Byte temp = this->instructionCache[0];
		Byte result = temp >> 1;
		this->registers.set_flag(c, temp & 0x1);
		this->bus->set_memory(this->registers.get_HL(), result, MEMORY_ACCESS_TYPE::cpu);
		this->registers.set_flag(z, (result == 0));

		this->isExecutingCB = false;
		this->isExecutingInstruction = false;
	}
}

void CPU::ins_SWAP(Byte* registerOne)
{
	*registerOne = (this->get_nibble(*registerOne, false) << 4) + this->get_nibble(*registerOne, true);
	this->registers.set_flag(z, (*registerOne == 0x0));
	this->registers.set_flag(n, false);
	this->registers.set_flag(h, false);
	this->registers.set_flag(c, false);

	this->isExecutingCB = false;
	this->isExecutingInstruction = false;

}

void CPU::ins_SWAP_bHLb()
{
	switch (this->mCyclesUsed)
	{
	case 0: this->mCyclesUsed++; break;
	case 1: this->instructionCache[0] = this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu); this->mCyclesUsed++; break;
	case 2:

		Byte temp = this->instructionCache[0];
		Byte swappedTemp = (this->get_nibble(temp, false) << 4) + this->get_nibble(temp, true);
		this->bus->set_memory(this->registers.get_HL(), swappedTemp, MEMORY_ACCESS_TYPE::cpu);
		this->registers.set_flag(z, (swappedTemp == 0x0));
		this->registers.set_flag(n, false);
		this->registers.set_flag(h, false);
		this->registers.set_flag(c, false);

		this->isExecutingCB = false;
		this->isExecutingInstruction = false;
	}
}

void CPU::ins_BIT_b_r(Byte bit, Byte* registerOne)
{
	this->registers.set_flag(n, 0);
	this->registers.set_flag(h, 1);
	this->registers.set_flag(z, ((*registerOne & (1 << bit)) == 0));

	this->isExecutingCB = false;
	this->isExecutingInstruction = false;
}

void CPU::ins_BIT_b_r_bHLb(Byte bit)
{
	switch (this->mCyclesUsed)
	{
	case 0: this->mCyclesUsed++; break;
	case 1:

		this->instructionCache[0] = this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu);
		this->registers.set_flag(n, 0);
		this->registers.set_flag(h, 1);
		this->registers.set_flag(z, ((this->instructionCache[0] & (1 << bit)) == 0));

		this->isExecutingCB = false;
		this->isExecutingInstruction = false;
	}
}

void CPU::ins_RES_b_r(Byte bit, Byte* registerOne)
{
	*registerOne &= ~(1 << bit);

	this->isExecutingCB = false;
	this->isExecutingInstruction = false;
}

void CPU::ins_RES_b_r_bHLb(Byte bit)
{
	switch (this->mCyclesUsed)
	{
	case 0: this->mCyclesUsed++; break;
	case 1:	this->instructionCache[0] = this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu); this->mCyclesUsed++; break;
	case 2:

		Byte temp = this->instructionCache[0];
		temp &= ~(1 << bit);
		this->bus->set_memory(this->registers.get_HL(), temp, MEMORY_ACCESS_TYPE::cpu);

		this->isExecutingCB = false;
		this->isExecutingInstruction = false;
	}
}

void CPU::ins_SET_b_r(Byte bit, Byte* registerOne)
{
	*registerOne |= (1 << bit);

	this->isExecutingCB = false;
	this->isExecutingInstruction = false;
}

void CPU::ins_SET_b_r_bHLb(Byte bit)
{
	switch (this->mCyclesUsed)
	{
	case 0: this->mCyclesUsed++; break;
	case 1:	this->instructionCache[0] = this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu); this->mCyclesUsed++; break;
	case 2:

		Byte temp = this->instructionCache[0];
		temp |= (1 << bit);
		this->bus->set_memory(this->registers.get_HL(), temp, MEMORY_ACCESS_TYPE::cpu);

		this->isExecutingCB = false;
		this->isExecutingInstruction = false;
	}
}


// INSTRUCTION HANDLING OLD


void CPU::ins_DAA()
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


	this->isExecutingInstruction = false;
	return;

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
	this->registers.set_flag(c, (need_carry));

	this->registers.set_flag(h, 0);

	this->registers.a += adjust;

	this->registers.set_flag(z, (this->registers.a == 0));



	this->isExecutingInstruction = false;
	return;

}












