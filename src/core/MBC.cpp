#include "MBC.hpp"
#include <fstream>

MBC::MBC()
{
}

MBC::~MBC()
{
	saveData();
	std::cout << "MBC destructor called\n";
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
	case 2: ram.resize(1 * 0x2000); break;
	case 3: ram.resize(4 * 0x2000); break;
	case 4: ram.resize(16 * 0x2000); break;
	case 5: ram.resize(8 * 0x2000); break;
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
		ramEnable(address, data); // enable ram bank writing
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

void MBC::ramEnable(const Word address, const Byte data)
{
}

void MBC::ramEnableHandler(const Word address, const Byte data)
{
	ramEnable(address, data);
	
	if (!ram_bank_enable)
		saveData();
}

void MBC::ramBankChange(const Word address, const Byte data)
{
}

void MBC::romBankChange(const Word address, const Byte data)
{
}


void MBC::saveData()
{
	if (has_battery)
	{
		std::ofstream file(save_path, std::ios::binary | std::ios::out | std::ios::trunc); //check for exisiting save file, truncate
		if (file.is_open())
		{
			file.write((char*)ram.data(), ram.size());
			file.close();
			return;
		}
	}
}

void MBC::checkAndLoadSave()
{
	// check if cartridge has saving
	switch (cartridge_type)
	{
	case 0x03:
	case 0x06:
	case 0x09:
	case 0x0D:
	case 0x0F:
	case 0x10:
	case 0x13:
	case 0x1B:
	case 0x1E:
	case 0x22:
	case 0xFC:
	case 0xFF:
		has_battery = true;
		break;
	default:
		return;
	}

	if (has_battery)
	{
		std::ifstream file(save_path, std::ios::binary | std::ios::in); //check for exisiting save, load in data
		if (file.is_open())
		{
			file.read((char*)ram.data(), ram.size());
			file.close();

			return;
		}
	}
}