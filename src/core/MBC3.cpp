#include "MBC3.hpp"

void MBC3::ramEnable(const Word address, const Byte data)
{
	ram_bank_enable = (data & 0xF) == 0xA;
}


void MBC3::ramBankChange(const Word address, const Byte data)
{
	current_ram_bank = data;
}

void MBC3::romBankChange(const Word address, const Byte data)
{
	Byte rom_bank_to_select = data & 0x7F;
	if (rom_bank_to_select == 0)
		rom_bank_to_select++;

	current_rom_bank = rom_bank_to_select;
	current_rom_bank %= number_of_rom_banks;
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
		return 0xFF;

	if (address >= 0xA000 && address <= 0xBFFF)
	{
		if (current_ram_bank < 8)
		{
			return ram[(address - 0xA000) + (current_ram_bank * 0x2000)];
		}
		//fprintf(stderr, "MBC3 RTC not implemented");  exit(-1);
	}
}

void MBC3::setMemory(const Word address, const Byte data)
{
	if (address <= 0x1FFF)
	{
		ramEnableHandler(address, data); // enable ram bank writing
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
		// or rtc register select
		return;
	}

	if (address >= 0x6000 && address <= 0x7FFF) // rom / ram bank change
	{
		// latch rtc clock data
		return;
	}

	if (address >= 0xA000 && address <= 0xBFFF && ram_bank_enable) // ram write
	{
		ram[(address - 0xA000) + (current_ram_bank * 0x2000)] = data;
	}
}