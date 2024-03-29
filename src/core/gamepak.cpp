#include <fstream>
#include <vector>
#include <memory>


#include "gamepak.hpp"



GamePak::GamePak(const std::string filename) : GamePak::GamePak()
{
	loadROM(filename);
}

GamePak::GamePak()
{
}

void GamePak::loadROM(const std::string filename)
{
	rom_path = filename;
	size_t last_index = rom_path.find_last_of(".");
	
	std::string extension = rom_path.substr(last_index);

	if (!(extension == ".gb" || extension == ".gbc"))
	{
		fprintf(stderr, "ROM file not valid");  exit(-1);
	}

	save_path = rom_path.substr(0, last_index) + ".sav";

	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (file.is_open())
	{

		std::vector<Byte> rom_data;
		std::ifstream::pos_type pos = file.tellg();
		rom_data.resize(pos); //reallocate memory space to fit entire rom
		file.seekg(0, std::ios::beg);
		file.read((char*)rom_data.data(), pos);
		file.close();

		allocateMBC(rom_data);

		gamepak_loaded = true;
		return;
	}
	fprintf(stderr, "cannot load cartridge");  exit(-1);
}

void GamePak::allocateMBC(std::vector<Byte>& rom_data)
{
	Byte cartridge_type = rom_data[0x147];

	if (cartridge_type == 0)
		memory_bank_controller = std::make_unique<NO_MBC>();
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

	if (!save_path.empty())
		memory_bank_controller->save_path = save_path;

	memory_bank_controller->checkAndLoadSave();


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
