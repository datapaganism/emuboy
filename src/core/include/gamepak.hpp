#pragma once
#include <vector>
#include <string>
#include <memory>

#include "config.hpp"

#include "MBC_ROM_ONLY.hpp"
#include "MBC1.hpp"
#include "MBC2.hpp"
#include "MBC3.hpp"
#include "MBC5.hpp"


class GamePak
{
public:
	GamePak(const std::string filename);
	GamePak();

	bool gamepak_loaded = false;
	

	void initMBC(std::vector<Byte>& rom_data);
	
	std::unique_ptr<MBC> memory_bank_controller;
	
	Byte getMemory(const Word address);
	void setMemory(const Word address, const Byte data);

	void checkAndLoadSave();

	

private:

	std::string filename;
};

