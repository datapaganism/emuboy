#include "cpu.hpp"
#include "bus.hpp"

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
		Byte hl = this->registers.get_HL();
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
	this->registers.a ^= *registerByte;

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
		this->registers.a ^= this->get_byte_from_pc();

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
		this->registers.a ^= this->bus->get_memory(this->registers.get_HL(), MEMORY_ACCESS_TYPE::cpu);

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
		*registerTwo = this->bus->get_memory(this->registers.sp, MEMORY_ACCESS_TYPE::cpu);
		if (registerTwo == &this->registers.f)
			*registerTwo &= 0xF0;
		this->registers.sp++;

		this->mCyclesUsed++; break;
	case 2:
		*registerOne = this->bus->get_memory(this->registers.sp, MEMORY_ACCESS_TYPE::cpu);
		this->registers.sp++;

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
		this->bus->set_memory(--this->registers.sp, ((wordRegisterValue & 0xff00) >> 8), MEMORY_ACCESS_TYPE::cpu);

		this->mCyclesUsed++; break;
	case 3:
		// move high byte to lower (sp)
		this->bus->set_memory(--this->registers.sp, (wordRegisterValue & 0x00ff), MEMORY_ACCESS_TYPE::cpu);

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
	case 4: this->registers.pc = (instructionCache[0] << 8) | instructionCache[1]; this->isExecutingInstruction = false;
	}
}

void CPU::ins_RET()
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1: this->instructionCache[0] = this->bus->get_memory(this->registers.sp++, MEMORY_ACCESS_TYPE::cpu); this->mCyclesUsed++; break;
	case 2: this->instructionCache[1] = this->bus->get_memory(this->registers.sp++, MEMORY_ACCESS_TYPE::cpu); this->mCyclesUsed++; break;
	case 3: this->registers.pc = (instructionCache[0] << 8) | instructionCache[1]; this->isExecutingInstruction = false;
	}
}

void CPU::ins_RETI()
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1: this->instructionCache[0] = this->bus->get_memory(this->registers.sp++, MEMORY_ACCESS_TYPE::cpu); this->mCyclesUsed++; break;
	case 2: this->instructionCache[1] = this->bus->get_memory(this->registers.sp++, MEMORY_ACCESS_TYPE::cpu); this->mCyclesUsed++; break;
	case 3: this->registers.pc = (instructionCache[0] << 8) | instructionCache[1]; this->interrupt_master_enable = 1; this->isExecutingInstruction = false;
	}
}



void CPU::ins_JP_HL()
{
	this->registers.pc = this->registers.get_HL();
	this->isExecutingInstruction = false;
}


