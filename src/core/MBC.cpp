#include "MBC.hpp"
#include <fstream>
#include <iostream>

MBC::MBC()
{
}

MBC::~MBC()
{
	saveData();
	std::cout << "MBC destructor called";

}


void MBC::allocateRam()
{
	if (cartridge_type == 0x05 || cartridge_type == 0x06)
	{
		//ram.resize(256);
		ram.resize(512); // nibbles in bytes
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

	number_of_ram_banks = ram.size() / 0x4000;
}

Byte MBC::getMemory(const Word address)
{
	/*
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
		*/
	return 0;
}

void MBC::setMemory(const Word address, const Byte data)
{
	/*
	if (address <= 0x1FFF)
	{ 
		ramBankEnableHandler(address, data); // enable ram bank writing
		return;
	}

	if (address >= 0x2000 && address < 0x4000) // rom bank change
	{
		romBankChange(address, data);
		return;
	}

	if (address >= 0x4000 && address < 0x6000) // rom / ram bank change
	{
		ramBankChange(address, data);
		return;
	}

	if (address >= 0x6000 && address < 0x8000) // rom / ram bank change
	{
		bankingModeSelect(address, data);
		return;
	}
	*/
}

void MBC::ramBankEnableHandler(const Word address, const Byte data)
{
}

void MBC::ramBankEnable(const Word address, const Byte data)
{
	ramBankEnableHandler(address, data);
	
	if (!ram_bank_enable)
		saveData();
}

void MBC::ramBankChange(const Word address, const Byte data)
{
}

void MBC::romBankChange(const Word address, const Byte data)
{
}

void MBC::bankingModeSelect(const Word address, const Byte data)
{
}

void MBC::saveData()
{
	if (has_battery)
	{
		std::ofstream file(save_path, std::ios::binary | std::ios::out | std::ios::trunc); //check for exisiting save, load in data
		if (file.is_open())
		{
			file.write((char*)ram.data(), ram.size());
			file.close();
			return;
		}
	}
}
