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
	Byte rom_bank_to_select = data;

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
