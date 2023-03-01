#include "NO_MBC.hpp"

Byte NO_MBC::getMemory(const Word address)
{	
	if (address <= 0x3FFF) // if address is within rom bank 0
		return rom[address];

	if (address >= 0x4000 && address <= 0x7FFF) // otherwise return calculate the address by the rom bank used and return that data
	{
		return rom[(address - 0x4000) + (current_rom_bank * 0x4000)];
	}
	if (ram.size() != 0)
	{
		if (address >= 0xA000 && address <= 0xBFFF) // otherwise return calculate the address by the rom bank used and return that data
		{
			return ram[(address - 0x2000)];
		}
	}
}

void NO_MBC::setMemory(const Word address, const Byte data)
{
	if (ram.size() != 0)
	{
		if (address >= 0xA000 && address <= 0xBFFF) // otherwise return calculate the address by the rom bank used and return that data
		{
			ram[(address - 0x2000)] = data;
		}
	}
}

