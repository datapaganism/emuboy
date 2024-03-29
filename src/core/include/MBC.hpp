#pragma once
#include <vector>
#include <string>
#include <memory>
#include <filesystem>
#include <iostream>

#include "config.hpp"

class MBC
{
public:
	MBC();
	~MBC();

	Word current_rom_bank = 1;
	Byte current_ram_bank = 0;
	std::vector<Byte> rom;
	std::vector<Byte> ram;
	
	bool ram_bank_enable = false;

	Byte cartridge_type = 0;
	Byte rom_size = 0;
	Byte ram_size = 0;
	Word number_of_rom_banks = 0;
	Word number_of_ram_banks = 0;
	

	bool has_battery = false;

	void allocateRam();

	virtual Byte getMemory(const Word address);
	virtual void setMemory(const Word address, const Byte data);

	virtual void ramEnable(const Word address, const Byte data);
	void ramEnableHandler(const Word address, const Byte data);

	virtual void ramBankChange(const Word address, const Byte data);
	virtual void romBankChange(const Word address, const Byte data);

	void saveData();
	void checkAndLoadSave();
	std::string save_path;

protected:



};

