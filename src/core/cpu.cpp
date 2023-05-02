#include "cpu.hpp"
#include "bus.hpp"

#include <iostream>
#include <bitset>
#include <sstream>
#include <cstdio>

//#define BREAKPOINT 0xCb15
#define BREAKPOINT 0x00EB

/*
Not sure where you got that documentation from, but it's not correct.

Like you said above, if a HALT instruction is executed while interrupts are enabled, the Game Boy CPU will halt until an interrupt is requested, at which point the CPU will resume operation and service the interrupt.

If a HALT instruction is executed while interrupts are disabled and there are no pending interrupts, then the CPU will still halt! The difference is that when an interrupt is requested, the CPU resumes operation and continues executing normally without servicing the interrupt. This is because the IME flag actually does not disable interrupts altogether, rather it disables servicing them.

The behavior you describe above is the HALT bug. This bug is triggered when a HALT instruction is executed while interrupts are disabled, and there are pending interrupts waiting to be serviced. That is, IF & IE != 0. In this case, the CPU will not halt, and instead fail to increment the PC, causing the next instruction to be executed twice. It does not execute HALT twice, because the PC has already incremented during instruction decoding.

	How important is it to pass this test? Does implementing HALT really matter if I'm going to just fake it without saving any processing power?

Yes! Not implementing the HALT instruction will cause your timings to be wildly off in basically every game. In addition, Thunderbirds is a game which actually hits the HALT bug, and depends on it being emulated accurately to run correctly!
*/


void CPU::mStepCPU()
{	
	// fetch
	if (!is_executing_instruction && !is_halted)
	{
		current_running_opcode = getByteFromPC();	
		setupForNextInstruction();
	}
	
	if (is_executing_instruction)
	{
		//decode and execute instruction // takes a cycle
		instructionHandler();
			
		//if finished instruction after this execution
		if (!is_executing_instruction)
		{
			if (!is_halted)
			{
				is_instruction_complete = true;


				if (ei_triggered && current_running_opcode != 0xFB)
				{
					interrupt_master_enable = true;
					ei_triggered = false;
				}

				checkForInterrupts();
				if (interrupt_vector != 0)
				{
					setupForNextInstruction();
					return;
				}
			}
		}

		if (halt_bug)
		{
			halt_bug = false;
			registers.pc--;
		}
		return;
	}

	if (is_halted)
	{
		haltHandler();
		return;
	}
};

/*
// run mode, fetch decode and part execute
	else if (!is_executing_instruction)
	{
		current_running_opcode = getByteFromPC(); // fetch
		setupForNextInstruction(); //restart counting mcycles, change state to execute instruction
		if (halt_bug)
		{
			registers.pc--;
			halt_bug = false;
		}
		executePartOfInstruction();
		return;
	}
*/


int CPU::mStepCPUOneInstruction()
{
	int i = 0;
	is_instruction_complete = false;
	while (!is_instruction_complete)
	{
		mStepCPU();
		i++;
	}
	is_instruction_complete = false;
	return i;
}
void CPU::haltHandler()
{

	Byte interrupt_request_register = getMemory(IF_REGISTER);
	Byte interrupt_types_enabled_register = getMemory(IE_REGISTER);
	bool any_pending_interrupts = ((interrupt_request_register & interrupt_types_enabled_register) & 0x1F) != 0;
	bool ime = interrupt_master_enable;

	if (!ime && !any_pending_interrupts)
		halt_bug = true;

	if (ime)
	{
		if (any_pending_interrupts)
		{
			is_halted = false;
			prefetchInstruction();
			checkForInterrupts();
		}
		is_halted = false;
		is_instruction_complete = true;
		return;
	}
	if (any_pending_interrupts && halt_bug)
	{
		is_halted = false;
		halt_bug = false;
		prefetchInstruction();
		return;
	}
	if (any_pending_interrupts)
	{
		is_halted = false;
		prefetchInstruction();
		registers.pc--;		
		return;
	}	
};

// if IME is disabled
	// If interrupt pending, exit HALT, however enter HALT bug area, during this condition, halt exits immediately but instead fails to increment the PC, causing the next instruction to be executed twice. It does not execute HALT twice, because the PC has already incremented during instruction decoding.

	// The halt bug fails to increment the pc, so the next byte read is the same, this can cause a completely different instruction to execute
	// halt
	// 3E 14
	// actually
	// halt
	// 3E 3E
	// 14
	// to emulate it we need to make sure that post fetch we decrement the pc by one

void CPU::checkForInterrupts()
{
	//check master to see if interrupts are enabled
	if (interrupt_master_enable == true)
	{
		// iterate through all types, this allows us to check each interrupt and also service them in order of priority
		for (const auto type : eInterruptTypes_all)
		{
			// if it has been requested and its corresponding flag is enabled, due to priority
			if (getInterruptFlag(type, IF_REGISTER) && getInterruptFlag(type, IE_REGISTER))
			{
				//disable interrupts in general and for this type

				setInterruptFlag(type, 0, IF_REGISTER);
				interrupt_master_enable = false;

				// set jump vector
				switch (type)
				{
				case eInterruptTypes::vblank:  { interrupt_vector = 0x0040; } break;
				case eInterruptTypes::lcdstat: { interrupt_vector = 0x0048; } break;
				case eInterruptTypes::timer:   { interrupt_vector = 0x0050; } break;
				case eInterruptTypes::serial:  { interrupt_vector = 0x0058; } break;
				case eInterruptTypes::joypad:  { interrupt_vector = 0x0060; } break;
				default: fprintf(stderr, "Unreachable interrupt type"); exit(-1); break;
				}
				return;
			}
		}
	}
}

// https://gbdev.io/pandocs/Interrupts.html
// source on length
void CPU::setupInterruptHandler()
{
	switch (mcycles_used)
	{
	case 0:
		mcycles_used++;
		// undocumented_internal_operation
		break;
	case 1:
		mcycles_used++;
		// undocumented_internal_operation
		break;
	case 2:
		setMemory(--registers.sp, (registers.pc >> 8));
		mcycles_used++;
		break;

	case 3:
		setMemory(--registers.sp, (registers.pc & 0xff));
		mcycles_used++;
		break;

	case 4:

		registers.pc = interrupt_vector;

		interrupt_vector = 0;
		is_executing_instruction = false;
		break;
	}
}


void CPU::DEBUG_printCurrentState(Word pc)
{
	if (registers.pc == pc)
		debug_toggle = true;
	if (debug_toggle)
	{
		printf("%s:0x%.4X  ", "pc", registers.pc);
		//printf("%s:0x%.2X  ", "cyclesused", mcycles_used);
		printf("op:0x%.2X | ", getMemory(registers.pc));
		printf("%s:0x%.2X%.2X  ", "AF", registers.a, registers.f);
		printf("%s:0x%.2X%.2X  ", "BC", registers.b, registers.c);
		printf("%s:0x%.2X%.2X  ", "DE", registers.d, registers.e);
		printf("%s:0x%.2X%.2X  ", "HL", registers.h, registers.l);
		printf("%s:0x%.4X  ", "SP", registers.sp);
		printf("%s:0x%.4X  ", "STAT", getMemory(STAT));
		printf("%s:%i  ", "IME", interrupt_master_enable);
		//printf("%s:%x  ", "DIV", bus->io[4]);
		//printf("%s:%x  ", "TIMA", bus->io[5]);
		//printf("%s:%x  ", "TMA", bus->io[6]);
		//printf("%s:%x  ", "TAC", bus->io[7]);
		//printf("%s:%x  ", "divC", divtimer_counter);
		//printf("%s:%x  ", "timC", timer_counter);
		

		/*printf("%s:%i  ","z", registers.get_flag(z));
		printf("%s:%i  ","n", registers.get_flag(n));
		printf("%s:%i  ","h", registers.get_flag(h));
		printf("%s:%i  ","c", registers.get_flag(c));
		printf("%s:","0xFF00");
		std::cout << std::bitset<8>(bus->io[0]);*/

		printf("\n");
	}
	if (registers.pc == pc)
		debug_toggle = true;
}

void CPU::DEBUG_printCurrentState()
{
	if (debug_toggle)
	{
		printf("%s:0x%.4X  ", "pc", registers.pc - 1);
		//printf("%s:0x%.2X  ", "cyclesused", mcycles_used);
		printf("op:0x%.2X | ", getMemory(registers.pc - 1));
		printf("%s:0x%.2X%.2X  ", "AF", registers.a, registers.f);
		printf("%s:0x%.2X%.2X  ", "BC", registers.b, registers.c);
		printf("%s:0x%.2X%.2X  ", "DE", registers.d, registers.e);
		printf("%s:0x%.2X%.2X  ", "HL", registers.h, registers.l);
		printf("%s:0x%.4X  ", "SP", registers.sp);
		printf("%s:0x%.4X  ", "STAT", getMemory(STAT));
		printf("%s:%i  ", "IME", interrupt_master_enable);
		//printf("%s:%x  ", "DIV", bus->io[4]);
		//printf("%s:%x  ", "TIMA", bus->io[5]);
		//printf("%s:%x  ", "TMA", bus->io[6]);
		//printf("%s:%x  ", "TAC", bus->io[7]);
		//printf("%s:%x  ", "divC", divtimer_counter);
		//printf("%s:%x  ", "timC", timer_counter);


		/*printf("%s:%i  ","z", registers.get_flag(z));
		printf("%s:%i  ","n", registers.get_flag(n));
		printf("%s:%i  ","h", registers.get_flag(h));
		printf("%s:%i  ","c", registers.get_flag(c));
		printf("%s:","0xFF00");
		std::cout << std::bitset<8>(bus->io[0]);*/

		printf("\n");
	}
}

/// <summary>
/// sets the registers of the cpu to their initial power up state, program counter is set to skip the bios program by default
/// </summary>
void CPU::init()
{
	registers.pc = 0x100;
	registers.a = 0x01;
	registers.b = 0x00;
	registers.c = 0x13;
	registers.d = 0x00;
	registers.e = 0xd8;
	registers.f = 0xb0;
	registers.h = 0x01;
	registers.l = 0x4d;
	registers.sp = 0xFFFE;
}

void CPU::biosInit()
{
	registers.pc = 0;
	registers.a = 0;
	registers.b = 0;
	registers.c = 0;
	registers.d = 0;
	registers.e = 0;
	registers.f = 0;
	registers.h = 0;
	registers.l = 0;
	registers.sp = 0;
}


CPU::CPU()
{
	init();
}


const Byte CPU::getByteFromPC()
{
	return getMemory(registers.pc++);
}

void CPU::DEBUG_print_IO()
{
	std::cout << "\t0xFF00  [JOYP]: ";  printf("%.2X\n", getMemory(0xFF00));
	std::cout << "\t0xFF01    [SB]: ";  printf("%.2X\n", getMemory(0xFF01));
	std::cout << "\t0xFF02    [SC]: ";  printf("%.2X\n", getMemory(0xFF02));
	std::cout << "\t0xFF04   [DIV]: ";  printf("%.2X\n", getMemory(0xFF04));
	std::cout << "\t0xFF05  [TIMA]: ";  printf("%.2X\n", getMemory(0xFF05));
	std::cout << "\t0xFF06   [TMA]: ";  printf("%.2X\n", getMemory(0xFF06));
	std::cout << "\t0xFF07   [TAC]: ";  printf("%.2X\n", getMemory(0xFF07));
	std::cout << "\t0xFF0F    [IF]: ";  printf("%.2X\n", getMemory(0xFF0F));
	std::cout << "\t0xFF40  [LCDC]: ";  printf("%.2X\n", getMemory(0xFF40));
	std::cout << "\t0xFF41  [STAT]: ";  printf("%.2X\n", getMemory(0xFF41));
	std::cout << "\t0xFF42   [SCY]: ";  printf("%.2X\n", getMemory(0xFF42));
	std::cout << "\t0xFF43   [SCX]: ";  printf("%.2X\n", getMemory(0xFF43));
	std::cout << "\t0xFF44    [LY]: ";  printf("%.2X\n", getMemory(0xFF44));
	std::cout << "\t0xFF45   [LYC]: ";  printf("%.2X\n", getMemory(0xFF45));
	std::cout << "\t0xFF47   [BGP]: ";  printf("%.2X\n", getMemory(0xFF47));
	std::cout << "\t0xFF48  [OPB0]: ";  printf("%.2X\n", getMemory(0xFF48));
	std::cout << "\t0xFF49  [OPB1]: ";  printf("%.2X\n", getMemory(0xFF49));
	std::cout << "\t0xFF4A    [WY]: ";  printf("%.2X\n", getMemory(0xFF4A));
	std::cout << "\t0xFF4B    [WX]: ";  printf("%.2X\n", getMemory(0xFF4B));
	std::cout << "\t0xFFFF    [IE]: ";  printf("%.2X\n", getMemory(0xFFFF));
	std::cout << "\t         [IME]: ";  printf("%.2X\n", bus->interrupt_enable_register);
}

