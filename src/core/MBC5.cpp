#include "MBC5.hpp"

void MBC5::ramEnable(const Word address, const Byte data)
{
	ram_bank_enable = (data & 0xF) == 0xA;
}


void MBC5::ramBankChange(const Word address, const Byte data)
{
	if (data <= 0xF)
		current_ram_bank = data;
}

void MBC5::romBankChange(const Word address, const Byte data)
{
	current_rom_bank = (current_rom_bank & 0xFF00) | data;
	current_rom_bank %= number_of_rom_banks;
}

void MBC5::romBankChange9thBit(Word address, Byte data)
{
	current_rom_bank = (current_rom_bank & 0xFF) | (bool)data << 8 ;
	current_rom_bank %= number_of_rom_banks;
}

Byte MBC5::getMemory(const Word address)
{
	if (address <= 0x3FFF) // if address is within rom bank 0
		return rom[address];

	if (address >= 0x4000 && address <= 0x7FFF) // otherwise return calculate the address by the rom bank used and return that data
	{
		return rom[(address - 0x4000) + (current_rom_bank * 0x4000)];
	}

	if (!ram_bank_enable)
		return 0xFF;

	if (address >= 0xA000 && address <= 0xBFFF)
	{
		return ram[(address - 0xA000) + (current_ram_bank * 0x2000)];
	}
}

void MBC5::setMemory(const Word address, const Byte data)
{
	if (address <= 0x1FFF)
	{
		ramEnableHandler(address, data); // enable ram bank writing
		return;
	}

	if (address >= 0x2000 && address <= 0x2FFF) // rom bank change
	{
		romBankChange(address, data);
		return;
	}
	if (address >= 0x3000 && address <= 0x3FFF) // rom bank change
	{
		romBankChange9thBit(address, data);
		return;
	}

	if (address >= 0x4000 && address <= 0x5FFF) // rom / ram bank change
	{
		ramBankChange(address, data);
		return;
	}

	if (address >= 0xA000 && address <= 0xBFFF && ram_bank_enable) // ram write
	{
		ram[(address - 0xA000) + (current_ram_bank * 0x2000)] = data;
	}
}