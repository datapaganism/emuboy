#include "MBC.hpp"

MBC::MBC()
{
}


void MBC::allocateRam()
{
	if (cartridge_type == 0x05 || cartridge_type == 0x06)
	{
		ram.resize(256);
		return;
	}

	switch (ram_size)
	{
	case 0: break;
	case 1: break;
	case 2: this->ram.resize(1 * 0x2000); break;
	case 3: this->ram.resize(4 * 0x2000); break;
	case 4: this->ram.resize(16 * 0x2000); break;
	case 5: this->ram.resize(8 * 0x2000); break;
	default: fprintf(stderr, "Unreachable number of banks");  exit(-1);;
	}
}

Byte MBC::getMemory(const Word address)
{
	if (address <= 0x3FFF) // if address is within rom bank 0
		return rom[address];

	if (address <= 0x7FFF) // otherwise return calculate the address by the rom bank used and return that data
	{
		Word new_address = (address - 0x4000) + (current_rom_bank * 0x4000);
		return rom[new_address];

	}

	if (!ram_bank_enable)
		return 0;

	if (address <= 0xBFFF)
		return ram[(address - 0x2000) + (current_ram_bank * 0x2000)];
}

void MBC::setMemory(const Word address, const Byte data)
{
	if (address <= 0x1FFF)
	{ // enable ram bank writing
		this->ramBankEnableHandler(address, data);
		return;
	}

	if (address <= 0x4000) // rom bank change
	{
		this->romBankChange(address, data);
		return;
	}

	if (address <= 0x6000) // rom / ram bank change
	{
		return;
	}
}

void MBC::ramBankEnableHandler(const Word address, const Byte data)
{
}

void MBC::ramBankChange(const Word address, const Byte data)
{
}

void MBC::romBankChange(const Word address, const Byte data)
{
}