void CPU::connectToBus(BUS* pBus)
{
	bus = pBus;
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

bool CPU::checkJumpCondition(enum eJumpCondition condition)
{
	switch (condition)
	{
	case BYPASS:
		return true;
		break;
	case NZ:
		if (!registers.getFlag(z))
			return true;
		break;
	case Z:
		if (registers.getFlag(z))
			return true;
		break;
	case NC:
		if (!registers.getFlag(c))
			return true;
		break;
	case C:
		if (registers.getFlag(c))
			return true;
		break;
	default: fprintf(stderr, "Unreachable Jump condition");  exit(-1); break;
	}
	return false;
}

Byte CPU::getMemory(const Word address)
{
	return bus->getMemory(address, eMemoryAccessType::cpu);
}

void CPU::setMemory(const Word address, const Byte data)
{
	bus->setMemory(address, data, eMemoryAccessType::cpu);
}

Byte CPU::getNibble(const Byte input, const bool getHi)
{
	Byte result = 0;
	(getHi) ? result = (input & 0xF0) >> 4 : result = input & 0x0F;
	return result;
}

void CPU::setNibble(Byte* reigster_one, const Byte value, const bool setHi)
{
	(setHi) ? *reigster_one = ((*reigster_one & 0x0F) | (value << 4)) : *reigster_one = ((*reigster_one & 0xF0) | value);
}


void CPU::updateTimers(const int tcycles)
{
	//https://www.reddit.com/r/EmuDev/comments/5qa3x1/comment/dcykiwt/?utm_source=share&utm_medium=web2x&context=3
	/*
		Timestep is 4 tcycles, 4.2M cycles in a second
		div overflows 16384 times a second
		4194304 / 16384 = 256, every 256 tcyles div will overflow
	*/
	divtimer_counter += tcycles;
	if (divtimer_counter >= DIV_INC_RATE)
	{
		divtimer_counter = 0;
		timer_registers.div++;
	}

	// if TMC bit 2 is set, this means that the timer is enabled
	if (timer_registers.tac & (0b00000100))
	{
		timer_counter += tcycles;
		if (timer_counter >= getTACFrequency())
		{
			timer_counter = 0;
			if (timer_registers.tima == 0xFF)
			{
				timer_registers.tima = timer_registers.tma;
				requestInterrupt(timer);
				return;
			}
			timer_registers.tima++;
		}
	}
}

/// <summary>
/// This is the master function for handling the CPU timers, timers do not increment every CPU cycle but after a specified amount of cycles.
/// Since there are two types of timers (normal timer and div timer) we need to store a rolling counter of how many cycles have been executed before the timer needs to advance.
/// this is done in the divTimerCounter and timerCounter respectively, each hold the amount of cycles it takes for the counter to increment.
/// 
/// the div counter increments once every 256 counters, we remove the amount of cycles done this CPU 'tick' from divTimerCounter and when that reaches 0 or less then we increment the register at position DIV, however the Gameboy cannot directly write to postion DIV since every write will reset the register.
/// so we access the io regsiters directly and increment the register.
/// divCounter is reset back to its initial value and we can continue all over again.
/// 
/// the TAC, timer controller is responsible for two things, enabling the timer and setting the rate of increment, the 2nd bit of the TAC shows if it is enabled. we modify timerCounter and check if the register needs incrementing, of the value of the TIMA register is 0xFF (TIMA is like DIV) then we need to overfllow
/// and when the TIMA overflows we set to the value of TMA and request an interrupt of type timer.
/// </summary>
/// <param name="cycles"></param>

Byte CPU::getTMCFrequency()
{
	return timer_registers.tac & 0x3;
}

int CPU::getTACFrequency()
{
	switch (getTMCFrequency())
	{
	case 0b00: { return GB_CPU_TCYCLE_CLOCKSPEED / 4096;   } break;
	case 0b01: { return GB_CPU_TCYCLE_CLOCKSPEED / 262144; } break;
	case 0b10: { return GB_CPU_TCYCLE_CLOCKSPEED / 65536;  } break;
	case 0b11: { return GB_CPU_TCYCLE_CLOCKSPEED / 16384;  } break;
	default: fprintf(stderr, "Unreachable timer frequency");  exit(-1); break;
	}

}

void CPU::requestInterrupt(const eInterruptTypes type)
{
	setInterruptFlag(type, 1, IF_REGISTER);
}

Byte CPU::getInterruptFlag(const enum eInterruptTypes type, Word address)
{
	return bus->getMemory(address, eMemoryAccessType::interrupt_handler) & type;
}

void CPU::setInterruptFlag(const enum eInterruptTypes type, const bool value, Word address)
{
	auto mem_type = eMemoryAccessType::interrupt_handler;
	auto original_value = bus->getMemory(address, mem_type);
	auto new_value = (value) ? (original_value | type) : (original_value & ~type);
	bus->setMemory(address, new_value, mem_type);
}

// some instructions only take a single cycle, peforming the fetch decode and execute in one mCycle

// need to implement the prefetcher:
// on the last mCycle of the execution of an instruction, the next opcode is prefetched and ready to run
// therefore next cycle we just execute it, this is why it may looks like f d e is performed in one go





void CPU::setupForNextInstruction()
{
	mcycles_used = 0;
	is_executing_instruction = true;
}

void CPU::prefetchInstruction()
{
	setupForNextInstruction();
	current_running_opcode = getByteFromPC();
}

void CPU::instructionHandler()
{
	if (interrupt_vector != 0)
	{
		setupInterruptHandler();
		return;
	}
	switch (current_running_opcode)
	{
	case 0x00: { is_executing_instruction = false; } break; //NOP
	case 0x01: { ins_LD_XX_u16(&registers.b, &registers.c); } break;
	case 0x02: { ins_LD_bXXb_Y(registers.getBC(), &registers.a); } break;
	case 0x03: { ins_INC_XX(&registers.b, &registers.c); } break;
	case 0x04: { ins_INC_X(&registers.b); } break;
	case 0x05: { ins_DEC_X(&registers.b); } break;
	case 0x06: { ins_LD_X_u8(&registers.b); } break;
	case 0x07: { ins_RLCA(); } break;

	case 0x08: { ins_LD_bu16b_SP(); } break;
	case 0x09: { ins_ADD_HL_XX(&registers.b, &registers.c); } break;
	case 0x0A: { ins_LD_X_bYYb(&registers.a, &registers.b, &registers.c);  } break;
	case 0x0B: { ins_DEC_XX(&registers.b, &registers.c); } break;
	case 0x0C: { ins_INC_X(&registers.c); } break;
	case 0x0D: { ins_DEC_X(&registers.c); } break;
	case 0x0E: { ins_LD_X_u8(&registers.c); } break;
	case 0x0F: { ins_RRCA(); } break;

	case 0x10: { instructionHandlerSTOP(); } break;
	case 0x11: { ins_LD_XX_u16(&registers.d, &registers.e); } break;
	case 0x12: { ins_LD_bXXb_Y(registers.getDE(), &registers.a); } break;
	case 0x13: { ins_INC_XX(&registers.d, &registers.e); } break;
	case 0x14: { ins_INC_X(&registers.d); } break;
	case 0x15: { ins_DEC_X(&registers.d); } break;
	case 0x16: { ins_LD_X_u8(&registers.d); } break;
	case 0x17: { ins_RLA(); } break;

	case 0x18: { ins_JR_i8(); } break;
	case 0x19: { ins_ADD_HL_XX(&registers.d, &registers.e); } break;
	case 0x1A: { ins_LD_X_bYYb(&registers.a, &registers.d, &registers.e);  } break;
	case 0x1B: { ins_DEC_XX(&registers.d, &registers.e); } break;
	case 0x1C: { ins_INC_X(&registers.e); } break;
	case 0x1D: { ins_DEC_X(&registers.e); } break;
	case 0x1E: { ins_LD_X_u8(&registers.e); } break;
	case 0x1F: { ins_RRA(); } break;

	case 0x20: { ins_JR_i8(NZ); } break;
	case 0x21: { ins_LD_XX_u16(&registers.h, &registers.l); } break;
	case 0x22: { ins_LD_bHLb_Apm(true); } break;
	case 0x23: { ins_INC_XX(&registers.h, &registers.l); } break;
	case 0x24: { ins_INC_X(&registers.h); } break;
	case 0x25: { ins_DEC_X(&registers.h); } break;
	case 0x26: { ins_LD_X_u8(&registers.h); } break;
	case 0x27: { ins_DAA(); } break;

	case 0x28: { ins_JR_i8(Z); } break;
	case 0x29: { ins_ADD_HL_XX(&registers.h, &registers.l); } break;
	case 0x2A: { ins_LD_X_bYYb(&registers.a, &registers.h, &registers.l, +1);  } break;
	case 0x2B: { ins_DEC_XX(&registers.h, &registers.l); } break;
	case 0x2C: { ins_INC_X(&registers.l); } break;
	case 0x2D: { ins_DEC_X(&registers.l); } break;
	case 0x2E: { ins_LD_X_u8(&registers.l); } break;
	case 0x2F: { ins_CPL(); } break;

	case 0x30: { ins_JR_i8(NC); } break;
	case 0x31: { ins_LD_SP_u16(&registers.sp); } break; //sp is a word
	case 0x32: { ins_LD_bHLb_Apm(false); } break;
	case 0x33: { ins_INC_SP(); } break;
	case 0x34: { ins_INC_bHLb(); } break;
	case 0x35: { ins_DEC_bHLb(); } break;
	case 0x36: { ins_LD_bHLb_u8(); } break;
	case 0x37: { ins_SCF(); } break;

	case 0x38: { ins_JR_i8(C); } break;
	case 0x39: { ins_ADD_HL_SP(); } break;
	case 0x3A: { ins_LD_X_bYYb(&registers.a, &registers.h, &registers.l, -1);  } break;
	case 0x3B: { ins_DEC_SP(); } break;
	case 0x3C: { ins_INC_X(&registers.a); } break;
	case 0x3D: { ins_DEC_X(&registers.a); } break;
	case 0x3E: { ins_LD_X_u8(&registers.a); } break;
	case 0x3F: { ins_CCF(); } break;

	case 0x40: { ins_LD_X_Y(&registers.b, &registers.b); } break;
	case 0x41: { ins_LD_X_Y(&registers.b, &registers.c); } break;
	case 0x42: { ins_LD_X_Y(&registers.b, &registers.d); } break;
	case 0x43: { ins_LD_X_Y(&registers.b, &registers.e); } break;
	case 0x44: { ins_LD_X_Y(&registers.b, &registers.h); } break;
	case 0x45: { ins_LD_X_Y(&registers.b, &registers.l); } break;
	case 0x46: { ins_LD_X_bYYb(&registers.b, &registers.h, &registers.l); } break;
	case 0x47: { ins_LD_X_Y(&registers.b, &registers.a); } break;

	case 0x48: { ins_LD_X_Y(&registers.c, &registers.b); } break;
	case 0x49: { ins_LD_X_Y(&registers.c, &registers.c); } break;
	case 0x4A: { ins_LD_X_Y(&registers.c, &registers.d); } break;
	case 0x4B: { ins_LD_X_Y(&registers.c, &registers.e); } break;
	case 0x4C: { ins_LD_X_Y(&registers.c, &registers.h); } break;
	case 0x4D: { ins_LD_X_Y(&registers.c, &registers.l); } break;
	case 0x4E: { ins_LD_X_bYYb(&registers.c, &registers.h, &registers.l); } break;
	case 0x4F: { ins_LD_X_Y(&registers.c, &registers.a); } break;

	case 0x50: { ins_LD_X_Y(&registers.d, &registers.b); } break;
	case 0x51: { ins_LD_X_Y(&registers.d, &registers.c); } break;
	case 0x52: { ins_LD_X_Y(&registers.d, &registers.d); } break;
	case 0x53: { ins_LD_X_Y(&registers.d, &registers.e); } break;
	case 0x54: { ins_LD_X_Y(&registers.d, &registers.h); } break;
	case 0x55: { ins_LD_X_Y(&registers.d, &registers.l); } break;
	case 0x56: { ins_LD_X_bYYb(&registers.d, &registers.h, &registers.l); } break;
	case 0x57: { ins_LD_X_Y(&registers.d, &registers.a); } break;

	case 0x58: { ins_LD_X_Y(&registers.e, &registers.b); } break;
	case 0x59: { ins_LD_X_Y(&registers.e, &registers.c); } break;
	case 0x5A: { ins_LD_X_Y(&registers.e, &registers.d); } break;
	case 0x5B: { ins_LD_X_Y(&registers.e, &registers.e); } break;
	case 0x5C: { ins_LD_X_Y(&registers.e, &registers.h); } break;
	case 0x5D: { ins_LD_X_Y(&registers.e, &registers.l); } break;
	case 0x5E: { ins_LD_X_bYYb(&registers.e, &registers.h, &registers.l); } break;
	case 0x5F: { ins_LD_X_Y(&registers.e, &registers.a); } break;

	case 0x60: { ins_LD_X_Y(&registers.h, &registers.b); } break;
	case 0x61: { ins_LD_X_Y(&registers.h, &registers.c); } break;
	case 0x62: { ins_LD_X_Y(&registers.h, &registers.d); } break;
	case 0x63: { ins_LD_X_Y(&registers.h, &registers.e); } break;
	case 0x64: { ins_LD_X_Y(&registers.h, &registers.h); } break;
	case 0x65: { ins_LD_X_Y(&registers.h, &registers.l); } break;
	case 0x66: { ins_LD_X_bYYb(&registers.h, &registers.h, &registers.l); } break;
	case 0x67: { ins_LD_X_Y(&registers.h, &registers.a); } break;

	case 0x68: { ins_LD_X_Y(&registers.l, &registers.b); } break;
	case 0x69: { ins_LD_X_Y(&registers.l, &registers.c); } break;
	case 0x6A: { ins_LD_X_Y(&registers.l, &registers.d); } break;
	case 0x6B: { ins_LD_X_Y(&registers.l, &registers.e); } break;
	case 0x6C: { ins_LD_X_Y(&registers.l, &registers.h); } break;
	case 0x6D: { ins_LD_X_Y(&registers.l, &registers.l); } break;
	case 0x6E: { ins_LD_X_bYYb(&registers.l, &registers.h, &registers.l); } break;
	case 0x6F: { ins_LD_X_Y(&registers.l, &registers.a); } break;

	case 0x70: { ins_LD_bXXb_Y(registers.getHL(), &registers.b); } break;
	case 0x71: { ins_LD_bXXb_Y(registers.getHL(), &registers.c); } break;
	case 0x72: { ins_LD_bXXb_Y(registers.getHL(), &registers.d); } break;
	case 0x73: { ins_LD_bXXb_Y(registers.getHL(), &registers.e); } break;
	case 0x74: { ins_LD_bXXb_Y(registers.getHL(), &registers.h); } break;
	case 0x75: { ins_LD_bXXb_Y(registers.getHL(), &registers.l); } break;
	case 0x76: { is_halted = true; is_executing_instruction = false; } break; //HALT
	case 0x77: { ins_LD_bXXb_Y(registers.getHL(), &registers.a); } break;

	case 0x78: { ins_LD_X_Y(&registers.a, &registers.b); } break;
	case 0x79: { ins_LD_X_Y(&registers.a, &registers.c); } break;
	case 0x7A: { ins_LD_X_Y(&registers.a, &registers.d); } break;
	case 0x7B: { ins_LD_X_Y(&registers.a, &registers.e); } break;
	case 0x7C: { ins_LD_X_Y(&registers.a, &registers.h); } break;
	case 0x7D: { ins_LD_X_Y(&registers.a, &registers.l); } break;
	case 0x7E: { ins_LD_X_bYYb(&registers.a, &registers.h, &registers.l); } break;
	case 0x7F: { ins_LD_X_Y(&registers.a, &registers.a); } break;

	case 0x80: { ins_ADD_A_X(&registers.b); } break;
	case 0x81: { ins_ADD_A_X(&registers.c); } break;
	case 0x82: { ins_ADD_A_X(&registers.d); } break;
	case 0x83: { ins_ADD_A_X(&registers.e); } break;
	case 0x84: { ins_ADD_A_X(&registers.h); } break;
	case 0x85: { ins_ADD_A_X(&registers.l); } break;
	case 0x86: { ins_ADD_A_bHLb(); } break;
	case 0x87: { ins_ADD_A_X(&registers.a); } break;

	case 0x88: { ins_ADC_A_X(&registers.b); } break;
	case 0x89: { ins_ADC_A_X(&registers.c); } break;
	case 0x8A: { ins_ADC_A_X(&registers.d); } break;
	case 0x8B: { ins_ADC_A_X(&registers.e); } break;
	case 0x8C: { ins_ADC_A_X(&registers.h); } break;
	case 0x8D: { ins_ADC_A_X(&registers.l); } break;
	case 0x8E: { ins_ADC_A_bHLb(); } break;
	case 0x8F: { ins_ADC_A_X(&registers.a); } break;

	case 0x90: { ins_SUB_A_X(&registers.b); } break;
	case 0x91: { ins_SUB_A_X(&registers.c); } break;
	case 0x92: { ins_SUB_A_X(&registers.d); } break;
	case 0x93: { ins_SUB_A_X(&registers.e); } break;
	case 0x94: { ins_SUB_A_X(&registers.h); } break;
	case 0x95: { ins_SUB_A_X(&registers.l); } break;
	case 0x96: { ins_SUB_A_bHLb(); } break;
	case 0x97: { ins_SUB_A_X(&registers.a); } break;

	case 0x98: { ins_SBC_A_X(&registers.b); } break;
	case 0x99: { ins_SBC_A_X(&registers.c); } break;
	case 0x9A: { ins_SBC_A_X(&registers.d); } break;
	case 0x9B: { ins_SBC_A_X(&registers.e); } break;
	case 0x9C: { ins_SBC_A_X(&registers.h); } break;
	case 0x9D: { ins_SBC_A_X(&registers.l); } break;
	case 0x9E: { ins_SBC_A_bHLb(); } break;
	case 0x9F: { ins_SBC_A_X(&registers.a); } break;

	case 0xA0: { ins_AND_A_X(&registers.b); } break;
	case 0xA1: { ins_AND_A_X(&registers.c); } break;
	case 0xA2: { ins_AND_A_X(&registers.d); } break;
	case 0xA3: { ins_AND_A_X(&registers.e); } break;
	case 0xA4: { ins_AND_A_X(&registers.h); } break;
	case 0xA5: { ins_AND_A_X(&registers.l); } break;
	case 0xA6: { ins_AND_A_bHLb(); } break;
	case 0xA7: { ins_AND_A_X(&registers.a); } break;

	case 0xA8: { ins_XOR_A_X(&registers.b); } break;
	case 0xA9: { ins_XOR_A_X(&registers.c); } break;
	case 0xAA: { ins_XOR_A_X(&registers.d); } break;
	case 0xAB: { ins_XOR_A_X(&registers.e); } break;
	case 0xAC: { ins_XOR_A_X(&registers.h); } break;
	case 0xAD: { ins_XOR_A_X(&registers.l); } break;
	case 0xAE: { ins_XOR_A_bHLb(); } break;
	case 0xAF: { ins_XOR_A_X(&registers.a); } break;

	case 0xB0: { ins_OR_A_X(&registers.b); } break;
	case 0xB1: { ins_OR_A_X(&registers.c); } break;
	case 0xB2: { ins_OR_A_X(&registers.d); } break;
	case 0xB3: { ins_OR_A_X(&registers.e); } break;
	case 0xB4: { ins_OR_A_X(&registers.h); } break;
	case 0xB5: { ins_OR_A_X(&registers.l); } break;
	case 0xB6: { ins_OR_A_bHLb(); } break;
	case 0xB7: { ins_OR_A_X(&registers.a); } break;

	case 0xB8: { ins_CP_A_X(&registers.b); } break;
	case 0xB9: { ins_CP_A_X(&registers.c); } break;
	case 0xBA: { ins_CP_A_X(&registers.d); } break;
	case 0xBB: { ins_CP_A_X(&registers.e); } break;
	case 0xBC: { ins_CP_A_X(&registers.h); } break;
	case 0xBD: { ins_CP_A_X(&registers.l); } break;
	case 0xBE: { ins_CP_A_bHLb(); } break;
	case 0xBF: { ins_CP_A_X(&registers.a); } break;

	case 0xC0: { ins_RET_CC(NZ); } break;
	case 0xC1: { ins_POP_XX(&registers.b, &registers.c); } break;
	case 0xC2: { ins_JP_u16(NZ); } break;
	case 0xC3: { ins_JP_u16(); } break;
	case 0xC4: { ins_CALL_u16(NZ); } break;
	case 0xC5: { ins_PUSH_XX(registers.getBC()); } break;
	case 0xC6: { ins_ADD_A_u8(); } break;
	case 0xC7: { ins_RST(0x00); } break;

	case 0xC8: { ins_RET_CC(Z); } break;
	case 0xC9: { ins_RET(); } break;
	case 0xCA: { ins_JP_u16(Z); } break;
	case 0xCB: { instructionHandlerCB(); } break;
	case 0xCC: { ins_CALL_u16(Z); } break;
	case 0xCD: { ins_CALL_u16(); } break;
	case 0xCE: { ins_ADC_A_u8(); } break;
	case 0xCF: { ins_RST(0x08); } break;

	case 0xD0: { ins_RET_CC(NC); } break;
	case 0xD1: { ins_POP_XX(&registers.d, &registers.e); } break;
	case 0xD2: { ins_JP_u16(NC); } break;
		//case 0xD3: { ins_SBC_A_n(nullptr, get_byte_from_pc()); } break;
	case 0xD4: { ins_CALL_u16(NC); } break;
	case 0xD5: { ins_PUSH_XX(registers.getDE()); } break;
	case 0xD6: { ins_SUB_A_u8();  } break;
	case 0xD7: { ins_RST(0x10); } break;

	case 0xD8: { ins_RET_CC(C); } break; //35
	case 0xD9: { ins_RETI(); } break;
	case 0xDA: { ins_JP_u16(C); } break;
		//case 0xDB: { } break;
	case 0xDC: { ins_CALL_u16(C); } break;
		//case 0xDD: { } break;
	case 0xDE: { ins_SBC_A_u8(); } break;
	case 0xDF: { ins_RST(0x18); } break;

	case 0xE0: { ins_LD_bFF00_u8b_A(); } break;
	case 0xE1: { ins_POP_XX(&registers.h, &registers.l); } break;
	case 0xE2: { ins_LD_bXXb_Y(0xFF00 + registers.c, &registers.a); } break;
		//case 0xE3: { } break;
		//case 0xE4: { } break;
	case 0xE5: { ins_PUSH_XX(registers.getHL()); } break;
	case 0xE6: { ins_AND_A_u8(); } break;
	case 0xE7: { ins_RST(0x20); } break;

	case 0xE8: { ins_ADD_SP_i8(); } break;
	case 0xE9: { ins_JP_HL(); } break;
	case 0xEA: { ins_LD_bu16b_A(); } break;
		//case 0xEB: { } break;
		//case 0xEC: { } break;
		//case 0xED: { } break;
	case 0xEE: { ins_XOR_A_u8(); } break;
	case 0xEF: { ins_RST(0x28); } break;

	case 0xF0: { ins_LD_A_bFF00_u8b(); } break;
	case 0xF1: { ins_POP_XX(&registers.a, &registers.f); } break;
	case 0xF2: { ins_LD_A_bFF00_Cb(); } break;
	case 0xF3: { interrupt_master_enable = 0; ei_triggered = false; is_executing_instruction = false; } break; //DIsable interrupts immediately, if EI is set to trigger, we can disable it so we can keep the behaviour of EI then DI never enabling interrupts
	//case 0xF4: { } break;
	case 0xF5: { ins_PUSH_XX(registers.getAF()); } break;
	case 0xF6: { ins_OR_A_u8(); } break;
	case 0xF7: { ins_RST(0x30); } break;

	case 0xF8: { ins_LD_HL_SP_i8(); } break;
	case 0xF9: { ins_LD_SP_HL(); } break;
	case 0xFA: { ins_LD_A_bu16b(); } break;
	case 0xFB: { ei_triggered = true; is_executing_instruction = false; return; } break; // EI is set to trigger, only activates after next instruction finishes note the early return
	//case 0xFC: { } break;
	//case 0xFD: { } break;
	case 0xFE: { ins_CP_A_u8(); } break;
	case 0xFF: { ins_RST(0x38); } break;
	default:
		{
			printf("ILLEGAL OPCODE CALL %0.2X \n", current_running_opcode);
			// treat as nop
			is_executing_instruction = false;
			break;
		}
	}
}
void CPU::instructionHandlerCB()
{
	//fetch cb instruction
	if (!is_executing_cb)
	{
		current_running_cb = getByteFromPC();
		is_executing_cb = true;
		return;
	}

	switch (current_running_cb)
	{
	case 0x00: { ins_RLC(&registers.b); } break;
	case 0x01: { ins_RLC(&registers.c); } break;
	case 0x02: { ins_RLC(&registers.d); } break;
	case 0x03: { ins_RLC(&registers.e); } break;
	case 0x04: { ins_RLC(&registers.h); } break;
	case 0x05: { ins_RLC(&registers.l); } break;
	case 0x06: { ins_RLC_bHLb(); } break;
	case 0x07: { ins_RLC(&registers.a); } break;

	case 0x08: { ins_RRC(&registers.b); } break;
	case 0x09: { ins_RRC(&registers.c); } break;
	case 0x0A: { ins_RRC(&registers.d); } break;
	case 0x0B: { ins_RRC(&registers.e); } break;
	case 0x0C: { ins_RRC(&registers.h); } break;
	case 0x0D: { ins_RRC(&registers.l); } break;
	case 0x0E: { ins_RRC_bHLb(); } break;
	case 0x0F: { ins_RRC(&registers.a); } break;

	case 0x10: { ins_RL(&registers.b); } break;
	case 0x11: { ins_RL(&registers.c); } break;
	case 0x12: { ins_RL(&registers.d); } break;
	case 0x13: { ins_RL(&registers.e); } break;
	case 0x14: { ins_RL(&registers.h); } break;
	case 0x15: { ins_RL(&registers.l); } break;
	case 0x16: { ins_RL_bHLb(); } break;
	case 0x17: { ins_RL(&registers.a); } break;

	case 0x18: { ins_RR(&registers.b); } break;
	case 0x19: { ins_RR(&registers.c); } break;
	case 0x1A: { ins_RR(&registers.d); } break;
	case 0x1B: { ins_RR(&registers.e); } break;
	case 0x1C: { ins_RR(&registers.h); } break;
	case 0x1D: { ins_RR(&registers.l); } break;
	case 0x1E: { ins_RR_bHLb(); } break;
	case 0x1F: { ins_RR(&registers.a); } break;

	case 0x20: { ins_SLA(&registers.b); } break;
	case 0x21: { ins_SLA(&registers.c); } break;
	case 0x22: { ins_SLA(&registers.d); } break;
	case 0x23: { ins_SLA(&registers.e); } break;
	case 0x24: { ins_SLA(&registers.h); } break;
	case 0x25: { ins_SLA(&registers.l); } break;
	case 0x26: { ins_SLA_bHLb(); } break;
	case 0x27: { ins_SLA(&registers.a); } break;

	case 0x28: { ins_SRA(&registers.b); } break;
	case 0x29: { ins_SRA(&registers.c); } break;
	case 0x2A: { ins_SRA(&registers.d); } break;
	case 0x2B: { ins_SRA(&registers.e); } break;
	case 0x2C: { ins_SRA(&registers.h); } break;
	case 0x2D: { ins_SRA(&registers.l); } break;
	case 0x2E: { ins_SRA_bHLb(); } break;
	case 0x2F: { ins_SRA(&registers.a); } break;

	case 0x30: { ins_SWAP(&registers.b); } break;
	case 0x31: { ins_SWAP(&registers.c); } break;
	case 0x32: { ins_SWAP(&registers.d); } break;
	case 0x33: { ins_SWAP(&registers.e); } break;
	case 0x34: { ins_SWAP(&registers.h); } break;
	case 0x35: { ins_SWAP(&registers.l); } break;
	case 0x36: { ins_SWAP_bHLb(); } break;
	case 0x37: { ins_SWAP(&registers.a); } break;

	case 0x38: { ins_SRL(&registers.b); } break;
	case 0x39: { ins_SRL(&registers.c); } break;
	case 0x3A: { ins_SRL(&registers.d); } break;
	case 0x3B: { ins_SRL(&registers.e); } break;
	case 0x3C: { ins_SRL(&registers.h); } break;
	case 0x3D: { ins_SRL(&registers.l); } break;
	case 0x3E: { ins_SRL_bHLb(); } break;
	case 0x3F: { ins_SRL(&registers.a); } break;

	case 0x40: { ins_BIT_b_r(0, &registers.b); } break;
	case 0x41: { ins_BIT_b_r(0, &registers.c); } break;
	case 0x42: { ins_BIT_b_r(0, &registers.d); } break;
	case 0x43: { ins_BIT_b_r(0, &registers.e); } break;
	case 0x44: { ins_BIT_b_r(0, &registers.h); } break;
	case 0x45: { ins_BIT_b_r(0, &registers.l); } break;
	case 0x46: { ins_BIT_b_r_bHLb(0); } break;
	case 0x47: { ins_BIT_b_r(0, &registers.a); } break;

	case 0x48: { ins_BIT_b_r(1, &registers.b); } break;
	case 0x49: { ins_BIT_b_r(1, &registers.c); } break;
	case 0x4A: { ins_BIT_b_r(1, &registers.d); } break;
	case 0x4B: { ins_BIT_b_r(1, &registers.e); } break;
	case 0x4C: { ins_BIT_b_r(1, &registers.h); } break;
	case 0x4D: { ins_BIT_b_r(1, &registers.l); } break;
	case 0x4E: { ins_BIT_b_r_bHLb(1); } break;
	case 0x4F: { ins_BIT_b_r(1, &registers.a); } break;

	case 0x50: { ins_BIT_b_r(2, &registers.b); } break;
	case 0x51: { ins_BIT_b_r(2, &registers.c); } break;
	case 0x52: { ins_BIT_b_r(2, &registers.d); } break;
	case 0x53: { ins_BIT_b_r(2, &registers.e); } break;
	case 0x54: { ins_BIT_b_r(2, &registers.h); } break;
	case 0x55: { ins_BIT_b_r(2, &registers.l); } break;
	case 0x56: { ins_BIT_b_r_bHLb(2); } break;
	case 0x57: { ins_BIT_b_r(2, &registers.a); } break;

	case 0x58: { ins_BIT_b_r(3, &registers.b); } break;
	case 0x59: { ins_BIT_b_r(3, &registers.c); } break;
	case 0x5A: { ins_BIT_b_r(3, &registers.d); } break;
	case 0x5B: { ins_BIT_b_r(3, &registers.e); } break;
	case 0x5C: { ins_BIT_b_r(3, &registers.h); } break;
	case 0x5D: { ins_BIT_b_r(3, &registers.l); } break;
	case 0x5E: { ins_BIT_b_r_bHLb(3); } break;
	case 0x5F: { ins_BIT_b_r(3, &registers.a); } break;

	case 0x60: { ins_BIT_b_r(4, &registers.b); } break;
	case 0x61: { ins_BIT_b_r(4, &registers.c); } break;
	case 0x62: { ins_BIT_b_r(4, &registers.d); } break;
	case 0x63: { ins_BIT_b_r(4, &registers.e); } break;
	case 0x64: { ins_BIT_b_r(4, &registers.h); } break;
	case 0x65: { ins_BIT_b_r(4, &registers.l); } break;
	case 0x66: { ins_BIT_b_r_bHLb(4); } break;
	case 0x67: { ins_BIT_b_r(4, &registers.a); } break;

	case 0x68: { ins_BIT_b_r(5, &registers.b); } break;
	case 0x69: { ins_BIT_b_r(5, &registers.c); } break;
	case 0x6A: { ins_BIT_b_r(5, &registers.d); } break;
	case 0x6B: { ins_BIT_b_r(5, &registers.e); } break;
	case 0x6C: { ins_BIT_b_r(5, &registers.h); } break;
	case 0x6D: { ins_BIT_b_r(5, &registers.l); } break;
	case 0x6E: { ins_BIT_b_r_bHLb(5); } break;
	case 0x6F: { ins_BIT_b_r(5, &registers.a); } break;

	case 0x70: { ins_BIT_b_r(6, &registers.b); } break;
	case 0x71: { ins_BIT_b_r(6, &registers.c); } break;
	case 0x72: { ins_BIT_b_r(6, &registers.d); } break;
	case 0x73: { ins_BIT_b_r(6, &registers.e); } break;
	case 0x74: { ins_BIT_b_r(6, &registers.h); } break;
	case 0x75: { ins_BIT_b_r(6, &registers.l); } break;
	case 0x76: { ins_BIT_b_r_bHLb(6); } break;
	case 0x77: { ins_BIT_b_r(6, &registers.a); } break;

	case 0x78: { ins_BIT_b_r(7, &registers.b); } break;
	case 0x79: { ins_BIT_b_r(7, &registers.c); } break;
	case 0x7A: { ins_BIT_b_r(7, &registers.d); } break;
	case 0x7B: { ins_BIT_b_r(7, &registers.e); } break;
	case 0x7C: { ins_BIT_b_r(7, &registers.h); } break;
	case 0x7D: { ins_BIT_b_r(7, &registers.l); } break;
	case 0x7E: { ins_BIT_b_r_bHLb(7); } break;
	case 0x7F: { ins_BIT_b_r(7, &registers.a); } break;

	case 0x80: { ins_RES_b_r(0, &registers.b); } break;
	case 0x81: { ins_RES_b_r(0, &registers.c); } break;
	case 0x82: { ins_RES_b_r(0, &registers.d); } break;
	case 0x83: { ins_RES_b_r(0, &registers.e); } break;
	case 0x84: { ins_RES_b_r(0, &registers.h); } break;
	case 0x85: { ins_RES_b_r(0, &registers.l); } break;
	case 0x86: { ins_RES_b_r_bHLb(0); } break;
	case 0x87: { ins_RES_b_r(0, &registers.a); } break;

	case 0x88: { ins_RES_b_r(1, &registers.b); } break;
	case 0x89: { ins_RES_b_r(1, &registers.c); } break;
	case 0x8A: { ins_RES_b_r(1, &registers.d); } break;
	case 0x8B: { ins_RES_b_r(1, &registers.e); } break;
	case 0x8C: { ins_RES_b_r(1, &registers.h); } break;
	case 0x8D: { ins_RES_b_r(1, &registers.l); } break;
	case 0x8E: { ins_RES_b_r_bHLb(1); } break;
	case 0x8F: { ins_RES_b_r(1, &registers.a); } break;

	case 0x90: { ins_RES_b_r(2, &registers.b); } break;
	case 0x91: { ins_RES_b_r(2, &registers.c); } break;
	case 0x92: { ins_RES_b_r(2, &registers.d); } break;
	case 0x93: { ins_RES_b_r(2, &registers.e); } break;
	case 0x94: { ins_RES_b_r(2, &registers.h); } break;
	case 0x95: { ins_RES_b_r(2, &registers.l); } break;
	case 0x96: { ins_RES_b_r_bHLb(2); } break;
	case 0x97: { ins_RES_b_r(2, &registers.a); } break;

	case 0x98: { ins_RES_b_r(3, &registers.b); } break;
	case 0x99: { ins_RES_b_r(3, &registers.c); } break;
	case 0x9A: { ins_RES_b_r(3, &registers.d); } break;
	case 0x9B: { ins_RES_b_r(3, &registers.e); } break;
	case 0x9C: { ins_RES_b_r(3, &registers.h); } break;
	case 0x9D: { ins_RES_b_r(3, &registers.l); } break;
	case 0x9E: { ins_RES_b_r_bHLb(3); } break;
	case 0x9F: { ins_RES_b_r(3, &registers.a); } break;

	case 0xA0: { ins_RES_b_r(4, &registers.b); } break;
	case 0xA1: { ins_RES_b_r(4, &registers.c); } break;
	case 0xA2: { ins_RES_b_r(4, &registers.d); } break;
	case 0xA3: { ins_RES_b_r(4, &registers.e); } break;
	case 0xA4: { ins_RES_b_r(4, &registers.h); } break;
	case 0xA5: { ins_RES_b_r(4, &registers.l); } break;
	case 0xA6: { ins_RES_b_r_bHLb(4); } break;
	case 0xA7: { ins_RES_b_r(4, &registers.a); } break;

	case 0xA8: { ins_RES_b_r(5, &registers.b); } break;
	case 0xA9: { ins_RES_b_r(5, &registers.c); } break;
	case 0xAA: { ins_RES_b_r(5, &registers.d); } break;
	case 0xAB: { ins_RES_b_r(5, &registers.e); } break;
	case 0xAC: { ins_RES_b_r(5, &registers.h); } break;
	case 0xAD: { ins_RES_b_r(5, &registers.l); } break;
	case 0xAE: { ins_RES_b_r_bHLb(5); } break;
	case 0xAF: { ins_RES_b_r(5, &registers.a); } break;

	case 0xB0: { ins_RES_b_r(6, &registers.b); } break;
	case 0xB1: { ins_RES_b_r(6, &registers.c); } break;
	case 0xB2: { ins_RES_b_r(6, &registers.d); } break;
	case 0xB3: { ins_RES_b_r(6, &registers.e); } break;
	case 0xB4: { ins_RES_b_r(6, &registers.h); } break;
	case 0xB5: { ins_RES_b_r(6, &registers.l); } break;
	case 0xB6: { ins_RES_b_r_bHLb(6); } break;
	case 0xB7: { ins_RES_b_r(6, &registers.a); } break;

	case 0xB8: { ins_RES_b_r(7, &registers.b); } break;
	case 0xB9: { ins_RES_b_r(7, &registers.c); } break;
	case 0xBA: { ins_RES_b_r(7, &registers.d); } break;
	case 0xBB: { ins_RES_b_r(7, &registers.e); } break;
	case 0xBC: { ins_RES_b_r(7, &registers.h); } break;
	case 0xBD: { ins_RES_b_r(7, &registers.l); } break;
	case 0xBE: { ins_RES_b_r_bHLb(7); } break;
	case 0xBF: { ins_RES_b_r(7, &registers.a); } break;

	case 0xC0: { ins_SET_b_r(0, &registers.b); } break;
	case 0xC1: { ins_SET_b_r(0, &registers.c); } break;
	case 0xC2: { ins_SET_b_r(0, &registers.d); } break;
	case 0xC3: { ins_SET_b_r(0, &registers.e); } break;
	case 0xC4: { ins_SET_b_r(0, &registers.h); } break;
	case 0xC5: { ins_SET_b_r(0, &registers.l); } break;
	case 0xC6: { ins_SET_b_r_bHLb(0); } break;
	case 0xC7: { ins_SET_b_r(0, &registers.a); } break;

	case 0xC8: { ins_SET_b_r(1, &registers.b); } break;
	case 0xC9: { ins_SET_b_r(1, &registers.c); } break;
	case 0xCA: { ins_SET_b_r(1, &registers.d); } break;
	case 0xCB: { ins_SET_b_r(1, &registers.e); } break;
	case 0xCC: { ins_SET_b_r(1, &registers.h); } break;
	case 0xCD: { ins_SET_b_r(1, &registers.l); } break;
	case 0xCE: { ins_SET_b_r_bHLb(1); } break;
	case 0xCF: { ins_SET_b_r(1, &registers.a); } break;

	case 0xD0: { ins_SET_b_r(2, &registers.b); } break;
	case 0xD1: { ins_SET_b_r(2, &registers.c); } break;
	case 0xD2: { ins_SET_b_r(2, &registers.d); } break;
	case 0xD3: { ins_SET_b_r(2, &registers.e); } break;
	case 0xD4: { ins_SET_b_r(2, &registers.h); } break;
	case 0xD5: { ins_SET_b_r(2, &registers.l); } break;
	case 0xD6: { ins_SET_b_r_bHLb(2); } break;
	case 0xD7: { ins_SET_b_r(2, &registers.a); } break;

	case 0xD8: { ins_SET_b_r(3, &registers.b); } break;
	case 0xD9: { ins_SET_b_r(3, &registers.c); } break;
	case 0xDA: { ins_SET_b_r(3, &registers.d); } break;
	case 0xDB: { ins_SET_b_r(3, &registers.e); } break;
	case 0xDC: { ins_SET_b_r(3, &registers.h); } break;
	case 0xDD: { ins_SET_b_r(3, &registers.l); } break;
	case 0xDE: { ins_SET_b_r_bHLb(3); } break;
	case 0xDF: { ins_SET_b_r(3, &registers.a); } break;

	case 0xE0: { ins_SET_b_r(4, &registers.b); } break;
	case 0xE1: { ins_SET_b_r(4, &registers.c); } break;
	case 0xE2: { ins_SET_b_r(4, &registers.d); } break;
	case 0xE3: { ins_SET_b_r(4, &registers.e); } break;
	case 0xE4: { ins_SET_b_r(4, &registers.h); } break;
	case 0xE5: { ins_SET_b_r(4, &registers.l); } break;
	case 0xE6: { ins_SET_b_r_bHLb(4); } break;
	case 0xE7: { ins_SET_b_r(4, &registers.a); } break;

	case 0xE8: { ins_SET_b_r(5, &registers.b); } break;
	case 0xE9: { ins_SET_b_r(5, &registers.c); } break;
	case 0xEA: { ins_SET_b_r(5, &registers.d); } break;
	case 0xEB: { ins_SET_b_r(5, &registers.e); } break;
	case 0xEC: { ins_SET_b_r(5, &registers.h); } break;
	case 0xED: { ins_SET_b_r(5, &registers.l); } break;
	case 0xEE: { ins_SET_b_r_bHLb(5); } break;
	case 0xEF: { ins_SET_b_r(5, &registers.a); } break;

	case 0xF0: { ins_SET_b_r(6, &registers.b); } break;
	case 0xF1: { ins_SET_b_r(6, &registers.c); } break;
	case 0xF2: { ins_SET_b_r(6, &registers.d); } break;
	case 0xF3: { ins_SET_b_r(6, &registers.e); } break;
	case 0xF4: { ins_SET_b_r(6, &registers.h); } break;
	case 0xF5: { ins_SET_b_r(6, &registers.l); } break;
	case 0xF6: { ins_SET_b_r_bHLb(6); } break;
	case 0xF7: { ins_SET_b_r(6, &registers.a); } break;

	case 0xF8: { ins_SET_b_r(7, &registers.b); } break;
	case 0xF9: { ins_SET_b_r(7, &registers.c); } break;
	case 0xFA: { ins_SET_b_r(7, &registers.d); } break;
	case 0xFB: { ins_SET_b_r(7, &registers.e); } break;
	case 0xFC: { ins_SET_b_r(7, &registers.h); } break;
	case 0xFD: { ins_SET_b_r(7, &registers.l); } break;
	case 0xFE: { ins_SET_b_r_bHLb(7); } break;
	case 0xFF: { ins_SET_b_r(7, &registers.a); } break;
	}
}
void CPU::instructionHandlerSTOP()
{

	printf("STOP CALLED\n");

	if (mcycles_used == 1)
	{
		setMemory(DIV, 0);
		if (getMemory(0xFF00))
			is_executing_instruction = false;
		return;
	}

	// first m-cycle
	if (getMemory(0xFF00))
	{
		if ((getMemory(IF_REGISTER) & getMemory(IE_REGISTER)) != 0)
		{
			is_executing_instruction = false;
			return;
		}
		// stop is a two byte opcode, halt mode entered, div not reset
		current_running_opcode = 0x76;
		return;
	}
	// if (speed_switch_requested){}


	if ((getMemory(IF_REGISTER) & getMemory(IE_REGISTER)) != 0)
	{
		// stop one byte opcode, stop mode is entered, div is reset
		return;
	}
	mcycles_used++;
	//stop is a 2 byte opcode
	//stop mode is entrered
	//div is reset
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
	switch (mcycles_used)
	{
	case 0:
	{
		//logic

		mcycles_used++;
		break;
	}

	case 1:
	{
		//logic

		mcycles_used++;
		break;
	}

	case 2:
	{
		//logic

		is_executing_instruction = false;
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
void CPU::ins_LD_XX_u16(Byte* const reigster_one, Byte* const reigster_two)
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: *reigster_two = getByteFromPC(); mcycles_used++; break;
	case 2: *reigster_one = getByteFromPC(); is_executing_instruction = false;
	}
}

// 0x031
void CPU::ins_LD_SP_u16(Word* const register_word)
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: *register_word = (*register_word & 0xFF00) | getByteFromPC(); mcycles_used++; break;
	case 2: *register_word = (*register_word & 0x00FF) | (getByteFromPC() << 0x8); is_executing_instruction = false;
	}
}

