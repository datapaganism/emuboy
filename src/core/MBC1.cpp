#include "MBC1.hpp"

void MBC1::ramBankEnableHandler(const Word address, const Byte data)
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
}


void MBC1::ramBankChange(const Word address, const Byte data)
{
}

void MBC1::romBankChange(const Word address, const Byte data)
{
	Byte rom_bank_to_select = data & 0x1F;

	if (rom_bank_to_select == 0)
	{
		current_rom_bank = 1;
		return;
	}
	if (rom_bank_to_select > number_of_rom_banks)
	{
		current_rom_bank = (rom_bank_to_select & number_of_rom_banks);
		return;
	}
	current_rom_bank = rom_bank_to_select;
	return;
}
