#pragma once
#include <vector>
#include <string>

#include "config.hpp"

class GamePak
{
public:
	GamePak(const std::string filename);
	GamePak();

	std::vector<Byte> rom;
	bool gamepak_loaded = false;

	bool save_loaded = false;
	void readSave();
	void writeSave();

	Byte current_rom_bank = 0;
	Byte current_ram_bank = 0;
	Byte getCartridgeType();
	Byte getRomSize();
	Byte getRamSize();

	Byte getMemory(const Word address);
	void setMemory(const Word address, const Byte data);

	std::unique_ptr<Byte[]> ram = nullptr;

	void allocateRam();
	bool ram_bank_enable = false;
	void ramBankEnableHandler(const Word address, const Byte data);
	void ramBankChange(const Word address, const Byte data);
	void romBankChange(const Word address, const Byte data);
private:
	
	std::string filename;

	
};