// 0x02, 0x12, 0x70 - 0x75, 0x77, 0xE2
void CPU::ins_LD_bXXb_Y(const Word register_wordvalue, Byte* const register_byte)
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: setMemory(register_wordvalue, *register_byte); is_executing_instruction = false;
	}
}

// 0x22, 0x32
void CPU::ins_LD_bHLb_Apm(bool add)
{

	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1:
	{
		Word hl = registers.getHL();
		setMemory(hl, registers.a);
		add ? registers.setHL(hl + 1) : registers.setHL(hl - 1);
		is_executing_instruction = false;
	}
	}
}

void CPU::ins_INC_XX(Byte* reigster_one, Byte* reigster_two)
{
	switch (mcycles_used)
	{
	case 0:
	{
		Word newValue = registers.getWord(reigster_one, reigster_two) + 1;
		instruction_cache[0] = (newValue & 0xFF00) >> 8;
		*reigster_two = newValue & 0x00FF;
		mcycles_used++;
		break;
	}
	case 1:
		*reigster_one = (instruction_cache[0]);
		is_executing_instruction = false;
	}
}

void CPU::ins_INC_SP()
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: registers.sp++; is_executing_instruction = false;
	}
}

void CPU::ins_INC_X(Byte* register_byte)
{
	registers.setFlag(h, checkCarry(*register_byte, 1, 4));

	(*register_byte)++;

	(*register_byte == 0x0) ? registers.setFlag(z, 1) : registers.setFlag(z, 0);
	registers.setFlag(n, 0);

	is_executing_instruction = false;
}

