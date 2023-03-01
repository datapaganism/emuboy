#pragma once
#include <vector>
#include <string>
#include <memory>
#include <filesystem>

#include "config.hpp"

#include "NO_MBC.hpp"
#include "MBC1.hpp"
#include "MBC2.hpp"
#include "MBC3.hpp"
#include "MBC5.hpp"


class GamePak
{
public:
	GamePak(const std::string filename);
	GamePak();

	void loadROM(const std::string filename);
	void allocateMBC(std::vector<Byte>& rom_data);

	bool gamepak_loaded = false;
	
	std::unique_ptr<MBC> memory_bank_controller;
	
	Byte getMemory(const Word address);
	void setMemory(const Word address, const Byte data);

private:

	std::string rom_path;
	std::string save_path;
};