void CPU::ins_CALL_u16(const enum JumpCondition condition)
{
	switch (this->mCyclesUsed)
	{
	case 0:  this->mCyclesUsed++; break;
	case 1: this->instructionCache[0] = this->get_byte_from_pc(); this->mCyclesUsed++; break;
	case 2: this->instructionCache[1] = this->get_byte_from_pc(); this->mCyclesUsed++; break;
	case 3: checkJumpCondition(condition) ? this->mCyclesUsed++ : this->isExecutingInstruction = false; break;
	case 4:
		this->bus->set_memory(--this->registers.sp, this->registers.get_highByte(&this->registers.pc), MEMORY_ACCESS_TYPE::cpu);
		this->mCyclesUsed++; break;
	case 5:
		this->bus->set_memory(--this->registers.sp, this->registers.get_lowByte(&this->registers.pc), MEMORY_ACCESS_TYPE::cpu);
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
		this->bus->set_memory(--this->registers.sp, this->registers.get_highByte(&this->registers.pc), MEMORY_ACCESS_TYPE::cpu);
		this->mCyclesUsed++; break;
	case 3:
		this->bus->set_memory(--this->registers.sp, this->registers.get_lowByte(&this->registers.pc), MEMORY_ACCESS_TYPE::cpu);
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
	case 0:  this->mCyclesUsed++; break;
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
			this->registers.set_flag(h, ((this->registers.sp & 0xF) + value) > 0xF);
		}
		this->registers.set_flag(z, 0);
		this->registers.set_flag(n, 0);

		instructionCache[1] = this->registers.sp + value;
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
			this->registers.set_flag(h, ((this->registers.sp & 0xF) + value) > 0xF);
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



/*

int CPU::ins_LD_nn_n(Byte* registerOne, Byte value)
{
	//LD nn,n
	*registerOne = value;
	return 8;
}

int CPU::ins_LD_X_Y(Byte* registerOne, Word address, Byte* registerTwo, Byte value)
{
	if (registerOne && registerTwo)
	{
		*registerOne = *registerTwo;
		return 4;
	}
	if (registerOne && address)
	{
		*registerOne = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);
		return 8;
	}
	if (address && registerTwo)
	{
		this->bus->set_memory(address, *registerTwo, MEMORY_ACCESS_TYPE::cpu);
		return 8;
	}
	// if (address && value)

	this->bus->set_memory(address, value, MEMORY_ACCESS_TYPE::cpu);
	return 12;

}

int CPU::ins_LD_r1_nn(Byte* registerOne, const Word address, const int cyclesUsed)
{
	// Cycles used is a param since it can change
	//take value pointed to by nn and put into register one,
	// LD a,(nn), mem byte eg: FA 34 12, ld a,(1234)
	// check memory at address 0x1234, put that data into register one

	*registerOne = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);
	return cyclesUsed;
};
// Take value from registerOne, place at memory location
int CPU::ins_LD_nn_r1(Word address, Byte* registerOne)
{
	this->bus->set_memory(address, *registerOne, MEMORY_ACCESS_TYPE::cpu);
	return 16;
};

int CPU::ins_LDDI_nn_r1(Word address, const Byte* registerOne, Byte* registerTwo, Byte* registerThree, const int addSubValue)
{
	this->bus->set_memory(address, *registerOne, MEMORY_ACCESS_TYPE::cpu);
	this->registers.set_word(registerTwo, registerThree, (this->registers.get_word(registerTwo, registerThree) + addSubValue));
	return 8;
};

int CPU::ins_LDDI_r1_nn(Byte* registerOne, const Word address, Byte* registerTwo, Byte* registerThree, const int addSubValue)
{
	*registerOne = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);
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

int CPU::ins_LDHL_SP_n(Byte* wordRegisterNibbleHi, Byte* wordRegisterNibbleLo, const Word stackPointerValue, const Byte_s value)
{
	Word sum = stackPointerValue + value;
	this->registers.set_flag(z, 0);
	this->registers.set_flag(n, 0);


	if (value < 0)
	{
		this->registers.set_flag(c, (sum & 0xFF) <= (stackPointerValue & 0xFF));
		this->registers.set_flag(h, (sum & 0xF) <= (stackPointerValue & 0xF));
	}
	else
	{
		this->registers.set_flag(c, ((stackPointerValue & 0xFF) + value) > 0xFF);
		this->registers.set_flag(h, ((stackPointerValue & 0xF) + value) > 0xF);
	}

	this->registers.set_word(wordRegisterNibbleHi, wordRegisterNibbleLo, sum);
	return 12;
}

int CPU::ins_LD_nn_SP(const Word address, const Word stackPointerValue)
{
	//place value of SP into memory at address and address + 1
	this->bus->set_memory_word(address, stackPointerValue, MEMORY_ACCESS_TYPE::cpu);
	return 20;
}

//new push, pushes addr -1 then addr -2
int CPU::ins_PUSH_nn(const Word wordRegisterValue)
{
	// move low byte to higher (sp)
	this->registers.sp--;
	this->bus->set_memory(this->registers.sp, ((wordRegisterValue & 0xff00) >> 8), MEMORY_ACCESS_TYPE::cpu);

	this->registers.sp--;
	// move high byte to lower (sp)
	this->bus->set_memory(this->registers.sp, (wordRegisterValue & 0x00ff), MEMORY_ACCESS_TYPE::cpu);

	return 16;
}

int CPU::ins_POP_nn(Byte* registerOne, Byte* registerTwo)
// a f
{
	*registerTwo = this->bus->get_memory(this->registers.sp, MEMORY_ACCESS_TYPE::cpu);
	if (registerTwo == &this->registers.f)
		*registerTwo &= 0xF0;
	this->registers.sp++;

	*registerOne = this->bus->get_memory(this->registers.sp, MEMORY_ACCESS_TYPE::cpu);
	this->registers.sp++;

	return 12;
}

int CPU::ins_ADD_A_n(const Byte* registerOne, const Byte immediateValue)
{
	if (registerOne)
	{
		this->registers.set_flag(c, this->checkCarry(this->registers.a, *registerOne, 8));
		this->registers.set_flag(h, this->checkCarry(this->registers.a, *registerOne, 4));

		// perform addition
		this->registers.a += *registerOne;

		//evaluate z flag an clear the n flag
		this->registers.set_flag(z, (this->registers.a == 0x0));
		this->registers.set_flag(n, 0);

		return 4;
	}
	// else if immediate value passed
	this->registers.set_flag(c, this->checkCarry(this->registers.a, immediateValue, 8));
	this->registers.set_flag(h, this->checkCarry(this->registers.a, immediateValue, 4));

	// perform addition
	this->registers.a += immediateValue;

	//evaluate z flag an clear the n flag
	this->registers.set_flag(z, (this->registers.a == 0x0));
	this->registers.set_flag(n, 0);


	return 8;
}

int CPU::ins_ADC_A_n(const Byte* registerOne, const Byte immediateValue)
{
	Byte a = this->registers.a;
	Byte b = (registerOne) ? *registerOne : immediateValue;
	Byte C = (int)this->registers.get_flag(c);
	Byte cyclesUsed = (registerOne) ? 4 : 8;

	this->registers.a = a + b + C;

	this->registers.set_flag(z, (this->registers.a == 0x0));
	this->registers.set_flag(n, 0);
	this->registers.set_flag(c, this->checkCarry(a, b, 8, C));
	this->registers.set_flag(h, this->checkCarry(a, b, 4, C));

	return cyclesUsed;
}


int CPU::ins_SUB_n(const Byte* registerOne, const Byte immediateValue)
{
	if (registerOne)
	{
		//check carry and half carry flags, I realise this is out of order but it should work the same.
		this->registers.set_flag(c, this->checkBorrow(this->registers.a, *registerOne, 8));
		this->registers.set_flag(h, this->checkBorrow(this->registers.a, *registerOne, 4));

		// perform addition
		this->registers.a -= *registerOne;

		//evaluate z flag an clear the n flag
		this->registers.set_flag(z, (this->registers.a == 0x0));
		this->registers.set_flag(n, 1);

		return 4;
	}
	// else if immediate value passed

	//check carry and half carry flags, I realise this is out of order but it should work the same.
	this->registers.set_flag(c, this->checkBorrow(this->registers.a, immediateValue, 8));
	this->registers.set_flag(h, this->checkBorrow(this->registers.a, immediateValue, 4));

	// perform addition
	this->registers.a -= immediateValue;

	//evaluate z flag an clear the n flag
	this->registers.set_flag(z, (this->registers.a == 0x0));
	this->registers.set_flag(n, 1);

	return 8;
}

int CPU::ins_SBC_A_n(const Byte* registerOne, const Byte immediateValue)
{

	Byte a = this->registers.a;
	Byte b = (registerOne) ? *registerOne : immediateValue;
	Byte C = (int)this->registers.get_flag(c);
	Byte cyclesUsed = (registerOne) ? 4 : 8;

	this->registers.a = a - b - C;

	this->registers.set_flag(z, (this->registers.a == 0x0));
	this->registers.set_flag(n, 1);
	this->registers.set_flag(c, this->checkBorrow(a, b, 8, C));
	this->registers.set_flag(h, this->checkBorrow(a, b, 4, C));

	return cyclesUsed;

}

int CPU::ins_AND_n(const Byte* registerOne, const Byte immediateValue)
{
	if (registerOne)
	{
		this->registers.a &= *registerOne;

		//evaluate z flag an clear the n flag
		this->registers.set_flag(z, (this->registers.a == 0x0));
		this->registers.set_flag(h, 1);
		this->registers.set_flag(n, 0);
		this->registers.set_flag(c, 0);

		return 4;
	}
	// else if immediate value passed

	this->registers.a &= immediateValue;

	//evaluate z flag an clear the n flag
	this->registers.set_flag(z, (this->registers.a == 0x0));
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
		this->registers.set_flag(z, (this->registers.a == 0x0));
		this->registers.set_flag(h, 0);
		this->registers.set_flag(n, 0);
		this->registers.set_flag(c, 0);

		return 4;
	}
	// else if immediate value passed

	this->registers.a |= immediateValue;

	//evaluate z flag an clear the n flag
	this->registers.set_flag(z, (this->registers.a == 0x0));
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
		this->registers.set_flag(z, (this->registers.a == 0x0));
		this->registers.set_flag(h, 0);
		this->registers.set_flag(n, 0);
		this->registers.set_flag(c, 0);

		return 4;
	}
	// else if immediate value passed

	this->registers.a ^= immediateValue;

	//evaluate z flag an clear the n flag
	this->registers.set_flag(z, (this->registers.a == 0x0));
	this->registers.set_flag(h, 0);
	this->registers.set_flag(n, 0);
	this->registers.set_flag(c, 0);


	return 8;
}

int CPU::ins_CP_n(const Byte* registerOne, const Byte immediateValue)
{
	if (registerOne)
	{

		this->registers.set_flag(c, this->checkBorrow(this->registers.a, *registerOne, 8));
		this->registers.set_flag(h, this->checkBorrow(this->registers.a, *registerOne, 4));

		//evaluate z flag an clear the n flag
		(this->registers.a == *registerOne) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
		this->registers.set_flag(n, 1);

		return 4;
	}
	// else if immediate value passed
	this->registers.set_flag(c, this->checkBorrow(this->registers.a, immediateValue, 8));
	this->registers.set_flag(h, this->checkBorrow(this->registers.a, immediateValue, 4));

	//evaluate z flag an clear the n flag
	(this->registers.a == immediateValue) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
	this->registers.set_flag(n, 1);

	return 8;
}

int CPU::ins_INC_n(Byte* registerOne, Word address)
{
	if (registerOne)
	{
		this->registers.set_flag(h, this->checkCarry(*registerOne, 1, 4));

		// perform addition
		(*registerOne)++;

		//evaluate z flag an clear the n flag
		(*registerOne == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
		this->registers.set_flag(n, 0);

		return 4;
	}
	// else if immediate value passed

	//check carry and half carry flags, I realise this is out of order but it should work the same.

	auto temp = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);
	this->registers.set_flag(h, this->checkCarry(temp, 1, 4));

	// perform addition
	this->bus->set_memory(address, (temp + 1), MEMORY_ACCESS_TYPE::cpu);
	//evaluate z flag an clear the n flag
	(this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu) == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
	this->registers.set_flag(n, 0);

	return 12;
}

int CPU::ins_DEC_n(Byte* registerOne, Word address)
{
	if (registerOne)
	{
		this->registers.set_flag(h, this->checkBorrow(*registerOne, 1, 4));
		// perform addition
		(*registerOne)--;

		//evaluate z flag an clear the n flag
		(*registerOne == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
		this->registers.set_flag(n, 1);

		return 4;
	}
	// else if immediate value passed
	auto temp = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);
	this->registers.set_flag(h, this->checkBorrow(temp, 1, 4));

	// perform addition
	this->bus->set_memory(address, (temp - 1), MEMORY_ACCESS_TYPE::cpu);
	//evaluate z flag an clear the n flag
	(this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu) == 0x0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
	this->registers.set_flag(n, 1);

	return 12;
}

int CPU::ins_ADD_HL_n(const Word value)
{
	Word HLvalue = this->registers.get_HL();

	this->registers.set_flag(c, this->checkCarry(HLvalue, value, 16));
	this->registers.set_flag(h, this->checkCarry(HLvalue, value, 12));

	this->registers.set_HL(HLvalue + value);

	this->registers.set_flag(n, 0);
	return 8;
}

int CPU::ins_ADD_SP_n(const Byte_s value)
{
	//if negative

	if (value < 0)
	{
		this->registers.set_flag(c, ((this->registers.sp + value) & 0xFF) <= (this->registers.sp & 0xFF));
		this->registers.set_flag(h, ((this->registers.sp + value) & 0xF) <= (this->registers.sp & 0xF));
	}
	else
	{
		this->registers.set_flag(c, ((this->registers.sp & 0xFF) + value) > 0xFF);
		this->registers.set_flag(h, ((this->registers.sp & 0xF) + value) > 0xF);
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
	this->registers.set_word(registerOne, registerTwo, (this->registers.get_word(registerOne, registerTwo) + 1));
	return 8;
}

int CPU::ins_DEC_nn(Byte* registerOne, Byte* registerTwo, Word* stackPointer)
{
	if (stackPointer)
	{
		this->registers.sp--;
		return 8;
	}
	this->registers.set_word(registerOne, registerTwo, (this->registers.get_word(registerOne, registerTwo) - 1));
	return 8;
}
*/

//int CPU::ins_SWAP_nn(Byte* registerOne, Word address)
//{
//	int cyclesUsed = 0;
//	if (registerOne)
//	{
//		*registerOne = (this->get_nibble(*registerOne, false) << 4) + this->get_nibble(*registerOne, true);
//		(*registerOne == 0x0) ? this->registers.set_flag(z, true) : this->registers.set_flag(z, false);
//		cyclesUsed = 8;
//	}
//	else
//	{
//		this->bus->set_memory(address, (this->get_nibble(this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu), false) << 4) + this->get_nibble(this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu), true), MEMORY_ACCESS_TYPE::cpu);
//		(this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu) == 0x0) ? this->registers.set_flag(z, true) : this->registers.set_flag(z, false);
//		cyclesUsed = 16;
//	}
//	this->registers.set_flag(n, false);
//	this->registers.set_flag(h, false);
//	this->registers.set_flag(c, false);
//
//	return cyclesUsed;
//}





//int CPU::ins_RL(Byte* registerOne, Word address)
//{
//	Byte flagCarry = this->registers.get_flag(c);
//	bool newCarry = 0;
//
//	this->registers.set_flag(n, 0);
//	this->registers.set_flag(h, 0);
//
//	if (registerOne)
//	{
//		newCarry = ((*registerOne & 0x80) >> 7);
//		*registerOne = ((*registerOne << 1) | (flagCarry));
//
//		this->registers.set_flag(c, newCarry);
//
//		(*registerOne == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
//
//		return 8;
//	}
//
//	Byte temp = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);
//	Byte result = ((temp << 1) | (flagCarry));
//	newCarry = ((temp & 0x80) >> 7);
//	this->bus->set_memory(address, result, MEMORY_ACCESS_TYPE::cpu);
//
//	this->registers.set_flag(c, newCarry);
//
//	(result == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
//
//	return 16;
//}
//
//int CPU::ins_RRC(Byte* registerOne, Word address)
//{
//
//	this->registers.set_flag(n, 0);
//	this->registers.set_flag(h, 0);
//
//	if (registerOne)
//	{
//		this->registers.set_flag(c, (*registerOne & 0x1));
//		*registerOne = ((*registerOne >> 1) | (*registerOne << 7));
//
//
//		(*registerOne == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
//
//		return 8;
//	}
//
//	Byte temp = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);
//	Byte result = ((temp >> 1) | (temp << 7));
//
//	this->registers.set_flag(c, (temp & 0x1));
//	this->bus->set_memory(address, result, MEMORY_ACCESS_TYPE::cpu);
//
//	(result == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
//
//	return 16;
//}

//int CPU::ins_RR(Byte* registerOne, Word address)
//{
//	Byte flagCarry = this->registers.get_flag(c);
//	bool newCarry = 0;
//
//	this->registers.set_flag(n, 0);
//	this->registers.set_flag(h, 0);
//
//	if (registerOne)
//	{
//		newCarry = (*registerOne & 0x1);
//		*registerOne = ((*registerOne >> 1) | (flagCarry << 7));
//
//		this->registers.set_flag(c, newCarry);
//
//		(*registerOne == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
//
//		return 8;
//	}
//
//	Byte temp = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);
//	Byte result = ((temp >> 1) | (flagCarry << 7));
//	newCarry = (temp & 0x01);
//	this->bus->set_memory(address, result, MEMORY_ACCESS_TYPE::cpu);
//
//	this->registers.set_flag(c, newCarry);
//	(result == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
//	return 16;
//}

//int CPU::ins_SLA(Byte* registerOne, Word address)
//{
//	this->registers.set_flag(n, 0);
//	this->registers.set_flag(h, 0);
//
//	if (registerOne)
//	{
//		this->registers.set_flag(c, (*registerOne & 0x80) >> 7);
//		*registerOne = *registerOne << 1;
//
//		(*registerOne == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
//
//		return 8;
//	}
//	Byte temp = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);
//	Byte result = temp << 1;
//
//	this->registers.set_flag(c, (temp & 0x80) >> 7);
//	this->bus->set_memory(address, result, MEMORY_ACCESS_TYPE::cpu);
//
//	(result == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
//
//	return 16;
//}

//int CPU::ins_SRA_n(Byte* registerOne, Word address)
//{
//	this->registers.set_flag(n, 0);
//	this->registers.set_flag(h, 0);
//
//	Byte bit7 = 0;
//	// shift right into carry, msb does not change
//	if (registerOne)
//	{
//		this->registers.set_flag(c, *registerOne & 0x1);
//		bit7 = *registerOne >> 7;
//
//		*registerOne = (*registerOne >> 1) | (bit7 << 7);
//		(*registerOne == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
//		return 8;
//	}
//
//	Byte temp = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);
//
//	this->registers.set_flag(c, temp & 0x1);
//	bit7 = temp >> 7;
//
//	Byte result = (temp >> 1) | (bit7 << 7);
//	this->bus->set_memory(address, result, MEMORY_ACCESS_TYPE::cpu);
//
//	(result == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
//
//	return 16;
//}
//
//int CPU::ins_SRL_n(Byte* registerOne, Word address)
//{
//	this->registers.set_flag(n, 0);
//	this->registers.set_flag(h, 0);
//
//	// shift right into carry, msb does not change
//	if (registerOne)
//	{
//		this->registers.set_flag(c, *registerOne & 0x1);
//		*registerOne = *registerOne >> 1;
//		(*registerOne == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
//		return 8;
//	}
//	Byte temp = this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu);
//	Byte result = temp >> 1;
//
//	this->registers.set_flag(c, temp & 0x1);
//
//	this->bus->set_memory(address, result, MEMORY_ACCESS_TYPE::cpu);
//	(result == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
//	return 16;
//}
//
//int CPU::ins_BIT_b_r(Byte bit, Byte* registerOne, Word address)
//{
//	this->registers.set_flag(n, 0);
//	this->registers.set_flag(h, 1);
//
//	if (registerOne)
//	{
//		((*registerOne & (1 << bit)) == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
//		return 8;
//	}
//
//	((this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu) & (1 << bit)) == 0) ? this->registers.set_flag(z, 1) : this->registers.set_flag(z, 0);
//	return 16;
//}
//
//int CPU::ins_SET_b_r(Byte bit, Byte* registerOne, Word address)
//{
//	if (registerOne)
//	{
//		*registerOne |= (1 << bit);
//		return 8;
//	}
//
//	this->bus->set_memory(address, this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu) | (1 << bit), MEMORY_ACCESS_TYPE::cpu);
//	return 16;
//}
//
//int CPU::ins_RES_b_r(Byte bit, Byte* registerOne, Word address)
//{
//	if (registerOne)
//	{
//		*registerOne &= ~(1 << bit);
//		return 8;
//	}
//
//	this->bus->set_memory(address, this->bus->get_memory(address, MEMORY_ACCESS_TYPE::cpu) & ~(1 << bit), MEMORY_ACCESS_TYPE::cpu);
//	return 16;
//}
/*
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
		{
			this->ins_JP_nn(address);
			return 16;
		}
	} break;

	case Z:
	{
		if (this->registers.get_flag(z) == 1)
		{
			this->ins_JP_nn(address);
			return 16;
		}
	} break;

	case NC:
	{
		if (this->registers.get_flag(c) == 0)
		{
			this->ins_JP_nn(address);
			return 16;
		}
	} break;

	case C:
	{
		if (this->registers.get_flag(c) == 1)
		{
			this->ins_JP_nn(address);
			return 16;
		}
	} break;

	default: throw "Unreachable Jump condition"; break;
	}
	return 12;
}


int CPU::ins_JR_n(Byte_s jumpOffset)
{
	this->registers.pc += jumpOffset;

	return 12;
}

int CPU::ins_JR_cc_n(const enum JumpCondition condition, Byte_s jumpOffset)
{
	switch (condition)
	{
	case NZ:
	{
		if (!this->registers.get_flag(z))
		{
			this->ins_JR_n(jumpOffset);
			return 12;
		}
	} break;

	case Z:
	{
		if (this->registers.get_flag(z))
		{
			this->ins_JR_n(jumpOffset);
			return 12;
		}
	} break;

	case NC:
	{
		if (!this->registers.get_flag(c))
		{
			this->ins_JR_n(jumpOffset);
			return 12;
		}
	} break;

	case C:
	{
		if (this->registers.get_flag(c))
		{
			this->ins_JR_n(jumpOffset);
			return 12;
		}
	} break;
	default: throw "Unreachable Jump condition"; break;
	}
	return 8;
}

int CPU::ins_CALL_nn(Word address)
{
	this->ins_PUSH_nn(this->registers.pc);
	this->ins_JP_nn(address);
	return 12;
}

int CPU::ins_CALL_cc_nn(enum JumpCondition condition, Word address)
{

	switch (condition)
	{
	case NZ:
	{
		if (!this->registers.get_flag(z))
		{
			this->ins_CALL_nn(address);
			return 24;
		}
	} break;

	case Z:
	{
		if (this->registers.get_flag(z))
		{
			this->ins_CALL_nn(address);
			return 24;
		}
	} break;

	case NC:
	{
		if (!this->registers.get_flag(c))
		{
			this->ins_CALL_nn(address);
			return 24;
		}
	} break;

	case C:
	{
		if (this->registers.get_flag(c))
		{
			this->ins_CALL_nn(address);
			return 24;
		}
	} break;
	default: throw "Unreachable Jump condition"; break;
	}
	return 12;
}

int CPU::ins_RST_n(const Byte addrOffset)
{
	this->ins_CALL_nn(0x0000 + addrOffset);
	return 16;
}


int CPU::ins_RET()
{
	Word SP = this->registers.sp;
	this->registers.pc = this->bus->get_memory_word_lsbf(SP, MEMORY_ACCESS_TYPE::cpu);
	this->registers.sp += 2;
	return 16;
}

int CPU::ins_RETI()
{
	this->interrupt_master_enable = 1;
	this->ins_RET();
	return 16;
}

int CPU::ins_RET_cc(const enum JumpCondition condition)
{
	switch (condition)
	{
	case NZ:
	{
		if (!this->registers.get_flag(z))
		{
			this->ins_RET();
			return 20;
		}
	} break;

	case Z:
	{
		if (this->registers.get_flag(z))
		{
			this->ins_RET();
			return 20;
		}
	} break;

	case NC:
	{
		if (!this->registers.get_flag(c))
		{
			this->ins_RET();
			return 20;
		}
	} break;

	case C:
	{
		if (this->registers.get_flag(c))
		{
			this->ins_RET();
			return 20;
		}
	} break;
	default: throw "Unreachable Jump condition"; break;
	}
	// If we don't require jumping
	return 8;
}
*/