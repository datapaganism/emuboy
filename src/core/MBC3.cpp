#include "MBC3.hpp"
#include <iostream>



void MBC3::ramBankEnableHandler(const Word address, const Byte data)
{
	if ((data & 0xF) == 0xA)
	{
		ram_bank_enable = true;
		return;
	}
	if ((data & 0xF) == 0x0)
	{
		ram_bank_enable = false;
		return;
	}
	return;
}


void MBC3::ramBankChange(const Word address, const Byte data)
{
	if (data <= 0x7)
	{
		current_ram_bank = data;
		return;
	}
	std::cout << "RTC register select\n";
}

void MBC3::romBankChange(const Word address, const Byte data)
{
	Byte rom_bank_to_select = data & 0x7F;

	if (rom_bank_to_select == 0)
	{
		current_rom_bank = 1;
		return;
	}
	//if (rom_bank_to_select > number_of_rom_banks)
	//{
	//	current_rom_bank = (rom_bank_to_select & number_of_rom_banks);
	//	return;
	//}
	current_rom_bank = rom_bank_to_select;
	return;
}

Byte MBC3::getMemory(const Word address)
{
	if (address <= 0x3FFF) // if address is within rom bank 0
		return rom[address];

	if (address >= 0x4000 && address <= 0x7FFF) // otherwise return calculate the address by the rom bank used and return that data
	{
		return rom[(address - 0x4000) + (current_rom_bank * 0x4000)];
	}

	if (!ram_bank_enable)
		return 0;

	if (address >= 0xA000 && address <= 0xBFFF)
		return ram[(address - 0x2000) + (current_ram_bank * 0x2000)];
}

void MBC3::setMemory(const Word address, const Byte data)
{
	if (address <= 0x1FFF)
	{
		ramBankEnableHandler(address, data); // enable ram bank writing
		return;
	}

	if (address >= 0x2000 && address <= 0x3FFF) // rom bank change
	{
		romBankChange(address, data);
		return;
	}

	if (address >= 0x4000 && address <= 0x5FFF) // rom / ram bank change
	{
		ramBankChange(address, data);
		return;
	}

	if (address >= 0x6000 && address <= 0x7FFF) // rom / ram bank change
	{
		bankingModeSelect(address, data);
		return;
	}

	if (address >= 0xA000 && address <= 0xBFFF && ram_bank_enable) // ram write
	{
		ram[(address - 0x2000) + (current_ram_bank * 0x2000)] = data;
	}

}
