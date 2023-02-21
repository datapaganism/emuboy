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
	}
}

GamePak::GamePak()
{
}

void GamePak::initMBC(std::vector<Byte>& rom_data)
{
	Byte cartridge_type = rom_data[0x147];
	switch (cartridge_type)
	{
	case 0x0:
		memory_bank_controller = std::make_unique<MBC_ROM_ONLY>();
		break;

	case 0x1:
	case 0x2:
	case 0x3:
		memory_bank_controller = std::make_unique<MBC1>();
		break;

	case 0x13:
		memory_bank_controller = std::make_unique<MBC3>();
		break;
	default:
		break;
	}

	memory_bank_controller->cartridge_type = rom_data[0x147];
	memory_bank_controller->rom_size = rom_data[0x148];
	memory_bank_controller->ram_size = rom_data[0x149];
	memory_bank_controller->number_of_rom_banks = rom_data.size() / 0x4000;

	memory_bank_controller->rom = std::move(rom_data);

	memory_bank_controller->allocateRam();


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