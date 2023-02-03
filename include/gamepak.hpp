#pragma once
#include <vector>
#include <string>
#include <memory>

#include "config.hpp"

class GamePak
{
public:
	GamePak(const std::string filename);
	GamePak();

	Byte current_rom_bank = 1;
	Byte current_ram_bank = 0;
	std::vector<Byte> rom;
	std::unique_ptr<Byte[]> ram = nullptr;
	bool gamepak_loaded = false;
	bool save_loaded = false;
	bool ram_bank_enable = false;
	Byte cartridge_type = 0;
	Byte rom_size = 0;
	Byte ram_size = 0;
	Byte number_of_rom_banks = 0;

	void readSave();
	void writeSave();	
	Byte getCartridgeType();
	Byte getRomSize();
	Byte getRamSize();
	Byte getMemory(const Word address);
	void setMemory(const Word address, const Byte data);
	void allocateRam();
	void ramBankEnableHandler(const Word address, const Byte data);
	void ramBankChange(const Word address, const Byte data);
	void romBankChange(const Word address, const Byte data);
	void cartridgeTypeInit();
private:
	
	std::string filename;

	
};