void CPU::ins_INC_bHLb()
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: instruction_cache[0] = getMemory(registers.getHL()); mcycles_used++; break;
	case 2:
		Word address = registers.getHL();
		Byte temp = instruction_cache[0];

		setMemory(address, (temp + 1));

		registers.setFlag(h, checkCarry(temp, 1, 4));
		registers.setFlag(z, (getMemory(address) == 0x0));
		registers.setFlag(n, 0);

		is_executing_instruction = false;
	}
}

void CPU::ins_DEC_X(Byte* register_byte)
{
	registers.setFlag(h, checkBorrow(*register_byte, 1, 4));
	(*register_byte)--;

	registers.setFlag(z, (*register_byte == 0x0));
	registers.setFlag(n, 1);

	is_executing_instruction = false;
}

void CPU::ins_DEC_bHLb()
{
	switch (mcycles_used)
	{
	case 0:  mcycles_used++; break;
	case 1:  mcycles_used++; break;
	case 2:
		Word address = registers.getHL();
		Byte temp = getMemory(address);

		registers.setFlag(h, checkBorrow(temp, 1, 4));
		setMemory(address, (temp - 1));
		registers.setFlag(z, (getMemory(address) == 0x0));
		registers.setFlag(n, 1);

		is_executing_instruction = false;
	}
}

