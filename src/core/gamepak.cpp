#include <fstream>
#include <vector>
#include <memory>


#include "gamepak.hpp"



GamePak::GamePak(const std::string filename) : GamePak::GamePak()
{
	this->filename = filename;
	std::ifstream file(this->filename, std::ios::binary | std::ios::ate);
	if (file.is_open())
	{
		std::vector<Byte> rom_data;
		std::ifstream::pos_type pos = file.tellg();
		rom_data.resize(pos); //reallocate memory space to fit entire rom
		file.seekg(0, std::ios::beg);
		file.read((char*)rom_data.data(), pos);
		file.close();

		initMBC(rom_data);

		gamepak_loaded = true;
		return;
	}
	fprintf(stderr, "cannot load cartridge");  exit(-1);
}

GamePak::GamePak()
{
}

void GamePak::initMBC(std::vector<Byte>& rom_data)
{
	Byte cartridge_type = rom_data[0x147];

	if (cartridge_type == 0)
		memory_bank_controller = std::make_unique<MBC_ROM_ONLY>();
	else if (cartridge_type <= 0x3)
		memory_bank_controller = std::make_unique<MBC1>();
	else if (cartridge_type <= 0x6)
		memory_bank_controller = std::make_unique<MBC2>();
	else if (cartridge_type <= 0x9)
	{
		fprintf(stderr, "ROM + RAM + BAT Not implemented");  exit(-1);
	}
	else if (cartridge_type <= 0x0D)
	{
		fprintf(stderr, "MMM01 Not implemented");  exit(-1);
	}
	else if (cartridge_type <= 0x13)
		memory_bank_controller = std::make_unique<MBC3>();
	else if (cartridge_type <= 0x1E)
	{
		memory_bank_controller = std::make_unique<MBC5>();
	}
	else if (cartridge_type <= 0x20)
	{
		fprintf(stderr, "MBC6 Not implemented");  exit(-1);
	}
	else if (cartridge_type <= 0x22)
	{
		fprintf(stderr, "MBC7 Not implemented");  exit(-1);
	}
	else if (cartridge_type <= 0xFF)
	{
		fprintf(stderr, "misc MBC Not implemented");  exit(-1);
	}

	
	memory_bank_controller->cartridge_type = rom_data[0x147];
	memory_bank_controller->rom_size = rom_data[0x148];
	memory_bank_controller->ram_size = rom_data[0x149];
	memory_bank_controller->number_of_rom_banks = rom_data.size() / 0x4000;

	memory_bank_controller->rom = std::move(rom_data);

	memory_bank_controller->allocateRam();

	checkAndLoadSave();


}

Byte GamePak::getMemory(const Word address)
{
	if (!gamepak_loaded)
		return 0xFF;
	return memory_bank_controller->getMemory(address);
}

void GamePak::setMemory(const Word address, const Byte data)
{
	if (!gamepak_loaded)
		return;
	memory_bank_controller->setMemory(address,data);
}

void GamePak::checkAndLoadSave()
{
	// check if cartridge has saving
    	switch (memory_bank_controller->cartridge_type)
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
		memory_bank_controller->has_battery = true;
		break;
	default:
		break;
	}

	if (memory_bank_controller->has_battery)
	{
		std::ifstream file(filename + ".sav", std::ios::binary | std::ios::in); //check for exisiting save, load in data
		if (file.is_open())
		{
			file.read((char*)memory_bank_controller->ram.data(), memory_bank_controller->ram.size());
			file.close();

			return;
		}
	}
}

