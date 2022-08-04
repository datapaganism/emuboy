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
	bool gamepakLoaded = false;

	bool saveLoaded = false;
	void read_save();
	void write_save();

	Byte current_rom_bank = 0;
	Byte current_ram_bank = 0;
	Byte get_cartridge_type();
	Byte get_rom_size();
	Byte get_ram_size();

	Byte get_memory(const Word address);
	void set_memory(const Word address, const Byte data);

	std::unique_ptr<Byte[]> ram = nullptr;

	void allocate_ram();
	bool ram_bank_enable = false;
	void ram_bank_enable_handler(const Word address, const Byte data);
	void ram_bank_change(const Word address, const Byte data);
	void rom_bank_change(const Word address, const Byte data);
private:
	
	std::string filename;

	
};