void CPU::ins_LD_X_u8(Byte* const register_byte)
{
	switch (mcycles_used)
	{
	case 0:  mcycles_used++; break;
	case 1: *register_byte = getByteFromPC(); is_executing_instruction = false;
	}
}

// 0x36
void CPU::ins_LD_bHLb_u8()
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: mcycles_used++; break;
	case 2:
		setMemory(registers.getHL(), getByteFromPC());
		is_executing_instruction = false;
	}
}


void CPU::ins_CPL()
{
	registers.a = ~registers.a;
	registers.setFlag(n, true);
	registers.setFlag(h, true);
	is_executing_instruction = false;
}

void CPU::ins_CCF()
{
	registers.setFlag(n, 0);
	registers.setFlag(h, 0);
	registers.setFlag(c, !registers.getFlag(c));
	is_executing_instruction = false;
}

void CPU::ins_SCF()
{
	registers.setFlag(n, 0);
	registers.setFlag(h, 0);
	registers.setFlag(c, 1);
	is_executing_instruction = false;
}

void CPU::ins_RLCA()
{
	//set c flag to whatever is the value of the leftest bit from register a
	registers.setFlag(c, (registers.a & 0x80));
	registers.setFlag(n, 0);
	registers.setFlag(h, 0);

	// move everything to the left by one, toggle bit 0 with bit 7 shifted right 7 places
	registers.a = (registers.a << 1) | (registers.a >> (7));

	// z is reset
	registers.setFlag(z, 0);

	is_executing_instruction = false;
}

