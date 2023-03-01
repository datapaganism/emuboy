#include "MBC2.hpp"

void MBC2::ramBankEnable(const Word address, const Byte data)
{
	ram_bank_enable = (data == 0xA);
}

void MBC2::romBankChange(const Word address, const Byte data)
{
	Byte rom_bank_to_select = data & 0b1111;
	if (rom_bank_to_select == 0)
		rom_bank_to_select++;

	current_rom_bank = rom_bank_to_select;
}

Byte MBC2::getMemory(const Word address)
{
	if (address <= 0x3FFF) // if address is within rom bank 0
		return rom[address];

	else if (address >= 0x4000 && address <= 0x7FFF) // otherwise return calculate the address by the rom bank used and return that data
	{
		return rom[(address - 0x4000) + (current_rom_bank * 0x4000)];
	}

	else if (!ram_bank_enable)
		return 0xFF;

	else if (address >= 0xA000 && address <= 0xBFFF)
	{
		return ram[(address - 0xA000) & 0x200] & 0b1111;
	}
}

void MBC2::setMemory(const Word address, const Byte data)
{
	if (address <= 0x3FFF)
	{
		if (address & 0b1 << 8) // if rom mode 
		{
			romBankChange(address,data);
			return;
		}
		ramBankEnableHandler(address, data);
		return;
	}

	if (address >= 0xA000 && address <= 0xBFFF && ram_bank_enable) // ram write
	{
		Word address2 = ((address - 0xA000) & 0x1FF);
		ram[address2] = data & 0b1111;
		return;
	}

}