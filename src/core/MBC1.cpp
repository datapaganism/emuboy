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
	if (banking_mode == 1)
	{
		current_rom_bank = data;
		return;
	}
	if (data <= 0x03)
	{
		current_ram_bank = data;
		return;
	}
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

void MBC1::bankingModeSelect(Word address, Byte data)
{
	banking_mode = data & 0x1;
	/*
		00 = Simple Banking Mode (default)
		0000–3FFF and A000–BFFF locked to bank 0 of ROM/RAM
		01 = RAM Banking Mode / Advanced ROM Banking Mode
		0000–3FFF and A000–BFFF can be bank-switched via the 4000–5FFF bank register
	*/
}

Byte MBC1::getMemory(const Word address)
{
	//if (banking_mode == 0)
	//{
	//	if (address <= 0x3FFF) // if address is within rom bank 0
	//		return rom[address];

	//	if (address >= 0xA000 && address <= 0xBFFF)
	//		return ram[(address - 0x2000) + (current_rom_bank * 0x2000)];
	//}

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

void MBC1::setMemory(const Word address, const Byte data)
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
}