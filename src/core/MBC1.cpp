#include "MBC1.hpp"

void MBC1::ramBankEnableHandler(const Word address, const Byte data)
{
	ram_bank_enable = (data & 0xF) == 0xA;
}


void MBC1::ramBankChange(const Word address, const Byte data)
{
	Byte new_ram_bank = data & 3;
	if (banking_mode == 1)
	{
		current_rom_bank &= 0x1F;
		current_rom_bank |= new_ram_bank << 5;

		current_ram_bank = 0;
	}
	else
	{
		current_ram_bank = data;
		current_rom_bank &= 0x1F;
	}
	
	current_rom_bank %= number_of_rom_banks;
	/*if (ram_bank_enable)
		current_ram_bank %= number_of_ram_banks;*/
}

void MBC1::romBankChange(const Word address, const Byte data)
{
	Byte rom_bank_to_select = data & 0x1F;
	if (rom_bank_to_select % 0x20 == 0)
		rom_bank_to_select++;

	current_rom_bank &= 0x60;
	current_rom_bank |= rom_bank_to_select;
	current_rom_bank %= number_of_rom_banks;
}

void MBC1::bankingModeSelect(Word address, Byte data)
{
	banking_mode = (data & 0x1) == 0;
	/*
		00 = Simple Banking Mode (default)
		0000–3FFF and A000–BFFF locked to bank 0 of ROM/RAM
		01 = RAM Banking Mode / Advanced ROM Banking Mode
		0000–3FFF and A000–BFFF can be bank-switched via the 4000–5FFF bank register
	*/
}

Byte MBC1::getMemory(const Word address)
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
		return ram[(address - 0xA000) + (current_ram_bank * 0x2000)];
}

void MBC1::setMemory(const Word address, const Byte data)
{
	if (address <= 0x1FFF)
	{
		ramBankEnable(address, data); // enable ram bank writing
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

	if (address >= 0xA000 && address <= 0xBFFF && ram_bank_enable && rom_size) // ram write
	{
		ram[(address - 0xA000) + (current_ram_bank * 0x2000)] = data;
	}
}