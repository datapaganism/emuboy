#pragma once
#include <vector>
#include <string>
#include <memory>

#include "config.hpp"

class MBC
{
public:
	MBC();
	~MBC();

	Byte current_rom_bank = 1;
	Byte current_ram_bank = 0;
	std::vector<Byte> rom;
	std::vector<Byte> ram;
	
	bool ram_bank_enable = false;

	Byte cartridge_type = 0;
	Byte rom_size = 0;
	Byte ram_size = 0;
	Byte number_of_rom_banks = 0;
	Byte number_of_ram_banks = 0;
	bool banking_mode = false;

	bool has_battery = false;

	/*
	virtual void readSave();
	virtual void writeSave();
	
	Byte getCartridgeType();
	Byte getRomSize();
	Byte getRamSize();
	*/

	void allocateRam();

	virtual Byte getMemory(const Word address) = 0;
	virtual void setMemory(const Word address, const Byte data);

	virtual void ramBankEnableHandler(const Word address, const Byte data);
	void ramBankEnable(const Word address, const Byte data);

	virtual void ramBankChange(const Word address, const Byte data);
	virtual void romBankChange(const Word address, const Byte data);
	virtual void bankingModeSelect(const Word address, const Byte data);

	void saveData();
	
	
	//void cartridgeTypeInit();
protected:

	std::string filename;


};