void CPU::ins_RLA()
{
	// swap leftest most bit with the carry flag, then rotate to the left

	Byte flagCarry = registers.getFlag(c);
	bool registerCarry = ((registers.a & 0x80) >> 7);

	registers.a = (registers.a << 1) | (flagCarry);

	registers.setFlag(c, registerCarry);
	registers.setFlag(n, 0);
	registers.setFlag(h, 0);
	registers.setFlag(z, 0);


	is_executing_instruction = false;
}

void CPU::ins_RRCA()
{
	registers.setFlag(c, registers.a & 0x1);

	registers.a = (registers.a >> 1) | (registers.a << (7));

	registers.setFlag(n, 0);
	registers.setFlag(h, 0);
	registers.setFlag(z, 0);


	is_executing_instruction = false;
}

void CPU::ins_RRA()
{
	Byte flagCarry = registers.getFlag(c);
	bool registerCarry = (registers.a & 0x1);

	registers.a = (registers.a >> 1) | (flagCarry << (7));

	registers.setFlag(c, registerCarry);
	registers.setFlag(n, 0);
	registers.setFlag(h, 0);
	registers.setFlag(z, 0);


	is_executing_instruction = false;
}

//0x08
void CPU::ins_LD_bu16b_SP()
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: instruction_cache[0] = getByteFromPC(); mcycles_used++; break;
	case 2: instruction_cache[1] = getByteFromPC(); mcycles_used++; break;
	case 3:
	{
		Word address = (instruction_cache[1] << 8) | instruction_cache[0];
		if (instruction_cache[1] == 0x91 || instruction_cache[1] == 0xFF)
		{
			printf("");
		}
		setMemory(address, (registers.sp & 0xFF));
		mcycles_used++; break;
	}
	case 4:
	{
		Word address = (instruction_cache[1] << 8) | instruction_cache[0];
	
		setMemory(address + 1, (registers.sp >> 8));
		is_executing_instruction = false;
	}
	}
}

// 0x18
void CPU::ins_JR_i8(const enum eJumpCondition condition)
{
	switch (mcycles_used)
	{
	case 0:  mcycles_used++; break;
	case 1:
		instruction_cache[0] = getByteFromPC();
		checkJumpCondition(condition) ? mcycles_used++ : is_executing_instruction = false;
		break;
	case 2:
		registers.pc += (Byte_s)instruction_cache[0];
		is_executing_instruction = false;
	}
}

void CPU::ins_ADD_HL_XX(Byte* const reigster_one, Byte* const reigster_two)
{
	switch (mcycles_used)
	{
	case 0:
	{
		Word HLvalue = registers.getHL();
		Word register_word = (*reigster_one << 8) | *reigster_two;
		Word sum = HLvalue + register_word;
		instruction_cache[0] = sum >> 8;

		registers.setFlag(c, checkCarry(HLvalue, register_word, 16));
		registers.setFlag(h, checkCarry(HLvalue, register_word, 12));
		registers.setFlag(n, 0);
		registers.l = sum & 0xFF;

		mcycles_used++;
		break;
	}
	case 1:
		registers.h = instruction_cache[0];
		is_executing_instruction = false;
	}
}

void CPU::ins_ADD_HL_SP()
{
	switch (mcycles_used)
	{
	case 0:
	{
		Word HLvalue = registers.getHL();
		Word sum = HLvalue + registers.sp;
		instruction_cache[0] = sum >> 8;

		registers.setFlag(c, checkCarry(HLvalue, registers.sp, 16));
		registers.setFlag(h, checkCarry(HLvalue, registers.sp, 12));
		registers.setFlag(n, 0);
		registers.l = sum & 0xFF;

		mcycles_used++; break;
	}
	case 1:
		registers.h = instruction_cache[0];
		is_executing_instruction = false;
	}
}

void CPU::ins_ADD_A_X(Byte* const register_byte)
{
	registers.setFlag(c, checkCarry(registers.a, *register_byte, 8));
	registers.setFlag(h, checkCarry(registers.a, *register_byte, 4));

	// perform addition
	registers.a += *register_byte;

	//evaluate z flag an clear the n flag
	registers.setFlag(z, (registers.a == 0x0));
	registers.setFlag(n, 0);

	is_executing_instruction = false;
}

void CPU::ins_ADD_A_bHLb()
{
	switch (mcycles_used)
	{
	case 0:  mcycles_used++; break;
	case 1:
		Byte immediateValue = getMemory(registers.getHL());
		registers.setFlag(c, checkCarry(registers.a, immediateValue, 8));
		registers.setFlag(h, checkCarry(registers.a, immediateValue, 4));

		// perform addition
		registers.a += immediateValue;

		//evaluate z flag an clear the n flag
		registers.setFlag(z, (registers.a == 0x0));
		registers.setFlag(n, 0);

		is_executing_instruction = false;
	}
}

void CPU::ins_ADD_A_u8()
{
	switch (mcycles_used)
	{
	case 0:	mcycles_used++; break;
	case 1:
		Byte immediateValue = getByteFromPC();
		registers.setFlag(c, checkCarry(registers.a, immediateValue, 8));
		registers.setFlag(h, checkCarry(registers.a, immediateValue, 4));

		// perform addition
		registers.a += immediateValue;

		//evaluate z flag an clear the n flag
		registers.setFlag(z, (registers.a == 0x0));
		registers.setFlag(n, 0);

		is_executing_instruction = false;
	}
}

void CPU::ins_LD_X_Y(Byte* const reigster_one, Byte* const reigster_two)
{
	*reigster_one = *reigster_two;
	is_executing_instruction = false;
}

void CPU::ins_LD_X_bYYb(Byte* const left_register, Byte* const rightreigster_one, Byte* const rightreigster_two, const Byte_s add_to_hl)
{
	switch (mcycles_used)
	{
	case 0:  mcycles_used++; break;
	case 1:
		*left_register = getMemory((*rightreigster_one << 8) | *rightreigster_two);
		if (add_to_hl != NULL)
			registers.setHL(registers.getHL() + add_to_hl);
		is_executing_instruction = false;
	}
}

void CPU::ins_DEC_XX(Byte* reigster_one, Byte* reigster_two)
{
	switch (mcycles_used)
	{
	case 0:
	{
		Word sum = registers.getWord(reigster_one, reigster_two) - 1;
		instruction_cache[0] = sum >> 8;
		*reigster_two = (sum & 0xFF);
		mcycles_used++; break;
	}
	case 1:
		*reigster_one = instruction_cache[0];
		is_executing_instruction = false;
	}
}

void CPU::ins_DEC_SP()
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: registers.sp--; is_executing_instruction = false;
	}
}

void CPU::ins_ADC_A_X(const Byte* register_byte)
{
	Byte a = registers.a;
	Byte b = *register_byte;
	Byte C = (int)registers.getFlag(c);

	registers.a = a + b + C;

	registers.setFlag(z, (registers.a == 0x0));
	registers.setFlag(n, 0);
	registers.setFlag(c, checkCarry(a, b, 8, C));
	registers.setFlag(h, checkCarry(a, b, 4, C));

	is_executing_instruction = false;
}

void CPU::ins_ADC_A_bHLb()
{
	switch (mcycles_used)
	{
	case 0:  mcycles_used++; break;
	case 1:
		Byte a = registers.a;
		Byte b = getMemory(registers.getHL());
		Byte C = (int)registers.getFlag(c);

		registers.a = a + b + C;

		registers.setFlag(z, (registers.a == 0x0));
		registers.setFlag(n, 0);
		registers.setFlag(c, checkCarry(a, b, 8, C));
		registers.setFlag(h, checkCarry(a, b, 4, C));

		is_executing_instruction = false;
	}
}

void CPU::ins_ADC_A_u8()
{
	switch (mcycles_used)
	{
	case 0:  mcycles_used++; break;
	case 1:
		Byte a = registers.a;
		Byte b = getByteFromPC();
		Byte C = (int)registers.getFlag(c);

		registers.a = a + b + C;

		registers.setFlag(z, (registers.a == 0x0));
		registers.setFlag(n, 0);
		registers.setFlag(c, checkCarry(a, b, 8, C));
		registers.setFlag(h, checkCarry(a, b, 4, C));

		is_executing_instruction = false;
	}
}


void CPU::ins_SUB_A_X(Byte* const register_byte)
{
	registers.setFlag(c, checkBorrow(registers.a, *register_byte, 8));
	registers.setFlag(h, checkBorrow(registers.a, *register_byte, 4));

	// perform addition
	registers.a -= *register_byte;

	//evaluate z flag an clear the n flag
	registers.setFlag(z, (registers.a == 0x0));
	registers.setFlag(n, 1);

	is_executing_instruction = false;
}

void CPU::ins_SUB_A_u8()
{
	switch (mcycles_used)
	{
	case 0:  mcycles_used++; break;
	case 1:

		Byte immediateByte = getByteFromPC();

		registers.setFlag(c, checkBorrow(registers.a, immediateByte, 8));
		registers.setFlag(h, checkBorrow(registers.a, immediateByte, 4));

		// perform addition
		registers.a -= immediateByte;

		//evaluate z flag an clear the n flag
		registers.setFlag(z, (registers.a == 0x0));
		registers.setFlag(n, 1);

		is_executing_instruction = false;
	}
}


void CPU::ins_SUB_A_bHLb()
{
	switch (mcycles_used)
	{
	case 0:  mcycles_used++; break;
	case 1:

		Byte immediateByte = getMemory(registers.getHL());

		registers.setFlag(c, checkBorrow(registers.a, immediateByte, 8));
		registers.setFlag(h, checkBorrow(registers.a, immediateByte, 4));

		// perform addition
		registers.a -= immediateByte;

		//evaluate z flag an clear the n flag
		registers.setFlag(z, (registers.a == 0x0));
		registers.setFlag(n, 1);
		is_executing_instruction = false;
	}
}


void CPU::ins_SBC_A_X(Byte* const register_byte)
{
	Byte a = registers.a;
	Byte b = *register_byte;
	Byte C = (int)registers.getFlag(c);

	registers.a = a - b - C;

	registers.setFlag(z, (registers.a == 0x0));
	registers.setFlag(n, 1);
	registers.setFlag(c, checkBorrow(a, b, 8, C));
	registers.setFlag(h, checkBorrow(a, b, 4, C));


	is_executing_instruction = false;
}

void CPU::ins_SBC_A_u8()
{
	switch (mcycles_used)
	{
	case 0:  mcycles_used++; break;
	case 1:
		Byte a = registers.a;
		Byte b = getByteFromPC();
		Byte C = (int)registers.getFlag(c);

		registers.a = a - b - C;

		registers.setFlag(z, (registers.a == 0x0));
		registers.setFlag(n, 1);
		registers.setFlag(c, checkBorrow(a, b, 8, C));
		registers.setFlag(h, checkBorrow(a, b, 4, C));
		is_executing_instruction = false;
	}
}


void CPU::ins_SBC_A_bHLb()
{
	switch (mcycles_used)
	{
	case 0:  mcycles_used++; break;
	case 1:
		Byte a = registers.a;
		Byte b = getMemory(registers.getHL());
		Byte C = (int)registers.getFlag(c);

		registers.a = a - b - C;

		registers.setFlag(z, (registers.a == 0x0));
		registers.setFlag(n, 1);
		registers.setFlag(c, checkBorrow(a, b, 8, C));
		registers.setFlag(h, checkBorrow(a, b, 4, C));

		is_executing_instruction = false;
	}
}

void CPU::ins_AND_A_X(Byte* const register_byte)
{
	registers.a &= *register_byte;

	//evaluate z flag an clear the n flag
	registers.setFlag(z, (registers.a == 0x0));
	registers.setFlag(h, 1);
	registers.setFlag(n, 0);
	registers.setFlag(c, 0);

	is_executing_instruction = false;
}

void CPU::ins_AND_A_u8()
{
	switch (mcycles_used)
	{
	case 0:  mcycles_used++; break;
	case 1:
		registers.a &= getByteFromPC();

		//evaluate z flag an clear the n flag
		registers.setFlag(z, (registers.a == 0x0));
		registers.setFlag(h, 1);
		registers.setFlag(n, 0);
		registers.setFlag(c, 0);

		is_executing_instruction = false;
	}
}


void CPU::ins_AND_A_bHLb()
{
	switch (mcycles_used)
	{
	case 0:  mcycles_used++; break;
	case 1:
		registers.a &= getMemory(registers.getHL());

		//evaluate z flag an clear the n flag
		registers.setFlag(z, (registers.a == 0x0));
		registers.setFlag(h, 1);
		registers.setFlag(n, 0);
		registers.setFlag(c, 0);

		is_executing_instruction = false;
	}
}

void CPU::ins_XOR_A_X(Byte* const register_byte)
{
	registers.a ^= *register_byte;

	//evaluate z flag an clear the n flag
	registers.setFlag(z, (registers.a == 0x0));
	registers.setFlag(h, 0);
	registers.setFlag(n, 0);
	registers.setFlag(c, 0);

	is_executing_instruction = false;
}

void CPU::ins_XOR_A_u8()
{
	switch (mcycles_used)
	{
	case 0:  mcycles_used++; break;
	case 1:
		registers.a ^= getByteFromPC();

		//evaluate z flag an clear the n flag
		registers.setFlag(z, (registers.a == 0x0));
		registers.setFlag(h, 0);
		registers.setFlag(n, 0);
		registers.setFlag(c, 0);

		is_executing_instruction = false;
	}
}


void CPU::ins_XOR_A_bHLb()
{
	switch (mcycles_used)
	{
	case 0:  mcycles_used++; break;
	case 1:
		registers.a ^= getMemory(registers.getHL());

		//evaluate z flag an clear the n flag
		registers.setFlag(z, (registers.a == 0x0));
		registers.setFlag(h, 0);
		registers.setFlag(n, 0);
		registers.setFlag(c, 0);

		is_executing_instruction = false;
	}
}

void CPU::ins_OR_A_X(Byte* const register_byte)
{
	registers.a |= *register_byte;

	//evaluate z flag an clear the n flag
	registers.setFlag(z, (registers.a == 0x0));
	registers.setFlag(h, 0);
	registers.setFlag(n, 0);
	registers.setFlag(c, 0);


	is_executing_instruction = false;
}

void CPU::ins_OR_A_u8()
{
	switch (mcycles_used)
	{
	case 0:  mcycles_used++; break;
	case 1:
		registers.a |= getByteFromPC();

		//evaluate z flag an clear the n flag
		registers.setFlag(z, (registers.a == 0x0));
		registers.setFlag(h, 0);
		registers.setFlag(n, 0);
		registers.setFlag(c, 0);

		is_executing_instruction = false;
	}
}


void CPU::ins_OR_A_bHLb()
{
	switch (mcycles_used)
	{
	case 0:  mcycles_used++; break;
	case 1:
		registers.a |= getMemory(registers.getHL());

		//evaluate z flag an clear the n flag
		registers.setFlag(z, (registers.a == 0x0));
		registers.setFlag(h, 0);
		registers.setFlag(n, 0);
		registers.setFlag(c, 0);

		is_executing_instruction = false;
	}
}

void CPU::ins_CP_A_X(Byte* const register_byte)
{
	registers.setFlag(c, checkBorrow(registers.a, *register_byte, 8));
	registers.setFlag(h, checkBorrow(registers.a, *register_byte, 4));

	//evaluate z flag an clear the n flag
	registers.setFlag(z, (registers.a == *register_byte));
	registers.setFlag(n, 1);


	is_executing_instruction = false;
}

void CPU::ins_CP_A_u8()
{
	switch (mcycles_used)
	{
	case 0:  mcycles_used++; break;
	case 1:
		Byte immediateValue = getByteFromPC();
		registers.setFlag(c, checkBorrow(registers.a, immediateValue, 8));
		registers.setFlag(h, checkBorrow(registers.a, immediateValue, 4));
		registers.setFlag(z, (registers.a == immediateValue));
		registers.setFlag(n, 1);

		is_executing_instruction = false;
	}
}


void CPU::ins_CP_A_bHLb()
{
	switch (mcycles_used)
	{
	case 0:  mcycles_used++; break;
	case 1:
		Byte immediateValue = getMemory(registers.getHL());
		registers.setFlag(c, checkBorrow(registers.a, immediateValue, 8));
		registers.setFlag(h, checkBorrow(registers.a, immediateValue, 4));
		registers.setFlag(z, (registers.a == immediateValue));
		registers.setFlag(n, 1);

		is_executing_instruction = false;
	}
}


void CPU::ins_POP_XX(Byte* reigster_one, Byte* reigster_two)
{
	switch (mcycles_used)
	{
	case 0:  mcycles_used++; break;
	case 1:
		*reigster_two = getMemory(registers.sp++);
		if (reigster_two == &registers.f)
			*reigster_two &= 0xF0;


		mcycles_used++; break;
	case 2:
		*reigster_one = getMemory(registers.sp++);
		if (reigster_one == &registers.f)
			*reigster_one &= 0xF0;

		is_executing_instruction = false;
	}
}


void CPU::ins_PUSH_XX(const Word reigster_word_value)
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: mcycles_used++; break;
	case 2:
		// move low byte to higher (sp)
		setMemory(--registers.sp, (reigster_word_value >> 8));

		mcycles_used++; break;
	case 3:
		// move high byte to lower (sp)
		setMemory(--registers.sp, (reigster_word_value & 0xff));

		is_executing_instruction = false;
	}
}



void CPU::ins_JP_u16(const enum eJumpCondition condition)
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: instruction_cache[0] = getByteFromPC(); mcycles_used++; break;
	case 2: 
		instruction_cache[1] = getByteFromPC();
		checkJumpCondition(condition) ? mcycles_used++ : is_executing_instruction = false; break;
	case 3:
		registers.pc = (instruction_cache[1] << 8) | instruction_cache[0];
		is_executing_instruction = false;
	}
}

//this and standard RET (and by extension RETI) are completely different, one would expect it to be 3m-4m but instead it is 2m-5m, which is longer than RET's 4m, for this reason I will have to treat it as a different instruction.
void CPU::ins_RET_CC(const enum eJumpCondition condition)
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: checkJumpCondition(condition) ? mcycles_used++ : is_executing_instruction = false; break;
	case 2: instruction_cache[0] = getMemory(registers.sp++); mcycles_used++; break;
	case 3: instruction_cache[1] = getMemory(registers.sp++); mcycles_used++; break;
	case 4: registers.pc = (instruction_cache[1] << 8) | instruction_cache[0]; is_executing_instruction = false;
	}
}

void CPU::ins_RET()
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: instruction_cache[0] = getMemory(registers.sp++); mcycles_used++; break;
	case 2: instruction_cache[1] = getMemory(registers.sp++); mcycles_used++; break;
	case 3: registers.pc = (instruction_cache[1] << 8) | instruction_cache[0]; is_executing_instruction = false;
	}
}

void CPU::ins_RETI()
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: instruction_cache[0] = getMemory(registers.sp++); mcycles_used++; break;
	case 2: instruction_cache[1] = getMemory(registers.sp++); mcycles_used++; break;
	case 3: registers.pc = (instruction_cache[1] << 8) | instruction_cache[0]; interrupt_master_enable = true; is_executing_instruction = false;
	}
}



void CPU::ins_JP_HL()
{
	registers.pc = registers.getHL();
	is_executing_instruction = false;
}

// call is correct
void CPU::ins_CALL_u16(const enum eJumpCondition condition)
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: instruction_cache[0] = getByteFromPC(); mcycles_used++; break;
	case 2: 
		instruction_cache[1] = getByteFromPC();
		checkJumpCondition(condition) ? mcycles_used++ : is_executing_instruction = false; break;
	case 3: mcycles_used++; break; // strange behaviour since a short call is only 3m long
	case 4:
		setMemory(--registers.sp, registers.getLowByte(&registers.pc));
		mcycles_used++; break;
	case 5:
		setMemory(--registers.sp, registers.getHighByte(&registers.pc));
		registers.pc = (instruction_cache[1] << 8) | instruction_cache[0];
		is_executing_instruction = false;
	}
}


void CPU::ins_RST(Byte jumpVector)
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: mcycles_used++; break;
	case 2:
		setMemory(--registers.sp, registers.getLowByte(&registers.pc));
		mcycles_used++; break;
	case 3:
		setMemory(--registers.sp, registers.getHighByte(&registers.pc));
		registers.pc = 0x0000 + jumpVector;
		is_executing_instruction = false;
	}
}

void CPU::ins_LD_bFF00_u8b_A()
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: instruction_cache[0] = getByteFromPC(); mcycles_used++; break;
	case 2:
		setMemory(0xFF00 + instruction_cache[0], registers.a);
		is_executing_instruction = false;
	}
}

void CPU::ins_LD_A_bFF00_u8b()
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: instruction_cache[0] = getByteFromPC(); mcycles_used++; break;
	case 2:
		registers.a = getMemory(0xFF00 + instruction_cache[0]);
		is_executing_instruction = false;
	}
}

void CPU::ins_LD_A_bFF00_Cb()
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: registers.a = getMemory(0xFF00 + registers.c); is_executing_instruction = false;
	}
}


void CPU::ins_ADD_SP_i8()
{
	switch (mcycles_used)
	{
	case 0:  mcycles_used++; break;
	case 1: instruction_cache[0] = getByteFromPC(); mcycles_used++; break;
	case 2:
	{
		Byte_s value = (Byte_s)instruction_cache[0];
		registers.setFlag(h, (registers.sp & 0xF) + (value & 0xF) > 0xF);
		registers.setFlag(c, (registers.sp & 0xFF) + (value & 0xFF) > 0xFF);

		registers.setFlag(z, 0);
		registers.setFlag(n, 0);

		Word sum = (registers.sp + value);
		instruction_cache[1] = sum >> 8;
		registers.setLowByte(&registers.sp, sum & 0xFF);

		mcycles_used++; break;
	}
	case 3:
		registers.setHighByte(&registers.sp, instruction_cache[1]);
		is_executing_instruction = false;
	}
}

void CPU::ins_LD_HL_SP_i8()
{
	switch (mcycles_used)
	{
	case 0:  mcycles_used++; break;
	case 1: instruction_cache[0] = getByteFromPC(); mcycles_used++; break;
	case 2:
		Byte_s value = (Byte_s)instruction_cache[0];
		Word sum = registers.sp + value;
		registers.setFlag(z, 0);
		registers.setFlag(n, 0);
		registers.setFlag(h, (registers.sp & 0xF) + (value & 0xF) > 0xF);
		registers.setFlag(c, (registers.sp & 0xFF) + (value & 0xFF) > 0xFF);

		registers.setHL(sum);

		is_executing_instruction = false;
	}
}

void CPU::ins_LD_SP_HL()
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: registers.sp = registers.getHL(); is_executing_instruction = false;
	}
}

void CPU::ins_LD_bu16b_A()
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: instruction_cache[0] = getByteFromPC(); mcycles_used++; break;
	case 2: instruction_cache[1] = getByteFromPC(); mcycles_used++; break;
	case 3:
		setMemory((instruction_cache[1] << 8) | instruction_cache[0], registers.a);
		is_executing_instruction = false;
	}
}

void CPU::ins_LD_A_bu16b()
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: instruction_cache[0] = getByteFromPC(); mcycles_used++; break;
	case 2: instruction_cache[1] = getByteFromPC(); mcycles_used++; break;
	case 3:
	{
		Word address = (instruction_cache[1] << 8) | instruction_cache[0];
		registers.a = getMemory(address);
		is_executing_instruction = false;
	}
	}
}


void CPU::ins_RLC(Byte* reigster_one)
{
	registers.setFlag(n, 0);
	registers.setFlag(h, 0);
	registers.setFlag(c, ((*reigster_one & 0x80) >> 7));
	*reigster_one = ((*reigster_one << 1) | (*reigster_one >> 7));

	registers.setFlag(z, (*reigster_one == 0));

	is_executing_cb = false;
	is_executing_instruction = false;

}

void CPU::ins_RLC_bHLb()
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: instruction_cache[0] = getMemory(registers.getHL()); mcycles_used++; break;
	case 2:

		registers.setFlag(n, 0);
		registers.setFlag(h, 0);
		Byte temp = instruction_cache[0];
		Byte result = ((temp << 1) | (temp >> 7));
		registers.setFlag(c, ((temp & 0x80) >> 7));
		setMemory(registers.getHL(), result);
		registers.setFlag(z, (result == 0));

		is_executing_cb = false;
		is_executing_instruction = false;
	}
}

void CPU::ins_RL(Byte* reigster_one)
{
	Byte flagCarry = registers.getFlag(c);
	bool newCarry = 0;

	registers.setFlag(n, 0);
	registers.setFlag(h, 0);

	newCarry = ((*reigster_one & 0x80) >> 7);
	*reigster_one = ((*reigster_one << 1) | (flagCarry));

	registers.setFlag(c, newCarry);
	registers.setFlag(z, (*reigster_one == 0));

	is_executing_cb = false;
	is_executing_instruction = false;

}

void CPU::ins_RL_bHLb()
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: instruction_cache[0] = getMemory(registers.getHL()); mcycles_used++; break;
	case 2:

		Byte flagCarry = registers.getFlag(c);
		bool newCarry = 0;

		registers.setFlag(n, 0);
		registers.setFlag(h, 0);

		Byte temp = instruction_cache[0];
		Byte result = ((temp << 1) | (flagCarry));
		newCarry = ((temp & 0x80) >> 7);
		setMemory(registers.getHL(), result);

		registers.setFlag(c, newCarry);
		registers.setFlag(z, (result == 0));

		is_executing_cb = false;
		is_executing_instruction = false;
	}
}


void CPU::ins_RRC(Byte* reigster_one)
{
	registers.setFlag(n, 0);
	registers.setFlag(h, 0);
	registers.setFlag(c, (*reigster_one & 0x1));
	*reigster_one = ((*reigster_one >> 1) | (*reigster_one << 7));

	registers.setFlag(z, (*reigster_one == 0));

	is_executing_cb = false;
	is_executing_instruction = false;

}

void CPU::ins_RRC_bHLb()
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: instruction_cache[0] = getMemory(registers.getHL()); mcycles_used++; break;
	case 2:

		registers.setFlag(n, 0);
		registers.setFlag(h, 0);
		Byte temp = instruction_cache[0];
		Byte result = ((temp >> 1) | (temp << 7));
		registers.setFlag(c, (temp & 0x1));
		setMemory(registers.getHL(), result);
		registers.setFlag(z, (result == 0));

		is_executing_cb = false;
		is_executing_instruction = false;
	}
}

void CPU::ins_RR(Byte* reigster_one)
{
	Byte flagCarry = registers.getFlag(c);
	bool newCarry = 0;

	registers.setFlag(n, 0);
	registers.setFlag(h, 0);

	newCarry = (*reigster_one & 0x1);
	*reigster_one = ((*reigster_one >> 1) | (flagCarry << 7));

	registers.setFlag(c, newCarry);
	registers.setFlag(z, (*reigster_one == 0));

	is_executing_cb = false;
	is_executing_instruction = false;

}

void CPU::ins_RR_bHLb()
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: instruction_cache[0] = getMemory(registers.getHL()); mcycles_used++; break;
	case 2:

		Byte flagCarry = registers.getFlag(c);
		bool newCarry = 0;

		registers.setFlag(n, 0);
		registers.setFlag(h, 0);

		Byte temp = instruction_cache[0];
		Byte result = ((temp >> 1) | (flagCarry << 7));
		newCarry = (temp & 0x01);
		setMemory(registers.getHL(), result);

		registers.setFlag(c, newCarry);
		registers.setFlag(z, (result == 0));

		is_executing_cb = false;
		is_executing_instruction = false;
	}
}

void CPU::ins_SLA(Byte* reigster_one)
{
	registers.setFlag(n, 0);
	registers.setFlag(h, 0);

	registers.setFlag(c, (*reigster_one & 0x80) >> 7);
	*reigster_one = *reigster_one << 1;
	registers.setFlag(z, (*reigster_one == 0));

	is_executing_cb = false;
	is_executing_instruction = false;

}

void CPU::ins_SLA_bHLb()
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: instruction_cache[0] = getMemory(registers.getHL()); mcycles_used++; break;
	case 2:

		registers.setFlag(n, 0);
		registers.setFlag(h, 0);

		Byte temp = instruction_cache[0];
		Byte result = temp << 1;

		registers.setFlag(c, (temp & 0x80) >> 7);
		setMemory(registers.getHL(), result);

		registers.setFlag(z, (result == 0));

		is_executing_cb = false;
		is_executing_instruction = false;
	}
}

void CPU::ins_SRA(Byte* reigster_one)
{
	registers.setFlag(n, 0);
	registers.setFlag(h, 0);
	registers.setFlag(c, *reigster_one & 0x1);
	Byte bit7 = *reigster_one >> 7;

	*reigster_one = (*reigster_one >> 1) | (bit7 << 7);
	registers.setFlag(z, (*reigster_one == 0));

	is_executing_cb = false;
	is_executing_instruction = false;

}

void CPU::ins_SRA_bHLb()
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: instruction_cache[0] = getMemory(registers.getHL()); mcycles_used++; break;
	case 2:

		registers.setFlag(n, 0);
		registers.setFlag(h, 0);

		Byte temp = instruction_cache[0];
		registers.setFlag(c, temp & 0x1);
		Byte bit7 = temp >> 7;

		Byte result = (temp >> 1) | (bit7 << 7);
		setMemory(registers.getHL(), result);

		registers.setFlag(z, (result == 0));

		is_executing_cb = false;
		is_executing_instruction = false;
	}
}

void CPU::ins_SRL(Byte* reigster_one)
{
	registers.setFlag(n, 0);
	registers.setFlag(h, 0);
	registers.setFlag(c, *reigster_one & 0x1);
	*reigster_one = *reigster_one >> 1;
	registers.setFlag(z, (*reigster_one == 0));

	is_executing_cb = false;
	is_executing_instruction = false;

}

void CPU::ins_SRL_bHLb()
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: instruction_cache[0] = getMemory(registers.getHL()); mcycles_used++; break;
	case 2:

		registers.setFlag(n, 0);
		registers.setFlag(h, 0);
		Byte temp = instruction_cache[0];
		Byte result = temp >> 1;
		registers.setFlag(c, temp & 0x1);
		setMemory(registers.getHL(), result);
		registers.setFlag(z, (result == 0));

		is_executing_cb = false;
		is_executing_instruction = false;
	}
}

void CPU::ins_SWAP(Byte* reigster_one)
{
	*reigster_one = (getNibble(*reigster_one, false) << 4) + getNibble(*reigster_one, true);
	registers.setFlag(z, (*reigster_one == 0x0));
	registers.setFlag(n, false);
	registers.setFlag(h, false);
	registers.setFlag(c, false);

	is_executing_cb = false;
	is_executing_instruction = false;

}

void CPU::ins_SWAP_bHLb()
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1: instruction_cache[0] = getMemory(registers.getHL()); mcycles_used++; break;
	case 2:

		Byte temp = instruction_cache[0];
		Byte swappedTemp = (getNibble(temp, false) << 4) + getNibble(temp, true);
		setMemory(registers.getHL(), swappedTemp);
		registers.setFlag(z, (swappedTemp == 0x0));
		registers.setFlag(n, false);
		registers.setFlag(h, false);
		registers.setFlag(c, false);

		is_executing_cb = false;
		is_executing_instruction = false;
	}
}

void CPU::ins_BIT_b_r(Byte bit, Byte* reigster_one)
{
	registers.setFlag(n, 0);
	registers.setFlag(h, 1);
	registers.setFlag(z, ((*reigster_one & (1 << bit)) == 0));

	is_executing_cb = false;
	is_executing_instruction = false;
}

void CPU::ins_BIT_b_r_bHLb(Byte bit)
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1:

		instruction_cache[0] = getMemory(registers.getHL());
		registers.setFlag(n, 0);
		registers.setFlag(h, 1);
		registers.setFlag(z, ((instruction_cache[0] & (1 << bit)) == 0));

		is_executing_cb = false;
		is_executing_instruction = false;
	}
}

void CPU::ins_RES_b_r(Byte bit, Byte* reigster_one)
{
	*reigster_one &= ~(1 << bit);

	is_executing_cb = false;
	is_executing_instruction = false;
}

void CPU::ins_RES_b_r_bHLb(Byte bit)
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1:	instruction_cache[0] = getMemory(registers.getHL()); mcycles_used++; break;
	case 2:

		Byte temp = instruction_cache[0];
		temp &= ~(1 << bit);
		setMemory(registers.getHL(), temp);

		is_executing_cb = false;
		is_executing_instruction = false;
	}
}

void CPU::ins_SET_b_r(Byte bit, Byte* reigster_one)
{
	*reigster_one |= (1 << bit);

	is_executing_cb = false;
	is_executing_instruction = false;
}

void CPU::ins_SET_b_r_bHLb(Byte bit)
{
	switch (mcycles_used)
	{
	case 0: mcycles_used++; break;
	case 1:	instruction_cache[0] = getMemory(registers.getHL()); mcycles_used++; break;
	case 2:

		Byte temp = instruction_cache[0];
		temp |= (1 << bit);
		setMemory(registers.getHL(), temp);

		is_executing_cb = false;
		is_executing_instruction = false;
	}
}


void CPU::ins_DAA()
{
	// Source from
	//https://ehaskins.com/2018-01-30%20Z80%20DAA/
	// 
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

	Byte adjust = 0;
	bool setFlagC = false;

	if (registers.getFlag(h) || (!registers.getFlag(n) && (registers.a & 0xF) > 9))
		adjust |= 0x6;

	if (registers.getFlag(c) || (!registers.getFlag(n) && registers.a > 0x99))
	{
		adjust |= 0x60;
		setFlagC = true;
	}
	
	registers.a += (registers.getFlag(n)) ? -adjust : adjust;

	registers.setFlag(z, (registers.a == 0));
	registers.setFlag(c, setFlagC);
	registers.setFlag(h, 0);

	is_executing_instruction = false;
}












